/* Runs player motion, drawing collisions, controller input, and CPU movement. */
#include "REL/m613Dll/types.h"
#include "game/main.h"
#include "game/pad.h"
#include "dolphin/math.h"
#include "dolphin/gx.h"
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
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "dolphin/mtx.h"
#include "game/hu3d.h"
#include "game/flag.h"
#include "dolphin/types.h"

#define MSM_SE_MOWTOWN_INTRO_PLAYER_0 1703
#define MSM_SE_MOWTOWN_INTRO_PLAYER_1 1704
#define MSM_SE_MOWTOWN_INTRO_PLAYER_2 1705
#define MSM_SE_MOWTOWN_INTRO_PLAYER_3 1706
#define MSM_SE_MOWTOWN_MOVE_PLAYER_0 1707
#define MSM_SE_MOWTOWN_MOVE_PLAYER_1 1708
#define MSM_SE_MOWTOWN_MOVE_PLAYER_2 1709
#define MSM_SE_MOWTOWN_MOVE_PLAYER_3 1710
#define MSM_SE_MOWTOWN_PLAYER_CONTACT 1712
#define MSM_SE_MOWTOWN_ARENA_CONTACT 1713

extern s32 lbl_1_bss_8;
extern s16 lbl_1_bss_1210[4];
extern s16 fn_1_1D7C(void);
extern s16 fn_1_1D84(void);
extern s16 fn_1_1EF4(s16, s16);
extern void fn_1_1F54(s16, s16, Point3d *);
extern s32 fn_1_10C5C(Point3d *, Point3d *);
extern f64 fn_1_10E3C(f64, f64);

s32 fn_1_10C5C(Point3d *, Point3d *);

extern s32 fn_1_20D0(const Point3d *, const Point3d *, const Point3d *, const Point3d *);
extern void fn_1_F6E0(s16, Point3d *);
extern void fn_1_1054C(const Point3d *, const Point3d *, s16, s32);

extern s16 lbl_1_bss_125C;
M613PlayerState lbl_1_bss_11C[4];
s32 lbl_1_bss_10C[4];
s32 lbl_1_bss_FC[4];
s16 lbl_1_bss_F8;
extern s16 fn_1_FA48(void);
extern void fn_1_F2A4(void);
extern void fn_1_FC40(void);
extern void fn_1_1A7C(void);
extern void fn_1_106C0(void);

void fn_1_3C80(s16 playerIndex, s16 mode);
void fn_1_3EB0(s16 playerIndex);
void fn_1_3FFC(f32 angle_x, f32 angle_y, f32 angle_z,
               const Vec source, Vec *destination);
void fn_1_40BC(Point3d *position, Point3d *firstTarget, Point3d *unusedThirdTarget,
               Point3d *secondTarget, Point3d *result);
void fn_1_4208(void);
s32 fn_1_4D38(s16 frame);
void fn_1_5038(s16 mode, s16 frameNo);
void fn_1_5058(void);
void fn_1_5178(void);
s32 fn_1_96E8(Point3d *movingPosition, Point3d *displacement, f32 radius);
f32 fn_1_99A4(Point3d movingPosition, Point3d displacement, f32 radius, u8 side);
void fn_1_9C7C(s16 playerIndex);
void fn_1_B4A8(void);
void fn_1_C2AC(void);
void fn_1_C4E4(s16 awardMask);
s32 fn_1_C850(s16 frameNo);
void fn_1_D0C8(void);
void fn_1_D61C(void);
void fn_1_DAE8(void);
void fn_1_DBDC(s16 playerIndex);
void fn_1_DCF4(s16 playerIndex);
void fn_1_DD54(void);
void fn_1_DE20(s16 playerIndex);

u32 lbl_1_data_78[5] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 55),
    DATANUM(DATA_mariomot, 33), DATANUM(DATA_mariomot, 39),
    DATANUM(DATA_mariomot, 40)
};
Point3d lbl_1_data_8C[4] = { { -600.0f, 0.0f, -449.0f },
                             { 600.0f, 0.0f, -449.0f },
                             { -600.0f, 0.0f, 449.0f },
                             { 600.0f, 0.0f, 449.0f } };
Point3d lbl_1_data_BC[4] = { { -300.0f, -16.0f, -100.0f },
                             { -100.0f, -16.0f, -100.0f },
                             { 100.0f, -16.0f, -100.0f },
                             { 300.0f, -16.0f, -100.0f } };
f32 lbl_1_data_EC[4] = {90.0f,270.0f,90.0f,270.0f};
M613MotionEntry lbl_1_data_FC[6] = {
 {0,8.0f,HU3D_MOTATTR_LOOP}, {1,8.0f,HU3D_MOTATTR_LOOP}, {2,8.0f,HU3D_MOTATTR_LOOP},
 {2,8.0f,(HU3D_MOTATTR_LOOP | HU3D_MOTATTR_REV)}, {3,8.0f,HU3D_MOTATTR_LOOP}, {4,8.0f,HU3D_MOTATTR_LOOP}
};

/* Builds the three-axis rotation used by the player's movement and collision helpers. */
static inline void m613RotateWithMatrices(f32 angleX, f32 angleY, f32 angleZ,
                                         const Vec *source, Vec *destination,
                                         Mtx xRotation, Mtx yRotation) {
    PSMTXRotRad(xRotation, 'x', 0.017453292f * angleX);
    PSMTXRotRad(yRotation, 'y', 0.017453292f * angleY);
    PSMTXConcat(yRotation, xRotation, xRotation);
    PSMTXRotRad(yRotation, 'z', 0.017453292f * angleZ);
    PSMTXConcat(yRotation, xRotation, xRotation);
    PSMTXMultVecSR(xRotation, source, destination);
}

/* Rotates one vector for the player model and its collision points. */
static inline void m613RotateVector(f32 angleX, f32 angleY, f32 angleZ,
                                    const Vec *source, Vec *destination) {
    Mtx xRotation;
    Mtx yRotation;
    m613RotateWithMatrices(angleX, angleY, angleZ, source, destination, xRotation, yRotation);
}

/* Selects a player's idle, forward, reverse, win, or lose motion when its state changes. */
void fn_1_3C80(s16 playerIndex, s16 mode) {
    s16 motionIndex;

    if ((playerIndex < 0) || (playerIndex >= 4) || (mode < 0) || (mode >= 6)) {
        return;
    }
    if (mode != lbl_1_bss_11C[playerIndex].motionState) {
        motionIndex = lbl_1_data_FC[mode].motionIndex;
        if (motionIndex != -1) {
            if (lbl_1_data_FC[mode].duration <= 1.0f) {
                CharMotionSet(lbl_1_bss_11C[playerIndex].characterNo,
                              lbl_1_bss_11C[playerIndex].motionIds[motionIndex]);
                Hu3DModelAttrReset(lbl_1_bss_11C[playerIndex].model,
                                  Hu3DModelMotionAttrGet(lbl_1_bss_11C[playerIndex].model));
                CharModelAttrSet(lbl_1_bss_11C[playerIndex].characterNo,
                                 lbl_1_data_FC[mode].attributes);
            } else {
                CharMotionShiftSet(lbl_1_bss_11C[playerIndex].characterNo,
                                   lbl_1_bss_11C[playerIndex].motionIds[motionIndex],
                                   0.0f, lbl_1_data_FC[mode].duration,
                                   lbl_1_data_FC[mode].attributes);
            }
        }
        lbl_1_bss_11C[playerIndex].motionState = mode;
    }
}

/* Positions a character model at its rotated item-hook offset during the player update. */
void fn_1_3EB0(s16 playerIndex)
{
    Mtx rotationMatrix;
    Mtx combinedMatrix;
    Point3d rotatedPosition;

    PSMTXRotRad(rotationMatrix, 'x', 0.017453292f * lbl_1_bss_11C[playerIndex].rotation.x);
    PSMTXRotRad(combinedMatrix, 'y', 0.017453292f * lbl_1_bss_11C[playerIndex].rotation.y);
    PSMTXConcat(combinedMatrix, rotationMatrix, rotationMatrix);
    PSMTXRotRad(combinedMatrix, 'z', 0.017453292f * lbl_1_bss_11C[playerIndex].rotation.z);
    PSMTXConcat(combinedMatrix, rotationMatrix, rotationMatrix);
    PSMTXMultVecSR(rotationMatrix, &lbl_1_bss_11C[playerIndex].hookPosition, &rotatedPosition);
    PSVECAdd(&lbl_1_bss_11C[playerIndex].position, &rotatedPosition, &rotatedPosition);
    Hu3DModelPosSetV(lbl_1_bss_11C[playerIndex].model, &rotatedPosition);
}

/* Applies the requested Euler rotation to a vector for movement and collision calculations. */
void fn_1_3FFC(f32 angle_x, f32 angle_y, f32 angle_z,
               const Vec source, Vec *destination)
{
    Mtx xRotation;
    Mtx yRotation;
    m613RotateWithMatrices(angle_x, angle_y, angle_z, &source, destination, xRotation, yRotation);
}

/* Samples the effect quad from its first, second, and fourth corners; the third is ignored.
 * The first corner argument is modified in place by the first sampled edge displacement. */
void fn_1_40BC(Point3d *position, Point3d *firstTarget, Point3d *unusedThirdTarget,
               Point3d *secondTarget, Point3d *result)
{
    Point3d firstDirection;
    Point3d secondDirection;
    f32 firstAmount;
    f32 secondAmount;

    result->x = result->y = result->z = 0.0f;
    PSVECSubtract(firstTarget, position, &firstDirection);
    PSVECSubtract(secondTarget, position, &secondDirection);
    firstAmount = (f32) frandmod(10) / 10.0f;
    secondAmount = (f32) frandmod(10) / 10.0f;
    PSVECScale(&firstDirection, &firstDirection, firstAmount);
    PSVECScale(&secondDirection, &secondDirection, secondAmount);
    PSVECAdd(position, &firstDirection, position);
    PSVECAdd(position, &secondDirection, result);
}

