#include <dolphin/os.h>

#define TRUNC(n, a) (((u32)(n)) & ~((a)-1))

void *__OSArenaHi;
void *__OSArenaLo = (void *)-1;

void *OSGetArenaHi(void)
{
    return __OSArenaHi;
}

void *OSGetArenaLo(void)
{
    return __OSArenaLo;
}

void OSSetArenaHi(void *newHi)
{
    __OSArenaHi = newHi;
}

void OSSetArenaLo(void *newLo)
{
    __OSArenaLo = newLo;
}

void *OSAllocFromArenaLo(u32 size, u32 align)
{
    void *ptr;
    u8 *arenaLo;

    ptr = OSGetArenaLo();
    arenaLo = ptr = OSRoundUpPtr(ptr, align);
    arenaLo += size;
    arenaLo = OSRoundUpPtr(arenaLo, align);
    OSSetArenaLo(arenaLo);
    return ptr;
}
