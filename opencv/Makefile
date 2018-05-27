SO_EXT?=so
prefix?=/usr/local
SRC=bimage-opencv.cpp
CFLAGS=-I../src `pkg-config --cflags opencv`
LDFLAGS=-lm ../libbimage.a
TIFF?=YES

ifneq ($(TIFF),NO)
	LDFLAGS += -ltiff
endif

build:
	$(CXX) -std=c++11 -fPIC -shared -o libbimage-opencv.$(SO_EXT) $(CFLAGS) $(SRC)  $(LDFLAGS)

install:
	install libbimage-opencv.$(SO_EXT) $(prefix)/lib/
	install bimage-opencv.hpp $(prefix)/include

uninstall:
	rm -f $(prefix)/include/bimage-opencv.hpp $(prefix)/lib/libbimage-opencv.*

clean:
	rm -f ./libbimage-opencv*
