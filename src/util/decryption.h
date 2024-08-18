#ifndef ANIMEDOWN_BASE64_H
#define ANIMEDOWN_BASE64_H

#include <stdio.h>

int base64_decode(char *b64message, unsigned char **buffer, size_t *length);
unsigned char *decryptAES(char *ciphertext, char *salttext, char *password);

#endif