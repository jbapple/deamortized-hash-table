.PHONY: all 
.PRECIOUS: o1.exe o2.exe

all: o1.exe o2.exe
	time -p ./o1.exe
	time -p ./o2.exe

CXX=clang++
HERE=Makefile
CXXFLAGS=-std=c++0x -DNDEBUG -I..  insert-maxtime.cc -g
LDFLAGS=-lrt

o1.exe: insert-maxtime.cc ${HERE} ../lazy-linear.cc
	${CXX} -O1 ${CXXFLAGS} -o $@ ${LDFLAGS}

o2.exe: insert-maxtime.cc ${HERE} ../lazy-linear.cc
	${CXX} -O2 ${CXXFLAGS} -o $@ ${LDFLAGS}