/* Creates the four player models, their motions, and the arena props during minigame setup. */
void fn_1_4208(void) {
    s16 nightMode = GwMgNightF;
    s16 modelNightMode = nightMode;
    s32 player;
    s32 motion;
    Point3d hookPosition;

    for (player = 0; player < 4; player++) {

        lbl_1_bss_FC[player] = -1;
        lbl_1_bss_11C[player].playerIndex = player;
        lbl_1_bss_11C[player].characterNo = GwPlayerConf[player].charNo;
        lbl_1_bss_11C[player].padNo = GwPlayerConf[player].padNo;
        lbl_1_bss_11C[player].groupNo = GwPlayerConf[player].grpNo;
        lbl_1_bss_11C[player].playerType = GwPlayerConf[player].type;
        lbl_1_bss_11C[player].cpuDifficulty = GwPlayerConf[player].comDif;
        lbl_1_bss_11C[player].sequenceState = 1;
        lbl_1_bss_11C[player].state = 7;
        lbl_1_bss_11C[player].resultPositionMode = 0;
        lbl_1_bss_11C[player].drawingActive = 0;
        lbl_1_bss_11C[player].motionTransitionTimer = 0;
        lbl_1_bss_11C[player].displacement.x =
            lbl_1_bss_11C[player].displacement.y =
                lbl_1_bss_11C[player].displacement.z = 0.0f;
        lbl_1_bss_11C[player].rotationOffset.x =
            lbl_1_bss_11C[player].rotationOffset.y =
                lbl_1_bss_11C[player].rotationOffset.z = 0.0f;
        lbl_1_bss_11C[player].clearedPixelCount = 0;
        lbl_1_bss_11C[player].model = CharModelCreate(lbl_1_bss_11C[player].characterNo, 4);
        for (motion = 0; motion < 5; motion++) {
            lbl_1_bss_11C[player].motionIds[motion] =
                CharMotionCreate(lbl_1_bss_11C[player].characterNo, lbl_1_data_78[motion]);
        }
        CharMotionDataClose(lbl_1_bss_11C[player].characterNo);
        lbl_1_bss_11C[player].motionState = 1;
        CharMotionSet(lbl_1_bss_11C[player].characterNo, lbl_1_bss_11C[player].motionIds[1]);
        CharModelAttrSet(lbl_1_bss_11C[player].characterNo, HU3D_MOTATTR_LOOP);
        Hu3DModelCameraSet(lbl_1_bss_11C[player].model, 1);
        Hu3DModelLayerSet(lbl_1_bss_11C[player].model, 1);
        Hu3DModelShadowSet(lbl_1_bss_11C[player].model);
        lbl_1_bss_11C[player].hookRadius = 50.0f;
        lbl_1_bss_11C[player].characterHeight =
            CharModelHeightGet(lbl_1_bss_11C[player].characterNo);

        if (modelNightMode == 0) {
            lbl_1_bss_11C[player].modelPart0 = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_m613, 10), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_bss_11C[player].modelPart1 = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_m613, 11), HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            lbl_1_bss_11C[player].modelPart0 = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_m613, 10), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_bss_11C[player].modelPart1 = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_m613, 12), HU_MEMNUM_OVL, HEAP_MODEL));
        }
        Hu3DModelAttrSet(lbl_1_bss_11C[player].modelPart0, HU3D_MOTATTR_PAUSE);
        Hu3DModelAttrSet(lbl_1_bss_11C[player].modelPart1, HU3D_MOTATTR_LOOP);
        Hu3DModelCameraSet(lbl_1_bss_11C[player].modelPart0, 1);
        Hu3DModelCameraSet(lbl_1_bss_11C[player].modelPart1, 1);
        Hu3DModelLayerSet(lbl_1_bss_11C[player].modelPart0, 1);
        Hu3DModelLayerSet(lbl_1_bss_11C[player].modelPart1, 1);
        Hu3DModelShadowSet(lbl_1_bss_11C[player].modelPart0);
        lbl_1_bss_11C[player].radius = 65.0f;
        Hu3DMotionCalc(lbl_1_bss_11C[player].model);
        Hu3DMotionCalc(lbl_1_bss_11C[player].modelPart0);
        Hu3DModelObjPosGet(lbl_1_bss_11C[player].modelPart0, "m613_kari-a",
                           &lbl_1_bss_11C[player].firstObjectPosition);
        Hu3DModelObjPosGet(lbl_1_bss_11C[player].modelPart0, "m613_kari-b",
                           &lbl_1_bss_11C[player].secondObjectPosition);
        Hu3DModelObjPosGet(lbl_1_bss_11C[player].modelPart0, "m613_kari-itemhook-r",
                           &hookPosition);

        lbl_1_bss_11C[player].position = lbl_1_data_8C[player];
        lbl_1_bss_11C[player].rotation.x =
            lbl_1_bss_11C[player].rotation.z = 0.0f;
        lbl_1_bss_11C[player].rotation.y = lbl_1_data_EC[player];
        Hu3DModelPosSetV(lbl_1_bss_11C[player].modelPart0, &lbl_1_bss_11C[player].position);
        Hu3DModelRotSetV(lbl_1_bss_11C[player].modelPart0, &lbl_1_bss_11C[player].rotation);
        Hu3DModelPosSetV(lbl_1_bss_11C[player].modelPart1, &lbl_1_bss_11C[player].position);
        Hu3DModelRotSetV(lbl_1_bss_11C[player].modelPart1, &lbl_1_bss_11C[player].rotation);
        Hu3DModelObjPosGet(lbl_1_bss_11C[player].model,
                           CharModelItemHookGet(lbl_1_bss_11C[player].characterNo, 4, 0),
                           &lbl_1_bss_11C[player].hookPosition);
        /* Only hook Z coordinates contribute: both X/Y pairs are forced to zero, and positive
         * character-hook Z is negated before adding the prop-hook Z. */
        hookPosition.x = hookPosition.y = lbl_1_bss_11C[player].hookPosition.x =
            lbl_1_bss_11C[player].hookPosition.y = 0.0f;

        if (lbl_1_bss_11C[player].hookPosition.z > 0.0f) {
            lbl_1_bss_11C[player].hookPosition.z *= -1.0f;
        }
        lbl_1_bss_11C[player].hookPosition.z += hookPosition.z;
        fn_1_3EB0(player);
        Hu3DModelRotSetV(lbl_1_bss_11C[player].model, &lbl_1_bss_11C[player].rotation);
        lbl_1_bss_11C[player].stickX = lbl_1_bss_11C[player].stickY = 0;
        lbl_1_bss_11C[player].buttons =
            lbl_1_bss_11C[player].buttonsDown = 0;
        lbl_1_bss_11C[player].contactFeedbackTimer = 0;
    }
    CharEffectLayerSet(3);
    if (modelNightMode == 0) {
        Vec lightPosition;
        Vec lightDirection;
        GXColor lightColor;
        s16 light;
        lightPosition.x = 0.0f;
        lightPosition.y = 3000.0f;
        lightPosition.z = 0.0f;
        lightDirection.x = 0.0f;
        lightDirection.y = -1.0f;
        lightDirection.z = 0.0f;
        lightColor.r = lightColor.g = lightColor.b = lightColor.a = 255;
        light = CharLightCreateV(&lightPosition, &lightDirection, &lightColor);
    }
    fn_1_DAE8();
}

/* Advances the opening player states and pans their sounds during the intro sequence. */
s32 fn_1_4D38(s16 frame) {
    s16 playerIndex;
    s16 initialState;

    playerIndex = 0;
    initialState = 0;
    for (; playerIndex < 4; playerIndex++) {
        switch (lbl_1_bss_11C[playerIndex].state) {
        case 7:
            if ((frame == 0) && (playerIndex == 0)) {
                lbl_1_bss_10C[0] = HuAudFXPlay(MSM_SE_MOWTOWN_INTRO_PLAYER_0);
                lbl_1_bss_10C[1] = HuAudFXPlay(MSM_SE_MOWTOWN_INTRO_PLAYER_1);
                lbl_1_bss_10C[2] = HuAudFXPlay(MSM_SE_MOWTOWN_INTRO_PLAYER_2);
                lbl_1_bss_10C[3] = HuAudFXPlay(MSM_SE_MOWTOWN_INTRO_PLAYER_3);
            }
            if (frame >= 45) {
                lbl_1_bss_11C[playerIndex].state = 8;
            }
            break;
        case 8:
            lbl_1_bss_11C[playerIndex].buttons = PAD_BUTTON_A;
            if (frame >= 120) {
                lbl_1_bss_11C[playerIndex].state = 9;
            }
            break;
        case 9:
            lbl_1_bss_11C[playerIndex].buttons = PAD_BUTTON_A;
            if (frame >= 140) {
                lbl_1_bss_11C[playerIndex].state = 10;
            }
            break;
        case 10:
            lbl_1_bss_11C[playerIndex].buttons = PAD_BUTTON_A;
            if (frame >= 155) {
                lbl_1_bss_11C[playerIndex].state = 11;
            }
            break;
        case 11:
        default:
            lbl_1_bss_11C[playerIndex].state = 11;
            break;
        }

        {
            f32 pan = 1000.0f;
            pan = (lbl_1_bss_11C[playerIndex].position.x - -500.0f) / pan;
            pan = 32.0f + 64.0f * pan;
            HuAudFXPanning(lbl_1_bss_10C[playerIndex], (s16)pan);
        }
    }

    fn_1_5178();
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_11C[playerIndex].state != 11) {
            break;
        }
    }
    if (playerIndex >= 4) {
        return 1;
    }
    return 0;
}

void fn_1_5038(s16 mode, s16 frameNo) {
    fn_1_5178();
}

/* Reads human and CPU controls, resolves contacts, and updates player sound panning. */
void fn_1_5058(void)
{
    s16 playerIndex;

    fn_1_D61C();
    fn_1_DD54();
    fn_1_5178();
    playerIndex = 0;
    while (playerIndex < 4) {
            f32 panValue = 1000.0f;

            panValue = (lbl_1_bss_11C[playerIndex].position.x -
                -500.0f) / panValue;
            panValue = 32.0f + (64.0f * panValue);

        HuAudFXPanning(lbl_1_bss_10C[playerIndex], (s16) panValue);
        if (lbl_1_bss_FC[playerIndex] >= 0) {
            HuAudFXPanning(lbl_1_bss_FC[playerIndex], (s16) panValue);
        }
        playerIndex++;
    }
}

/* Resolves player and arena-edge contacts, then updates collision feedback and player positions. */
void fn_1_5178(void) {
    struct PairFlags { s16 playerContact; s16 arenaEdge; } pairFlags[4];
    Point3d playerPosition;
    Point3d playerAxis;
    Point3d playerDisplacement;
    Point3d otherPosition;
    Point3d otherAxis;
    Point3d otherDisplacement;
    Point3d separation;
    Point3d correction;
    s16 resolved[4];
    f32 playerRadius;
    f32 hookRadiusOrProjection;
    f32 otherRadius;
    f32 otherHookRadius;
    f32 separationOrPan;
    s16 playerIndex;
    s16 otherIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        fn_1_9C7C(playerIndex);
        resolved[playerIndex] = 0;
        pairFlags[playerIndex].playerContact = pairFlags[playerIndex].arenaEdge = 0;
    }

/* In these two contact passes, sphere helpers also modify playerDisplacement locally; only
 * otherDisplacement is written back to a player's stored displacement. */
for (playerIndex = 0; playerIndex < 4; playerIndex++) {
if (lbl_1_bss_11C[playerIndex].state == 11) {
playerPosition = lbl_1_bss_11C[playerIndex].position;
playerAxis = lbl_1_bss_11C[playerIndex].hookPosition;
playerDisplacement = lbl_1_bss_11C[playerIndex].displacement;
playerRadius = lbl_1_bss_11C[playerIndex].radius;
hookRadiusOrProjection = lbl_1_bss_11C[playerIndex].hookRadius;
playerPosition.y = playerAxis.y = playerDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[playerIndex].rotation.y + lbl_1_bss_11C[playerIndex].rotationOffset.y,
          0.0f, playerAxis, &playerAxis);
PSVECAdd(&playerPosition, &playerAxis, &playerAxis);
for (otherIndex = 0; otherIndex < 4; otherIndex++) {
if ((lbl_1_bss_11C[otherIndex].state == 11) && (playerIndex != otherIndex)) {
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y + lbl_1_bss_11C[otherIndex].rotationOffset.y,
          0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
lbl_1_bss_0 = 0;
if (fn_1_70C(playerPosition, lbl_1_bss_11C[playerIndex].rotation.y, playerRadius,
             playerDisplacement, otherPosition, otherRadius, &otherDisplacement) != 0) {
    lbl_1_bss_11C[otherIndex].displacement = otherDisplacement;
    fn_1_DBDC(playerIndex);
    fn_1_DBDC(otherIndex);
    resolved[playerIndex] = resolved[otherIndex] = 1;
    pairFlags[playerIndex].playerContact = pairFlags[otherIndex].playerContact = 1;
}
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y + lbl_1_bss_11C[otherIndex].rotationOffset.y,
          0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
lbl_1_bss_0 = 0;
if (fn_1_28C(playerPosition, &playerDisplacement, playerRadius, otherAxis, &otherDisplacement,
             otherHookRadius) != 0) {
    lbl_1_bss_11C[otherIndex].displacement = otherDisplacement;
    fn_1_DBDC(playerIndex);
    resolved[playerIndex] = resolved[otherIndex] = 1;
}
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y + lbl_1_bss_11C[otherIndex].rotationOffset.y,
          0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
if (fn_1_28C(playerAxis, &playerDisplacement, hookRadiusOrProjection, otherPosition,
             &otherDisplacement, otherRadius) != 0) {
    lbl_1_bss_11C[otherIndex].displacement = otherDisplacement;
    fn_1_DBDC(otherIndex);
    resolved[playerIndex] = resolved[otherIndex] = 1;
}
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y + lbl_1_bss_11C[otherIndex].rotationOffset.y,
          0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
if (fn_1_28C(playerAxis, &playerDisplacement, hookRadiusOrProjection, otherAxis, &otherDisplacement,
             otherHookRadius) != 0) {
    lbl_1_bss_11C[otherIndex].displacement = otherDisplacement;
}
}
}
}
}

