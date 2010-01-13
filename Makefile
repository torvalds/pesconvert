CC=gcc
CFLAGS=-Wall -g

pesconvert: main.o pes.o svg.o png.o cairo.o
	$(CC) -o $@ $^ -lpng -lcairo
