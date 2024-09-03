#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libanim/anim.h"
#include "libanim/util.h"

#include "src/log/log.h"
#include "src/opt/opts.h"
#include "src/util/util.h"

#define RESET_COLORS "\x1b[0m"
#define TEXT_COLOR "\x1b[38;2;20;235;201m"
#define PROGRAM_COLOR "\x1b[38;2;235;201;20m"
#define USER_COLOR "\x1b[38;2;201;20;235m"
#define ERROR_COLOR "\x1b[38;2;234;21;66m"
#define PRINT(...)                                                             \
    if (!QUERY_FLAG)                                                           \
    printf(__VA_ARGS__)

static animProvider *provider = NULL;
static size_t found = 0;
static animEntry *animes = NULL;
static animEntry *anime = NULL;
static animPart *episode = NULL;
static animSource *source = NULL;

int _provider(size_t *running);
int _search(size_t *running);
int _episode(size_t *running);
int _source(size_t *running);
int _download(size_t *running);
int _stream(size_t *running);

void _query();

int main(int argc, char **argv) {
    int retval = 0;

    anim_initialize();
    parse_opts(argc, argv);

    int (*functions[5])(size_t *) = {_provider, _search, _episode, _source,
                                     STREAM_FLAG ? _stream : _download};

    size_t functions_size = sizeof(functions) / sizeof(void *);
    size_t running = 0;

    while (running >= 0 && running < functions_size) {
        retval = functions[running](&running);
        if (retval)
            break;
    }

    if (QUERY_FLAG) {
        _query();
    }

    anim_free_entries(animes, found);
    anim_cleanup();
    free(DOWNLOAD_FILE);
    free(SOURCE);
    free(SEARCH);
    free(PROVIDER);
    return retval;
}

int _provider(size_t *running) {
    if (!PROVIDER) {
        if (QUERY_FLAG) {
            return 1;
        }

        size_t providers_size;
        animProvider *providers = anim_list_providers(&providers_size);
        PRINT(TEXT_COLOR "Available providers:\n");
        for (size_t i = 0; i < providers_size; ++i) {
            PRINT(PROGRAM_COLOR "%zu - %s\n", i + 1, providers[i].name);
        }

        int success = -1;
        long in = 0;
        do {
            PRINT(TEXT_COLOR "Select a provider: " USER_COLOR);
        } while ((success = input_number(&in)) || in < 1 ||
                 in > providers_size);
        provider = &providers[in - 1];
    } else {
        provider = anim_get_provider(PROVIDER, 0);
    }

    if (!provider) {
        log_error(
            "Error while getting provider. Provider command argument was %s",
            PROVIDER ? PROVIDER : "not set");
        return 1;
    }

    PRINT(TEXT_COLOR "Selected provider: " PROGRAM_COLOR "%s\n",
          provider->name);

    (*running)++;
    return 0;
}

int _search(size_t *running) {
    int cli_search = SEARCH != NULL;
    if (!cli_search) {
        if (QUERY_FLAG) {
            return 1;
        }

        PRINT(TEXT_COLOR "What do you want to search: " USER_COLOR);
        SEARCH = calloc(1024, sizeof(char));
        if (!fgets(SEARCH, 1024, stdin)) {
            log_error("fgets failed! ferror: %i", ferror(stdin));
            return 1;
        }
        // remove trailing newline
        SEARCH[strcspn(SEARCH, "\r\n")] = 0;
    }

    PRINT(TEXT_COLOR "Searching " PROGRAM_COLOR "%s\n", SEARCH);

    anim_free_entries(animes, found); // Free old ones
    if (anim_search(provider, SEARCH, &found, &animes) != 0) {
        log_error("Search failed! Search string was %s", SEARCH);
        return 1;
    }

    if (found == 0) {
        if (cli_search) {
            log_warn("No search results found! Exiting...");
            return 1;
        } else {
            PRINT(ERROR_COLOR "No search results!\n");

            free(SEARCH);
            SEARCH = NULL;
            return 0; // Not editing running so it will run the search function
                      // again.
        }
    }

    if (!cli_search) {
        PRINT(TEXT_COLOR "Search results: \n");
        for (size_t i = 0; i < found; ++i) {
            PRINT(PROGRAM_COLOR "%zu - %s\n", i + 1, animes[i].name);
        }
        int success = -1;
        long in = 0;
        do {
            PRINT(TEXT_COLOR "Select an anime: " USER_COLOR);
        } while ((success = input_number(&in)) || in < 1 || in > found);
        anime = &animes[in - 1];
    } else {
        anime = animes;
    }

    (*running)++;
    return 0;
}

int _episode(size_t *running) {
    PRINT(TEXT_COLOR "Getting details for: " PROGRAM_COLOR "%s\n", anime->name);
    if (anim_details(provider, anime) != 0) {
        log_error("Details failed! Anime link was %s", anime->link);
        return 1;
    }

    if (anime->parts_size < 1) {
        if (EPISODE > -1) {
            log_error("No episodes found for this anime.");
            return 1;
        } else {
            PRINT(ERROR_COLOR "No episodes found!\n");

            SEARCH = NULL;
            (*running)--; // get back to search
            return 0;
        }
    }

    if (EPISODE == -1) {
        if (QUERY_FLAG) {
            return 1;
        }

        PRINT(TEXT_COLOR "Episodes: \n");
        for (size_t i = 0; i < anime->parts_size; ++i) {
            PRINT(PROGRAM_COLOR "%zu - %s\n", i + 1, anime->parts[i].name);
        }
        int success = -1;
        do {
            PRINT(TEXT_COLOR "Select an episode: " USER_COLOR);
        } while ((success = input_number(&EPISODE)) || EPISODE < 1 ||
                 EPISODE > anime->parts_size);
    }

    episode = &anime->parts[EPISODE - 1];

    (*running)++;
    return 0;
}

