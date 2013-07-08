set key left box
set title "Average Maximum Lookup Time"
set xlabel "Unique Items"
set ylabel "Seconds"
#set xrange [1:100000]
#set yrange [0.0000001:0.000001]
set logscale y 10
set logscale x 10
set terminal png size 800, 600
set output "lookup-maxtime.png"
plot "./lookup-std-set-maxtime-1024-30.dat" ps 0.1 lc rgb "green" title "trie", "./lookup-linear-maxtime-1024-30.dat" ps 0.2 lc rgb "blue" title "linear", "./lookup-std-unordered_set-maxtime-1024-30.dat" ps 0.01 lc rgb "red" title "hash table"