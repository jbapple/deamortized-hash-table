.PHONY: all
.PRECIOUS: %_hash.dat %_tree.dat

all: insert-maxtime.png

HERE=Makefile
CXXFLAGS=-std=c++0x -O3 -DNDEBUG -I..
LDFLAGS=-lrt

%.png: %_hash.dat %_tree.dat %.gnuplot ${HERE}
	gnuplot $*.gnuplot

%_hash.dat %_tree.dat: %.exe ${HERE}
	sudo ./$*.exe hash >$*_hash.dat
	sudo ./$*.exe tree >$*_tree.dat

%.exe: %.cc ${HERE} 
	${CXX} ${CXXFLAGS} ../util.o $*.cc -o $@ ${LDFLAGS}