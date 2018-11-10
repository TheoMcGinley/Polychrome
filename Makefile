PREFIX?=/usr/X11R6
CFLAGS?=-Os -pedantic -Wall

all:
	$(CC) $(CFLAGS) -I$(PREFIX)/include eventHandlers.c focus.c hide.c main.c manage.c scoring.c size.c utils.c polychrome.h -L$(PREFIX)/lib -lX11 -o polychrome

install:
	test -d $(DESTDIR)/bin || mkdir -p $(DESTDIR)/bin
	install -pm 755 polychrome $(DESTDIR)/bin

clean:
	rm -f polychrome

