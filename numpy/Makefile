EXT=so
UNAME=$(shell uname)

ifeq ($(UNAME),Darwin)
	EXT=dylib
endif

CFLAGS+= -I/usr/local/include
LDFLAGS+= -L/usr/local/lib
PYTHON?=python3

lib: static shared

static:
	$(CC) $(CFLAGS) -fPIC -c -o bimage-numpy.o `pkg-config --cflags $(PYTHON)` bimage-numpy.c
	ar rcs libbimage-numpy.a bimage-numpy.o

shared:
	$(CC) $(CFLAGS) -shared -o libbimage-numpy.$(EXT) `pkg-config --cflags $(PYTHON)` bimage-numpy.o $(LDFLAGS)

install:
	install libbimage-numpy.$(EXT) libbimage-numpy.a /usr/local/lib
	install bimage-numpy.h /usr/local/include

uninstall:
	rm -f /usr/local/lib/libbimage-numpy .* /usr/local/include/bimage-numpy.h

clean:
	rm *.o *.a *.$(EXT)
