#include "dolphin/types.h"
#include "REL/framework/heap.h"
extern "C" {
char lbl_1_data_15F8[] = "bad ptr";
char lbl_1_data_1600[] = "!\"bad ptr\"";
char lbl_1_data_160B[] = "free.cpp";
void __msl_assertion_failed(const char *, const char *, int);
}










void MemoryManager::release(void *data)
{
    s32 originalSize;
    HeapNode *allocationHeader;
    HeapNode *block;

    if (data != 0) {
        allocationHeader = (HeapNode *)data - 2;
        block = allocationHeader;

        if (block->size > 0) {
            (&lbl_1_data_15F8 == 0) ? (void)0 :
                __msl_assertion_failed(lbl_1_data_1600, lbl_1_data_160B, 0x16);
        }

        originalSize = block->size;
        block->size = -originalSize;
        block->chain = this->first;
        this->first = block;

        HeapNode *next = block->next;
        if (next->size > 0 && next > block && next->previous == block) {
            block->size += next->size;
            block->next = next->next;
            next->next->previous = block;
            unlinkFreeBlock(next);
        }

        HeapNode *previous = block->previous;
        if (previous->size > 0 && previous < block && previous->next == block) {
            previous->size += block->size;
            previous->next = block->next;
            previous->next->previous = previous;
            unlinkFreeBlock(block);
        }
    }
}

void MemoryManager::unlinkFreeBlock(HeapNode *node) {
    --this->count; HeapNode **link=&this->first;
    while(*link!=node) { HeapNode *current=*link; link=&current->chain; }
    *link=node->chain;
}
void MemoryManager::initializeBlocks(void *memory,unsigned long bytes) {
    struct HeapRegion { HeapNode *first; void *original; } region;
    region.original=memory;
    region.first=(HeapNode *)(((unsigned long)region.original+31UL)&~31UL);
    bytes-=(unsigned long)region.first-(unsigned long)region.original; bytes&=~31UL;
    this->first=region.first; this->count=1; this->free=region.first;
    region.first->next=region.first; region.first->previous=region.first;
    region.first->size=bytes; region.first->chain=0;
}
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
