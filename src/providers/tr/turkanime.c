#include "turkanime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cjson/cJSON.h"

#include "extractors/extractors.h"
#include "net/connect.h"
#include "parsing/anime.h"
#include "parsing/document.h"
#include "util/decryption.h"
#include "util/log.h"

#define BASE_URL "https://www.turkanime.co"
#define ANIME_ENDPOINT "https://www.turkanime.co/anime"
#define SEARCH_ENDPOINT "https://www.turkanime.co/arama"

#define TURKANIME_AES_PASSWORD                                                 \
  "710^8A@3@>T2}#zN5xK?kR7KNKb@-A!LzYL5~M1qU0UfdWsZoBm4UUat%}ueUv6E--*"        \
  "hDPPbH7K2bp9^3o41hw,khL:}Kx8080@M"

void xpath_script_redirect(void *ptr, xmlNodeSetPtr nodeset);
void xpath_content(void *ptr, xmlNodeSetPtr nodeset);
void xpath_dataid(void *ptr, xmlNodeSetPtr nodeset);
void xpath_search_results(void *ptr, xmlNodeSetPtr nodeset);
void xpath_episodes(void *ptr, xmlNodeSetPtr nodeset);
void xpath_fansubs(void *ptr, xmlNodeSetPtr nodeset);
void xpath_decrypt_embed(void *ptr, xmlNodeSetPtr nodeset);

char *fix_decrypted_string(const char *orig);

int list_fansubs(const episode *episode, fansub_list *ptr);

int __search(const char *search_str, void **data_ptr) {
  if (strncmp(ANIME_ENDPOINT, search_str, strlen(ANIME_ENDPOINT)) == 0) {
    anime *ptr = malloc(sizeof(anime));

    if (!ptr)
      return -1;

    ptr->name = "";
    ptr->link = strdup(search_str);
    *((anime **)data_ptr) = ptr;
    return 1;
  }

  char *urlencoded;
  urlencode(search_str, &urlencoded);

  char *search_data;
  asprintf(&search_data, "arama=%s", urlencoded);

  document doc;
  if (fetch_document_url(SEARCH_ENDPOINT, &doc, NULL, 0, search_data) != 0) {
    free(urlencoded);
    free(search_data);
    return -1;
  }

  char *redirecting;
  exec_xpath(
      "//div[@class='panel-body']//script[contains(.,'window.location')]", &doc,
      xpath_script_redirect, &redirecting);

  free(urlencoded);
  free(search_data);

  if (redirecting) {
    anime *ptr = malloc(sizeof(anime));

    if (!ptr)
      return -1;

    ptr->name = "";
    asprintf(&ptr->link, BASE_URL "/%s", redirecting);
    *((anime **)data_ptr) = ptr;

    free(redirecting);
    free_document(&doc);
    return 1;
  }

  anime_list *list = malloc(sizeof(anime_list));

  if (!list)
    return -1;

  exec_xpath("//div[@class='col-md-6 col-sm-6 col-xs-12']", &doc,
             xpath_search_results, list);

  *((anime_list **)data_ptr) = list;

  free_document(&doc);
  return 0;
}

int __populate(anime *ptr) {
  document doc;
  if (fetch_document_url(ptr->link, &doc, NULL, 0, NULL) != 0)
    return 1;

  exec_xpath("//div[@id='detayPaylas']//div[@class='panel']//"
             "div[@class='panel-ust']//div[@class='panel-title']",
             &doc, xpath_content, &ptr->name);

  char *data_id;
  exec_xpath("//div[@id='animedetay']//div[@class='oylama']", &doc,
             xpath_dataid, &data_id);

  const char *header = "X-Requested-With: XMLHttpRequest";

  char *episodes_ajax_url;
  asprintf(&episodes_ajax_url,
           "https://www.turkanime.co/ajax/bolumler?animeId=%s", data_id);
  free(data_id);

  free_document(&doc);
  if (fetch_document_url(episodes_ajax_url, &doc, &header, 1, NULL) != 0) {
    free(episodes_ajax_url);
    return 1;
  }

  exec_xpath("//div[@id='bolumler']//li", &doc, xpath_episodes, ptr);
  free_document(&doc);

  free(episodes_ajax_url);
  return 0;
}

