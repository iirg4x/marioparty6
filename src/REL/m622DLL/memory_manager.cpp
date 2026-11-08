/* Aligns an initial free block and installs it in the heap. */
#include "REL/framework/heap.h"

void MemoryManager::initializeBlocks(void *memory, unsigned long bytes)
{
    struct HeapRegion { HeapNode *first; void *original; } region;
    region.original = memory;
    region.first = (HeapNode *)(((unsigned long)region.original + 31UL) & ~31UL);
    bytes -= (unsigned long)region.first - (unsigned long)region.original;
    bytes &= ~31UL;
    this->first = region.first;
    this->count = 1;
    this->free = region.first;
    region.first->next = region.first;
    region.first->previous = region.first;
    region.first->size = bytes;
    region.first->chain = 0;
}


void *MemoryManager::allocate(unsigned long bytes) {
    unsigned long rounded=((bytes+31)&~31UL)+32; bytes=rounded;
    return this->allocateBlock(this->findFreeBlock(bytes),bytes);
}

HeapNode *MemoryManager::findFreeBlock(unsigned long bytes) {
    HeapNode **link=&first;
    while(*link) {
        HeapNode *node=*link; int remaining=node->size-bytes;
        if(remaining>=0) { *link=node->chain; return node; }
        link=&node->chain;
    }
    return 0;
}
