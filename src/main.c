#include "libanim/anim.h"
#include "src/log/log.h"
#include "src/opt/opts.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEXT_COLOR "\x1b[38;2;20;235;201m"
#define PROGRAM_COLOR "\x1b[38;2;235;201;20m"
#define USER_COLOR "\x1b[38;2;201;20;235m"
#define ERROR_COLOR "\x1b[38;2;234;21;66m"
#define PRINT(...) printf(__VA_ARGS__)

int input_number(long *number);

int main(int argc, char **argv) {
    int retval = 0;
    animProvider *provider = NULL;
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

    size_t found = 0;
    animEntry *animes;
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
            PRINT(TEXT_COLOR "Select an episode: " USER_COLOR);
        } while ((success = input_number(&in)) || in < 1 ||
                 in > found);
        anime = &animes[in - 1];
    } else {
        anime = animes;
    }


end:
    anim_cleanup();
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
