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
#include <hashmap.h>
#include <linenoise.h>
#include <lru.h>
#include <md2.h>
#include <md4.h>
#include <md5.h>
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
              printf("%s\n", ERR_INTERNAL);
            } else {
              printf("%s\n", outstring);
            }
          } else if (strncmp(commands[1], BASE64_COMMAND_DECODE, strlen(BASE64_COMMAND_DECODE)) == 0) {
            char *string = commands[2];
            unsigned char *outstring = (unsigned char *)calloc(strlen(string) + 1, sizeof(unsigned char));
            unsigned long out = sizeof(outstring);
            if (base64_decode(string, strlen(string) + 1, outstring, &out)) {
              printf("%s\n", ERR_INTERNAL);
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
              printf("%s\n", ERR_INTERNAL);
            } else {
              printf("%s\n", outstring);
            }
            free(outstring);
          } else if (strncmp(commands[1], BASE64URL_COMMAND_DECODE, strlen(BASE64URL_COMMAND_DECODE)) == 0) {
            char *string = commands[2];
            unsigned char *outstring = (unsigned char *)calloc(strlen(string) + 1, sizeof(unsigned char));
            unsigned long out = sizeof(outstring);
            if (base64url_decode(string, strlen(string), outstring, &out)) {
              printf("%s\n", ERR_INTERNAL);
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
              printf("%s\n", ERR_INTERNAL);
            } else {
              printf("%s\n", outstring);
            }
            free(outstring);
          } else if (strncmp(commands[1], BASE32_COMMAND_DECODE, strlen(BASE32_COMMAND_DECODE)) == 0) {
            char *string = commands[2];
            unsigned char *outstring = (unsigned char *)calloc(strlen(string) + 1, sizeof(unsigned char));
            unsigned long out = sizeof(outstring);
            if (base32_decode(string, strlen(string), outstring, &out, BASE32_RFC4648) != CRYPT_OK) {
              printf("%s\n", ERR_INTERNAL);
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
              printf("%s\n", ERR_INTERNAL);
            } else {
              printf("%s\n", outstring);
            }
            free(outstring);
          } else if (strncmp(commands[1], BASE16_COMMAND_DECODE, strlen(BASE16_COMMAND_DECODE)) == 0) {
            char *string = commands[2];
            unsigned char *outstring = (unsigned char *)calloc(strlen(string) + 1, sizeof(unsigned char));
            unsigned long out = sizeof(outstring);
            if (base16_decode(string, strlen(string), outstring, &out) != CRYPT_OK) {
              printf("%s\n", ERR_INTERNAL);
            } else {
              printf("%s\n", outstring);
            }
            free(outstring);
          } else {
            printf("%s\n", ERR_COMMAND_NOT_FOUND);
          }
        } else if (strncmp(commands[0], MD5_COMMAND, strlen(MD5_COMMAND)) == 0) {
          command_length_check(!=, 2);
          char *string = commands[1];
          hash_state *context = (hash_state *)malloc(sizeof(hash_state));
          if (md5_init(context) != CRYPT_OK) {
            printf("%s\n", ERR_INTERNAL);
            goto md5flag;
          }
          if (md5_process(context, (unsigned char *)string, strlen(string)) != CRYPT_OK) {
            printf("%s\n", ERR_INTERNAL);
            goto md5flag;
          }
          unsigned char *outbyte = (unsigned char *)calloc(MD5_SIZE, sizeof(unsigned char));
          if (md5_done(context, outbyte) != CRYPT_OK) {
            printf("%s\n", ERR_INTERNAL);
            goto md5flag;
          }
          unsigned long out = MD5_SIZE * 2 + 1;
          char *outstring = (char *)calloc(out, sizeof(char));
          if (base16_encode(outbyte, MD5_SIZE, outstring, &out, 0) != CRYPT_OK) {
            printf("%s\n", ERR_INTERNAL);
          } else {
            printf("%s\n", outstring);
          }
          free(outstring);
        md5flag:
          free(context);
        } else if (strncmp(commands[0], MD4_COMMAND, strlen(MD4_COMMAND)) == 0) {
          command_length_check(!=, 2);
          char *string = commands[1];
          hash_state *context = (hash_state *)malloc(sizeof(hash_state));
          if (md4_init(context) != CRYPT_OK) {
            printf("%s\n", ERR_INTERNAL);
            goto md4flag;
          }
          if (md4_process(context, (unsigned char *)string, strlen(string)) != CRYPT_OK) {
            printf("%s\n", ERR_INTERNAL);
            goto md4flag;
          }
          unsigned char *outbyte = (unsigned char *)calloc(MD4_SIZE, sizeof(unsigned char));
          if (md4_done(context, outbyte) != CRYPT_OK) {
            printf("%s\n", ERR_INTERNAL);
            goto md4flag;
          }
          unsigned long out = MD4_SIZE * 2 + 1;
          char *outstring = (char *)calloc(out, sizeof(char));
          if (base16_encode(outbyte, MD4_SIZE, outstring, &out, 0) != CRYPT_OK) {
            printf("%s\n", ERR_INTERNAL);
          } else {
            printf("%s\n", outstring);
          }
          free(outstring);
        md4flag:
          free(context);
        } else if (strncmp(commands[0], MD2_COMMAND, strlen(MD2_COMMAND)) == 0) {
          command_length_check(!=, 2);
          char *string = commands[1];
          hash_state *context = (hash_state *)malloc(sizeof(hash_state));
          if (md2_init(context) != CRYPT_OK) {
            printf("%s\n", ERR_INTERNAL);
            goto md2flag;
          }
          if (md2_process(context, (unsigned char *)string, strlen(string)) != CRYPT_OK) {
            printf("%s\n", ERR_INTERNAL);
            goto md2flag;
          }
          unsigned char *outbyte = (unsigned char *)calloc(MD2_SIZE, sizeof(unsigned char));
          if (md2_done(context, outbyte) != CRYPT_OK) {
            printf("%s\n", ERR_INTERNAL);
            goto md2flag;
          }
          unsigned long out = MD2_SIZE * 2 + 1;
          char *outstring = (char *)calloc(out, sizeof(char));
          if (base16_encode(outbyte, MD2_SIZE, outstring, &out, 0) != CRYPT_OK) {
            printf("%s\n", ERR_INTERNAL);
          } else {
            printf("%s\n", outstring);
          }
          free(outstring);
        md2flag:
          free(context);
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