int __sourcext(const episode *episode, source_list **ptr) {
  source_list *list = malloc(sizeof(source_list));

  if (!list) {
    ANIDO_ERRN("Cannot allocate memory for source list");
    return -1;
  }

  list->len = 0;
  list->sources = malloc(0);

  if (!list->sources) {
    ANIDO_ERRN("Cannot allocate memory for sources ptr");
    return -1;
  }

  document doc;
  fansub_list fansubs;
  int i, j;
  const char *header = "X-Requested-With: XMLHttpRequest";

  if (list_fansubs(episode, &fansubs) != 0) {
    ANIDO_ERRN("An error occurred while gathering fansubs");
    return -1;
  }

  for (i = 0; i < fansubs.len; i++) {
    fansub fansub = fansubs.fansubs[i];
    for (j = 0; j < fansub.count; j++) {
      if (fetch_document_url(fansub.links[j], &doc, &header, 1, NULL) != 0)
        continue;

      char *player_name;
      exec_xpath("//div[@id='videodetay']//div[@class='video-icerik']/"
                 "following-sibling::div//button[@class='btn btn-sm "
                 "btn-danger']",
                 &doc, xpath_content, &player_name);

      extractor *extract_source = get_extractor(
          player_name + 1); // Content starts with a space skip it.

      if (!extract_source) {
        free(player_name);
        free_document(&doc);
        continue;
      }

      char *decrypted_embed_link;
      exec_xpath("//div[@id='videodetay']//div[@class='video-icerik']//iframe",
                 &doc, xpath_decrypt_embed, &decrypted_embed_link);

      char *fixed_link = fix_decrypted_string(decrypted_embed_link);

      char *source_name;
      asprintf(&source_name, "%s - %s", fansub.name + 1, player_name + 1);

      source_list *extracted_sources =
          extract_source->extract(fixed_link, source_name);

      free(source_name);
      free(fixed_link);
      free(decrypted_embed_link);
      free(extract_source);
      free(player_name);
      free_document(&doc);

      if (!extracted_sources)
        return -1; // if null its a error.

      if (extracted_sources->len > 0) {
        source *sptr =
            realloc(list->sources,
                    sizeof(source) * (list->len + extracted_sources->len));
        if (!sptr) {
          ANIDO_ERRN("Cannot realloc sources");
          return -1;
        }
        list->sources = sptr;
        for (int j = 0; j < extracted_sources->len; j++) {
          list->sources[list->len + j].link =
              strdup(extracted_sources->sources[j].link);
          list->sources[list->len + j].quality =
              strdup(extracted_sources->sources[j].quality);
          list->sources[list->len + j].name =
              strdup(extracted_sources->sources[j].name);
          list->sources[list->len + j].extractor =
              strdup(extracted_sources->sources[j].extractor);
        }
        list->len += extracted_sources->len;
      }

      parser_free_source_list(extracted_sources);
      free(extracted_sources);
    }
  }

  parser_free_fansub_list(&fansubs);
  *(ptr) = list;
  return 0;
}

int list_fansubs(const episode *episode, fansub_list *ptr) {
  const char *header = "X-Requested-With: XMLHttpRequest";
  document doc;

  if (fetch_document_url(episode->link, &doc, NULL, 0, NULL) != 0)
    return -1;

  exec_xpath("//div[@id='videodetay']//div[@class='pull-right']//button", &doc,
             xpath_fansubs, ptr);

  free_document(&doc);

  for (int i = 0; i < ptr->len; i++) {
    fansub *fansub = &ptr->fansubs[i];

    if (fetch_document_url(fansub->links[0], &doc, &header, 1, NULL) != 0)
      continue;
    fansub_list others;
    exec_xpath("//div[@id='videodetay']//div[@class='video-icerik']/"
               "following-sibling::div//button[position()>1]",
               &doc, xpath_fansubs, &others);

    char **lptr =
        realloc(fansub->links, sizeof(char *) * (fansub->count + others.len));
    if (!lptr) {
      ANIDO_ERRN("Cannot realloc fansub links");
      return -1;
    }

    fansub->links = lptr;
    for (int j = 0; j < others.len; j++) {
      fansub->links[fansub->count + j] = strdup(others.fansubs[j].links[0]);
    }
    fansub->count += others.len;

    parser_free_fansub_list(&others);
    free_document(&doc);
  }

  return 0;
}

void xpath_script_redirect(void *ptr, xmlNodeSetPtr nodeset) {
  char **cptr = (char **)ptr;
  if (nodeset->nodeNr < 1) {
    *cptr = NULL;
    return;
  }

  xmlNodePtr node = nodeset->nodeTab[0];
  if (node && node->type == XML_ELEMENT_NODE) {
    xmlChar *content = xmlNodeGetContent(node);

    char *windowlocation = "window.location = \"";
    char *start = (char *)content + strlen(windowlocation);
    char *end = strchr(start, '"');
    *cptr = malloc(sizeof(char) * (end - start));
    strncpy(*cptr, start, end - start);

    free(content);
  }
}

void xpath_content(void *ptr, xmlNodeSetPtr nodeset) {
  if (nodeset->nodeNr < 1)
    return;
  xmlNodePtr node = nodeset->nodeTab[0];
  if (node && node->type == XML_ELEMENT_NODE) {
    xmlChar *content = xmlNodeGetContent(node);
    *((char **)ptr) = (char *)content;
  }
}

void xpath_dataid(void *ptr, xmlNodeSetPtr nodeset) {
  if (nodeset->nodeNr < 1)
    return;
  xmlNodePtr node = nodeset->nodeTab[0];
  if (node && node->type == XML_ELEMENT_NODE) {
    xmlChar *data_id = xmlGetProp(node, (const xmlChar *)"data-id");
    *((char **)ptr) = (char *)data_id;
  }
}

