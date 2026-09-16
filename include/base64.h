#pragma once

#include <stddef.h>

/*
 * base64 encode/decode, binary safe (works on lengths and never stops at NUL).
 * The returned buffer is allocated with malloc and owned by the caller.
 * Encoding returns NULL on failure (out of memory); decoding returns NULL when
 * the input length is not a multiple of 4, when it holds a non base64 character,
 * or when '=' appears in an invalid position.
 */
char *map_base64_encode(const unsigned char *data, size_t size);
unsigned char *map_base64_decode(const char *data, size_t size, size_t *out_size);
