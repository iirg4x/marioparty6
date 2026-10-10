#ifndef FRAMEWORK_HEAP_H
#define FRAMEWORK_HEAP_H

// Interface used by the framework's C++ allocation operators.
struct MemoryManager {
    MemoryManager *initialize(unsigned long size);
    void *allocate(unsigned long size);
    void release(void *allocation);
};

extern MemoryManager *g_memman;
void initializeMemoryManager();

#endif
