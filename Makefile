SRC=src/pixel.c src/image.c src/io.c src/resize.c \
	src/filter.c src/hash.c src/kiss_fft.c src/kiss_fftr.c

UNAME := $(shell uname)
PREFIX=/usr/local

CFLAGS+=-I/usr/local/include $(CFLAGS_$(UNAME_M))
LDFLAGS+=-L/usr/local/lib -lm
THREADS?=YES
BENCHMARK?=YES
TIFF?=YES

ifeq ($(THREADS),YES)
	LDFLAGS+= -lpthread
else
	CFLAGS+= -DBIMAGE_NO_PTHREAD
endif

# Set correct library extension
ifeq ($(UNAME),Darwin)
	EXT=dylib
else
	EXT=so
endif

ifeq ($(TIFF),YES)
	LDFLAGS+= -ltiff
	SRC+=src/tiff.c
else
	CFLAGS+= -DBIMAGE_NO_TIFF
endif

$(shell echo $(CFLAGS) > .cflags)
$(shell echo $(LDFLAGS) > .ldflags)

OBJ=$(SRC:.c=.o)

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
test: debug
	@CFLAGS='$(CFLAGS)' BENCHMARK='$(BENCHMARK)' $(MAKE) -C test

update-deps:
	curl https://raw.githubusercontent.com/nothings/stb/master/stb_image.h > src/stb_image.h
	curl https://raw.githubusercontent.com/nothings/stb/master/stb_image_resize.h > src/stb_image_resize.h
	curl https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h > src/stb_image_write.h
	curl https://raw.githubusercontent.com/mborgerding/kissfft/master/_kiss_fft_guts.h > src/_kiss_fft_guts.h
	curl https://raw.githubusercontent.com/mborgerding/kissfft/master/kiss_fft.h > src/kiss_fft.h
	curl https://raw.githubusercontent.com/mborgerding/kissfft/master/kiss_fft.c > src/kiss_fft.c
	curl https://raw.githubusercontent.com/mborgerding/kissfft/master/tools/kiss_fftr.h > src/kiss_fftr.h
	curl https://raw.githubusercontent.com/mborgerding/kissfft/master/tools/kiss_fftr.c > src/kiss_fftr.c

