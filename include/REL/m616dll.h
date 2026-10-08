/* Shared state for M616's timed choice sequence and its camera and character models. */
#ifndef M616DLL_H
#define M616DLL_H

#include "dolphin.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/mg/timer.h"

typedef struct M616Work {
    OMOBJMAN *objectManager; /* Object manager for the camera update callback. */
    s16 sequenceState; /* Current step of the choice-round sequence. */
    u32 sequenceFrame; /* Frames elapsed in the current sequence step. */
    HU3D_MODELID activeCameraModel; /* Camera motion model currently driving camera 0. */
    HU3D_MOTIONID cameraMotions[17]; /* Camera motions loaded from M616 data. */
    OMOBJ *cameraObject; /* Object that samples the active camera motion each frame. */
    float cameraTime; /* Current elapsed time of the active camera motion, in frames. */
    float cameraMaxTime; /* Duration of the active camera motion, in frames. */
    HU3D_MODELID stageModels[2]; /* Two animated stage models. */
    HU3D_MODELID playerBaseModels[4]; /* Player display bases, one for each seat. */
    HU3D_MOTIONID playerBaseMotions[4][13]; /* Display-base motions for each seat. */
    HU3D_MODELID centerModel; /* Center gear model used to present round and final results. */
    HU3D_MOTIONID centerMotions[3]; /* Center gear motions for the three outcomes. */
    HU3D_MODELID playerModels[4]; /* Per-seat player display models. */
    HU3D_MOTIONID playerMotions[6]; /* Motions shared by the player display models. */
    /* Model that owns the attachment points used to place the four seat display models. */
    HU3D_MODELID playerHookRoot;
    /* Model carrying the P1stmov object and timing hook during the opening animation. */
    HU3D_MODELID openingMotionModel;
    s32 characterNos[4]; /* Character IDs assigned to the four seats. */
    s32 padNos[4]; /* Controller port assigned to each seat. */
    HU3D_MODELID characterModels[4]; /* Animated character models for the four seats. */
    HU3D_MOTIONID characterMotions[4][11]; /* Character motions indexed by seat and action. */
    u16 cpuChoices[4]; /* CPU's selected answer for the current round, 1 through 4. */
    u32 cpuInputFrames[4]; /* Answer-phase frame at which each CPU submits its answer. */
    u16 previousButtons[4]; /* Held choice buttons from the previous update. */
    s32 choices[4]; /* Current player answers; zero means no answer yet. */
    s32 scores[4]; /* Unique-answer points earned by each seat. */
    s32 roundNo; /* Zero-based round number, ending at 9. */
    MGTIMER *timer; /* Countdown timer for the answer window. */
    s32 timingState; /* Next opening-animation event handled by fn_1_139C. */
    HU3D_MODELID resultModels[4]; /* Per-seat result props shown for winners. */
    HuVecF shadowPos; /* Position of the scene shadow light. */
    HuVecF shadowTarget; /* Point the scene shadow light aims toward. */
    HuVecF shadowUp; /* Up direction used to orient the scene shadow light. */
    s32 streamNo; /* Active streamed music handle, or -1 when none is active. */
} M616Work;

extern M616Work lbl_1_bss_10;

void Hu3DMotionTimingHookReset(HU3D_MODELID modelId);

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
void fn_1_1364(OMOBJ *cameraObject);
void fn_1_139C(HU3D_MODELID modelId, HU3D_MOTIONID motionId, BOOL timingLag);
void fn_1_1590(void);
void fn_1_1FF4(s32 cameraIndex);
BOOL fn_1_20D8(void);
void fn_1_2104(s32 playerNo, s32 motionNo);
void fn_1_215C(s32 playerNo, s32 motionNo);
void fn_1_2254(s32 motionNo);
void fn_1_22BC(s32 playerNo, s32 motionNo, u32 motionAttributes);
void fn_1_23B0(void);
s32 fn_1_2580(s32 playerNo);

#endif
