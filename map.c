#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <tomcrypt.h>

#include <avl.h>
#include <command.h>
#include <hashmap.h>
#include <kilo.h>
#include <linenoise.h>
#include <lru.h>

#define VERSION "v1.0.0"

#define VERSION_COMMAND "version"
#define HELP_COMMAND "help"
#define PRNG_COMMAND "prng"
#define HASH_COMMAND "hash"

#define BASE64_COMMAND "base64"
#define BASE64_COMMAND_DECODE "dec"
#define BASE64_COMMAND_ENCODE "enc"

#define BASE64URL_COMMAND "base64url"
#define BASE64URL_COMMAND_DECODE "dec"
#define BASE64URL_COMMAND_ENCODE "enc"

#define BASE32_COMMAND "base32"
#define BASE32_COMMAND_DECODE "dec"
#define BASE32_COMMAND_ENCODE "enc"

#define BASE16_COMMAND "base16"
#define BASE16_COMMAND_DECODE "dec"
#define BASE16_COMMAND_ENCODE "enc"

#define VI_COMMAND "vi"

#define ERR_COMMAND "invalid command"
#define ERR_COMMAND_NOT_FOUND "command not found"

#define ERR_INTERNAL "internal error"

void map_err(char *level, char *info) {
#ifdef NDEBUG
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  char buf[16];
  buf[strftime(buf, sizeof(buf), "%H:%M:%S", tm)] = '\0';
  printf("[%s] %s:%d %s: %s.\n", buf, __FILE__, __LINE__, level, info);
#else
  printf("%s: %s.\n", level, info);
#endif
}

#define MAP_COMMANDS_OK true
#define MAP_COMMANDS_ERROR false

#define command_length_check(x, y) \
  if (commands_length x y) {       \
    printf("%s\n", ERR_COMMAND);   \
    return MAP_COMMANDS_ERROR;     \
  }

#define MAP_EXIT "exit"

// 命令自动完成
void completion(const char *buf, linenoiseCompletions *lc);

// 命令格式提示
char *hints(const char *buf, int *color, int *bold);

bool prng_command(commands_t commands, int commands_length);
bool hash_command(commands_t commands, int commands_length);

bool base64_command(commands_t commands, int commands_length);
bool base64url_command(commands_t commands, int commands_length);
bool base32_command(commands_t commands, int commands_length);
bool base16_command(commands_t commands, int commands_length);

void base16_encode_command(commands_t commands);
void base16_decode_command(commands_t commands);
void base32_encode_command(commands_t commands);
void base32_decode_command(commands_t commands);
void base64url_encode_command(commands_t commands);
void base64url_decode_command(commands_t commands);
void base64_decode_command(commands_t commands);
void base64_encode_command(commands_t commands);

bool lru_command(commands_t commands, int commands_length);
bool hmap_command(commands_t commands, int commands_length);
bool avl_command(commands_t commands, int commands_length);

bool help_command(commands_t commands, int commands_length);

bool vi_command(commands_t commands, int commands_length);

#define COMMANDS_CHECK(x)                     \
  if (x) {                                    \
    commands_free(commands, commands_length); \
    continue;                                 \
  }

hashmap_t *hmap;
LRU *lru;
avl_entry_t *avl;

void clear() {
  hashmap_free(hmap);
  LRU_free(lru);
  avl_free(avl);
  printf("clear all, bye\n");
}

