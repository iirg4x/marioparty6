/* Fill settings and backing-buffer allocation for the minigame framework's heap. */
#ifndef FRAMEWORK_HEAP_BUFFER_H
#define FRAMEWORK_HEAP_BUFFER_H

#include "dolphin/types.h"

/* Initialization settings for the model-heap buffers used by the framework. */
struct HeapSettings {
    u32 fillWord; /* Pattern written to each complete word of a new buffer. */
};

/* The current fill pattern; its initial value is zero. */
extern HeapSettings heapSettings;
/* Reserves byteCount bytes and fills only complete words in the requested range. */
void *allocateHeapMemory(s32 byteCount);

#endif
