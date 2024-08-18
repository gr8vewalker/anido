#include "extractors/extractors.h"
#include "opt/opts.h"
#include "parsing/anime.h"
#include "util/file.h"
#include "util/video.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CLEAR() printf("\x1b[1;1H\x1b[2J");

#define ANSI_BOLD "\x1b[1m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_BRIGHT_GREEN "\x1b[92m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define PREFIX_ARROW ANSI_COLOR_CYAN "\u276f " ANSI_COLOR_RESET
#define PRINT_PREFIX PREFIX_ARROW ANSI_BOLD ANSI_COLOR_BRIGHT_GREEN

#include "providers/providers.h"

void select_provider();
char *search_input();
void select_anime(const anime_list *list, anime **ptr);
void select_episode(const anime *anime, episode **ptr);
void select_source(const source_list *list, source **ptr);
void stream_sources(const source_list *list, const char *file);
void download_source(const char *episode_name, const source *ptr);

int main(int argc, char **argv) {
#ifdef _WIN32
  // for future!
  SetConsoleOutputCP(65001);
#endif

  parse_opts(argc, argv);

  if (remove_dir("animetmp") != 0)
    fputs("warn: temp directory may not be fully cleared\n", stderr);

  if (create_dir("animetmp") != 0)
    fputs("warn: temp directory may not be created\n", stderr);

  curl_global_init(CURL_GLOBAL_ALL);

  select_provider();
  provider *selected_provider = get_current_provider();

  char *search_query = search_input();

  if (!search_query) {
    fputs("search query string was NULL, quitting.\n", stderr);
    return -1;
  }

  void *ptr;
  int res = selected_provider->search(search_query, &ptr);
  free(search_query);

  if (res == -1) {
    fputs("search was unsuccessful, quitting.\n", stderr);
    return -1;
  }

  anime *selected;

  if (res == 1) {
    selected = (anime *)ptr;
  } else {
    anime_list *list = (anime_list *)ptr;
    select_anime(list, &selected);
    parser_free_anime_list(list);
  }

  selected_provider->populate(selected);

  episode *selected_episode;
  select_episode(selected, &selected_episode);

  source_list *sources_found;
  selected_provider->sourcext(selected_episode, &sources_found);

  if (STREAM_FLAG) {
    char *stream_file;
    asprintf(&stream_file, "%s.m3u8", selected_episode->name);
    puts(stream_file);

    stream_sources(sources_found, stream_file);
    run_player(PLAYER, stream_file);

    free(stream_file);
  } else {
    source *selected_source;
    select_source(sources_found, &selected_source);
    download_source(selected_episode->name, selected_source);
  }

  parser_free_source_list(sources_found);
  free(sources_found);
  parser_free_anime(selected);
  free(selected);
  free(selected_provider);

  curl_global_cleanup();
  return 0;
}

void select_provider() {
  CLEAR();
  puts(PRINT_PREFIX "Available providers:" ANSI_COLOR_RESET);

  int len, i;
  char **providers = list_providers(&len);

  for (i = 0; i < len; i++) {
    printf(ANSI_BOLD ANSI_COLOR_BLUE "%u: " ANSI_COLOR_RESET ANSI_COLOR_MAGENTA
                                     "%s\n" ANSI_COLOR_RESET,
           i + 1, providers[i]);
  }

  printf(PRINT_PREFIX "Select: " ANSI_COLOR_RESET ANSI_COLOR_BLUE);

  int sel;
  scanf(" %u", &sel);
  --sel;

  if (sel < 0 || sel >= len) {
    select_provider();
    return;
  }

  load_provider(providers[sel]);
  free(providers);
}

char *search_input() {
  CLEAR();

  printf(PRINT_PREFIX "Provider: " ANSI_COLOR_RESET ANSI_COLOR_RED "%s\n",
         get_current_provider()->name);

  printf(PRINT_PREFIX "Search: " ANSI_COLOR_RESET);

  char *search =
      calloc(500 + 1, sizeof(char)); // max 500 chars is enough i think
  if (!search) {
    fputs("Memory allocation error!\n", stderr);
    return NULL;
  }

  scanf(" %500[^\n]", search);
  return search;
}

void select_anime(const anime_list *list, anime **ptr) {
  CLEAR();
  printf(PRINT_PREFIX "Search results " ANSI_COLOR_RESET ANSI_COLOR_RED
                      "(%d)\n",
         list->len);

  for (int i = 0; i < list->len; i++) {
    anime anime = list->animes[i];
    printf(ANSI_BOLD ANSI_COLOR_BLUE "%u: " ANSI_COLOR_RESET ANSI_COLOR_RED
                                     "%s\n",
           i + 1, anime.name);
  }

  printf(PRINT_PREFIX "Select: " ANSI_COLOR_RESET);

  int sel;
  scanf(" %u", &sel);
  --sel;

  if (sel < 0 || sel >= list->len) {
    select_anime(list, ptr);
    return;
  }

  *ptr = malloc(sizeof(anime));
  (*ptr)->name = strdup(list->animes[sel].name);
  (*ptr)->link = strdup(list->animes[sel].link);
  (*ptr)->episodes_len = 0; // no information about episode yet.
}

void select_episode(const anime *anime, episode **ptr) {
  CLEAR();
  printf(PRINT_PREFIX "Episodes for " ANSI_COLOR_RESET ANSI_COLOR_RED
                      "%s (%d)\n",
         anime->name, anime->episodes_len);

  for (int i = 0; i < anime->episodes_len; i++) {
    episode episode = anime->episodes[i];
    printf(ANSI_BOLD ANSI_COLOR_BLUE "%u: " ANSI_COLOR_RESET ANSI_COLOR_RED
                                     "%s\n",
           i + 1, episode.name);
  }

  printf(PRINT_PREFIX "Select: " ANSI_COLOR_RESET);

  int sel;
  scanf(" %u", &sel);
  --sel;

  if (sel < 0 || sel >= anime->episodes_len) {
    select_episode(anime, ptr);
    return;
  }

  *ptr = &anime->episodes[sel];
}

void select_source(const source_list *list, source **ptr) {
  CLEAR();
  puts(PRINT_PREFIX "Found sources: " ANSI_COLOR_RESET);

  for (int i = 0; i < list->len; i++) {
    source source = list->sources[i];
    printf(ANSI_BOLD ANSI_COLOR_BLUE "%u: " ANSI_COLOR_RESET ANSI_COLOR_RED
                                     "%s (%s)\n",
           i + 1, source.name, source.quality);
  }

  printf(PRINT_PREFIX "Select: " ANSI_COLOR_RESET);

  int sel;
  scanf(" %u", &sel);
  --sel;

  if (sel < 0 || sel >= list->len) {
    select_source(list, ptr);
    return;
  }

  *ptr = &list->sources[sel];
}

void stream_sources(const source_list *list, const char *file) {
  FILE *f = fopen(file, "wb");
  fputs("#EXTM3U\n", f);

  for (int i = 0; i < list->len; i++) {
    source src = list->sources[i];
    char *output = get_extractor(src.extractor)->stream(&src);
    fprintf(f, "#EXTINF\n%s\n", output);
    free(output);
  }

  fclose(f);
}

void download_source(const char *episode_name, const source *ptr) {
  char *file = get_extractor(ptr->extractor)->download(ptr, episode_name);

  printf(PRINT_PREFIX "Anime saved to " ANSI_COLOR_RESET ANSI_COLOR_YELLOW
                      "%s\n",
         file);
}