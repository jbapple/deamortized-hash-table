.PHONY: all
DATS=%_hash.dat %_tree.dat %_lazy.dat %_separate.dat %_aho.dat
.PRECIOUS: ${DATS} insert-maxtime.exe insert-maxtime.dat

all: insert-maxtime.png

CXX=g++
HERE=Makefile
CXXFLAGS=-std=c++0x -O3 -DNDEBUG -I..
LDFLAGS=-lrt
SUDO=sudo

#%.png: ${DATS} %.gnuplot ${HERE}
#	gnuplot $*.gnuplot

# ${DATS}: %.exe ${HERE}
# 	#${SUDO} ./$*.exe hash >$*_hash.dat
# 	${SUDO} ./$*.exe tree >$*_tree.dat
# 	#${SUDO} ./$*.exe lazy >$*_lazy.dat
# 	${SUDO} ./$*.exe separate >$*_separate.dat
# 	${SUDO} ./$*.exe aho >$*_aho.dat

insert-maxtime.dat: insert-maxtime.exe ${HERE}
	${SUDO} ./insert-maxtime.exe garbage >insert-maxtime.dat

insert-maxtime.png: insert-maxtime.gnuplot insert-maxtime.dat ${HERE}
	gnuplot insert-maxtime.gnuplot

%.exe: %.cc ${HERE}
	${CXX} ${CXXFLAGS} ../util.o $*.cc -o $@ ${LDFLAGS}