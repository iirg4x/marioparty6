#ifndef REL_M672DLL_HEAP_H
#define REL_M672DLL_HEAP_H
#include "dolphin/types.h"

struct HeapNode {
    HeapNode *next;
    HeapNode *previous;
    HeapNode *chain;
    s32 size;
    /* The allocator reserves this part of the 0x20-byte prefix; game code never reads it. */
    unsigned char unknown_10[0x10]; /* No code names this field. */
};
class MemoryManager {
public:
    HeapNode *first;
    HeapNode *free;
    unsigned long count;
    MemoryManager *initialize(unsigned long);
    void *allocateBlock(HeapNode *, unsigned long);
    void release(void *);
    void unlinkFreeBlock(HeapNode *);
    void initializeBlocks(void *, unsigned long);
    void *allocate(unsigned long);
    HeapNode *findFreeBlock(unsigned long);
};
void initializeMemoryManager();
#endif
