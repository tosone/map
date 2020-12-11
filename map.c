#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <command.h>
#include <error.h>
#include <hashmap.h>
#include <linenoise.h>
#include <lru.h>
#include <version.h>

#define command_length_check(x, y)            \
  if (commands_length x y) {                  \
    printf("%s\n", ERR_COMMAND);              \
    Commands_free(commands, commands_length); \
    continue;                                 \
  }

void completion(const char *buf, linenoiseCompletions *lc) {
  if (buf[0] == 'l') {
    linenoiseAddCompletion(lc, "lru");
  } else if (buf[0] == 'h') {
    linenoiseAddCompletion(lc, "hmap");
  }
}

char *hints(const char *buf, int *color, int *bold) {
  *color = 32;
  *bold = 0;
  if (strcmp(buf, "lru") == 0) {
    return " <command>";
  } else if (strcmp(buf, "lru len") == 0) {
    return " <length>";
  } else if (strcmp(buf, "lru get") == 0) {
    return " <key>";
  } else if (strcmp(buf, "lru set") == 0) {
    return " <key> <value>";
  } else if (strcmp(buf, "hmap") == 0) {
    return " <command>";
  } else if (strcmp(buf, "hmap get") == 0) {
    return " <key>";
  } else if (strcmp(buf, "hmap set") == 0) {
    return " <key>";
  }
  return NULL;
}

int main(int argc, char **argv) {
  linenoiseSetCompletionCallback(completion);
  linenoiseSetHintsCallback(hints);
  linenoiseHistoryLoad("history.txt");
  linenoiseHistorySetMaxLen(1000);

  char *line;
  while ((line = linenoise("map> ")) != NULL) {
    if (line[0] != '\0' && line[0] != '/') {
      linenoiseHistoryAdd(line);
      linenoiseHistorySave("history.txt");

      char *line_copy = (char *)malloc(sizeof(char) * (strlen(line) + 1));
      memcpy(line_copy, line, strlen(line) + 1);

      int commands_length;
      Commands commands = Commands_parse(line, &commands_length);

      free(line_copy);

      if (commands_length >= 1) {
        if (strncmp(commands[0], VERSION_COMMAND, strlen(VERSION_COMMAND)) == 0) {
          printf("%s\n", VERSION);
        } else if (strncmp(commands[0], LRU_COMMAND, strlen(LRU_COMMAND)) == 0) {
          command_length_check(<=, 2);
          if (strncmp(commands[1], LRU_COMMAND_LEN, strlen(LRU_COMMAND_LEN)) == 0) {
            command_length_check(!=, 3);
            int len = atoi(commands[2]);
            printf("%d\n", len);
          }
        } else {
          printf("%s\n", ERR_COMMAND_NOT_FOUND);
        }
      } else {
        printf("%s\n", ERR_COMMAND);
      }
      Commands_free(commands, commands_length);
    }
    free(line);
  }
  return EXIT_SUCCESS;
}
