CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

frcc: $(OBJS)
				$(CC) -o frcc $(OBJS) $(LDFLAGS)

$(OBJS): frcc.h

test: frcc
		./test.sh

clean:
		rm -f frcc *.o *~ tmp*

.PHONY: test clean
