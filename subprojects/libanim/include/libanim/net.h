#ifndef LIBANIM_NET_H
#define LIBANIM_NET_H

// Connection Functions
// Wrapper around libcurl.

#include <stddef.h>

int net_init();
void net_cleanup();

int get(const char *url, const char **headers, int headers_size,
        char **response);
int post(const char *url, const char **headers, int headers_size,
         const char *request, size_t request_len, char **response);

int downloadfile(const char *url, const char **headers, int headers_size,
                 const char *path);

typedef struct threaded_download_info {
    char *url;
    const char **headers;
    int headers_size;
    char *path;
} threaded_download_info;
void *downloadfile_t(void *ptr);

// Utility
int urlenc(const char *data, char **encoded);

#endif
