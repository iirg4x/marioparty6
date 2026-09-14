#ifndef _REL_M612DLL_H
#define _REL_M612DLL_H

#include "game/main.h"
#include "game/object.h"
#include "game/mg/seqman.h"

typedef struct M612Scene {
    OMOBJMAN *manager;
    s32 state;
    s16 flag;
    HU3D_MODELID tiles[4][49];
    /* Unused two-byte interval; original type and purpose are unresolved. */
    s16 unk_192;
    HU3D_MODELID borders[2];
    OMOBJ *players;
    OMOBJ *guide;
} M612Scene;

extern M612Scene lbl_1_bss_4AC;
extern HU3D_MOTIONID lbl_1_bss_44C[4][8];
extern s8 lbl_1_data_198[90][60];
extern s8 lbl_1_data_16B0[10][7];
extern GXColor lbl_1_data_84[2];
void fn_1_A0(void);
void fn_1_8F0(void);
void fn_1_F0C(void);
s32 fn_1_13A4(s32 x, s32 y);
void fn_1_1430(void);
void fn_1_1694(s16 mode, s16 frameNo);
void fn_1_16D4(s16 mode, s16 frameNo);
void fn_1_1DBC(s16 mode, s16 frameNo);
void fn_1_1E18(s16 mode, s16 frameNo);
void fn_1_1FB4(s16 mode, s16 frameNo);
void fn_1_242C(s16 mode, s16 frameNo);
void fn_1_2BC8(s16 mode, s16 frameNo);
void fn_1_2C8C(s16 mode, s16 frameNo);
void fn_1_2C90(s16 mode, s16 frameNo);
void fn_1_2C94(OMOBJ *obj);
void fn_1_2CEC(s32 player);
s32 fn_1_4038(s32 player);
s32 fn_1_4150(s32 player);
s32 fn_1_42C4(s32 player);
s32 fn_1_4480(s32 player);
void fn_1_48D4(void);
void fn_1_4B94(s32 arg0);
void fn_1_51D8(void);
void fn_1_54D4(OMOBJ *obj);
void fn_1_5924(OMOBJ *obj);
float fn_1_5E00(s32 direction);
void fn_1_5E90(s32 direction, HuVecF *pos, float distance);
void fn_1_5EF0(s32 direction, s32 *x, s32 *y);
void fn_1_5F58(s32 player);
u32 MgSeqModeNext(void);

#endif
