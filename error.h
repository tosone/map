#include <time.h>

#pragma once

#define ERR_COMMAND "invalid command"
#define ERR_COMMAND_NOT_FOUND "command not found"

#define ERR_INTERNAL "internal error"

void map_err(char *level, char *info) {
#ifdef NDEBUG
  time_t t = time(NULL);
  char buf[16];
  buf[strftime(buf, sizeof(buf), "%H:%M:%S", t)] = '\0';
  printf("[%s] %s:%d %s: %s.\n", buf, __FILE__, __LINE__, level, x);
#else
  printf("%s: %s.\n", level, info);
#endif
}
