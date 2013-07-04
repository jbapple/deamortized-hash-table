.PHONY: all
DATS=insert-maxtime-%.dat
.PRECIOUS: ${DATS} insert-maxtime.exe insert-maxtime.dat

all: insert-maxtime.png insert-tradeoff.png

CXX=g++
HERE=Makefile
CXXFLAGS=-W -Wall -Wextra -Weffc++ -std=c++0x -I.. -O3 -DNDEBUG
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

insert-tradeoff.png: insert-tradeoff.gnuplot insert-maxtime-0.dat insert-maxtime-6.dat insert-maxtime-1.dat ${HERE}
	gnuplot insert-tradeoff.gnuplot

insert-maxtime-%.dat: insert-maxtime.exe ${HERE}
	${SUDO} ./insert-maxtime.exe $* >insert-maxtime-$*.dat

insert-maxtime-0.png: insert-maxtime.gnuplot insert-maxtime-0.dat ${HERE}
	sed 's/REPLACE1/0/g' insert-maxtime.gnuplot | sed 's/REPLACE2/hash table/g' | sed 's/REPLACE3/tree/g' | sed 's/COLOR1/#aa0000/g' | sed 's/COLOR2/#00aa00/g' > insert-maxtime-0.gnuplot
	gnuplot insert-maxtime-0.gnuplot

insert-maxtime-1.png: insert-maxtime.gnuplot insert-maxtime-1.dat ${HERE}
	sed 's/REPLACE1/1/g' insert-maxtime.gnuplot | sed 's/REPLACE2/tree/g' | sed 's/REPLACE3/dummy/g' | sed 's/COLOR1/#000099/g' | sed 's/COLOR2/black/g' > insert-maxtime-1.gnuplot
	gnuplot insert-maxtime-1.gnuplot

insert-maxtime-2.png: insert-maxtime.gnuplot insert-maxtime-2.dat ${HERE}
	sed 's/REPLACE1/2/g' insert-maxtime.gnuplot | sed 's/REPLACE2/hash table/g' | sed 's/REPLACE3/partially deamortized/g' | sed 's/COLOR1/#aa0000/g' | sed 's/COLOR2/#ff9900/g' > insert-maxtime-2.gnuplot
	gnuplot insert-maxtime-2.gnuplot

insert-maxtime-3.png: insert-maxtime.gnuplot insert-maxtime-3.dat ${HERE}
	sed 's/REPLACE1/3/g' insert-maxtime.gnuplot | sed 's/REPLACE2/partially deamortized/g' | sed 's/REPLACE3/tree/g' | sed 's/COLOR1/#ff9900/g' | sed 's/COLOR2/#00aa00/g' > insert-maxtime-3.gnuplot
	gnuplot insert-maxtime-3.gnuplot

insert-maxtime-4.png: insert-maxtime.gnuplot insert-maxtime-4.dat ${HERE}
	sed 's/REPLACE1/4/g' insert-maxtime.gnuplot | sed 's/REPLACE2/hash table/g' | sed 's/REPLACE3/dummy/g' | sed 's/COLOR1/#990000/g' | sed 's/COLOR2/black/g' > insert-maxtime-4.gnuplot
	gnuplot insert-maxtime-4.gnuplot

insert-maxtime-5.png: insert-maxtime.gnuplot insert-maxtime-5.dat ${HERE}
	sed 's/REPLACE1/5/g' insert-maxtime.gnuplot | sed 's/REPLACE2/partially deamortized/g' | sed 's/REPLACE3/dummy/g' | sed 's/COLOR1/#663300/g' | sed 's/COLOR2/black/g' > insert-maxtime-5.gnuplot
	gnuplot insert-maxtime-5.gnuplot

insert-maxtime-6.png: insert-maxtime.gnuplot insert-maxtime-6.dat ${HERE}
	sed 's/REPLACE1/6/g' insert-maxtime.gnuplot | sed 's/REPLACE2/tiered deamortized/g' | sed 's/REPLACE3/dummy/g' | sed 's/COLOR1/#336600/g' | sed 's/COLOR2/black/g' > insert-maxtime-6.gnuplot
	gnuplot insert-maxtime-6.gnuplot

insert-maxtime-7.png: insert-maxtime.gnuplot insert-maxtime-7.dat ${HERE}
	sed 's/REPLACE1/7/g' insert-maxtime.gnuplot | sed 's/REPLACE2/tiered with mmap/g' | sed 's/REPLACE3/dummy/g' | sed 's/COLOR1/#009900/g' | sed 's/COLOR2/black/g' > insert-maxtime-7.gnuplot
	gnuplot insert-maxtime-7.gnuplot

insert-maxtime-8.png: insert-maxtime.gnuplot insert-maxtime-8.dat ${HERE}
	sed 's/REPLACE1/8/g' insert-maxtime.gnuplot | sed 's/REPLACE2/both mmap/g' | sed 's/REPLACE3/dummy/g' | sed 's/COLOR1/#006633/g' | sed 's/COLOR2/black/g' > insert-maxtime-8.gnuplot
	gnuplot insert-maxtime-8.gnuplot

#insert-maxtime-9.png: insert-maxtime.gnuplot insert-maxtime-9.dat ${HERE}
#	sed 's/REPLACE1/9/g' insert-maxtime.gnuplot | sed 's/REPLACE2/btree/g' | sed 's/REPLACE3/dummy/g' | sed 's/COLOR1/#0000ff/g' | sed 's/COLOR2/black/g' > insert-maxtime-9.gnuplot
#	gnuplot insert-maxtime-9.gnuplot

%.exe: %.cc ${HERE} ../util.hh ../util.cc #make_realtime.sh
	${CXX} ${CXXFLAGS} ../util.o $*.cc -o $@ ${LDFLAGS}
#	./make_realtime.sh $@

make_realtime.sh:
	sudo su -c "echo 'setcap cap_sys_nice+ep \$$1'>$@"
	sudo chmod ugo+s $@