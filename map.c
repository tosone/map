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
skiplist *sklist;

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
    slFree(sklist);
  }
  printf("clear all, bye\n");
}

bool command_algorithm(commands_t commands, int commands_length) {
  algorithm_init();
  printf("Algorithm mode started\n");
  return MAP_COMMANDS_OK;
}

void algorithm_init() {
  hmap = hashmap_create();
  lru = LRU_create();
  sklist = slCreate();

  slInsert(sklist, 3.0, sdsnew("test"));
  slInsert(sklist, 3.1, sdsnew("test1")); // 2
  slInsert(sklist, 3.2, sdsnew("test2")); // 3
  slInsert(sklist, 3.3, sdsnew("test3")); // 4
  slInsert(sklist, 3.3, sdsnew("test4")); // 5
  slInsert(sklist, 3.4, sdsnew("test4")); // 6
  slInsert(sklist, 3.4, sdsnew("test4")); // 7
  slInsert(sklist, 3.4, sdsnew("test4")); // 8
  slInsert(sklist, 3.4, sdsnew("test4")); // 9
  slInsert(sklist, 3.4, sdsnew("test4")); // 10
  slInsert(sklist, 3.4, sdsnew("test4")); // 11
  printf("rank test: %lu\n", slGetRank(sklist, 3.0, sdsnew("test")));
  printf("rank test1: %lu\n", slGetRank(sklist, 3.1, sdsnew("test1")));
  printf("rank test2: %lu\n", slGetRank(sklist, 3.2, sdsnew("test2")));
  printf("rank test3: %lu\n", slGetRank(sklist, 3.3, sdsnew("test3")));
  printf("rank test4: %lu\n", slGetRank(sklist, 3.3, sdsnew("test4")));
  printf("rank rake test4: %lu\n", slGetRank(sklist, 3.4, sdsnew("test4")));
  algorithm = true;
}

bool command_sklist(commands_t commands, int commands_length) {
  if (!algorithm) {
    algorithm_init();
  }
  command_length_check( <, 3);
  if (strncasecmp(commands[1], COMMAND_SKLIST_SET, strlen(COMMAND_SKLIST_SET)) == 0) {
    command_length_check( !=, 4);
    double score = strtod(commands[2], NULL);
    sds ele = sdsnew(commands[3]);
    slInsert(sklist, score, ele);
  } else if (strncasecmp(commands[1], COMMAND_SKLIST_GET, strlen(COMMAND_SKLIST_GET)) == 0) {
    command_length_check( !=, 4);
    double score = strtod(commands[2], NULL);
    sds ele = sdsnew(commands[3]);
    unsigned long rank = slGetRank(sklist, score, ele);
    sdsfree(ele);
    skiplistNode *node = sklist->header->level->forward;
    while (node != NULL) {
      printf("%s\n", node->ele);
      node = node->level->forward;
    }
    if (rank == 0) {
      printf("cannot find this element\n");
    } else {
      printf("%lu\n", rank);
    }
  } else if (strncasecmp(commands[1], COMMAND_SKLIST_DEL, strlen(COMMAND_SKLIST_DEL)) == 0) {
    command_length_check( !=, 4);
    double score = strtod(commands[2], NULL);
    sds ele = sdsnew(commands[3]);
    int ret = slDelete(sklist, score, ele, NULL);
    sdsfree(ele);
    if (ret == 0) {
      printf("cannot find this element");
    }
  }
  return MAP_COMMANDS_OK;
}

