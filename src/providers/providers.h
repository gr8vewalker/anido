#ifndef ANIMEDOWN_PROVIDERS_H
#define ANIMEDOWN_PROVIDERS_H

#include "../parsing/anime.h"

typedef int (*search_fnc)(const char *, void **);
typedef int (*populate_fnc)(anime *);
typedef int (*sourcext_fnc)(const episode *, source_list **);

typedef struct {
  char *name;
  search_fnc search;
  populate_fnc populate;
  sourcext_fnc sourcext;
} provider;

// Providers:
provider *get_current_provider();
char **list_providers(int *len);
void load_provider(const char *name);

#endif