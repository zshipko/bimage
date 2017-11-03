EXT=so
UNAME=$(shell uname)

ifeq ($(UNAME),Darwin)
	EXT=dylib
endif

lib: static shared bin

static:
	$(CC) -fPIC -c -o bimage-gtk.o `pkg-config --cflags gtk+-3.0` bimage-gtk.c
	ar rcs libbimage-gtk.a bimage-gtk.o

shared:
	$(CC) -shared -o libbimage-gtk.$(EXT) `pkg-config --cflags gtk+-3.0` bimage-gtk.o

.PHONY: bin
bin:
	$(CC) -o bin/bimage-gtk `pkg-config --cflags --libs  gtk+-3.0` -lbimage bimage-gtk.c bin/main.c

install:
	install libbimage-gtk.$(EXT) libbimage-gtk.a /usr/local/lib
	install bimage-gtk.h /usr/local/include
	install bin/bimage-gtk /usr/local/bin

uninstall:
	rm -f /usr/local/lib/libbimage-gtk .* /usr/local/include/bimage-gtk.h /usr/local/bin/bimage-gtk

clean:
	rm *.o *.a *.$(EXT)
