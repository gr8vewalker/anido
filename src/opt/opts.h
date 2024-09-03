#ifndef ANIDO_OPTS_H
#define ANIDO_OPTS_H

extern int STREAM_FLAG;
extern int QUERY_FLAG;
extern char *PLAYER;
extern char *DOWNLOAD_FILE;
extern char *TEMP_FOLDER;

extern char *PROVIDER;
extern char *SEARCH;
extern long EPISODE;
extern char *SOURCE;

void parse_opts(int argc, char **argv);

#endif
