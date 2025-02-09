all: tpool_test

tpool_test: test.c libtpool.a
	clang -Wall -o tpool_test test.c -L. -ltpool

tpool.o: tpool.c tpool.h
	clang -O -c tpool.c

libtpool.a: tpool.o
	ar rcv libtpool.a tpool.o

libs: libtpool.a

clear:
	rm *.a *.o tpool_test