void xpath_search_results(void *v_ptr, xmlNodeSetPtr nodeset) {
  anime_list *ptr = (anime_list *)v_ptr;

  ptr->len = nodeset->nodeNr;
  ptr->animes = malloc(ptr->len * sizeof(anime));

  for (int i = 0; i < nodeset->nodeNr; i++) {
    xmlNodePtr node = nodeset->nodeTab[i];
    xmlNode a = node->children[0].children[0].children[0].children[0];

    xmlChar *title = xmlGetProp(&a, (const xmlChar *)"title");
    title[strlen((char *)title) - 5] = 0; // remove last 5 characters because
                                          // TurkAnime adds " izle" at end

    xmlChar *href = xmlGetProp(&a, (const xmlChar *)"href");

    char *link;
    asprintf(&link, "https:%s", href);

    free(href);

    ptr->animes[i].name = (char *)title;
    ptr->animes[i].link = link;
  }
}

void xpath_episodes(void *ptr, xmlNodeSetPtr nodeset) {
  anime *anime_ptr = ptr;
  anime_ptr->episodes_len = nodeset->nodeNr;
  anime_ptr->episodes = malloc(anime_ptr->episodes_len * sizeof(episode));

  for (int i = 0; i < nodeset->nodeNr; i++) {
    xmlNodePtr node = nodeset->nodeTab[i];
    xmlNodePtr a = node->children[0].next;
    xmlNode span = a->children[0];

    xmlChar *name = xmlNodeGetContent(&span);
    xmlChar *href = xmlGetProp(a, (const xmlChar *)"href");

    char *link;
    asprintf(&link, "https:%s", href);

    anime_ptr->episodes[i].name = (char *)name;
    anime_ptr->episodes[i].link = link;

    free(href);
  }
}

void xpath_fansubs(void *ptr, xmlNodeSetPtr nodeset) {
  fansub_list *fansubs_ptr = ptr;
  fansubs_ptr->len = nodeset->nodeNr;
  fansubs_ptr->fansubs = malloc(fansubs_ptr->len * sizeof(fansub));

  for (int i = 0; i < nodeset->nodeNr; i++) {
    xmlNodePtr node = nodeset->nodeTab[i];

    xmlChar *content = xmlNodeGetContent(node);
    xmlChar *onclick = xmlGetProp(node, (const xmlChar *)"onclick");

    char *start = "IndexIcerik('";
    char *end = "','videodetay'); return false;";

    char *link;
    asprintf(&link, "https://www.turkanime.co/%s",
             (char *)onclick + strlen(start));
    link[strlen(link) - strlen(end)] = 0;

    fansubs_ptr->fansubs[i].name = (char *)content;
    fansubs_ptr->fansubs[i].count = 1;
    fansubs_ptr->fansubs[i].links = malloc(sizeof(char *));
    fansubs_ptr->fansubs[i].links[0] = link;
    free(onclick);
  }
}

void xpath_decrypt_embed(void *ptr, xmlNodeSetPtr nodeset) {
  if (nodeset->nodeNr < 1)
    return;

  xmlNodePtr node = nodeset->nodeTab[0];
  if (node && node->type == XML_ELEMENT_NODE) {
    xmlChar *src = xmlGetProp(node, (const xmlChar *)"src");

    char *encoded_section =
        strdup((char *)src + strlen("//www.turkanime.co/embed/#/url/"));
    encoded_section[strlen(encoded_section) - strlen("?status=0")] = 0;

    unsigned char *cipher_json;
    size_t cipher_json_len;
    base64_decode(encoded_section, &cipher_json, &cipher_json_len);

    cJSON *json = cJSON_ParseWithLength((char *)cipher_json, cipher_json_len);
    char *ciphertext = cJSON_GetObjectItem(json, "ct")->valuestring;
    char *salt = cJSON_GetObjectItem(json, "s")->valuestring;

    *((char **)ptr) =
        (char *)decryptAES(ciphertext, salt, TURKANIME_AES_PASSWORD);

    free(src);
    free(encoded_section);
    free(cipher_json);
    cJSON_Delete(json);
  }
}

char *fix_decrypted_string(const char *orig) {
  char *cleared =
      malloc(sizeof(char) *
             (strlen(orig) +
              strlen("https:"))); // make sure there is enough room for https:
  char *cleared_inc = stpcpy(cleared, "https:");

  const char *last =
      orig + 2; // first two character is a quote and a backslash so skip them
  const char *ptr;
  while ((ptr = strchr(last, '\\')) != NULL) {
    cleared_inc = stpncpy(cleared_inc, last, ptr - last);
    last = ptr + 1;
  }
  ptr = strchr(last, '"'); // get the last part.
  cleared_inc = stpncpy(cleared_inc, last, ptr - last);
  cleared_inc[0] = 0; // end the string

  return cleared;
}

provider *TurkAnime() {
  provider *turkanime = malloc(sizeof(provider));
  turkanime->name = "TurkAnime";
  turkanime->search = __search;
  turkanime->populate = __populate;
  turkanime->sourcext = __sourcext;
  return turkanime;
}