for (playerIndex = 3; playerIndex >= 0; playerIndex--) {
if (lbl_1_bss_11C[playerIndex].state == 11) {
playerPosition = lbl_1_bss_11C[playerIndex].position;
playerAxis = lbl_1_bss_11C[playerIndex].hookPosition;
playerDisplacement = lbl_1_bss_11C[playerIndex].displacement;
playerRadius = lbl_1_bss_11C[playerIndex].radius;
hookRadiusOrProjection = lbl_1_bss_11C[playerIndex].hookRadius;
playerPosition.y = playerAxis.y = playerDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[playerIndex].rotation.y + lbl_1_bss_11C[playerIndex].rotationOffset.y,
          0.0f, playerAxis, &playerAxis);
PSVECAdd(&playerPosition, &playerAxis, &playerAxis);
for (otherIndex = playerIndex; otherIndex >= 0; otherIndex--) {
if ((lbl_1_bss_11C[otherIndex].state == 11) && (playerIndex != otherIndex)) {
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y + lbl_1_bss_11C[otherIndex].rotationOffset.y,
          0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
lbl_1_bss_0 = 0;
if (fn_1_70C(playerPosition, lbl_1_bss_11C[playerIndex].rotation.y, playerRadius,
             playerDisplacement, otherPosition, otherRadius, &otherDisplacement) != 0) {
    lbl_1_bss_11C[otherIndex].displacement = otherDisplacement;
    resolved[playerIndex] = resolved[otherIndex] = 1;
    pairFlags[playerIndex].playerContact = pairFlags[otherIndex].playerContact = 1;
}
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y + lbl_1_bss_11C[otherIndex].rotationOffset.y,
          0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
lbl_1_bss_0 = 0;
if (fn_1_28C(playerPosition, &playerDisplacement, playerRadius, otherAxis, &otherDisplacement,
             otherHookRadius) != 0) {
    lbl_1_bss_11C[otherIndex].displacement = otherDisplacement;
    resolved[playerIndex] = resolved[otherIndex] = 1;
}
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y + lbl_1_bss_11C[otherIndex].rotationOffset.y,
          0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
if (fn_1_28C(playerAxis, &playerDisplacement, hookRadiusOrProjection, otherPosition,
             &otherDisplacement, otherRadius) != 0) {
    lbl_1_bss_11C[otherIndex].displacement = otherDisplacement;
    resolved[playerIndex] = resolved[otherIndex] = 1;
}
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y + lbl_1_bss_11C[otherIndex].rotationOffset.y,
          0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
if (fn_1_28C(playerAxis, &playerDisplacement, hookRadiusOrProjection, otherAxis, &otherDisplacement,
             otherHookRadius) != 0) {
    lbl_1_bss_11C[otherIndex].displacement = otherDisplacement;
}
}
}
}
}
/* These two overlap passes reuse playerRadius from the preceding pass instead of loading it for
 * each current player. */
