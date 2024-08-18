#include "decryption.h"

#include <assert.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// BASE 64 DECODING.
// Taken from https://gist.github.com/barrysteyn/7308212

size_t calc_decode_len(const char *b64input) {
  size_t len = strlen(b64input), padding = 0;

  if (b64input[len - 1] == '=' && b64input[len - 2] == '=')
    padding = 2;
  else if (b64input[len - 1] == '=')
    padding = 1;

  return (len * 3) / 4 - padding;
}

int base64_decode(char *b64message, unsigned char **buffer, size_t *length) {
  BIO *bio, *b64;

  int decodeLen = calc_decode_len(b64message);
  *buffer = (unsigned char *)malloc(decodeLen + 1);
  (*buffer)[decodeLen] = '\0';

  bio = BIO_new_mem_buf(b64message, -1);
  b64 = BIO_new(BIO_f_base64());
  bio = BIO_push(b64, bio);

  BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
  *length = BIO_read(bio, *buffer, strlen(b64message));
  assert(*length == decodeLen);
  BIO_free_all(bio);

  return 0;
}

// HEX DECODING
// Taken from https://stackoverflow.com/a/41841815

unsigned char hex_char(char c) {
  if ('0' <= c && c <= '9')
    return (unsigned char)(c - '0');
  if ('A' <= c && c <= 'F')
    return (unsigned char)(c - 'A' + 10);
  if ('a' <= c && c <= 'f')
    return (unsigned char)(c - 'a' + 10);
  return 0xFF;
}

int hex_to_bin(const char *s, unsigned char *buff, int length) {
  int result;
  if (!s || !buff || length <= 0)
    return -1;

  for (result = 0; *s; ++result) {
    unsigned char msn = hex_char(*s++);
    if (msn == 0xFF)
      return -1;
    unsigned char lsn = hex_char(*s++);
    if (lsn == 0xFF)
      return -1;
    unsigned char bin = (msn << 4) + lsn;

    if (length-- <= 0)
      return -1;
    *buff++ = bin;
  }
  return result;
}

// AES decryption

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext) {
  EVP_CIPHER_CTX *ctx;

  int len;

  int plaintext_len;

  if (!(ctx = EVP_CIPHER_CTX_new()))
    return -1;

  if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    return -1;

  if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
    return -1;
  plaintext_len = len;

  if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
    return -1;
  plaintext_len += len;

  EVP_CIPHER_CTX_free(ctx);

  return plaintext_len;
}

int key_and_iv(char *salttext, unsigned char *key, unsigned char *iv,
               char *password) {
  unsigned char hexsalt[8];
  hex_to_bin(salttext, hexsalt, strlen(salttext));

  return EVP_BytesToKey(EVP_aes_256_cbc(), EVP_md5(), hexsalt,
                        (unsigned char *)password, strlen(password), 1, key,
                        iv);
}

unsigned char *decryptAES(char *ciphertext, char *salttext, char *password) {
  unsigned char *ciphertext_decoded;
  size_t ciphertext_len;
  base64_decode(ciphertext, &ciphertext_decoded, &ciphertext_len);

  unsigned char key[EVP_MAX_KEY_LENGTH], iv[EVP_MAX_IV_LENGTH];
  if (!key_and_iv(salttext, key, iv, password)) {
    fprintf(stderr, "Cannot derive key and iv\n");
    return NULL;
  }

  unsigned char *plain = malloc(sizeof(unsigned char) * 512);
  int len = decrypt(ciphertext_decoded, ciphertext_len, key, iv, plain);
  plain[len] = 0;

  free(ciphertext_decoded);
  return plain;
}