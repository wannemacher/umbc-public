CC=gcc
CFLAGS=-c -Wall

all: mmap-sum

mmap-sum: mmap-sum.o
	$(CC) -pthread mmap-sum.o -o mmap-sum

	@rm mmap-sum.o

mmap-sum.o: mmap-sum.c
	$(CC) $(CFLAGS) mmap-sum.c

clean:
	@rm -rf *.o 2> /dev/null || true
	@rm -rf mmap-sum 2> /dev/null || true
	@rm -rf sum 2> /dev/null || true
	@rm -rf *~ 2> /dev/null || true
