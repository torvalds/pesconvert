CC=gcc
CFLAGS=-Wall

pesconvert: pes.o svg.o
	$(CC) -o $@ $^
