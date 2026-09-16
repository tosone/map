#pragma once

#include <stddef.h>

/*
 * base64 编解码，二进制安全（按长度处理，不在 NUL 处截断）。
 * 返回值由 malloc 分配，调用方负责 free。
 * 编码失败（内存不足）返回 NULL；
 * 解码时输入长度不是 4 的倍数、含非 base64 字符或 '=' 位置不合法返回 NULL。
 */
char *map_base64_encode(const unsigned char *data, size_t size);
unsigned char *map_base64_decode(const char *data, size_t size, size_t *out_size);
