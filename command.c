#include <stdlib.h>
#include <string.h>

#include <command.h>

Commands Commands_parse(char *cmd, int *len) {
  int index = 0;
  Commands commands;
  char *token = strtok(cmd, " ");
  while (token != NULL) {
    if (index == 0) {
      commands = (char **)malloc(sizeof(char *) * (index + 1));
    } else {
      commands = (char **)realloc(commands, sizeof(char *) * (index + 1));
    }
    size_t size = strlen(token) + 1;
    commands[index] = (char *)malloc(sizeof(char) * size);
    memcpy(commands[index], token, size);
    token = strtok(NULL, " ");
    index++;
  }
  *len = index;
  return commands;
}

void Commands_free(Commands commands, int length) {
  for (int i = 0; i < length; i++) {
    free(commands[i]);
  }
  free(commands);
}
