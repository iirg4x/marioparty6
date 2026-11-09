#include "REL/framework/heap.h"

void *MemoryManager::allocateBlock(HeapNode *node,
                                      unsigned long bytes)
{
    if (node == 0) return 0;
    int remaining = node->size - bytes;
    if (remaining < 32) {
        node->size = -node->size;
        return (char *)node + 32;
    }
    HeapNode *split = (HeapNode *)((char *)node + bytes);
    split->next = node->next;
    split->previous = node;
    node->next->previous = split;
    node->next = split;
    node->size = -bytes;
    split->size = remaining;
    split->chain = this->first;
    this->first = split;
    ++this->count;
    return (char *)node + 32;
}

MemoryManager *MemoryManager::initialize(unsigned long bytes)
{
    MemoryManager *heap = this;
    bytes -= sizeof(*heap);
    heap->initializeBlocks(heap + 1, bytes);
    return heap;
}
