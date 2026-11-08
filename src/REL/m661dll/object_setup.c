/* Stage setup: loads the players, stage models and pieces, places the pieces at random
 * free points, creates the spotlight, timer and score display, and runs the opening
 * scene. */
#include "game/data.h"
#include "game/hu3d.h"
#include "game/memory.h"

typedef struct ModelSetupRecord {
    HU3D_MODELID modelId;
    HuVecF position;
    HuVecF shadowPosition;
    HuVecF rotation;
} ModelSetupRecord;

extern ModelSetupRecord lbl_1_bss_1E3C[0x71];
extern s32 lbl_1_data_100[35];

#include "math.h"
#include "dolphin.h"
#include "dolphin/types.h"
#include "REL/m661dll/motion.h"
#include "REL/m661dll/system_state.h"
#include "REL/m661dll/player_actors.h"
#include "REL/m661dll/particle.h"
#include "REL/m661dll/sound.h"

extern MotionControl lbl_1_bss_10[0x4E];

typedef struct BoundaryEdge { f32 x0; f32 x1; f32 z0; f32 z1; } BoundaryEdge;
s32 lbl_1_data_100[35] = {
    DATANUM(0x780000, 0), DATANUM(0x780000, 1), DATANUM(0x780000, 2),
    DATANUM(0x780000, 3), DATANUM(0x780000, 4), DATANUM(0x780000, 5),
    DATANUM(0x780000, 6), DATANUM(0x780000, 7), DATANUM(0x780000, 8),
    DATANUM(0x780000, 9), DATANUM(0x780000, 10), DATANUM(0x780000, 11),
    DATANUM(0x780000, 12), DATANUM(0x780000, 13), DATANUM(0x780000, 14),
    DATANUM(0x780000, 15), DATANUM(0x780000, 16), DATANUM(0x780000, 17),
    DATANUM(0x780000, 18), DATANUM(0x780000, 19), DATANUM(0x780000, 20),
    DATANUM(0x780000, 21), DATANUM(0x780000, 22), DATANUM(0x780000, 23),
    DATANUM(0x780000, 24), DATANUM(0x780000, 25), DATANUM(0x780000, 26),
    DATANUM(0x780000, 27), DATANUM(0x780000, 28), DATANUM(0x780000, 29),
    DATANUM(0x780000, 30), DATANUM(0x780000, 31), DATANUM(0x780000, 32),
    DATANUM(0x780000, 33), DATANUM(0x780000, 34)
};

#include "REL/m661dll.h"

typedef struct PointLightState {
    u8 unknown00[4]; /* No code names this field. */
    s16 handle;
    s16 modelId;
    s16 lightIndex;
    s16 modelLightId;
    s16 lightType;
    HuVecF position;
    u8 unk_1C[0x0C]; /* No code names this field. */
    HuVecF unusedVector; /* The game's code never reads this vector. */
    HuVecF direction;
    GXColor color;
    f32 referenceDistance;
    f32 referenceBrightness;
    f32 cutoff;
} PointLightState;

extern PointLightState lbl_1_bss_1870[];

#include "dolphin/os.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "string.h"
#include "humath.h"
#include "datadir_enum.h"
#include "messdir_enum.h"
#include "game/object.h"
#include "game/gamework.h"
#include "game/audio.h"
#include "game/sprite.h"
#include "game/main.h"
#include "game/pad.h"
#include "game/window.h"
#include "game/wipe.h"
#include "game/frand.h"
#include "game/charman.h"
#include "game/process.h"
#include "game/hsfex.h"
#include "game/gamemes.h"
#include "game/flag.h"
#include "game/armem.h"
#include "game/mgdata.h"
#include "game/printfunc.h"
#include "game/saveload.h"
#include "game/esprite.h"
#include "game/thpmain.h"
#include "game/mg/seqman.h"
#include "game/mg/actman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/board/object.h"
#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/tutorial.h"
#include "game/board/masu.h"
#include "game/board/player.h"

/* The unidentified byte intervals lie between fields used by the player code. */
struct PlayerSequenceState {
    s32 buttons;
    s32 buttonsDown;
    u16 buttonsRepeat;
    s8 stickX, stickY, subStickX, subStickY;
    u8 triggerL, triggerR, dPad, dPadRepeat;
    u8 unknown12[2]; /* No code names this field. */
    s8 groupSlot;
    s8 playerIndex;
    s8 character, controllerPort, group, playerType, computerDifficulty;
    s8 choiceMode, selectedCharacter;
    u8 unknown1D[0x13]; /* No code names this field. */
    s8 flagBeforePosition;
    Point3d position;
    u8 unknown40[0xC]; /* No code names this field. */
    Point3d rotation;
    s8 flagAfterRotation;
    u8 unknown59[0xB]; /* No code names this field. */
    s16 score;
    s16 unknownAfterScore;
    s32 padX, padY, inputStickX, inputStickY;
    s16 inputPhase;
    u8 unknown7A[4]; /* No code names this field. */
    s16 movementPhase;
    Point3d target;
    u8 unknown8C[0xC]; /* No code names this field. */
    f32 chaseSpeed;
    f32 chaseRadius;
    u8 unknownA0[4]; /* No code names this field. */
    s16 action;
    u8 unknownA6[0x10]; /* No code names this field. */
    s16 pathOffset;
    s16 pathVariant;
};
struct PlayerAnimationWork {
    s16 motionIds[5];
    s16 motionId, appliedMotionId;
    f32 motionSpeed, appliedMotionSpeed;
    f32 motionTime, appliedMotionTime, motionDuration;
    u32 modelAttributes, appliedModelAttributes, attributesToReset;
    s32 motionEnded;
    s16 repeatMotion, nextMotionId;
    f32 nextMotionTime;
    u32 nextModelAttributes, transitionState;
    s16 shiftState, shiftedMotionId;
    u32 shiftAttributes;
    f32 shiftTime, shiftDuration, shiftElapsed, shiftMaxTime;
    /* The array's 0x60-byte element stride leaves four unidentified bytes here. */
    u8 unknown5C[4]; /* No code names this field. */
};
s32 fn_1_20E50(struct PlayerSequenceState *, s16);
s32 fn_1_2EA4(f32 *, f32, f32, f32);
extern struct PlayerSequenceState lbl_1_bss_1C04[2];
extern struct PlayerAnimationWork lbl_1_bss_1D7C[2];

