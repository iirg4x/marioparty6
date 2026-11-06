#ifndef _GAME_MEMORY_H
#define _GAME_MEMORY_H

#include "dolphin/types.h"

enum {
    HEAP_HEAP,
};
typedef int HEAPID;

void fn_8000B2A8(void);
void *fn_8000B3E0(void *ptr, s32 size);
void HuMemDCFlushAll();
void HuMemDCFlush(HEAPID heap);
void *HuMemDirectMalloc(HEAPID heap, s32 size);
void *HuMemDirectMallocNum(HEAPID heap, s32 size, u32 num);
void *fn_802137C0(s32 size);
void *fn_80213818(s32 size, u32 num);
void *fn_80213874(s32 size);
void *fn_802138C8(s32 size);
void *fn_8000B630(HEAPID heap, void *allocation, s32 allocationSize);
void *fn_8000B560(HEAPID heap, s32 size);
void *fn_8000B5C4(HEAPID heap, s32 size, u32 num);

void HuMemDirectFree(void *ptr);
void HuMemDirectFreeNum(HEAPID heap, u32 num);
s32 HuMemUsedMallocSizeGet(HEAPID heap);
s32 HuMemUsedMallocBlockGet(HEAPID heap);
u32 fn_8000B790(HEAPID heap);
void *fn_8000B7A8(HEAPID heap);

void *HuMemHeapInit(void *heap, s32 size);
void *fn_8000B84C(void *heap, s32 size, u32 retAddr);
void *fn_8000B80C(void *heap, s32 size, u32 num, u32 retAddr);
void *fn_8000B9FC(void *heap, s32 size, u32 retAddr);
void *fn_8000B9BC(void *heap, s32 size, u32 num, u32 retAddr);
BOOL fn_8000C164(void *ptr, u32 file);
u32 fn_8000C1B4(void *ptr);
BOOL fn_8000C200(void *ptr, u32 num);

void HuMemMemoryFree(void *heap, u32 retAddr);
void HuMemMemoryFreeNum(void *heap, u32 num, u32 retAddr);
void *fn_8000BB44(void *heap, void *ptr, s32 size, u32 retAddr);
s32 fn_8000BF58(void *heap);
s32 HuMemUsedMemorySizeGet(void *heap);
s32 HuMemUsedMemoryBlockGet(void *heap);
s32 HuMemMemorySizeGet(void *heap);
s32 HuMemMemoryAllocSizeGet(s32 size);
void fn_8000BFB4(void *heap, s16 status);
void *mbMallocFlushModelNum(s32 size, u32 num);

static inline void *ModelHeapMallocNum(s32 size, u32 allocationNo)
{
    return HuMemDirectMallocNum(2, size, allocationNo);
}

#endif
