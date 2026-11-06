#ifndef _BOARD_STAR_H
#define _BOARD_STAR_H

#include "game/hu3d.h"

void mbStarInit(void);
void mbStarClose(void);
void mbStarFlagInit(void);
void mbStarNextNoSet(s8 nextNo);
int mbStarNoRandGet(void);
void mbStarNoSet(int no);
void mbStarReset(void);
int mbStarDispPlayerCreate(int playerNo, int num);
BOOL mbStarDispCheck(int playerNo);
int mbStarObjCreate(void);
void mbStarObjPosSet(int objNo, float x, float y, float z);
void mbStarObjPosGet(int objNo, HuVecF *pos);
void mbStarObjPosSetV(int objNo, const HuVecF *pos);
void mbStarObjRotSet(int objNo, float x, float y, float z);
void mbStarObjRotSetV(int objNo, const HuVecF *rot);
void mbStarObjScaleSet(int objNo, float x, float y, float z);
void mbStarObjScaleSetV(int objNo, const HuVecF *scale);
void mbStarObjDispSet(int objNo, BOOL dispF);
void mbStarObjDispFlagSet(int objNo, BOOL dispF);
void mbStarObjDispSetAll(BOOL dispF);
void mbStarObjKill(int objNo);
void mbStarChestCreate(int objNo, int playerNo);
void mbStarGetExec(int playerNo);
void mbZtarGetExec(int playerNo);
int mbStarAddExec(int playerNo, int starNum);
int mbStarAddProcExec(int playerNo, int starNum, BOOL dispF, BOOL fastF);
void mbStarAddAllProcExecV(int *addNum, BOOL *dispF, BOOL fastF);
void mbStarAddAllExec(int num0, int num1, int num2, int num3);
void mbStarMasuFuncSet(void (*func)(void));
void mbStarMoveHookSet(void (*hook)(void));
void mbStarMasuNextSet(int masuId);
void mbStarMapViewProcExec(void);
void mbStarMasuDispSet(int masuId, BOOL dispF);
void mbStarDispSetAll(BOOL dispF);

#endif
