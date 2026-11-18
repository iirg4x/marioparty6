// Manages the framework's C++ allocation buffer.
// Allocation marks a selected block as occupied and splits off any usable remainder.
// Initialization places the manager before its block storage.
#include "REL/framework/heap.h"

// Takes an available block for byteCount bytes, splitting off any usable remainder.
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
