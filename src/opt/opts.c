#include "opts.h"

#include "libanim/libanimv.h"
#include "src/log/log.h"
#include "src/version.h"
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

int STREAM_FLAG = 0;
char *PLAYER = "mpv";

char *PROVIDER = NULL;
char *SEARCH = NULL;
long EPISODE = -1;
char *SOURCE = NULL;
char *QUALITY = NULL;

static struct option options[] = {{"stream", no_argument, 0, 's'},
                                  {"player", required_argument, 0, 'p'},
                                  {"help", no_argument, 0, 'h'},
                                  {"version", no_argument, 0, 'v'},
                                  {"provider", required_argument, 0, 'r'},
                                  {"search", required_argument, 0, 'c'},
                                  {"episode", required_argument, 0, 'e'},
                                  {"source", required_argument, 0, 'u'},
                                  {"quality", required_argument, 0, 'q'}};

void parse_opts(int argc, char **argv) {
    int opt;
    while ((opt = getopt_long(argc, argv, "sp:hvr:c:e:u:q:", options, 0)) != -1) {
        switch (opt) {
        case 's':
            STREAM_FLAG = 1;
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
        case 'q':
            QUALITY = strdup(optarg);
            break;
        case 'v':
            log_info("Running anido (%s), built on %s", anido_version(),
                     anido_build_date());
            log_info("libanim (%s), built on %s", libanim_version(),
                     libanim_build_date());
            exit(0);
        case 'h':
            log_info("anido [options]");
            log_info("");
            log_info("Options:");
            log_info(
                "   -s,--stream                Enables stream mode to playback the anime instead of downloading.");
            log_info(
                "   -p,--player=<player>       Set player to use in stream mode.");
            log_info("   -h,--help                  Show help.");
            log_info("   -v,--version               Show version.");
            exit(0);
            break;
        }
    }
}
