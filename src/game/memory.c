/* Manages the game's linked heaps used for resource, model, sound and process allocations. */
#include "game/memory.h"
#include "dolphin/os.h"

#define DATA_GET_BLOCK(payload) ((MEMORY_BLOCK *)(((char *)(payload))-32))
#define BLOCK_GET_DATA(block) (((char *)(block))+32)

#define MEMORY_ALIGNMENT_MASK 0xFFFFFFE0
#define MEMORY_UNUSED_CALLER_BIT_PATTERN 0xCDCDCDCD
#define MEMORY_CHAIN_BROKEN_MASK 0x80000000
#define MEMORY_FREE_MARKER_BIT_PATTERN 0xCD
#define MEMORY_ALLOCATED_MARKER_BIT_PATTERN 0xA5
#define MEMORY_UNNUMBERED_ALLOCATION_TAG (-256)
#define MEMORY_DUMP_ALL_STATES 10
#define MEM_ALLOC_SIZE(payloadBytes) (((payloadBytes)+63) & MEMORY_ALIGNMENT_MASK)

#define BLOCK_CHECK_BROKEN(block) (((u32)((block)->next) & MEMORY_CHAIN_BROKEN_MASK) == 0)

typedef struct MemoryBlock_s MEMORY_BLOCK;

struct MemoryBlock_s {
    s32 spanBytes;                 /* Block size in bytes, including this 32-byte header. */
    u8 validityMarker;             /* Allocated marker while live; free marker after release. */
    u8 isAllocated;                /* 1 while game data owns this block; 0 when it is reusable. */
    MEMORY_BLOCK *previous;        /* Previous block by address in this heap's circular list. */
    MEMORY_BLOCK *next;            /* Next block by address in this heap's circular list. */
    u32 allocationTag;             /* Group used by numbered frees; -256 marks an unnumbered
                                    * block. */
    u32 callerAddress;             /* Caller return address shown by heap diagnostics. */
    u32 dataFileId;                /* Resource data number associated with this payload, if any. */
};

static void *HuMemMemoryAlloc2(void *heapHead, s32 payloadBytes, u32 allocationTag,
                               u32 callerAddress);
static void *HuMemTailMemoryAlloc2(void *heapHead, s32 payloadBytes, u32 allocationTag,
                                   u32 callerAddress);

/* HuMemInit calls this during heap setup to describe the whole region as one available block. */
void *HuMemHeapInit(void *heapHead, s32 heapSize)
{
    MEMORY_BLOCK *block = heapHead;
    block->spanBytes = heapSize;
    block->validityMarker = MEMORY_FREE_MARKER_BIT_PATTERN;
    block->isAllocated = 0;
    block->previous = block;
    block->next = block;
    block->allocationTag = MEMORY_UNNUMBERED_ALLOCATION_TAG;
    block->callerAddress = MEMORY_UNUSED_CALLER_BIT_PATTERN;
    return block;
}

/* HuMemDirectMallocNum calls this to allocate a block carrying its caller's group tag. */
void *HuMemMemoryAllocNum(void *heapHead, s32 payloadBytes, u32 allocationTag, u32 callerAddress)
{
    return HuMemMemoryAlloc2(heapHead, payloadBytes, allocationTag, callerAddress);
}

/* HuMemDirectMalloc calls this to allocate a block without a caller-supplied group tag. */
void *HuMemMemoryAlloc(void *heapHead, s32 payloadBytes, u32 callerAddress)
{
    return HuMemMemoryAlloc2(heapHead, payloadBytes, MEMORY_UNNUMBERED_ALLOCATION_TAG,
                             callerAddress);
}

/* HuMemDirectMalloc variants use this first-fit scan; a free remainder is split off only when it
 * exceeds the 32-byte header size. */
