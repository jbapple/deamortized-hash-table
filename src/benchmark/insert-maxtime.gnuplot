set key left box
set title "Average Maximum Insert Time"
set xlabel "Items"
set ylabel "Seconds"
#set xrange [1:100000]
set terminal png size 800, 600
set output "insert-maxtime.png"
#set logscale y 10
#set logscale x 10
plot "./insert-maxtime_tree.dat" with lines lc rgb "green" lw 2 title "tree", "./insert-maxtime_hash.dat" with lines lc rgb "red" lw 2 title "hash table"