#include "extractors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXTRACTOR_COUNT 1
#define EXTRACTOR_ALUCARD "ALUCARD(BETA)"

char **list_extractors(int *len) {
  *len = EXTRACTOR_COUNT;
  char **extractors = calloc(EXTRACTOR_COUNT, sizeof(char *));
  if (extractors == NULL) {
    fprintf(stderr, "Cannot allocate memory for extractors\n");
    return NULL;
  }
  extractors[0] = EXTRACTOR_ALUCARD;
  return extractors;
}

// extractors:
#include "alucard/alucard.h"

extractor *get_extractor(char *name) {
  if (strcmp(name, EXTRACTOR_ALUCARD) == 0) {
    return Alucard_Extractor();
  }
  return NULL;
}