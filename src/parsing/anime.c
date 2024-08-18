#include "anime.h"
#include <stdlib.h>

void parser_free_episode(episode *ptr) {
  free(ptr->link);
  free(ptr->name);
}

void parser_free_anime(anime *ptr) {
  free(ptr->link);
  free(ptr->name);
  for (int i = 0; i < ptr->episodes_len; ++i) {
    parser_free_episode(&ptr->episodes[i]);
  }
  free(ptr->episodes);
}

void parser_free_anime_list(anime_list *ptr) {
  for (int i = 0; i < ptr->len; ++i) {
    parser_free_anime(&ptr->animes[i]);
  }
  free(ptr->animes);
}

void parser_free_source(source *ptr) {
  free(ptr->link);
  free(ptr->name);
  free(ptr->quality);
  free(ptr->extractor);
}

void parser_free_source_list(source_list *ptr) {
  for (int i = 0; i < ptr->len; ++i) {
    parser_free_source(&ptr->sources[i]);
  }
  free(ptr->sources);
}

void parser_free_fansub(fansub *ptr) {
  free(ptr->name);
  for (int i = 0; i < ptr->count; i++) {
    free(ptr->links[i]);
  }
  free(ptr->links);
}

void parser_free_fansub_list(fansub_list *ptr) {
  for (int i = 0; i < ptr->len; ++i) {
    parser_free_fansub(&ptr->fansubs[i]);
  }
  free(ptr->fansubs);
}