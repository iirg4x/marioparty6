#ifndef _BOARD_SNPC_H
#define _BOARD_SNPC_H

#include "game/board/object_data.h"

typedef struct MBSNPCSAVEWORK MBSNPCSAVEWORK;

void mbSNpcInit(void);
void mbSNpcCreate(MBSNPCSAVEWORK *saveWork, u32 excludedMasuAttr);
void mbSNpcDispSet(BOOL dispF);
int mbSNpcMasuGet(void);
void mbSNpcMotWinSet(void);
void mbSNpcMotIdleSet(void);
void mbSNpcPlayerWalkSet(int playerNo, int masuId);
void mbSNpcKill(void);
void mbSNpcMotReset(void);
void mbSNpcStarExec(int playerNo, int masuId);
void mbObjFadeCreate(MBMODELID modelId, HuVecF *pos);
void mbObjFadeKill(MBMODELID modelId);
void mbObjFadeTexColorSet(MBMODELID modelId, u8 r, u8 g, u8 b, float alpha);
void mbObjFadeTexRotSet(MBMODELID modelId, HuVecF *pos, HuVecF *rot);
void mbObjMetalCreate(MBMODELID modelId);
BOOL mbObjMetalKill(MBMODELID modelId);
void mbObjMetalTPLvlSet(MBMODELID modelId, float level);
void mbObjMetalColorSet(MBMODELID modelId, GXColor shadowColor, GXColor hiliteColor);
void mbObjBiriQCreate(MBMODELID modelId);
BOOL mbObjBiriQKill(MBMODELID modelId);
void mbObjBiriQColorSet(MBMODELID modelId, BOOL enableF, float level, GXColor color);

#endif
