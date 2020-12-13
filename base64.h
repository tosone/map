#include <ctype.h>

#pragma once

#define BASE64_COMMAND "base64"
#define BASE64_COMMAND_DECODE "dec"
#define BASE64_COMMAND_ENCODE "enc"

unsigned char *base64_encode(const unsigned char *src, size_t len, size_t *out_len);
unsigned char *base64_decode(const unsigned char *src, size_t len, size_t *out_len);
