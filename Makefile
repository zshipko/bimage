SRC=src/pixel.c src/image.c src/tiff.c src/io.c src/resize.c src/filter.c
OBJ=$(SRC:.c=.o)

CFLAGS+=-I/usr/local/include
LDFLAGS+=-L/usr/local/lib -ltiff
UNAME := $(shell uname)
PREFIX=/usr/local

# Set correct library extension
ifeq ($(UNAME),Darwin)
	EXT=dylib
else
	EXT=so
endif

lib: shared static

shared: $(OBJ)
	$(CC) -shared -fPIC -I/usr/local/include $(OBJ) -o libbimage.$(EXT) $(LDFLAGS)

static: $(SRC)
	ar rcs libbimage.a $(OBJ)

%.o: %.c
	$(CC) -O3 -fPIC -c $*.c $(CFLAGS) -o $@

clean:
	rm src/*.o libbimage.*

install:
	install libbimage.a libbimage.$(EXT) $(PREFIX)/lib
	install src/bimage.h $(PREFIX)/include

uninstall:
	rm -f $(PREFIX)/lib/libbimage.*
	rm -f $(PREFIX)/include/bimage.h

.PHONY: test
test:
	@$(MAKE) -C test

