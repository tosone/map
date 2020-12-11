#pragma once

typedef char **Commands;

Commands Commands_parse(char *cmd, int *len);
void Commands_free(Commands commands, int length);
