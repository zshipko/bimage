EXT=so
UNAME=$(shell uname)
CFLAGS=`pkg-config --cflags gtk+-3.0` -I../src
LDFLAGS=-lm `pkg-config --libs gtk+-3.0` ../libbimage.a
TIFF?=YES
prefix?=/usr/local

ifeq ($(UNAME),Darwin)
	EXT=dylib
endif

ifneq ($(TIFF),NO)
	LDFLAGS+=-ltiff
endif

lib: static shared bin

static:
	$(CC) -fPIC -c -o bimage-gtk.o $(CFLAGS) bimage-gtk.c
	ar rcs libbimage-gtk.a bimage-gtk.o

shared:
	$(CC) -shared -o libbimage-gtk.$(EXT) $(CFLAGS)  bimage-gtk.o $(LDFLAGS)

.PHONY: bin
bin:
	$(CC) -o bin/bimage-gtk $(CFLAGS) bimage-gtk.c bin/main.c ../libbimage.a $(LDFLAGS)

install:
	install libbimage-gtk.$(EXT) libbimage-gtk.a $(prefix)/lib
	install bimage-gtk.h $(prefix)/include
	install bin/bimage-gtk $(prefix)/bin

uninstall:
	rm -f $(prefix)/lib/libbimage-gtk .* $(prefix)/include/bimage-gtk.h $(prefix)/bin/bimage-gtk

clean:
	rm *.o *.a *.$(EXT)
