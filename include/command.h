#pragma once

typedef char **commands_t;

commands_t commands_parse(char *cmd, int *len);
/* take argv as the command arguments without splitting on spaces, so paths containing spaces work */
commands_t commands_from_argv(char **argv, int count, int *len);
void commands_free(commands_t commands, int length);
