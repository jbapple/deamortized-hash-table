include ../Makefile.common

.PHONY: all
DATS=insert-maxtime-%.dat
.PRECIOUS: ${DATS} insert-maxtime.exe insert-maxtime.dat

all: insert-tradeoff.png

insert-tradeoff.png: insert-tradeoff.gnuplot insert-maxtime-0.dat insert-maxtime-6.dat insert-maxtime-1.dat ${HERE}
	gnuplot insert-tradeoff.gnuplot

insert-maxtime-%.dat: insert-maxtime.exe ${HERE}
	./insert-maxtime.exe $* >insert-maxtime-$*.dat

%.exe: %.cc util.o ${HERE}
	${CXX} ${CXXFLAGS} -I../../src util.o $*.cc -o $@ ${LDFLAGS}

util.o: util.cc util.hh ${HERE}
	${CXX} ${CXXFLAGS} -c util.cc ${LDFLAGS}