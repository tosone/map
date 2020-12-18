#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/stat.h>

#include <mongoose.h>
#include <tomcrypt.h>

#include <command.h>
#include <kilo.h>
#include <linenoise.h>
#include <tcp.h>

#define VERSION "v1.0.0"

#define VERSION_COMMAND "version"
#define HELP_COMMAND "help"
#define PRNG_COMMAND "prng"
#define HASH_COMMAND "hash"

#define BASE_COMMAND "base"
#define BASE_COMMAND_DECODE "dec"
#define BASE_COMMAND_ENCODE "enc"

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

bool base_command(commands_t commands, int commands_length);

void base16_encode_command(commands_t commands);
void base16_decode_command(commands_t commands);
void base32_encode_command(commands_t commands);
void base32_decode_command(commands_t commands);
void base64_decode_command(commands_t commands);
void base64_encode_command(commands_t commands);

bool help_command(commands_t commands, int commands_length);

bool vi_command(commands_t commands, int commands_length);

bool tcp_command(commands_t commands, int commands_length);

#define COMMANDS_CHECK(x)                     \
  if (x) {                                    \
    commands_free(commands, commands_length); \
    continue;                                 \
  }

void clear() {
  printf("clear all, bye\n");
}

static const char *s_web_root_dir = "./";
static const char *s_listening_address = "http://localhost:8000";

static void cb(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_HTTP_MSG) mg_http_serve_dir(c, ev_data, s_web_root_dir);
}

