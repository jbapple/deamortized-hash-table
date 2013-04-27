set key right box
set title "Insert Time from Slowest to Fastest"
set xlabel "Items Ordered by Insert Time"
set ylabel "Percent"
set xrange [1:1024]
set terminal png size 800, 600
set output "insert-gini.png"
set logscale x
set logscale y
plot "./std-gini-set-1024-300.dat" ps 1 title "trie" with lines, "./std-gini-unordered_set-1024-300.dat" ps 0.333 lc rgb "black" title "hash table" with lines