bool command_avl(commands_t commands, int commands_length) {
  if (!algorithm) {
    algorithm_init();
  }
  command_length_check( <, 3);
  if (strncasecmp(commands[1], COMMAND_AVL_SET, strlen(COMMAND_AVL_SET)) == 0) {
    command_length_check( !=, 3);
    int key = atoi(commands[2]);
    avl = avl_set(avl, key);
  } else if (strncasecmp(commands[1], COMMAND_AVL_GET, strlen(COMMAND_AVL_GET)) == 0) {
    command_length_check( !=, 3);
    int key = atoi(commands[2]);
    printf("%s\n", avl_get(avl, key) ? "true" : "false");
  } else if (strncasecmp(commands[1], COMMAND_AVL_PRINT, strlen(COMMAND_AVL_PRINT)) == 0) {
    command_length_check( !=, 3);
    if (strncasecmp(commands[2], COMMAND_AVL_PRE, strlen(COMMAND_AVL_PRE)) == 0) {
      avl_pre_order(avl);
    } else if (strncasecmp(commands[2], COMMAND_AVL_IN, strlen(COMMAND_AVL_IN)) == 0) {
      avl_in_order(avl);
    } else if (strncasecmp(commands[2], COMMAND_AVL_POST, strlen(COMMAND_AVL_POST)) == 0) {
      avl_post_order(avl);
    }
  } else if (strncasecmp(commands[1], COMMAND_AVL_DUMP, strlen(COMMAND_AVL_DUMP)) == 0) {
    command_length_check( !=, 3);
    char *filename = commands[2];
    avl_dump(avl, filename);
  }
  return MAP_COMMANDS_OK;
}

bool command_lru(commands_t commands, int commands_length) {
  if (!algorithm) {
    algorithm_init();
  }
  command_length_check( <, 2);
  if (strncasecmp(commands[1], COMMAND_LRU_LEN, strlen(COMMAND_LRU_LEN)) == 0) {
    command_length_check( !=, 2);
    printf("%d\n", lru->len);
  } else if (strncasecmp(commands[1], COMMAND_LRU_SET, strlen(COMMAND_LRU_SET)) == 0) {
    command_length_check( !=, 4);
    char *key = commands[2];
    void *value = commands[3];
    LRU_set(lru, key, value, strlen(value) + 1);
  } else if (strncasecmp(commands[1], COMMAND_LRU_GET, strlen(COMMAND_LRU_GET)) == 0) {
    command_length_check( !=, 3);
    char *key = commands[2];
    size_t value_length = 0;
    char *value = (char *)LRU_get(lru, key, &value_length);
    if (value == NULL) {
      printf("key not found\n");
    } else {
      printf("%s\n", value);
    }
  } else if (strncasecmp(commands[1], COMMAND_LRU_CAP, strlen(COMMAND_LRU_CAP)) == 0) {
    command_length_check( <, 3);
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
    command_length_check( !=, 2);
    LRU_print(lru);
  }
  return MAP_COMMANDS_OK;
}

