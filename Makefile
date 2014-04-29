.PHONY: all test clean

all: src/tlsf.o test/tlsf_test.exe

src/tlsf.o: src/tlsf.c
	gcc -W -Wall -Wextra -Werror -std=c99 -ggdb3 -O0 -c src/tlsf.c -o src/tlsf.o

test/tlsf_test.exe: src/tlsf.o test/tlsf_test.c src/tlsf-internals.h
	gcc -W -Wall -Wextra -Werror -std=c99 -ggdb3 -O0 -Isrc test/tlsf_test.c src/tlsf.o -o test/tlsf_test.exe -lm

test: test/tlsf_test.exe
	test/tlsf_test.exe

clean:
	rm -f src/tlsf.o test/tlsf_test.exe