int main(int argc, char **argv) {
  linenoiseSetCompletionCallback(completion);
  linenoiseSetHintsCallback(hints);
  linenoiseHistoryLoad("history.txt");
  linenoiseHistorySetMaxLen(1000);

  hmap = hashmap_create();
  lru = LRU_create();
  atexit(clear);

  if (register_all_ciphers() != CRYPT_OK) {
    map_err(ERR_INTERNAL, "register all ciphers with error");
  }
  if (register_all_hashes() != CRYPT_OK) {
    map_err(ERR_INTERNAL, "register all hashes with error");
  }
  if (register_all_prngs() != CRYPT_OK) {
    map_err(ERR_INTERNAL, "register all prngs with error");
  }

  char *line;
  while ((line = linenoise("map> ")) != NULL) {
    if (line[0] != '\0') {
      linenoiseHistoryAdd(line);
      linenoiseHistorySave("history.txt");

      char *line_copy = (char *)malloc(sizeof(char) * (strlen(line) + 1));
      memcpy(line_copy, line, strlen(line) + 1);
      int commands_length;
      commands_t commands = commands_parse(line_copy, &commands_length);
      free(line_copy);

      if (commands_length >= 1) {
        if (strncasecmp(commands[0], HELP_COMMAND, strlen(HELP_COMMAND)) == 0) {
          COMMANDS_CHECK(!help_command(commands, commands_length));
        } else if (strncasecmp(commands[0], MAP_EXIT, strlen(MAP_EXIT)) == 0) {
          commands_free(commands, commands_length);
          return EXIT_SUCCESS;
        } else if (strncasecmp(commands[0], VERSION_COMMAND, strlen(VERSION_COMMAND)) == 0) {
          printf("%s\n", VERSION);
        } else if (strncasecmp(commands[0], LRU_COMMAND, strlen(LRU_COMMAND)) == 0) {
          COMMANDS_CHECK(!lru_command(commands, commands_length));
        } else if (strncasecmp(commands[0], HMAP_COMMAND, strlen(HMAP_COMMAND)) == 0) {
          COMMANDS_CHECK(!hmap_command(commands, commands_length));
        } else if (strncasecmp(commands[0], AVL_COMMAND, strlen(AVL_COMMAND)) == 0) {
          COMMANDS_CHECK(!avl_command(commands, commands_length));
        } else if (strncasecmp(commands[0], BASE64_COMMAND, strlen(BASE64_COMMAND)) == 0) {
          COMMANDS_CHECK(!base64_command(commands, commands_length));
        } else if (strncasecmp(commands[0], BASE64URL_COMMAND, strlen(BASE64URL_COMMAND)) == 0) {
          COMMANDS_CHECK(!base64url_command(commands, commands_length));
        } else if (strncasecmp(commands[0], BASE32_COMMAND, strlen(BASE32_COMMAND)) == 0) {
          COMMANDS_CHECK(!base32_command(commands, commands_length));
        } else if (strncasecmp(commands[0], BASE16_COMMAND, strlen(BASE16_COMMAND)) == 0) {
          COMMANDS_CHECK(!base16_command(commands, commands_length));
        } else if (strncasecmp(commands[0], HASH_COMMAND, strlen(HASH_COMMAND)) == 0) {
          COMMANDS_CHECK(!hash_command(commands, commands_length));
        } else if (strncasecmp(commands[0], PRNG_COMMAND, strlen(PRNG_COMMAND)) == 0) {
          COMMANDS_CHECK(!prng_command(commands, commands_length));
        } else if (strncasecmp(commands[0], VI_COMMAND, strlen(VI_COMMAND)) == 0) {
          COMMANDS_CHECK(!vi_command(commands, commands_length));
        } else {
          printf("%s\n", ERR_COMMAND_NOT_FOUND);
        }
      }
      commands_free(commands, commands_length);
    }
    free(line);
  }
  return EXIT_SUCCESS;
}

bool vi_command(commands_t commands, int commands_length) {
  command_length_check(!=, 2);
  initEditor();
  editorSelectSyntaxHighlight(commands[1]);
  editorOpen(commands[1]);
  enableRawMode(STDIN_FILENO);
  editorSetStatusMessage("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find");
  while (1) {
    editorRefreshScreen();
    if (!editorProcessKeypress(STDIN_FILENO)) {
      disableRawMode(STDIN_FILENO);
      linenoiseClearScreen();
      break;
    }
  }
  return MAP_COMMANDS_OK;
}

