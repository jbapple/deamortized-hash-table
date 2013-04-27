insert-benchmark.exe: insert-benchmark.cc util.hh util.o Makefile
	clang++ -std=c++0x insert-benchmark.cc util.o -o insert-benchmark.exe -lrt -O3 -DNDEBUG

util.o: util.hh util.cc Makefile
	clang++ -std=c++0x -c util.cc -o util.o -O3 -DNDEBUG

std-set-1024-30.dat: insert-benchmark.exe Makefile
	./insert-benchmark.exe "std::set" 1024 30 >std-set-1024-30.dat

std-unordered_set-1024-30.dat: insert-benchmark.exe Makefile
	./insert-benchmark.exe "std::set" 1024 30 >std-unordered_set-1024-30.dat

insert-cumulative.png: std-set-1024-30.dat std-unordered_set-1024-30.dat Makefile insert-cumulative.gnuplot
	gnuplot insert-cumulative.gnuplot

