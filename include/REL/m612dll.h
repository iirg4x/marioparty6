/* Shared scene data and function declarations for Memory Lane. */
#ifndef _REL_M612DLL_H
#define _REL_M612DLL_H

#include "game/main.h"
#include "game/object.h"
#include "game/mg/seqman.h"

/* Scene-wide objects and models used by the four Memory Lane boards. */
typedef struct M612Scene {
    OMOBJMAN *manager; /* Object manager for scene callbacks and objects. */
    s32 state; /* Current shared minigame state. */
    s16 flag; /* Scene flag cleared during initialization. */
    HU3D_MODELID tiles[4][49]; /* Forty-nine tile models for each player board. */
    /* Reserved scene value; it has no gameplay use in the Memory Lane scene code. */
    s16 unused;
    HU3D_MODELID borders[2]; /* Boundary models at the near and far board edges. */
    OMOBJ *players; /* Player-object pointer; no use is established in this file. */
    OMOBJ *guide; /* Object that advances the shared guide along the selected route. */
} M612Scene;

extern M612Scene lbl_1_bss_4AC;
extern HU3D_MOTIONID lbl_1_bss_44C[4][8];
extern s8 lbl_1_data_198[90][60];
extern s8 lbl_1_data_16B0[10][7];
extern GXColor lbl_1_data_84[2];
void fn_1_A0(void);
void fn_1_8F0(void);
void fn_1_F0C(void);
s32 fn_1_13A4(s32 column, s32 row);
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
void fn_1_4B94(s32 cameraIndex);
void fn_1_51D8(void);
void fn_1_54D4(OMOBJ *obj);
void fn_1_5924(OMOBJ *obj);
float fn_1_5E00(s32 direction);
void fn_1_5E90(s32 direction, HuVecF *position, float distance);
void fn_1_5EF0(s32 direction, s32 *column, s32 *row);
void fn_1_5F58(s32 player);
u32 MgSeqModeNext(void);

#endif
