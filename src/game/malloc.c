/* Initializes and provides access to the game's memory heaps. */
#include "game/memory.h"
#include "game/init.h"
#include "dolphin/os.h"

/* Initial backing capacities in bytes for HEAP_HEAP through HEAP_DVD. */
#define HEAP_HEAP_CAPACITY_BYTES (2176 * 1024)
#define HEAP_SOUND_CAPACITY_BYTES (768 * 1024)
#define HEAP_MODEL_CAPACITY_BYTES (11 * 1024 * 1024)
#define HEAP_DVD_CAPACITY_BYTES (5 * 1024 * 1024)
#define HEAP_SPACE_CAPACITY_BYTES 0
static u32 HeapSizeTbl[HEAP_MAX] = { HEAP_HEAP_CAPACITY_BYTES, HEAP_SOUND_CAPACITY_BYTES,
                                     HEAP_MODEL_CAPACITY_BYTES, HEAP_DVD_CAPACITY_BYTES,
                                     HEAP_SPACE_CAPACITY_BYTES };
/* Base addresses returned by HuMemInit for each heap. */
static void *HeapTbl[HEAP_MAX];

/* At startup, initializes HEAP_HEAP through HEAP_DVD with fixed capacities, then reserves the
 * remaining OS heap space for HEAP_SPACE. */
void HuMemInitAll(void)
{
    s32 heapIndex;
    void *heapStorage;
    u32 freeSpaceBytes;
    for(heapIndex=0; heapIndex<HEAP_SPACE; heapIndex++) {
        heapStorage = OSAlloc(HeapSizeTbl[heapIndex]);
        if(heapStorage == NULL) {
            OSReport("HuMem> Failed OSAlloc Size:%d(left:%x)\n", HeapSizeTbl[heapIndex],
                     OSCheckHeap(currentHeapHandle));
            return;
        }
        HeapTbl[heapIndex] = HuMemInit(heapStorage, HeapSizeTbl[heapIndex]);
    }
    freeSpaceBytes = OSCheckHeap(currentHeapHandle);
    OSReport("HuMem> left memory space %dKB(%d)\n", freeSpaceBytes/1024, freeSpaceBytes);
    heapStorage = OSAlloc(freeSpaceBytes);
    if(heapStorage == NULL) {
        OSReport("HuMem> Failed OSAlloc left space\n");
        return;
    }
    HeapTbl[HEAP_SPACE] = HuMemInit(heapStorage, freeSpaceBytes);
    HeapSizeTbl[HEAP_SPACE] = freeSpaceBytes;
}

/* Creates the free-block list used by gameplay allocations; startup calls this for each heap. */
void *HuMemInit(void *heapStorage, s32 heapSize)
{
    return HuMemHeapInit(heapStorage, heapSize);
}

/* Before an overlay prolog runs, writes the model and general heap ranges back to memory. */
void HuMemDCFlushAll()
{
    HuMemDCFlush(HEAP_MODEL);
    HuMemDCFlush(HEAP_HEAP);
}

/* Hu3DModelAllKill in hsfman.c calls this after clearing model records; it writes the selected
 * heap's full range back to memory. */
void HuMemDCFlush(HEAPID heap)
{
    DCFlushRangeNoSync(HeapTbl[heap], HeapSizeTbl[heap]);
}

/* Gameplay and board loading request aligned blocks here; heap reports retain each caller
 * address. */
void *HuMemDirectMalloc(HEAPID heap, s32 allocationSize)
{
    register u32 retaddr;
    asm {
        mflr retaddr
    }
    allocationSize = OSRoundUp32B(allocationSize);
    return HuMemMemoryAlloc(HeapTbl[heap], allocationSize, retaddr);
}

/* Overlay and gameplay blocks can share a group number so HuMemDirectFreeNum can release them. */
void *HuMemDirectMallocNum(HEAPID heap, s32 allocationSize, u32 allocationNumber)
{
    register u32 retaddr;
    asm {
        mflr retaddr
    }
    allocationSize = OSRoundUp32B(allocationSize);
    return HuMemMemoryAllocNum(HeapTbl[heap], allocationSize, allocationNumber,
                               retaddr);
}

/* DVD file loading requests storage at the end of a heap to preserve space for regular
 * allocations. */
void *HuMemDirectTailMalloc(HEAPID heap, s32 allocationSize)
{
    register u32 retaddr;
    asm {
        mflr retaddr
    }
    allocationSize = OSRoundUp32B(allocationSize);
    return HuMemTailMemoryAlloc(HeapTbl[heap], allocationSize, retaddr);
}

/* Tail blocks can share a group number so a loading system can release related resources
 * together. */
void *HuMemDirectTailMallocNum(HEAPID heap, s32 allocationSize, u32 allocationNumber)
{
    register u32 retaddr;
    asm {
        mflr retaddr
    }
    allocationSize = OSRoundUp32B(allocationSize);
    return HuMemTailMemoryAllocNum(HeapTbl[heap], allocationSize, allocationNumber,
                                   retaddr);
}

/* Speech-engine callbacks keep a resized block in place when it fits. Growing a block copies its
 * contents to a new block with the same group number and frees the old block; failure leaves the
 * old block intact and returns NULL. */
void *HuMemDirectRealloc(HEAPID heap, void *allocation, s32 allocationSize)
{
    register u32 retaddr;
    asm {
        mflr retaddr
    }
    return HuMemMemoryRealloc(HeapTbl[heap], allocation, allocationSize, retaddr);
}

/* Gameplay systems release a block after its resource or working data is no longer needed. */
void HuMemDirectFree(void *allocation)
{
    register u32 retaddr;
    asm {
        mflr retaddr
    }
    HuMemMemoryFree(allocation, retaddr);
}

/* Game and board systems call this when a tagged group is no longer needed, such as overlay
 * data. */
void HuMemDirectFreeNum(HEAPID heap, u32 allocationNumber)
{
    register u32 retaddr;
    asm {
        mflr retaddr
    }
    HuMemMemoryFreeNum(HeapTbl[heap], allocationNumber, retaddr);
}

/* Returns the total allocated block span in bytes, including each 32-byte header. */
s32 HuMemUsedMallocSizeGet(HEAPID heap)
{
    return HuMemUsedMemorySizeGet(HeapTbl[heap]);
}

/* Returns the number of allocated blocks in a heap, used by the on-screen heap meter. */
s32 HuMemUsedMallocBlockGet(HEAPID heap)
{
    return HuMemUsedMemoryBlockGet(HeapTbl[heap]);
}

/* Returns the backing capacity in bytes for a heap. */
u32 HuMemHeapSizeGet(HEAPID heap)
{
    return HeapSizeTbl[heap];
}

/* Returns the allocator header at the start of a heap, for diagnostics and allocator queries. */
void *HuMemHeapPtrGet(HEAPID heap)
{
    return HeapTbl[heap];
}
