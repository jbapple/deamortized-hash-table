Deamortized Hash Tables
======================

Hash tables are usually very fast, but have some very slow operations when there are too many collisions or rehashing is required.

![](insert-cumulative.png "Cumulative Insert Times")

The discontinuities in the hash table times are artifacts of rehashing.
The trie is a little bit slower, but it is more consistent.
