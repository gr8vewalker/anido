#include "anidopch.h"
#include "extractors.h"

#define EXTRACTOR_COUNT 1
#define EXTRACTOR_ALUCARD "ALUCARD(BETA)"

char **list_extractors(int *len) {
  *len = EXTRACTOR_COUNT;
  char **extractors = calloc(EXTRACTOR_COUNT, sizeof(char *));
  if (extractors == NULL) {
    ANIDO_ERRN("Cannot allocate memory for extractors");
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