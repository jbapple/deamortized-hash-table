#include <stdint.h>

#include "tlsf.h"

// The number of bytes in a word
#define word_bytes sizeof(size_t)
// The number of bits in a word
#define word_bits (word_bytes * 8)
// The log, base 2, of the number of bits in a word, assuming a word
// is 64, 32, or 16 bits long
#define word_log (3 + (18*word_bytes - word_bytes*word_bytes - 8)/24)

// A block of memory in a TLSF arena.
struct block {
  // left is the address of the block with the next smallest address,
  // assuming it can be coalesced with this block. Since blocks are
  // word-aligned and words are at least two bytes, the last bit of
  // left is always 0. We set it to 1 if this block is free.
  struct block * left;
  // The size of this block in bytes. Since size is always even, the
  // last bit of size is always 0. We set it to 1 if the block with
  // the next larger address can be coalesced with this block.
  size_t size;

  // The links to the left and right blocks set up the "coalescing
  // list", a doubly-linked list of blocks that can be coalesced with
  // their neighbors in the list.

  // If this block is free, then the payload contains garbage except
  // for payload[0] and payload[1], which are pointers in the
  // doubly-linked free list this block is a member of.
  struct block * payload[]; 
  // TODO: payload is word aligned, but some users may want it to be
  // more aligned for some mallocs. This should be configurable at the
  // time tlsf_malloc is called.
};

// big_buckets is the degree of the root of that tree.
#define big_buckets (word_bits - 2*word_log + 2)

// The TLSF free list directory is a trie of height 2. The first level
// is keyed off the binary logarithm of the size of the blocks, and
// the second level is keyed off the lower order bits next to the one
// bit with highest signifigance.

struct tlsf_arena {
  // TODO: better names than coarse and fine?
  // TODO: lock for multi-threading
  // TODO: move left counter into previous free block in case sizes support it 

  // coarse's bit i is set iff fine[i] is non-NULL
  size_t coarse;
  // fine[i]'s jth bit is set iff top[i][j] is non-NULL
  size_t fine[big_buckets];
  // Each non-NULL pointer in top[i][j] points to a free list in which
  // every block has size between
  //
  // (2^i-1) * word_bits + j 2^i + 2
  //
  // and
  //
  // (2^i-1) * word_bits + j 2^i + 2 + (2^i - 1)
  //
  // words
  struct block * top[big_buckets][word_bits];
};

void mask_set_bit(size_t * const x, const size_t i, const int b);
int mask_get_bit(const size_t x, const size_t i);

// A location is a path in the trie of free lists
struct location {
  uint8_t root, leaf;
};

struct location size_get_location(const size_t bytes);

size_t block_get_size(const struct block * const x);

struct block * block_get_left(struct block * const x);
struct block * block_get_right(struct block * const x);
int block_get_freedom(const struct block * const x);


