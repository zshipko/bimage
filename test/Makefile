CFLAGS+=-I/usr/local/include
LDFLAGS+=-L.. -lbimage -L/usr/local/lib -ltiff -lcheck -lpthread -lm -lrt

run: test
	@LD_LIBRARY_PATH=.. ./test

.PHONY: test
test:
	@$(CC) $(CFLAGS) -o test test.c $(LDFLAGS)

clean:
	@rm -f test
