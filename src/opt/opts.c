#include "opts.h"

#include "libanim/libanimv.h"
#include "src/log/log.h"
#include "src/version.h"
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

int STREAM_FLAG = 0;
int QUERY_FLAG = 0;
char *PLAYER = "mpv";
char *DOWNLOAD_FILE = NULL;
char *TEMP_FOLDER = NULL;

char *PROVIDER = NULL;
char *SEARCH = NULL;
long EPISODE = -1;
char *SOURCE = NULL;

static struct option options[] = {{"stream", no_argument, 0, 's'},
                                  {"player", required_argument, 0, 'p'},
                                  {"help", no_argument, 0, 'h'},
                                  {"version", no_argument, 0, 'v'},
                                  {"provider", required_argument, 0, 'r'},
                                  {"search", required_argument, 0, 'c'},
                                  {"episode", required_argument, 0, 'e'},
                                  {"source", required_argument, 0, 'u'},
                                  {"download", required_argument, 0, 'd'},
                                  {"temp", required_argument, 0, 't'},
                                  {"query", no_argument, 0, 'q'}};

void parse_opts(int argc, char **argv) {
    int opt;
    while ((opt = getopt_long(argc, argv, "sp:hvr:c:e:u:d:t:q", options, 0)) !=
           -1) {
        switch (opt) {
        case 's':
            STREAM_FLAG = 1;
            break;
        case 'q':
            QUERY_FLAG = 1;
            break;
        case 'p':
            PLAYER = strdup(optarg);
            break;
        case 'r':
            PROVIDER = strdup(optarg);
            break;
        case 'c':
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
                "   -s,--stream                Enables stream mode to playback the anime instead of downloading.\n"
                "   -p,--player=<player>       Set player to use in stream mode.\n"
                "   -h,--help                  Show help.\n"
                "   -v,--version               Show version.\n"
                "   -r,--provider=<provider>   Set provider to use.\n"
                "   -c,--search=<search>       Set what to search. Automatically selects first result.\n"
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
