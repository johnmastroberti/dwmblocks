#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <X11/Xlib.h>

#include "xcolors.h"
#define LENGTH(X)               (sizeof(X) / sizeof (X[0]))

typedef struct {
	char* icon;
	char* command;
	unsigned int interval;
	int signal;
} Block;
void sighandler(int num);
void replace(char *str, char old, char new);
void getcmds(int time);
void getsigcmds(int signal);
void setupsignals();
void getstatus(char *str);
void setroot();
void statusloop();
void statusinit();
void sighandler(int signum);
void termhandler(int signum);


#include "blocks.h"

static Display *dpy;
static int screen;
static Window root;
#define MAX_BLOCK_LEN 128
#define MAX_TOT_LEN 1024
static char statusbar[LENGTH(blocks)][MAX_BLOCK_LEN] = {0};
static Color colors[LENGTH(blocks)];
static char statusstr[MAX_TOT_LEN];
//static char *statuscat;
//static const char *volupcmd[]  = { "volup", NULL };
//static const char *voldowncmd[]  = { "voldown", NULL };
//static const char *volmutecmd[]  = { "volmute", NULL };
static int statusContinue = 1;
//static int volmuted = 0;

void replace(char *str, char old, char new)
{
	int N = strlen(str);
	for(int i = 0; i < N; i++)
		if(str[i] == old)
			str[i] = new;
}

//opens process *cmd and stores output in *output
void getcmd(const Block *block, const Color *color, char *output)
{
  output[0] = '\0';
  snprintf(output, MAX_BLOCK_LEN, "%s%s ", color->fg, color->bg);
  size_t i = strlen(output);
  size_t color_len = i;
	char *cmd = block->command;
	FILE *cmdf = popen(cmd,"r");
	if (!cmdf)
		return;
	char c;
	while((c = fgetc(cmdf)) != EOF) {
    if (i >= MAX_BLOCK_LEN-1) break;
    if (c != '\n') output[i++] = c;
  }
	pclose(cmdf);
  if (i == color_len) {
    output[0] = '\0';
    return;
  } else if (i+5 < MAX_BLOCK_LEN-2) {
    output[i]='\0';
    strcat(output, " ^d^");
    i += 5;
    if (delim != '\0' && --i)
      output[i++] = delim;
  }
	output[i] = '\0';
}

void getcmds(int time)
{
	const Block* current;
	for(size_t i = 0; i < LENGTH(blocks); i++)
	{
		current = blocks + i;
		if ((current->interval != 0 && time % current->interval == 0) || time == -1) {
			getcmd(current, &colors[i], statusbar[i]);
      //printf("statusbar[%lu] = %s\n", i, statusbar[i]);
    }

	}
}

void getsigcmds(int signal)
{
	const Block *current;
	for (size_t i = 0; i < LENGTH(blocks); i++)
	{
		current = blocks + i;
		if (current->signal == signal) {
			getcmd(current, &colors[i], statusbar[i]);
      //printf("statusbar[%lu] = %s\n", i, statusbar[i]);
    }
	}
}

void setupsignals()
{
	for(size_t i = 0; i < LENGTH(blocks); i++)
	{
		if (blocks[i].signal > 0)
			signal(SIGRTMIN+blocks[i].signal, sighandler);
	}
  // also register colorsig
  signal(SIGRTMIN+colorsig, sighandler);

}

void getstatus(char *str)
{
	int j = 0;
	for(size_t i = 0; i < LENGTH(blocks); j+=strlen(statusbar[i++]))
	{
    if (j >= MAX_TOT_LEN) return;
		strcpy(str + j, statusbar[i]);
	}
	str[--j] = '\0';
}

void setroot()
{
	Display *d = XOpenDisplay(NULL);
	if (d) {
		dpy = d;
	}
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
	getstatus(statusstr);
	XStoreName(dpy, root, statusstr);
	XCloseDisplay(dpy);
}


void statusloop()
{
	setupsignals();
	int i = 0;
	getcmds(-1);
	while(statusContinue)
	{
		getcmds(i);
		setroot();
		sleep(1.0);
		i++;
	}
}

void statusinit()
{
  loadxrdb(LENGTH(blocks), colors);
	statusloop();
}


void sighandler(int signum)
{
  int sig_adj = signum-SIGRTMIN;
  if (sig_adj == colorsig) {
    loadxrdb(LENGTH(blocks), colors);
    getcmds(-1);
  } else {
    getsigcmds(sig_adj);
  }
  setroot();
}

void termhandler(int signum)
{
  (void) signum;
	statusContinue = 0;
	exit(0);
}

int main(int argc, char** argv)
{
	for(int i = 0; i < argc; i++)
	{
		if (!strcmp("-d",argv[i]))
			delim = argv[++i][0];
	}
	signal(SIGTERM, termhandler);
	signal(SIGINT, termhandler);
	statusinit();
}
