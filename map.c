#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/stat.h>

#include <mbedtls/base64.h>
#include <mongoose.h>
#include <tomcrypt.h>

#include <command.h>
#include <kilo.h>
#include <linenoise.h>
#include <tcp.h>

#define VERSION "v1.0.0"

#define VERSION_COMMAND "version"
#define HELP_COMMAND "help"
#define HASH_COMMAND "hash"

#define BASE64_COMMAND "base64"
#define BASE32_COMMAND "base32"
#define BASE16_COMMAND "base16"
#define BASE_COMMAND_DECODE "dec"
#define BASE_COMMAND_ENCODE "enc"

#define VI_COMMAND "vi"

#define SERVER_COMMAND "server"
#define SERVER_COMMAND_START "start"
#define SERVER_COMMAND_STOP "stop"

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

bool hash_command(commands_t commands, int commands_length);

bool base_command(commands_t commands, int commands_length);
void base64_decode_command(commands_t commands);
void base64_encode_command(commands_t commands);

bool help_command(commands_t commands, int commands_length);

bool vi_command(commands_t commands, int commands_length);

bool tcp_command(commands_t commands, int commands_length);

bool server_command(commands_t commands, int commands_length);

#define COMMANDS_CHECK(x)                     \
  if (x) {                                    \
    commands_free(commands, commands_length); \
    continue;                                 \
  }

struct server_dir_t {
  char *dir;
  int port;
};

struct server_dir_t *serve_dir_params;
bool serve_dir_status = false;
char *serve_dir_root = NULL;

static void serve_dir_cb(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_HTTP_MSG) mg_http_serve_dir(c, ev_data, serve_dir_root);
}

void *serve_dir(void *params) {
  struct server_dir_t *_params = (struct server_dir_t *)params;
  struct mg_mgr mgr;
  mg_mgr_init(&mgr);
  char address[50] = {0};
  sprintf(address, "http://0.0.0.0:%d", _params->port);
  serve_dir_root = _params->dir;
  mg_http_listen(&mgr, address, serve_dir_cb, &mgr);
  while (true) {
    mg_mgr_poll(&mgr, 1000);
    if (!serve_dir_status) {
      break;
    }
  }
  mg_mgr_free(&mgr);
  pthread_exit(NULL);
  return NULL;
}

