CFLAGS=-std=c11 -g -static

frcc: frcc.c

test: frcc
		./test.sh

clean:
		rm -f frcc *.o *~ tmp*

.PHONY: test clean