#include "REL/m661dll/routes.h"
#include "REL/m661dll/score_display.h"

extern void fn_1_68A8(MGACTOR *, int);
extern void fn_1_6928(MGACTOR *, int);
extern void fn_1_2B58(void);
extern MGTIMER *fn_1_2FC4(int, int, int, float, float);

s32 fn_1_AAB4(void);
void fn_1_71C(void);
void fn_1_2B58(void);
void fn_1_24E8(void);

s32 fn_1_88B8(void)
{
    MGACTOR_PARAM actorParam;
    s32 index;

    fn_1_3D0();
    fn_1_C8D4();
    fn_1_3D7C();
    fn_1_8E50();
    fn_1_949C();
    lbl_1_bss_1E3C[2].modelId = fn_80025A04(
        fn_80008DD0(lbl_1_data_100[0], 0x10000000, 2));
    fn_800269E8(lbl_1_bss_1E3C[2].modelId, 1);
    fn_80026B64(lbl_1_bss_1E3C[2].modelId, 1);
    fn_80026B8C(lbl_1_bss_1E3C[2].modelId, 0);
    lbl_1_bss_1E3C[2].position.x = 0.0f;
    lbl_1_bss_1E3C[2].position.y = 0.0f;
    lbl_1_bss_1E3C[2].position.z = 0.0f;
    lbl_1_bss_1E3C[2].rotation.x = 0.0f;
    lbl_1_bss_1E3C[2].rotation.y = 0.0f;
    lbl_1_bss_1E3C[2].rotation.z = 0.0f;
    lbl_1_bss_1960.modelId = lbl_1_bss_1E3C[2].modelId;
    fn_800804E8(&lbl_1_bss_1960.modelId, 1, 0xA);

    actorParam.height = 150.0f;
    actorParam.radius = 50.0f;
    actorParam.type = 0;
    actorParam.attr = 0x8000;
    actorParam.narrowHook = (COL_NARROW_HOOK)fn_1_68A8;
    actorParam.correctHookParam = 0;
    actorParam.correctHook = (MGACTOR_CORRECT_HOOK)fn_1_6928;
    index = 0;
    for (; index < 2; index++) {
        actorParam.param = index;
        lbl_1_bss_1960.players[index] =
            fn_80080728(lbl_1_bss_1C04[index].groupSlot,
                           &actorParam, 4, 1, -0x3F, NULL);
        lbl_1_bss_1E3C[index].modelId = (s16)lbl_1_bss_1960.players[index]->actor->mdlId;
        lbl_1_bss_1960.players[index]->actor->pos.x = lbl_1_bss_1C04[index].position.x;
        lbl_1_bss_1960.players[index]->actor->pos.y = lbl_1_bss_1C04[index].position.y;
        lbl_1_bss_1960.players[index]->actor->pos.z = lbl_1_bss_1C04[index].position.z;
        lbl_1_bss_1960.players[index]->actor->rotY = lbl_1_bss_1C04[index].rotation.y;
        fn_80026E44(lbl_1_bss_1E3C[index].modelId);
        fn_80026F9C((s16)lbl_1_bss_1960.players[index]->actor->mdlId);
        fn_80026B8C((s16)lbl_1_bss_1960.players[index]->actor->mdlId, 1);
        fn_8002D414(lbl_1_bss_1E3C[index].modelId, 20.0f);
        fn_8005F77C(lbl_1_bss_1C04[index].character, 0);
        lbl_1_bss_1C04[index].flagAfterRotation = 1;
        lbl_1_bss_1C04[index].flagBeforePosition = 1;
        lbl_1_bss_1C04[index].choiceMode = 1;
        fn_8005DFC0(lbl_1_bss_1C04[index].character, 1);
    }

    fn_8005DDEC(1);
    index = 0x23;
    for (; index < 0x71; index++) {
        fn_800273EC(lbl_1_bss_1E3C[index].modelId, 0.7f,
                        0.7f, 0.7f);
    }
    index = 0x62;
    for (; index <= 0x70; index++) {
        fn_800273EC(lbl_1_bss_1E3C[index].modelId, 0.7f,
                        0.7f, 0.7f);
    }
    fn_1_2B58();
    fn_1_EB8();
    fn_1_A6C0();
    fn_1_249A8();
    lbl_1_bss_196C.action = fn_1_2FC4(0, 0x14, 0, 288.0f,
                                          64.0f);
    lbl_1_bss_1C04[0].action = 2;
    lbl_1_bss_1C04[1].action = 2;
    return 0;
}