bool help_command(commands_t commands, int commands_length) {
  printf("Hashmap\n");
  printf("    \033[0;32mhmap\033[0m set <key> <value>\n");
  printf("    \033[0;32mhmap\033[0m get <key>\n");
  printf("    \033[0;32mhmap\033[0m del <key>\n");
  printf("    \033[0;32mhmap\033[0m cap\n");
  printf("    \033[0;32mhmap\033[0m len\n");
  printf("    \033[0;32mhmap\033[0m print\n");
  printf("LRU\n");
  printf("    \033[0;32mlru\033[0m set <num> <value>\n");
  printf("    \033[0;32mlru\033[0m get <num>\n");
  printf("    \033[0;32mlru\033[0m cap set <num>\n");
  printf("    \033[0;32mlru\033[0m cap get\n");
  printf("    \033[0;32mlru\033[0m len\n");
  printf("    \033[0;32mlru\033[0m print\n");
  printf("AVL-Tree\n");
  printf("    \033[0;32mavl\033[0m set <num>\n");
  printf("    \033[0;32mavl\033[0m get <num>\n");
  printf("    \033[0;32mavl\033[0m print <pre/in/post>\n");
  printf("    \033[0;32mavl\033[0m dump <filename>\n");
  printf("Base64\n");
  printf("    \033[0;32mbase64\033[0m enc <string>\n");
  printf("    \033[0;32mbase64\033[0m dec <string>\n");
  printf("Base64url\n");
  printf("    \033[0;32mbase64url\033[0m enc <string>\n");
  printf("    \033[0;32mbase64url\033[0m dec <string>\n");
  printf("Base32\n");
  printf("    \033[0;32mbase32\033[0m enc <string>\n");
  printf("    \033[0;32mbase32\033[0m dec <string>\n");
  printf("Base16\n");
  printf("    \033[0;32mbase16\033[0m enc <string>\n");
  printf("    \033[0;32mbase16\033[0m dec <string>\n");
  printf("Hash\n");
  printf("    \033[1;33mSupport hash methods: ");
  for (int i = 0; i < TAB_SIZE - 1; i++) {
    if (i == 0) {
      printf("%s", hash_descriptor[i].name);
    } else {
      if (i % 7 == 0) {
        printf("\n   ");
      }
      printf(" %s", hash_descriptor[i].name);
    }
  }
  printf("\033[0m\n");
  printf("    \033[0;32mhash\033[0m <method> <string>\n");
  printf("PRNG\n");
  printf("    \033[1;33mSupport prng methods: ");
  for (int i = 0; i < 6; i++) {
    if (i == 0) {
      printf("%s", prng_descriptor[i].name);
    } else {
      if (i % 7 == 0) {
        printf("\n   ");
      }
      printf(" %s", prng_descriptor[i].name);
    }
  }
  printf("\033[0m\n");
  printf("    \033[0;32mprng\033[0m <method> <string> <length>\n");
  printf("Vim\n");
  printf("    \033[0;32mvi\033[0m <filename>\n");
  return MAP_COMMANDS_OK;
}

bool base64_command(commands_t commands, int commands_length) {
  command_length_check(!=, 3);
  if (strncasecmp(commands[1], BASE64_COMMAND_ENCODE, strlen(BASE64_COMMAND_ENCODE)) == 0) {
    base64_encode_command(commands);
  } else if (strncasecmp(commands[1], BASE64_COMMAND_DECODE, strlen(BASE64_COMMAND_DECODE)) == 0) {
    base64_decode_command(commands);
  } else {
    printf("%s\n", ERR_COMMAND_NOT_FOUND);
  }
  return MAP_COMMANDS_OK;
}

bool base64url_command(commands_t commands, int commands_length) {
  command_length_check(!=, 3);
  if (strncasecmp(commands[1], BASE64URL_COMMAND_ENCODE, strlen(BASE64URL_COMMAND_ENCODE)) == 0) {
    base64url_encode_command(commands);
  } else if (strncasecmp(commands[1], BASE64URL_COMMAND_DECODE, strlen(BASE64URL_COMMAND_DECODE)) == 0) {
    base64url_decode_command(commands);
  } else {
    printf("%s\n", ERR_COMMAND_NOT_FOUND);
  }
  return MAP_COMMANDS_OK;
}

