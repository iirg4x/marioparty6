#ifndef _REL_M621DLL_H
#define _REL_M621DLL_H

#include "game/main.h"
#include "game/object.h"
#include "game/mg/seqman.h"
#include "game/mg/actman.h"
#include "game/mg/score.h"

typedef struct M621Target_s M621Target;
typedef struct M621Effect_s M621Effect;
typedef struct M621Environment_s M621Environment;

typedef struct {
    s16 unk_00;
    s16 timer;
    s32 sound;
} M621Round;

typedef struct {
    s32 state;
    HU3D_MOTIONID motion[2];
    HU3D_MODELID cameraModel;
} M621Camera;

typedef struct {
    s32 timer;
    MGPLAYER *player;
    float baseRotation;
    HuVecF basePos;
    s16 score;
    s16 winnerF;
    s16 playerNo;
    s16 charNo;
    s32 motionNo;
    s16 attackF;
    s16 unk_26;
    s16 delay;
    s16 scoreBox;
    MGSCORE *scoreDisplay;
    s16 computerF;
    s16 difficulty;
    s16 cpuState;
    s16 cpuTimer;
    s16 cpuDelay;
    M621Target *target;
} M621Player;

extern OMOBJMAN *lbl_1_bss_0;
extern MGSEQ_PARAM lbl_1_data_0;
extern s32 lbl_1_data_28;
extern s16 lbl_1_bss_8;
extern s32 lbl_1_bss_C;
extern M621Player *lbl_1_bss_10[4];
extern M621Effect *lbl_1_bss_20;
extern HU3D_MOTIONID lbl_1_bss_24[4];
extern HU3D_MODELID lbl_1_bss_2C[4];
extern s32 lbl_1_data_9C[4][2];
extern M621Environment *lbl_1_bss_4C;

void *fn_1_A0(s32 priority, u32 size, OMOBJ_FUNC callback);
void fn_1_140(OMOBJ *obj, OMOBJ_FUNC callback);
void fn_1_148(void);
void fn_1_2BC(void);
void fn_1_2E0(s16 mode, s16 frameNo);
void fn_1_358(s16 mode, s16 frameNo);
void fn_1_35C(s16 mode, s16 frameNo);
void fn_1_3B8(s16 mode, s16 frameNo);
void fn_1_3BC(s16 mode, s16 frameNo);
void fn_1_410(s16 mode, s16 frameNo);
void fn_1_414(s16 mode, s16 frameNo);
void fn_1_418(s16 mode, s16 frameNo);
void fn_1_41C(s16 mode, s16 frameNo);
s32 fn_1_420(HuVecF *pos, s16 soundId);
s32 fn_1_594(s16 pan, s16 soundId);
void fn_1_5C8(s32 sound);
void fn_1_5F0(OMOBJ *obj);
void fn_1_70C(OMOBJ *obj);
void fn_1_778(OMOBJ *obj);
void fn_1_A5C(OMOBJ *obj);
void fn_1_B4C(OMOBJ *obj);
void fn_1_C90(OMOBJ *obj);
void fn_1_CEC(OMOBJ *obj);
void fn_1_D28(OMOBJ *obj);
void fn_1_106C(M621Player *work, s32 motionNo, float blendTime, u32 attr);
void fn_1_13EC(OMOBJ *obj);
void fn_1_175C(OMOBJ *obj);
void fn_1_1844(OMOBJ *obj);
void fn_1_19C4(OMOBJ *obj);
void fn_1_1A3C(OMOBJ *obj);
void fn_1_1B5C(OMOBJ *obj);
void fn_1_1BF4(OMOBJ *obj);
void fn_1_215C(OMOBJ *obj);
void fn_1_27A0(void);
void fn_1_5054(OMOBJ *obj);
void fn_1_52D4(s32 playerNo, s16 coins);
void fn_1_5328(s32 playerNo, s32 score);
s16 fn_1_5340(void);
float fn_1_5350(float value);
double fn_1_537C(double value);

#endif
