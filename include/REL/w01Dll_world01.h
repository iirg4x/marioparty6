#ifndef REL_W01DLL_WORLD01_H
#define REL_W01DLL_WORLD01_H

#include "dolphin.h"
#include "game/board/object_data.h"
#include "game/gamework.h"

extern BOOL mbSaveNewF;

typedef void (*W01_MBHOOK)(void);

static inline int MBBoardNoGet(void)
{
    return GwSystem.boardNo;
}

void mbObjectSetup(s32 boardNo, W01_MBHOOK init, W01_MBHOOK close);
void mbLightFuncSet(W01_MBHOOK setHook, W01_MBHOOK resetHook);
void mbev_NextTimeSet(W01_MBHOOK hook);

int mbCapThrowColCreate(int dataNum);

void *mbMalloc(s32 size);

float mbBezierCalcSlope(float a, float b, float c, float t);
float mbHermiteCalcSlope(float a, float b, float c, float d, float t);
float mbAngleLerp(float a, float b, float t);
float mbSinDeg(float angle);
float mbCosDeg(float angle);
float mbSinRad(float angle);
void mbMtxRotAxisDeg(Mtx mtx, s8 axis, float angle);
void mbMtxRot(Mtx mtx, float x, float y, float z);

void mbScrollInit(int dataNum);
void mbMapCameraSet(const HuVecF *rot, const HuVecF *pos, float zoom);
void mbMapHookSet(void (*hook)(BOOL enterF));

void mbev_ShopExInit(int dataNum, void (*hook)(int modelId, int shopNo));
void mbev_ShopBackCreate(int dataNum, int motDataNum, int motNo, BOOL linkF);

void mbStarMoveHookSet(void (*hook)(void));
void mbStarMasuNextSet(s16 masuId);

void mbTelopTimeChangeCreate(void);
BOOL mbTelopTimeChangeCheck(void);

void mbWipeWait(void);
void mbWipeFadeOut(void);
void mbWipeFadeIn(void);

int mbGuideSpeakerNoGet(void);

void mbObjBiriQCreate(MBMODELID modelId);
BOOL mbObjBiriQKill(MBMODELID modelId);

#endif
