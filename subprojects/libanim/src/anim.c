#include "libanim/anim.h"
#include "libanim/net.h"
#include "src/extractors/extractor.h"
#include "src/providers/provider.h"
#include "src/providers/tr/turkanime/turkanime.h"

#include <stdlib.h>

static int INITIALIZED = 0;

typedef int (*provider_getter)(_animProvider *);

static size_t PROVIDER_COUNT = 1;
static _animProvider *PROVIDERS;

static provider_getter getters[1] = {tr_turkanime_provider};

int anim_initialize() {
    if (INITIALIZED)
        return 0;

    if (net_init() != 0)
        return 1;

    PROVIDERS = calloc(PROVIDER_COUNT, sizeof(animProvider));
    if (!PROVIDERS)
        return 1;

    for (size_t i = 0; i < PROVIDER_COUNT; i++) {
        if (getters[i](&PROVIDERS[i]) != 0)
            return 1;
    }

    INITIALIZED = 1;
    return 0;
}

void anim_cleanup() {
    if (!INITIALIZED)
        return;
    net_cleanup();

    for (size_t i = 0; i < PROVIDER_COUNT; i++) {
        provider_free(&PROVIDERS[i]);
    }
    free(PROVIDERS);
}

animProvider *anim_list_providers(size_t *size) {
    if (!INITIALIZED)
        return NULL;

    *size = PROVIDER_COUNT;
    return (animProvider *)PROVIDERS;
}

animProvider *anim_get_provider(int id) {
    if (!INITIALIZED)
        return NULL;

    for (size_t i = 0; i < PROVIDER_COUNT; i++) {
        animProvider *ptr = (animProvider *)&PROVIDERS[i];
        if (ptr->id == id)
            return ptr;
    }

    return NULL;
}

animFilter *anim_get_filters(animProvider *ptr, size_t *size) {
    if (!INITIALIZED)
        return NULL;
    _animProvider *_ptr = (_animProvider *)ptr;

    animFilter *filters;
    if (_ptr->data->create_filters(size, &filters) != 0)
        return NULL;

    return filters;
}

// TODO: add filter support.
int anim_search(animProvider *provider, const char *input, size_t *size,
                animEntry **entries) {
    if (!INITIALIZED)
        return -1;
    _animProvider *ptr = (_animProvider *)provider;

    if (ptr->data->search(input, size, entries) != 0)
        return -1;

    for (size_t i = 0; i < *size; ++i) {
        (*entries)[i].provider = provider;
    }

    return 0;
}

int anim_details(animProvider *provider, animEntry *entry) {
    if (!INITIALIZED)
        return -1;
    _animProvider *ptr = (_animProvider *)provider;

    if (ptr->data->details(entry) != 0)
        return -1;
    return 0;
}

int anim_sources(animProvider *provider, animPart *part) {
    if (!INITIALIZED)
        return -1;
    _animProvider *ptr = (_animProvider *)provider;

    if (ptr->data->sources(part) != 0)
        return -1;
    return 0;
}

int anim_download(animSource *source, const char *path, const char *tmp) {
    if (!INITIALIZED)
        return -1;
    if (source->extractor->download(source, path, tmp) != 0)
        return -1;
    return 0;
}

int anim_stream(animSource *source, char **result, const char *tmp) {
    if (!INITIALIZED)
        return -1;
    if (source->extractor->stream(source, result, tmp) != 0)
        return -1;
    return 0;
}

