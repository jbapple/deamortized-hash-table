set key left box
set title "Average Cumulative Insert Time"
set xlabel "Unique Items"
set ylabel "Seconds"
set xrange [1:100000]
set terminal png size 800, 600
set output "insert-maxtime.png"
set logscale y 10
set logscale x 10
plot "./std-set-maxtime-1024-30.dat" with lines lc rgb "green" lw 2 title "trie", "./std-unordered_set-maxtime-1024-30.dat" with lines lc rgb "red" lw 2 title "hash table", "./linear-maxtime-1024-30.dat" with lines lc rgb "blue" lw 2 title "linear"