CFLAGS+=-I/usr/local/include
LDFLAGS+=-L.. -lbimage -L/usr/local/lib -ltiff -lcheck -lm -lpthread
UNAME:=$(shell uname)

ifeq ($(UNAME),Linux)
	LDFLAGS+=-lrt -lsubunit 
endif

run: test
	@LD_LIBRARY_PATH=.. ./test

.PHONY: test
test:
	@$(CC) $(CFLAGS) -o test test.c $(LDFLAGS)

clean:
	@rm -f test
