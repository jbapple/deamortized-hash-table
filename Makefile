insert-benchmark.exe: insert-benchmark.cc util.hh util.o Makefile
	clang++ -std=c++0x insert-benchmark.cc util.o -o insert-benchmark.exe -lrt -O3 -DNDEBUG

insert-gini.exe: insert-gini.cc util.hh util.o Makefile
	clang++ -std=c++0x insert-gini.cc util.o -o insert-gini.exe -lrt -O3 -DNDEBUG

util.o: util.hh util.cc Makefile
	clang++ -std=c++0x -c util.cc -o util.o -O3 -DNDEBUG

std-set-1024-30.dat: insert-benchmark.exe Makefile
	./insert-benchmark.exe "std::set" 1024 30 >std-set-1024-30.dat

std-unordered_set-1024-30.dat: insert-benchmark.exe Makefile
	./insert-benchmark.exe "std::unordered_set" 1024 30 >std-unordered_set-1024-30.dat

std-gini-set-1024-300.dat: insert-gini.exe Makefile
	./insert-gini.exe "all" "std::set" 1024 300 >std-gini-set-1024-300.dat

std-gini-unordered_set-1024-300.dat: insert-gini.exe Makefile
	./insert-gini.exe "all" "std::unordered_set" 1024 300 >std-gini-unordered_set-1024-300.dat

std-gini-avg-set-1024-300.dat: insert-gini.exe Makefile
	./insert-gini.exe "avg" "std::set" 1024 300 >std-gini-avg-set-1024-300.dat

std-gini-avg-unordered_set-1024-300.dat: insert-gini.exe Makefile
	./insert-gini.exe "avg" "std::unordered_set" 1024 300 >std-gini-avg-unordered_set-1024-300.dat

insert-cumulative.png: std-set-1024-30.dat std-unordered_set-1024-30.dat Makefile insert-cumulative.gnuplot
	gnuplot insert-cumulative.gnuplot

insert-gini.png: std-gini-set-1024-300.dat std-gini-unordered_set-1024-300.dat std-gini-avg-set-1024-300.dat std-gini-avg-unordered_set-1024-300.dat Makefile insert-gini.gnuplot
	gnuplot insert-gini.gnuplot

std-gini-avg-set-1M-100.dat: insert-gini.exe Makefile
	./insert-gini.exe "avg" "std::set" 100000 100 >std-gini-avg-set-1M-100.dat

std-gini-avg-unordered_set-1M-100.dat: insert-gini.exe Makefile
	./insert-gini.exe "avg" "std::unordered_set" 100000 100 >std-gini-avg-unordered_set-1M-100.dat

insert-gini-1m.png insert-gini-1m-logscale.png: std-gini-avg-set-1M-100.dat std-gini-avg-unordered_set-1M-100.dat Makefile insert-gini-1m.gnuplot
	gnuplot insert-gini-1m.gnuplot

##########################

insert-cumulative-example.png: std-set-example-1024-30.dat std-unordered_set-example-1024-30.dat Makefile insert-cumulative-example.gnuplot linear-example-1024-30.dat
	gnuplot insert-cumulative-example.gnuplot

std-set-example-1024-30.dat: insert-benchmark-example.exe Makefile
	./insert-benchmark-example.exe "std::set" 10240 30 >std-set-example-1024-30.dat

std-unordered_set-example-1024-30.dat: insert-benchmark-example.exe Makefile
	./insert-benchmark-example.exe "std::unordered_set" 10240 30 >std-unordered_set-example-1024-30.dat

linear-example-1024-30.dat: insert-benchmark-example.exe Makefile
	./insert-benchmark-example.exe "linear-probing" 10240 30 >linear-example-1024-30.dat

insert-benchmark-example.exe: insert-benchmark-example.cc util.hh util.o Makefile src/linear-probing.cc
	clang++ -std=c++0x insert-benchmark-example.cc util.o -o insert-benchmark-example.exe -lrt -O3 -DNDEBUG

# TODO: make sure vector.push_back isn't changing benchmark results
# TODO: make sure reading rand values is subtracted off performance

#################################


insert-maxtime.png: std-set-maxtime-1024-30.dat std-unordered_set-maxtime-1024-30.dat Makefile insert-maxtime.gnuplot linear-maxtime-1024-30.dat
	gnuplot insert-maxtime.gnuplot

std-set-maxtime-1024-30.dat: insert-maxtime.exe Makefile
	./insert-maxtime.exe "std::set" 100000 30 >std-set-maxtime-1024-30.dat

std-unordered_set-maxtime-1024-30.dat: insert-maxtime.exe Makefile
	./insert-maxtime.exe "std::unordered_set" 100000 30 >std-unordered_set-maxtime-1024-30.dat

linear-maxtime-1024-30.dat: insert-maxtime.exe Makefile
	./insert-maxtime.exe "linear-probing" 100000 30 >linear-maxtime-1024-30.dat

insert-maxtime.exe: insert-maxtime.cc util.hh util.o Makefile src/linear-probing.cc
	clang++ -std=c++0x insert-maxtime.cc util.o -o insert-maxtime.exe -lrt -O3 -DNDEBUG
