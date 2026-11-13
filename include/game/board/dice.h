#ifndef _BOARD_DICE_H
#define _BOARD_DICE_H

#include "game/hu3d.h"
#include "game/object.h"

void mbDiceInit(void);
void mbDiceClose(void);
int mbDiceExec(int playerNo, int diceType, s8 *valueTbl, int tutorialVal,
    BOOL padWinF, BOOL waitF, HuVecF *pos, int color);
void mbDiceKill(int playerNo);
int mbDiceMaxGet(int diceType);
int mbDiceValueMaxGet(int diceType);
void mbDiceNumKill(int playerNo);
int mbDiceProcExec(int playerNo, int diceType, s8 *valueTbl,
    int *tutorialVal, BOOL padWinF, BOOL waitF, HuVecF *pos, int color);
void mbDiceMotHookSet(int playerNo, void (*hook)(int));
void mbDiceHitHookSet(int playerNo, void (*hook)(int));
void mbDiceFadeSet(int playerNo);
void mbDicePadBtnHookSet(int playerNo, u16 (*hook)(int));
int mbDiceResultGet(int playerNo);
BOOL mbDiceKillCheck(int playerNo);
void mbDiceObjHit(int playerNo);
OMOBJ *mbDiceSNpcNumCreate(int playerNo, HuVecF *pos);
OMOBJ *mbDiceSNpcNumObjCreate(HuVecF *pos, HuVecF *offset, int value,
    BOOL flagF, int color);
void mbDiceSNpcNumOfsSet(OMOBJ *obj, HuVecF *offset);
void mbDiceSNpcNumOfsGet(OMOBJ *obj, HuVecF *offset);
void mbDiceSNpcNumKill(OMOBJ *obj);
void mbDiceSNpcNumDispSet(OMOBJ *obj, BOOL dispF);
void mbDiceSNpcNumSet(OMOBJ *obj, int value);
void mbDiceSNpcNumPosSet(OMOBJ *obj, HuVecF *pos);
void mbDiceNumShrinkSet(int playerNo);

#endif