lbl_1_bss_0 = 0;
for (playerIndex = 0; playerIndex < 4; playerIndex++) {
if (lbl_1_bss_11C[playerIndex].state == 11) {
playerPosition = lbl_1_bss_11C[playerIndex].position;
playerAxis = lbl_1_bss_11C[playerIndex].hookPosition;
playerDisplacement = lbl_1_bss_11C[playerIndex].displacement;
hookRadiusOrProjection = lbl_1_bss_11C[playerIndex].hookRadius;
playerPosition.y = playerAxis.y = playerDisplacement.y = 0.0f;
PSVECAdd(&playerPosition, &playerDisplacement, &playerPosition);
fn_1_3FFC(0.0f, lbl_1_bss_11C[playerIndex].rotation.y + lbl_1_bss_11C[playerIndex].rotationOffset.y,
          0.0f, playerAxis, &playerAxis);
PSVECAdd(&playerPosition, &playerAxis, &playerAxis);
for (otherIndex = playerIndex + 1; otherIndex < 4; otherIndex++) {
if ((lbl_1_bss_11C[otherIndex].state == 11) && (playerIndex != otherIndex)) {
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
PSVECAdd(&otherPosition, &otherDisplacement, &otherPosition);
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y + lbl_1_bss_11C[otherIndex].rotationOffset.y,
          0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
if (fn_1_0(playerPosition, playerRadius, otherAxis, otherHookRadius) != 0) {
lbl_1_bss_11C[otherIndex].rotationOffset.y = 0.0f;
resolved[playerIndex] = resolved[otherIndex] = 0;
}
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
PSVECAdd(&otherPosition, &otherDisplacement, &otherPosition);
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y + lbl_1_bss_11C[otherIndex].rotationOffset.y,
          0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
if (fn_1_0(playerAxis, hookRadiusOrProjection, otherAxis, otherHookRadius) != 0) {
lbl_1_bss_11C[otherIndex].rotationOffset.y = 0.0f;
}
}
}
}
}
for (playerIndex = 3; playerIndex >= 0; playerIndex--) {
if (lbl_1_bss_11C[playerIndex].state == 11) {
playerPosition = lbl_1_bss_11C[playerIndex].position;
playerAxis = lbl_1_bss_11C[playerIndex].hookPosition;
playerDisplacement = lbl_1_bss_11C[playerIndex].displacement;
hookRadiusOrProjection = lbl_1_bss_11C[playerIndex].hookRadius;
playerPosition.y = playerAxis.y = playerDisplacement.y = 0.0f;
PSVECAdd(&playerPosition, &playerDisplacement, &playerPosition);
fn_1_3FFC(0.0f, lbl_1_bss_11C[playerIndex].rotation.y + lbl_1_bss_11C[playerIndex].rotationOffset.y,
          0.0f, playerAxis, &playerAxis);
PSVECAdd(&playerPosition, &playerAxis, &playerAxis);
for (otherIndex = playerIndex - 1; otherIndex >= 0; otherIndex--) {
if ((lbl_1_bss_11C[otherIndex].state == 11) && (playerIndex != otherIndex)) {
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
PSVECAdd(&otherPosition, &otherDisplacement, &otherPosition);
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y + lbl_1_bss_11C[otherIndex].rotationOffset.y,
          0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
if (fn_1_0(playerPosition, playerRadius, otherAxis, otherHookRadius) != 0) {
lbl_1_bss_11C[otherIndex].rotationOffset.y = 0.0f;
resolved[playerIndex] = resolved[otherIndex] = 0;
}
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
PSVECAdd(&otherPosition, &otherDisplacement, &otherPosition);
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y + lbl_1_bss_11C[otherIndex].rotationOffset.y,
          0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
if (fn_1_0(playerAxis, hookRadiusOrProjection, otherAxis, otherHookRadius) != 0) {
lbl_1_bss_11C[otherIndex].rotationOffset.y = 0.0f;
}
}
}
}
}
for (playerIndex = 0; playerIndex < 4; playerIndex++) {
if (lbl_1_bss_11C[playerIndex].state == 11) {
playerPosition = lbl_1_bss_11C[playerIndex].position;
playerDisplacement = lbl_1_bss_11C[playerIndex].displacement;
playerRadius = lbl_1_bss_11C[playerIndex].radius;
playerPosition.y = playerDisplacement.y = 0.0f;
PSVECAdd(&playerPosition, &playerDisplacement, &playerPosition);
fn_1_10C5C(&playerDisplacement, &playerAxis);
otherPosition.x = 0.0f;
otherPosition.y = 0.0f;
otherPosition.z = -500.0f;
otherDisplacement.x = 0.0f;
otherDisplacement.y = 0.0f;
otherDisplacement.z = 1.0f;
if ((playerPosition.z < (otherPosition.z + playerRadius)) && (playerDisplacement.z < 0.0f)) {
playerDisplacement.z = 0.0f;
fn_1_DCF4(playerIndex);
hookRadiusOrProjection = PSVECDotProduct(&otherDisplacement, &playerAxis);
if (hookRadiusOrProjection < -0.1) {
resolved[playerIndex] = 1;
                    pairFlags[playerIndex].arenaEdge = 1;
}
}
otherPosition.x = 0.0f;
otherPosition.y = 0.0f;
otherPosition.z = 500.0f;
otherDisplacement.x = 0.0f;
otherDisplacement.y = 0.0f;
otherDisplacement.z = -1.0f;
if ((playerPosition.z > (otherPosition.z - playerRadius)) && (playerDisplacement.z > 0.0f)) {
playerDisplacement.z = 0.0f;
fn_1_DCF4(playerIndex);
hookRadiusOrProjection = PSVECDotProduct(&otherDisplacement, &playerAxis);
if (hookRadiusOrProjection < -0.1) {
resolved[playerIndex] = 1;
                    pairFlags[playerIndex].arenaEdge = 1;
}
}
otherPosition.x = -500.0f;
otherPosition.y = 0.0f;
otherPosition.z = 0.0f;
otherDisplacement.x = 1.0f;
otherDisplacement.y = 0.0f;
otherDisplacement.z = 0.0f;
if ((playerPosition.x < (otherPosition.x + playerRadius)) && (playerDisplacement.x < 0.0f)) {
playerDisplacement.x = 0.0f;
fn_1_DCF4(playerIndex);
hookRadiusOrProjection = PSVECDotProduct(&otherDisplacement, &playerAxis);
if (hookRadiusOrProjection < -0.1) {
resolved[playerIndex] = 1;
                    pairFlags[playerIndex].arenaEdge = 1;
}
}
otherPosition.x = 500.0f;
otherPosition.y = 0.0f;
otherPosition.z = 0.0f;
otherDisplacement.x = -1.0f;
otherDisplacement.y = 0.0f;
otherDisplacement.z = 0.0f;
if ((playerPosition.x > (otherPosition.x - playerRadius)) && (playerDisplacement.x > 0.0f)) {
playerDisplacement.x = 0.0f;
fn_1_DCF4(playerIndex);
hookRadiusOrProjection = PSVECDotProduct(&otherDisplacement, &playerAxis);
if (hookRadiusOrProjection < -0.1) {
resolved[playerIndex] = 1;
                    pairFlags[playerIndex].arenaEdge = 1;
}
}
lbl_1_bss_11C[playerIndex].displacement = playerDisplacement;
}
}
for (playerIndex = 0; playerIndex < 4; playerIndex++) {
if (lbl_1_bss_11C[playerIndex].state == 11) {
playerPosition = lbl_1_bss_11C[playerIndex].position;
playerAxis = lbl_1_bss_11C[playerIndex].hookPosition;
playerDisplacement = lbl_1_bss_11C[playerIndex].displacement;
playerRadius = lbl_1_bss_11C[playerIndex].radius;
hookRadiusOrProjection = lbl_1_bss_11C[playerIndex].hookRadius;
playerPosition.y = playerAxis.y = playerDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[playerIndex].rotation.y, 0.0f, playerAxis, &playerAxis);
PSVECAdd(&playerPosition, &playerAxis, &playerAxis);
for (otherIndex = playerIndex + 1; otherIndex < 4; otherIndex++) {
if ((lbl_1_bss_11C[otherIndex].state == 11) && (playerIndex != otherIndex)) {
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y, 0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
if ((fn_1_0(playerPosition, playerRadius, otherPosition, otherRadius) != 0) &&
    (PSVECSquareMag(&otherDisplacement) > 0.0f)) {
    separation.x = lbl_1_bss_11C[otherIndex].position.x - lbl_1_bss_11C[playerIndex].position.x;
    separation.y = lbl_1_bss_11C[otherIndex].position.y - lbl_1_bss_11C[playerIndex].position.y;
    separation.z = lbl_1_bss_11C[otherIndex].position.z - lbl_1_bss_11C[playerIndex].position.z;
    fn_1_10C5C(&separation, &correction);
    separationOrPan = lbl_1_bss_11C[playerIndex].radius + lbl_1_bss_11C[otherIndex].radius;
    correction.x *= separationOrPan;
    correction.y *= separationOrPan;
    correction.z *= separationOrPan;
    lbl_1_bss_11C[otherIndex].position.x = correction.x + lbl_1_bss_11C[playerIndex].position.x;
    lbl_1_bss_11C[otherIndex].position.y = correction.y + lbl_1_bss_11C[playerIndex].position.y;
    lbl_1_bss_11C[otherIndex].position.z = correction.z + lbl_1_bss_11C[playerIndex].position.z;
}
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y, 0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
if ((fn_1_0(playerPosition, playerRadius, otherAxis, otherHookRadius) != 0) &&
    (PSVECSquareMag(&otherDisplacement) > 0.0f)) {
    separation.x = otherAxis.x - playerPosition.x;
    separation.y = otherAxis.y - playerPosition.y;
    separation.z = otherAxis.z - playerPosition.z;
    fn_1_10C5C(&separation, &correction);
    separationOrPan = playerRadius + otherHookRadius;
    correction.x *= separationOrPan;
    correction.y *= separationOrPan;
    correction.z *= separationOrPan;
    separation.x = (playerPosition.x + correction.x) - otherAxis.x;
    separation.y = (playerPosition.y + correction.y) - otherAxis.y;
    separation.z = (playerPosition.z + correction.z) - otherAxis.z;
    lbl_1_bss_11C[otherIndex].position.x += separation.x;
    lbl_1_bss_11C[otherIndex].position.y += separation.y;
    lbl_1_bss_11C[otherIndex].position.z += separation.z;
}
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y, 0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
if ((fn_1_0(playerAxis, hookRadiusOrProjection, otherPosition, otherRadius) != 0) &&
    (PSVECSquareMag(&otherDisplacement) > 0.0f)) {
    separation.x = otherPosition.x - playerAxis.x;
    separation.y = otherPosition.y - playerAxis.y;
    separation.z = otherPosition.z - playerAxis.z;
    fn_1_10C5C(&separation, &correction);
    separationOrPan = hookRadiusOrProjection + otherRadius;
    correction.x *= separationOrPan;
    correction.y *= separationOrPan;
    correction.z *= separationOrPan;
    separation.x = (playerAxis.x + correction.x) - otherPosition.x;
    separation.y = (playerAxis.y + correction.y) - otherPosition.y;
    separation.z = (playerAxis.z + correction.z) - otherPosition.z;
    lbl_1_bss_11C[otherIndex].position.x += separation.x;
    lbl_1_bss_11C[otherIndex].position.y += separation.y;
    lbl_1_bss_11C[otherIndex].position.z += separation.z;
}
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y, 0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
if ((fn_1_0(playerAxis, hookRadiusOrProjection, otherAxis, otherHookRadius) != 0) &&
    (PSVECSquareMag(&otherDisplacement) > 0.0f)) {
    separation.x = otherAxis.x - playerAxis.x;
    separation.y = otherAxis.y - playerAxis.y;
    separation.z = otherAxis.z - playerAxis.z;
    fn_1_10C5C(&separation, &correction);
    separationOrPan = hookRadiusOrProjection + otherHookRadius;
    correction.x *= separationOrPan;
    correction.y *= separationOrPan;
    correction.z *= separationOrPan;
    separation.x = (playerAxis.x + correction.x) - otherAxis.x;
    separation.y = (playerAxis.y + correction.y) - otherAxis.y;
    separation.z = (playerAxis.z + correction.z) - otherAxis.z;
    lbl_1_bss_11C[otherIndex].position.x += separation.x;
    lbl_1_bss_11C[otherIndex].position.y += separation.y;
    lbl_1_bss_11C[otherIndex].position.z += separation.z;
}
}
}
}
}
for (playerIndex = 3; playerIndex >= 0; playerIndex--) {
if (lbl_1_bss_11C[playerIndex].state == 11) {
playerPosition = lbl_1_bss_11C[playerIndex].position;
playerAxis = lbl_1_bss_11C[playerIndex].hookPosition;
playerDisplacement = lbl_1_bss_11C[playerIndex].displacement;
playerRadius = lbl_1_bss_11C[playerIndex].radius;
hookRadiusOrProjection = lbl_1_bss_11C[playerIndex].hookRadius;
playerPosition.y = playerAxis.y = playerDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[playerIndex].rotation.y, 0.0f, playerAxis, &playerAxis);
PSVECAdd(&playerPosition, &playerAxis, &playerAxis);
for (otherIndex = playerIndex - 1; otherIndex >= 0; otherIndex--) {
if ((lbl_1_bss_11C[otherIndex].state == 11) && (playerIndex != otherIndex)) {
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y, 0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
if ((fn_1_0(playerPosition, playerRadius, otherPosition, otherRadius) != 0) &&
    (PSVECSquareMag(&otherDisplacement) > 0.0f)) {
    separation.x = otherPosition.x - playerPosition.x;
    separation.y = otherPosition.y - playerPosition.y;
    separation.z = otherPosition.z - playerPosition.z;
    fn_1_10C5C(&separation, &correction);
    separationOrPan = playerRadius + otherRadius;
    correction.x *= separationOrPan;
    correction.y *= separationOrPan;
    correction.z *= separationOrPan;
    lbl_1_bss_11C[otherIndex].position.x = playerPosition.x + correction.x;
    lbl_1_bss_11C[otherIndex].position.y = playerPosition.y + correction.y;
    lbl_1_bss_11C[otherIndex].position.z = playerPosition.z + correction.z;
}
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y, 0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
if ((fn_1_0(playerPosition, playerRadius, otherAxis, otherHookRadius) != 0) &&
    (PSVECSquareMag(&otherDisplacement) > 0.0f)) {
    separation.x = otherAxis.x - playerPosition.x;
    separation.y = otherAxis.y - playerPosition.y;
    separation.z = otherAxis.z - playerPosition.z;
    fn_1_10C5C(&separation, &correction);
    separationOrPan = playerRadius + otherHookRadius;
    correction.x *= separationOrPan;
    correction.y *= separationOrPan;
    correction.z *= separationOrPan;
    separation.x = (playerPosition.x + correction.x) - otherAxis.x;
    separation.y = (playerPosition.y + correction.y) - otherAxis.y;
    separation.z = (playerPosition.z + correction.z) - otherAxis.z;
    lbl_1_bss_11C[otherIndex].position.x += separation.x;
    lbl_1_bss_11C[otherIndex].position.y += separation.y;
    lbl_1_bss_11C[otherIndex].position.z += separation.z;
}
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y, 0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
if ((fn_1_0(playerAxis, hookRadiusOrProjection, otherPosition, otherRadius) != 0) &&
    (PSVECSquareMag(&otherDisplacement) > 0.0f)) {
    separation.x = otherPosition.x - playerAxis.x;
    separation.y = otherPosition.y - playerAxis.y;
    separation.z = otherPosition.z - playerAxis.z;
    fn_1_10C5C(&separation, &correction);
    separationOrPan = hookRadiusOrProjection + otherRadius;
    correction.x *= separationOrPan;
    correction.y *= separationOrPan;
    correction.z *= separationOrPan;
    separation.x = (playerAxis.x + correction.x) - otherPosition.x;
    separation.y = (playerAxis.y + correction.y) - otherPosition.y;
    separation.z = (playerAxis.z + correction.z) - otherPosition.z;
    lbl_1_bss_11C[otherIndex].position.x += separation.x;
    lbl_1_bss_11C[otherIndex].position.y += separation.y;
    lbl_1_bss_11C[otherIndex].position.z += separation.z;
}
otherPosition = lbl_1_bss_11C[otherIndex].position;
otherAxis = lbl_1_bss_11C[otherIndex].hookPosition;
otherDisplacement = lbl_1_bss_11C[otherIndex].displacement;
otherRadius = lbl_1_bss_11C[otherIndex].radius;
otherHookRadius = lbl_1_bss_11C[otherIndex].hookRadius;
otherPosition.y = otherAxis.y = otherDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[otherIndex].rotation.y, 0.0f, otherAxis, &otherAxis);
PSVECAdd(&otherPosition, &otherAxis, &otherAxis);
if ((fn_1_0(playerAxis, hookRadiusOrProjection, otherAxis, otherHookRadius) != 0) &&
    (PSVECSquareMag(&otherDisplacement) > 0.0f)) {
    separation.x = otherAxis.x - playerAxis.x;
    separation.y = otherAxis.y - playerAxis.y;
    separation.z = otherAxis.z - playerAxis.z;
    fn_1_10C5C(&separation, &correction);
    separationOrPan = hookRadiusOrProjection + otherHookRadius;
    correction.x *= separationOrPan;
    correction.y *= separationOrPan;
    correction.z *= separationOrPan;
    separation.x = (playerAxis.x + correction.x) - otherAxis.x;
    separation.y = (playerAxis.y + correction.y) - otherAxis.y;
    separation.z = (playerAxis.z + correction.z) - otherAxis.z;
    lbl_1_bss_11C[otherIndex].position.x += separation.x;
    lbl_1_bss_11C[otherIndex].position.y += separation.y;
    lbl_1_bss_11C[otherIndex].position.z += separation.z;
}
}
}
}
}
for (playerIndex = 0; playerIndex < 4; playerIndex++) {
if (lbl_1_bss_11C[playerIndex].state == 11) {
playerPosition = lbl_1_bss_11C[playerIndex].position;
playerAxis = lbl_1_bss_11C[playerIndex].hookPosition;
playerDisplacement = lbl_1_bss_11C[playerIndex].displacement;
playerRadius = lbl_1_bss_11C[playerIndex].radius;
hookRadiusOrProjection = lbl_1_bss_11C[playerIndex].hookRadius;
playerPosition.y = playerAxis.y = playerDisplacement.y = 0.0f;
fn_1_3FFC(0.0f, lbl_1_bss_11C[playerIndex].rotation.y, 0.0f, playerAxis, &playerAxis);
PSVECAdd(&playerPosition, &playerAxis, &playerAxis);
separationOrPan = fn_1_99A4(playerPosition, playerDisplacement, playerRadius, 0);
lbl_1_bss_11C[playerIndex].position.z -= separationOrPan;
separationOrPan = fn_1_99A4(playerPosition, playerDisplacement, playerRadius, 1);
lbl_1_bss_11C[playerIndex].position.z -= separationOrPan;
separationOrPan = fn_1_99A4(playerPosition, playerDisplacement, playerRadius, 2);
lbl_1_bss_11C[playerIndex].position.x -= separationOrPan;
separationOrPan = fn_1_99A4(playerPosition, playerDisplacement, playerRadius, 3);
lbl_1_bss_11C[playerIndex].position.x -= separationOrPan;
}
}
for (playerIndex = 0; playerIndex < 4; playerIndex++) {
    /* Only A/B and stick X qualify as input for the contact-feedback counter; stick Y alone
     * does not. */
    if ((resolved[playerIndex] != 0) &&
        (((lbl_1_bss_11C[playerIndex].buttons & (PAD_BUTTON_A | PAD_BUTTON_B)) != 0) ||
         (lbl_1_bss_11C[playerIndex].stickX != 0))) {
        lbl_1_bss_11C[playerIndex].contactFeedbackTimer++;
    } else {
        lbl_1_bss_11C[playerIndex].contactFeedbackTimer = 0;
    }
if (lbl_1_bss_11C[playerIndex].contactFeedbackTimer % 60 == 1) {
omVibrate(playerIndex, 20, 7, 3);
            if (pairFlags[playerIndex].playerContact != 0) {
separationOrPan = 1000.0f;
separationOrPan = (lbl_1_bss_11C[playerIndex].position.x - -500.0f) / separationOrPan;
separationOrPan = 32.0f + 64.0f * separationOrPan;
HuAudFXPanning(HuAudFXPlay(MSM_SE_MOWTOWN_PLAYER_CONTACT), (s16)separationOrPan);
}
            if (pairFlags[playerIndex].arenaEdge != 0) {
separationOrPan = 1000.0f;
separationOrPan = (lbl_1_bss_11C[playerIndex].position.x - -500.0f) / separationOrPan;
separationOrPan = 32.0f + 64.0f * separationOrPan;
HuAudFXPanning(HuAudFXPlay(MSM_SE_MOWTOWN_ARENA_CONTACT), (s16)separationOrPan);
}
}
}
fn_1_B4A8();
}