static void *HuMemMemoryAlloc2(void *heapHead, s32 payloadBytes, u32 allocationTag,
                               u32 callerAddress)
{
    s32 alignedSpanBytes = MEM_ALLOC_SIZE(payloadBytes);
    MEMORY_BLOCK *block = heapHead;
    MEMORY_BLOCK *previousBlock;
    do {
        if(!block->isAllocated && block->spanBytes >= alignedSpanBytes) {
            if(block->spanBytes-alignedSpanBytes > 32u) {
                MEMORY_BLOCK *splitBlock = (MEMORY_BLOCK *)(((u32)block)+alignedSpanBytes);
                splitBlock->spanBytes = block->spanBytes-alignedSpanBytes;
                splitBlock->validityMarker = MEMORY_FREE_MARKER_BIT_PATTERN;
                splitBlock->isAllocated = 0;
                splitBlock->callerAddress = callerAddress;
                block->next->previous = splitBlock;
                splitBlock->next = block->next;
                block->next = splitBlock;
                splitBlock->previous = block;
                block->spanBytes = alignedSpanBytes;
            }
            block->isAllocated = 1;
            block->validityMarker = MEMORY_ALLOCATED_MARKER_BIT_PATTERN;
            block->allocationTag = allocationTag;
            block->callerAddress = callerAddress;
            block->dataFileId = 0;
            return BLOCK_GET_DATA(block);
        }
        if(BLOCK_CHECK_BROKEN(block)) {
            OSReport("Error: memory chain broken!\n");
        }
        previousBlock = block;
        block = block->next;
        
    } while(block != heapHead);
    OSReport("HuMem>memory alloc error %08x(%08X): Call %08x\n", payloadBytes, allocationTag,
             callerAddress);
    HuMemHeapDump(heapHead, -1);
    return NULL;
}

/* HuMemDirectTailMallocNum calls this to allocate from the tail with its caller's group tag. */
void *HuMemTailMemoryAllocNum(void *heapHead, s32 payloadBytes, u32 allocationTag,
                              u32 callerAddress)
{
    return HuMemTailMemoryAlloc2(heapHead, payloadBytes, allocationTag, callerAddress);
}

/* HuMemDirectTailMalloc calls this to allocate from the tail without a caller group tag. */
void *HuMemTailMemoryAlloc(void *heapHead, s32 payloadBytes, u32 callerAddress)
{
    return HuMemTailMemoryAlloc2(heapHead, payloadBytes, MEMORY_UNNUMBERED_ALLOCATION_TAG,
                                 callerAddress);
}

/* HuMemDirectTailMalloc variants scan from the high end and carve payloads from a block's end;
 * a free remainder is kept only when it exceeds the 32-byte header size. */
static void *HuMemTailMemoryAlloc2(void *heapHead, s32 payloadBytes, u32 allocationTag,
                                   u32 callerAddress)
{
    s32 alignedSpanBytes = MEM_ALLOC_SIZE(payloadBytes);
    MEMORY_BLOCK *block = heapHead;
    while(block->next != heapHead) {
        block = block->next;
    }
    do {
        if(!block->isAllocated && block->spanBytes >= alignedSpanBytes) {
            if(block->spanBytes-alignedSpanBytes > 32u) {
                MEMORY_BLOCK *tailBlock = block;
                block = (MEMORY_BLOCK *) (((char *) tailBlock) +
                                          (tailBlock->spanBytes - alignedSpanBytes));
                block->spanBytes = alignedSpanBytes;
                block->previous = tailBlock;
                block->next = tailBlock->next;
                tailBlock->spanBytes = tailBlock->spanBytes-alignedSpanBytes;
                tailBlock->next = tailBlock->next->previous = block;
                tailBlock->callerAddress = callerAddress;
            }
            block->validityMarker = MEMORY_ALLOCATED_MARKER_BIT_PATTERN;
            block->isAllocated = 1;
            block->allocationTag = allocationTag;
            block->callerAddress = callerAddress;
            block->dataFileId = 0;
            return BLOCK_GET_DATA(block);
        }
        block = block->previous;
    } while(block != heapHead);
    printf("memory allocation(tail) error.\n");
    return NULL;
}

/* HuMemDirectRealloc resizes in place or moves a resource; a shrink creates a free tail only when
 * the remainder exceeds 32 bytes, and does not merge that tail here. */
