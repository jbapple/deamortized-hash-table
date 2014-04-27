.PHONY: all test

all: src/tlsf.o test/tlsf_test.exe

src/tlsf.o: src/tlsf.c
	gcc -std=c99 -ggdb3 -O0 -c src/tlsf.c -o src/tlsf.o

test/tlsf_test.exe: src/tlsf.o test/tlsf_test.c src/tlsf-internals.h
	gcc -std=c99 -ggdb3 -O0 -Isrc test/tlsf_test.c src/tlsf.o -o test/tlsf_test.exe

test: test/tlsf_test.exe
	test/tlsf_test.exe