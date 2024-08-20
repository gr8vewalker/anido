#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <curl/curl.h>

#include "extractors/extractors.h"
#include "opt/opts.h"
#include "parsing/anime.h"
#include "providers/providers.h"
#include "util/file.h"
#include "util/log.h"
#include "util/video.h"

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

  // TODO: make this $TMPDIR/animetmp default and changeable from options
  if (remove_dir("animetmp") != 0)
    ANIDO_LOGN(ESCAPE_YELLOW
               "warn: temp directory may not be fully cleared" ESCAPE_RESET);

  if (create_dir("animetmp") != 0)
    ANIDO_LOGN(ESCAPE_YELLOW
               "warn: temp directory may not be created" ESCAPE_RESET);

  curl_global_init(CURL_GLOBAL_ALL);

  select_provider();
  provider *selected_provider = get_current_provider();

  char *search_query = search_input();

  if (!search_query) {
    ANIDO_ERRN("search query string was NULL, quitting.");
    return -1;
  }

  void *ptr;
  int res = selected_provider->search(search_query, &ptr);
  free(search_query);

  if (res == -1) {
    ANIDO_ERRN("search was unsuccessful, quitting.");
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
  CLEAR_LOG();
  ANIDO_LOGPN("Available providers:");

  int len, i;
  char **providers = list_providers(&len);

  for (i = 0; i < len; i++) {
    ANIDO_LOGFN(ESCAPE_BOLD ESCAPE_BLUE "%u: " ESCAPE_RESET ESCAPE_MAGENTA "%s",
                i + 1, providers[i]);
  }

  ANIDO_LOGP("Select: " ESCAPE_BLUE);

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
  CLEAR_LOG();

  ANIDO_LOGFPN("Provider: " ESCAPE_RESET ESCAPE_RED "%s",
               get_current_provider()->name);
  ANIDO_LOGP("Search: ");

  char *search =
      calloc(500 + 1, sizeof(char)); // max 500 chars is enough i think
  if (!search) {
    ANIDO_ERRN("Memory allocation error!");
    return NULL;
  }

  scanf(" %500[^\n]", search);
  return search;
}

void select_anime(const anime_list *list, anime **ptr) {
  CLEAR_LOG();
  ANIDO_LOGFPN("Search results " ESCAPE_RESET ESCAPE_RED "(%d)", list->len);

  for (int i = 0; i < list->len; i++) {
    anime anime = list->animes[i];
    ANIDO_LOGFN(ESCAPE_BOLD ESCAPE_BLUE "%u: " ESCAPE_RESET ESCAPE_RESET "%s",
                i + 1, anime.name);
  }

  ANIDO_LOGP("Select: ");

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
  CLEAR_LOG();
  ANIDO_LOGFPN("Episodes for " ESCAPE_RESET ESCAPE_RED "%s (%d)", anime->name,
               anime->episodes_len);

  for (int i = 0; i < anime->episodes_len; i++) {
    episode episode = anime->episodes[i];
    ANIDO_LOGFN(ESCAPE_BOLD ESCAPE_BLUE "%u: " ESCAPE_RESET ESCAPE_RED "%s",
                i + 1, episode.name);
  }

  ANIDO_LOGP("Select: ");

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
  CLEAR_LOG();
  ANIDO_LOGPN("Found sources: ");

  for (int i = 0; i < list->len; i++) {
    source source = list->sources[i];
    ANIDO_LOGFN(ESCAPE_BOLD ESCAPE_BLUE "%u: " ESCAPE_RESET ESCAPE_RED
                                        "%s (%s)",
                i + 1, source.name, source.quality);
  }

  ANIDO_LOGP("Select: ");

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

  ANIDO_LOGFPN("Anime saved to " ESCAPE_RESET ESCAPE_YELLOW "%s", file);
}