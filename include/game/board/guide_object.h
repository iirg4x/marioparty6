#ifndef _BOARD_GUIDE_OBJECT_H
#define _BOARD_GUIDE_OBJECT_H

#include "game/board/guide.h"

// Board object model system (linked from DOL)
MBMODELID mbObjCreate(int dataNum, int *motData, BOOL link);
int mbObjMotionNoCreate(int modelId, int dataNum, int motNo);
int mbObjMotionIDGet(int modelId, int motNo);
int mbObjMotionShiftIDGet(MBMODELID modelId);
BOOL mbObjMotionEndCheck(int modelId);
void mbObjMotionSet(int modelId, int motNo, u32 attr);
void mbObjMotionShiftSet(int modelId, int motNo, float start, float end, BOOL loopF);
void mbObjPosSetV(int modelId, HuVecF *pos);
void mbObjPosGet(int modelId, HuVecF *pos);
void mbObjRotSet(int modelId, float rotX, float rotY, float rotZ);
void mbObjRotSetV(int modelId, HuVecF *rot);
void mbObjScaleSet(int modelId, float x, float y, float z);
void mbObjScaleGet(int modelId, HuVecF *scale);
void mbObjMtxSet(int modelId, Mtx *mtx);
void mbObjMtxGet(int modelId, Mtx *mtx);
void mbObjCameraSet(int modelId, u16 cameraBit);
void mbObjLayerSet(int modelId, int layer);
void mbObjAttrSet(int modelId, u32 attr);
void mbObjDispSet(int modelId, BOOL dispF);
void mbObjKill(int modelId);
void mbObjHookSet(int modelId, char *objName, int hookMdlId);
MBOBJMODEL *mbObjGet(MBMODELID modelId);

void mbObjFadeCreate(int modelId, HuVecF *pos);
void mbObjFadeKill(int modelId);
void mbObjFadeTexColorSet(int modelId, int r, int g, int b, float a);
void mbObjFadeTexRotSet(int modelId, HuVecF *pos, HuVecF *rot);

float mbSinDeg(float deg);
float mbCosDeg(float deg);
BOOL mbPauseProcCheck(void);

#endif
