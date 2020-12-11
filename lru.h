#pragma once

#define LRU_COMMAND "lru"
#define LRU_COMMAND_LEN "len"
#define LRU_COMMAND_GET "get"
#define LRU_COMMAND_SET "set"

typedef struct LRUEntry {
  void *data; // val
  char *key;  // key
} LRUEntry;

typedef struct LRU {
  int len;
  int cap;
  LRUEntry data;
  LRUEntry *next;
} LRU;

LRU *LRU_init(int cap);

void LRU_put(char *key, char *val);

void LRU_get(char *key);
