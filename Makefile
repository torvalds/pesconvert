CC=gcc
CFLAGS=-Wall -g

pesconvert: main.o pes.o svg.o png.o cairo.o
	$(CC) -o $@ $^ -lpng -lcairo

update-mime:
	cp PES-Mime.xml /usr/share/mime/packages/
	update-mime-database /usr/share/mime
