#ifndef _GAME_ARMEM_H
#define _GAME_ARMEM_H

#include "dolphin.h"
#include "game/memory.h"
#include "game/data.h"

typedef u32 AMEM_PTR;

void HuARInit(void);
AMEM_PTR HuARMalloc(u32 size);
void HuARFree(AMEM_PTR aMemP);
void HuAMemDump(void);
AMEM_PTR HuAR_DVDtoARAM(unsigned int dir);
AMEM_PTR HuAR_MRAMtoARAM(int dir);
AMEM_PTR HuAR_MRAMtoARAM2(void *dirPtr);
void *fn_80055D40(AMEM_PTR aMemP);
void *fn_80055D6C(AMEM_PTR aMemP, s32 num);
s32 fn_80055FB4(void);
AMEM_PTR HuARDirCheck(unsigned int dir);
void HuARDirFree(unsigned int dir);
void *fn_8005607C(unsigned int dataNum, u32 num, HEAPID heap);

#endif
