#include "video.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ffmpeg_concat(const char *concat_file, const char *merged_video) {
  char *str = "";
  asprintf(&str, "ffmpeg -f concat -i \"%s\" -c copy \"%s\"", concat_file,
           merged_video);
  system(str);
}

void run_player(const char *player, const char *file) {
  char *extra_args = "";
  if (strcmp(player, "mpv") == 0) {
    extra_args =
        "--demuxer-lavf-o=\"protocol_whitelist=[file,tcp,tls,http,https]\"";
  }

  char *cmd;
  asprintf(&cmd, "\"%s\" %s \"%s\"", player, extra_args, file);
  system(cmd);
}