/* Tests whether movement crosses an arena edge; zeroes both Y values and adds displacement to
 * movingPosition in place. The constructed inward normals are unused. */
s32 fn_1_96E8(Point3d *movingPosition, Point3d *displacement, f32 radius) {
    Point3d targetBoundary;
    Point3d unusedInwardNormal;
    s32 crossesBoundary = 0;

    movingPosition->y = displacement->y = 0.0f;
    PSVECAdd(movingPosition, displacement, movingPosition);

    targetBoundary.x = 0.0f;
    targetBoundary.y = 0.0f;
    targetBoundary.z = -500.0f;
    unusedInwardNormal.x = 0.0f;
    unusedInwardNormal.y = 0.0f;
    unusedInwardNormal.z = 1.0f;
    if ((movingPosition->z < (targetBoundary.z + radius)) &&
        (displacement->z < 0.0f)) {
        crossesBoundary = 1;
    }

    targetBoundary.x = 0.0f;
    targetBoundary.y = 0.0f;
    targetBoundary.z = 500.0f;
    unusedInwardNormal.x = 0.0f;
    unusedInwardNormal.y = 0.0f;
    unusedInwardNormal.z = -1.0f;
    if ((movingPosition->z > (targetBoundary.z - radius)) &&
        (displacement->z > 0.0f)) {
        crossesBoundary = 1;
    }

    targetBoundary.x = -500.0f;
    targetBoundary.y = 0.0f;
    targetBoundary.z = 0.0f;
    unusedInwardNormal.x = 1.0f;
    unusedInwardNormal.y = 0.0f;
    unusedInwardNormal.z = 0.0f;
    if ((movingPosition->x < (targetBoundary.x + radius)) &&
        (displacement->x < 0.0f)) {
        crossesBoundary = 1;
    }

    targetBoundary.x = 500.0f;
    targetBoundary.y = 0.0f;
    targetBoundary.z = 0.0f;
    unusedInwardNormal.x = -1.0f;
    unusedInwardNormal.y = 0.0f;
    unusedInwardNormal.z = 0.0f;
    if ((movingPosition->x > (targetBoundary.x - radius)) &&
        (displacement->x > 0.0f)) {
        crossesBoundary = 1;
    }
    return crossesBoundary;
}

/* Returns the amount to move a player back inside the selected arena edge.
 * The constructed inward normals are unused. */
f32 fn_1_99A4(Point3d movingPosition, Point3d displacement, f32 radius, u8 side) {
    Point3d boundary;
    Point3d unusedInwardNormal;
    f32 distance = 0.0f;

    movingPosition.y = displacement.y = 0.0f;
    PSVECAdd(&movingPosition, &displacement, &movingPosition);

    if (side == 0) {
        boundary.x = 0.0f;
        boundary.y = 0.0f;
        boundary.z = -500.0f;
        unusedInwardNormal.x = 0.0f;
        unusedInwardNormal.y = 0.0f;
        unusedInwardNormal.z = 1.0f;
        if (movingPosition.z < (boundary.z + radius)) {
            distance = movingPosition.z - (boundary.z + radius);
        }
    }
    if (side == 1) {
        boundary.x = 0.0f;
        boundary.y = 0.0f;
        boundary.z = 500.0f;
        unusedInwardNormal.x = 0.0f;
        unusedInwardNormal.y = 0.0f;
        unusedInwardNormal.z = -1.0f;
        if (movingPosition.z > (boundary.z - radius)) {
            distance = movingPosition.z - (boundary.z - radius);
        }
    }
    if (side == 2) {
        boundary.x = -500.0f;
        boundary.y = 0.0f;
        boundary.z = 0.0f;
        unusedInwardNormal.x = 1.0f;
        unusedInwardNormal.y = 0.0f;
        unusedInwardNormal.z = 0.0f;
        if (movingPosition.x < (boundary.x + radius)) {
            distance = movingPosition.x - (boundary.x + radius);
        }
    }
    if (side == 3) {
        boundary.x = 500.0f;
        boundary.y = 0.0f;
        boundary.z = 0.0f;
        unusedInwardNormal.x = -1.0f;
        unusedInwardNormal.y = 0.0f;
        unusedInwardNormal.z = 0.0f;
        if (movingPosition.x > (boundary.x - radius)) {
            distance = movingPosition.x - (boundary.x - radius);
        }
    }
    return distance;
}

/* Applies one player's controls to movement, turn, animation, and movement sound each update. */
void fn_1_9C7C(s16 playerIndex) {
    Point3d referenceDirection;
    Point3d inputDirection;
    f32 inputAlignment;
    s16 transitionStatus;

    switch (lbl_1_bss_11C[playerIndex].sequenceState) {
    case 1:
        if (lbl_1_bss_11C[playerIndex].motionTransitionTimer != 0) {
            goto m613TickDown;
        }
        break;
    case 0:
    default:
        goto m613TickDownEnd;
    }
    {
        lbl_1_bss_11C[playerIndex].drawingActive = 0;
        if ((lbl_1_bss_11C[playerIndex].stickX != 0) || (lbl_1_bss_11C[playerIndex].stickY != 0)) {
            transitionStatus = 0;
            referenceDirection.x = 1.0f;
            referenceDirection.y = referenceDirection.z = 0.0f;

            fn_1_3FFC(0.0f, lbl_1_bss_11C[playerIndex].rotation.y, 0.0f, referenceDirection,
                      &referenceDirection);
            inputDirection.x = (f32)lbl_1_bss_11C[playerIndex].stickX;
            inputDirection.y = 0.0f;
            inputDirection.z = (f32)(lbl_1_bss_11C[playerIndex].stickY * -1);
            fn_1_10C5C(&inputDirection, &inputDirection);
            inputAlignment = (referenceDirection.z * inputDirection.z) +
                             ((referenceDirection.x * inputDirection.x) +
                              (referenceDirection.y * inputDirection.y));
            if (inputAlignment < -0.1) {
                lbl_1_bss_11C[playerIndex].rotationOffset.y = -2.0f;
                transitionStatus = 1;
            } else if (inputAlignment > 0.1) {
                lbl_1_bss_11C[playerIndex].rotationOffset.y = 2.0f;
                transitionStatus = 1;
            } else {
                referenceDirection.x = referenceDirection.y = 0.0f;
                referenceDirection.z = 1.0f;

                fn_1_3FFC(0.0f, lbl_1_bss_11C[playerIndex].rotation.y, 0.0f, referenceDirection,
                          &referenceDirection);
                inputAlignment = (referenceDirection.z * inputDirection.z) +
                                 ((referenceDirection.x * inputDirection.x) +
                                  (referenceDirection.y * inputDirection.y));
                if (inputAlignment < 0.0f) {
                    lbl_1_bss_11C[playerIndex].rotationOffset.y = -2.0f;
                    transitionStatus = 1;
                }
            }
            if (transitionStatus != 0) {
                if (((s32) (lbl_1_bss_11C[playerIndex].buttons & PAD_BUTTON_A) == 0) &&
                    ((s32) (lbl_1_bss_11C[playerIndex].buttons & PAD_BUTTON_B) != 0)) {
                    if (lbl_1_bss_11C[playerIndex].motionState != 3) {
                        fn_1_3C80(playerIndex, 3);
                        lbl_1_bss_11C[playerIndex].motionTransitionTimer = 10;
                    }
                    Hu3DModelAttrReset(lbl_1_bss_11C[playerIndex].modelPart0,
                                       HU3D_MOTATTR_LOOP | HU3D_MOTATTR_PAUSE);
                    Hu3DModelAttrSet(lbl_1_bss_11C[playerIndex].modelPart0, HU3D_MOTATTR_REV);
                } else {
                    if (lbl_1_bss_11C[playerIndex].motionState != 2) {
                        fn_1_3C80(playerIndex, 2);
                        lbl_1_bss_11C[playerIndex].motionTransitionTimer = 10;
                    }
                    Hu3DModelAttrReset(lbl_1_bss_11C[playerIndex].modelPart0,
                                       HU3D_MOTATTR_LOOP | HU3D_MOTATTR_REV);
                    Hu3DModelAttrSet(lbl_1_bss_11C[playerIndex].modelPart0, HU3D_MOTATTR_PAUSE);
                }
            }
            if ((transitionStatus == 0) &&
                ((s32) (lbl_1_bss_11C[playerIndex].buttons & PAD_BUTTON_A) == 0) &&
                ((s32) (lbl_1_bss_11C[playerIndex].buttons & PAD_BUTTON_B) == 0)) {
                if (lbl_1_bss_11C[playerIndex].motionState != 1) {
                    fn_1_3C80(playerIndex, 1);
                    lbl_1_bss_11C[playerIndex].motionTransitionTimer = 10;
                }
                Hu3DModelAttrReset(lbl_1_bss_11C[playerIndex].modelPart0,
                                   HU3D_MOTATTR_LOOP | HU3D_MOTATTR_REV);
                Hu3DModelAttrSet(lbl_1_bss_11C[playerIndex].modelPart0, HU3D_MOTATTR_PAUSE);
            }
        } else if (((s32) (lbl_1_bss_11C[playerIndex].buttons & PAD_BUTTON_A) == 0) &&
                   ((s32) (lbl_1_bss_11C[playerIndex].buttons & PAD_BUTTON_B) == 0)) {
            if (lbl_1_bss_11C[playerIndex].motionState != 1) {
                fn_1_3C80(playerIndex, 1);
                lbl_1_bss_11C[playerIndex].motionTransitionTimer = 10;
            }
            Hu3DModelAttrReset(lbl_1_bss_11C[playerIndex].modelPart0,
                               HU3D_MOTATTR_LOOP | HU3D_MOTATTR_REV);
            Hu3DModelAttrSet(lbl_1_bss_11C[playerIndex].modelPart0, HU3D_MOTATTR_PAUSE);
        }
        if ((s32) (lbl_1_bss_11C[playerIndex].buttons & PAD_BUTTON_A) != 0) {
            lbl_1_bss_11C[playerIndex].displacement.x = lbl_1_bss_11C[playerIndex].displacement.y =
                0.0f;
            lbl_1_bss_11C[playerIndex].displacement.z = 2.5f;

            fn_1_3FFC(lbl_1_bss_11C[playerIndex].rotation.x, lbl_1_bss_11C[playerIndex].rotation.y,
                      lbl_1_bss_11C[playerIndex].rotation.z,
                      lbl_1_bss_11C[playerIndex].displacement,
                      &lbl_1_bss_11C[playerIndex].displacement);
            if (lbl_1_bss_11C[playerIndex].motionState != 2) {
                fn_1_3C80(playerIndex, 2);
                lbl_1_bss_11C[playerIndex].motionTransitionTimer = 10;
            }
            Hu3DModelAttrReset(lbl_1_bss_11C[playerIndex].modelPart0,
                               HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_REV);
            Hu3DModelAttrSet(lbl_1_bss_11C[playerIndex].modelPart0, HU3D_MOTATTR_LOOP);
            lbl_1_bss_11C[playerIndex].drawingActive = 1;
            if ((s32) lbl_1_bss_FC[playerIndex] < 0) {
                if (playerIndex == 0) {
                    lbl_1_bss_FC[playerIndex] = HuAudFXPlay(MSM_SE_MOWTOWN_MOVE_PLAYER_0);
                }
                if (playerIndex == 1) {
                    lbl_1_bss_FC[playerIndex] = HuAudFXPlay(MSM_SE_MOWTOWN_MOVE_PLAYER_1);
                }
                if (playerIndex == 2) {
                    lbl_1_bss_FC[playerIndex] = HuAudFXPlay(MSM_SE_MOWTOWN_MOVE_PLAYER_2);
                }
                if (playerIndex == 3) {
                    lbl_1_bss_FC[playerIndex] = HuAudFXPlay(MSM_SE_MOWTOWN_MOVE_PLAYER_3);
                }
            }
        } else {
            if ((s32) (lbl_1_bss_11C[playerIndex].buttons & PAD_BUTTON_B) != 0) {
                lbl_1_bss_11C[playerIndex].displacement.x =
                    lbl_1_bss_11C[playerIndex].displacement.y = 0.0f;
                lbl_1_bss_11C[playerIndex].displacement.z = -2.5f;

                fn_1_3FFC(
                    lbl_1_bss_11C[playerIndex].rotation.x, lbl_1_bss_11C[playerIndex].rotation.y,
                    lbl_1_bss_11C[playerIndex].rotation.z, lbl_1_bss_11C[playerIndex].displacement,
                    &lbl_1_bss_11C[playerIndex].displacement);
                if (lbl_1_bss_11C[playerIndex].motionState != 3) {
                    fn_1_3C80(playerIndex, 3);
                    lbl_1_bss_11C[playerIndex].motionTransitionTimer = 10;
                }
                Hu3DModelAttrReset(lbl_1_bss_11C[playerIndex].modelPart0,
                                   HU3D_MOTATTR_LOOP | HU3D_MOTATTR_PAUSE);
                Hu3DModelAttrSet(lbl_1_bss_11C[playerIndex].modelPart0, HU3D_MOTATTR_REV);
                if ((s32) lbl_1_bss_FC[playerIndex] > 0) {
                    HuAudFXStop(lbl_1_bss_FC[playerIndex]);
                }
                lbl_1_bss_FC[playerIndex] = -1;
                return;
            }
            if ((s32) lbl_1_bss_FC[playerIndex] > 0) {
                HuAudFXStop(lbl_1_bss_FC[playerIndex]);
            }
            lbl_1_bss_FC[playerIndex] = -1;
        }
    }
    goto m613TickDownEnd;
m613TickDown:
    {
        lbl_1_bss_11C[playerIndex].motionTransitionTimer -= 1;
        if (lbl_1_bss_11C[playerIndex].motionTransitionTimer <= 0) {
            lbl_1_bss_11C[playerIndex].motionTransitionTimer = 0;
        }
        Hu3DModelAttrReset(lbl_1_bss_11C[playerIndex].modelPart0,
                           HU3D_MOTATTR_LOOP | HU3D_MOTATTR_REV);
        Hu3DModelAttrSet(lbl_1_bss_11C[playerIndex].modelPart0, HU3D_MOTATTR_PAUSE);
        lbl_1_bss_11C[playerIndex].drawingActive = 0;
    }
m613TickDownEnd:
    ;
}

