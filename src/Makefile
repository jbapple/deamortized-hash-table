.PHONY: benchmark

HERE=Makefile
CXX=g++
CXXFLAGS=-std=c++0x -O0 -DNDEBUG -g
LDFLAGS=-lrt

util.o: util.cc util.hh ${HERE}

benchmark:
	${MAKE} -C benchmark