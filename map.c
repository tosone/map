#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linenoise.h>

void completion(const char *buf, linenoiseCompletions *lc) {
  if (buf[0] == 'l') {
    linenoiseAddCompletion(lc, "lru");
    linenoiseAddCompletion(lc, "lru len");
    linenoiseAddCompletion(lc, "lru get");
    linenoiseAddCompletion(lc, "lru set");
  }
}

int main(int argc, char **argv) {
  char *line;
  char *prgname = argv[0];

  while (argc > 1) {
    argc--;
    argv++;
    if (!strcmp(*argv, "--multiline")) {
      linenoiseSetMultiLine(1);
      printf("Multi-line mode enabled.\n");
    } else if (!strcmp(*argv, "--keycodes")) {
      linenoisePrintKeyCodes();
      exit(0);
    } else {
      fprintf(stderr, "Usage: %s [--multiline] [--keycodes]\n", prgname);
      exit(1);
    }
  }

  linenoiseSetCompletionCallback(completion);

  linenoiseHistoryLoad("history.txt");

  while ((line = linenoise("map> ")) != NULL) {
    if (line[0] != '\0' && line[0] != '/') {
      printf("echo: '%s'\n", line);
      linenoiseHistoryAdd(line);
      linenoiseHistorySave("history.txt");
    } else if (!strncmp(line, "/historylen", 11)) {
      int len = atoi(line + 11);
      linenoiseHistorySetMaxLen(len);
    } else if (!strncmp(line, "/mask", 5)) {
      linenoiseMaskModeEnable();
    } else if (!strncmp(line, "/unmask", 7)) {
      linenoiseMaskModeDisable();
    } else if (line[0] == '/') {
      printf("Unreconized command: %s\n", line);
    }
    free(line);
  }
  return 0;
}
