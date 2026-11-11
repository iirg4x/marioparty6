#ifndef _REL_MICQUIZMYOKODLL_MODULE_TYPES_H_
#define _REL_MICQUIZMYOKODLL_MODULE_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "math.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
void fn_1_8200(void);
void fn_1_8280(s32 modelIndex);
void fn_1_829C(s16 layerNo);
void fn_1_1BAC(s32 messageSet);
void fn_1_3258(void);

typedef struct MicquizControlWork_s {
    s16 sideModelIds[2];
    s16 shadowModelIds[2];
    s16 modelIds[5];
    s16 phase;
    s16 timer;
    s16 completionFlag;
    s16 characterModelsCreated;
    u8 mode;
    u8 currentPlayer;
    u8 flag_80 : 1;
    u8 flag_40 : 1;
    u8 unknown_1C_2 : 6;
} MicquizControlWork;

typedef struct MicquizAnimationState_s {
    s16 modelIds[5];
    s16 states[5];
    s16 frame;
    s16 currentModel;
    s16 animationIds[5];
    u8 unknown_22[2]; /* No code names this field. */
    ANIMDATA *spriteAnimations[5];
    s16 playerStates[5];
} MicquizAnimationState;

typedef struct MicquizHostState_s {
    s16 charNo;
    s16 modelId;
    s16 motionIds[9];
} MicquizHostState;

typedef struct MicquizPlayerWork_s {
    s16 positionIndex;
    s16 characterId;
    s16 accessorySetIndex;
    s16 accessoryCount;
    s16 secondarySetIndex;
    s16 secondaryCount;
    s16 motionStep;
    s16 modelId;
    s16 alternateModelId;
    s16 motionId;
    s16 characterModelId;
    s16 characterMotions[9];
    s16 models_28[5];
    s16 models_32[5];
    f32 motionValueA;
    f32 motionValueB;
    f32 motionValueC;
    f32 motionValueD;
} MicquizPlayerWork;

typedef struct MicquizAccessoryPoolEntry_s {
    s16 modelId;
    s16 jointMotions[4];
} MicquizAccessoryPoolEntry;

typedef struct MicquizSequence_s {
    s16 playerIndex;
    s16 modelIndex;
    s16 phase;
    s16 playerSlot;
    s16 responseMode;
    s16 motionStep;
    s16 timer;
    s16 unknown_0E;
    f32 sequenceValue;
    f32 sequenceOffset;
    f32 oscillationPhase;
    f32 verticalMotion;
} MicquizSequence;

#ifdef __cplusplus
}
#endif

#endif
