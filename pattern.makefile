HERE=pattern.makefile
CXXFLAGS=-std=c++0x -O3 -DNDEBUG
LDFLAGS=-lrt

%.png: %_hash.dat %_tree.dat %.gnuplot ${HERE}
	gnuplot $*.gnuplot

%_hash.dat %_tree.dat: %.exe ${HERE}
	sudo ./$*.exe hash >$*_hash.dat
	sudo ./$*.exe tree >$*_tree.dat

%.exe: util.o %.cc ${HERE} 
	${CXX} ${CXXFLAGS} util.o $*.cc -o $@ ${LDFLAGS}

util.o: util.cc ${HERE}