/* Updates player models during the intro, play, and results. Drawing emits marks except in
* sequence states five and six; applied movement and input are cleared each update. */
void fn_1_B4A8(void) {
    Point3d corners[4];
    Point3d effectTarget;
    s32 cleared;
    s16 player;

    for (player = 0; player < 4; player++) {
        if (lbl_1_bss_11C[player].motionState == 2 || lbl_1_bss_11C[player].motionState == 3) {
            CharMotionSpeedSet(lbl_1_bss_11C[player].characterNo, 1.5f);
        } else {
            CharMotionSpeedSet(lbl_1_bss_11C[player].characterNo, 1.0f);
        }
        {
            Point3d source = lbl_1_bss_11C[player].firstObjectPosition;
            m613RotateVector(lbl_1_bss_11C[player].rotation.x, lbl_1_bss_11C[player].rotation.y,
                             lbl_1_bss_11C[player].rotation.z, &source, &corners[0]);
        }
        PSVECAdd(&lbl_1_bss_11C[player].position, &corners[0], &corners[0]);
        {
            Point3d source = lbl_1_bss_11C[player].secondObjectPosition;
            m613RotateVector(lbl_1_bss_11C[player].rotation.x, lbl_1_bss_11C[player].rotation.y,
                             lbl_1_bss_11C[player].rotation.z, &source, &corners[1]);
        }
        PSVECAdd(&lbl_1_bss_11C[player].position, &corners[1], &corners[1]);
        lbl_1_bss_11C[player].position.x += lbl_1_bss_11C[player].displacement.x;
        lbl_1_bss_11C[player].position.y += lbl_1_bss_11C[player].displacement.y;
        lbl_1_bss_11C[player].position.z += lbl_1_bss_11C[player].displacement.z;
        lbl_1_bss_11C[player].rotation.x += lbl_1_bss_11C[player].rotationOffset.x;
        lbl_1_bss_11C[player].rotation.y += lbl_1_bss_11C[player].rotationOffset.y;
        lbl_1_bss_11C[player].rotation.z += lbl_1_bss_11C[player].rotationOffset.z;
        Hu3DModelPosSetV(lbl_1_bss_11C[player].modelPart0, &lbl_1_bss_11C[player].position);
        Hu3DModelRotSetV(lbl_1_bss_11C[player].modelPart0, &lbl_1_bss_11C[player].rotation);
        Hu3DModelPosSetV(lbl_1_bss_11C[player].modelPart1, &lbl_1_bss_11C[player].position);
        Hu3DModelRotSetV(lbl_1_bss_11C[player].modelPart1, &lbl_1_bss_11C[player].rotation);
        if (lbl_1_bss_11C[player].resultPositionMode == 0) {
            fn_1_3EB0(player);
        } else {
            Hu3DModelPosSetV(lbl_1_bss_11C[player].model, &lbl_1_bss_11C[player].position);
        }
        Hu3DModelRotSetV(lbl_1_bss_11C[player].model, &lbl_1_bss_11C[player].rotation);
        {
            Point3d source = lbl_1_bss_11C[player].secondObjectPosition;
            m613RotateVector(lbl_1_bss_11C[player].rotation.x, lbl_1_bss_11C[player].rotation.y,
                             lbl_1_bss_11C[player].rotation.z, &source, &corners[2]);
        }
        PSVECAdd(&lbl_1_bss_11C[player].position, &corners[2], &corners[2]);
        {
            Point3d source = lbl_1_bss_11C[player].firstObjectPosition;
            m613RotateVector(lbl_1_bss_11C[player].rotation.x, lbl_1_bss_11C[player].rotation.y,
                             lbl_1_bss_11C[player].rotation.z, &source, &corners[3]);
        }
        PSVECAdd(&lbl_1_bss_11C[player].position, &corners[3], &corners[3]);
        lbl_1_bss_11C[player].rotationOffset.x = lbl_1_bss_11C[player].rotationOffset.y =
            lbl_1_bss_11C[player].rotationOffset.z = 0.0f;
        lbl_1_bss_11C[player].displacement.x = lbl_1_bss_11C[player].displacement.y =
            lbl_1_bss_11C[player].displacement.z = 0.0f;
        lbl_1_bss_11C[player].stickX = lbl_1_bss_11C[player].stickY = 0;
        lbl_1_bss_11C[player].buttons = lbl_1_bss_11C[player].buttonsDown = 0;
        if (lbl_1_bss_11C[player].drawingActive != 0 &&
            (lbl_1_bss_11C[player].sequenceState < 5 || lbl_1_bss_11C[player].sequenceState > 6)) {
            Point3d first;
            Point3d second;
            Point3d third;
            Point3d fourth;
            Point3d *fourthPoint;
            Point3d *thirdPoint;
            Point3d *secondPoint;
            Point3d *firstPoint;
            fourth = corners[3];
            fourthPoint = &fourth;
            third = corners[2];
            thirdPoint = &third;
            second = corners[1];
            secondPoint = &second;
            first = corners[0];
            firstPoint = &first;
            cleared = fn_1_20D0(firstPoint, secondPoint, thirdPoint, fourthPoint);
            lbl_1_bss_11C[player].clearedPixelCount += cleared;
            while (lbl_1_bss_11C[player].clearedPixelCount >= 90) {
                Point3d corner1;
                Point3d corner2;
                Point3d corner3;
                Point3d corner4;
                Point3d origin;
                Point3d targetPosition;
                Point3d *targetPoint;
                Point3d *originPoint;
                corner4 = corners[3];
                corner3 = corners[2];
                corner2 = corners[1];
                corner1 = corners[0];
                /* Each emission replaces the first quad corner with its sampled source; another
                 * emission in this loop samples from that changed quad. */
                fn_1_40BC(&corner1, &corner2, &corner3, &corner4, &corners[0]);
                fn_1_F6E0(player, &effectTarget);
                targetPosition = effectTarget;
                targetPoint = &targetPosition;
                origin = corners[0];
                originPoint = &origin;
                fn_1_1054C(originPoint, targetPoint, player, 90);
                lbl_1_bss_11C[player].clearedPixelCount -= 90;
            }
        }
    }
}

/* Returns every player to the idle motion and stops their movement sounds after play. */
void fn_1_C2AC(void)
{
    s16 playerIndex = 0;

    while (playerIndex < 4) {
        s16 motionIndex;

        if ((playerIndex >= 0) && (playerIndex < 4) &&
            (lbl_1_bss_11C[playerIndex].motionState != 1)) {
            motionIndex = lbl_1_data_FC[1].motionIndex;
            if (motionIndex != -1) {
                if (lbl_1_data_FC[1].duration <= 1.0f) {
                    CharMotionSet(
                        lbl_1_bss_11C[playerIndex].characterNo,
                        lbl_1_bss_11C[playerIndex].motionIds[motionIndex]);
                    Hu3DModelAttrReset(
                        lbl_1_bss_11C[playerIndex].model,
                        Hu3DModelMotionAttrGet(
                            lbl_1_bss_11C[playerIndex].model));
                    CharModelAttrSet(
                        lbl_1_bss_11C[playerIndex].characterNo,
                        lbl_1_data_FC[1].attributes);
                } else {
                    CharMotionShiftSet(
                        lbl_1_bss_11C[playerIndex].characterNo,
                        lbl_1_bss_11C[playerIndex].motionIds[motionIndex],
                        0.0f, lbl_1_data_FC[1].duration,
                        lbl_1_data_FC[1].attributes);
                }
            }
            lbl_1_bss_11C[playerIndex].motionState = 1;
        }
        if (lbl_1_bss_FC[playerIndex] > 0) {
            HuAudFXStop(lbl_1_bss_FC[playerIndex]);
        }
        lbl_1_bss_FC[playerIndex] = -1;
        playerIndex++;
    }
}

/* Awards 10 minigame bonus coins outside practice mode and stores this player's character slot. */
static inline void m613AwardPlayer(s16 playerIndex, s16 *winnerCharacter) {
    s32 player = lbl_1_bss_11C[playerIndex].playerIndex;
    if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
        GwPlayer[player].mgCoinBonus = 10;
    }
    *winnerCharacter = lbl_1_bss_11C[playerIndex].characterNo;
}

