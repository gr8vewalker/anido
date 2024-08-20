#include "anidopch.h"
#include "opts.h"

#include <getopt.h>
#include "version.h"

int STREAM_FLAG = 0;
int MAX_CURL_HANDLES = 10;
char *PLAYER = "mpv";

static struct option options[] = {{"stream", no_argument, 0, 's'},
                                  {"player", required_argument, 0, 'p'},
                                  {"max-handles", required_argument, 0, 'm'},
                                  {"help", no_argument, 0, 'h'},
                                  {"version", no_argument, 0, 'v'}};

void parse_opts(int argc, char **argv) {
  int opt;
  while ((opt = getopt_long(argc, argv, "sp:m:hv", options, 0)) != -1) {
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
    case 'v':
      ANIDO_LOGFN("Running anido (%s), build on %s", anido_version(),
                  build_date());
      exit(0);
    case 'h':
      ANIDO_LOGN("anido [options]");
      ANIDO_LOGN("");
      ANIDO_LOGN("Options:");
      ANIDO_LOGN(
          "   -s,--stream                Enables stream mode to playback the "
          "anime instead of downloading.");
      ANIDO_LOGN(
          "   -p,--player=<player>       Set player to use in stream mode.");
      ANIDO_LOGN(
          "   -m,--max-handles=<count>   Set how many curl handles to run "
          "parallel while downloading.");
      ANIDO_LOGN("   -h,--help                  Show help.");
      ANIDO_LOGN("   -v,--version               Show version.");
      exit(0);
      break;
    }
  }
}