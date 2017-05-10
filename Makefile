SRC=src/pixel.c src/image.c src/tiff.c src/io.c src/resize.c \
	src/filter.c src/hash.c src/histogram.c
OBJ=$(SRC:.c=.o)

UNAME := $(shell uname)
PREFIX=/usr/local

CFLAGS+=-I/usr/local/include $(CFLAGS_$(UNAME_M))
LDFLAGS+=-L/usr/local/lib -ltiff


# Set correct library extension
ifeq ($(UNAME),Darwin)
	EXT=dylib
else
	EXT=so
endif

lib: shared static

debug:
	$(MAKE) CFLAGS="$(CFLAGS) -g -Wall"

shared: $(OBJ)
	$(CC) -shared -fPIC -I/usr/local/include $(OBJ) -o libbimage.$(EXT) $(LDFLAGS)

static: $(SRC)
	ar rcs libbimage.a $(OBJ)

%.o: %.c
	$(CC) -O3 -fPIC -c $*.c $(CFLAGS) -o $@

clean:
	rm -f src/*.o libbimage.* test/test test/random.png

install:
	install libbimage.a libbimage.$(EXT) $(PREFIX)/lib
	install src/bimage.h $(PREFIX)/include

uninstall:
	rm -f $(PREFIX)/lib/libbimage.*
	rm -f $(PREFIX)/include/bimage.h

.PHONY: test
test:
	@$(MAKE) -C test

