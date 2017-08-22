#ifndef MSG_RING_H
#define MSG_RING_H

#include "mem/block.h"

struct MemRing {
    MemBlock buf;
    size_t front;
    size_t len;
};
typedef struct MemRing MemRing;

/**
 * Allocates a memory ring of the given capacity in bytes.
 */
MemRing mem_ring_alloc(size_t capacity);

/**
 * Frees the memory ring, effectively resizing it to size zero.
 */
void mem_ring_free(MemRing* ring);

/**
 * Obtains a reference to the oldest peek_len bytes in the queue by overwriting
 * the given pointer with the address of the first byte.
 *
 * Note that the capacity has to be a multiple of the peek_len in order for this
 * function to correctly wrap around a full buffer.
 *
 * If no data is available, returns false, otherwise true.
 */
bool mem_ring_peek(MemRing* ring, void** peek_addr, size_t peek_len);

/**
 * Copies the oldest buf_len bytes into the given buffer and discards the data in
 * the queue after copying, making room for enqueuing.
 * 
 * If not enough data is available, returns false, otherwise true.
 */
bool mem_ring_dequeue(MemRing* ring, void* buf, size_t buf_len);

/**
 * Copies the given buffer into the ring buffer, making the data available
 * for dequeuing in the order it was enqueued.
 *
 * Returns false if not enough space is available to enqueue the whole buffer.
 */
bool mem_ring_enqueue(MemRing* ring, void* buf, size_t buf_len);

#endif // MSG_RING_H