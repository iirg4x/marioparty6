// The memory manager behind the framework's C++ new and delete: setting up the manager and splitting a
// block for an allocation. free.cpp, linked right after this file, holds the rest. The file and function
// names are descriptive, not recovered: retail names only free.cpp.
#include "REL/framework/heap.h"

// Takes an available block for byteCount bytes, splitting off the rest when it can hold another block.
void *MemoryManager::allocateBlock(HeapNode *node, unsigned long byteCount)
{
    if (node == 0) return 0;
    int remaining = node->size - byteCount;
    if (remaining < 32) {
        node->size = -node->size;
        return node + 2;
    }
    HeapNode *split = (HeapNode *)((char *)node + byteCount);
    split->next = node->next;
    split->previous = node;
    node->next->previous = split;
    node->next = split;
    node->size = -byteCount;
    split->size = remaining;
    split->chain = first;
    first = split;
    ++count;
    return node + 2;
}

// Places the manager at the start of its byteCount-byte buffer and the blocks after it.
MemoryManager *MemoryManager::initialize(unsigned long byteCount)
{
    MemoryManager *heap = this;
    byteCount -= sizeof(*heap);
    heap->initializeBlocks(heap + 1, byteCount);
    return heap;
}
