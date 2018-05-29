SO_EXT?=so
prefix?=/usr/local
SRC=bimage-cv.cpp
CFLAGS=-I../src `pkg-config --cflags opencv`
LDFLAGS=-lm ../libbimage.a
TIFF?=YES

ifneq ($(TIFF),NO)
	LDFLAGS += -ltiff
endif

build:
	$(CXX) -std=c++11 -fPIC -shared -o libbimage-cv.$(SO_EXT) $(CFLAGS) $(SRC)  $(LDFLAGS)

install:
	install libbimage-cv.$(SO_EXT) $(prefix)/lib/
	install bimage-cv.h $(prefix)/include

uninstall:
	rm -f $(prefix)/include/bimage-cv.h $(prefix)/lib/libbimage-cv.*

clean:
	rm -f ./libbimage-cv.*
