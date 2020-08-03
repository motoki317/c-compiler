CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

main: $(OBJS)
	$(CC) -o main $(OBJS) $(LDFLAGS)

$(OBJS): main.h

test: main
	./test.sh

bench: main
	./test_bench.sh

clean:
	rm -f main *.o *~ tmp*

.PHONY: test bench clean
