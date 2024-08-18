#ifndef ANIMEDOWN_OPTS_H
#define ANIMEDOWN_OPTS_H

extern int STREAM_FLAG;
extern int MAX_CURL_HANDLES;
extern char *PLAYER;

void parse_opts(int argc, char**argv);

#endif