/* Runs one frame of the opening scene along with player input, actor and model
 * updates; returns 1 once the scene has finished. */
s16 fn_1_8DD4() {
    s16 result = 0;
    result = fn_1_AAB4();
    fn_1_71C();
    fn_1_2B58();
    fn_1_24E8();
    return result;
}

/* Runs one frame of the opening scene along with player input, actor and model
 * updates, always returning 0. */
s16 fn_1_8E14() {
    s16 result = 0;
    fn_1_AAB4();
    fn_1_71C();
    fn_1_2B58();
    fn_1_24E8();
    return result;
}

void fn_1_8E50(void)
{
    HuVecF shadowPositions[16] = { { -650.0f, 0.0f, -650.0f }, { -225.0f, 0.0f, -650.0f }, { 225.0f, 0.0f, -650.0f }, { 650.0f, 0.0f, -650.0f }, { -650.0f, 0.0f, -225.0f }, { -225.0f, 0.0f, -225.0f }, { 225.0f, 0.0f, -225.0f }, { 650.0f, 0.0f, -225.0f }, { -650.0f, 0.0f, 225.0f }, { -225.0f, 0.0f, 225.0f }, { 225.0f, 0.0f, 225.0f }, { 650.0f, 0.0f, 225.0f }, { -650.0f, 0.0f, 650.0f }, { -225.0f, 0.0f, 650.0f }, { 225.0f, 0.0f, 650.0f }, { 650.0f, 0.0f, 650.0f } };
    s16 modelIndex;
    s16 fileIndex;
    s16 shadowPositionIndex;

    modelIndex = 3;
    fileIndex = 1;
    shadowPositionIndex = 0;
    while (modelIndex <= 0x19) {
        lbl_1_bss_1E3C[modelIndex].modelId = fn_80025A04(fn_80008DD0(
            lbl_1_data_100[fileIndex], 0x10000000, 2));
        fn_800269E8(lbl_1_bss_1E3C[modelIndex].modelId, 1);
        fn_80026A44(lbl_1_bss_1E3C[modelIndex].modelId, 1);
        fn_80026B64(lbl_1_bss_1E3C[modelIndex].modelId, 1);
        fn_80026B8C(lbl_1_bss_1E3C[modelIndex].modelId, 0);
        if (modelIndex == 0x11 || modelIndex == 0x18) {
            fn_80026B8C(lbl_1_bss_1E3C[modelIndex].modelId, 1);
        }
        if (modelIndex == 0x12 || modelIndex == 0x13 || modelIndex == 0x14 ||
            modelIndex == 0x15) {
            fn_80026B8C(lbl_1_bss_1E3C[modelIndex].modelId, 2);
        }
        if (modelIndex == 3 || modelIndex == 0x18 || modelIndex == 0x19) {
            fn_80026FFC(lbl_1_bss_1E3C[modelIndex].modelId);
            lbl_1_bss_1E3C[modelIndex].shadowPosition =
                shadowPositions[shadowPositionIndex];
        }
        fn_8002D414(lbl_1_bss_1E3C[modelIndex].modelId, 0.0f);
        if (modelIndex == 0xE || modelIndex == 0xF || modelIndex == 0x11) {
            fn_800269E8(lbl_1_bss_1E3C[modelIndex].modelId, ((1 << 30) | (1 << 0)));
            fn_8002D86C(lbl_1_bss_1E3C[modelIndex].modelId, 1.0f);
            if (modelIndex == 0x11) {
                fn_8002D414(lbl_1_bss_1E3C[modelIndex].modelId, 1100.0f);
            }
        } else {
            fn_80026A44(lbl_1_bss_1E3C[modelIndex].modelId, ((1 << 30) | (1 << 0)));
            fn_8002D86C(lbl_1_bss_1E3C[modelIndex].modelId, 0.0f);
        }
        lbl_1_bss_1E3C[modelIndex].position.x = 0.0f;
        lbl_1_bss_1E3C[modelIndex].position.y = 0.0f;
        lbl_1_bss_1E3C[modelIndex].position.z = 0.0f;
        lbl_1_bss_1E3C[modelIndex].rotation.x = 0.0f;
        lbl_1_bss_1E3C[modelIndex].rotation.y = 0.0f;
        lbl_1_bss_1E3C[modelIndex].rotation.z = 0.0f;
        modelIndex++;
        fileIndex++;
        shadowPositionIndex++;
    }

    modelIndex = 0x1A;
    fileIndex = 0x18;
    while (modelIndex <= 0x22) {
        lbl_1_bss_1E3C[modelIndex].modelId = fn_80025A04(fn_80008DD0(
            lbl_1_data_100[fileIndex], 0x10000000, 2));
        fn_800269E8(lbl_1_bss_1E3C[modelIndex].modelId, 1);
        fn_80026B64(lbl_1_bss_1E3C[modelIndex].modelId, 1);
        fn_80026B8C(lbl_1_bss_1E3C[modelIndex].modelId, 0);
        fn_8002D414(lbl_1_bss_1E3C[modelIndex].modelId, 0.0f);
        fn_80026A44(lbl_1_bss_1E3C[modelIndex].modelId, ((1 << 30) | (1 << 0)));
        fn_8002D86C(lbl_1_bss_1E3C[modelIndex].modelId, 0.0f);
        lbl_1_bss_1E3C[modelIndex].position.x = 0.0f;
        lbl_1_bss_1E3C[modelIndex].position.y = 0.0f;
        lbl_1_bss_1E3C[modelIndex].position.z = 0.0f;
        lbl_1_bss_1E3C[modelIndex].rotation.x = 0.0f;
        lbl_1_bss_1E3C[modelIndex].rotation.y = 0.0f;
        lbl_1_bss_1E3C[modelIndex].rotation.z = 0.0f;
        modelIndex++;
        fileIndex++;
    }
}