void *HuMemMemoryRealloc(void *heapHead, void *payload, s32 payloadBytes, u32 callerAddress)
{
    s32 alignedSpanBytes = MEM_ALLOC_SIZE(payloadBytes);
    MEMORY_BLOCK *block = DATA_GET_BLOCK(payload);
    if(block->spanBytes >= alignedSpanBytes) {
        if(block->spanBytes-alignedSpanBytes > 32u) {
            MEMORY_BLOCK *splitBlock = (MEMORY_BLOCK *)(((u32)block)+alignedSpanBytes);
            splitBlock->spanBytes = block->spanBytes-alignedSpanBytes;
            splitBlock->validityMarker = MEMORY_FREE_MARKER_BIT_PATTERN;
            splitBlock->isAllocated = 0;
            splitBlock->callerAddress = callerAddress;
            block->next->previous = splitBlock;
            splitBlock->next = block->next;
            block->next = splitBlock;
            splitBlock->previous = block;
            block->spanBytes = alignedSpanBytes;
        }
        block->callerAddress = callerAddress;
        return BLOCK_GET_DATA(block);
    } else {
        void *resizedPayload =
            HuMemMemoryAllocNum(heapHead, payloadBytes, block->allocationTag, callerAddress);
        if(resizedPayload) {
            /* Preserve the old block's full payload capacity, not only the new requested size. */
            memcpy(resizedPayload, payload, block->spanBytes-32);
            HuMemMemoryFree(payload, callerAddress);
        }
        return resizedPayload;
    }
}

/* HuMemDirectFreeNum calls this when a game system releases every block in one tag group. */
void HuMemMemoryFreeNum(void *heapHead, u32 allocationTag, u32 callerAddress)
{
    MEMORY_BLOCK *block = heapHead;
    do {
        MEMORY_BLOCK *nextBlock = block->next;
        if(block->isAllocated && block->allocationTag == allocationTag) {
            HuMemMemoryFree(BLOCK_GET_DATA(block), callerAddress);
        }
        block = nextBlock;
    } while(block != heapHead);
    
}

/* HuMemDirectFree and HuMemDirectRealloc's move path release a payload and coalesce with adjacent
 * free blocks; address checks prevent joining across the circular-list wrap. */
void HuMemMemoryFree(void *payload, u32 callerAddress)
{
    MEMORY_BLOCK *block;
    if(!payload) {
        return;
    }
    block = DATA_GET_BLOCK(payload);
    if(block->validityMarker != MEMORY_ALLOCATED_MARKER_BIT_PATTERN) {
        OSReport("HuMem>memory free error. %08x( call %08x)\n", payload, callerAddress);
        return;
    }
    if(block->previous < block && !block->previous->isAllocated) {
        block->isAllocated  = 0;
        block->validityMarker = MEMORY_FREE_MARKER_BIT_PATTERN;
        block->next->previous = block->previous;
        block->previous->next = block->next;
        block->previous->spanBytes += block->spanBytes;
        block = block->previous;
    }
    if(block->next > block && !block->next->isAllocated) {
        block->next->next->previous = block;
        block->spanBytes += block->next->spanBytes;
        block->next = block->next->next;
    }
    block->isAllocated = 0;
    block->validityMarker = MEMORY_FREE_MARKER_BIT_PATTERN;
    block->callerAddress = callerAddress;
}

/* HuMemUsedMallocSizeGet uses this for the heap meter's allocated byte total, including headers. */
s32 HuMemUsedMemorySizeGet(void *heapHead)
{
    MEMORY_BLOCK *block = heapHead;
    s32 usedSpanBytes = 0;
    do {
        if(block->isAllocated == 1) {
            usedSpanBytes += block->spanBytes;
        }
        block = block->next;
    } while(block != heapHead);
    return usedSpanBytes;
}

/* HuMemUsedMallocBlockGet uses this for the heap meter's number of live allocations. */
s32 HuMemUsedMemoryBlockGet(void *heapHead)
{
    MEMORY_BLOCK *block = heapHead;
    s32 allocatedBlockCount = 0;
    do {
        if(block->isAllocated == 1) {
            allocatedBlockCount++;
        }
        block = block->next;
    } while(block != heapHead);
    return allocatedBlockCount;
}

/* Audio loading checks this before choosing a heap for a sample buffer. */
s32 HuMemMaxMemorySizeGet(void *heapHead)
{
    MEMORY_BLOCK *block = heapHead;
    s32 largestFreeSpanBytes = 0;
    do {
        if(block->isAllocated == 0 && largestFreeSpanBytes < block->spanBytes) {
            largestFreeSpanBytes = block->spanBytes;
        }
        block = block->next;
    } while(block != heapHead);
    return largestFreeSpanBytes;
}
/* Allocation callers use this to round a payload plus its 32-byte header to a 32-byte span. */
s32 HuMemMemoryAllocSizeGet(s32 payloadBytes)
{
    return MEM_ALLOC_SIZE(payloadBytes);
}

