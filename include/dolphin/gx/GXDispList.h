#ifndef _DOLPHIN_GXDISPLIST
#define _DOLPHIN_GXDISPLIST

#include <dolphin/gx/GXVert.h>
#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void GXBeginDisplayList(void* list, u32 size);
u32 GXEndDisplayList(void);
void GXCallDisplayList(const void* list, u32 nbytes);

static inline void GXFastCallDisplayList(void* list, u32 size)
{
    GXWGFifo.u8 = 0x40;
    GXWGFifo.u32 = (u32)list;
    GXWGFifo.u32 = size;
}

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_GXDISPLIST
