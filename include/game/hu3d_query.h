#ifndef GAME_HU3D_QUERY_H
#define GAME_HU3D_QUERY_H

#include "dolphin/mtx.h"
#include "dolphin/gx/GXStruct.h"

#ifdef __cplusplus
extern "C" {
#endif

void Hu3DModelObjPosGet(s16 modelId, char *objName, Vec *pos);
s16 fn_800297D4(s16 gLightId, Vec *pos, Vec *dir, GXColor *color);

#ifdef __cplusplus
}
#endif

#endif
