#include <hash.h>

#include <stdint.h>
#include <stdio.h>

#include <WjCryptLib_Md5.h>
#include <WjCryptLib_Sha1.h>
#include <WjCryptLib_Sha256.h>
#include <WjCryptLib_Sha512.h>

#define BUFFER_SIZE 16384

static size_t digest_size_of(map_hash_t algo) {
  switch (algo) {
  case MAP_HASH_MD5:
    return MD5_HASH_SIZE;
  case MAP_HASH_SHA1:
    return SHA1_HASH_SIZE;
  case MAP_HASH_SHA256:
    return SHA256_HASH_SIZE;
  case MAP_HASH_SHA512:
    return SHA512_HASH_SIZE;
  default:
    return 0;
  }
}

/* the caller has already validated the hex buffer size through digest_size_of() */
static void digest_to_hex(const uint8_t *digest, size_t size, char *hex) {
  static const char hex_table[] = "0123456789abcdef";
  for (size_t i = 0; i < size; i++) {
    hex[i * 2] = hex_table[digest[i] >> 4];
    hex[i * 2 + 1] = hex_table[digest[i] & 0x0f];
  }
  hex[size * 2] = '\0';
}

/* all four algorithms are initialise -> update in blocks -> finalise, files are read in 16KB blocks */

static int md5_file(FILE *file, char *hex) {
  Md5Context context;
  Md5Initialise(&context);
  uint8_t buffer[BUFFER_SIZE];
  size_t size;
  while ((size = fread(buffer, 1, sizeof(buffer), file)) > 0) {
    Md5Update(&context, buffer, (uint32_t)size);
  }
  if (ferror(file)) {
    return -1;
  }
  MD5_HASH digest;
  Md5Finalise(&context, &digest);
  digest_to_hex(digest.bytes, sizeof(digest.bytes), hex);
  return 0;
}

static int sha1_file(FILE *file, char *hex) {
  Sha1Context context;
  Sha1Initialise(&context);
  uint8_t buffer[BUFFER_SIZE];
  size_t size;
  while ((size = fread(buffer, 1, sizeof(buffer), file)) > 0) {
    Sha1Update(&context, buffer, (uint32_t)size);
  }
  if (ferror(file)) {
    return -1;
  }
  SHA1_HASH digest;
  Sha1Finalise(&context, &digest);
  digest_to_hex(digest.bytes, sizeof(digest.bytes), hex);
  return 0;
}

static int sha256_file(FILE *file, char *hex) {
  Sha256Context context;
  Sha256Initialise(&context);
  uint8_t buffer[BUFFER_SIZE];
  size_t size;
  while ((size = fread(buffer, 1, sizeof(buffer), file)) > 0) {
    Sha256Update(&context, buffer, (uint32_t)size);
  }
  if (ferror(file)) {
    return -1;
  }
  SHA256_HASH digest;
  Sha256Finalise(&context, &digest);
  digest_to_hex(digest.bytes, sizeof(digest.bytes), hex);
  return 0;
}

static int sha512_file(FILE *file, char *hex) {
  Sha512Context context;
  Sha512Initialise(&context);
  uint8_t buffer[BUFFER_SIZE];
  size_t size;
  while ((size = fread(buffer, 1, sizeof(buffer), file)) > 0) {
    Sha512Update(&context, buffer, (uint32_t)size);
  }
  if (ferror(file)) {
    return -1;
  }
  SHA512_HASH digest;
  Sha512Finalise(&context, &digest);
  digest_to_hex(digest.bytes, sizeof(digest.bytes), hex);
  return 0;
}

/* in-memory data is hashed in one call; sizes come from the command line and fit in uint32_t */

static void md5_data(const unsigned char *data, size_t size, char *hex) {
  MD5_HASH digest;
  Md5Calculate(data, (uint32_t)size, &digest);
  digest_to_hex(digest.bytes, sizeof(digest.bytes), hex);
}

static void sha1_data(const unsigned char *data, size_t size, char *hex) {
  SHA1_HASH digest;
  Sha1Calculate(data, (uint32_t)size, &digest);
  digest_to_hex(digest.bytes, sizeof(digest.bytes), hex);
}

static void sha256_data(const unsigned char *data, size_t size, char *hex) {
  SHA256_HASH digest;
  Sha256Calculate(data, (uint32_t)size, &digest);
  digest_to_hex(digest.bytes, sizeof(digest.bytes), hex);
}

static void sha512_data(const unsigned char *data, size_t size, char *hex) {
  SHA512_HASH digest;
  Sha512Calculate(data, (uint32_t)size, &digest);
  digest_to_hex(digest.bytes, sizeof(digest.bytes), hex);
}

int map_hash_file(map_hash_t algo, const char *path, char *hex, size_t hex_size) {
  size_t digest_size = digest_size_of(algo);
  if (digest_size == 0 || hex_size < digest_size * 2 + 1) {
    return -1;
  }
  FILE *file = fopen(path, "rb");
  if (file == NULL) {
    return -1;
  }

  int result;
  switch (algo) {
  case MAP_HASH_MD5:
    result = md5_file(file, hex);
    break;
  case MAP_HASH_SHA1:
    result = sha1_file(file, hex);
    break;
  case MAP_HASH_SHA256:
    result = sha256_file(file, hex);
    break;
  case MAP_HASH_SHA512:
    result = sha512_file(file, hex);
    break;
  default:
    result = -1;
    break;
  }

  fclose(file);
  return result;
}

int map_hash_data(map_hash_t algo, const unsigned char *data, size_t size, char *hex, size_t hex_size) {
  size_t digest_size = digest_size_of(algo);
  if (digest_size == 0 || hex_size < digest_size * 2 + 1) {
    return -1;
  }

  switch (algo) {
  case MAP_HASH_MD5:
    md5_data(data, size, hex);
    break;
  case MAP_HASH_SHA1:
    sha1_data(data, size, hex);
    break;
  case MAP_HASH_SHA256:
    sha256_data(data, size, hex);
    break;
  case MAP_HASH_SHA512:
    sha512_data(data, size, hex);
    break;
  default:
    return -1;
  }
  return 0;
}
