SO_EXT?=so
prefix?=/usr/local
SRC=bimage-opencv.cpp

build:
	$(CXX) -std=c++11 -fPIC -shared -o libbimage-opencv.$(SO_EXT) $(SRC) `pkg-config --cflags --libs opencv` -L.. -lbimage

install:
	install libbimage-opencv.$(SO_EXT) $(prefix)/lib/
	install bimage-opencv.hpp $(prefix)/include

uninstall:
	rm -f $(prefix)/include/bimage-opencv.hpp $(prefix)/lib/libbimage-opencv.*

clean:
	rm -f ./libbimage-opencv*
