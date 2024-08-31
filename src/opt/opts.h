#ifndef ANIDO_OPTS_H
#define ANIDO_OPTS_H

extern int STREAM_FLAG;
extern char *PLAYER;
extern char *DOWNLOAD_FILE;

extern char *PROVIDER;
extern char *SEARCH;
extern int EPISODE;
extern char *SOURCE;
extern char *QUALITY;

void parse_opts(int argc, char **argv);

#endif
