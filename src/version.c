#include "version.h"

const char *anido_version() {
#ifdef ANIDO_VERSION
    return ANIDO_VERSION;
#else
    return "source";
#endif
}

const char *anido_build_date() { return __DATE__ " " __TIME__; }
