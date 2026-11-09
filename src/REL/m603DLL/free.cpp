#include "REL/framework/heap.h"
#include "REL/m603DLL/singleton_32930.h"

extern "C" {
extern char lbl_1_data_D00[];
extern char lbl_1_data_D08[];
extern char lbl_1_data_D13[];
}

/* allocate() reserves 0x20 bytes before user storage; allocateBlock() and
 * release() cross that complete header. Only its first 0x10 bytes are named. */
struct HeapBlockHeader {
    HeapNode node;
    unsigned char unknown_10[0x10]; /* No code names this field. */
};

void *MemoryManager::allocateBlock(HeapNode *node, unsigned long bytes)
{
    if (node == 0) return 0;
    int remaining = node->size - bytes;
    if (remaining < 32) {
        node->size = -node->size;
        return (char *)node + 32;
    }
    HeapNode *split = (HeapNode *)((unsigned long)node + bytes);
    split->next = node->next;
    split->previous = node;
    node->next->previous = split;
    node->next = split;
    node->size = -bytes;
    split->size = remaining;
    split->chain = first;
    first = split;
    ++count;
    return (char *)node + 32;
}

MemoryManager *MemoryManager::initialize(unsigned long bytes)
{
    MemoryManager *heap = this;
    bytes -= sizeof(*heap);
    heap->initializeBlocks(heap + 1, bytes);
    return heap;
}

void MemoryManager::release(void *allocation)
{
    int originalSize;
    HeapNode *allocationHeader;
    HeapNode *block;

    if (allocation != 0) {
        allocationHeader = &((HeapBlockHeader *)allocation)[-1].node;
        block = allocationHeader;
        if (block->size > 0) {
            (lbl_1_data_D00 == 0) ? (void)0 :
                __msl_assertion_failed(lbl_1_data_D08, lbl_1_data_D13, 0x16);
        }
        originalSize = block->size;
        block->size = -originalSize;
        block->chain = first;
        first = block;
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

void MemoryManager::unlinkFreeBlock(HeapNode *node)
{
    --count;
    HeapNode **link = &first;
    while (*link != node) {
        HeapNode *current = *link;
        link = &current->chain;
    }
    *link = node->chain;
}

void MemoryManager::initializeBlocks(void *memory, unsigned long bytes)
{
    struct HeapRegion { HeapNode *first; void *original; } region;
    region.original = memory;
    region.first = (HeapNode *)(((unsigned long)region.original + 31UL) & ~31UL);
    bytes -= (unsigned long)region.first - (unsigned long)region.original;
    bytes &= ~31UL;
    first = region.first;
    count = 1;
    free = region.first;
    region.first->next = region.first;
    region.first->previous = region.first;
    region.first->size = bytes;
    region.first->chain = 0;
}

char lbl_1_data_D00[8] = "bad ptr";
char lbl_1_data_D08[11] = "!\"bad ptr\"";
char lbl_1_data_D13[13] = "free.cpp";
