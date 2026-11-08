/* Heap block splitting and initialization of an in-place heap. */
#include "dolphin/types.h"
#include "REL/framework/heap.h"

/* Consumes a free block, retaining a remainder only when it spans a block header. */
void *MemoryManager::allocateBlock(HeapNode *node, unsigned long bytes)
{
    if (node == 0) return 0;
    int remaining = node->size - bytes;
    if (remaining < 32) {
        node->size = -node->size;
        return node + 2;
    }
    HeapNode *split = (HeapNode *)((char *)node + bytes);
    split->next = node->next;
    split->previous = node;
    node->next->previous = split;
    node->next = split;
    node->size = -bytes;
    split->size = remaining;
    split->chain = first;
    first = split;
    ++count;
    return node + 2;
}

/* Initializes the heap in the memory following its own control record. */
MemoryManager *MemoryManager::initialize(unsigned long bytes)
{
    MemoryManager *heap = this;
    bytes -= sizeof(*heap);
    heap->initializeBlocks(heap + 1, bytes);
    return heap;
}

extern "C" {
#include "REL/framework/assertion.h"
extern char lbl_1_data_4C8[];
extern char lbl_1_data_4D3[];
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
            (!"bad ptr") ? (void)0 :
                __msl_assertion_failed(lbl_1_data_4C8, lbl_1_data_4D3, 0x16);
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

void MemoryManager::unlinkFreeBlock(HeapNode *node)
{
    --this->count;
    HeapNode **link = &this->first;
    while (*link != node) {
        HeapNode *current = *link;
        link = &current->chain;
    }
    *link = node->chain;
}

char lbl_1_data_4C8[11] = "!\"bad ptr\"";
char lbl_1_data_4D3[] = "free.cpp";
