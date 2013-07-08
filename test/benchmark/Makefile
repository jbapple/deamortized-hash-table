include ../Makefile.common

.PHONY: all
DATS=output/insert-maxtime-%.dat
.PRECIOUS: ${DATS} insert-maxtime.exe

all: output/insert-tradeoff.png

output/insert-tradeoff.png: insert-tradeoff.gnuplot output/insert-maxtime-0.dat output/insert-maxtime-6.dat output/insert-maxtime-1.dat ${HERE}
	gnuplot insert-tradeoff.gnuplot

output/insert-maxtime-%.dat: insert-maxtime.exe ${HERE}
	./insert-maxtime.exe $* >output/insert-maxtime-$*.dat

%.exe: %.cc util.o ${HERE}
	${CXX} ${CXXFLAGS} -I../../src util.o $*.cc -o $@ ${LDFLAGS}

util.o: util.cc util.hh ${HERE}
	${CXX} ${CXXFLAGS} -c util.cc ${LDFLAGS}