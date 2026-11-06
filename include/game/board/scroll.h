#ifndef _BOARD_SCROLL_H
#define _BOARD_SCROLL_H

#include "dolphin.h"

void mbev_Scroll(int playerNo, BOOL mapF);
void mbev_ScrollCapsule(int playerNo);
void mbScrollClose(void);
void mbScrollStarFindFuncSet(s16 (*findFunc)(int playerNo));
void mbScrollHookSet(void (*hook)(BOOL));
void mbev_StarScroll(HuVecF *startPos, HuVecF *endPos, s16 time);
void mbMapSprAdd(int type, s16 id);

#endif
