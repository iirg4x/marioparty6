/* Defines the callback job pool used for entrances, eliminations and effects. */
#ifndef REL_M631_UTILITIES_H
#define REL_M631_UTILITIES_H
#include "REL/m631Dll/m631_work.h"
typedef struct M631Job M631Job;
typedef struct M631Jobs {
    M631Job *freeHead; /* First available job, or NULL when the pool is full. */
    M631Job *activeHead; /* First scheduled job, or NULL when no jobs are active. */
    M631Job *storage; /* Beginning of the pool's job array. */
} M631Jobs;
struct M631Job {
    M631Jobs *owner; /* Pool to which this job returns. */
    M631Job *prev; /* Previous active job; NULL at the head or while free. */
    M631Job *next; /* Next active or free job, depending on its list. */
    s32 (*callback)(void *); /* Runs once per update; nonzero completes the job. */
    void *data; /* Actor or effect state passed to the callback. */
};
extern M631MotionPairNode *lbl_1_bss_10;
extern M631Job lbl_1_bss_14[256];
extern M631Jobs lbl_1_bss_1414;
void fn_1_5284(OMOBJ *object);
void fn_1_5AAC(M631ActorWork *actor, s32 teamIndex, s32 isGroundPlayer);
void fn_1_A8F0(M631ActorWork *actor);
void fn_1_AA78(M631ActorWork *actor);
void fn_1_228C(HU3D_MODELID *models);
f32 fn_1_2528(void);
M631AnimationQueue *fn_1_2E9C(void);
M631AnimationNode *fn_1_31D8(M631AnimationQueue *owner);
void fn_1_32C8(M631AnimationNode *inputWork);
s32 fn_1_37FC(void *callbackData);
void fn_1_38E8(M631MotionPairNode *callback, M631MotionPairNode *context);
void fn_1_3B44(M631ActorWork *actor);
void fn_1_3B7C(HuVecF *position);
void fn_1_3BC4(void);
void fn_1_3BF4(M631ActorMotionState *hitEffect);
M631ActorWork *fn_1_469C(const HuVecF *position);
void fn_1_5014(HuVecF *destination);
void fn_1_5054(void);
void fn_1_5084(void);
s16 fn_1_50B4(void);
void fn_1_50DC(HuVecF *direction, f32 magnitude);
s32 fn_1_51D0(f32 positionX);
s32 fn_1_AAD8(void *callbackData);
void fn_1_AB4C(void);
void fn_1_AB60(void);
void fn_1_AF94(M631MotionPairNode *object);
M631MotionPairNode *fn_1_B004(void);
void fn_1_B014(void);
void *fn_1_B0B8(s32 (*callback)(void *), void *data);
void fn_1_B1AC(void);
#endif
