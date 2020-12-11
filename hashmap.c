#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hashmap.h>
#include <murmurhash.h>

hashmap_t *hashmap_create(int size) {
  hashmap_t *hashmap = (hashmap_t *)malloc(sizeof(hashmap_t));
  hashmap->cap = size;
  hashmap->len = 0;
  hashmap->entries = (entry_t **)malloc(sizeof(entry_t *) * size);
  for (int i = 0; i < size; i++) {
    hashmap->entries[i] = NULL;
  }
  return hashmap;
}

entry_t *hashmap_pair(const char *key, const void *value, const int value_length) {
  entry_t *entry = (entry_t *)malloc(sizeof(entry_t));
  entry->key = (char *)malloc(strlen(key) + 1);
  entry->value = (void *)malloc(value_length);

  strcpy(entry->key, key);
  memcpy(entry->value, value, value_length);
  entry->next = NULL;

  return entry;
}

void hashmap_set(hashmap_t *hashtable, const char *key, const void *value, const int value_length) {
  uint32_t slot = murmurhash(key, strlen(key), 0);
  entry_t *entry = hashtable->entries[slot % hashtable->cap];
  if (entry == NULL) {
    hashtable->entries[slot % hashtable->cap] = hashmap_pair(key, value, value_length);
    return;
  }

  entry_t *prev;

  while (entry != NULL) {
    if (strcmp(entry->key, key) == 0) {
      free(entry->value);
      entry->value = (void *)malloc(value_length);
      memcpy(entry->value, value, value_length);
      return;
    }

    prev = entry;
    entry = prev->next;
  }

  prev->next = hashmap_pair(key, value, value_length);
}

char *hashmap_get(hashmap_t *hashtable, const char *key) {
  uint32_t slot = murmurhash(key, strlen(key), 0);
  entry_t *entry = hashtable->entries[slot % hashtable->cap];
  if (entry == NULL) {
    return NULL;
  }
  while (entry != NULL) {
    if (strcmp(entry->key, key) == 0) {
      return entry->value;
    }
    entry = entry->next;
  }
  return NULL;
}

void hashmap_del(hashmap_t *hashtable, const char *key) {
  uint32_t slot = murmurhash(key, strlen(key), 0);
  entry_t *entry = hashtable->entries[slot % hashtable->cap];
  if (entry == NULL) {
    return;
  }
  entry_t *prev;
  int idx = 0;
  while (entry != NULL) {
    if (strcmp(entry->key, key) == 0) {
      if (entry->next == NULL && idx == 0) {
        hashtable->entries[slot % hashtable->cap] = NULL;
      }
      if (entry->next != NULL && idx == 0) {
        hashtable->entries[slot % hashtable->cap] = entry->next;
      }
      if (entry->next == NULL && idx != 0) {
        prev->next = NULL;
      }
      if (entry->next != NULL && idx != 0) {
        prev->next = entry->next;
      }
      free(entry->key);
      free(entry->value);
      free(entry);
      return;
    }
    prev = entry;
    entry = prev->next;
    ++idx;
  }
}

void entry_free(entry_t *entry) {
  while (entry->next != NULL) {
    entry_free(entry->next);
    free(entry->next);
  }
  free(entry->key);
  free(entry->value);
  free(entry);
}

void hashmap_free(hashmap_t *hashtable) {
  for (int i = 0; i < hashtable->cap; i++) {
    if (hashtable->entries[i] != NULL) {
      entry_free(hashtable->entries[i]);
    }
  }
}
