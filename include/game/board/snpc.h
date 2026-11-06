#ifndef _BOARD_SNPC_H
#define _BOARD_SNPC_H

#include "game/board/object_data.h"

void mbSNpcInit(void);
void mbSNpcDispSet(BOOL dispF);
int mbSNpcMasuGet(void);
void mbSNpcMotWinSet(void);
void mbSNpcMotIdleSet(void);
void mbSNpcPlayerWalkSet(int playerNo, int masuId);
void mbSNpcKill(void);
void mbSNpcMotReset(void);
void mbSNpcStarExec(int playerNo, int masuId);
void mbObjMetalCreate(MBMODELID modelId);
BOOL mbObjMetalKill(MBMODELID modelId);
void mbObjMetalTPLvlSet(MBMODELID modelId, float level);
void mbObjMetalColorSet(MBMODELID modelId, GXColor shadowColor, GXColor hiliteColor);
void mbObjBiriQCreate(MBMODELID modelId);
BOOL mbObjBiriQKill(MBMODELID modelId);
void mbObjBiriQColorSet(MBMODELID modelId, BOOL enableF, float level, GXColor color);

#endif
