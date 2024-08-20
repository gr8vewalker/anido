#ifndef ANIMEDOWN_LOG_H
#define ANIMEDOWN_LOG_H

#include <stdio.h>

#define ESCAPE_BOLD "\x1b[1m"
#define ESCAPE_RED "\x1b[31m"
#define ESCAPE_GREEN "\x1b[32m"
#define ESCAPE_YELLOW "\x1b[33m"
#define ESCAPE_BLUE "\x1b[34m"
#define ESCAPE_MAGENTA "\x1b[35m"
#define ESCAPE_CYAN "\x1b[36m"
#define ESCAPE_BRIGHT_GREEN "\x1b[92m"
#define ESCAPE_RESET "\x1b[0m"

#define ANIDO_PREFIX ESCAPE_CYAN "\u276f " ESCAPE_BOLD ESCAPE_BRIGHT_GREEN
#define ANIDO_DEBUG_PREFIX ESCAPE_YELLOW "DEBUG: "

#define CLEAR_LOG() printf("\x1b[1;1H\x1b[2J")

#define ANIDO_LOG(log) fputs(log, stdout)
#define ANIDO_LOGN(log) puts(log)
#define ANIDO_LOGP(log) ANIDO_LOG(ANIDO_PREFIX log ESCAPE_RESET)
#define ANIDO_LOGPN(log) ANIDO_LOGN(ANIDO_PREFIX log ESCAPE_RESET)

#define ANIDO_LOGF(...) printf(__VA_ARGS__)
#define ANIDO_LOGFN(fmt, ...) printf(fmt "\n", __VA_ARGS__)
#define ANIDO_LOGFP(fmt, ...) ANIDO_LOGF(ANIDO_PREFIX fmt ESCAPE_RESET, __VA_ARGS__)
#define ANIDO_LOGFPN(fmt, ...) ANIDO_LOGFN(ANIDO_PREFIX fmt ESCAPE_RESET, __VA_ARGS__)

#define ANIDO_ERR(err) fputs(err, stderr)
#define ANIDO_ERRN(err) ANIDO_ERR(err "\n")

#define ANIDO_ERRF(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
#define ANIDO_ERRFN(fmt, ...) ANIDO_ERRF(fmt "\n", __VA_ARGS__)

#ifdef DEBUG
#define ANIDO_LOG_DEBUG(log) puts(ANIDO_DEBUG_PREFIX log ESCAPE_RESET);
#define ANIDO_LOGF_DEBUG(fmt, ...) printf(ANIDO_DEBUG_PREFIX fmt ESCAPE_RESET "\n", __VA_ARGS__);
#else
#define ANIDO_LOG_DEBUG(log) (void)0
#define ANIDO_LOGF_DEBUG(fmt, ...) (void)0
#endif

#endif