void fn_1_949C(void)
{
    Point3d normalDirections[2] = { { 1.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 1.0f } };
    s16 modelIndex;
    s16 motionIndex;
    s16 scanIndex;
    s16 pathIndex;
    s16 retryCount;
    s16 blocked;
    f32 distance;
    f32 motionTime;
    Point3d candidatePosition;
    Point3d delta;
    Point3d side;
    Point3d nearest;
    Point3d start;
    f32 lengthSquared;
    f32 projectionNumerator;
    f32 alongEdge;
    BoundaryEdge boundaryData[6];
    f32 collisionRadius;

    boundaryData[0].x0 = -600.0f;
    boundaryData[0].x1 = -800.0f;
    boundaryData[0].z0 = -800.0f;
    boundaryData[0].z1 = -600.0f;
    boundaryData[1].x0 = 600.0f;
    boundaryData[1].x1 = 800.0f;
    boundaryData[1].z0 = -800.0f;
    boundaryData[1].z1 = -600.0f;
    boundaryData[2].x0 = -1160.0f;
    boundaryData[2].x1 = 860.0f;
    boundaryData[3].x0 = 1160.0f;
    boundaryData[3].x1 = 860.0f;
    collisionRadius = 500.0f;
    collisionRadius *= collisionRadius;

    {
    for (modelIndex = 0x62, motionIndex = 0x3F;
         modelIndex <= 0x70;
         modelIndex++, motionIndex++) {
        lbl_1_bss_1E3C[modelIndex].modelId = fn_80025A04(
            fn_80008DD0(lbl_1_data_100[33], 0x10000000, 2));
        fn_800269E8(lbl_1_bss_1E3C[modelIndex].modelId, 1);
        fn_80026B64(lbl_1_bss_1E3C[modelIndex].modelId, 1);
        fn_80026B8C(lbl_1_bss_1E3C[modelIndex].modelId, 0);
        fn_800268B0(lbl_1_bss_1E3C[modelIndex].modelId, 0.6f, 0.6f, 0.6f);
        fn_80026E44(lbl_1_bss_1E3C[modelIndex].modelId);
        fn_80026F9C(lbl_1_bss_1E3C[modelIndex].modelId);
        lbl_1_bss_10[motionIndex].motionIds[0] = fn_80030458(
            lbl_1_bss_1E3C[modelIndex].modelId,
            fn_80008DD0(DATANUM(0x780000, 35), 0x10000000, 2));
        lbl_1_bss_10[motionIndex].motionIds[1] = fn_80030458(
            lbl_1_bss_1E3C[modelIndex].modelId,
            fn_80008DD0(DATANUM(0x780000, 36), 0x10000000, 2));
        fn_8002C95C(lbl_1_bss_1E3C[modelIndex].modelId, lbl_1_bss_10[motionIndex].motionIds[0]);
        fn_800269E8(lbl_1_bss_1E3C[modelIndex].modelId, ((1 << 30) | (1 << 0)));
        fn_8002D414(lbl_1_bss_1E3C[modelIndex].modelId, 0.0f);
        fn_8002D86C(lbl_1_bss_1E3C[modelIndex].modelId, 1.0f);
        lbl_1_bss_1E3C[modelIndex].position.x = -290.8f;
        lbl_1_bss_1E3C[modelIndex].position.y = 50.0f;
        lbl_1_bss_1E3C[modelIndex].position.z = 670.3f;
        lbl_1_bss_1E3C[modelIndex].rotation.x = 0.0f;
        lbl_1_bss_1E3C[modelIndex].rotation.y = 0.0f;
        lbl_1_bss_1E3C[modelIndex].rotation.z = 0.0f;
        lbl_1_bss_10[motionIndex].active = 0;
        lbl_1_bss_10[motionIndex].animationState =
            lbl_1_bss_10[motionIndex].previousAnimationState = 0;
        lbl_1_bss_10[motionIndex].inputDirection[0] = -1;
        lbl_1_bss_10[motionIndex].inputDirection[1] = -1;
        lbl_1_bss_10[motionIndex].inputLimit[0] = -1;
        lbl_1_bss_10[motionIndex].inputLimit[1] = -1;
    }
    }

    lbl_1_bss_10[0x3F].active = 1;
    fn_80026A44(lbl_1_bss_1E3C[0x62].modelId, 1);

    {
    for (modelIndex = 0x23, motionIndex = 0, pathIndex = 0;
         modelIndex < 0x62;
         modelIndex++, motionIndex++, pathIndex++) {

        lbl_1_bss_1E3C[modelIndex].modelId = fn_80025A04(
            fn_80008DD0(lbl_1_data_100[34],
                                 0x10000000, 2));
        fn_800269E8(lbl_1_bss_1E3C[modelIndex].modelId, 1);
        fn_80026A44(lbl_1_bss_1E3C[modelIndex].modelId, 1);
        fn_80026B64(lbl_1_bss_1E3C[modelIndex].modelId, 1);
        fn_80026B8C(lbl_1_bss_1E3C[modelIndex].modelId, 1);
        lbl_1_bss_10[motionIndex].motionIds[0] = fn_80030458(
            lbl_1_bss_1E3C[modelIndex].modelId,
            fn_80008DD0(DATANUM(0x780000, 37), 0x10000000, 2));
        lbl_1_bss_10[motionIndex].motionIds[1] = fn_80030458(
            lbl_1_bss_1E3C[modelIndex].modelId,
            fn_80008DD0(DATANUM(0x780000, 38), 0x10000000, 2));
        fn_8002C95C(lbl_1_bss_1E3C[modelIndex].modelId, lbl_1_bss_10[motionIndex].motionIds[0]);
        fn_800269E8(lbl_1_bss_1E3C[modelIndex].modelId, ((1 << 30) | (1 << 0)));
        fn_8002D86C(lbl_1_bss_1E3C[modelIndex].modelId, 1.0f);
        fn_80026E44(lbl_1_bss_1E3C[modelIndex].modelId);
        fn_80026F9C(lbl_1_bss_1E3C[modelIndex].modelId);
        motionTime = fn_1_3D04(100.0f);
        fn_8002D414(lbl_1_bss_1E3C[modelIndex].modelId, motionTime);
        fn_80026CBC(lbl_1_bss_1E3C[modelIndex].modelId, 0.0f);
        lbl_1_bss_1E3C[modelIndex].position.x = -1000.0f;

        retryCount = 0;
        start.x = lbl_1_bss_196C.initialPositions[pathIndex / 4].x;
        start.z = lbl_1_bss_196C.initialPositions[pathIndex / 4].z;
        do {
            candidatePosition.x = start.x + fn_1_3D04(lbl_1_bss_196C.initialGridStep.x);
            candidatePosition.y = -50.0f;
            candidatePosition.z = start.z + fn_1_3D04(lbl_1_bss_196C.initialGridStep.z);
            for (scanIndex = 0x23, blocked = 0; scanIndex < 0x71; scanIndex++) {
                delta.x = candidatePosition.x - lbl_1_bss_1E3C[scanIndex].position.x;
                delta.z = candidatePosition.z - lbl_1_bss_1E3C[scanIndex].position.z;
                distance = (f32)sqrt((f64)(delta.x * delta.x + delta.z * delta.z));
                if (distance < 60.0f) {
                    blocked = 1;
                    break;
                }
            }
            if (blocked == 0) {
                for (scanIndex = 0; scanIndex < 2; scanIndex++) {
                    delta.x = candidatePosition.x - lbl_1_bss_1C04[scanIndex].position.x;
                    delta.z = candidatePosition.z - lbl_1_bss_1C04[scanIndex].position.z;
                    distance = (f32)sqrt((f64)(delta.x * delta.x + delta.z * delta.z));
                    if (distance < 310.0f) {
                        blocked = 1;
                        break;
                    }
                }
            }
            if (blocked == 0) {
                for (scanIndex = 0; scanIndex < 2; scanIndex++) {
                    delta.x = boundaryData[scanIndex].x0 - boundaryData[scanIndex].x1;
                    delta.z = boundaryData[scanIndex].z0 - boundaryData[scanIndex].z1;
                    lengthSquared = delta.x * delta.x + delta.z * delta.z;
                    projectionNumerator = -1.0f * (delta.x * (boundaryData[scanIndex].x0 - candidatePosition.x)
                        + delta.z * (boundaryData[scanIndex].z0 - candidatePosition.z));
                    alongEdge = lengthSquared ? projectionNumerator / lengthSquared : 0.0f;
                    nearest.x = delta.x * alongEdge + boundaryData[scanIndex].x0;
                    nearest.z = delta.z * alongEdge + boundaryData[scanIndex].z0;
                    delta.x = candidatePosition.x - nearest.x;
                    delta.z = candidatePosition.z - nearest.z;
                    side.x = delta.x != 0.0f ? (delta.x > 0.0f ? 1.0f : -1.0f) : normalDirections[scanIndex].x;
                    side.z = delta.z != 0.0f ? (delta.z > 0.0f ? 1.0f : -1.0f) : normalDirections[scanIndex].z;
                    if ((side.x != normalDirections[scanIndex].x) ||
                        (side.z != normalDirections[scanIndex].z)) {
                        blocked = 1;
                        break;
                    }
                }
            }
            if (blocked == 0) {
                for (scanIndex = 2; scanIndex < 4; scanIndex++) {
                    delta.x = candidatePosition.x - boundaryData[scanIndex].x0;
                    delta.z = candidatePosition.z - boundaryData[scanIndex].x1;
                    distance = delta.x * delta.x + delta.z * delta.z;
                    if (distance < collisionRadius) {
                        blocked = 1;
                        break;
                    }
                }
            }
        } while (blocked != 0 && retryCount++ <= 0x3E8);

        lbl_1_bss_1E3C[modelIndex].position = candidatePosition;
        lbl_1_bss_1E3C[0x23].position.x = 400.0f;
        lbl_1_bss_1E3C[0x23].position.z = -10.0f;
        lbl_1_bss_1E3C[0x61].position.x = -400.0f;
        lbl_1_bss_1E3C[0x61].position.z = -400.0f;
        lbl_1_bss_1E3C[modelIndex].rotation.x = 0.0f;
        lbl_1_bss_1E3C[modelIndex].rotation.y = fn_1_3D04(360.0f);
        lbl_1_bss_1E3C[modelIndex].rotation.z = 0.0f;
        lbl_1_bss_10[motionIndex].active = 1;
        lbl_1_bss_10[motionIndex].animationState =
            lbl_1_bss_10[motionIndex].previousAnimationState = 0;
        lbl_1_bss_10[motionIndex].inputDirection[0] = -1;
        lbl_1_bss_10[motionIndex].inputDirection[1] = -1;
        lbl_1_bss_10[motionIndex].inputLimit[0] = -1;
        lbl_1_bss_10[motionIndex].inputLimit[1] = -1;
    }
    }
}

