set key left box
set title "Maximum Insert Time"
set xlabel "Items (thousands)"
set ylabel "Microseconds"
#set xrange [1:100000]
#set yrange [0:0.00002]
set terminal png size 640, 480
set output "insert-maxtime-REPLACE1.png"
#set logscale y 10
#set logscale x 10
plot \
"insert-maxtime-REPLACE1.dat" using 1:2 with lines lc rgb "COLOR1" lw 2 title "REPLACE2", \
"insert-maxtime-REPLACE1.dat" using 1:3 with lines lc rgb "COLOR2" lw 2 title "REPLACE3"
#"./insert-maxtime_tree.dat" with lines lc rgb "green" lw 2 title "tree", \
#"./insert-maxtime_separate.dat" with lines lc rgb "purple" lw 2 title "separate hash table", \
#"./insert-maxtime_aho.dat" with lines lc rgb "orange" lw 2 title "aho hash table"#, \
#"./insert-maxtime_lazy.dat" with lines lc rgb "blue" lw 2 title "lazy hash table", \
#"./insert-maxtime_hash.dat" with lines lc rgb "red" lw 2 title "hash table"
