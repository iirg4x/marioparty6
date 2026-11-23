/* Heap allocation and release operations shared by the voice SDK. */
#ifndef GSSDK_HEAP_H
#define GSSDK_HEAP_H

#include "types.h"

/* Opens the heap using the client-supplied value and returns its handle. */
u32 heap_Open(void **heapHandle, unsigned long heapUserData);

/* Releases the allocations associated with the heap. */
u32 heap_Close(void *heapHandle);

/* Allocates a zeroed array. */
void *heap_Calloc(void *heapHandle, u32 elementCount, u32 elementSize);

/* Allocates a block of the requested size. */
void *heap_Alloc(void *heapHandle, u32 byteCount);

/* Resizes a block and preserves its existing contents up to the new size. */
void *heap_Realloc(void *heapHandle, void *existingBlock, u32 byteCount);

/* Releases an allocation to its heap. */
void heap_Free(void *heapHandle, void *block);

#endif
