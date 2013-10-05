.PHONY: benchmark

HERE=Makefile
CXX=clang++
CXXFLAGS=-std=c++0x -O3 -DNDEBUG
LDFLAGS=-lrt

#util.o: util.cc util.hh ${HERE}

#benchmark:
#	${MAKE} -C benchmark

tlsf.o: tlsf.c
	clang -std=c99 -c tlsf.c