#ifndef XCOLORS_H
#define XCOLORS_H
typedef struct {
  char fg[11], bg[11];
} Color;

void loadxrdb(size_t len, Color* colors);
#endif
