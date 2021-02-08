#include <map.h>

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

int serve_dir_port = 0;
bool serve_dir_status = false;
char *serve_dir_root = NULL;

hashmap_t *hmap;
LRU *lru;
avl_entry_t *avl;

bool algorithm = false;

void clear() {
  if (serve_dir_status) {
    serve_dir_status = false;
    mg_usleep(2 * 1e6);
  }
  if (serve_dir_root != NULL) {
    free(serve_dir_root);
  }
  if (algorithm) {
    hashmap_free(hmap);
    LRU_free(lru);
    avl_free(avl);
  }
  printf("clear all, bye\n");
}

bool command_algorithm(commands_t commands, int commands_length) {
  algorithm_init();
  printf("Algorithm mode started");
  return MAP_COMMANDS_OK;
}

void algorithm_init() {
  hmap = hashmap_create();
  lru = LRU_create();
  algorithm = true;
}

bool command_avl(commands_t commands, int commands_length) {
  if (!algorithm) {
    algorithm_init();
  }
  command_length_check(<, 3);
  if (strncasecmp(commands[1], COMMAND_AVL_SET, strlen(COMMAND_AVL_SET)) == 0) {
    command_length_check(!=, 3);
    int key = atoi(commands[2]);
    avl = avl_set(avl, key);
  } else if (strncasecmp(commands[1], COMMAND_AVL_GET, strlen(COMMAND_AVL_GET)) == 0) {
    command_length_check(!=, 3);
    int key = atoi(commands[2]);
    printf("%s\n", avl_get(avl, key) ? "true" : "false");
  } else if (strncasecmp(commands[1], COMMAND_AVL_PRINT, strlen(COMMAND_AVL_PRINT)) == 0) {
    command_length_check(!=, 3);
    if (strncasecmp(commands[2], COMMAND_AVL_PRE, strlen(COMMAND_AVL_PRE)) == 0) {
      avl_pre_order(avl);
    } else if (strncasecmp(commands[2], COMMAND_AVL_IN, strlen(COMMAND_AVL_IN)) == 0) {
      avl_in_order(avl);
    } else if (strncasecmp(commands[2], COMMAND_AVL_POST, strlen(COMMAND_AVL_POST)) == 0) {
      avl_post_order(avl);
    }
  } else if (strncasecmp(commands[1], COMMAND_AVL_DUMP, strlen(COMMAND_AVL_DUMP)) == 0) {
    command_length_check(!=, 3);
    char *filename = commands[2];
    avl_dump(avl, filename);
  }
  return MAP_COMMANDS_OK;
}

bool command_lru(commands_t commands, int commands_length) {
  if (!algorithm) {
    algorithm_init();
  }
  command_length_check(<, 2);
  if (strncasecmp(commands[1], COMMAND_LRU_LEN, strlen(COMMAND_LRU_LEN)) == 0) {
    command_length_check(!=, 2);
    printf("%d\n", lru->len);
  } else if (strncasecmp(commands[1], COMMAND_LRU_SET, strlen(COMMAND_LRU_SET)) == 0) {
    command_length_check(!=, 4);
    char *key = commands[2];
    void *value = commands[3];
    LRU_set(lru, key, value, strlen(value) + 1);
  } else if (strncasecmp(commands[1], COMMAND_LRU_GET, strlen(COMMAND_LRU_GET)) == 0) {
    command_length_check(!=, 3);
    char *key = commands[2];
    size_t value_length = 0;
    char *value = (char *)LRU_get(lru, key, &value_length);
    if (value == NULL) {
      printf("key not found\n");
    } else {
      printf("%s\n", value);
    }
  } else if (strncasecmp(commands[1], COMMAND_LRU_CAP, strlen(COMMAND_LRU_CAP)) == 0) {
    command_length_check(<, 3);
    if (strncasecmp(commands[2], COMMAND_LRU_GET, strlen(COMMAND_LRU_GET)) == 0) {
      printf("%d\n", lru->cap);
    } else if (strncasecmp(commands[2], COMMAND_LRU_SET, strlen(COMMAND_LRU_SET)) == 0) {
      int cap = atoi(commands[3]);
      if (cap < lru->cap) {
        printf("please set more bigger cap\n");
      } else {
        lru->cap = cap;
      }
    }
  } else if (strncasecmp(commands[1], COMMAND_LRU_PRINT, strlen(COMMAND_LRU_PRINT)) == 0) {
    command_length_check(!=, 2);
    LRU_print(lru);
  }
  return MAP_COMMANDS_OK;
}

