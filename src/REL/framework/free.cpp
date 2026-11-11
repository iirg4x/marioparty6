// The memory manager's chain of available blocks: freeing, finding and setting up blocks. Retail names
// this file in its one assert, on line 22; the text of the lines before it is not recovered and the
// function names are descriptive. NDEBUG is undefined because the build defines it for every object
// while retail kept this assert.
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "REL/framework/heap.h"

// Gives an allocation's block back to the chain and merges it with an available neighbour on either
// side.
void MemoryManager::release(void *data)
{
    long originalSize;
    HeapNode *allocationHeader;
    HeapNode *block;

    if (data != 0) {
        allocationHeader = (HeapNode *)data - 2;
        block = allocationHeader;
        // A positive size is an available block: not allocated here, or freed already.
        if (block->size > 0) {
            assert(!"bad ptr");
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

// Removes a block from the chain of available blocks.
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

// Aligns the buffer to 32 bytes and makes all of it one available block.
void MemoryManager::initializeBlocks(void *memory, unsigned long byteCount)
{
    struct HeapRegion { HeapNode *first; void *original; } region;
    region.original = memory;
    region.first = (HeapNode *)(((unsigned long)region.original + 31UL) & ~31UL);
    byteCount -= (unsigned long)region.first - (unsigned long)region.original;
    byteCount &= ~31UL;
    first = region.first;
    count = 1;
    free = region.first;
    region.first->next = region.first;
    region.first->previous = region.first;
    region.first->size = byteCount;
    region.first->chain = 0;
}

// Rounds the request up to whole 32-byte units plus the block header.
void *MemoryManager::allocate(unsigned long byteCount)
{
    unsigned long rounded = ((byteCount + 31) & ~31UL) + 32;
    byteCount = rounded;
    return allocateBlock(findFreeBlock(byteCount), byteCount);
}

// Unlinks the first available block that is large enough.
HeapNode *MemoryManager::findFreeBlock(unsigned long byteCount)
{
    HeapNode **link = &first;
    while (*link) {
        HeapNode *node = *link;
        int remaining = node->size - byteCount;
        if (remaining >= 0) {
            *link = node->chain;
            return node;
        }
        link = &node->chain;
    }
    return 0;
}
