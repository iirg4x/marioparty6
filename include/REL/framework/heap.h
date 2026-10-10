/* Shared memory manager used by the minigame framework's C++ allocation operators. */
#ifndef FRAMEWORK_HEAP_H
#define FRAMEWORK_HEAP_H

/* Manages the storage used by C++ objects and arrays in the current minigame. */
struct MemoryManager {
    /* Initializes the manager in a backing buffer of byteCount bytes. */
    MemoryManager *initialize(unsigned long byteCount);
    /* Requests byteCount bytes for a new object or array. */
    void *allocate(unsigned long byteCount);
    /* Returns an object's or array's storage after destruction. */
    void release(void *allocation);
};

/* Null until C++ new or new[] initializes the manager. */
extern MemoryManager *g_memman;
/* Creates the shared manager on the allocation operators' first use. */
void initializeMemoryManager();

#endif
