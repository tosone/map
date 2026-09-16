#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 算法标识，值和顺序只在本文件与 hash.cpp 之间约定 */
typedef enum {
  MAP_HASH_INVALID = -1,
  MAP_HASH_MD5 = 0,
  MAP_HASH_SHA1,
  MAP_HASH_SHA256,
  MAP_HASH_SHA512,
} map_hash_t;

/* 最长摘要（sha512 64 字节）的十六进制长度加上结尾 '\0' */
#define MAP_HASH_HEX_SIZE (64 * 2 + 1)

/*
 * hashlib 是 C++11 的 header only 库，这里用一层 extern "C" 包住，
 * 让 C 代码可以用它计算文件或内存中的摘要。
 * 成功返回 0 并把小写十六进制摘要写入 hex；失败返回 -1。
 */
int map_hash_file(map_hash_t algo, const char *path, char *hex, size_t hex_size);
int map_hash_data(map_hash_t algo, const unsigned char *data, size_t size, char *hex, size_t hex_size);

#ifdef __cplusplus
}
#endif
