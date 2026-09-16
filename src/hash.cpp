#include <cstdio>

#include <hashlib/md5.hpp>
#include <hashlib/sha1.hpp>
#include <hashlib/sha2.hpp>

#include <hash.h>

namespace {

constexpr size_t buffer_size = 16384;
constexpr char hex_table[] = "0123456789abcdef";

template <typename H>
int digest_to_hex(H &h, char *hex, size_t hex_size) {
  if (hex_size < H::digest_size * 2 + 1) {
    return -1;
  }
  size_t index = 0;
  for (auto unit : h.digest()) {
    hex[index++] = hex_table[unit >> 4];
    hex[index++] = hex_table[unit & 0x0f];
  }
  hex[index] = '\0';
  return 0;
}

template <typename H>
int hash_file(const char *path, char *hex, size_t hex_size) {
  std::FILE *file = std::fopen(path, "rb");
  if (file == NULL) {
    return -1;
  }
  H h;
  unsigned char buffer[buffer_size];
  size_t size;
  while ((size = std::fread(buffer, 1, sizeof(buffer), file)) > 0) {
    h.update(hashlib::span<const hashlib::byte>(buffer, size));
  }
  bool failed = std::ferror(file) != 0;
  std::fclose(file);
  if (failed) {
    return -1;
  }
  return digest_to_hex(h, hex, hex_size);
}

template <typename H>
int hash_data(const unsigned char *data, size_t size, char *hex, size_t hex_size) {
  H h;
  h.update(hashlib::span<const hashlib::byte>(data, size));
  return digest_to_hex(h, hex, hex_size);
}

} // namespace

extern "C" int map_hash_file(map_hash_t algo, const char *path, char *hex, size_t hex_size) {
  switch (algo) {
  case MAP_HASH_MD5:
    return hash_file<hashlib::md5>(path, hex, hex_size);
  case MAP_HASH_SHA1:
    return hash_file<hashlib::sha1>(path, hex, hex_size);
  case MAP_HASH_SHA256:
    return hash_file<hashlib::sha256>(path, hex, hex_size);
  case MAP_HASH_SHA512:
    return hash_file<hashlib::sha512>(path, hex, hex_size);
  default:
    return -1;
  }
}

extern "C" int map_hash_data(map_hash_t algo, const unsigned char *data, size_t size, char *hex, size_t hex_size) {
  switch (algo) {
  case MAP_HASH_MD5:
    return hash_data<hashlib::md5>(data, size, hex, hex_size);
  case MAP_HASH_SHA1:
    return hash_data<hashlib::sha1>(data, size, hex, hex_size);
  case MAP_HASH_SHA256:
    return hash_data<hashlib::sha256>(data, size, hex, hex_size);
  case MAP_HASH_SHA512:
    return hash_data<hashlib::sha512>(data, size, hex, hex_size);
  default:
    return -1;
  }
}