void clear() {
  if (serve_dir_status) {
    serve_dir_status = false;
    mg_usleep(2 * 1e6);
  }
  if (serve_dir_root != NULL) {
    free(serve_dir_root);
  }
  if (serve_dir_params != NULL) {
    free(serve_dir_params);
  }
  printf("clear all, bye\n");
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
        } else if (strncasecmp(commands[0], BASE64_COMMAND, strlen(BASE64_COMMAND)) == 0 ||
                   strncasecmp(commands[0], BASE32_COMMAND, strlen(BASE32_COMMAND)) == 0 ||
                   strncasecmp(commands[0], BASE16_COMMAND, strlen(BASE16_COMMAND)) == 0) {
          COMMANDS_CHECK(!base_command(commands, commands_length));
        } else if (strncasecmp(commands[0], HASH_COMMAND, strlen(HASH_COMMAND)) == 0) {
          COMMANDS_CHECK(!hash_command(commands, commands_length));
        } else if (strncasecmp(commands[0], VI_COMMAND, strlen(VI_COMMAND)) == 0) {
          COMMANDS_CHECK(!vi_command(commands, commands_length));
        } else if (strncasecmp(commands[0], TCP_COMMAND, strlen(TCP_COMMAND)) == 0) {
          COMMANDS_CHECK(!tcp_command(commands, commands_length));
        } else if (strncasecmp(commands[0], SERVER_COMMAND, strlen(SERVER_COMMAND)) == 0) {
          COMMANDS_CHECK(!server_command(commands, commands_length));
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

bool server_command(commands_t commands, int commands_length) {
  command_length_check(<, 2);
  if (strncasecmp(commands[1], SERVER_COMMAND_START, strlen(SERVER_COMMAND_START)) == 0) {
    if (serve_dir_status) {
      printf("server started already\n");
      return MAP_COMMANDS_OK;
    }
    command_length_check(<, 3);
    int port = 8000;
    if (commands_length == 4) {
      port = atoi(commands[3]);
    }
    if (port == 0) {
      port = 8000;
    }
    serve_dir_status = true;
    pthread_t tid;
    serve_dir_params = (struct server_dir_t *)malloc(sizeof(struct server_dir_t));
    serve_dir_params->dir = strdup(commands[2]);
    serve_dir_params->port = port;
    pthread_create(&tid, NULL, serve_dir, (void *)serve_dir_params);
  } else if (strncasecmp(commands[1], SERVER_COMMAND_STOP, strlen(SERVER_COMMAND_STOP)) == 0) {
    if (!serve_dir_status) {
      printf("server stopped already\n");
      return MAP_COMMANDS_OK;
    }
    serve_dir_status = false;
    free(serve_dir_root);
    serve_dir_root = NULL;
    free(serve_dir_params);
    serve_dir_params = NULL;
  }
  return MAP_COMMANDS_OK;
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
  command_length_check(!=, 3);
  if (strncasecmp(commands[1], BASE_COMMAND_ENCODE, strlen(BASE_COMMAND_ENCODE)) == 0) {
    base64_encode_command(commands);
  } else if (strncasecmp(commands[1], BASE_COMMAND_DECODE, strlen(BASE_COMMAND_DECODE)) == 0) {
    base64_decode_command(commands);
  } else {
    printf("%s\n", ERR_COMMAND_NOT_FOUND);
    return MAP_COMMANDS_OK;
  }
  return MAP_COMMANDS_OK;
}

void base64_decode_command(commands_t commands) {
  char *string = commands[2];
  size_t olen = 0;
  mbedtls_base64_decode(NULL, 0, &olen, (unsigned char *)string, strlen(string));
  char *outstring = (char *)calloc(olen, sizeof(char));
  if (mbedtls_base64_decode((unsigned char *)outstring, olen, &olen, (unsigned char *)string, strlen(string)) != 0) {
    map_err(ERR_INTERNAL, "base64 encode with error");
  } else {
    printf("%s\n", outstring);
  }
  free(outstring);
}

void base64_encode_command(commands_t commands) {
  char *string = commands[2];
  size_t olen = 0;
  mbedtls_base64_encode(NULL, 0, &olen, (unsigned char *)string, strlen(string));
  char *outstring = (char *)calloc(olen, sizeof(char));
  if (mbedtls_base64_encode((unsigned char *)outstring, olen, &olen, (unsigned char *)string, strlen(string)) != 0) {
    map_err(ERR_INTERNAL, "base64 encode with error");
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

void completion(const char *buf, linenoiseCompletions *lc) {
  if (buf[0] == 'b') {
    linenoiseAddCompletion(lc, "base64");
  } else if (buf[0] == 'h') {
    linenoiseAddCompletion(lc, "help");
  }
}

char *hints(const char *buf, int *color, int *bold) {
  *color = 32;
  *bold = 0;
  if (strcmp(buf, "base64") == 0) {
    return " <enc/dec> <string>";
  } else if (strcmp(buf, "hash") == 0) {
    return " <method> <string>";
  } else if (strcmp(buf, "prng") == 0) {
    return " <method> <string>";
  } else if (strcmp(buf, "vi") == 0) {
    return " <filename>";
  } else if (strcmp(buf, "tcp") == 0) {
    return " <hostname> <port>";
  } else if (strcmp(buf, "server") == 0) {
    return " <start/stop> <dir> <port>";
  }
  return NULL;
}

bool help_command(commands_t commands, int commands_length) {
  printf("Base64\n");
  printf("  \033[0;32mbase64\033[0m <enc/dec> <string>\n");
  printf("Hash\n");
  printf("  \033[0;32mhash\033[0m <method> <string>              "
         "\033[1;33msupport Hash methods: md5 sha1 sha256 sha512\033[0m\n");
  printf("PRNG\n");
  printf("  \033[0;32mprng\033[0m <method> <string> <length>     "
         "\033[1;33msupport PRNG methods: yarrow rc4 chacha20\033[0m\n");
  printf("Editor\n");
  printf("  \033[0;32mvi\033[0m <filename>\n");
  printf("Network\n");
  printf("  \033[0;32mtcp\033[0m <hostname> <port>\n");
  printf("  \033[0;32mserver\033[0m <start/stop> <dir> <port>\n");
  return MAP_COMMANDS_OK;
}
