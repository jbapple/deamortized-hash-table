.PHONY: benchmark

HERE=Makefile
CXX=clang++
CXXFLAGS=-std=c++0x -O0 -DNDEBUG
LDFLAGS=-lrt

util.o: util.cc util.hh ${HERE}

benchmark:
	${MAKE} -C benchmark