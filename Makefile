CC=gcc
CFLAGS=-Wall -Wextra
LD=gcc
LDFLAGS=-lX11

dwmblocks: dwmblocks.o xcolors.o
dwmblocks.o: dwmblocks.c blocks.h
xcolors.o: xcolors.c xcolors.h
clean:
	-rm -f *.o *.gch dwmblocks

install: dwmblocks
	mkdir -p /usr/local/bin
	cp -f dwmblocks /usr/local/bin