bool command_hmap(commands_t commands, int commands_length) {
  if (!algorithm) {
    algorithm_init();
  }
  command_length_check( <, 2);
  if (strncasecmp(commands[1], COMMAND_HMAP_CAP, strlen(COMMAND_HMAP_CAP)) == 0) {
    command_length_check( !=, 2);
    printf("%d\n", hmap->cap);
  } else if (strncasecmp(commands[1], COMMAND_HMAP_LEN, strlen(COMMAND_HMAP_LEN)) == 0) {
    command_length_check( !=, 2);
    printf("%d\n", hmap->len);
  } else if (strncasecmp(commands[1], COMMAND_HMAP_SET, strlen(COMMAND_HMAP_SET)) == 0) {
    command_length_check( !=, 4);
    char *key = commands[2];
    void *value = commands[3];
    hmap = hashmap_set(hmap, key, value, strlen(value) + 1);
  } else if (strncasecmp(commands[1], COMMAND_HMAP_GET, strlen(COMMAND_HMAP_GET)) == 0) {
    command_length_check( !=, 3);
    char *key = commands[2];
    size_t value_length = 0;
    char *value = (char *)hashmap_get(hmap, key, &value_length);
    if (value == NULL) {
      printf("key not found\n");
    } else {
      printf("%s\n", value);
    }
  } else if (strncasecmp(commands[1], COMMAND_HMAP_DEL, strlen(COMMAND_HMAP_DEL)) == 0) {
    command_length_check( !=, 3);
    char *key = commands[2];
    hashmap_del(hmap, key);
  } else if (strncasecmp(commands[1], COMMAND_HMAP_PRINT, strlen(COMMAND_HMAP_PRINT)) == 0) {
    command_length_check( !=, 2);
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
  command_length_check( <, 2);
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
  command_length_check( <, 2);
  if (strncasecmp(commands[1], COMMAND_SERVER_START, strlen(COMMAND_SERVER_START)) == 0) {
    if (serve_dir_status) {
      printf("server started already\n");
      return MAP_COMMANDS_OK;
    }
    command_length_check( <, 3);
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
  command_length_check( !=, 3);
  int port = atoi(commands[2]);
  if (port <= 0) {
    printf("port is invalid\n");
    return MAP_COMMANDS_OK;
  }
  tcp_check(commands[1], port);
  return MAP_COMMANDS_OK;
}

bool command_vi(commands_t commands, int commands_length) {
  command_length_check( !=, 2);
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
  command_length_check( !=, 3);

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
      if (file_exist) {
        printf("base64 file: %s\n", string);
      } else {
        printf("base64 string: %s\n", string);
      }
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
      if (file_exist) {
        printf("base64 file: %s\n", string);
      } else {
        printf("base64 string: %s\n", string);
      }
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
  command_length_check( !=, 3);
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
      printf("hash file: %s\n", string);
      print_hex((uint8_t *)outbyte, hash_size);
    }
  } else {
    if (mbedtls_md(md_info, (unsigned char *)string, strlen(string), outbyte) != 0) {
      map_err(ERR_INTERNAL, "hash string with error");
    } else {
      printf("hash string: %s\n", string);
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

#define ANSI_CODE_RESET "\033[00m"
#define ANSI_CODE_BOLD "\033[1m"
#define ANSI_CODE_DARK "\033[2m"
#define ANSI_CODE_UNDERLINE "\033[4m"
#define ANSI_CODE_BLINK "\033[5m"
#define ANSI_CODE_REVERSE "\033[7m"
#define ANSI_CODE_CONCEALED "\033[8m"
#define ANSI_CODE_GRAY "\033[30m"
#define ANSI_CODE_GREY "\033[30m"
#define ANSI_CODE_RED "\033[31m"
#define ANSI_CODE_GREEN "\033[32m"
#define ANSI_CODE_YELLOW "\033[33m"
#define ANSI_CODE_BLUE "\033[34m"
#define ANSI_CODE_MAGENTA "\033[35m"
#define ANSI_CODE_CYAN "\033[36m"
#define ANSI_CODE_WHITE "\033[37m"

bool command_help(commands_t commands, int commands_length) {
  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "help");
  printf(ANSI_CODE_YELLOW "\n\tprint help information" ANSI_CODE_RESET "\n");

  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "exit");
  printf(ANSI_CODE_YELLOW "\n\texit the application" ANSI_CODE_RESET "\n");

  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "version");
  printf(ANSI_CODE_YELLOW "\n\tprint version" ANSI_CODE_RESET "\n\n");

  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "hmap");
  printf(" <set> <key> <value>\n");
  printf(ANSI_CODE_YELLOW "\tadd new key and value to hashmap" ANSI_CODE_RESET "\n");
  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "hmap");
  printf(" <get/del> <key>\n");
  printf(ANSI_CODE_YELLOW "\tget/delete the specific key from hashmap" ANSI_CODE_RESET "\n");
  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "hmap");
  printf(" <cap/len>\n");
  printf(ANSI_CODE_YELLOW "\tget the cap/len from hashmap" ANSI_CODE_RESET "\n");
  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "hmap");
  printf(" <print>\n");
  printf(ANSI_CODE_YELLOW "\tprint all of keys in the hashmap" ANSI_CODE_RESET "\n\n");

  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "lru");
  printf(" <set> <key> <value>\n");
  printf(ANSI_CODE_YELLOW "\tadd new key and value to lru" ANSI_CODE_RESET "\n");
  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "lru");
  printf(" <get> <key>\n");
  printf(ANSI_CODE_YELLOW "\tget the specific key from lru" ANSI_CODE_RESET "\n");
  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "lru");
  printf(" <cap/len>\n");
  printf(ANSI_CODE_YELLOW "\tget the cap/len from lru" ANSI_CODE_RESET "\n");
  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "lru");
  printf(" <print>\n");
  printf(ANSI_CODE_YELLOW "\tprint all of keys in the lru" ANSI_CODE_RESET "\n\n");

  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "avl");
  printf(" <set> <key> <value>\n");
  printf(ANSI_CODE_YELLOW "\tadd new key and value to avl" ANSI_CODE_RESET "\n");
  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "avl");
  printf(" <get> <key>\n");
  printf(ANSI_CODE_YELLOW "\tget the specific key from avl" ANSI_CODE_RESET "\n");
  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "avl");
  printf(" <print>\n");
  printf(ANSI_CODE_YELLOW "\tprint all of keys in the avl" ANSI_CODE_RESET "\n");
  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "avl");
  printf(" <in/pre/post>\n");
  printf(ANSI_CODE_YELLOW "\ttraverse the avl with specific method" ANSI_CODE_RESET "\n");
  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "avl");
  printf(" <dump> <filename>\n");
  printf(ANSI_CODE_YELLOW "\tdump the avl to dot file" ANSI_CODE_RESET "\n\n");

  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "sklist");
  printf(" <set> <key> <value>\n");
  printf(ANSI_CODE_YELLOW "\tadd new key and value to sklist" ANSI_CODE_RESET "\n");
  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "sklist");
  printf(" <get/del> <key>\n");
  printf(ANSI_CODE_YELLOW "\tget/delete the specific key from sklist" ANSI_CODE_RESET "\n\n");

  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "base64");
  printf(" <enc/dec> <string/filename>\n");
  printf(ANSI_CODE_YELLOW "\tbase64 decode/encode string/file" ANSI_CODE_RESET "\n\n");

  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "hash");
  printf(" <method> <string/filename>\n");
  printf(ANSI_CODE_YELLOW "\thash string/file, support methods: md5 sha1 sha256 sha512" ANSI_CODE_RESET "\n\n");

  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "vi");
  printf(" <filename>\n");
  printf(ANSI_CODE_YELLOW "\tedit file like vim" ANSI_CODE_RESET "\n\n");

  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "tcp");
  printf(" <ip> <port>\n");
  printf(ANSI_CODE_YELLOW "\ttest ip:port is reachable or not" ANSI_CODE_RESET "\n\n");

  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "server");
  printf(" <start/stop> <dir> <port>\n");
  printf(ANSI_CODE_YELLOW "\tserve the dir at 0.0.0.0:port" ANSI_CODE_RESET "\n\n");

  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "pi");
  printf(" <length>\n");
  printf(ANSI_CODE_YELLOW "\tcalculate the pi to specific length" ANSI_CODE_RESET "\n\n");

  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "uuid");
  printf(ANSI_CODE_YELLOW "\n\tgenerate a valid uuid string" ANSI_CODE_RESET "\n");

  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "uname");
  printf(ANSI_CODE_YELLOW "\n\tlike linux uname output" ANSI_CODE_RESET "\n");

  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "uptime");
  printf(ANSI_CODE_YELLOW "\n\tlike linux uptime output" ANSI_CODE_RESET "\n");

  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "hostname");
  printf(ANSI_CODE_YELLOW "\n\tlike linux hostname output" ANSI_CODE_RESET "\n");

  printf(ANSI_CODE_GREEN "%s" ANSI_CODE_RESET, "game");
  printf(" <name>\n");
  printf(ANSI_CODE_YELLOW "\tplay some games,  support: 2048" ANSI_CODE_RESET "\n");

  return MAP_COMMANDS_OK;
}
