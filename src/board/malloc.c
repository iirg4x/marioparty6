#include "game/memory.h"
#include "dolphin/os/OSCache.h"
#include "string.h"

void *fn_802137C0(s32 size)
{
    void *ptr;

    ptr = HuMemDirectMallocNum(HEAP_HEAP, size, 0x10000000);
    memset(ptr, 0, size);
    return ptr;
}

void *fn_80213818(s32 size, u32 num)
{
    void *ptr;

    ptr = HuMemDirectMallocNum(2, size, num);
    memset(ptr, 0, size);
    return ptr;
}

void *fn_80213874(s32 size)
{
    void *ptr;

    ptr = HuMemDirectMallocNum(HEAP_HEAP, size, 0x10000000);
    DCFlushRange(ptr, size);
    return ptr;
}

void *fn_802138C8(s32 size)
{
    void *ptr;

    ptr = HuMemDirectMallocNum(2, size, 0x10000000);
    DCFlushRange(ptr, size);
    return ptr;
}

void *mbMallocFlushModelNum(s32 size, u32 num)
{
    void *ptr;

    ptr = HuMemDirectMallocNum(2, size, num);
    DCFlushRange(ptr, size);
    return ptr;
}
