#ifndef M651DLL_H
#define M651DLL_H

#include "dolphin.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/mg/timer.h"
#include "game/mg/seqman.h"

typedef struct M651Work14 {
    s32 unk_00;
    float unk_04;
    s32 triggeredF[5];
    HU3D_MODELID unk_1C;
    HU3D_MODELID modelIds1E[5];
    HU3D_MODELID modelIds28[5];
    HU3D_MODELID modelIds32[5];
    HU3D_MOTIONID motionIds[5];
    s16 delayFrames[5];
} M651Work14;

typedef struct M651Work64 {
    HU3D_MODELID modelIds[5];
    s32 unk_0C;
    float unk_10;
    u32 unk_14;
    float unk_18;
    float unk_1C;
    float unk_20;
    float unk_24;
    float unk_28;
    float unk_2C;
    float unk_30;
    HU3D_MODELID unk_34;
    float unk_38;
    HU3D_MODELID unk_3C;
    HU3D_MODELID unk_3E;
    s32 unk_40;
} M651Work64;

typedef struct M651Player {
    s16 playerNo;
    s16 groupNo;
    s16 characterNo;
    s16 padNo;
    HU3D_MODELID modelId;
    HU3D_MOTIONID motionIds[5];
    HuVecF pos;
    s16 unk_20;
    s32 unk_24;
    s32 unk_28;
    s32 unk_2C;
    HU3D_MODELID unk_30;
    HU3D_MOTIONID unk_32;
    HuVecF unk_34;
    HuVecF unk_40;
    HuVecF unk_4C;
    HuVecF unk_58;
    float unk_64;
    s32 cpuF;
    s16 difficulty;
    s16 unk_6E;
    s32 fxNo;
} M651Player;

extern OMOBJMAN *lbl_1_bss_0;
extern s16 lbl_1_bss_4;
extern float lbl_1_bss_10;
extern M651Work14 lbl_1_bss_14;
extern M651Work64 lbl_1_bss_64;
extern M651Player lbl_1_bss_A8[2];
extern s16 lbl_1_data_0[6];
extern s32 lbl_1_data_C;
extern s32 lbl_1_data_10;
extern MGSEQ_PARAM lbl_1_data_14;
extern unsigned int lbl_1_data_70[6];
extern int lbl_1_data_88[4];
extern float lbl_1_data_98;
extern HuVecF lbl_1_data_9C;
extern float lbl_1_data_A8;
extern s32 lbl_1_data_AC;
extern s32 lbl_1_data_B0;

OMOBJMAN *fn_1_A0(void);
void fn_1_B0(s16 playerNo);
void fn_1_10C(s16 playerNo);
void fn_1_190(void);
void fn_1_1B0(s16 mode, s16 frameNo);
void fn_1_1D0(s16 mode, s16 frameNo);
void fn_1_240(s16 mode, s16 frameNo);
void fn_1_244(s16 mode, s16 frameNo);
void fn_1_268(s16 mode, s16 frameNo);
void fn_1_3D4(s16 mode, s16 frameNo);
void fn_1_400(s16 mode, s16 frameNo);
void fn_1_404(s16 mode, s16 frameNo);
void fn_1_408(s16 mode, s16 frameNo);
void fn_1_40C(void);
void fn_1_4A0(void);
void fn_1_62C(void);
void fn_1_69C(OMOBJ *obj);
void fn_1_72C(OMOBJ *obj);
void fn_1_C20(s16 layerNo);
void fn_1_D28(void);
void fn_1_1220(void);
void fn_1_1344(void);
void fn_1_13D8(void);
void fn_1_18DC(OMOBJ *obj);
void fn_1_18EC(OMOBJ *obj);
void fn_1_192C(OMOBJ *obj);
void fn_1_1E60(void);
void fn_1_2034(OMOBJ *obj);
void fn_1_23D4(OMOBJ *obj);
void fn_1_242C(OMOBJ *obj);
void fn_1_24BC(OMOBJ *obj);
void fn_1_25B0(void);
void fn_1_29A4(OMOBJ *obj);
void fn_1_2D8C(OMOBJ *obj);
void fn_1_35D8(OMOBJ *obj);
void fn_1_35EC(M651Player *work, float y1, float y2, float x1, float x2);
void fn_1_36F8(void);
void fn_1_3F30(void);

#endif
