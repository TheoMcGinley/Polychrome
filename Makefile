PREFIX?=/usr/X11R6
CFLAGS?=-Os -pedantic -Wall

all:
	$(CC) $(CFLAGS) -I$(PREFIX)/include polychrome.c -L$(PREFIX)/lib -lX11 -o polychrome

install:
	test -d $(DESTDIR)/bin || mkdir -p $(DESTDIR)/bin
	install -pm 755 polychrome $(DESTDIR)/bin

clean:
	rm -f polychrome

