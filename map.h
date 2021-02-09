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

#include <cmddefine.h>

#include <2048.h>
#include <command.h>
#include <pi.h>
#include <tcp.h>

#include <avl.h>
#include <hashmap.h>
#include <lru.h>
#include <skiplist.h>

#pragma once

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
