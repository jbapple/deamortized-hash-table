.PHONY: all
DATS=%_hash.dat %_tree.dat %_lazy.dat %_separate.dat %_aho.dat
.PRECIOUS: ${DATS} insert-maxtime.exe

all: insert-maxtime.png

CXX=clang++
HERE=Makefile
CXXFLAGS=-std=c++0x -O2 -DNDEBUG -I..
LDFLAGS=-lrt
SUDO=

%.png: ${DATS} %.gnuplot ${HERE}
	gnuplot $*.gnuplot

${DATS}: %.exe ${HERE}
	${SUDO} ./$*.exe hash >$*_hash.dat
	${SUDO} ./$*.exe tree >$*_tree.dat
	${SUDO} ./$*.exe lazy >$*_lazy.dat
	${SUDO} ./$*.exe separate >$*_separate.dat
	${SUDO} ./$*.exe aho >$*_aho.dat

%.exe: %.cc ${HERE}
	${CXX} ${CXXFLAGS} ../util.o $*.cc -o $@ ${LDFLAGS} garbage