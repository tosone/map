#include <stdbool.h>

#pragma once

void initEditor(void);
void editorSelectSyntaxHighlight(char *filename);
int editorOpen(char *filename);
int enableRawMode(int fd);
void editorSetStatusMessage(const char *fmt, ...);
void editorRefreshScreen(void);
bool editorProcessKeypress(int fd);
void disableRawMode(int fd);
