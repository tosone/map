#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <map.h>

#define VERSION "v3.0.5"

#define COMMANDS_CHECK(x)                     \
  if (x) {                                    \
    commands_free(commands, commands_length); \
    continue;                                 \
  }

const char *hostory_filename = ".map_history";
const char *prompt = "map> ";

int main(int argc, char **argv) {
  linenoiseSetCompletionCallback(completion);

  struct passwd *pw = getpwuid(getuid());
  const char *homedir = pw->pw_dir;
  char *history_file = (char *)malloc(strlen(homedir) + strlen(hostory_filename) + 2);
  bzero(history_file, strlen(homedir) + strlen(hostory_filename) + 2);
  strcpy(history_file, homedir);
  strcat(history_file + strlen(homedir), "/");
  strcpy(history_file + strlen(homedir) + 1, hostory_filename);
  printf("history file: %s\n", history_file);

  linenoiseHistoryLoad(history_file);
  linenoiseHistorySetMaxLen(1000);

  atexit(clear);

  char *line;
  while ((line = linenoise(prompt)) != NULL) {
    if (line[0] != '\0') {
      linenoiseHistoryAdd(line);
      linenoiseHistorySave(history_file);

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
        } else if (strncasecmp(commands[0], COMMAND_HOSTNAME, strlen(COMMAND_HOSTNAME)) == 0) {
          COMMANDS_CHECK(!command_hostname(commands, commands_length));
        } else if (strncasecmp(commands[0], COMMAND_GAME, strlen(COMMAND_GAME)) == 0) {
          COMMANDS_CHECK(!command_game(commands, commands_length));
        } else if (strncasecmp(commands[0], COMMAND_PI, strlen(COMMAND_PI)) == 0) {
          COMMANDS_CHECK(!command_pi(commands, commands_length));
        } else if (strncasecmp(commands[0], COMMAND_UUID, strlen(COMMAND_UUID)) == 0) {
          COMMANDS_CHECK(!command_uuid(commands, commands_length));
        } else if (strncasecmp(commands[0], COMMAND_GZIP, strlen(COMMAND_GZIP)) == 0) {
          COMMANDS_CHECK(!command_gzip(commands, commands_length));
        } else if (strncasecmp(commands[0], COMMAND_GENPASSWD, strlen(COMMAND_GENPASSWD)) == 0) {
          COMMANDS_CHECK(!command_genpasswd(commands, commands_length));
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
