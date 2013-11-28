CFLAGS=-std=c99
CXXFLAGS=-std=c++0x

.PHONY: all

all: tlsf.o tlsf_allocator.hpp.gch

tlsf.o: Makefile tlsf.c tlsf.h
	cc $(CFLAGS) tlsf.c -c

tlsf_allocator.hpp.gch: tlsf_allocator.hpp Makefile tlsf.h
	cc $(CXXFLAGS) tlsf_allocator.hpp