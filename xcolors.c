#include <stdio.h>
#include <string.h>
#include <X11/Xresource.h>
#include <X11/Xlib.h>
#define XRDB_LOAD_COLOR(R,V)    if (XrmGetResource(xrdb, R, NULL, &type, &value) == True) { \
                                  if (value.addr != NULL && strnlen(value.addr, 8) == 7 && value.addr[0] == '#') { \
                                    int i = 1; \
                                    for (; i <= 6; i++) { \
                                      if (value.addr[i] < 48) break; \
                                      if (value.addr[i] > 57 && value.addr[i] < 65) break; \
                                      if (value.addr[i] > 70 && value.addr[i] < 97) break; \
                                      if (value.addr[i] > 102) break; \
                                    } \
                                    if (i == 7) { \
                                      strncpy(V, value.addr, 7); \
                                      V[7] = '\0'; \
                                    } \
                                  } \
                                }
#include "xcolors.h"

void loadxrdb(size_t len, Color* colors) {
  // Reads in the primary colors from xrdb
  // and sets the colors array fg, bg values
  Display *display;
  char * resm;
  XrmDatabase xrdb;
  char *type;
  XrmValue value;

  display = XOpenDisplay(NULL);

  if (!display) goto backup;
  resm = XResourceManagerString(display);
  if (!resm) goto backup;
  xrdb = XrmGetStringDatabase(resm);
  if (!xrdb) goto backup;

  size_t namelen = 12;
  char bcolorname[12], fcolorname[] = "dwm.color0";
  char fcolor[10], bcolor[10];
  XRDB_LOAD_COLOR(fcolorname, fcolor);
  for (size_t i=0; i<len; i++) {
    snprintf(bcolorname, namelen, "dwm.color%lu", i%6+1);
    XRDB_LOAD_COLOR(bcolorname, bcolor);
    sprintf(colors[i].fg, "^c%s^", fcolor);
    sprintf(colors[i].bg, "^b%s^", bcolor);
  }

  XCloseDisplay(display);
  return;

backup:
  for (size_t i=0; i<len; i++) {
    sprintf(colors[i].fg, "^c#FFFFFF^");
    sprintf(colors[i].bg, "^b#000000^");
  }
  return;
}
