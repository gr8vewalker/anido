#ifndef LIBANIM_UTIL_H
#define LIBANIM_UTIL_H

#include <stddef.h>
#include <stdint.h>

// For mkdir, rmdir etc.
#include <sys/stat.h>
#include <unistd.h>

// For String Comparison
uint64_t murmur64(const char *key);

char *format_string(const char *fmt, ...);

int base64_decode(const char *encoded, unsigned char **buffer, size_t *length);

int hex_to_bin(const char *hex, unsigned char *buff, int length);

char *decrypt_aes(char *ct, char *salt, char *pass);

#endif
