/* Black Hole's shared types and declarations for the duel sequence. */
#ifndef M651DLL_H
#define M651DLL_H

#include "dolphin.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/mg/timer.h"
#include "game/mg/seqman.h"

typedef struct M651Work14 {
    s32 resultStarted; /* Set when either player reaches the finish. */
    float meteorMotionSpeed; /* Shared motion speed, starting at 1.0 and capped at 2.6. */
    s32 approachEffectTriggered[5]; /* Whether each of the first four meteors has triggered its
                                     * preloaded approach effect. */
    HU3D_MODELID hookedMeteorModel; /* Model attached to the final meteor's kuriboo_null joint. */
    HU3D_MODELID meteorDisplayModels[5]; /* Linked display models for the meteor pairs; the first
                                          * four follow their meteors and entry 4 stays hidden. */
    HU3D_MODELID meteorApproachModels[5]; /* Linked approach-effect models used by the first four
                                           * meteors near the scene center. */
    HU3D_MODELID meteorModels[5]; /* Five falling meteor models. */
    HU3D_MOTIONID meteorMotions[5]; /* Assigned motion for each falling meteor model. */
    s16 meteorDelayFrames[5]; /* Frames remaining before one of the first four meteors starts its
                               * next fall. */
} M651Work14;

typedef struct M651Work64 {
    HU3D_MODELID sceneModels[5]; /* Five models used to build the opening and duel scene. */
    s32 resultAnimationActive; /* Set when the result animation begins. */
    float resultScaleStep; /* Per-frame reduction subtracted from both background scales while the
                            * result scene shrinks. */
    u32 sceneFrame; /* Frame counter for the opening scene; reset when the result animation
                     * starts. */
    float mainSceneRotation; /* Rotation angle in degrees for sceneModels[0]. */
    float mainSceneScale; /* Uniform scale for sceneModels[0], from 0 to 1. */
    float backgroundRotation; /* Rotation angle in degrees for sceneModels[1] and sceneModels[2]. */
    float firstBackgroundScale; /* Scale for sceneModels[1], enlarged during the duel and shrunk for
                                 * results. */
    float secondBackgroundScale; /* Scale for sceneModels[2], enlarged during the duel and shrunk
                                  * for results. */
    float closingModelRotation; /* Rotation angle in degrees for sceneModels[4]. */
    float sceneRotationSpeedOffset; /* Added to scene rotation each update; rises to 1.2 before a
                                     * finish, then decreases by 0.01 per update and can become
                                     * negative. */
    HU3D_MODELID animatedSceneModel; /* Animated scene model whose motion time drives sound cues. */
    float unusedSceneSpeedAccumulator; /* Increases by 0.002 each update but is not read
                                        * elsewhere. */
    HU3D_MODELID approachEffectModel; /* Scene effect model set to loop during setup; its loop is
                                       * cleared after a player reaches the finish. */
    HU3D_MODELID approachEffectOverlayModel; /* Overlay initially hidden and shown when either
                                              * player reaches z<1700; its loop is disabled when
                                              * neither player is below that depth or either reaches
                                              * the finish. */
    s32 finalMeteorCheckDone; /* Set before the random check, so a failed roll is not retried. */
} M651Work64;

typedef struct M651Player {
    s16 playerNo; /* Global player slot, from 0 to 3. */
    s16 groupNo; /* Duel side, 0 for P1 and 1 for P2. */
    s16 characterNo; /* Character selected for this player. */
    s16 padNo; /* Controller slot used for human input. */
    HU3D_MODELID modelId; /* Character model controlled during the duel. */
    HU3D_MOTIONID motionIds[5]; /* Character motions for crossing, run, finish, win, and final
                                 * result poses, in that order. */
    HuVecF pos; /* Character position in scene coordinates. */
    s16 movementBoostFrames; /* Remaining frames of faster character motion after an input. */
    s32 reachedFinish; /* Set to 1 after the character crosses scene depth z=1000. */
    s32 finishResolved; /* Set once the duel has selected its result. */
    s32 inputReceived; /* Set after this player presses A or receives a CPU input during the
                        * duel. */
    HU3D_MODELID resultHookModel; /* Scene model holding the P1 or P2 hook used for the result
                                   * pose. */
    HU3D_MOTIONID resultHookMotion; /* Joint motion assigned to resultHookModel. */
    HuVecF crossingStart; /* Start point of the post-result path, in scene coordinates. */
    HuVecF crossingEnd; /* End point of the post-result path, in scene coordinates. */
    HuVecF crossingPosition; /* Current position along the post-result path, in scene
                              * coordinates. */
    HuVecF crossingDirection; /* Unit vector from crossingStart toward crossingEnd. */
    float crossingYaw; /* Character yaw in degrees while crossing the result scene. */
    s32 isCpu; /* Nonzero when this player is controlled by the CPU. */
    s16 cpuDifficulty; /* Configured CPU difficulty index, used to select its input delay. */
    s16 cpuInputDelayFrames; /* Frames until the next CPU input. */
    s32 effectHandle; /* Character effect handle, or -1 before the finish effect starts. */
} M651Player;

extern OMOBJMAN *lbl_1_bss_0;
extern s16 lbl_1_bss_4[6];
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
void fn_1_1B0(s16 sequenceMode, s16 sequenceFrame);
void fn_1_1D0(s16 sequenceMode, s16 sequenceFrame);
void fn_1_240(s16 sequenceMode, s16 sequenceFrame);
void fn_1_244(s16 sequenceMode, s16 sequenceFrame);
void fn_1_268(s16 sequenceMode, s16 sequenceFrame);
void fn_1_3D4(s16 sequenceMode, s16 sequenceFrame);
void fn_1_400(s16 sequenceMode, s16 sequenceFrame);
void fn_1_404(s16 sequenceMode, s16 sequenceFrame);
void fn_1_408(s16 sequenceMode, s16 sequenceFrame);
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
void fn_1_35EC(M651Player *playerState, float startY, float endY, float startX, float endX);
void fn_1_36F8(void);
void fn_1_3F30(void);

#endif
