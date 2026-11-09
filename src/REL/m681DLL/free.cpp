#include "dolphin/types.h"

struct HeapNode {
    HeapNode *next, *previous, *chain;
    s32 size;
    u8 unk_10[0x10]; /* No code names this field. */
};
struct MemoryManager {
    HeapNode *first, *free;
    unsigned long count;
    MemoryManager *initialize(unsigned long);
    void *allocate(unsigned long);
    void release(void *);
};
typedef MemoryManager Heap;
extern "C" {
}
void fn_1_17328(Heap *, HeapNode *);
extern "C" {
}
void fn_1_17374(Heap *, void *, unsigned long);
extern "C" {
}
HeapNode *fn_1_17454(Heap *, unsigned long);
extern "C" {
void __msl_assertion_failed(const char *, const char *, int);
}


void *fn_1_17110(Heap *self, HeapNode *node, unsigned long bytes)
{
    if (node == 0) return 0;
    int remaining = node->size - bytes;
    if (remaining < 32) {
        node->size = -node->size;
        return node + 1;
    }
    HeapNode *split = (HeapNode *)((char *)node + bytes);
    split->next = node->next;
    split->previous = node;
    node->next->previous = split;
    node->next = split;
    node->size = -bytes;
    split->size = remaining;
    split->chain = self->first;
    self->first = split;
    ++self->count;
    return node + 1;
}

MemoryManager *MemoryManager::initialize(unsigned long bytes)
{
    MemoryManager *heap = this;
    bytes -= sizeof(*heap);
    fn_1_17374(heap, heap + 1, bytes);
    return heap;
}

void MemoryManager::release(void *data)
{
    s32 originalSize;
    HeapNode *allocationHeader;
    HeapNode *block;

    if (data != 0) {
        allocationHeader = (HeapNode *)data - 1;
        block = allocationHeader;

        if (block->size > 0) {
            (!"bad ptr") ? (void)0 :
                __msl_assertion_failed("!\"bad ptr\"", "free.cpp", 0x16);
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
            fn_1_17328(this, next);
        }

        HeapNode *previous = block->previous;
        if (previous->size > 0 && previous < block && previous->next == block) {
            previous->size += block->size;
            previous->next = block->next;
            previous->next->previous = previous;
            fn_1_17328(this, block);
        }
    }
}

void fn_1_17328(Heap *self,HeapNode *node) {
    --self->count; HeapNode **link=&self->first;
    while(*link!=node) { HeapNode *current=*link; link=&current->chain; }
    *link=node->chain;
}

void fn_1_17374(MemoryManager *self,void *memory,unsigned long bytes) {
    struct HeapRegion { HeapNode *first; void *original; } region;
    region.original=memory;
    region.first=(HeapNode *)(((unsigned long)region.original+31UL)&~31UL);
    bytes-=(unsigned long)region.first-(unsigned long)region.original; bytes&=~31UL;
    self->first=region.first; self->count=1; self->free=region.first;
    region.first->next=region.first; region.first->previous=region.first;
    region.first->size=bytes; region.first->chain=0;
}

void *MemoryManager::allocate(unsigned long bytes) {
    extern void *fn_1_17110(Heap *, HeapNode *, unsigned long);
    unsigned long rounded=((bytes+31)&~31UL)+32; bytes=rounded;
    return fn_1_17110(this,fn_1_17454(this,bytes),bytes);
}

HeapNode *fn_1_17454(Heap *self,unsigned long bytes) {
    HeapNode **link=&self->first;
    while(*link) {
        HeapNode *node=*link; int remaining=node->size-bytes;
        if(remaining>=0) { *link=node->chain; return node; }
        link=&node->chain;
    }
    return 0;
}
