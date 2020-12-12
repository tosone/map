#pragma once

#define HMAP_COMMAND "hmap"
#define HMAP_COMMAND_GET "get"
#define HMAP_COMMAND_SET "set"
#define HMAP_COMMAND_DEL "del"
#define HMAP_COMMAND_CAP "cap"
#define HMAP_COMMAND_LEN "len"

typedef struct entry_t {
  char *key;
  void *value;
  size_t value_length;
  struct entry_t *next;
} entry_t;

typedef struct {
  int cap;
  int len;
  entry_t **entries;
} hashmap_t;

unsigned int hash(const char *key);
hashmap_t *hashmap_create();
entry_t *hashmap_pair(const char *key, const void *value, const int value_length);
hashmap_t *hashmap_set(hashmap_t *hashtable, const char *key, const void *value, const int value_length);
char *hashmap_get(hashmap_t *hashtable, const char *key);
void hashmap_del(hashmap_t *hashtable, const char *key);
void hashmap_free(hashmap_t *hashtable);