bool base32_command(commands_t commands, int commands_length) {
  command_length_check(!=, 3);
  if (strncasecmp(commands[1], BASE32_COMMAND_ENCODE, strlen(BASE32_COMMAND_ENCODE)) == 0) {
    base32_encode_command(commands);
  } else if (strncasecmp(commands[1], BASE32_COMMAND_DECODE, strlen(BASE32_COMMAND_DECODE)) == 0) {
    base32_decode_command(commands);
  } else {
    printf("%s\n", ERR_COMMAND_NOT_FOUND);
  }
  return MAP_COMMANDS_OK;
}

bool base16_command(commands_t commands, int commands_length) {
  command_length_check(!=, 3);
  if (strncasecmp(commands[1], BASE16_COMMAND_ENCODE, strlen(BASE16_COMMAND_ENCODE)) == 0) {
    base16_encode_command(commands);
  } else if (strncasecmp(commands[1], BASE16_COMMAND_DECODE, strlen(BASE16_COMMAND_DECODE)) == 0) {
    base16_decode_command(commands);
  } else {
    printf("%s\n", ERR_COMMAND_NOT_FOUND);
  }
  return MAP_COMMANDS_OK;
}

bool avl_command(commands_t commands, int commands_length) {
  command_length_check(<, 3);
  if (strncasecmp(commands[1], AVL_COMMAND_SET, strlen(AVL_COMMAND_SET)) == 0) {
    command_length_check(!=, 3);
    int key = atoi(commands[2]);
    avl = avl_set(avl, key);
  } else if (strncasecmp(commands[1], AVL_COMMAND_GET, strlen(AVL_COMMAND_GET)) == 0) {
    command_length_check(!=, 3);
    int key = atoi(commands[2]);
    printf("%s\n", avl_get(avl, key) ? "true" : "false");
  } else if (strncasecmp(commands[1], AVL_COMMAND_PRINT, strlen(AVL_COMMAND_PRINT)) == 0) {
    command_length_check(!=, 3);
    if (strncasecmp(commands[2], AVL_COMMAND_PRE, strlen(AVL_COMMAND_PRE)) == 0) {
      avl_pre_order(avl);
    } else if (strncasecmp(commands[2], AVL_COMMAND_IN, strlen(AVL_COMMAND_IN)) == 0) {
      avl_in_order(avl);
    } else if (strncasecmp(commands[2], AVL_COMMAND_POST, strlen(AVL_COMMAND_POST)) == 0) {
      avl_post_order(avl);
    }
  } else if (strncasecmp(commands[1], AVL_COMMAND_DUMP, strlen(AVL_COMMAND_DUMP)) == 0) {
    command_length_check(!=, 3);
    char *filename = commands[2];
    avl_dump(avl, filename);
  }
  return MAP_COMMANDS_OK;
}

bool hmap_command(commands_t commands, int commands_length) {
  command_length_check(<, 2);
  if (strncasecmp(commands[1], HMAP_COMMAND_CAP, strlen(HMAP_COMMAND_CAP)) == 0) {
    command_length_check(!=, 2);
    printf("%d\n", hmap->cap);
  } else if (strncasecmp(commands[1], HMAP_COMMAND_LEN, strlen(HMAP_COMMAND_LEN)) == 0) {
    command_length_check(!=, 2);
    printf("%d\n", hmap->len);
  } else if (strncasecmp(commands[1], HMAP_COMMAND_SET, strlen(HMAP_COMMAND_SET)) == 0) {
    command_length_check(!=, 4);
    char *key = commands[2];
    void *value = commands[3];
    hmap = hashmap_set(hmap, key, value, strlen(value) + 1);
  } else if (strncasecmp(commands[1], HMAP_COMMAND_GET, strlen(HMAP_COMMAND_GET)) == 0) {
    command_length_check(!=, 3);
    char *key = commands[2];
    size_t value_length = 0;
    char *value = (char *)hashmap_get(hmap, key, &value_length);
    if (value == NULL) {
      printf("key not found\n");
    } else {
      printf("%s\n", value);
    }
  } else if (strncasecmp(commands[1], HMAP_COMMAND_DEL, strlen(HMAP_COMMAND_DEL)) == 0) {
    command_length_check(!=, 3);
    char *key = commands[2];
    hashmap_del(hmap, key);
  } else if (strncasecmp(commands[1], HMAP_COMMAND_PRINT, strlen(HMAP_COMMAND_PRINT)) == 0) {
    command_length_check(!=, 2);
    hashmap_print(hmap);
  }
  return MAP_COMMANDS_OK;
}