/* Resets all players to idle, awards mask-selected winners 10 bonus coins outside practice mode,
 * and stores the winner slots. */
void fn_1_C4E4(s16 awardMask) {
    s16 winnerChars[4];
    s16 i;

    for (i = 0; i < 4; i++) {
        winnerChars[i] = -1;
        fn_1_3C80(i, 1);
    }

    if ((awardMask & 1) != 0) {
        m613AwardPlayer(0, &winnerChars[0]);
    }
    if ((awardMask & 2) != 0) {
        m613AwardPlayer(1, &winnerChars[1]);
    }
    if ((awardMask & 4) != 0) {
        m613AwardPlayer(2, &winnerChars[2]);
    }
    if ((awardMask & 8) != 0) {
        m613AwardPlayer(3, &winnerChars[3]);
    }
    MgSeqWinnerSet(winnerChars[0], winnerChars[1], winnerChars[2], winnerChars[3]);
}

/* Advances the result sequence through its outgoing wipe, winner setup, and return wipe. */
s32 fn_1_C850(s16 frameNo) {
    s16 i;
    i = 0;
    switch (lbl_1_bss_11C[i].sequenceState)
    {
    case 2 :
        for (i = 0; i < 4; i++)
        {
            lbl_1_bss_11C[i].sequenceState = 3;
        }
        break;
    case 3 :
        for (i = 0; i < 4; i++)
        {
            HuAudFXStop(lbl_1_bss_10C[i]);
        }
        WipeCreate(2, 6, 60);
        for (i = 0; i < 4; i++)
        {
            lbl_1_bss_11C[i].sequenceState = 4;
        }
        break;
    case 4 :
        if (WipeCheck() == 0)
        {
            lbl_1_bss_125C = fn_1_FA48();
            fn_1_C4E4(lbl_1_bss_125C);
            fn_1_F2A4();
            fn_1_FC40();
            fn_1_1A7C();
            fn_1_106C0();
            for (i = 0; i < 4; i++)
            {
                lbl_1_bss_11C[i].sequenceState = 5;
                lbl_1_bss_11C[i].resultPositionMode = 1;
                Hu3DModelAttrSet(lbl_1_bss_11C[i].modelPart0, 1);
                Hu3DModelAttrSet(lbl_1_bss_11C[i].modelPart1, 1);
                lbl_1_bss_11C[i].position = lbl_1_data_BC[i];
                lbl_1_bss_11C[i].rotation.y = 0.0f;
                fn_1_3C80(i, 0);
            }
            WipeCreate(1, 6, 60);
            lbl_1_bss_F8 = - 1;
        }
        break;
    case 5 :
        if (WipeCheck() == 0)
        {
            if (lbl_1_bss_F8 > 0)
            {
                lbl_1_bss_F8--;
                if (lbl_1_bss_F8 <= 0)
                {
                    for (i = 0; i < 4; i++)
                    {
                        lbl_1_bss_11C[i].sequenceState = 6;
                    }
                }
            }
            else
            {
                lbl_1_bss_F8 = 50;
            }
        }
        break;
    case 6 :
        for (i = 0; i < 4; i++)
        {
            lbl_1_bss_11C[i].sequenceState = 6;
        }
        return 1;
    default :
        for (i = 0; i < 4; i++)
        {
            lbl_1_bss_11C[i].sequenceState = 2;
        }
        break;
    }
    fn_1_B4A8();
    return 0;
}

/* Selects each player's win or lose motion during the result sequence. */
void fn_1_D0C8(void) {
    s16 playerIndex;
    s16 winMotionIndex;
    s16 loseMotionIndex;
    f32 motionEndTime;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {

        if ((lbl_1_bss_11C[playerIndex].motionState != 4) &&
            (lbl_1_bss_11C[playerIndex].motionState != 5)) {
            if ((lbl_1_bss_125C & (1 << playerIndex)) != 0) {
                if ((playerIndex >= 0) && (playerIndex < 4) &&
                    (lbl_1_bss_11C[playerIndex].motionState != 4)) {
                    winMotionIndex = lbl_1_data_FC[4].motionIndex;
                    if (winMotionIndex != -1) {
                        if (lbl_1_data_FC[4].duration <= 1.0f) {
                            CharMotionSet(lbl_1_bss_11C[playerIndex].characterNo,
                                          lbl_1_bss_11C[playerIndex].motionIds[winMotionIndex]);
                            Hu3DModelAttrReset(
                                lbl_1_bss_11C[playerIndex].model,
                                Hu3DModelMotionAttrGet(lbl_1_bss_11C[playerIndex].model));
                            CharModelAttrSet(lbl_1_bss_11C[playerIndex].characterNo,
                                             lbl_1_data_FC[4].attributes);
                        } else {
                            CharMotionShiftSet(lbl_1_bss_11C[playerIndex].characterNo,
                                               lbl_1_bss_11C[playerIndex].motionIds[winMotionIndex],
                                               0.0f,
                                               lbl_1_data_FC[4].duration,
                                               lbl_1_data_FC[4].attributes);
                        }
                    }
                    lbl_1_bss_11C[playerIndex].motionState = 4;
                }
                lbl_1_bss_11C[playerIndex].rotation.y = 0.0f;
            } else {
                if ((playerIndex >= 0) && (playerIndex < 4) &&
                    (lbl_1_bss_11C[playerIndex].motionState != 5)) {
                    loseMotionIndex = lbl_1_data_FC[5].motionIndex;
                    if (loseMotionIndex != -1) {
                        if (lbl_1_data_FC[5].duration <= 1.0f) {
                            CharMotionSet(lbl_1_bss_11C[playerIndex].characterNo,
                                          lbl_1_bss_11C[playerIndex].motionIds[loseMotionIndex]);
                            Hu3DModelAttrReset(
                                lbl_1_bss_11C[playerIndex].model,
                                Hu3DModelMotionAttrGet(lbl_1_bss_11C[playerIndex].model));
                            CharModelAttrSet(lbl_1_bss_11C[playerIndex].characterNo,
                                             lbl_1_data_FC[5].attributes);
                        } else {
                            CharMotionShiftSet(
                                lbl_1_bss_11C[playerIndex].characterNo,
                                lbl_1_bss_11C[playerIndex].motionIds[loseMotionIndex], 0.0f,
                                lbl_1_data_FC[5].duration, lbl_1_data_FC[5].attributes);
                        }
                    }
                    lbl_1_bss_11C[playerIndex].motionState = 5;
                }
                lbl_1_bss_11C[playerIndex].rotation.y = 0.0f;
            }
            continue;
        }
        if ((lbl_1_bss_11C[playerIndex].motionState == 4) ||
            (lbl_1_bss_11C[playerIndex].motionState == 5)) {
            if (CharMotionShiftIDGet(lbl_1_bss_11C[playerIndex].characterNo) < 0) {
                motionEndTime = CharMotionMaxTimeGet(lbl_1_bss_11C[playerIndex].characterNo);
                if ((CharMotionTimeGet(lbl_1_bss_11C[playerIndex].characterNo) +
                     Hu3DData[lbl_1_bss_11C[playerIndex].model].motWork.speed) >= motionEndTime) {
                    CharModelAttrSet(lbl_1_bss_11C[playerIndex].characterNo, HU3D_MOTATTR_PAUSE);
                }
            }
        }
    }
    fn_1_B4A8();
}

/* Reads and scales each player's pad stick and button state during input updates. */
void fn_1_D61C(void) {
    s16 playerIndex;
    s16 xDeadzone;
    s16 yDeadzone;

    xDeadzone = 6;
    yDeadzone = 6;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        lbl_1_bss_11C[playerIndex].stickX =
            lbl_1_bss_11C[playerIndex].stickY = 0;
        lbl_1_bss_11C[playerIndex].buttons =
            lbl_1_bss_11C[playerIndex].buttonsDown = 0;
        lbl_1_bss_11C[playerIndex].stickX =
            HuPadStkX[lbl_1_bss_11C[playerIndex].padNo];
        if (-xDeadzone < lbl_1_bss_11C[playerIndex].stickX &&
            lbl_1_bss_11C[playerIndex].stickX < xDeadzone) {
            lbl_1_bss_11C[playerIndex].stickX = 0;
        }
        lbl_1_bss_11C[playerIndex].stickX =
            (s32)(lbl_1_bss_11C[playerIndex].stickX /
                      5.6f);
        lbl_1_bss_11C[playerIndex].stickX *= 5;
        if (lbl_1_bss_11C[playerIndex].stickX > 56) {
            lbl_1_bss_11C[playerIndex].stickX = 56;
        }
        if (lbl_1_bss_11C[playerIndex].stickX < -56) {
            lbl_1_bss_11C[playerIndex].stickX = -56;
        }
        lbl_1_bss_11C[playerIndex].stickY =
            HuPadStkY[lbl_1_bss_11C[playerIndex].padNo];
        if (-yDeadzone < lbl_1_bss_11C[playerIndex].stickY &&
            lbl_1_bss_11C[playerIndex].stickY < yDeadzone) {
            lbl_1_bss_11C[playerIndex].stickY = 0;
        }
        lbl_1_bss_11C[playerIndex].stickY =
            (s32)(lbl_1_bss_11C[playerIndex].stickY /
                      5.6f);
        lbl_1_bss_11C[playerIndex].stickY *= 5;
        if (lbl_1_bss_11C[playerIndex].stickY > 56) {
            lbl_1_bss_11C[playerIndex].stickY = 56;
        }
        if (lbl_1_bss_11C[playerIndex].stickY < -56) {
            lbl_1_bss_11C[playerIndex].stickY = -56;
        }
        lbl_1_bss_11C[playerIndex].buttons =
            HuPadBtn[lbl_1_bss_11C[playerIndex].padNo];
        lbl_1_bss_11C[playerIndex].buttonsDown =
            HuPadBtnDown[lbl_1_bss_11C[playerIndex].padNo];
    }
}

/* Initializes target state and seeds movement and turn timers for all four players during setup. */
void fn_1_DAE8(void) {
    s16 playerIndex;

    playerIndex = 0;
    while (playerIndex < 4) {
        lbl_1_bss_11C[playerIndex].hasTarget = 0;
        lbl_1_bss_11C[playerIndex].cpuCollisionDelay = 0;
        lbl_1_bss_11C[playerIndex].movementTime = (s16) (frandmod(60) * -1);
        if (lbl_1_bss_11C[playerIndex].movementTime == 0) {
            lbl_1_bss_11C[playerIndex].movementTime = -1;
        }
        lbl_1_bss_11C[playerIndex].turnTime = 0;
        playerIndex += 1;
    }
}

/* Clamps playerIndex and samples a difficulty-based collision delay on every call, storing it only
 * when the previous value is nonpositive. CPU turn phases clear this value; it is never counted
 * down or used to steer. */
void fn_1_DBDC(s16 playerIndex)
{
    s16 delayFrames = 0;

    if (playerIndex < 0) {
        playerIndex = 0;
    }
    if (playerIndex >= 4) {
        playerIndex = 3;
    }
    switch (lbl_1_bss_11C[playerIndex].cpuDifficulty) {
    case 0:
        delayFrames = frandmod(60) + 60;
        break;
    case 1:
        delayFrames = frandmod(45) + 45;
        break;
    case 2:
        delayFrames = frandmod(30) + 30;
        break;
    case 3:
        delayFrames = frandmod(30) + 30;
        break;
    }
    if (lbl_1_bss_11C[playerIndex].cpuCollisionDelay <= 0) {
        lbl_1_bss_11C[playerIndex].cpuCollisionDelay = delayFrames;
    }
}

