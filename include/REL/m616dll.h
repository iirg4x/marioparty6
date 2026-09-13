#ifndef M616DLL_H
#define M616DLL_H

#include "dolphin.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/mg/timer.h"

typedef struct M616Work {
    OMOBJMAN *objectManager;
    s16 sequenceState;
    u32 sequenceFrame;
    HU3D_MODELID activeCameraModel;
    HU3D_MOTIONID cameraMotions[17];
    OMOBJ *cameraObject;
    float cameraTime;
    float cameraMaxTime;
    HU3D_MODELID stageModels[2];
    HU3D_MODELID playerBaseModels[4];
    HU3D_MOTIONID playerBaseMotions[4][13];
    HU3D_MODELID centerModel;
    HU3D_MOTIONID centerMotions[3];
    HU3D_MODELID playerModels[4];
    HU3D_MOTIONID playerMotions[6];
    HU3D_MODELID unk_CC;
    HU3D_MODELID unk_CE;
    s32 characterNos[4];
    s32 padNos[4];
    HU3D_MODELID characterModels[4];
    HU3D_MOTIONID characterMotions[4][11];
    u16 cpuChoices[4];
    u32 cpuInputFrames[4];
    u16 previousButtons[4];
    s32 choices[4];
    s32 scores[4];
    s32 roundNo;
    MGTIMER *timer;
    s32 timingState;
    HU3D_MODELID resultModels[4];
    HuVecF shadowPos;
    HuVecF shadowTarget;
    HuVecF shadowUp;
    s32 streamNo;
} M616Work;

extern M616Work lbl_1_bss_10;

s32 fn_1_A0(s32 streamNo, s32 bgmId);
void fn_1_104(s32 streamNo);
void fn_1_140(s16 mode, s16 frameNo);
void fn_1_160(s16 mode, s16 frameNo);
void fn_1_4BC(s16 mode, s16 frameNo);
void fn_1_4C0(s16 mode, s16 frameNo);
void fn_1_F14(s16 mode, s16 frameNo);
void fn_1_F78(s16 mode, s16 frameNo);
void fn_1_1284(s16 mode, s16 frameNo);
void fn_1_135C(s16 mode, s16 frameNo);
void fn_1_1360(s16 mode, s16 frameNo);
void fn_1_1364(OMOBJ *object);
void fn_1_139C(HU3D_MODELID modelId, HU3D_MOTIONID motionId, BOOL lagF);
void fn_1_1590(void);
void fn_1_1FF4(s32 index);
BOOL fn_1_20D8(void);
void fn_1_2104(s32 playerNo, s32 motionNo);
void fn_1_215C(s32 playerNo, s32 motionNo);
void fn_1_2254(s32 motionNo);
void fn_1_22BC(s32 playerNo, s32 motionNo, u32 attr);
void fn_1_23B0(void);
s32 fn_1_2580(s32 playerNo);

#endif
