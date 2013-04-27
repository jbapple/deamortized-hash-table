Deamortized Hash Tables
======================

Hash tables are usually very fast, but have some very slow operations when there are too many collisions or rehashing is required.

![](insert-cumulative.png "Cumulative Insert Times")

The discontinuities in the `std::unordered_set` times are artifacts of rehashing.