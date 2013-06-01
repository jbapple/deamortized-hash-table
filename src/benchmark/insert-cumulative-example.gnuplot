set key left box
set title "Average Cumulative Insert Time"
set xlabel "Unique Items"
set ylabel "Seconds"
set xrange [0:10240]
set terminal png size 800, 600
set output "insert-cumulative-example.png"
#set logscale y 2
#set logscale x 2
plot "./std-set-example-1024-30.dat" with lines lc rgb "green" lw 2 title "trie", "./std-unordered_set-example-1024-30.dat" with lines lc rgb "red" lw 2 title "hash table", "./linear-example-1024-30.dat" with lines lc rgb "blue" lw 2 title "linear"