/* Heap reports print blocks: 0 selects free, positive selects used, and negative selects both;
 * failed requests and board diagnostics call this to inspect the heap. */
void HuMemHeapDump(void *heapHead, s16 status)
{
    MEMORY_BLOCK *block = heapHead;
    s32 allocatedSpanBytes = 0;
    s32 freeSpanBytes = 0;
    s32 allocatedBlockCount = 0;
    s32 freeBlockCount = 0;
    u8 allocationStateFilter;

    if(status < 0) {
        allocationStateFilter = MEMORY_DUMP_ALL_STATES;
    } else if(status == 0) {
        allocationStateFilter = 0;
    } else {
        allocationStateFilter = 1;
    }
    OSReport("======== HuMem heap dump %08x ========\n", heapHead);
    OSReport("MCB-----+Size----+MG+FL+Prev----+Next----+UNum----+Body----+Call----+File----\n");
    do {
        if(allocationStateFilter == MEMORY_DUMP_ALL_STATES ||
           block->isAllocated == allocationStateFilter) {
            OSReport("%08x %08x %02x %02x %08x %08x %08x %08x %08x %08x\n", block, block->spanBytes,
                     block->validityMarker, block->isAllocated, block->previous, block->next,
                     block->allocationTag, BLOCK_GET_DATA(block), block->callerAddress,
                     block->dataFileId);
        }
        if(block->isAllocated == 1) {
            allocatedSpanBytes += block->spanBytes;
            allocatedBlockCount++;
        } else {
            freeSpanBytes += block->spanBytes;
            freeBlockCount++;
        }
        
        block = block->next;
    } while(block != heapHead);
    OSReport("MCB:%d(%d/%d) MEM:%08x(%08x/%08x)\n", allocatedBlockCount + freeBlockCount,
             allocatedBlockCount, freeBlockCount, allocatedSpanBytes + freeSpanBytes,
             allocatedSpanBytes, freeSpanBytes);
    OSReport("======== HuMem heap dump %08x end =====\n", heapHead);
}

/* The ARAM loader gets a valid block's allocated payload capacity, including alignment padding;
 * this is its rounded span minus the 32-byte header. Null, free, or invalid blocks return zero. */
s32 HuMemMemorySizeGet(void *payload)
{
    MEMORY_BLOCK *block;
    if(!payload) {
        return 0;
    }
    block = DATA_GET_BLOCK(payload);
    if(block->isAllocated == 1 && block->validityMarker == MEMORY_ALLOCATED_MARKER_BIT_PATTERN) {
        return block->spanBytes-32;
    } else {
        return 0;
    }
}

/* Data and ARAM loaders store the supplied data number, including any directory bits, in the
 * block header. */
BOOL HuMemMemoryFileSet(void *payload, u32 dataFileId)
{
    MEMORY_BLOCK *block;
    if(!payload) {
        return FALSE;
    }
    block = DATA_GET_BLOCK(payload);
    if(block->isAllocated == 1 && block->validityMarker == MEMORY_ALLOCATED_MARKER_BIT_PATTERN) {
        block->dataFileId = dataFileId;
        return TRUE;
    } else {
        return FALSE;
    }
}

/* Model setup and diagnostics read this resource ID; an invalid pointer returns zero. */
u32 HuMemMemoryFileGet(void *payload)
{
    MEMORY_BLOCK *block;
    if(!payload) {
        return 0;
    }
    block = DATA_GET_BLOCK(payload);
    if(block->isAllocated == 1 && block->validityMarker == MEMORY_ALLOCATED_MARKER_BIT_PATTERN) {
        return block->dataFileId;
    } else {
        return 0;
    }
}

/* The external numbered-memory API uses this to change which HuMemDirectFreeNum releases a
 * block. */
BOOL HuMemMemoryNumSet(void *payload, u32 allocationTag)
{
    MEMORY_BLOCK *block;
    if(!payload) {
        return FALSE;
    }
    block = DATA_GET_BLOCK(payload);
    if(block->isAllocated == 1 && block->validityMarker == MEMORY_ALLOCATED_MARKER_BIT_PATTERN) {
        block->allocationTag = allocationTag;
        return TRUE;
    } else {
        return FALSE;
    }
}
