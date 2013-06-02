.PHONY: all
DATS=%_hash.dat %_tree.dat %_lazy.dat %_separate.dat %_aho.dat
.PRECIOUS: ${DATS}

all: insert-maxtime.png

CXX=clang++
HERE=Makefile
CXXFLAGS=-std=c++0x -O0 -ggdb3 -DNDEBUG -I..
LDFLAGS=-lrt

%.png: ${DATS} %.gnuplot ${HERE}
	gnuplot $*.gnuplot

${DATS}: %.exe ${HERE}
	sudo ./$*.exe hash >$*_hash.dat
	sudo ./$*.exe tree >$*_tree.dat
	sudo ./$*.exe lazy >$*_lazy.dat
	sudo ./$*.exe separate >$*_separate.dat
	sudo ./$*.exe aho >$*_aho.dat

%.exe: %.cc ${HERE} 
	${CXX} ${CXXFLAGS} ../util.o $*.cc -o $@ ${LDFLAGS}