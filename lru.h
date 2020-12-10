typedef struct LRUEntry {
  void *data;
  char *key;
} LRUEntry;

typedef struct LRU {
  int len;
  int cap;
  LRUEntry data;
  LRUEntry *next;
} LRU;
