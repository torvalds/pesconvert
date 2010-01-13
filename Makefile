CC=gcc
CFLAGS=-Wall -g

pesconvert: pes.o svg.o png.o
	$(CC) -o $@ $^ -lpng
