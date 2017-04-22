EXT=so
UNAME=$(shell uname)

ifeq ($(UNAME),Darwin)
	EXT=dylib
endif

lib: static shared bin

static:
	$(CC) -c -o view.o `pkg-config --cflags gtk+-3.0` view.c
	ar rcs libbimage-view.a view.o

shared:
	$(CC) -shared -o libbimage-view.$(EXT) `pkg-config --cflags gtk+-3.0` view.o

.PHONY: bin
bin:
	$(CC) -o bin/bimage-view `pkg-config --cflags --libs  gtk+-3.0` -lbimage view.c bin/main.c

install:
	install libbimage-view.$(EXT) libbimage-view.a /usr/local/lib
	install bimage-view.h /usr/local/include
	install bin/bimage-view /usr/local/bin

uninstall:
	rm -f /usr/local/lib/libbimage-view.* /usr/local/include/bimage-view.h /usr/local/bin/bimage-view

clean:
	rm *.o *.a *.$(EXT)