bool lru_command(commands_t commands, int commands_length) {
  command_length_check(<, 2);
  if (strncasecmp(commands[1], LRU_COMMAND_LEN, strlen(LRU_COMMAND_LEN)) == 0) {
    command_length_check(!=, 2);
    printf("%d\n", lru->len);
  } else if (strncasecmp(commands[1], LRU_COMMAND_SET, strlen(LRU_COMMAND_SET)) == 0) {
    command_length_check(!=, 4);
    char *key = commands[2];
    void *value = commands[3];
    LRU_set(lru, key, value, strlen(value) + 1);
  } else if (strncasecmp(commands[1], LRU_COMMAND_GET, strlen(LRU_COMMAND_GET)) == 0) {
    command_length_check(!=, 3);
    char *key = commands[2];
    size_t value_length = 0;
    char *value = (char *)LRU_get(lru, key, &value_length);
    if (value == NULL) {
      printf("key not found\n");
    } else {
      printf("%s\n", value);
    }
  } else if (strncasecmp(commands[1], LRU_COMMAND_CAP, strlen(LRU_COMMAND_CAP)) == 0) {
    command_length_check(<, 3);
    if (strncasecmp(commands[2], LRU_COMMAND_GET, strlen(LRU_COMMAND_GET)) == 0) {
      printf("%d\n", lru->cap);
    } else if (strncasecmp(commands[2], LRU_COMMAND_SET, strlen(LRU_COMMAND_SET)) == 0) {
      int cap = atoi(commands[3]);
      if (cap < lru->cap) {
        printf("please set more bigger cap\n");
      } else {
        lru->cap = cap;
      }
    }
  } else if (strncasecmp(commands[1], LRU_COMMAND_PRINT, strlen(LRU_COMMAND_PRINT)) == 0) {
    command_length_check(!=, 2);
    LRU_print(lru);
  }
  return MAP_COMMANDS_OK;
}

void base64url_encode_command(commands_t commands) {
  char *string = commands[2];
  unsigned long out = 4 * ((strlen(string) + 2) / 3);
  char *outstring = (char *)calloc(out, sizeof(char));
  if (base64url_encode((unsigned char *)string, strlen(string), outstring, &out) != CRYPT_OK) {
    map_err(ERR_INTERNAL, "base64 url encode with error");
  } else {
    printf("%s\n", outstring);
  }
  free(outstring);
}

void base64url_decode_command(commands_t commands) {
  char *string = commands[2];
  unsigned char *outstring = (unsigned char *)calloc(strlen(string) + 1, sizeof(unsigned char));
  unsigned long out = sizeof(outstring);
  if (base64url_decode(string, strlen(string), outstring, &out)) {
    map_err(ERR_INTERNAL, "base64 url decode with error");
  } else {
    printf("%s\n", outstring);
  }
  free(outstring);
}

void base64_decode_command(commands_t commands) {
  char *string = commands[2];
  unsigned char *outstring = (unsigned char *)calloc(strlen(string) + 1, sizeof(unsigned char));
  unsigned long out = sizeof(outstring);
  if (base64_decode(string, strlen(string) + 1, outstring, &out)) {
    map_err(ERR_INTERNAL, "base64 decode with error");
  } else {
    printf("%s\n", outstring);
  }
}

