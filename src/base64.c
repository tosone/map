#include <base64.h>

#include <stdlib.h>

static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* base64 字符返回 0~63，其它返回 -1 */
static int base64_value(unsigned char c) {
  if (c >= 'A' && c <= 'Z') {
    return c - 'A';
  }
  if (c >= 'a' && c <= 'z') {
    return c - 'a' + 26;
  }
  if (c >= '0' && c <= '9') {
    return c - '0' + 52;
  }
  if (c == '+') {
    return 62;
  }
  if (c == '/') {
    return 63;
  }
  return -1;
}

char *map_base64_encode(const unsigned char *data, size_t size) {
  size_t out_size = (size + 2) / 3 * 4; /* 每 3 字节一组，凑不满的补 '=' */
  char *out = (char *)malloc(out_size + 1);
  if (out == NULL) {
    return NULL;
  }

  size_t i = 0;
  size_t j = 0;
  for (; i + 3 <= size; i += 3) {
    out[j++] = alphabet[data[i] >> 2];
    out[j++] = alphabet[((data[i] & 0x03) << 4) | (data[i + 1] >> 4)];
    out[j++] = alphabet[((data[i + 1] & 0x0f) << 2) | (data[i + 2] >> 6)];
    out[j++] = alphabet[data[i + 2] & 0x3f];
  }

  if (i + 1 == size) { /* 剩 1 字节，补 2 个 '=' */
    out[j++] = alphabet[data[i] >> 2];
    out[j++] = alphabet[(data[i] & 0x03) << 4];
  } else if (i + 2 == size) { /* 剩 2 字节，补 1 个 '=' */
    out[j++] = alphabet[data[i] >> 2];
    out[j++] = alphabet[((data[i] & 0x03) << 4) | (data[i + 1] >> 4)];
    out[j++] = alphabet[(data[i + 1] & 0x0f) << 2];
  }

  while (j < out_size) {
    out[j++] = '=';
  }
  out[j] = '\0';
  return out;
}

unsigned char *map_base64_decode(const char *data, size_t size, size_t *out_size) {
  if (size % 4 != 0) {
    return NULL;
  }

  size_t padding = 0;
  if (size > 0 && data[size - 1] == '=') {
    padding = data[size - 2] == '=' ? 2 : 1;
  }

  size_t decoded_size = size / 4 * 3 - padding;
  unsigned char *out = (unsigned char *)malloc(decoded_size + 1);
  if (out == NULL) {
    return NULL;
  }

  size_t j = 0;
  for (size_t i = 0; i < size; i += 4) {
    int last = i + 4 == size;
    unsigned char c2 = (unsigned char)data[i + 2];
    unsigned char c3 = (unsigned char)data[i + 3];
    int v0 = base64_value((unsigned char)data[i]);
    int v1 = base64_value((unsigned char)data[i + 1]);
    int v2 = last && c2 == '=' ? 0 : base64_value(c2);
    int v3 = last && c3 == '=' ? 0 : base64_value(c3);
    /* '=' 只能出现在最后 1~2 位，且补两位时必须连着补 */
    if (v0 < 0 || v1 < 0 || v2 < 0 || v3 < 0 || (c2 == '=' && c3 != '=')) {
      free(out);
      return NULL;
    }

    out[j++] = (unsigned char)((v0 << 2) | (v1 >> 4));
    if (j < decoded_size) {
      out[j++] = (unsigned char)(((v1 & 0x0f) << 4) | (v2 >> 2));
    }
    if (j < decoded_size) {
      out[j++] = (unsigned char)(((v2 & 0x03) << 6) | v3);
    }
  }
  out[j] = '\0';
  *out_size = j;
  return out;
}
