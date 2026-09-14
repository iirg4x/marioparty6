#ifndef _REL_M629DLL_H
#define _REL_M629DLL_H

#include "game/main.h"
#include "game/object.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"

typedef struct M629Impulse {
    s32 state;
    float value;
} M629Impulse;

typedef struct M629Player {
    s32 index;
    s32 playerNo;
    s32 unk_08;
    s32 charNo;
    s32 padNo;
    s32 difficulty;
    u8 unk_18[4];
    s32 unk_1C;
    s32 unk_20;
    HuVecF itemPos;
    float unk_30;
    float unk_34;
    float unk_38;
    s32 motion;
    s32 frame;
    float velocityY;
    float unk_48;
    s32 unk_4C;
    float unk_50;
} M629Player;

typedef struct M629Sound {
    s32 count;
    float x[3];
    s32 handle;
} M629Sound;

extern M629Impulse lbl_1_bss_69C[4][16];
extern Mtx lbl_1_bss_30[30];
extern OMOBJ *lbl_1_bss_5E0;
extern s32 lbl_1_bss_10;
extern s32 lbl_1_bss_0;
extern s32 lbl_1_bss_C;
extern s32 lbl_1_bss_28;
extern s32 lbl_1_bss_2C;
extern s32 lbl_1_bss_5D0;
extern s16 lbl_1_bss_5D4;
extern OMOBJMAN *lbl_1_bss_5D8;
extern OMOBJ *lbl_1_bss_5E4[4];
extern OMOBJ *lbl_1_bss_5F4;
extern OMOBJ *lbl_1_bss_5F8;
extern M629Sound lbl_1_bss_5FC[8];
extern MGTIMER *lbl_1_bss_4[2];
extern s32 lbl_1_bss_14[5];
extern s32 lbl_1_bss_5DC;
extern int lbl_1_data_38[10];
extern u8 lbl_1_data_60[10];
extern u8 lbl_1_data_6A[10];
extern u8 lbl_1_data_74[10];
extern s32 lbl_1_data_80[10];
extern s32 lbl_1_data_C0;
extern s32 lbl_1_data_AC;
extern s32 lbl_1_data_B4;
extern s32 lbl_1_data_B0;
extern float lbl_1_data_B8[2];
extern s32 lbl_1_data_C4;
extern s32 lbl_1_data_C8;
extern float lbl_1_data_160[2];
extern s32 lbl_1_data_168[2];
extern MGSEQ_PARAM lbl_1_data_FC;
extern s32 lbl_1_data_CC[4];
extern s32 lbl_1_data_DC[4];
extern s32 lbl_1_data_EC[4];
extern HuVecF lbl_1_data_134;
extern HuVecF lbl_1_data_140;
extern GXColor lbl_1_data_14C;
extern GXColor lbl_1_data_1B4;
extern HuVecF lbl_1_data_1B8;

float fn_1_0(float x, float y);
void fn_1_134(float *x, float *y, float limit);
void fn_1_2DC(int *x, int *y, float limit);
void fn_1_5CC(void);
void fn_1_940(s32 group, float value);
void fn_1_9E8(OMOBJ *obj);
float fn_1_1DCC(float frame, float rise, float hold, float fall);
void fn_1_1F90(OMOBJ *obj);
void fn_1_3324(s16 mode, s16 frameNo);
void fn_1_3328(s16 mode, s16 frameNo);
void fn_1_332C(s16 mode, s16 frameNo);
void fn_1_3330(s16 mode, s16 frameNo);
void fn_1_3334(s16 mode, s16 frameNo);
void fn_1_3338(s16 mode, s16 frameNo);
void fn_1_333C(s16 mode, s16 frameNo);
void fn_1_336C(OMOBJ *obj);
void fn_1_3C8C(OMOBJ *obj);
void fn_1_4290(HU3D_MODEL *model, Mtx *matrix);
void fn_1_4368(HU3D_MODEL *model, Mtx *matrix);
void fn_1_444C(OMOBJ *obj);
void fn_1_53B8(OMOBJ *obj);
void fn_1_53C8(void);
void fn_1_58BC(s16 mode, s16 frameNo);
void fn_1_5A08(s16 mode, s16 frameNo);

#endif