void base64_encode_command(commands_t commands) {
  char *string = commands[2];
  unsigned long out = 4 * ((strlen(string) + 2) / 3) + 1;
  char *outstring = (char *)calloc(out, sizeof(char));
  if (base64_encode((unsigned char *)string, strlen(string), outstring, &out) != CRYPT_OK) {
    map_err(ERR_INTERNAL, "base64 encode with error");
  } else {
    printf("%s\n", outstring);
  }
}

void base32_encode_command(commands_t commands) {
  char *string = commands[2];
  unsigned long out = (8 * strlen(string) + 4) / 5 + 1;
  char *outstring = (char *)calloc(out, sizeof(char));
  if (base32_encode((unsigned char *)string, strlen(string), outstring, &out, BASE32_RFC4648) != CRYPT_OK) {
    map_err(ERR_INTERNAL, "base32 encode with error");
  } else {
    printf("%s\n", outstring);
  }
  free(outstring);
}
void base32_decode_command(commands_t commands) {
  char *string = commands[2];
  unsigned char *outstring = (unsigned char *)calloc(strlen(string) + 1, sizeof(unsigned char));
  unsigned long out = sizeof(outstring);
  if (base32_decode(string, strlen(string), outstring, &out, BASE32_RFC4648) != CRYPT_OK) {
    map_err(ERR_INTERNAL, "base32 decode with error");
  } else {
    printf("%s\n", outstring);
  }
  free(outstring);
}

void base16_encode_command(commands_t commands) {
  char *string = commands[2];
  unsigned long out = strlen(string) * 2 + 1;
  char *outstring = (char *)calloc(out, sizeof(char));
  if (base16_encode((unsigned char *)string, strlen(string), outstring, &out, 0) != CRYPT_OK) {
    map_err(ERR_INTERNAL, "base16 encode with error");
  } else {
    printf("%s\n", outstring);
  }
  free(outstring);
}

void base16_decode_command(commands_t commands) {
  char *string = commands[2];
  unsigned char *outstring = (unsigned char *)calloc(strlen(string) + 1, sizeof(unsigned char));
  unsigned long out = sizeof(outstring);
  if (base16_decode(string, strlen(string), outstring, &out) != CRYPT_OK) {
    map_err(ERR_INTERNAL, "base16 decode with error");
  } else {
    printf("%s\n", outstring);
  }
  free(outstring);
}

bool hash_command(commands_t commands, int commands_length) {
  command_length_check(!=, 3);
  char *hash_name = commands[1];
  char *string = commands[2];

  int hash_index = find_hash(hash_name);
  if (hash_index < 0) {
    printf("cannot find hash method\n");
  }

  unsigned char *outbyte = (unsigned char *)calloc(hash_descriptor[hash_index].hashsize, sizeof(unsigned char));

  struct stat file_handler;
  if (stat(string, &file_handler) == 0) {
    unsigned long out = hash_descriptor[hash_index].hashsize;
    if (hash_file(hash_index, string, outbyte, &out) != CRYPT_OK) {
      map_err(ERR_INTERNAL, "hash file with error");
      goto hash_command_flag;
    }
  } else {
    hash_state context;
    if (hash_descriptor[hash_index].init(&context) != CRYPT_OK) {
      map_err(ERR_INTERNAL, "hash init with error");
      goto hash_command_flag;
    }
    if (hash_descriptor[hash_index].process(&context, (unsigned char *)string, strlen(string)) != CRYPT_OK) {
      map_err(ERR_INTERNAL, "hash process with error");
      goto hash_command_flag;
    }
    if (hash_descriptor[hash_index].done(&context, outbyte) != CRYPT_OK) {
      map_err(ERR_INTERNAL, "hash done with error");
      goto hash_command_flag;
    }
  }

  unsigned long out = hash_descriptor[hash_index].hashsize * 2 + 1;
  char *outstring = (char *)calloc(out, sizeof(char));
  if (base16_encode(outbyte, hash_descriptor[hash_index].hashsize, outstring, &out, 0) != CRYPT_OK) {
    map_err(ERR_INTERNAL, "hash base16 encode with error");
  } else {
    printf("%s\n", outstring);
  }
  free(outstring);
hash_command_flag:
  free(outbyte);
  return MAP_COMMANDS_OK;
}

