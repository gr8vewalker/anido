#include "util.h"

#include <stdio.h>
#include <stdlib.h>

#include "src/opt/opts.h"

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
