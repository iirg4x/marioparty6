#include "REL/m672DLL/heap.h"

void *MemoryManager::allocate(unsigned long bytes) {
    unsigned long rounded=((bytes+31)&~31UL)+32; bytes=rounded;
    return allocateBlock(findFreeBlock(bytes),bytes);
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
