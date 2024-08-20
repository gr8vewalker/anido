#ifndef ANIMEDOWN_CONNECT_H
#define ANIMEDOWN_CONNECT_H

#include <curl/curl.h>

typedef struct {
  CURLM *multi_handle;
  int handle_count;
  CURL **handles;
  FILE **files;
} downloader_session;

CURLcode fetch(const char *url, const char **headers, int headers_size,
           char **response);
CURLcode post(const char *url, const char **headers, int headers_size,
          const char *request, size_t request_len, char **response);
downloader_session *create_download_session(int max_handles);
void add_to_session(downloader_session *session, const char *url,
                    const char *path);
void perform_session(downloader_session *session);
void end_session(downloader_session *session);
void urlencode(const char *data, char **encoded);

#endif