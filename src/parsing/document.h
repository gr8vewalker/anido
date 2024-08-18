#ifndef ANIMEDOWN_DOCUMENT_H
#define ANIMEDOWN_DOCUMENT_H

#include "libxml/HTMLparser.h"
#include "libxml/xpath.h"

typedef struct {
  htmlDocPtr htmldoc;
  xmlXPathContextPtr xpath_ctx;
} document;

void fetch_document(const char *contents, document *doc);
void fetch_document_url(const char *url, document *doc, const char **headers,
                        int headers_size, const char *post_data);
void exec_xpath(const char *xpath, document *doc,
                void (*callback)(void *data, const xmlNodeSetPtr nodeset),
                void *data);
void free_document(document *doc);

#endif