void fn_1_A6C0(void)
{
    HuVecF lightPositions[3] = { { 0.0f, 700.0f, 0.0f }, { -1200.0f, 500.0f, 0.0f }, { 1200.0f, 500.0f, 0.0f } };
    GXColor lightColors[3] = { { 153, 153, 128, 0 }, { 128, 128, 102, 0 }, { 128, 128, 102, 0 } };
    s16 targetModelIndices[3] = { 3, 22, 23 };
    s16 lightTypes[3] = { 1, 2, 2 };
    s16 lightIndex;
    s16 positionIndex;
    s16 modelIndex;

    for (lightIndex = 0, positionIndex = 0; lightIndex < 3; lightIndex++, positionIndex++) {
        PointLightState *light;
        HU3D_MODEL *model;

        if (lightIndex != 0) {
            break;
        }
        light = &lbl_1_bss_1870[lightIndex];
            light->modelId = lbl_1_bss_1E3C[targetModelIndices[positionIndex]].modelId;
            light->lightIndex = lightIndex;
            light->position = lightPositions[positionIndex];
            light->unusedVector.x = 0.0f;
            light->unusedVector.y = 0.0f;
            light->unusedVector.z = 0.0f;
            light->direction.x = 0.0f;
            light->direction.y = -1.0f;
            light->direction.z = 0.0f;
            light->color = lightColors[positionIndex];
            light->referenceDistance = light->position.y;
            light->referenceBrightness = 1.0f;
            light->cutoff = 90.0f;
            light->lightType = lightTypes[positionIndex];

            lbl_1_bss_1870[0].referenceBrightness = 1.0f;
            lbl_1_bss_1870[1].referenceBrightness = 0.9f;
            lbl_1_bss_1870[2].referenceBrightness = 0.9f;
            lbl_1_bss_1870[0].cutoff = 40.0f;
            lbl_1_bss_1870[1].cutoff = 90.0f;
            lbl_1_bss_1870[2].cutoff = 90.0f;

            model = &lbl_802C01C8[light->modelId];
            light->handle = fn_800288B4(light->modelId, &light->position,
                                               &light->direction, &light->color);
            light->modelLightId = model->LLightId[light->handle];
            fn_800296E0(light->modelId, light->handle, TRUE);

            switch (light->lightType) {
            case 0:
                break;
            case 1:
                fn_80028C1C(light->modelId, light->handle,
                                   light->referenceDistance, light->referenceBrightness,
                                   GX_DA_STEEP);
                break;
            case 2:
                fn_80028AA8(light->modelId, light->handle, light->cutoff, GX_SP_COS);
                break;
            }

            for (modelIndex = 0; modelIndex < 0x71; modelIndex++) {
                fn_1_3B54(lbl_1_bss_1E3C[modelIndex].modelId,
                          light->modelLightId);
            }
    }
}

