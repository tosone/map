#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tomcrypt.h>

#include <avl.h>
#include <base16.h>
#include <base32.h>
#include <base64.h>
#include <base64url.h>
#include <command.h>
#include <error.h>
#include <hash.h>
#include <hashmap.h>
#include <linenoise.h>
#include <lru.h>
#include <prng.h>
#include <version.h>

#define command_length_check(x, y)            \
  if (commands_length x y) {                  \
    printf("%s\n", ERR_COMMAND);              \
    commands_free(commands, commands_length); \
    continue;                                 \
  }

#define MAP_EXIT "exit"
#define MAP_HELP "help"

// 命令自动完成
void completion(const char *buf, linenoiseCompletions *lc);

// 命令格式提示
char *hints(const char *buf, int *color, int *bold);

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
    ERR_INTERNAL("register all ciphers with error");
  }
  if (register_all_hashes() != CRYPT_OK) {
    ERR_INTERNAL("register all hashes with error");
  }
  if (register_all_prngs() != CRYPT_OK) {
    ERR_INTERNAL("register all prngs with error");
  }

  char *line;
  while ((line = linenoise("map> ")) != NULL) {
    if (line[0] != '\0') {
      linenoiseHistoryAdd(line);
      linenoiseHistorySave("history.txt");

      char *line_copy = (char *)malloc(sizeof(char) * (strlen(line) + 1));
      memcpy(line_copy, line, strlen(line) + 1);

      int commands_length;
      commands_t commands = commands_parse(line, &commands_length);

      free(line_copy);

      if (commands_length >= 1) {
        if (strncmp(commands[0], MAP_HELP, strlen(MAP_HELP)) == 0) {
          printf("%s\n", VERSION);
        } else if (strncmp(commands[0], MAP_EXIT, strlen(MAP_EXIT)) == 0) {
          commands_free(commands, commands_length);
          return EXIT_SUCCESS;
        } else if (strncmp(commands[0], VERSION_COMMAND, strlen(VERSION_COMMAND)) == 0) {
          printf("%s\n", VERSION);
        } else if (strncmp(commands[0], LRU_COMMAND, strlen(LRU_COMMAND)) == 0) {
          command_length_check(<, 2);
          if (strncmp(commands[1], LRU_COMMAND_LEN, strlen(LRU_COMMAND_LEN)) == 0) {
            command_length_check(!=, 2);
            printf("%d\n", lru->len);
          } else if (strncmp(commands[1], LRU_COMMAND_SET, strlen(LRU_COMMAND_SET)) == 0) {
            command_length_check(!=, 4);
            char *key = commands[2];
            void *value = commands[3];
            LRU_set(lru, key, value, strlen(value) + 1);
          } else if (strncmp(commands[1], LRU_COMMAND_GET, strlen(LRU_COMMAND_GET)) == 0) {
            command_length_check(!=, 3);
            char *key = commands[2];
            size_t value_length = 0;
            char *value = (char *)LRU_get(lru, key, &value_length);
            if (value == NULL) {
              printf("key not found\n");
            } else {
              printf("%s\n", value);
            }
          } else if (strncmp(commands[1], LRU_COMMAND_CAP, strlen(LRU_COMMAND_CAP)) == 0) {
            command_length_check(<, 3);
            if (strncmp(commands[2], LRU_COMMAND_GET, strlen(LRU_COMMAND_GET)) == 0) {
              printf("%d\n", lru->cap);
            } else if (strncmp(commands[2], LRU_COMMAND_SET, strlen(LRU_COMMAND_SET)) == 0) {
              int cap = atoi(commands[3]);
              if (cap < lru->cap) {
                printf("please set more bigger cap");
              } else {
                lru->cap = cap;
              }
            }
          } else if (strncmp(commands[1], LRU_COMMAND_PRINT, strlen(LRU_COMMAND_PRINT)) == 0) {
            command_length_check(!=, 2);
            LRU_print(lru);
          }
        } else if (strncmp(commands[0], HMAP_COMMAND, strlen(HMAP_COMMAND)) == 0) {
          command_length_check(<, 2);
          if (strncmp(commands[1], HMAP_COMMAND_CAP, strlen(HMAP_COMMAND_CAP)) == 0) {
            command_length_check(!=, 2);
            printf("%d\n", hmap->cap);
          } else if (strncmp(commands[1], HMAP_COMMAND_LEN, strlen(HMAP_COMMAND_LEN)) == 0) {
            command_length_check(!=, 2);
            printf("%d\n", hmap->len);
          } else if (strncmp(commands[1], HMAP_COMMAND_SET, strlen(HMAP_COMMAND_SET)) == 0) {
            command_length_check(!=, 4);
            char *key = commands[2];
            void *value = commands[3];
            hmap = hashmap_set(hmap, key, value, strlen(value) + 1);
          } else if (strncmp(commands[1], HMAP_COMMAND_GET, strlen(HMAP_COMMAND_GET)) == 0) {
            command_length_check(!=, 3);
            char *key = commands[2];
            size_t value_length = 0;
            char *value = (char *)hashmap_get(hmap, key, &value_length);
            if (value == NULL) {
              printf("key not found\n");
            } else {
              printf("%s\n", value);
            }
          } else if (strncmp(commands[1], HMAP_COMMAND_DEL, strlen(HMAP_COMMAND_DEL)) == 0) {
            command_length_check(!=, 3);
            char *key = commands[2];
            hashmap_del(hmap, key);
          } else if (strncmp(commands[1], HMAP_COMMAND_PRINT, strlen(HMAP_COMMAND_PRINT)) == 0) {
            command_length_check(!=, 2);
            hashmap_print(hmap);
          }
        } else if (strncmp(commands[0], AVL_COMMAND, strlen(AVL_COMMAND)) == 0) {
          command_length_check(<, 3);
          if (strncmp(commands[1], AVL_COMMAND_SET, strlen(AVL_COMMAND_SET)) == 0) {
            command_length_check(!=, 3);
            int key = atoi(commands[2]);
            avl = avl_set(avl, key);
          } else if (strncmp(commands[1], AVL_COMMAND_GET, strlen(AVL_COMMAND_GET)) == 0) {
            command_length_check(!=, 3);
            int key = atoi(commands[2]);
            printf("%s\n", avl_get(avl, key) ? "true" : "false");
          } else if (strncmp(commands[1], AVL_COMMAND_PRINT, strlen(AVL_COMMAND_PRINT)) == 0) {
            command_length_check(!=, 3);
            if (strncmp(commands[2], AVL_COMMAND_PRE, strlen(AVL_COMMAND_PRE)) == 0) {
              avl_pre_order(avl);
            } else if (strncmp(commands[2], AVL_COMMAND_IN, strlen(AVL_COMMAND_IN)) == 0) {
              avl_in_order(avl);
            } else if (strncmp(commands[2], AVL_COMMAND_POST, strlen(AVL_COMMAND_POST)) == 0) {
              avl_post_order(avl);
            }
          } else if (strncmp(commands[1], AVL_COMMAND_DUMP, strlen(AVL_COMMAND_DUMP)) == 0) {
            command_length_check(!=, 3);
            char *filename = commands[2];
            avl_dump(avl, filename);
          }
        } else if (strncmp(commands[0], BASE64_COMMAND, strlen(BASE64_COMMAND)) == 0) {
          command_length_check(<, 3);
          if (strncmp(commands[1], BASE64_COMMAND_ENCODE, strlen(BASE64_COMMAND_ENCODE)) == 0) {
            char *string = commands[2];
            unsigned long out = 4 * ((strlen(string) + 2) / 3);
            char *outstring = (char *)calloc(out, sizeof(char));
            if (base64_encode((unsigned char *)string, strlen(string), outstring, &out) != CRYPT_OK) {
              ERR_INTERNAL("base64 encode with error");
            } else {
              printf("%s\n", outstring);
            }
          } else if (strncmp(commands[1], BASE64_COMMAND_DECODE, strlen(BASE64_COMMAND_DECODE)) == 0) {
            char *string = commands[2];
            unsigned char *outstring = (unsigned char *)calloc(strlen(string) + 1, sizeof(unsigned char));
            unsigned long out = sizeof(outstring);
            if (base64_decode(string, strlen(string) + 1, outstring, &out)) {
              ERR_INTERNAL("base64 decode with error");
            } else {
              printf("%s\n", outstring);
            }
          } else {
            printf("%s\n", ERR_COMMAND_NOT_FOUND);
          }
        } else if (strncmp(commands[0], BASE64URL_COMMAND, strlen(BASE64URL_COMMAND)) == 0) {
          command_length_check(<, 3);
          if (strncmp(commands[1], BASE64URL_COMMAND_ENCODE, strlen(BASE64URL_COMMAND_ENCODE)) == 0) {
            char *string = commands[2];
            unsigned long out = 4 * ((strlen(string) + 2) / 3);
            char *outstring = (char *)calloc(out, sizeof(char));
            if (base64url_encode((unsigned char *)string, strlen(string), outstring, &out) != CRYPT_OK) {
              ERR_INTERNAL("base64 url encode with error");
            } else {
              printf("%s\n", outstring);
            }
            free(outstring);
          } else if (strncmp(commands[1], BASE64URL_COMMAND_DECODE, strlen(BASE64URL_COMMAND_DECODE)) == 0) {
            char *string = commands[2];
            unsigned char *outstring = (unsigned char *)calloc(strlen(string) + 1, sizeof(unsigned char));
            unsigned long out = sizeof(outstring);
            if (base64url_decode(string, strlen(string), outstring, &out)) {
              ERR_INTERNAL("base64 url decode with error");
            } else {
              printf("%s\n", outstring);
            }
            free(outstring);
          } else {
            printf("%s\n", ERR_COMMAND_NOT_FOUND);
          }
        } else if (strncmp(commands[0], BASE32_COMMAND, strlen(BASE32_COMMAND)) == 0) {
          command_length_check(<, 3);
          if (strncmp(commands[1], BASE32_COMMAND_ENCODE, strlen(BASE32_COMMAND_ENCODE)) == 0) {
            char *string = commands[2];
            unsigned long out = (8 * strlen(string) + 4) / 5 + 1;
            char *outstring = (char *)calloc(out, sizeof(char));
            if (base32_encode((unsigned char *)string, strlen(string), outstring, &out, BASE32_RFC4648) != CRYPT_OK) {
              ERR_INTERNAL("base32 encode with error");
            } else {
              printf("%s\n", outstring);
            }
            free(outstring);
          } else if (strncmp(commands[1], BASE32_COMMAND_DECODE, strlen(BASE32_COMMAND_DECODE)) == 0) {
            char *string = commands[2];
            unsigned char *outstring = (unsigned char *)calloc(strlen(string) + 1, sizeof(unsigned char));
            unsigned long out = sizeof(outstring);
            if (base32_decode(string, strlen(string), outstring, &out, BASE32_RFC4648) != CRYPT_OK) {
              ERR_INTERNAL("base32 decode with error");
            } else {
              printf("%s\n", outstring);
            }
            free(outstring);
          } else {
            printf("%s\n", ERR_COMMAND_NOT_FOUND);
          }
        } else if (strncmp(commands[0], BASE16_COMMAND, strlen(BASE16_COMMAND)) == 0) {
          command_length_check(<, 3);
          if (strncmp(commands[1], BASE16_COMMAND_ENCODE, strlen(BASE16_COMMAND_ENCODE)) == 0) {
            char *string = commands[2];
            unsigned long out = strlen(string) * 2 + 1;
            char *outstring = (char *)calloc(out, sizeof(char));
            if (base16_encode((unsigned char *)string, strlen(string), outstring, &out, 0) != CRYPT_OK) {
              ERR_INTERNAL("base16 encode with error");
            } else {
              printf("%s\n", outstring);
            }
            free(outstring);
          } else if (strncmp(commands[1], BASE16_COMMAND_DECODE, strlen(BASE16_COMMAND_DECODE)) == 0) {
            char *string = commands[2];
            unsigned char *outstring = (unsigned char *)calloc(strlen(string) + 1, sizeof(unsigned char));
            unsigned long out = sizeof(outstring);
            if (base16_decode(string, strlen(string), outstring, &out) != CRYPT_OK) {
              ERR_INTERNAL("base16 decode with error");
            } else {
              printf("%s\n", outstring);
            }
            free(outstring);
          } else {
            printf("%s\n", ERR_COMMAND_NOT_FOUND);
          }
        } else if (strncmp(commands[0], HASH_COMMAND, strlen(HASH_COMMAND)) == 0) {
          command_length_check(!=, 3);
          char *hash_name = commands[1];
          char *string = commands[2];
          hash_state context;
          int hash_index = find_hash(hash_name);
          if (hash_index < 0) {
            printf("cannot find hash method\n");
          }
          if (hash_descriptor[hash_index].init(&context) != CRYPT_OK) {
            ERR_INTERNAL("hash init with error");
          }
          if (hash_descriptor[hash_index].process(&context, (unsigned char *)string, strlen(string)) != CRYPT_OK) {
            ERR_INTERNAL("hash process with error");
          }
          unsigned char *outbyte = (unsigned char *)calloc(hash_descriptor[hash_index].hashsize, sizeof(unsigned char));
          if (hash_descriptor[hash_index].done(&context, outbyte) != CRYPT_OK) {
            ERR_INTERNAL("hash done with error");
          }
          unsigned long out = hash_descriptor[hash_index].hashsize * 2 + 1;
          char *outstring = (char *)calloc(out, sizeof(char));
          if (base16_encode(outbyte, hash_descriptor[hash_index].hashsize, outstring, &out, 0) != CRYPT_OK) {
            ERR_INTERNAL("hash base16 encode with error");
          } else {
            printf("%s\n", outstring);
          }
          free(outstring);
        } else if (strncmp(commands[0], PRNG_COMMAND, strlen(PRNG_COMMAND)) == 0) {
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
            ERR_INTERNAL("prng start with error");
          }
          if (prng_descriptor[prng_index].add_entropy((unsigned char *)entropy, strlen(entropy), &context) != CRYPT_OK) {
            ERR_INTERNAL("prng add entropy with error");
          }
          if (chacha20_prng_ready(&context) != CRYPT_OK) {
            ERR_INTERNAL("prng ready with error");
          }
          unsigned char *outbyte = (unsigned char *)calloc(length, sizeof(unsigned char));
          if (chacha20_prng_read(outbyte, length, &context) != length) {
            ERR_INTERNAL("prng read with error");
          }
          unsigned long out = length * 2 + 1;
          char *outstring = (char *)calloc(out, sizeof(char));
          if (base16_encode(outbyte, length, outstring, &out, 0) != CRYPT_OK) {
            ERR_INTERNAL("prng base16 encode with error");
          } else {
            printf("%s\n", outstring);
          }
          free(outstring);
        } else {
          printf("%s\n", ERR_COMMAND_NOT_FOUND);
        }
      } else {
        printf("%s\n", ERR_COMMAND);
      }
      commands_free(commands, commands_length);
    }
    free(line);
  }
  return EXIT_SUCCESS;
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
