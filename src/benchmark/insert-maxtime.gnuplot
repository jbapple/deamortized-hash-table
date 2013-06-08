set key left box
set title "Average Maximum Insert Time"
set xlabel "Items"
set ylabel "Seconds"
#set xrange [1:100000]
#set yrange [0:0.00002]
set terminal png size 800, 600
set output "insert-maxtime.png"
set logscale y 10
set logscale x 10
plot \
"insert-maxtime.dat" using 1:2 with lines lc rgb "red" lw 1 title "separate", \
"insert-maxtime.dat" using 1:3 with lines lc rgb "blue" lw 1 title "aho"
#"./insert-maxtime_tree.dat" with lines lc rgb "green" lw 2 title "tree", \
#"./insert-maxtime_separate.dat" with lines lc rgb "purple" lw 2 title "separate hash table", \
#"./insert-maxtime_aho.dat" with lines lc rgb "orange" lw 2 title "aho hash table"#, \
#"./insert-maxtime_lazy.dat" with lines lc rgb "blue" lw 2 title "lazy hash table", \
#"./insert-maxtime_hash.dat" with lines lc rgb "red" lw 2 title "hash table"
