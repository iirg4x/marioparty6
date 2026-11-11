#ifndef _BOARD_CAPSULE_H
#define _BOARD_CAPSULE_H

#include "game/hu3d.h"

#define MBCapsuleEffRandF() \
    (mbRandMod(1 << 28) * (1.0f / (1 << 28)))

void mbCapInit(void);
void mbCapThrowHookSet(void (*hook)(BOOL startF));
int mbCapUse(int playerNo, int capsuleNo);
int mbCapBonusCoinNumGet(int playerNo, int capsuleNo);
void mbCapNumInc(int capsuleNo, int mode);
int mbCapSelectComGet(int playerNo, int *capsuleTbl, int capsuleNum);
int mbCapSelectDeleteComGet(int playerNo, int *capsuleTbl, int capsuleNum);
int mbCapMasuNextGet(int playerNo);
int mbCapDescWinCreate(int capsuleNo);
int mbCapObjCreate(int capsuleNo, BOOL flag);
void mbCapObjKill(int objId);
s16 mbCapValueTypeGet(s16 value);
s16 mbCapValuePlayerGet(s16 value);
BOOL mbCapEffUseCreate(int playerNo, int capsuleNo);
BOOL mbCapEffUseWanWanCreate(int playerNo, int capsuleValue, HuVecF *pos);
int mbCapEffUseModeGet(int playerNo);
int mbCapThrowColCreate(int dataNum);
int mbCapSellCostGet(s16 capsuleNo, s16 playerNo);
int mbCapUseMesGet(int capsuleNo);
void mbCapEffThrowCreate(HuVecF *startPos, HuVecF *endPos);
BOOL mbCapEffThrowCheck(HuVecF *pos, int *maxTime);
int mbCapRandomListGet(int *capsuleList, int maxNum);
s16 mbCapUseModeGet(s16 capsuleNo);
BOOL mbCapThrowMasuCheck(int masuId);
s16 mbCapMasuPlayerGet(s16 masuId);
s16 mbCapMasuDispTypeGet(s16 masuId);
s16 mbCapMasuTypeGet(s16 masuId);
void mbCapAutoThrow(HuVecF *startPos, HuVecF *endPos, HuVecF *masuPos,
    int playerNo, int masuId, int capsuleNo, BOOL maxTime, float startT);
int mbCapObjColorCreate(int capsuleNo, BOOL createF);
void mbCapObjColorPosSet(int id, float x, float y, float z);
void mbCapObjColorPosSetV(int id, HuVecF *pos);
void mbCapObjColorScaleSet(int id, float x, float y, float z);
void mbCapObjColorAlphaSet(int id, u8 alpha);
void mbCapObjColorLayerSet(int id, u8 layer);
void mbCapObjColorKill(int id);
void mbCapMasuPlayerTypeSet(s16 masuId, s16 capsuleNo, s16 playerNo);
int mbCapBuyCostGet(s16 capsuleNo, s16 playerNo);
int mbCapColorGet(int capsuleNo);

#endif
