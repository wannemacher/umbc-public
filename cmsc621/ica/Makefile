CC=gcc
CFLAGS=-c -Wall

all: ica20150909

ica20150909: ica20150909.o
	$(CC) -pthread ica20150909.o -o ica20150909

	@rm ica20150909.o

ica20150909.o: ica20150909.c
	$(CC) $(CFLAGS) ica20150909.c

clean:
	@rm -rf *.o 2> /dev/null || true
	@rm -rf ica20150909 2> /dev/null || true
	@rm -rf *~ 2> /dev/null || true
