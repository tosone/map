#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/stat.h>
#include <sys/utsname.h>

#include <kilo.h>
#include <linenoise.h>
#include <mbedtls/base64.h>
#include <mbedtls/md.h>
#include <mongoose.h>

#include <2048.h>
#include <command.h>
#include <tcp.h>

#pragma once

#define COMMAND_VERSION "version"
#define COMMAND_HELP "help"
#define COMMAND_EXIT "exit"
#define COMMAND_UNAME "uname"
#define COMMAND_VI "vi"
#define COMMAND_TCP "tcp"

#define COMMAND_HASH "hash"
#define COMMAND_HASH_MD5 "md5"
#define COMMAND_HASH_SHA1 "sha1"
#define COMMAND_HASH_SHA256 "sha256"
#define COMMAND_HASH_SHA512 "sha512"

#define COMMAND_BASE64 "base64"
#define COMMAND_BASE64_DECODE "dec"
#define COMMAND_BASE64_ENCODE "enc"

#define COMMAND_SERVER "server"
#define COMMAND_SERVER_START "start"
#define COMMAND_SERVER_STOP "stop"

#define COMMAND_GAME "game"
#define COMMAND_GAME_2048 "2048"

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

void print_hex(const uint8_t *b, size_t len);
void completion(const char *buf, linenoiseCompletions *lc);
bool hash_command(commands_t commands, int commands_length);
bool base_command(commands_t commands, int commands_length);
bool help_command(commands_t commands, int commands_length);
bool vi_command(commands_t commands, int commands_length);
bool tcp_command(commands_t commands, int commands_length);
bool server_command(commands_t commands, int commands_length);
bool uname_command(commands_t commands, int commands_length);
bool game_command(commands_t commands, int commands_length);
