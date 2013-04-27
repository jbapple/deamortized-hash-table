insert-benchmark.exe: insert-benchmark.cc util.hh util.o Makefile
	clang++ -std=c++0x insert-benchmark.cc util.o -o insert-benchmark.exe -lrt

util.o: util.hh util.cc Makefile
	clang++ -std=c++0x -c util.cc -o util.o