#include "document.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libxml/xpath.h"

#include "net/connect.h"
#include "util/log.h"

void fetch_document(const char *contents, document *doc) {
  doc->htmldoc = htmlReadMemory(contents, strlen(contents), NULL, NULL,
                                HTML_PARSE_RECOVER | HTML_PARSE_NOERROR |
                                    HTML_PARSE_NOWARNING);

  if (doc->htmldoc == NULL) {
    ANIDO_ERRN("Failed to parse HTML");
    return;
  }

  doc->xpath_ctx = xmlXPathNewContext(doc->htmldoc);
}

int fetch_document_url(const char *url, document *doc, const char **headers,
                       int headers_size, const char *post_data) {
  char *response;

  int curlcode;
  if (post_data) {
    curlcode = post(url, headers, headers_size, post_data, strlen(post_data),
                    &response);
  } else {
    curlcode = fetch(url, headers, headers_size, &response);
  }

  if (curlcode != 0) {
    ANIDO_ERRFN(
        "An error occurred while fetching document. Curl error code: %i",
        curlcode);
    return 1;
  }

  fetch_document(response, doc);

  free(response);
  return 0;
}

void exec_xpath(const char *xpath, document *doc,
                void (*callback)(void *data, const xmlNodeSetPtr nodeset),
                void *data) {
  xmlXPathObjectPtr xpath_obj =
      xmlXPathEvalExpression((const xmlChar *)xpath, doc->xpath_ctx);
  if (xpath_obj == NULL) {
    ANIDO_ERRFN("Error: unable to evaluate XPath expression \"%s\"",
            xpath);
    return;
  }

  xmlNodeSetPtr nodeset = xpath_obj->nodesetval;

  if (nodeset == NULL) {
    ANIDO_LOGF_DEBUG("Null nodeset value for XPath expression \"%s\"", xpath);
  } else {
    callback(data, nodeset);
  }

  xmlXPathFreeObject(xpath_obj);
}

void free_document(document *doc) {
  xmlXPathFreeContext(doc->xpath_ctx);
  xmlFreeDoc(doc->htmldoc);
}