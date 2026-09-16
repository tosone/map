#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * include/base64.hpp 是 C++ 的 header only 实现，这里用一层 extern "C" 包住，
 * 让 C 代码可以用它编解码内存中的任意二进制数据。
 * 返回值由 malloc 分配，调用方负责 free。
 * 编码失败（内存不足）返回 NULL；解码时输入长度不是 4 的倍数或含非 base64 字符返回 NULL。
 */
char *map_base64_encode(const unsigned char *data, size_t size);
unsigned char *map_base64_decode(const char *data, size_t size, size_t *out_size);

#ifdef __cplusplus
}
#endif
