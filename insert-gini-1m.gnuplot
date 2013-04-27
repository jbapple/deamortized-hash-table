set key right box
set title "Insert Time from Slowest to Fastest"
set xlabel "Items Ordered by Insert Time"
set ylabel "Percent"
set xrange [1:100000]
set terminal png size 800, 600
set logscale x
# set yrange [0:10]
set output "insert-gini-1m.png"
plot "./std-gini-avg-set-1M-100.dat" with lines title "trie" lc rgb "green" lw 3, "./std-gini-avg-unordered_set-1M-100.dat" with lines title "hash table" lc rgb "red" lw 3
set logscale y
set output "insert-gini-1m-logscale.png"
plot "./std-gini-avg-set-1M-100.dat" with lines title "trie" lc rgb "green" lw 3, "./std-gini-avg-unordered_set-1M-100.dat" with lines title "hash table" lc rgb "red" lw 3