bool command_hmap(commands_t commands, int commands_length) {
  if (!algorithm) {
    algorithm_init();
  }
  command_length_check(<, 2);
  if (strncasecmp(commands[1], COMMAND_HMAP_CAP, strlen(COMMAND_HMAP_CAP)) == 0) {
    command_length_check(!=, 2);
    printf("%d\n", hmap->cap);
  } else if (strncasecmp(commands[1], COMMAND_HMAP_LEN, strlen(COMMAND_HMAP_LEN)) == 0) {
    command_length_check(!=, 2);
    printf("%d\n", hmap->len);
  } else if (strncasecmp(commands[1], COMMAND_HMAP_SET, strlen(COMMAND_HMAP_SET)) == 0) {
    command_length_check(!=, 4);
    char *key = commands[2];
    void *value = commands[3];
    hmap = hashmap_set(hmap, key, value, strlen(value) + 1);
  } else if (strncasecmp(commands[1], COMMAND_HMAP_GET, strlen(COMMAND_HMAP_GET)) == 0) {
    command_length_check(!=, 3);
    char *key = commands[2];
    size_t value_length = 0;
    char *value = (char *)hashmap_get(hmap, key, &value_length);
    if (value == NULL) {
      printf("key not found\n");
    } else {
      printf("%s\n", value);
    }
  } else if (strncasecmp(commands[1], COMMAND_HMAP_DEL, strlen(COMMAND_HMAP_DEL)) == 0) {
    command_length_check(!=, 3);
    char *key = commands[2];
    hashmap_del(hmap, key);
  } else if (strncasecmp(commands[1], COMMAND_HMAP_PRINT, strlen(COMMAND_HMAP_PRINT)) == 0) {
    command_length_check(!=, 2);
    hashmap_print(hmap);
  }
  return MAP_COMMANDS_OK;
}

bool command_uname(commands_t commands, int commands_length) {
  struct utsname name;
  uname(&name);
  printf("%s %s\n", name.sysname, name.machine);
  return MAP_COMMANDS_OK;
}

bool command_uptime(commands_t commands, int commands_length) {
  uint64_t uptime = getUptime();
  bool is_days = false;
  if (uptime > 24 * 3600 * 1000) {
    printf("%" PRIu64 " days", uptime / (24 * 3600 * 1000));
    uptime = uptime % (24 * 3600 * 1000);
    is_days = true;
  }
  bool is_hours = false;
  if (uptime > 3600 * 1000 || is_days) {
    printf(" %02" PRIu64 " hours", uptime / (3600 * 1000));
    uptime = uptime % (3600 * 1000);
    is_hours = true;
  }
  bool is_minutes = false;
  if (uptime > 60 * 1000 || is_days || is_hours) {
    printf(" %02" PRIu64 " minutes", uptime / (60 * 1000));
    uptime = uptime % (60 * 1000);
    is_minutes = true;
  }
  if (uptime > 1000 || is_days || is_hours || is_minutes) {
    printf(" %02" PRIu64 " seconds", uptime / (1000));
    uptime = uptime % (1000);
  }
  printf(" %03" PRIu64 " millseconds\n", uptime);
  return MAP_COMMANDS_OK;
}

bool command_hostname(commands_t commands, int commands_length) {
  char name[_POSIX_HOST_NAME_MAX + 1];
  if (gethostname(name, sizeof name) == -1) {
    printf("cannot get hostname\n");
  }
  printf("%s\n", name);
  return MAP_COMMANDS_OK;
}

bool command_game(commands_t commands, int commands_length) {
  command_length_check(<, 2);
  if (strncasecmp(commands[1], COMMAND_GAME_2048, strlen(COMMAND_GAME_2048)) == 0) {
    game2048(0, NULL);
  }
  return MAP_COMMANDS_OK;
}

