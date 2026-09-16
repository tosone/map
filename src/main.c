#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <map.h>

/* the Makefile injects -DVERSION=... (the git tag by default); this is the fallback */
#ifndef VERSION
#define VERSION "unknown"
#endif

const char *hostory_filename = ".map_history";
const char *prompt = "map> ";

typedef enum {
  COMMANDS_DONE,    /* command executed */
  COMMANDS_FAILED,  /* known command that failed */
  COMMANDS_EXIT,    /* exit requested in interactive mode */
  COMMANDS_UNKNOWN, /* unknown command */
} commands_status_t;

/* the interactive loop and one shot invocations both dispatch here, arguments are already split */
static commands_status_t execute(commands_t commands, int commands_length) {
  if (commands_length < 1) {
    return COMMANDS_DONE;
  }

  if (strncasecmp(commands[0], COMMAND_EXIT, strlen(COMMAND_EXIT)) == 0) {
    return COMMANDS_EXIT;
  }
  if (strncasecmp(commands[0], COMMAND_VERSION, strlen(COMMAND_VERSION)) == 0) {
    printf("%s\n", VERSION);
    return COMMANDS_DONE;
  }

  bool ok;
  if (strncasecmp(commands[0], COMMAND_HELP, strlen(COMMAND_HELP)) == 0) {
    ok = command_help(commands, commands_length);
  } else if (strncasecmp(commands[0], COMMAND_BASE64, strlen(COMMAND_BASE64)) == 0) {
    ok = command_base64(commands, commands_length);
  } else if (strncasecmp(commands[0], COMMAND_HASH, strlen(COMMAND_HASH)) == 0) {
    ok = command_hash(commands, commands_length);
  } else if (strncasecmp(commands[0], COMMAND_VI, strlen(COMMAND_VI)) == 0) {
    ok = command_vi(commands, commands_length);
  } else if (strncasecmp(commands[0], COMMAND_TCP, strlen(COMMAND_TCP)) == 0) {
    ok = command_tcp(commands, commands_length);
  } else if (strncasecmp(commands[0], COMMAND_SERVER, strlen(COMMAND_SERVER)) == 0) {
    ok = command_server(commands, commands_length);
  } else if (strncasecmp(commands[0], COMMAND_UNAME, strlen(COMMAND_UNAME)) == 0) {
    ok = command_uname(commands, commands_length);
  } else if (strncasecmp(commands[0], COMMAND_UPTIME, strlen(COMMAND_UPTIME)) == 0) {
    ok = command_uptime(commands, commands_length);
  } else if (strncasecmp(commands[0], COMMAND_HOSTNAME, strlen(COMMAND_HOSTNAME)) == 0) {
    ok = command_hostname(commands, commands_length);
  } else if (strncasecmp(commands[0], COMMAND_GAME, strlen(COMMAND_GAME)) == 0) {
    ok = command_game(commands, commands_length);
  } else if (strncasecmp(commands[0], COMMAND_PI, strlen(COMMAND_PI)) == 0) {
    ok = command_pi(commands, commands_length);
  } else if (strncasecmp(commands[0], COMMAND_UUID, strlen(COMMAND_UUID)) == 0) {
    ok = command_uuid(commands, commands_length);
  } else if (strncasecmp(commands[0], COMMAND_GENPASSWD, strlen(COMMAND_GENPASSWD)) == 0) {
    ok = command_genpasswd(commands, commands_length);
  } else {
    printf("%s\n", ERR_COMMAND_NOT_FOUND);
    return COMMANDS_UNKNOWN;
  }
  return ok ? COMMANDS_DONE : COMMANDS_FAILED;
}

int main(int argc, char **argv) {
  /* with arguments run a one shot command instead of the REPL, e.g. ./map version */
  if (argc > 1) {
    int commands_length = 0;
    commands_t commands = commands_from_argv(argv + 1, argc - 1, &commands_length);
    commands_status_t status = execute(commands, commands_length);
    commands_free(commands, commands_length);
    return status == COMMANDS_DONE || status == COMMANDS_EXIT ? EXIT_SUCCESS : EXIT_FAILURE;
  }

  linenoiseSetCompletionCallback(completion);

  struct passwd *pw = getpwuid(getuid());
  const char *homedir = pw->pw_dir;
  char *history_file = (char *)malloc(strlen(homedir) + strlen(hostory_filename) + 2);
  bzero(history_file, strlen(homedir) + strlen(hostory_filename) + 2);
  strcpy(history_file, homedir);
  strcat(history_file + strlen(homedir), "/");
  strcpy(history_file + strlen(homedir) + 1, hostory_filename);

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

      commands_status_t status = execute(commands, commands_length);
      commands_free(commands, commands_length);

      if (status == COMMANDS_EXIT) {
        free(line);
        return EXIT_SUCCESS;
      }
    }
    free(line);
  }
  return EXIT_SUCCESS;
}
