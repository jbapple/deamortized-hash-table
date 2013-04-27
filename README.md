Deamortized Hash Tables
======================

Hash tables are usually very fast, but have some very slow operations when there are too many collisions or rehashing is required.

![](insert-cumulative.png "Cumulative Insert Times")

The discontinuities in the hash table times are artifacts of rehashing.
The trie is a little bit slower, but it is more consistent.

If we order the insert times from slowest to fastest, it's clear that slow trie operations are usually much faster than slow hash table operations:

![](insert-gini.png "Insert Times From Slowest to Fastest")
