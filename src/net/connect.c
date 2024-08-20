#include "connect.h"
#include "../util/log.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *string;
  size_t size;
} curl_string;

static size_t write_string(void *contents, size_t size, size_t nmemb,
                           void *userp) {
  size_t realsize = size * nmemb;
  curl_string *str = (curl_string *)userp;

  char *ptr = realloc(str->string, str->size + realsize + 1);
  if (!ptr) {
    ANIDO_ERRN("not enough memory (realloc returned NULL)");
    return 0;
  }

  str->string = ptr;
  memcpy(&(str->string[str->size]), contents, realsize);
  str->size += realsize;
  str->string[str->size] = 0;

  return realsize;
}

static size_t write_file(void *ptr, size_t size, size_t nmemb, void *stream) {
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

CURLcode fetch(const char *url, const char **headers, int headers_size,
               char **response) {
  CURL *curl;
  CURLcode res;

  curl_string response_holder;
  response_holder.string = malloc(1);
  response_holder.size = 0;

  curl = curl_easy_init();
  if (curl) {
    struct curl_slist *chunk = NULL;
    for (int i = 0; i < headers_size; i++) {
      chunk = curl_slist_append(chunk, headers[i]);
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_string);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_holder);
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    res = curl_easy_perform(curl);

    curl_slist_free_all(chunk);
    curl_easy_cleanup(curl);
  }

  *response = response_holder.string;
  return res;
}

CURLcode post(const char *url, const char **headers, int headers_size,
              const char *request, size_t request_len, char **response) {
  CURL *curl;
  CURLcode res;

  curl_string response_holder;
  response_holder.string = malloc(1);
  response_holder.size = 0;

  curl = curl_easy_init();
  if (curl) {
    struct curl_slist *chunk = NULL;
    for (int i = 0; i < headers_size; i++) {
      chunk = curl_slist_append(chunk, headers[i]);
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request_len);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_string);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_holder);
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    res = curl_easy_perform(curl);

    curl_slist_free_all(chunk);
    curl_easy_cleanup(curl);
  }

  *response = response_holder.string;
  return res;
}

downloader_session *create_download_session(int max_handles) {
  downloader_session *ptr = malloc(sizeof(downloader_session));
  ptr->handles = malloc(sizeof(CURL *) * max_handles);
  ptr->files = malloc(sizeof(FILE *) * max_handles);
  ptr->handle_count = 0;
  return ptr;
}

void add_to_session(downloader_session *session, const char *url,
                    const char *path) {
  session->handles[session->handle_count] = curl_easy_init();
  CURL *curl = session->handles[session->handle_count];
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);
  session->files[session->handle_count] = fopen(path, "wb");
  curl_easy_setopt(curl, CURLOPT_WRITEDATA,
                   session->files[session->handle_count]);
  session->handle_count++;
}

// TODO: Add a way to handle errors, at least print them.
void perform_session(downloader_session *session) {
  session->multi_handle = curl_multi_init();
  for (int i = 0; i < session->handle_count; i++)
    curl_multi_add_handle(session->multi_handle, session->handles[i]);

  int still_running = 1;
  while (still_running) {
    CURLMcode mc = curl_multi_perform(session->multi_handle, &still_running);

    if (still_running)
      mc = curl_multi_poll(session->multi_handle, NULL, 0, 1000, NULL);

    if (mc)
      break;
  }

  for (int i = 0; i < session->handle_count; i++) {
    curl_multi_remove_handle(session->multi_handle, session->handles[i]);
    curl_easy_cleanup(session->handles[i]);
    fclose(session->files[i]);
  }

  curl_multi_cleanup(session->multi_handle);
  session->handle_count = 0;
}

void end_session(downloader_session *session) {
  perform_session(session); // Finish if any left
  free(session->handles);
  free(session->files);
  free(session);
}

void urlencode(const char *data, char **encoded) {
  CURL *curl;

  curl = curl_easy_init();
  if (curl) {
    *encoded = curl_easy_escape(curl, data, strlen(data));
    curl_easy_cleanup(curl);
  }
}