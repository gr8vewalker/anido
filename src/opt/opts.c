#include "opts.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

int STREAM_FLAG = 0;
int MAX_CURL_HANDLES = 10;
char *PLAYER = "mpv";

static struct option options[] = {
    {"stream", no_argument, 0, 's'},
    {"player", required_argument, 0, 'p'},
    {"max-handles", required_argument, 0, 'm'},
    {"help", no_argument, 0, 'h'}
};

void parse_opts(int argc, char **argv) {
  int opt;
  while ((opt = getopt_long(argc, argv, "sp:m:h", options, 0)) != -1) {
    switch (opt) {
    case 's':
      STREAM_FLAG = 1;
      break;
    case 'p':
      PLAYER = optarg;
      break;
    case 'm':
      MAX_CURL_HANDLES = atoi(optarg);
      break;
    case 'h':
      puts("anido [options]");
      puts("");
      puts("Options:");
      puts("   -s,--stream                Enables stream mode to playback the anime instead of downloading.");
      puts("   -p,--player=<player>       Set player to use in stream mode.");
      puts("   -m,--max-handles=<count>   Set how many curl handles to run parallel while downloading.");
      puts("   -h,--help                  Show help.");
      exit(0);
      break;
    }
  }
}