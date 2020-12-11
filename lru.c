#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lru.h>

LRU *LRU_init(int cap) {
  LRU *lru = (LRU *)malloc(sizeof(LRU));
  lru->next = NULL;
  lru->len = 0;
  lru->cap = cap;
  return lru;
}

void LRU_put(char *key, char *val) {
  printf("16: %lu %lu\n", strlen(key), strlen(val));
}

void LRU_get(char *key) {
}
