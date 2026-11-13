/* Declarations for the opening overlay's constructors and module storage. */
#ifndef REL_OPENINGDLL_OPENING_H
#define REL_OPENINGDLL_OPENING_H

#include "game/hu3d.h"
#include "game/object.h"
#include "game/window.h"

#define OPENING_WIN_COUNT 4

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

extern s16 lbl_1_bss_0;
extern s32 lbl_1_bss_4;
extern s32 lbl_1_bss_8;
extern s32 lbl_1_bss_C;
extern float lbl_1_bss_10;
extern HuVecF lbl_1_bss_14;
extern float lbl_1_bss_20;
extern HuVec2f lbl_1_bss_24;
extern s16 lbl_1_bss_2C;
extern float lbl_1_bss_38;
extern HuVecF lbl_1_bss_3C;
extern HuVecF lbl_1_bss_48;
extern HuVecF lbl_1_bss_54;
extern HUWINID lbl_1_bss_2E[OPENING_WIN_COUNT];
extern HU3D_ANIMID lbl_1_bss_60[12];
extern HU3D_ANIMID lbl_1_bss_78;
extern ANIMDATA *lbl_1_bss_7C[79];
extern ANIMDATA *lbl_1_bss_1B8[2];
extern void *lbl_1_bss_1C0;
extern void *lbl_1_bss_1C4[2];
extern HU3D_MODELID lbl_1_bss_1CE[8];
extern s16 lbl_1_bss_1DE[3];
extern OMOBJMAN *lbl_1_bss_1E4;
extern s16 lbl_1_data_0[OPENING_WIN_COUNT];
extern u32 lbl_1_data_8[80];
extern char *lbl_1_data_1DC[12];
extern char *lbl_1_data_2F0[12];
extern HuVecF lbl_1_data_320;
extern HuVecF lbl_1_data_32C;
extern char lbl_1_data_338[14];
extern char lbl_1_data_346[15];
extern char lbl_1_data_355[20];

#endif
