SO_EXT?=so
prefix?=/usr/local
SRC=bimage-opencv.cpp
CXXFLAGS=-I$(PWD)/../src `pkg-config --cflags opencv4`
LDFLAGS=-lm ../libbimage.a
TIFF?=YES

ifeq (TIFF,YES)
	LDFLAGS += -ltiff
endif

build:
	$(CXX) -std=c++11 -fPIC -shared -o libbimage-opencv.$(SO_EXT) $(CXXFLAGS) $(SRC)  $(LDFLAGS)

install:
	install libbimage-opencv.$(SO_EXT) $(prefix)/lib/
	install bimage-opencv.h $(prefix)/include

uninstall:
	rm -f $(prefix)/include/bimage-opencv.h $(prefix)/lib/libbimage-opencv.*

clean:
	rm -f ./libbimage-opencv.*