bool command_pi(commands_t commands, int commands_length) {
  int length = 100;
  if (commands_length == 2) {
    length = atoi(commands[1]);
  }
  if (length < 100) {
    length = 100;
  }
  pi(length);
  return MAP_COMMANDS_OK;
}

static void serve_dir_cb(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_HTTP_MSG) mg_http_serve_dir(c, ev_data, serve_dir_root);
}

void *serve_dir(void *a) {
  struct mg_mgr mgr;
  mg_mgr_init(&mgr);
  char address[50] = {0};
  sprintf(address, "http://0.0.0.0:%d", serve_dir_port);
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

bool command_server(commands_t commands, int commands_length) {
  command_length_check(<, 2);
  if (strncasecmp(commands[1], COMMAND_SERVER_START, strlen(COMMAND_SERVER_START)) == 0) {
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
    serve_dir_root = strdup(commands[2]);
    serve_dir_port = port;
    pthread_create(&tid, NULL, serve_dir, NULL);
  } else if (strncasecmp(commands[1], COMMAND_SERVER_STOP, strlen(COMMAND_SERVER_STOP)) == 0) {
    if (!serve_dir_status) {
      printf("server stopped already\n");
      return MAP_COMMANDS_OK;
    }
    serve_dir_status = false;
    free(serve_dir_root);
    serve_dir_root = NULL;
  }
  return MAP_COMMANDS_OK;
}

bool command_tcp(commands_t commands, int commands_length) {
  command_length_check(!=, 3);
  int port = atoi(commands[2]);
  if (port <= 0) {
    printf("port is invalid\n");
    return MAP_COMMANDS_OK;
  }
  tcp_check(commands[1], port);
  return MAP_COMMANDS_OK;
}

bool command_vi(commands_t commands, int commands_length) {
  command_length_check(!=, 2);
  char *filename = commands[1];

  initEditor();
  editorSelectSyntaxHighlight(filename);
  editorOpen(filename);
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

bool command_base64(commands_t commands, int commands_length) {
  command_length_check(!=, 3);

  char *string = commands[2];
  char *instring = NULL;
  size_t instring_length = 0;

  bool file_exist = false;

  FILE *f = NULL;

  struct stat file_handler;
  if (stat(string, &file_handler) == 0) {
    file_exist = true;
    f = fopen(string, "rb");
    unsigned char buf[1024];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
      if (instring_length == 0) {
        instring = (char *)malloc(n * sizeof(char));
        instring_length += n;
        memcpy(instring, buf, n);
      } else {
        instring = (char *)realloc(instring, (n + instring_length) * sizeof(char));
        memcpy(instring + instring_length, buf, n);
        instring_length += n;
      }
    }
    fclose(f);
  }

  if (!file_exist) {
    instring = string;
  }

  if (strncasecmp(commands[1], COMMAND_BASE64_ENCODE, strlen(COMMAND_BASE64_ENCODE)) == 0) {
    size_t olen = 0;
    mbedtls_base64_encode(NULL, 0, &olen, (unsigned char *)instring, strlen(instring));
    char *outstring = (char *)calloc(olen, sizeof(char));
    if (mbedtls_base64_encode((unsigned char *)outstring, olen, &olen, (unsigned char *)instring, strlen(instring)) != 0) {
      map_err(ERR_INTERNAL, "base64 encode with error");
    } else {
      printf("%s\n", outstring);
    }
    free(outstring);
  } else if (strncasecmp(commands[1], COMMAND_BASE64_DECODE, strlen(COMMAND_BASE64_DECODE)) == 0) {
    size_t olen = 0;
    mbedtls_base64_decode(NULL, 0, &olen, (unsigned char *)instring, strlen(instring));
    char *outstring = (char *)calloc(olen, sizeof(char));
    if (mbedtls_base64_decode((unsigned char *)outstring, olen, &olen, (unsigned char *)instring, strlen(instring)) != 0) {
      map_err(ERR_INTERNAL, "base64 encode with error");
    } else {
      printf("%s\n", outstring);
    }
    free(outstring);
  } else {
    printf("%s\n", ERR_COMMAND_NOT_FOUND);
    return MAP_COMMANDS_OK;
  }

  if (file_exist) {
    free(instring);
  }

  return MAP_COMMANDS_OK;
}

void print_hex(const uint8_t *b, size_t len) {
  const uint8_t *end = b + len;
  while (b < end) {
    printf("%02x", (uint8_t)*b++);
  }
  printf("\n");
}

bool command_hash(commands_t commands, int commands_length) {
  command_length_check(!=, 3);
  char *hash_name = commands[1];
  char *string = commands[2];
  const mbedtls_md_info_t *md_info = NULL;
  if (strncasecmp(hash_name, COMMAND_HASH_MD5, strlen(COMMAND_HASH_MD5)) == 0) {
    md_info = mbedtls_md_info_from_type(MBEDTLS_MD_MD5);
  } else if (strncasecmp(hash_name, COMMAND_HASH_SHA1, strlen(COMMAND_HASH_SHA1)) == 0) {
    md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
  } else if (strncasecmp(hash_name, COMMAND_HASH_SHA256, strlen(COMMAND_HASH_SHA256)) == 0) {
    md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
  } else if (strncasecmp(hash_name, COMMAND_HASH_SHA512, strlen(COMMAND_HASH_SHA512)) == 0) {
    md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);
  }
  int hash_size = (size_t)mbedtls_md_get_size(md_info);
  unsigned char *outbyte = (unsigned char *)calloc(hash_size, sizeof(unsigned char));

  struct stat file_handler;
  if (stat(string, &file_handler) == 0) {
    if (mbedtls_md_file(md_info, string, outbyte) != 0) {
      map_err(ERR_INTERNAL, "hash file with error");
    } else {
      print_hex((uint8_t *)outbyte, hash_size);
    }
  } else {
    if (mbedtls_md(md_info, (unsigned char *)string, strlen(string), outbyte) != 0) {
      map_err(ERR_INTERNAL, "hash string with error");
    } else {
      print_hex((uint8_t *)outbyte, hash_size);
    }
  }
  free(outbyte);
  return MAP_COMMANDS_OK;
}

