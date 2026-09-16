#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>

#include <base64.h>
#include <base64.hpp>

namespace {

constexpr std::string_view alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

bool is_valid_base64(std::string_view data) {
  if (data.size() % 4 != 0) {
    return false;
  }
  for (char c : data) {
    if (alphabet.find(c) == std::string_view::npos) {
      return false;
    }
  }
  return true;
}

void *duplicate(const void *data, size_t size) {
  void *out = std::malloc(size + 1);
  if (out == NULL) {
    return NULL;
  }
  std::memcpy(out, data, size);
  static_cast<unsigned char *>(out)[size] = '\0';
  return out;
}

} // namespace

extern "C" char *map_base64_encode(const unsigned char *data, size_t size) {
  /* to_base64 在收尾不足 3 字节时会多读 1 字节，std::string 保证 data()[size()] 是 '\0'，
     所以这里先拷进 std::string，调用方不需要保证缓冲区尾部可读 */
  std::string input(reinterpret_cast<const char *>(data), size);
  std::string encoded = to_base64(std::string_view(input));
  return static_cast<char *>(duplicate(encoded.data(), encoded.size()));
}

extern "C" unsigned char *map_base64_decode(const char *data, size_t size, size_t *out_size) {
  std::string_view input(data, size);
  if (!is_valid_base64(input)) {
    return NULL;
  }
  std::string decoded = from_base64(input);
  unsigned char *out = static_cast<unsigned char *>(duplicate(decoded.data(), decoded.size()));
  if (out != NULL) {
    *out_size = decoded.size();
  }
  return out;
}
