#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/utsname.h>

#include <kilo.h>
#include <linenoise.h>
#include <mbedtls/base64.h>
#include <mbedtls/md.h>
#include <mongoose.h>
#include <uptime.h>
#include <uuid4.h>

#include <2048.h>
#include <command.h>
#include <pi.h>
#include <tcp.h>

#include <avl.h>
#include <hashmap.h>
#include <lru.h>
#include <skiplist.h>

#pragma once

#define COMMAND_VERSION "version"
#define COMMAND_HELP "help"
#define COMMAND_EXIT "exit"

#define COMMAND_HASH "hash"
#define COMMAND_HASH_MD5 "md5"
#define COMMAND_HASH_SHA1 "sha1"
#define COMMAND_HASH_SHA256 "sha256"
#define COMMAND_HASH_SHA512 "sha512"

#define COMMAND_BASE64 "base64"
#define COMMAND_BASE64_DECODE "dec"
#define COMMAND_BASE64_ENCODE "enc"

#define COMMAND_VI "vi"

#define COMMAND_TCP "tcp"

#define COMMAND_SERVER "server"
#define COMMAND_SERVER_START "start"
#define COMMAND_SERVER_STOP "stop"

#define COMMAND_UNAME "uname"
#define COMMAND_UPTIME "uptime"
#define COMMAND_HOSTNAME "hostname"

#define COMMAND_GAME "game"
#define COMMAND_GAME_2048 "2048"

#define COMMAND_PI "pi"

#define COMMAND_UUID "uuid"

#define COMMAND_ALGORITHM "algo"

#define COMMAND_HMAP "hmap"
#define COMMAND_HMAP_GET "get"
#define COMMAND_HMAP_SET "set"
#define COMMAND_HMAP_DEL "del"
#define COMMAND_HMAP_CAP "cap"
#define COMMAND_HMAP_LEN "len"
#define COMMAND_HMAP_PRINT "print"

#define COMMAND_LRU "lru"
#define COMMAND_LRU_LEN "len"
#define COMMAND_LRU_GET "get"
#define COMMAND_LRU_SET "set"
#define COMMAND_LRU_CAP "cap"
#define COMMAND_LRU_PRINT "print"

#define COMMAND_AVL "avl"
#define COMMAND_AVL_GET "get"
#define COMMAND_AVL_SET "set"
#define COMMAND_AVL_PRINT "print"
#define COMMAND_AVL_IN "in"
#define COMMAND_AVL_PRE "pre"
#define COMMAND_AVL_POST "post"
#define COMMAND_AVL_DUMP "dump"

#define COMMAND_SKLIST "sklist"
#define COMMAND_SKLIST_SET "set"
#define COMMAND_SKLIST_GET "get"
#define COMMAND_SKLIST_DEL "del"

#define ERR_COMMAND "invalid command"
#define ERR_COMMAND_NOT_FOUND "command not found"
#define ERR_INTERNAL "internal error"

#define MAP_COMMANDS_OK true
#define MAP_COMMANDS_ERROR false

#define command_length_check(x, y) \
  if (commands_length x y) {       \
    printf("%s\n", ERR_COMMAND);   \
    return MAP_COMMANDS_ERROR;     \
  }

void clear();
void algorithm_init();

void print_hex(const uint8_t *b, size_t len);
void completion(const char *buf, linenoiseCompletions *lc);
bool command_hash(commands_t commands, int commands_length);
bool command_base64(commands_t commands, int commands_length);
bool command_help(commands_t commands, int commands_length);
bool command_vi(commands_t commands, int commands_length);
bool command_tcp(commands_t commands, int commands_length);
bool command_server(commands_t commands, int commands_length);
bool command_uname(commands_t commands, int commands_length);
bool command_uptime(commands_t commands, int commands_length);
bool command_hostname(commands_t commands, int commands_length);
bool command_game(commands_t commands, int commands_length);
bool command_pi(commands_t commands, int commands_length);
bool command_uuid(commands_t commands, int commands_length);

bool command_algorithm(commands_t commands, int commands_length);
bool command_hmap(commands_t commands, int commands_length);
bool command_lru(commands_t commands, int commands_length);
bool command_avl(commands_t commands, int commands_length);
bool command_sklist(commands_t commands, int commands_length);