bool prng_command(commands_t commands, int commands_length) {
  command_length_check(!=, 4);
  char *hash_name = commands[1];
  char *entropy = commands[2];
  int length = atoi(commands[3]);
  if (length <= 0) {
    printf("length is invalid\n");
  }
  prng_state context;
  int prng_index = find_prng(hash_name);
  if (prng_index < 0) {
    printf("cannot find prng method\n");
  }
  if (prng_descriptor[prng_index].start(&context) != CRYPT_OK) {
    map_err(ERR_INTERNAL, "prng start with error");
  }
  if (prng_descriptor[prng_index].add_entropy((unsigned char *)entropy, strlen(entropy), &context) != CRYPT_OK) {
    map_err(ERR_INTERNAL, "prng add entropy with error");
  }
  if (chacha20_prng_ready(&context) != CRYPT_OK) {
    map_err(ERR_INTERNAL, "prng ready with error");
  }
  unsigned char *outbyte = (unsigned char *)calloc(length, sizeof(unsigned char));
  if (chacha20_prng_read(outbyte, length, &context) != length) {
    map_err(ERR_INTERNAL, "prng read with error");
  }
  unsigned long out = length * 2 + 1;
  char *outstring = (char *)calloc(out, sizeof(char));
  if (base16_encode(outbyte, length, outstring, &out, 0) != CRYPT_OK) {
    map_err(ERR_INTERNAL, "prng base16 encode with error");
  } else {
    printf("%s\n", outstring);
  }
  free(outstring);
  return MAP_COMMANDS_OK;
}

void completion(const char *buf, linenoiseCompletions *lc) {
  if (buf[0] == 'l') {
    linenoiseAddCompletion(lc, "lru");
  } else if (buf[0] == 'h') {
    linenoiseAddCompletion(lc, "hmap");
  } else if (buf[0] == 'a') {
    linenoiseAddCompletion(lc, "avl");
  } else if (buf[0] == 'b') {
    linenoiseAddCompletion(lc, "base64");
  }
}

char *hints(const char *buf, int *color, int *bold) {
  *color = 32;
  *bold = 0;
  if (strcmp(buf, "lru") == 0) {
    return " <command>";
  } else if (strcmp(buf, "lru len") == 0) {
    return " <length>";
  } else if (strcmp(buf, "lru get") == 0) {
    return " <key>";
  } else if (strcmp(buf, "lru set") == 0) {
    return " <key> <value>";
  } else if (strcmp(buf, "lru cap") == 0) {
    return " <command> <value>";
  } else if (strcmp(buf, "lru cap set") == 0) {
    return " <value>";
  } else if (strcmp(buf, "hmap") == 0) {
    return " <command>";
  } else if (strcmp(buf, "hmap get") == 0) {
    return " <key>";
  } else if (strcmp(buf, "hmap set") == 0) {
    return " <key> <value>";
  } else if (strcmp(buf, "hmap del") == 0) {
    return " <key>";
  } else if (strcmp(buf, "avl") == 0) {
    return " <command> <params>";
  } else if (strcmp(buf, "avl get") == 0) {
    return " <key>";
  } else if (strcmp(buf, "avl set") == 0) {
    return " <key>";
  } else if (strcmp(buf, "avl print") == 0) {
    return " <pre/in/post>";
  } else if (strcmp(buf, "avl dump") == 0) {
    return " <filename>";
  } else if (strcmp(buf, "base64") == 0) {
    return " <command> <string>";
  } else if (strcmp(buf, "base64 dec") == 0) {
    return " <string>";
  } else if (strcmp(buf, "base64 enc") == 0) {
    return " <string>";
  }
  return NULL;
}
