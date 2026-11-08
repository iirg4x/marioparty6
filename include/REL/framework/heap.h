/* Shared memory manager used by the minigame framework's C++ allocation operators. */
#ifndef FRAMEWORK_HEAP_H
#define FRAMEWORK_HEAP_H

/* The header in front of each block: physical neighbors and the chain of available blocks. */
struct HeapNode {
    HeapNode *next;
    HeapNode *previous;
    HeapNode *chain;
    long size; /* Negative while allocated; positive while available. */
};

/* Manages the storage used by C++ objects and arrays in the current minigame. */
struct MemoryManager {
    HeapNode *first;
    HeapNode *free;
    unsigned long count;

    /* Initializes the manager in a backing buffer of byteCount bytes. */
    MemoryManager *initialize(unsigned long byteCount);
    /* Aligns the storage after the manager and installs it as one available block. */
    void initializeBlocks(void *memory, unsigned long byteCount);
    /* Requests byteCount bytes for a new object or array. */
    void *allocate(unsigned long byteCount);
    /* Returns an object's or array's storage after destruction. */
    void release(void *allocation);
    /* Takes an available block, keeping a remainder when it holds another block header. */
    void *allocateBlock(HeapNode *node, unsigned long byteCount);
    /* Unlinks the first available block of at least byteCount bytes. */
    HeapNode *findFreeBlock(unsigned long byteCount);
    /* Unlinks a block merged into its neighbor from the chain of available blocks. */
    void unlinkFreeBlock(HeapNode *node);
};

/* Null until C++ new or new[] initializes the manager. */
extern MemoryManager *g_memman;
/* Creates the shared manager on the allocation operators' first use. */
void initializeMemoryManager();

#endif
