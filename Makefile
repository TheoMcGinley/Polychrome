PREFIX?=/usr/X11R6
CFLAGS?=-Os -pedantic -Wall

all:
	$(CC) $(CFLAGS) -I$(PREFIX)/include theowm.c -L$(PREFIX)/lib -lX11 -o theowm

install:
	install -pm 755 theowm $(DESTDIR)/bin

clean:
	rm -f theowm

