.PHONY: benchmark

HERE=Makefile
CXX=g++
CXXFLAGS=-std=c++0x -O3 -DNDEBUG
LDFLAGS=-lrt

util.o: util.cc util.hh ${HERE}

benchmark:
	${MAKE} -C benchmark