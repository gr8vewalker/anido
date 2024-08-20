#ifndef ANIMEDOWN_EXTRACTORS_H
#define ANIMEDOWN_EXTRACTORS_H

#include "parsing/anime.h"

typedef source_list *(*extract_fnc)(const char*, const char*);
typedef char *(*stream_fnc)(const source*);
typedef char *(*download_fnc)(const source*, const char*);

typedef struct {
    char *name;
    extract_fnc extract;
    stream_fnc stream;
    download_fnc download;
} extractor;

char **list_extractors(int *len);
extractor *get_extractor(char *name);

#endif