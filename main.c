#include <map.h>

#define VERSION "v2.1.0"

#define COMMANDS_CHECK(x)                     \
  if (x) {                                    \
    commands_free(commands, commands_length); \
    continue;                                 \
  }

int main(int argc, char **argv) {
  linenoiseSetCompletionCallback(completion);
  linenoiseHistoryLoad("history.txt");
  linenoiseHistorySetMaxLen(1000);

  atexit(clear);

  char *line;
  while ((line = linenoise("map> ")) != NULL) {
    if (line[0] != '\0') {
      linenoiseHistoryAdd(line);
      linenoiseHistorySave("history.txt");

      char *line_copy = (char *)malloc(sizeof(char) * (strlen(line) + 1));
      memcpy(line_copy, line, strlen(line) + 1);
      int commands_length;
      commands_t commands = commands_parse(line_copy, &commands_length);
      free(line_copy);

      if (commands_length >= 1) {
        if (strncasecmp(commands[0], COMMAND_HELP, strlen(COMMAND_HELP)) == 0) {
          COMMANDS_CHECK(!command_help(commands, commands_length));
        } else if (strncasecmp(commands[0], COMMAND_EXIT, strlen(COMMAND_EXIT)) == 0) {
          commands_free(commands, commands_length);
          free(line);
          return EXIT_SUCCESS;
        } else if (strncasecmp(commands[0], COMMAND_VERSION, strlen(COMMAND_VERSION)) == 0) {
          printf("%s\n", VERSION);
        } else if (strncasecmp(commands[0], COMMAND_BASE64, strlen(COMMAND_BASE64)) == 0) {
          COMMANDS_CHECK(!command_base64(commands, commands_length));
        } else if (strncasecmp(commands[0], COMMAND_HASH, strlen(COMMAND_HASH)) == 0) {
          COMMANDS_CHECK(!command_hash(commands, commands_length));
        } else if (strncasecmp(commands[0], COMMAND_VI, strlen(COMMAND_VI)) == 0) {
          COMMANDS_CHECK(!command_vi(commands, commands_length));
        } else if (strncasecmp(commands[0], COMMAND_TCP, strlen(COMMAND_TCP)) == 0) {
          COMMANDS_CHECK(!command_tcp(commands, commands_length));
        } else if (strncasecmp(commands[0], COMMAND_SERVER, strlen(COMMAND_SERVER)) == 0) {
          COMMANDS_CHECK(!command_server(commands, commands_length));
        } else if (strncasecmp(commands[0], COMMAND_UNAME, strlen(COMMAND_UNAME)) == 0) {
          COMMANDS_CHECK(!command_uname(commands, commands_length));
        } else if (strncasecmp(commands[0], COMMAND_UPTIME, strlen(COMMAND_UPTIME)) == 0) {
          COMMANDS_CHECK(!command_uptime(commands, commands_length));
        } else if (strncasecmp(commands[0], COMMAND_GAME, strlen(COMMAND_GAME)) == 0) {
          COMMANDS_CHECK(!command_game(commands, commands_length));
        } else {
          printf("%s\n", ERR_COMMAND_NOT_FOUND);
        }
      }
      commands_free(commands, commands_length);
    }
    free(line);
  }
  return EXIT_SUCCESS;
}
