#include "libanim/anim.h"
#include "src/log/log.h"
#include "src/opt/opts.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESET_COLORS "\x1b[0m"
#define TEXT_COLOR "\x1b[38;2;20;235;201m"
#define PROGRAM_COLOR "\x1b[38;2;235;201;20m"
#define USER_COLOR "\x1b[38;2;201;20;235m"
#define ERROR_COLOR "\x1b[38;2;234;21;66m"
#define PRINT(...) printf(__VA_ARGS__)

int input_number(long *number);
char *tmpdir();

int main(int argc, char **argv) {
    int retval = 0;
    animProvider *provider = NULL;
    size_t found = 0;
    animEntry *animes = NULL;
    animEntry *anime = NULL;
    animPart *episode = NULL;
    animSource *source = NULL;
    size_t i;

    // ------------ INIT ------------
    anim_initialize();
    parse_opts(argc, argv);

    // ------------ PROVIDER ------------
    if (!PROVIDER) {
        size_t providers_size;
        animProvider *providers = anim_list_providers(&providers_size);
        PRINT(TEXT_COLOR "Available providers:\n");
        for (i = 0; i < providers_size; ++i) {
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
        goto end;
    }

    PRINT(TEXT_COLOR "Selected provider: " PROGRAM_COLOR "%s\n",
          provider->name);

    // ------------ SEARCH ------------
    int cli_search = SEARCH != NULL;
    if (!SEARCH) {
        PRINT(TEXT_COLOR "What do you want to search: " USER_COLOR);
        SEARCH = calloc(1024, sizeof(char));
        if (!fgets(SEARCH, 1024, stdin)) {
            log_error("fgets failed! ferror: %i", ferror(stdin));
            retval = 1;
            goto end;
        }
        // remove trailing newline
        SEARCH[strcspn(SEARCH, "\r\n")] = 0;
    }

    PRINT(TEXT_COLOR "Searching " PROGRAM_COLOR "%s\n", SEARCH);

    if (anim_search(provider, SEARCH, &found, &animes) != 0) {
        log_error("Search failed! Search string was %s", SEARCH);
        retval = 1;
        goto end;
    }

    if (found == 0) {
        if (cli_search) {
            log_warn("No search results found! Exiting...");
            goto end;
        } else {
            PRINT(ERROR_COLOR "No search results!\n");
            goto end; // TODO: get back to search section instead of exiting
        }
    }

    if (!cli_search) {
        PRINT(TEXT_COLOR "Search results: \n");
        for (i = 0; i < found; ++i) {
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

    // ------------ EPISODES ------------
    PRINT(TEXT_COLOR "Getting details for: " PROGRAM_COLOR "%s\n", anime->name);
    if (anim_details(provider, anime) != 0) {
        log_error("Details failed! Anime link was %s", anime->link);
        retval = 1;
        goto end;
    }

    if (anime->parts_size < 1) {
        if (EPISODE > -1) {
            log_error("No episodes found for this anime.");
            retval = 1;
            goto end;
        } else {
            PRINT(ERROR_COLOR "No episodes found!\n");
            goto end; // TODO: get back to search
        }
    }

    if (EPISODE == -1) {
        PRINT(TEXT_COLOR "Episodes: \n");
        for (i = 0; i < anime->parts_size; ++i) {
            PRINT(PROGRAM_COLOR "%zu - %s\n", i + 1, anime->parts[i].name);
        }
        int success = -1;
        do {
            PRINT(TEXT_COLOR "Select an episode: " USER_COLOR);
        } while ((success = input_number(&EPISODE)) || EPISODE < 1 ||
                 EPISODE > anime->parts_size);
    }

    episode = &anime->parts[EPISODE - 1];

    // ------------ SOURCES ------------
    PRINT(TEXT_COLOR "Getting sources for: " PROGRAM_COLOR "%s\n",
          episode->name);
    if (anim_sources(provider, episode) != 0) {
        log_error("Sources failed! Episode link was %s", episode->link);
        retval = 1;
        goto end;
    }

    if (!SOURCE) {
        PRINT(TEXT_COLOR "Sources: \n");
        for (i = 0; i < episode->sources_size; i++) {
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
        for (i = 0; i < episode->sources_size; i++) {
            if (strstr(episode->sources[i].name, SOURCE)) {
                source = &episode->sources[i];
                break;
            }
        }
        if (!source) {
            log_warn("Source not found! Exiting...");
            retval = -1;
            goto end;
        }
    }

    // ------------ DOWNLOADING ------------
    if (!DOWNLOAD_FILE) {
        PRINT(TEXT_COLOR "Where to save the anime: " USER_COLOR);
        DOWNLOAD_FILE = calloc(1024, sizeof(char));
        if (!fgets(DOWNLOAD_FILE, 1024, stdin)) {
            log_error("fgets failed! ferror: %i", ferror(stdin));
            retval = 1;
            goto end;
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
        retval = 1;
        goto end;
    }
    PRINT(TEXT_COLOR "Download finished! File saved to " PROGRAM_COLOR "%s\n", DOWNLOAD_FILE);

end:
    anim_free_entries(animes, found);
    anim_cleanup();
    free(DOWNLOAD_FILE);
    free(SOURCE);
    free(SEARCH);
    free(PROVIDER);
    return retval;
}

int input_number(long *number) {
    char buf[1024];
    if (!fgets(buf, 1024, stdin))
        return 1;

    char *endptr;

    errno = 0;
    *number = strtol(buf, &endptr, 10);
    if (errno == ERANGE) {
        return 2;
    } else if (endptr == buf) {
        return 3;
    } else if (*endptr && *endptr != '\n') {
        return 4;
    }

    return 0;
}

char *tmpdir() {
    if (TEMP_FOLDER) {
        return TEMP_FOLDER;
    }

    char *env[4] = {"TMPDIR", "TMP", "TEMP", "TEMPDIR"};
    for (size_t i = 0; i < sizeof(env) / sizeof(char *); i++) {
        char *value = getenv(env[i]);
        if (value)
            return value;
    }
    return "/tmp";
}