/* Steps the opening scene: the first player chases model 98 while the camera follows,
 * the model turns and runs off, and the camera eases to its play view. Both players
 * then change motion while the pieces fade in and rise, and 1 is returned once the
 * scene is over; each frame also spawns a particle at two fixed corner points. */
s32 fn_1_AAB4(void) {
    struct PlayerSequenceState *player;
    s32 finished = 0;
    s32 totalFrames = 0;
    f32 cameraMoveFrames;
    ModelSetupRecord *model;
    HuVecF spawnPositions[2] = {{-788.0f, 60.0f, -788.0f}, {788.0f, 60.0f, -788.0f}};

    player = &lbl_1_bss_1C04[0];
    model = &lbl_1_bss_1E3C[0x62];
        switch (lbl_1_bss_196C.sequenceState) {
        case 0:
        {
            player->position.x = 700.0f;
            player->position.z = -700.0f;
            fn_80086AA8(lbl_1_bss_1960.players[0]->actor, &player->position);
            model->position.x = 500.0f;
            model->position.z = -700.0f;
            fn_1_4EC4();
            if (lbl_1_bss_196C.sequenceTimer++ > 0x3C) {
                fn_8002C95C(model->modelId, lbl_1_bss_10[0x3F].motionIds[1]);
                fn_800269E8(model->modelId, ((1 << 30) | (1 << 0)));
                fn_8002D414(model->modelId, 0.0f);
                fn_8002D86C(model->modelId, 2.0f);
                fn_1_5020(0, 5, 0);
    lbl_1_bss_196C.sequenceTimer = 0;
                lbl_1_bss_196C.heading = -90.0f;
                lbl_1_bss_196C.sequenceState += 1;
            }
            break;
        }
        case 1:
        {
            Point3d cameraDifference;
            Point3d cameraMidpoint;
            f32 cameraAngle;
            model->position.x = (f32) ((f64) model->position.x + (10.0 * sin((M_PI * (f64) lbl_1_bss_196C.heading) / 180.0)));
            model->position.z = (f32) ((f64) model->position.z + (10.0 * cos((M_PI * (f64) lbl_1_bss_196C.heading) / 180.0)));
            model->rotation.y = lbl_1_bss_196C.heading;
            player->target.x = model->position.x;
            player->target.z = model->position.z;
            player->chaseSpeed = 10.0f;
            player->chaseRadius = 10.1f;
            totalFrames = fn_1_20E50(player, 1);
            if ((s16)totalFrames != 0) {
                lbl_1_bss_196C.sequenceState += 1;
            }
            cameraMidpoint.x = ((player->position.x - model->position.x) > 0.0f) ? player->position.x - model->position.x : -1.0f * (player->position.x - model->position.x);
            cameraMidpoint.z = ((player->position.z - model->position.z) > 0.0f) ? player->position.z - model->position.z : -1.0f * (player->position.z - model->position.z);
            cameraMidpoint.x = (player->position.x - (cameraMidpoint.x / 2.0f)) - 10.0f;
            cameraMidpoint.z = player->position.z - (cameraMidpoint.z / 2.0f);
            if ((cameraMidpoint.x <= 0.0f) && (cameraMidpoint.x >= -250.0f)) {
                lbl_80266A4C.x = cameraMidpoint.x;
            }
            if (cameraMidpoint.x <= 0.0f) {
                lbl_80266A4C.z = 700.0f + (38.0f + cameraMidpoint.z);
                cameraDifference.x = lbl_80266A4C.x - cameraMidpoint.x;
                cameraDifference.z = lbl_80266A4C.z - cameraMidpoint.z;
                cameraAngle = (f32) (180.0 *
                    (atan2((f64) cameraDifference.x,
                           (f64) cameraDifference.z) / M_PI));
                lbl_80266A40.y = cameraAngle;
            }
            if (player->position.x <= -380.0f) {
                fn_1_2EA4(&lbl_1_bss_196C.heading,
                          20.0f, -90.0f, 0.0f);
    }
            if (player->position.z >= 0.0f) {
                fn_8007E888(lbl_1_bss_1960.players[player->playerIndex]);
                player->movementPhase = 0;
                player->position.z = 0.0f;
                fn_80086AA8(lbl_1_bss_1960.players[0]->actor, &player->position);
                lbl_1_bss_196C.modelSpeed = 15.0f;
                lbl_1_bss_196C.sequenceState += 1;
            }
            break;
        }
        case 2:
        {
            model->position.x = (f32) ((f64) model->position.x + ((f64) lbl_1_bss_196C.modelSpeed * sin((M_PI * (f64) model->rotation.y) / 180.0)));
            model->position.z = (f32) ((f64) model->position.z + ((f64) lbl_1_bss_196C.modelSpeed * cos((M_PI * (f64) model->rotation.y) / 180.0)));
            if ((model->rotation.y += 4.0f) >= 45.0f) {
                model->rotation.y = 45.0f;
                lbl_1_bss_196C.savedCameraCenter = lbl_80266A4C;
                lbl_1_bss_196C.savedCameraRotation = lbl_80266A40;
                lbl_1_bss_196C.savedCameraZoom = (f32) lbl_802C028C;
                lbl_1_bss_196C.sequenceState += 1;
            }
            break;
        }
        case 3:
        {
            model->position.x = (f32) ((f64) model->position.x + ((f64) lbl_1_bss_196C.modelSpeed * sin((M_PI * (f64) model->rotation.y) / 180.0)));
            model->position.z = (f32) ((f64) model->position.z + ((f64) lbl_1_bss_196C.modelSpeed * cos((M_PI * (f64) model->rotation.y) / 180.0)));
            if (model->position.z > 500.0f) {
                fn_8002C95C(model->modelId, lbl_1_bss_10[0x3F].motionIds[0]);
                fn_800269E8(model->modelId, ((1 << 30) | (1 << 0)));
                fn_8002D414(model->modelId, 0.0f);
                fn_8002D86C(model->modelId, 1.0f);
                lbl_1_bss_1D7C->shiftState = 1;
                lbl_1_bss_1D7C->shiftedMotionId = 1;
                lbl_1_bss_1D7C->shiftTime = 0.0f;
                lbl_1_bss_1D7C->shiftDuration = 5.0f;
                lbl_1_bss_1D7C->shiftAttributes = ((1 << 30) | (1 << 0));
                lbl_1_bss_1D7C->attributesToReset = 0;
                lbl_1_bss_196C.sequenceState += 1;
            }
            break;
        }
        case 4:
        {
            model->position.x = (f32) ((f64) model->position.x + ((f64) lbl_1_bss_196C.modelSpeed * sin((M_PI * (f64) model->rotation.y) / 180.0)));
            model->position.z = (f32) ((f64) model->position.z + ((f64) lbl_1_bss_196C.modelSpeed * cos((M_PI * (f64) model->rotation.y) / 180.0)));
            if ((lbl_1_bss_196C.modelSpeed -= 0.5f) <= 0.0f) {
                lbl_1_bss_196C.modelSpeed = 0.0f;
            }
            cameraMoveFrames = 30.0f;
            totalFrames = fn_1_2EA4(&lbl_1_bss_1960.players[0]->actor->rotY,
                                cameraMoveFrames, 0.0f,
                                90.0f);
            if (lbl_1_bss_196C.sequenceTimer == 0 && (s16)totalFrames == 0) {
                lbl_1_bss_1D7C->shiftState = 1;
                lbl_1_bss_1D7C->shiftedMotionId = 0;
                lbl_1_bss_1D7C->shiftTime = 0.0f;
                lbl_1_bss_1D7C->shiftDuration = 10.0f;
                lbl_1_bss_1D7C->shiftAttributes = ((1 << 30) | (1 << 0));
                lbl_1_bss_1D7C->attributesToReset = 0;
                lbl_1_bss_196C.sequenceTimer = 1;
            }
            cameraMoveFrames = 60.0f;
            totalFrames = fn_1_2EA4(&lbl_80266A4C.x, cameraMoveFrames,
                                    lbl_1_bss_196C.savedCameraCenter.x,
                                    0.0f);
            totalFrames += fn_1_2EA4(&lbl_80266A4C.y, cameraMoveFrames,
                                     lbl_1_bss_196C.savedCameraCenter.y,
                                     196.0f);
            totalFrames += fn_1_2EA4(&lbl_80266A4C.z, cameraMoveFrames,
                                     lbl_1_bss_196C.savedCameraCenter.z,
                                     38.0f);
            totalFrames += fn_1_2EA4(&lbl_80266A40.x, cameraMoveFrames,
                                     lbl_1_bss_196C.savedCameraRotation.x,
                                     -47.9f);
            totalFrames += fn_1_2EA4(&lbl_80266A40.y, cameraMoveFrames,
                                     lbl_1_bss_196C.savedCameraRotation.y,
                                     0.0f);
            totalFrames += fn_1_2EA4(&lbl_80266A40.z, cameraMoveFrames,
                                     lbl_1_bss_196C.savedCameraRotation.z,
                                     0.0f);
            totalFrames += fn_1_2EA4(&lbl_802C028C, cameraMoveFrames,
                                     lbl_1_bss_196C.savedCameraZoom,
                                     4589.3f);
            if ((s16)totalFrames == 0) {
                lbl_1_bss_196C.sequenceTimer = 0;
                lbl_1_bss_196C.sequenceState += 1;
            }
            break;
        }
        case 5:
        {
            if (lbl_1_bss_196C.sequenceTimer++ > 0x1E) {
                lbl_1_bss_1D7C->shiftState = 1;
                lbl_1_bss_1D7C->shiftedMotionId = 4;
                lbl_1_bss_1D7C->shiftTime = 0.0f;
                lbl_1_bss_1D7C->shiftDuration = 10.0f;
                lbl_1_bss_1D7C->shiftAttributes = 0;
                lbl_1_bss_1D7C->attributesToReset = ((1 << 30) | (1 << 0));
                lbl_1_bss_1D7C[1].shiftState = 1;
                lbl_1_bss_1D7C[1].shiftedMotionId = 4;
                lbl_1_bss_1D7C[1].shiftTime = 0.0f;
                lbl_1_bss_1D7C[1].shiftDuration = 10.0f;
                lbl_1_bss_1D7C[1].shiftAttributes = 0;
                lbl_1_bss_1D7C[1].attributesToReset = ((1 << 30) | (1 << 0));
                fn_1_5020(1, 5, 0);
                lbl_1_bss_196C.sequenceTimer = 0;
                lbl_1_bss_196C.sequenceState += 1;
            }
            break;
        }
        case 6:
        {
            s16 modelIndex;

            if ((lbl_1_bss_196C.transparency += 0.025f) >= 1.0f) {
                lbl_1_bss_196C.transparency = 1.0f;
                lbl_1_bss_196C.sequenceState += 1;
            }
            cameraMoveFrames = 35.0f;
            for (modelIndex = 0x23; modelIndex <= 0x61; modelIndex++) {
                model = &lbl_1_bss_1E3C[modelIndex];
                fn_80026CBC(model->modelId, lbl_1_bss_196C.transparency);
                fn_1_2EA4(&model->position.y, cameraMoveFrames,
                          -50.0f, 50.0f);
            }
            break;
        }
        case 7:
        {
            if (lbl_1_bss_1D7C->motionTime >= 43.9f) {
                lbl_1_bss_1D7C->appliedMotionId = lbl_1_bss_1D7C->appliedModelAttributes = -1U;
                lbl_1_bss_1D7C[1].appliedMotionId = lbl_1_bss_1D7C[1].appliedModelAttributes = -1U;
                lbl_1_bss_1D7C->motionId = 0;
                lbl_1_bss_1D7C->motionSpeed = 1.0f;
                lbl_1_bss_1D7C->motionTime = 0.0f;
                lbl_1_bss_1D7C->modelAttributes = ((1 << 30) | (1 << 0));
                lbl_1_bss_1D7C->attributesToReset = 0;
                lbl_1_bss_1D7C[1].motionId = 0;
                lbl_1_bss_1D7C[1].motionSpeed = 1.0f;
                lbl_1_bss_1D7C[1].motionTime = 0.0f;
                lbl_1_bss_1D7C[1].modelAttributes = ((1 << 30) | (1 << 0));
                lbl_1_bss_1D7C[1].attributesToReset = 0;
                lbl_1_bss_196C.sequenceState += 1;
            }
            break;
        }
        case 8:
        {
            finished = 1;
            break;
        }
        }
    fn_1_400C(&spawnPositions[0]);
    fn_1_400C(&spawnPositions[1]);
    return finished;
}
