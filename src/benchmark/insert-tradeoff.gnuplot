#set key left box
set title "Average vs. Maximum Insert Time"
set xlabel "Average Insert Time (microseconds)"
set ylabel "Maximum Insert Time (microseconds)"
#set xrange [1:100000]
#set yrange [0:0.00002]
set terminal pngcairo rounded size 640, 400
set output "insert-tradeoff.png"
set logscale y 10
#set logscale x 10
# plot "./insert-maxtime-4.dat" using 6:4:(log((($4 * 3)>1)?($4 * 3):1)) pt 7 ps variable lc rgb "#aa0000", "./insert-maxtime-5.dat" using 6:4:(log((($4 * 3)>1)?($4 * 3):1)) pt 7 ps variable lc rgb "#ff9900", "./insert-maxtime-1.dat" using 6:4:(log((($4*3)>1)?($4*3):1)) pt 7 ps variable lc rgb "#00aa00"

plot \
     "./insert-maxtime-6.dat" using 6:4:(log((($4 * 3)>1)?($4 * 3):1)) pt 7 ps variable lc rgb "#cccc00" title "tiered hash", \
     "./insert-maxtime-7.dat" using 6:4:(log((($4 * 3)>1)?($4 * 3):1)) pt 7 ps variable lc rgb "#66cc00" title "tiered with mmap", \
     "./insert-maxtime-5.dat" using 6:4:(log((($4 * 3)>1)?($4 * 3):1)) pt 7 ps variable lc rgb "#cc6600" title "partially deamortized", \
     "./insert-maxtime-4.dat" using 6:4:(log((($4 * 3)>1)?($4 * 3):1)) pt 7 ps variable lc rgb "#cc0000" title "hash table", \
     "./insert-maxtime-1.dat" using 6:4:(log((($4 * 3)>1)?($4 * 3):1)) pt 7 ps variable lc rgb "#0000cc" title "tree"


     

#plot \
#"insert-maxtime-REPLACE1.dat" using 1:2 with lines lc rgb "COLOR1" lw 2 title "REPLACE2", \
#"insert-maxtime-REPLACE1.dat" using 1:3 with lines lc rgb "COLOR2" lw 2 title "REPLACE3"
#"./insert-maxtime_tree.dat" with lines lc rgb "green" lw 2 title "tree", \
#"./insert-maxtime_separate.dat" with lines lc rgb "purple" lw 2 title "separate hash table", \
#"./insert-maxtime_aho.dat" with lines lc rgb "orange" lw 2 title "aho hash table"#, \
#"./insert-maxtime_lazy.dat" with lines lc rgb "blue" lw 2 title "lazy hash table", \
#"./insert-maxtime_hash.dat" with lines lc rgb "red" lw 2 title "hash table"
