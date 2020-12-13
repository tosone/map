#include <stdbool.h>

#pragma once

#define AVL_COMMAND "avl"
#define AVL_COMMAND_GET "get"
#define AVL_COMMAND_SET "set"
#define AVL_COMMAND_PRINT "print"
#define AVL_COMMAND_IN "in"
#define AVL_COMMAND_PRE "pre"
#define AVL_COMMAND_POST "post"
#define AVL_COMMAND_DUMP "dump"

typedef struct avl_entry_t {
  int key;
  int height;
  struct avl_entry_t *left;
  struct avl_entry_t *right;
} avl_entry_t;

avl_entry_t *avl_create(avl_entry_t *node, int key);
avl_entry_t *avl_set(avl_entry_t *node, int key);
bool avl_get(avl_entry_t *root, int key);
void avl_pre_order(avl_entry_t *entry);
void avl_in_order(avl_entry_t *entry);
void avl_post_order(avl_entry_t *entry);
void avl_dump(avl_entry_t *entry, char *filename);
void avl_free(avl_entry_t *entry);
