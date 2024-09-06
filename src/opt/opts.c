#include "opts.h"

#include "libanim/libanimv.h"
#include "src/log/log.h"
#include "src/version.h"
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

int QUERY_FLAG = 0;
char *DOWNLOAD_FILE = NULL;
char *TEMP_FOLDER = NULL;

char *PROVIDER = NULL;
char *SEARCH = NULL;
long EPISODE = -1;
char *SOURCE = NULL;

static struct option options[] = {{"help", no_argument, 0, 'h'},
                                  {"version", no_argument, 0, 'v'},
                                  {"provider", required_argument, 0, 'p'},
                                  {"search", required_argument, 0, 's'},
                                  {"episode", required_argument, 0, 'e'},
                                  {"source", required_argument, 0, 'u'},
                                  {"download", required_argument, 0, 'd'},
                                  {"temp", required_argument, 0, 't'},
                                  {"query", no_argument, 0, 'q'}};

void parse_opts(int argc, char **argv) {
    int opt;
    while ((opt = getopt_long(argc, argv, "hvp:s:e:u:d:t:q", options, 0)) !=
           -1) {
        switch (opt) {
        case 'q':
            QUERY_FLAG = 1;
            break;
        case 'p':
            PROVIDER = strdup(optarg);
            break;
        case 's':
            SEARCH = strdup(optarg);
            break;
        case 'e':
            EPISODE = atoi(optarg);
            break;
        case 'u':
            SOURCE = strdup(optarg);
            break;
        case 'd':
            DOWNLOAD_FILE = strdup(optarg);
            break;
        case 't':
            TEMP_FOLDER = strdup(optarg);
            break;
        case 'v':
            log_info("Running anido (%s), built on %s", anido_version(),
                     anido_build_date());
            log_info("libanim (%s), built on %s", libanim_version(),
                     libanim_build_date());
            exit(0);
        case 'h':
            log_info(
                "Help: \n"
                "anido [options]\n"
                "\n"
                "Options:\n"
                "   -h,--help                  Show help.\n"
                "   -v,--version               Show version.\n"
                "   -p,--provider=<provider>   Set provider to use.\n"
                "   -s,--search=<search>       Set what to search. Automatically selects first result.\n"
                "   -e,--episode=<episode>     Set episode index to select.\n"
                "   -u,--source=<source>       Set source name to select. First one containing is selected.\n"
                "   -d,--download=<path>       Set download path.\n"
                "   -t,--tmp=<path>            Set tmp directory path.\n"
                "   -q,--query                 Switches to data querying mode.\n");
            exit(0);
            break;
        }
    }
}
