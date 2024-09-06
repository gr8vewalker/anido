#ifndef ANIDO_OPTS_H
#define ANIDO_OPTS_H

extern char *DOWNLOAD_FILE;
extern char *TEMP_FOLDER;

extern char *PROVIDER;
extern char *SEARCH;
extern long EPISODE;
extern char *SOURCE;

void parse_opts(int argc, char **argv);

#endif
