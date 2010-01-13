CC=gcc
CFLAGS=-Wall -g

pesconvert: main.o pes.o svg.o png.o cairo.o
	$(CC) -o $@ $^ -lpng -lcairo

update-mime:
	cp PES-Mime.xml /usr/share/mime/packages/
	update-mime-database /usr/share/mime
	cp pesconvert /usr/local/bin/application-x-pesfile-thumbnailer

gnome-thumbnailer:
	gconftool-2 --type bool -s "/desktop/gnome/thumbnailers/application@x-pesfile/enable" "true"
	gconftool-2 --type string -s "/desktop/gnome/thumbnailers/application@x-pesfile/command" "application-x-pesfile-thumbnailer -s %s %i %o"