int main(int argc, char **argv) {
  linenoiseSetCompletionCallback(completion);
  linenoiseSetHintsCallback(hints);
  linenoiseHistoryLoad("history.txt");
  linenoiseHistorySetMaxLen(1000);

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
        } else if (strncasecmp(commands[0], BASE_COMMAND, strlen(BASE_COMMAND)) == 0) {
          COMMANDS_CHECK(!base_command(commands, commands_length));
        } else if (strncasecmp(commands[0], HASH_COMMAND, strlen(HASH_COMMAND)) == 0) {
          COMMANDS_CHECK(!hash_command(commands, commands_length));
        } else if (strncasecmp(commands[0], PRNG_COMMAND, strlen(PRNG_COMMAND)) == 0) {
          COMMANDS_CHECK(!prng_command(commands, commands_length));
        } else if (strncasecmp(commands[0], VI_COMMAND, strlen(VI_COMMAND)) == 0) {
          COMMANDS_CHECK(!vi_command(commands, commands_length));
        } else if (strncasecmp(commands[0], TCP_COMMAND, strlen(TCP_COMMAND)) == 0) {
          COMMANDS_CHECK(!tcp_command(commands, commands_length));
        } else if (strncasecmp(commands[0], "server", strlen("server")) == 0) {
          struct mg_mgr mgr;
          mg_mgr_init(&mgr);
          mg_http_listen(&mgr, s_listening_address, cb, &mgr);
          for (;;) mg_mgr_poll(&mgr, 1000);
          mg_mgr_free(&mgr);
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

bool tcp_command(commands_t commands, int commands_length) {
  command_length_check(!=, 3);
  int port = atoi(commands[2]);
  if (port <= 0) {
    printf("port is invalid\n");
    return MAP_COMMANDS_OK;
  }
  tcp_check(commands[1], port);
  return MAP_COMMANDS_OK;
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

bool base_command(commands_t commands, int commands_length) {
  command_length_check(!=, 4);
  int basex = atoi(commands[1]);
  if (basex == 64) {
    if (strncasecmp(commands[2], BASE_COMMAND_ENCODE, strlen(BASE_COMMAND_ENCODE)) == 0) {
      base64_encode_command(commands);
    } else if (strncasecmp(commands[2], BASE_COMMAND_DECODE, strlen(BASE_COMMAND_DECODE)) == 0) {
      base64_decode_command(commands);
    } else {
      printf("%s\n", ERR_COMMAND_NOT_FOUND);
    }
  } else if (basex == 32) {
    if (strncasecmp(commands[2], BASE_COMMAND_ENCODE, strlen(BASE_COMMAND_ENCODE)) == 0) {
      base32_encode_command(commands);
    } else if (strncasecmp(commands[2], BASE_COMMAND_DECODE, strlen(BASE_COMMAND_DECODE)) == 0) {
      base32_decode_command(commands);
    } else {
      printf("%s\n", ERR_COMMAND_NOT_FOUND);
    }
  } else if (basex == 16) {
    if (strncasecmp(commands[1], BASE_COMMAND_ENCODE, strlen(BASE_COMMAND_ENCODE)) == 0) {
      base16_encode_command(commands);
    } else if (strncasecmp(commands[1], BASE_COMMAND_DECODE, strlen(BASE_COMMAND_DECODE)) == 0) {
      base16_decode_command(commands);
    } else {
      printf("%s\n", ERR_COMMAND_NOT_FOUND);
    }
  } else {
    printf("%s\n", ERR_COMMAND_NOT_FOUND);
  }
  return MAP_COMMANDS_OK;
}

void base64_decode_command(commands_t commands) {
  char *string = commands[3];
  unsigned char *outstring = (unsigned char *)calloc(strlen(string) + 1, sizeof(unsigned char));
  unsigned long out = sizeof(outstring);
  if (base64_decode(string, strlen(string) + 1, outstring, &out)) {
    map_err(ERR_INTERNAL, "base64 decode with error");
  } else {
    printf("%s\n", outstring);
  }
}

void base64_encode_command(commands_t commands) {
  char *string = commands[3];
  unsigned long out = 4 * ((strlen(string) + 2) / 3) + 1;
  char *outstring = (char *)calloc(out, sizeof(char));
  if (base64_encode((unsigned char *)string, strlen(string), outstring, &out) != CRYPT_OK) {
    map_err(ERR_INTERNAL, "base64 encode with error");
  } else {
    printf("%s\n", outstring);
  }
}

void base32_encode_command(commands_t commands) {
  char *string = commands[3];
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
  char *string = commands[3];
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
  char *string = commands[3];
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
  char *string = commands[3];
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
  if (buf[0] == 'b') {
    linenoiseAddCompletion(lc, "base");
  } else if (strcasecmp(buf, "ba")) {
    linenoiseAddCompletion(lc, "base");
  } else if (strcasecmp(buf, "bas")) {
    linenoiseAddCompletion(lc, "base");
  } else if (buf[0] == 'h') {
    linenoiseAddCompletion(lc, "help");
  } else if (buf[0] == 'a') {
    linenoiseAddCompletion(lc, "avl");
  } else if (buf[0] == 'b') {
    linenoiseAddCompletion(lc, "base64");
  }
}

char *hints(const char *buf, int *color, int *bold) {
  *color = 32;
  *bold = 0;
  if (strcmp(buf, "base") == 0) {
    return " <64/32/16> <enc/dec> <string>";
  } else if (strcmp(buf, "hash") == 0) {
    return " <method> <string>";
  } else if (strcmp(buf, "prng") == 0) {
    return " <method> <string>";
  } else if (strcmp(buf, "vi") == 0) {
    return " <filename>";
  } else if (strcmp(buf, "tcp") == 0) {
    return " <hostname> <port>";
  }
  return NULL;
}

bool help_command(commands_t commands, int commands_length) {
  printf("BaseX\n");
  printf("    \033[0;32mbase\033[0m <64/32/16> <enc/dec> <string>  "
         "\033[1;33msupport BaseX method: base64 base32 base16\033[0m\n");
  printf("Hash\n");
  printf("    \033[0;32mhash\033[0m <method> <string>              "
         "\033[1;33msupport Hash method: md5 sha1 sha256 sha512\033[0m\n");
  printf("PRNG\n");
  printf("    \033[0;32mprng\033[0m <method> <string> <length>     "
         "\033[1;33msupport PRNG methods: yarrow rc4 chacha20\033[0m\n");
  printf("Editor\n");
  printf("    \033[0;32mvi\033[0m <filename>\n");
  printf("Network\n");
  printf("    \033[0;32mtcp\033[0m <hostname> <port>\n");
  return MAP_COMMANDS_OK;
}
