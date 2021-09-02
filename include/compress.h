#include <assert.h>
#include <stdio.h>

#include <zlib.h>

#pragma once

#define CHUNK 16384

int def(FILE *source, FILE *dest);
int inf(FILE *source, FILE *dest);
