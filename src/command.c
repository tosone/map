#include <stdlib.h>
#include <string.h>

#include <command.h>

/* copy argument number index into the command array, growing it as needed */
static void commands_append(commands_t *commands, int index, const char *token) {
  if (index == 0) { // growing one element at a time is fine: commands come from a human and stay short
    *commands = (char **)malloc(sizeof(char *) * (index + 1));
  } else {
    *commands = (char **)realloc(*commands, sizeof(char *) * (index + 1));
  }
  size_t size = strlen(token) + 1; // room for the string itself plus '\0'
  (*commands)[index] = (char *)malloc(sizeof(char) * size);
  memcpy((*commands)[index], token, size); // copy the token into the freshly allocated memory
}

commands_t commands_parse(char *cmd, int *len) {
  int index = 0;
  commands_t commands = NULL;
  char *token = strtok(cmd, " ");
  while (token != NULL) {
    commands_append(&commands, index, token);
    token = strtok(NULL, " ");
    index++;
  }
  *len = index;
  return commands;
}

commands_t commands_from_argv(char **argv, int count, int *len) {
  commands_t commands = NULL;
  for (int index = 0; index < count; index++) {
    commands_append(&commands, index, argv[index]);
  }
  *len = count;
  return commands;
}

void commands_free(commands_t commands, int length) {
  for (int i = 0; i < length; i++) {
    free(commands[i]); // free every string
  }
  free(commands); // free the pointer array
}
