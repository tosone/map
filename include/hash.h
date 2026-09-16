#pragma once

#include <stddef.h>

/* algorithm ids, the values are agreed between this header and hash.c */
typedef enum {
  MAP_HASH_INVALID = -1,
  MAP_HASH_MD5 = 0,
  MAP_HASH_SHA1,
  MAP_HASH_SHA256,
  MAP_HASH_SHA512,
} map_hash_t;

/* hex length of the longest digest (sha512, 64 bytes) plus the trailing '\0' */
#define MAP_HASH_HEX_SIZE (64 * 2 + 1)

/*
 * Compute the digest of a file or of an in-memory buffer, backed by
 * deps/wjcryptlib (public domain).
 * Returns 0 and writes the lowercase hex digest into hex, or -1 on failure.
 */
int map_hash_file(map_hash_t algo, const char *path, char *hex, size_t hex_size);
int map_hash_data(map_hash_t algo, const unsigned char *data, size_t size, char *hex, size_t hex_size);
