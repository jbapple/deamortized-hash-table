.PHONY: all
DATS=insert-maxtime-%.dat
.PRECIOUS: ${DATS} insert-maxtime.exe insert-maxtime.dat

all: insert-maxtime.png insert-tradeoff.png

CXX=clang++
HERE=Makefile
CXXFLAGS=-W -Wall -Wextra -Weffc++ -std=c++0x -O3 -DNDEBUG -I..
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

insert-tradeoff.png: insert-tradeoff.gnuplot insert-maxtime-4.dat insert-maxtime-5.dat insert-maxtime-1.dat ${HERE}
	gnuplot insert-tradeoff.gnuplot

insert-maxtime-%.dat: insert-maxtime.exe ${HERE}
	${SUDO} ./insert-maxtime.exe $* >insert-maxtime-$*.dat

insert-maxtime-0.png: insert-maxtime.gnuplot insert-maxtime-0.dat ${HERE}
	sed 's/REPLACE1/0/g' insert-maxtime.gnuplot | sed 's/REPLACE2/hash table/g' | sed 's/REPLACE3/tree/g' | sed 's/COLOR1/red/g' | sed 's/COLOR2/green/g' > insert-maxtime-0.gnuplot
	gnuplot insert-maxtime-0.gnuplot

insert-maxtime-1.png: insert-maxtime.gnuplot insert-maxtime-1.dat ${HERE}
	sed 's/REPLACE1/1/g' insert-maxtime.gnuplot | sed 's/REPLACE2/tree/g' | sed 's/REPLACE3/dummy/g' | sed 's/COLOR1/green/g' | sed 's/COLOR2/blue/g' > insert-maxtime-1.gnuplot
	gnuplot insert-maxtime-1.gnuplot

insert-maxtime-2.png: insert-maxtime.gnuplot insert-maxtime-2.dat ${HERE}
	sed 's/REPLACE1/2/g' insert-maxtime.gnuplot | sed 's/REPLACE2/hash table/g' | sed 's/REPLACE3/partially deamortized/g' | sed 's/COLOR1/red/g' | sed 's/COLOR2/orange/g' > insert-maxtime-2.gnuplot
	gnuplot insert-maxtime-2.gnuplot

insert-maxtime-3.png: insert-maxtime.gnuplot insert-maxtime-3.dat ${HERE}
	sed 's/REPLACE1/3/g' insert-maxtime.gnuplot | sed 's/REPLACE2/partially deamortized/g' | sed 's/REPLACE3/tree/g' | sed 's/COLOR1/orange/g' | sed 's/COLOR2/green/g' > insert-maxtime-3.gnuplot
	gnuplot insert-maxtime-3.gnuplot

insert-maxtime-4.png: insert-maxtime.gnuplot insert-maxtime-4.dat ${HERE}
	sed 's/REPLACE1/4/g' insert-maxtime.gnuplot | sed 's/REPLACE2/hash table/g' | sed 's/REPLACE3/dummy/g' | sed 's/COLOR1/red/g' | sed 's/COLOR2/blue/g' > insert-maxtime-4.gnuplot
	gnuplot insert-maxtime-4.gnuplot

insert-maxtime-5.png: insert-maxtime.gnuplot insert-maxtime-5.dat ${HERE}
	sed 's/REPLACE1/5/g' insert-maxtime.gnuplot | sed 's/REPLACE2/partially deamortized/g' | sed 's/REPLACE3/dummy/g' | sed 's/COLOR1/orange/g' | sed 's/COLOR2/blue/g' > insert-maxtime-4.gnuplot
	gnuplot insert-maxtime-5.gnuplot

%.exe: %.cc ${HERE} ../util.hh ../util.cc #make_realtime.sh
	${CXX} ${CXXFLAGS} ../util.o $*.cc -o $@ ${LDFLAGS}
#	./make_realtime.sh $@

make_realtime.sh:
	sudo su -c "echo 'setcap cap_sys_nice+ep \$$1'>$@"
	sudo chmod ugo+s $@