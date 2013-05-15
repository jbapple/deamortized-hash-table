set key left box
set title "Cumulative Insert Time"
set xlabel "Unique Items"
set ylabel "Seconds"
set xrange [0:1024]
set terminal png size 800, 600
set output "insert-cumulative-example.png"
plot "./std-set-example-1024-30.dat" with lines lc rgb "green" title "trie", "./std-unordered_set-example-1024-30.dat" with lines lc rgb "red" title "hash table"