bool command_uuid(commands_t commands, int commands_length) {
  char buf[UUID4_LEN];
  if (uuid4_init() == UUID4_EFAILURE) {
    printf("uuid init error\n");
    return MAP_COMMANDS_OK;
  }
  uuid4_generate(buf);
  printf("%s\n", buf);
  return MAP_COMMANDS_OK;
}

void completion(const char *buf, linenoiseCompletions *lc) {
  if (buf[0] == 'b') {
    linenoiseAddCompletion(lc, "base64");
  } else if (buf[0] == 'h') {
    linenoiseAddCompletion(lc, "help");
  }
}

bool command_help(commands_t commands, int commands_length) {
  printf("Base64\n");
  printf("  \033[0;32mbase64\033[0m <enc/dec> <string/filename>\n");
  printf("Hash\n");
  printf("  \033[0;32mhash\033[0m <method> <string/filename>      "
         "\033[1;33msupport hash methods: md5 sha1 sha256 sha512\033[0m\n");
  printf("Editor\n");
  printf("  \033[0;32mvi\033[0m <filename>\n");
  printf("Network\n");
  printf("  \033[0;32mtcp\033[0m <ip> <port>\n");
  printf("  \033[0;32mserver\033[0m <start/stop> <dir> <port>\n");
  printf("System\n");
  printf("  \033[0;32muname\033[0m\n");
  printf("  \033[0;32muptime\033[0m\n");
  printf("  \033[0;32mhostname\033[0m\n");
  printf("Game\n");
  printf("  \033[0;32mgame\033[0m <name>                          "
         "\033[1;33msupport game: 2048\033[0m\n");
  printf("Math\n");
  printf("  \033[0;32mpi\033[0m <length>\n");
  printf("Random\n");
  printf("  \033[0;32muuid\033[0m\n");
  printf("Map\n");
  printf("  \033[0;32mhelp\033[0m\n");
  printf("  \033[0;32mexit\033[0m\n");
  printf("  \033[0;32mversion\033[0m\n");
  return MAP_COMMANDS_OK;
}