/* Starts the arena-contact counter during collision handling, clamping playerIndex to 0..3.
 * An active counter keeps its value; CPU steering changes the target on the first count. */
void fn_1_DCF4(s16 playerIndex) {
    if (playerIndex < 0) {
        playerIndex = 0;
    }
    if (playerIndex >= 4) {
        playerIndex = 3;
    }
    if (lbl_1_bss_11C[playerIndex].recoveryTime == 0) {
        lbl_1_bss_11C[playerIndex].recoveryTime = 1;
    }
}

/* Replaces human input with CPU steering for each computer-controlled player. */
void fn_1_DD54(void)
{
    s16 playerIndex = 0;

    while (playerIndex < 4) {
        if (lbl_1_bss_11C[playerIndex].playerType != 0) {
            lbl_1_bss_11C[playerIndex].stickX =
                lbl_1_bss_11C[playerIndex].stickY = 0;
            lbl_1_bss_11C[playerIndex].buttons =
                lbl_1_bss_11C[playerIndex].buttonsDown = 0;
            fn_1_DE20(playerIndex);
        }
        playerIndex++;
    }
}

/* Chooses CPU targets, steers around nearby players, and advances its movement timers. */
void fn_1_DE20(s16 playerIndex) {
    Point3d direction;
    Point3d nearestDisplacement;
    Point3d candidateDisplacement;
    f32 distanceLimitOrHeading;
    f32 distanceProjectionOrAngle;
    s16 firstRandomValue;
    s16 secondRandomValue;
    s16 firstMotionTimer;
    s16 secondMotionTimer;
    s16 minimumCellReferences;
    s16 cellReferencesOrPlayerIndex;

    /* Difficulties zero through two sample both durations every update, even when neither timer
     * needs a new duration. */
    switch (lbl_1_bss_11C[playerIndex].cpuDifficulty) {
    case 0:
        firstMotionTimer = frandmod(30) + 165;
        secondMotionTimer = frandmod(30) + 105;
        minimumCellReferences = 0;
        break;
    case 1:
        firstMotionTimer = frandmod(30) + 75;
        secondMotionTimer = frandmod(30) + 195;
        minimumCellReferences = 100;
        break;
    case 2:
        firstMotionTimer = frandmod(30) + 16;
        secondMotionTimer = frandmod(30) + 255;
        minimumCellReferences = 200;
        break;
    case 3:
        firstMotionTimer = 0;
        secondMotionTimer = 300;
        minimumCellReferences = 400;
        break;
    }
    if (lbl_1_bss_11C[playerIndex].hasTarget == 0) {
        s16 secondRange = fn_1_1D84();
        s16 firstRange = fn_1_1D7C();
        firstRandomValue = frandmod(secondRange);
        secondRandomValue = frandmod(firstRange);
        cellReferencesOrPlayerIndex = fn_1_1EF4(secondRandomValue, firstRandomValue);
        if (cellReferencesOrPlayerIndex > minimumCellReferences) {
            fn_1_1F54(secondRandomValue, firstRandomValue, &direction);
            PSVECSubtract(&direction, &lbl_1_bss_11C[playerIndex].position, &nearestDisplacement);
            distanceProjectionOrAngle = (nearestDisplacement.z * nearestDisplacement.z) +
                ((nearestDisplacement.x * nearestDisplacement.x) +
                 (nearestDisplacement.y * nearestDisplacement.y));
            if (distanceProjectionOrAngle >= 10000.0f) {
                lbl_1_bss_11C[playerIndex].targetPosition = direction;
                lbl_1_bss_11C[playerIndex].hasTarget = 1;
            }
        }
    }
    if (lbl_1_bss_11C[playerIndex].hasTarget != 0) {
        distanceProjectionOrAngle = 2000000.0f;
        for (cellReferencesOrPlayerIndex = 0; cellReferencesOrPlayerIndex < 4;
             cellReferencesOrPlayerIndex++) {
            if (playerIndex != cellReferencesOrPlayerIndex) {
                PSVECSubtract(&lbl_1_bss_11C[cellReferencesOrPlayerIndex].position,
                              &lbl_1_bss_11C[playerIndex].position, &candidateDisplacement);
                distanceLimitOrHeading = PSVECSquareMag(&candidateDisplacement);
                if (distanceProjectionOrAngle >= distanceLimitOrHeading) {
                    distanceProjectionOrAngle = distanceLimitOrHeading;
                    nearestDisplacement = candidateDisplacement;
                }
            }
        }
        distanceProjectionOrAngle = PSVECSquareMag(&nearestDisplacement);
        distanceLimitOrHeading = PSVECMag(&lbl_1_bss_11C[playerIndex].hookPosition);
        distanceLimitOrHeading += lbl_1_bss_11C[playerIndex].hookRadius;
        distanceLimitOrHeading *= distanceLimitOrHeading;
        distanceLimitOrHeading *= 2.0f;
        if (distanceProjectionOrAngle < distanceLimitOrHeading) {
            direction.x = direction.y = 0.0f;
            direction.z = 1.0f;

            fn_1_3FFC(0.0f, lbl_1_bss_11C[playerIndex].rotation.y, 0.0f, direction, &direction);
            fn_1_10C5C(&direction, &direction);
            candidateDisplacement.x = 1.0f;
            candidateDisplacement.y = candidateDisplacement.z = 0.0f;

            fn_1_3FFC(0.0f, lbl_1_bss_11C[playerIndex].rotation.y, 0.0f, candidateDisplacement,
                      &candidateDisplacement);
            distanceProjectionOrAngle =
                PSVECDotProduct(&candidateDisplacement, &nearestDisplacement);
            if (distanceProjectionOrAngle < 0.0f) {

                fn_1_3FFC(0.0f, 90.0f, 0.0f, direction, &direction);
            } else {

                fn_1_3FFC(0.0f, -90.0f, 0.0f, direction, &direction);
            }
        } else {
            PSVECSubtract(&lbl_1_bss_11C[playerIndex].targetPosition,
                          &lbl_1_bss_11C[playerIndex].position, &direction);
            fn_1_10C5C(&direction, &direction);
        }
        lbl_1_bss_11C[playerIndex].stickY = (s8) (56.0f * -direction.z);
        lbl_1_bss_11C[playerIndex].stickX = (s8)(56.0f * direction.x);
        nearestDisplacement.x = nearestDisplacement.y = 0.0f;
        nearestDisplacement.z = 1.0f;

        fn_1_3FFC(0.0f, lbl_1_bss_11C[playerIndex].rotation.y, 0.0f, nearestDisplacement,
                  &nearestDisplacement);
        fn_1_10C5C(&nearestDisplacement, &nearestDisplacement);
        /* Compare heading degrees directly; the absolute difference is not wrapped across zero. */
        distanceProjectionOrAngle = (f32)fn_1_10E3C((f64)direction.x, (f64)direction.z);
        distanceLimitOrHeading =
            (f32) fn_1_10E3C((f64) nearestDisplacement.x, (f64) nearestDisplacement.z);
        distanceProjectionOrAngle -= distanceLimitOrHeading;
        if (distanceProjectionOrAngle < 0.0f) {
            distanceProjectionOrAngle *= -1.0f;
        }
        if (distanceProjectionOrAngle < 10.0f) {
            lbl_1_bss_11C[playerIndex].stickY = lbl_1_bss_11C[playerIndex].stickX = 0;
        }
        lbl_1_bss_11C[playerIndex].buttons = PAD_BUTTON_A;
        PSVECSubtract(&lbl_1_bss_11C[playerIndex].targetPosition,
                      &lbl_1_bss_11C[playerIndex].position, &direction);
        distanceProjectionOrAngle = PSVECSquareMag(&direction);
        distanceLimitOrHeading = lbl_1_bss_11C[playerIndex].radius *
            lbl_1_bss_11C[playerIndex].radius;
        if (distanceProjectionOrAngle < distanceLimitOrHeading) {
            lbl_1_bss_11C[playerIndex].hasTarget = 0;
        }
    }
    if (lbl_1_bss_11C[playerIndex].movementTime < 0) {
        lbl_1_bss_11C[playerIndex].movementTime++;
        if (lbl_1_bss_11C[playerIndex].movementTime >= 0) {
            lbl_1_bss_11C[playerIndex].movementTime = firstMotionTimer;
            lbl_1_bss_11C[playerIndex].turnTime = 0;
        }
    } else if (lbl_1_bss_11C[playerIndex].movementTime > 0) {
        lbl_1_bss_11C[playerIndex].movementTime--;
        if (lbl_1_bss_11C[playerIndex].movementTime > 0) {
            if (lbl_1_bss_11C[playerIndex].turnTime == 0) {
                lbl_1_bss_11C[playerIndex].turnTime = frandmod(10) - 5;
                if (lbl_1_bss_11C[playerIndex].turnTime < 0) {
                    lbl_1_bss_11C[playerIndex].turnTime -= 45;
                } else {
                    lbl_1_bss_11C[playerIndex].turnTime += 45;
                }
            } else {
                direction.x = direction.y = 0.0f;
                direction.z = 1.0f;
                if (lbl_1_bss_11C[playerIndex].turnTime < 0) {

                    fn_1_3FFC(0.0f, 45.0f + lbl_1_bss_11C[playerIndex].rotation.y,
                                       0.0f, direction, &nearestDisplacement);
                    fn_1_10C5C(&nearestDisplacement, &nearestDisplacement);
                    lbl_1_bss_11C[playerIndex].stickX = (s8)(56.0f * nearestDisplacement.x);
                    lbl_1_bss_11C[playerIndex].stickY =
                        (s8) (-1.0f * (56.0f * nearestDisplacement.z));
                    lbl_1_bss_11C[playerIndex].turnTime++;
                    if (lbl_1_bss_11C[playerIndex].turnTime > 0) {
                        lbl_1_bss_11C[playerIndex].turnTime = 0;
                    }
                } else {

                    fn_1_3FFC(0.0f, lbl_1_bss_11C[playerIndex].rotation.y - 45.0f,
                                       0.0f, direction, &candidateDisplacement);
                    fn_1_10C5C(&candidateDisplacement, &candidateDisplacement);
                    lbl_1_bss_11C[playerIndex].stickX = (s8)(56.0f * candidateDisplacement.x);
                    lbl_1_bss_11C[playerIndex].stickY =
                        (s8) (-1.0f * (56.0f * candidateDisplacement.z));
                    lbl_1_bss_11C[playerIndex].turnTime--;
                    if (lbl_1_bss_11C[playerIndex].turnTime < 0) {
                        lbl_1_bss_11C[playerIndex].turnTime = 0;
                    }
                }
            }
            lbl_1_bss_11C[playerIndex].cpuCollisionDelay = 0;
            lbl_1_bss_11C[playerIndex].hasTarget = 0;
            lbl_1_bss_11C[playerIndex].buttons = 0;
            lbl_1_bss_11C[playerIndex].buttonsDown = 0;
        } else {
            lbl_1_bss_11C[playerIndex].movementTime = -secondMotionTimer;
        }
    }
    /* On the first count after arena contact, mirror and shrink the target about the origin without
     * setting hasTarget; the counter clears at 60. */
    if (lbl_1_bss_11C[playerIndex].recoveryTime != 0) {
        if (lbl_1_bss_11C[playerIndex].recoveryTime == 1) {
            lbl_1_bss_11C[playerIndex].targetPosition.x *= -0.8f;
            lbl_1_bss_11C[playerIndex].targetPosition.y *= -0.8f;
            lbl_1_bss_11C[playerIndex].targetPosition.z *= -0.8f;
        }
        lbl_1_bss_11C[playerIndex].recoveryTime++;
        if (lbl_1_bss_11C[playerIndex].recoveryTime >= 60) {
            lbl_1_bss_11C[playerIndex].recoveryTime = 0;
        }
    }
}
