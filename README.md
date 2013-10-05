This library is still in development; it is not ready for production use.

Deamortized Hash Tables
======================

Hash tables are usually very fast, but have some very slow operations
when rehashing is required or there are too many hash collisions.
This project provides a deamortized hash table that reduces the
worst-case insert time.  This reduction has a cost: the average insert
time increases.  For some applications, this may be a good tradeoff.

