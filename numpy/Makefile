EXT=so
UNAME=$(shell uname)

ifeq ($(UNAME),Darwin)
	EXT=dylib
endif

lib: static shared

static:
	$(CC) -fPIC -c -o bimage-numpy.o `pkg-config --cflags python3` bimage-numpy.c
	ar rcs libbimage-numpy.a bimage-numpy.o

shared:
	$(CC) -shared -o libbimage-numpy.$(EXT) `pkg-config --cflags python3` bimage-numpy.o

install:
	install libbimage-numpy.$(EXT) libbimage-numpy.a /usr/local/lib
	install bimage-numpy.h /usr/local/include

uninstall:
	rm -f /usr/local/lib/libbimage-numpy .* /usr/local/include/bimage-numpy.h

clean:
	rm *.o *.a *.$(EXT)
