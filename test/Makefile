CFLAGS+=-I/usr/local/include
LDFLAGS+=-L.. -lbimage -L/usr/local/lib -ltiff -lcheck -lm -lpthread
UNAME:=$(shell uname)
BENCHMARK?=YES

ifeq ($(UNAME),Linux)
	LDFLAGS+=-lrt -lsubunit
endif

ifeq ($(BENCHMARK),YES)
	CFLAGS+=-DBENCH_RUN
endif

run: test
	@LD_LIBRARY_PATH=.. ./test

.PHONY: test
test:
	@$(CC) $(CFLAGS) -o test test.c $(LDFLAGS)

clean:
	@rm -f test