int _source(size_t *running) {
    PRINT(TEXT_COLOR "Getting sources for: " PROGRAM_COLOR "%s\n",
          episode->name);
    if (anim_sources(provider, episode) != 0) {
        log_error("Sources failed! Episode link was %s", episode->link);
        return 1;
    }

    if (!SOURCE) {
        if (QUERY_FLAG) {
            return 1;
        }

        PRINT(TEXT_COLOR "Sources: \n");
        for (size_t i = 0; i < episode->sources_size; i++) {
            PRINT(PROGRAM_COLOR "%zu - %s\n", i + 1, episode->sources[i].name);
        }

        int success = -1;
        long in;
        do {
            PRINT(TEXT_COLOR "Select a source: " USER_COLOR);
        } while ((success = input_number(&in)) || in < 1 ||
                 in > episode->sources_size);
        source = &episode->sources[in - 1];
    } else {
        for (size_t i = 0; i < episode->sources_size; i++) {
            if (strstr(episode->sources[i].name, SOURCE)) {
                source = &episode->sources[i];
                break;
            }
        }
        if (!source) {
            log_warn("Source not found! Exiting...");
            return 1;
        }
    }

    (*running)++;
    return 0;
}

int _download(size_t *running) {
    if (QUERY_FLAG) {
        return 1;
    }

    if (!DOWNLOAD_FILE) {
        PRINT(TEXT_COLOR "Where to save the anime: " USER_COLOR);
        DOWNLOAD_FILE = calloc(1024, sizeof(char));
        if (!fgets(DOWNLOAD_FILE, 1024, stdin)) {
            log_error("fgets failed! ferror: %i", ferror(stdin));
            return 1;
        }
        DOWNLOAD_FILE[strcspn(DOWNLOAD_FILE, "\r\n")] = 0;
    }

    PRINT(TEXT_COLOR "Downloading " PROGRAM_COLOR "%s (%s) " TEXT_COLOR
                     "to " PROGRAM_COLOR "%s",
          episode->name, source->name, DOWNLOAD_FILE);

    PRINT(RESET_COLORS "\n"); // Do not color other outputs like ffmpeg
    if (anim_download(source, DOWNLOAD_FILE, tmpdir()) != 0) {
        log_error("Download failed! Was downloading %s to %s. tmp: %s",
                  source->link, DOWNLOAD_FILE, tmpdir());
        return 1;
    }
    PRINT(TEXT_COLOR "Download finished! File saved to " PROGRAM_COLOR "%s\n",
          DOWNLOAD_FILE);

    (*running)++;
    return 0;
}

int _stream(size_t *running) {
    if (QUERY_FLAG) {
        return 1;
    }

    PRINT(TEXT_COLOR "Playing " PROGRAM_COLOR "%s (%s) ", episode->name,
          source->name);

    PRINT(RESET_COLORS "\n"); // Do not color other outputs like mpv/vlc
    char *result;
    if (anim_stream(source, &result, tmpdir()) != 0) {
        log_error("Stream failed! Trying to stream %s. tmp: %s", source->link,
                  tmpdir());
        return 1;
    }

    char *cmd = format_string("%s \"%s\"", PLAYER, result);

    system(cmd);

    free(cmd);
    free(result);
    (*running)++;
    return 0;
}


void _query() {
    // TODO: add json support with cJSON
    if (provider) {
        printf("Provider: %s\n", provider->name);
    }

    if (animes) {
        printf("Animes found: %zu\n", found);
        for (size_t i = 0; i < found; i++) {
            printf("[%zu] Anime name: %s\n", i, animes[i].name);
            printf("[%zu] Anime link: %s\n", i, animes[i].link);
        }
    }

    if(anime) {
        printf("[SELECTED] Anime name: %s\n", anime->name);
        printf("[SELECTED] Anime link: %s\n", anime->link);
        printf("Episodes found: %zu\n", anime->parts_size);
        for (size_t i = 0; i < anime->parts_size; i++) {
            printf("[%zu] Episode name: %s\n", i, anime->parts[i].name);
            printf("[%zu] Episode link: %s\n", i, anime->parts[i].link);
        }
    }

    if (episode) {
        printf("[SELECTED] Episode name: %s\n", episode->name);
        printf("[SELECTED] Episode link: %s\n", episode->link);
        printf("Sources found: %zu\n", episode->sources_size);
        for (size_t i = 0; i < episode->sources_size; i++) {
            printf("[%zu] Source name: %s\n", i, episode->sources[i].name);
            printf("[%zu] Source link: %s\n", i, episode->sources[i].link);
        }
    }

    if (source) {
        printf("[SELECTED] Source name: %s\n", source->name);
        printf("[SELECTED] Source link: %s\n", source->link);
    }
}
