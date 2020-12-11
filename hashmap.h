#pragma once

#define HMAP_COMMAND "hmap"
#define HMAP_COMMAND_GET "get"
#define HMAP_COMMAND_SET "set"
#define HMAP_COMMAND_DEL "del"

const HMAP_MAX_LINK_LIST_DEPTH = 8;   // 分支链表的最大深度
const HMAP_INITIALIZE_SIZE = 8;       // 初始化空间
const HMAP_GROW_FACTOR = 2;           // 增长因子
const HMAP_GROW_FACTOR_USABLE = 1024; // 增长因子最大的有效范围
const HMAP_GROW_MAX = 1024;           // 增长的最大数值
const HMAP_THRESHOLD = 0.75f;         // 负载因子

typedef struct entry_t {
  char *key;
  void *value;
  struct entry_t *next;
} entry_t;

typedef struct {
  int cap;
  int len;
  entry_t **entries;
} hashmap_t;

unsigned int hash(const char *key);
hashmap_t *hashmap_create(int size);
entry_t *hashmap_pair(const char *key, const void *value, const int value_length);
void hashmap_set(hashmap_t *hashtable, const char *key, const void *value, const int value_length);
char *hashmap_get(hashmap_t *hashtable, const char *key);
void hashmap_del(hashmap_t *hashtable, const char *key);
void hashmap_free(hashmap_t *hashtable);
