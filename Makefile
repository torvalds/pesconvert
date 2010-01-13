CC=gcc
CFLAGS=-Wall -g

pesconvert: pes.o svg.o png.o cairo.o
	$(CC) -o $@ $^ -lpng -lcairo
