#ifndef _DOLPHIN_OSARENA
#define _DOLPHIN_OSARENA

#include <dolphin/types.h>

void* OSGetArenaHi(void);
void* OSGetArenaLo(void);
#ifdef __cplusplus
extern "C" {
#endif
void OSSetArenaHi(void* newHi);
void OSSetArenaLo(void* newLo);
#ifdef __cplusplus
}
#endif
void* OSAllocFromArenaLo(u32 size, u32 align);
void* OSAllocFromArenaLo(u32 size, u32 align);

#endif // _DOLPHIN_OSARENA
