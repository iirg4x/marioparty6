// Interfaces for Towering Treetop board objects and motion helpers.
#ifndef REL_W01DLL_WORLD01_H
#define REL_W01DLL_WORLD01_H

#include "dolphin.h"
#include "game/board/object_data.h"
#include "game/gamework.h"

extern BOOL mbSaveNewF;

typedef void (*W01_MBHOOK)(void);

void mbObjectSetup(s32 boardNo, W01_MBHOOK init, W01_MBHOOK close);
void mbLightFuncSet(W01_MBHOOK setHook, W01_MBHOOK resetHook);
void mbev_NextTimeSet(W01_MBHOOK hook);

int mbCapThrowColCreate(int dataNum);

void *mbMalloc(s32 size);

/* Curve derivatives use normalized progress from the start to the end value. */
float mbBezierCalcSlope(float start, float control, float end, float progress);
float mbHermiteCalcSlope(float start, float end, float startTangent, float endTangent,
    float progress);
/* Interpolates angles in degrees along the shortest arc. */
float mbAngleLerp(float startAngle, float endAngle, float progress);
float mbSinDeg(float angleDegrees);
float mbCosDeg(float angleDegrees);
float mbSinRad(float angleRadians);
void mbMtxRotAxisDeg(Mtx mtx, u8 axis, float angleDegrees);
void mbMtxRot(Mtx mtx, float xDegrees, float yDegrees, float zDegrees);

void mbScrollInit(int dataNum);
void mbMapCameraSet(const HuVecF *rot, const HuVecF *pos, float zoom);
void mbMapHookSet(void (*hook)(BOOL enterF));

void mbev_ShopExInit(int dataNum, void (*hook)(int modelId, int shopNo));
void mbev_ShopBackCreate(int dataNum, int motDataNum, int motNo, BOOL linkF);

void mbStarMoveHookSet(void (*hook)(void));
void mbStarMasuNextSet(int masuId);

void mbTelopTimeChangeCreate(void);
BOOL mbTelopTimeChangeCheck(void);

void mbWipeWait(void);
void mbWipeFadeOut(void);
void mbWipeFadeIn(void);

int mbGuideSpeakerNoGet(void);

void mbObjBiriQCreate(MBMODELID modelId);
BOOL mbObjBiriQKill(MBMODELID modelId);

#endif
