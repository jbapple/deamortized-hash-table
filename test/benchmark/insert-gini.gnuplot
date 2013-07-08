set key right box
set title "Insert Time from Slowest to Fastest"
set xlabel "Items Ordered by Insert Time"
set ylabel "Percent"
set xrange [1:1024]
set terminal png size 800, 600
set output "insert-gini.png"
set logscale x
# set logscale y
set yrange [0:10]
plot "./std-gini-set-1024-300.dat" ps 1  lc rgb "#00ff00" title "trie", "./std-gini-unordered_set-1024-300.dat" ps 0.5 lc rgb "#ff8888" title "hash table", "./std-gini-avg-set-1024-300.dat" title "trie average" with lines lw 3 lc rgb "#008800", "./std-gini-avg-unordered_set-1024-300.dat" title "hash table average" with lines lw 3 lc rgb "#ff0000"