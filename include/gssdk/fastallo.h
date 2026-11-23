/* Chunk allocation and free lists for small voice SDK allocations. */
#ifndef GSSDK_FASTALLO_H
#define GSSDK_FASTALLO_H

#include "types.h"

/* Next entry in a free list, overlaid on the first word of a free allocation. */
struct _FASTALLO_FREE_NODE {
    /* Next free allocation of the same size. */
    struct _FASTALLO_FREE_NODE *pNext;
};

/* Small-block allocator with one free list for each size in words. */
struct _FASTALLO_INST {
    /* Heap that owns the chunks. */
    void *hHeap;
    /* Size allocated for each new chunk. */
    unsigned short ChunkSize;
    /* Unused words in the current chunk. */
    unsigned short FreeMemorySize;
    /* Number of allocated chunks. */
    unsigned long NbrArrs;
    /* Allocations to release when the allocator terminates. */
    void *pArrs;
    /* Next unused word in the current chunk. */
    struct _FASTALLO_FREE_NODE *pFreeMemory;
    /* Free-list heads indexed by allocation size in words. */
    struct _FASTALLO_FREE_NODE *pFreeNodes[65];
};

/* Initializes an empty allocator with its heap and chunk size. */
extern void fastallo_Init(
    struct _FASTALLO_INST *pInst, void *hHeap, unsigned short ChunkSize);

/* Allocates Size words, from 1 to 64, and obtains a new chunk when needed. */
extern void *fastallo_AllocateMemoryFromChunk(struct _FASTALLO_INST *pInst, unsigned short Size);

/* Releases all chunks and the list that tracks them. */
extern void fastallo_Terminate(struct _FASTALLO_INST *pInst);

/* Takes a fixed-size block from the free list or allocates it from the current chunk. */
#define FASTALLO_TAKE(Heap, Size, Destination) \
    { \
        unsigned int TempSize = (Size); \
        struct _FASTALLO_FREE_NODE **pTemp = &(Heap)->pFreeNodes[TempSize]; \
        void *_pBl = (Destination); \
        if (*pTemp != NULL) { \
            *(void **)_pBl = *pTemp; \
            *pTemp = (*pTemp)->pNext; \
        } else { \
            *(void **)_pBl = fastallo_AllocateMemoryFromChunk((Heap), TempSize); \
        } \
    }

#endif
