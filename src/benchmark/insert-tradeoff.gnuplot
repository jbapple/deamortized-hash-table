#set key left box
set title "Average vs. Maximum Insert Time"
set xlabel "Average Insert Time (microseconds)"
set ylabel "Maximum Insert Time (microseconds)"
#set xrange [1:100000]
#set yrange [0:0.00002]
set terminal pngcairo size 800, 480
set output "insert-tradeoff.png"
set logscale y 10
#set logscale x 10
plot \
     "./insert-maxtime-5.dat" using 6:4 ps 0.2 lc rgb "#ff9900" title "partially deamortized", \
     "./insert-maxtime-4.dat" using 6:4 ps 0.7 lc rgb "#aa0000" title "hash table", \
     "./insert-maxtime-1.dat" using 6:4 ps 0.4 lc rgb "#00aa00" title "tree"

#plot \
#"insert-maxtime-REPLACE1.dat" using 1:2 with lines lc rgb "COLOR1" lw 2 title "REPLACE2", \
#"insert-maxtime-REPLACE1.dat" using 1:3 with lines lc rgb "COLOR2" lw 2 title "REPLACE3"
#"./insert-maxtime_tree.dat" with lines lc rgb "green" lw 2 title "tree", \
#"./insert-maxtime_separate.dat" with lines lc rgb "purple" lw 2 title "separate hash table", \
#"./insert-maxtime_aho.dat" with lines lc rgb "orange" lw 2 title "aho hash table"#, \
#"./insert-maxtime_lazy.dat" with lines lc rgb "blue" lw 2 title "lazy hash table", \
#"./insert-maxtime_hash.dat" with lines lc rgb "red" lw 2 title "hash table"
