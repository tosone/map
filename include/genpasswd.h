#include <stdlib.h>
#include <string.h>

#pragma once

#define ARR_SIZE(arr) (sizeof((arr)) / sizeof((arr[0])))

char *genpasswd(int length);
