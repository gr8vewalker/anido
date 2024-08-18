#ifndef ANIMEDOWN_FFMPEG_H
#define ANIMEDOWN_FFMPEG_H

void ffmpeg_concat(const char *concat_file, const char *merged_video);
void run_player(const char *player, const char *file);

#endif