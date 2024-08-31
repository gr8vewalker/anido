#ifndef LIBANIM_ANIM_H
#define LIBANIM_ANIM_H

#include <stddef.h>

typedef enum animFilterType { SELECTION, TEXT, CHECK } animFilterType;

typedef struct animSelectionFilter {
    size_t size;
    char **elements;
    char **data; // if elements are same with data this should be NULL.
    int value;
} animSelectionFilter;

typedef struct animTextFilter {
    size_t max_size;
    char *value;
} animTextFilter;

typedef struct animCheckFilter {
    int value;
} animCheckFilter;

typedef struct animFilter {
    animFilterType type;
    char *name;
    union {
        animSelectionFilter *selection;
        animTextFilter *text;
        animCheckFilter *check;
    } data;
} animFilter;

struct animExtractor;
struct animSource;
struct animEntry;
struct animPart;
struct animProvider;

typedef struct animSource {
    struct animPart *part;
    struct animExtractor *extractor;
    char *name;
    char *link;
} animSource;

typedef struct animPart {
    struct animEntry *entry;
    char *name;
    char *link;
    size_t sources_size;
    animSource *sources;
} animPart;

typedef struct animEntry {
    struct animProvider *provider;
    char *name;
    char *link;
    size_t parts_size;
    animPart *parts;
} animEntry;

typedef struct animProvider {
    int id;
    char *name;
    void *data; // internal
} animProvider;

int anim_initialize();
void anim_cleanup();

animProvider *anim_list_providers(size_t *size);
animProvider *anim_get_provider(int id);
animFilter *anim_get_filters(animProvider *provider,
                                            size_t *size);

int anim_search(animProvider *provider, const char *input,
                               size_t *size, animEntry **entries);
int anim_details(animProvider *provider, animEntry *entry);
int anim_sources(animProvider *provider, animPart *part);

int anim_download(animSource *source, const char *path, const char *tmp);
int anim_stream(animSource *source, char **result, const char *tmp);

#endif
