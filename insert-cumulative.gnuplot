set key left box
set title "Cumulative Insert Time"
set xlabel "Unique Items"
set ylabel "Seconds"
set xrange [0:1024]
set terminal png size 900, 600
set output "insert-cumulative.png"
plot "./std-set-1024-30.dat" ps 1 title "std::set<int>", "./std-unordered_set-1024-30.dat" ps 0.333 lc rgb "black" title "std::unordered_set<int>"