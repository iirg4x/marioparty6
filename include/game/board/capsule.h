#ifndef _BOARD_CAPSULE_H
#define _BOARD_CAPSULE_H

#include "game/hu3d.h"

#define MBCapsuleEffRandF() \
    (mbRandMod(1 << 28) * (1.0f / (1 << 28)))

int mbCapRandomListGet(int *capsuleList, int maxNum);
s16 mbCapUseModeGet(s16 capsuleNo);
BOOL mbCapThrowMasuCheck(int masuId);
s16 mbCapMasuPlayerGet(s16 masuId);
s16 mbCapMasuDispTypeGet(s16 masuId);
void mbCapAutoThrow(HuVecF *startPos, HuVecF *endPos, HuVecF *masuPos,
    int playerNo, int masuId, int capsuleNo, BOOL maxTime, float startT);

#endif
