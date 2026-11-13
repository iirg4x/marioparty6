#ifndef _GAME_DATA_H
#define _GAME_DATA_H

#include "game/memory.h"

#include "datadir_enum.h"

#include "dolphin/types.h"
#include "dolphin/dvd.h"

typedef struct HuDataStat_s {
    s32 dirId;
    union {
        void *dirP;
        u8 *dirBytes; // Byte address for continuing an archive read.
    };
    void *fileDataP;
    u32 rawLen;
    u32 readLen;
    u32 readOfs;
    u32 decodeType;
    BOOL used;
    s32 num;
    u32 status;
    DVDFileInfo dvdFile;
} HUDATASTAT;

void fn_80007668(void);
s32 fn_80007738(int dataNum);
HUDATASTAT *fn_800077AC(void *dirP);
void *fn_80007810(int dirNum);
HUDATASTAT *fn_800078C8(int dirNum);
HUDATASTAT *fn_80007B00(void *dirP, int dataNum);
void fn_80007C88(s32 result, DVDFileInfo* fileInfo);
s32 fn_80007F20(int dataNum);
s32 fn_800080C8(int dataNum, s32 num);
void fn_80007DE4(s16 statId);
void fn_80007E6C(s16 statId);
BOOL fn_80008270(s32 statId);
void *fn_800082D0(int dataNum);
void *fn_80008634(int dataNum, s32 num);
void *fn_800089EC(int dataNum, HEAPID heap);
void *fn_80008DD0(int dataNum, s32 num, HEAPID heap);
void **HuDataReadMulti(int *dataNum);
s32 fn_8000963C(int dataNum);
void fn_80009740(void *ptr);
void HuDataCloseMulti(void **ptrTbl);
void fn_800097F4(int dataNum);
void fn_800098E4(s32 num);
void fn_80009A30(void);
BOOL fn_80007EAC(s16 statId);
void *fn_80009B04(u32 dataNum, s32 num, HEAPID heap);

void fn_8000A384(void *src, void *dst, u32 size, s32 decode_type);

extern u32 lbl_802C0028;

#endif
