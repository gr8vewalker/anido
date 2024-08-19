#ifndef ANIMEDOWN_VERSION_H
#define ANIMEDOWN_VERSION_H

const char *anido_version() {
#ifdef ANIDO_VERSION
  return ANIDO_VERSION;
#else
  return "source";
#endif
}

const char *build_date() {
    return __DATE__ " " __TIME__;
}

#endif