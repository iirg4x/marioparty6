#ifndef _BOARD_OPENING_H
#define _BOARD_OPENING_H

#include "game/board/object_data.h"
#include "humath.h"
#include "game/memory.h"

void mbev_Opening(void);
void mbev_OpeningParty(void);
void mbev_OpeningSingle(void);

void mbOpeningViewSet(HuVecF *rot, HuVecF *pos, float zoom);
float mbOpeningZoomGet(void);
void mbOpeningRotGet(HuVecF *rot);
void mbOpeningPosGet(HuVecF *pos);
void mbOpeningInstHookSet(void (*hook)(void));
void mbOpeningStarInstHookSet(void (*hook)(void));
MBMODELID mbOpeningGuideObjIdGet(void);
void mbOpeningGuidePosRestore(void);
void mbOpeningCameraPosRestore(void);

// Used by ev_OpeningSingle and OpeningBezierCalc for temporary single-player opening data.
static inline void *OpeningAlloc(s32 size)
{
    return HuMemDirectMallocNum(HEAP_HEAP, size, 0x10000000);
}

#endif
