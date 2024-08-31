#include "libanim/anim.h"
#include "src/log/log.h"
#include "src/opt/opts.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BRIGHT_YELLOW "\x1b[93m"
#define BRIGHT_BLUE "\x1b[94m"
#define PRINT(...) printf(__VA_ARGS__)

int input_number(long *number);

int main(int argc, char **argv) {
    int retval = 0;
    animProvider *provider = NULL;
    animPart *part = NULL;
    animSource *source = NULL;
    size_t i;

    anim_initialize();
    parse_opts(argc, argv);

    size_t providers_size;
    animProvider *providers = anim_list_providers(&providers_size);

    if (!PROVIDER) {
        PRINT(BRIGHT_BLUE "Available providers:\n");
        for (i = 0; i < providers_size; ++i) {
            PRINT(BRIGHT_YELLOW "%zu - %s\n", i + 1, providers[i].name);
        }

        int success = -1;
        long in = 0;
        do {
            PRINT(BRIGHT_BLUE "Select a provider: " BRIGHT_YELLOW);
        } while ((success = input_number(&in)) || in < 1 ||
                 in > providers_size);
        provider = &providers[in - 1];
    } else {
        for (i = 0; i < providers_size; ++i) {
            if (strncmp(PROVIDER, providers[i].name, strlen(PROVIDER)) != 0)
                continue;
            provider = &providers[i];
            break;
        }
    }

    if (!provider) {
        log_error(
            "Error while getting provider. Provider command argument was %s",
            PROVIDER ? PROVIDER : "not set");
        goto end;
    }

    PRINT(BRIGHT_BLUE "Selected provider: " BRIGHT_YELLOW "%s\n",
          provider->name);

end:
    anim_cleanup();
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
