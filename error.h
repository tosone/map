#pragma once

#define ERR_COMMAND "Invalid command"
#define ERR_COMMAND_NOT_FOUND "Command not found"

#define ERR_INTERNAL(x) printf("[%s] %s:%d %s: %s.\n", __DATE__, __FILE__, __LINE__, "Internal error", x)
