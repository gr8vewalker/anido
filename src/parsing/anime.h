#ifndef ANIMEDOWN_ANIME_H
#define ANIMEDOWN_ANIME_H

typedef struct {
  char *name;
  char *link;
} episode;

typedef struct {
  char *name;
  char *link;
  int episodes_len;
  episode *episodes;
} anime;

typedef struct {
  int len;
  anime *animes;
} anime_list;

typedef struct {
  char *name;
  char *quality;
  char *link;
  char *extractor; // For using it again
} source;

typedef struct {
  int len;
  source *sources;
} source_list;

typedef struct {
  char *name;
  int count;
  char **links;
} fansub;

typedef struct {
  int len;
  fansub *fansubs;
} fansub_list;

// Freeing functions

void parser_free_episode(episode *ptr);
void parser_free_anime(anime *ptr);
void parser_free_anime_list(anime_list *ptr);
void parser_free_source(source *ptr);
void parser_free_source_list(source_list *ptr);
void parser_free_fansub(fansub *ptr);
void parser_free_fansub_list(fansub_list *ptr);

#endif