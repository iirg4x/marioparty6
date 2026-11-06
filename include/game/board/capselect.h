#ifndef _BOARD_CAPSELECT_H
#define _BOARD_CAPSELECT_H

#include "types.h"

int mbCapSelect(void);
int mbCapDelete(int capsuleNo, BOOL repeatF);
void mbCapMasuDispSet(BOOL dispF);
void fn_8019A620(void);
void mbCapCapsuleGet(int playerNo, int capsuleNo);
BOOL mbCapSelectShrinkCheck(int playerNo);
void mbCapSelectResultGet(int playerNo, int *objId, int *result);
void mbCapSelectResultReset(int playerNo);
void mbCapSelectResultSet(int playerNo, int objId, int result);
void mbCapMasuExec(int playerNo, int masuId);

#endif
