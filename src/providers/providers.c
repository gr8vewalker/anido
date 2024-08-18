#include "providers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// providers list
#define PROVIDER_COUNT 1
#define PROVIDER_TURKANIME "TurkAnime"

static provider *current_provider;

void load_TurkAnime();

provider *get_current_provider() { return current_provider; }

char **list_providers(int *len) {
  *len = PROVIDER_COUNT;
  char **providers = calloc(PROVIDER_COUNT, sizeof(char *));
  if (providers == NULL) {
    fprintf(stderr, "Cannot allocate memory for providers\n");
    return NULL;
  }
  providers[0] = PROVIDER_TURKANIME;
  return providers;
}

void load_provider(const char *name) {
  if (strcmp(name, PROVIDER_TURKANIME) == 0) {
    load_TurkAnime();
  }
}

#include "tr/turkanime.h"

void load_TurkAnime() {
  if (current_provider)
    free(current_provider);

  current_provider = TurkAnime();
}