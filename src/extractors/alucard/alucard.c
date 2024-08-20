#include "alucard.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cjson/cJSON.h"
#include "net/connect.h"
#include "opt/opts.h"
#include "util/log.h"
#include "util/video.h"

source_list *__extract(const char *link, const char *source_name) {
  char *sources_link;
  int start_len = strlen("https://www.turkanime.co/player/");
  asprintf(&sources_link, "https://www.turkanime.co/sources/%s/true",
           link + start_len);

  const char *source_headers[3] = {
      "X-Requested-With: XMLHttpRequest",
      "Csrf-Token: "
      "EqdGHqwZJvydjfbmuYsZeGvBxDxnQXeARRqUNbhRYnPEWqdDnYFEK"
      "VBaUPCAGTZA", // deobfuscated from turkanimes js
      "Cookie: __"};

  char *sources_json;
  fetch(sources_link, source_headers, 3, &sources_json);

  cJSON *json = cJSON_ParseWithLength(sources_json, strlen(sources_json));
  cJSON *json_response = cJSON_GetObjectItem(json, "response");
  cJSON *json_sources = cJSON_GetObjectItem(json_response, "sources");
  cJSON *json_source = cJSON_GetArrayItem(json_sources, 0);
  char *source_playlist_link =
      cJSON_GetObjectItem(json_source, "file")->valuestring;

  char *m3u8_data;
  fetch(source_playlist_link, NULL, 0, &m3u8_data);

  cJSON_Delete(json);
  free(sources_json);
  free(sources_link);

  char *line = strtok(m3u8_data, "\r\n");
  char *prefix = "#EXT-X-STREAM-INF:BANDWIDTH=";
  size_t prefix_len = strlen(prefix);

  source_list *sources = malloc(sizeof(source_list));
  sources->len = 0;
  sources->sources = malloc(0);

  int was_match = 0;
  while (line != NULL) {
    if (was_match) {
      was_match = 0;
      sources->sources[sources->len++].link = strdup(line);
    } else if (strncmp(line, prefix, prefix_len) == 0) {
      was_match = 1;
      source *ptr =
          realloc(sources->sources, (sources->len + 1) * sizeof(source));
      if (!ptr) {
        ANIDO_ERRN("Cannot realloc sources");
        return NULL;
      }
      sources->sources = ptr;
      char *resolution_keyword = "RESOLUTION=";
      char *resolution_equal_sign =
          strstr(line + prefix_len, resolution_keyword) +
          strlen(resolution_keyword);
      char *comma = strchr(resolution_equal_sign, ',');
      if (comma)
        comma[0] =
            0; // if resolution is not at the end of string, null terminate
      sources->sources[sources->len].quality = strdup(resolution_equal_sign);
      sources->sources[sources->len].name = strdup(source_name);
      sources->sources[sources->len].extractor = strdup("ALUCARD(BETA)");
    }
    line = strtok(NULL, "\r\n");
  }

  free(m3u8_data);

  return sources;
}

char *__stream(const source *source) {
  downloader_session *sess = create_download_session(1);
  char *path;
  asprintf(&path, "animetmp/%s%s.m3u8", source->name, source->quality);
  add_to_session(sess, source->link, path);
  end_session(sess);

  return path;
}

char *__download(const source *source, const char *name) {
  char *m3u8;
  fetch(source->link, NULL, 0, &m3u8);

  FILE *concatfile = fopen("animetmp/concat.txt", "wb");

  char *line = strtok(m3u8, "\r\n");
  char *prefix = "#EXTINF:";
  size_t prefix_len = strlen(prefix);

  downloader_session *session = create_download_session(MAX_CURL_HANDLES);

  int x = 0;
  int was_match = 0;
  while (line != NULL) {
    if (was_match) {
      was_match = 0;
      char *filename;
      int index = x - 1;
      asprintf(&filename, "animetmp/%u.mp4", index);
      fprintf(concatfile, "file '%u.mp4'\n", index);
      add_to_session(session, line, filename);
      if (session->handle_count == MAX_CURL_HANDLES) {
        ANIDO_LOGFN(ESCAPE_RED "[Alucard] " ESCAPE_RESET
                               "Downloading parts... " ESCAPE_YELLOW
                               "[%u]" ESCAPE_RESET,
                    x);
        perform_session(session);
      }
    } else if (strncmp(line, prefix, prefix_len) == 0) {
      x++;
      was_match = 1;
    }
    line = strtok(NULL, "\r\n");
  }

  end_session(session);
  fclose(concatfile);

  char *filename;
  asprintf(&filename, "%s %s.mp4", name, source->name);

  ffmpeg_concat("animetmp/concat.txt", filename);

  return filename;
}

extractor *Alucard_Extractor() {
  extractor *alucard = malloc(sizeof(extractor));
  alucard->name = "ALUCARD(BETA)";
  alucard->extract = __extract;
  alucard->stream = __stream;
  alucard->download = __download;
  return alucard;
}