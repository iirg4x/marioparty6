/* Reserves model-heap storage for the minigame framework's C++ memory manager. */
static const char rcsid[] = "$Id: heap.cpp,v 1.4 2004/08/12 01:32:17 saf Exp $";

extern "C" {
#include "game/memory.h"
#include "REL/framework/assertion.h"
}

#include "REL/framework/heap_buffer.h"
#include "REL/framework/heap.h"

/* Newly reserved backing storage is filled with zero words by default. */
HeapSettings heapSettings = { 0 };
/* Shared by C++ new and delete; null until the first allocation initializes it. */
MemoryManager *g_memman;
/* Backing storage capacity in bytes, supplied by the current minigame. */
extern const s32 heapSize;

/* allocateHeapMemory uses this to fill the requested range before manager initialization. */
template <class Value>
inline void fillHeapBuffer(Value *destination, Value *destinationEnd, const Value &fillValue)
{
    for (; destination != destinationEnd; ++destination) {
        *destination = fillValue;
    }
}

/* initializeMemoryManager reserves and fills its model-heap backing storage here. */
void *allocateHeapMemory(s32 byteCount)
{
    u32 *bufferWords = (u32 *)HuMemDirectMallocNum(HEAP_MODEL, byteCount, HU_MEMNUM_OVL);
    /* Exhaustion reports the assertion and aborts before the fill can run. */
    bufferWords != 0 ? (void)0 : __msl_assertion_failed("buf != NULL", "heap.cpp", 41);
    /* Trailing partial words and the allocator's alignment padding are left untouched. */
    fillHeapBuffer(bufferWords, bufferWords + byteCount / 4, heapSettings.fillWord);
    return bufferWords;
}

/* C++ new and new[] call this on first use to create their shared memory manager. */
void initializeMemoryManager()
{
    if (g_memman == 0) {
        s32 heapBytes = heapSize;
        MemoryManager *backingBuffer = (MemoryManager *)allocateHeapMemory(heapBytes);
        g_memman = backingBuffer->initialize(heapBytes);
    }
}
