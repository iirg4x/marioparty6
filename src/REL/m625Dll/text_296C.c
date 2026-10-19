/* Player models, movement, minigame rules, computer input, and water particles. */
#include "dolphin/math.h"
#include "REL/m625Dll/players.h"
#include "game/main.h"
#include "game/pad.h"
#include "dolphin/types.h"
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
#include "game/hu3d.h"

#define M625_SE_PLAYER_START 1796
#define M625_SE_SWIM_START 1800
#define M625_SE_TIMER_TICK 1801

extern s32 frandmod(s32 modulus);
extern u32 frand(void);
extern s16 fn_1_12700(void);
extern s16 fn_1_12760(void);
extern s32 fn_1_12768(s16 obstacleIndex);
extern void fn_1_127B0(s16 obstacleIndex, Point3d *position);
extern s32 fn_1_13AA8(f32 maxY, f32 minY, f32 minX, f32 maxX, Point3d position);
extern void fn_1_11040(void);
extern s32 fn_1_12254(Point3d position, Point3d *movement, f32 radius);
extern s32 fn_1_137DC(Point3d position, Point3d rotation, Point3d *movement,
    f32 radius, f32 height, Point3d otherPosition, Point3d otherRotation,
    Point3d *otherMovement, f32 otherRadius, f32 otherHeight);
extern s32 fn_1_13318(Point3d position, f32 radius, Point3d center, f32 centerRadius);
extern s32 fn_1_133A4(Point3d position, Point3d *movement, f32 radius,
    Point3d origin, Point3d normal, f32 amount);
extern void fn_1_11470(Point3d position);
extern void fn_1_12D7C(s16 playerIndex);
extern char *CharHeadObjNameTbl[];
extern s16 lbl_1_bss_814;
extern void fn_1_11698(void);
f32 fn_1_1C58(f32, f32);
void fn_1_11700(void);
void fn_1_12560(void);

void fn_1_296C(s16 index, s16 sequence);
void fn_1_2C60(s16 index, Point3d *result);
void fn_1_2D48(void);
s32 fn_1_3E8C(s16 player);
void fn_1_3F6C(s16 index, s16 frame);
void fn_1_4078(s16 index, s16 frame);
void fn_1_4E60(s16 mode, s16 frameNo);
void fn_1_4E80(void);
void fn_1_5108(void);
void fn_1_6CE0(s16 playerIndex);
void fn_1_860C(s16 playerIndex);
Point3d fn_1_B530(s16 playerIndex);
void fn_1_B958(s16 playerIndex);
void fn_1_BF54(void);
s16 fn_1_C19C(void);
void fn_1_C270(s16 mode);
s32 fn_1_C438(void);
void fn_1_DE68(void);
void fn_1_EB00(void);
void fn_1_EE94(void);
s16 fn_1_EF24(s16 playerIndex);
void fn_1_EF40(void);
void fn_1_EF54(void);
void fn_1_EFC4(void);
void fn_1_F07C(void);
void fn_1_F174(s16 playerIndex);
void fn_1_F900(s16 playerIndex);
void fn_1_10BD4(void);
void fn_1_10D74(HU3D_MODEL *model, HU3D_PARTICLE *particleSystem, Mtx matrix);
void fn_1_10E90(Point3d position, f32 scale, f32 randomVelocity, f32 verticalAcceleration);

static inline s16 M625NightModeGet(void)
{
    return GwMgNightF;
}

/* Selects a player's motions when setup or a player action update changes their pose. */
void fn_1_296C(s16 index, s16 sequence)
{
    s16 overlayMotionId;
    s16 motionId;

    if (index < 0 || index >= 4 || sequence < 0 || sequence >= 11) {
        return;
    }
    if (sequence != lbl_1_bss_1D8[index].motionSet) {
        motionId = lbl_1_data_2B8[sequence].motionId;
        overlayMotionId = lbl_1_data_2B8[sequence].overlayMotionId;
        if (motionId != -1) {
            if (lbl_1_data_2B8[sequence].blendTime <= 1.0f) {
                CharMotionSet(lbl_1_bss_1D8[index].characterNo,
                              lbl_1_bss_1D8[index].motionIds[motionId]);
                CharModelAttrSet(lbl_1_bss_1D8[index].characterNo,
                                 lbl_1_data_2B8[sequence].modelAttributes);
            } else {
                Hu3DModelAttrSet(lbl_1_bss_1D8[index].objectModel,
                HU3D_MOTATTR_PAUSE);
                CharMotionShiftSet(lbl_1_bss_1D8[index].characterNo,
                                   lbl_1_bss_1D8[index].motionIds[motionId],
                                   0.0f, lbl_1_data_2B8[sequence].blendTime,
                                   lbl_1_data_2B8[sequence].modelAttributes);
            }
        }
        if (overlayMotionId != -1) {
            Hu3DMotionOverlayReset(lbl_1_bss_1D8[index].objectModel);
            Hu3DMotionOverlaySet(lbl_1_bss_1D8[index].objectModel,
                                 lbl_1_bss_1D8[index].motionIds[overlayMotionId]);
            Hu3DModelAttrSet(lbl_1_bss_1D8[index].objectModel,
                             lbl_1_data_2B8[sequence].modelAttributes);
        } else {
            Hu3DMotionOverlayReset(lbl_1_bss_1D8[index].objectModel);
        }
        lbl_1_bss_1D8[index].motionSet = sequence;
    }
}

/* Builds the model rotation for fn_1_5108, adjusting the X angle for two rolled motion sets. */
void fn_1_2C60(s16 index, Point3d *result)
{
    Point3d motionRotation;

    motionRotation = lbl_1_bss_1D8[index].rotation;
    switch (lbl_1_bss_1D8[index].motionSet) {
    case 7:
        motionRotation.x = 90.0f + lbl_1_bss_1D8[index].rotation.x;
        break;
    case 9:
        motionRotation.x = lbl_1_bss_1D8[index].rotation.x - 90.0f;
        break;
    }
    *result = motionRotation;
}

u32 lbl_1_data_1E8[12] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mariomot, 2), DATANUM(DATA_mariomot, 6),
    DATANUM(DATA_mariomot, 7), DATANUM(DATA_mario, 43),
    DATANUM(DATA_mario, 136), DATANUM(DATA_mario, 33),
    DATANUM(DATA_mariomot, 26), DATANUM(DATA_mariomot, 27),
    DATANUM(DATA_mariomot, 34), DATANUM(DATA_mariomot, 25)
};

Point3d lbl_1_data_218[4] = {
    { 0.0f, 100.0f, 0.0f }, { -150.0f, -350.0f, 200.0f },
    { 1.0f, -350.0f, 200.0f }, { 150.0f, -350.0f, 200.0f }
};

Point3d lbl_1_data_248[4] = {
    { 0.0f, 100.0f, 0.0f }, { -190.0f, 0.0f, 200.0f },
    { 0.0f, 0.0f, 200.0f }, { 190.0f, 0.0f, 200.0f }
};

M625LightPreset lbl_1_data_278[2] = {
    { -1, { 0.0f, 1000.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 255, 255, 255, 255 } },
    { -1, { 0.0f, -3000.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 255, 255, 0, 255 } }
};

M625MotionPreset lbl_1_data_2B8[11] = {
    { 0, 5, 8.0f, HU3D_MOTATTR_OVL_LOOP | HU3D_MOTATTR_LOOP },
    { 1, 5, 8.0f, HU3D_MOTATTR_OVL_LOOP | HU3D_MOTATTR_LOOP },
    { 2, 5, 8.0f, HU3D_MOTATTR_OVL_LOOP | HU3D_MOTATTR_LOOP },
    { 6, -1, 0.0f, HU3D_MOTATTR_LOOP },
    { 3, -1, 8.0f, HU3D_MOTATTR_LOOP }, { 4, -1, 8.0f, HU3D_MOTATTR_LOOP },
    { 7, -1, 20.0f, HU3D_MOTATTR_LOOP }, { 8, -1, 20.0f, HU3D_MOTATTR_LOOP },
    { 9, -1, 20.0f, HU3D_MOTATTR_LOOP }, { 10, -1, 20.0f, HU3D_MOTATTR_LOOP },
    { 11, -1, 0.0f, HU3D_MOTATTR_LOOP }
};

/* Creates the four players and their models, lights and bubble effect during fn_1_13C04 setup. */
void fn_1_2D48(void)
{
    s16 nightMode;
    s16 teamIndex;
    s32 playerIndex;
    s32 motionIndex;

    nightMode = M625NightModeGet();
    lbl_1_bss_1D4 = 1;
    lbl_1_bss_1D0 = 1;
    lbl_1_bss_1CC = 0;
    teamIndex = 1;
    playerIndex = 0;
    while (playerIndex < 4) {
        lbl_1_bss_1D8[playerIndex].playerIndex = playerIndex;
        lbl_1_bss_1D8[playerIndex].characterNo = GwPlayerConf[playerIndex].charNo;
        lbl_1_bss_1D8[playerIndex].controllerIndex = GwPlayerConf[playerIndex].padNo;
        lbl_1_bss_1D8[playerIndex].computerPlayer = (s32)GwPlayerConf[playerIndex].type;
        lbl_1_bss_1D8[playerIndex].difficulty = GwPlayerConf[playerIndex].comDif;
        lbl_1_bss_1D8[playerIndex].status = 1;
        lbl_1_bss_1D8[playerIndex].actionState = 17;
        lbl_1_bss_1D8[playerIndex].timer = 600;
        lbl_1_bss_1D8[playerIndex].objectModel =
            CharModelCreate(lbl_1_bss_1D8[playerIndex].characterNo, 4);
        for (motionIndex = 0; motionIndex < 12; motionIndex++) {
            lbl_1_bss_1D8[playerIndex].motionIds[motionIndex] = CharMotionCreate(
                lbl_1_bss_1D8[playerIndex].characterNo, lbl_1_data_1E8[motionIndex]);
        }
        CharMotionDataClose(lbl_1_bss_1D8[playerIndex].characterNo);
        lbl_1_bss_1D8[playerIndex].velocity.x =
        lbl_1_bss_1D8[playerIndex].velocity.y =
        lbl_1_bss_1D8[playerIndex].velocity.z = 0.0f;
        lbl_1_bss_1D8[playerIndex].collisionRadius = 42.0f;
        lbl_1_bss_1D8[playerIndex].height =
            CharModelHeightGet(lbl_1_bss_1D8[playerIndex].characterNo);
        lbl_1_bss_1D8[playerIndex].targetRotation.x =
        lbl_1_bss_1D8[playerIndex].targetRotation.y =
        lbl_1_bss_1D8[playerIndex].targetRotation.z = 0.0f;
        lbl_1_bss_1D8[playerIndex].movedDistance = 100.0f;
        lbl_1_bss_1D8[playerIndex].moveSpeed = 0.0f;
        lbl_1_bss_1D8[playerIndex].storedVelocity.x =
        lbl_1_bss_1D8[playerIndex].storedVelocity.y =
        lbl_1_bss_1D8[playerIndex].storedVelocity.z = 0.0f;
        lbl_1_bss_1D8[playerIndex].target.x =
        lbl_1_bss_1D8[playerIndex].target.y =
        lbl_1_bss_1D8[playerIndex].target.z = 0.0f;
        lbl_1_bss_1D8[playerIndex].hasTarget = 0;
        lbl_1_bss_1D8[playerIndex].motionCounter = 0;
        lbl_1_bss_1D8[playerIndex].turnDirection = 0;

        if (GwPlayerConf[playerIndex].grpNo == 0) {
            lbl_1_bss_1D8[playerIndex].team = 0;
            lbl_1_bss_1D8[playerIndex].collisionRadius = 21.0f;
            lbl_1_bss_1D8[playerIndex].motionSet = -1;
            fn_1_296C(playerIndex, 0);
            CharModelAttrSet(lbl_1_bss_1D8[playerIndex].characterNo, HU3D_MOTATTR_LOOP);
            CharModelVoiceFlagSet(lbl_1_bss_1D8[playerIndex].characterNo, 0);
            Hu3DModelCameraSet(lbl_1_bss_1D8[playerIndex].objectModel, 1U);
            Hu3DModelLayerSet(lbl_1_bss_1D8[playerIndex].objectModel, 1);
            lbl_1_bss_1D8[playerIndex].position = lbl_1_data_218[0];
            lbl_1_bss_1D8[playerIndex].rotation.x =
            lbl_1_bss_1D8[playerIndex].rotation.y =
            lbl_1_bss_1D8[playerIndex].rotation.z = 0.0f;
            if (lbl_1_bss_1D8[playerIndex].characterNo == 3) {
                lbl_1_bss_1D8[playerIndex].rotation.x = 20.0f;
            }
            lbl_1_bss_1D8[playerIndex].scale.x =
            lbl_1_bss_1D8[playerIndex].scale.y =
            lbl_1_bss_1D8[playerIndex].scale.z = 1.0f;
            Hu3DModelPosSetV(lbl_1_bss_1D8[playerIndex].objectModel,
                             &lbl_1_bss_1D8[playerIndex].position);
            Hu3DModelRotSetV(lbl_1_bss_1D8[playerIndex].objectModel,
                             &lbl_1_bss_1D8[playerIndex].rotation);
            Hu3DModelScaleSetV(lbl_1_bss_1D8[playerIndex].objectModel,
                               &lbl_1_bss_1D8[playerIndex].scale);
            lbl_1_bss_1D8[playerIndex].light = -1;
            if (nightMode == 0) {
                lbl_1_bss_1D8[playerIndex].light = Hu3DLLightCreate(
                    lbl_1_bss_1D8[playerIndex].objectModel, lbl_1_data_278[0].position.x,
                    lbl_1_data_278[0].position.y, lbl_1_data_278[0].position.z,
                    lbl_1_data_278[0].direction.x, lbl_1_data_278[0].direction.y,
                    lbl_1_data_278[0].position.z, lbl_1_data_278[0].color.r,
                    lbl_1_data_278[0].color.g, lbl_1_data_278[0].color.b);
                Hu3DLLightInfinitytSet(lbl_1_bss_1D8[playerIndex].objectModel,
                                       lbl_1_bss_1D8[playerIndex].light);
                Hu3DModelAttrSet(lbl_1_bss_1D8[playerIndex].objectModel, HU3D_ATTR_LLIGHT);
            }
            if (nightMode == 1) {
                lbl_1_bss_1D8[playerIndex].light = Hu3DLLightCreate(
                    lbl_1_bss_1D8[playerIndex].objectModel, lbl_1_data_278[1].position.x,
                    lbl_1_data_278[1].position.y, lbl_1_data_278[1].position.z,
                    lbl_1_data_278[1].direction.x, lbl_1_data_278[1].direction.y,
                    lbl_1_data_278[1].position.z, lbl_1_data_278[1].color.r,
                    lbl_1_data_278[1].color.g, lbl_1_data_278[1].color.b);
            }
            if (nightMode == 0) {
                lbl_1_bss_1D8[playerIndex].scaleModel = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_m625, 9), HU_MEMNUM_OVL, HEAP_MODEL));
            } else {
                lbl_1_bss_1D8[playerIndex].scaleModel = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_m625, 11), HU_MEMNUM_OVL, HEAP_MODEL));
            }
            Hu3DModelHookSet(lbl_1_bss_1D8[playerIndex].objectModel,
                             CharModelItemHookGet(lbl_1_bss_1D8[playerIndex].characterNo, 4, 0),
                             lbl_1_bss_1D8[playerIndex].scaleModel);
            lbl_1_bss_1D8[playerIndex].modelScale = 1.0f;
        } else {
            lbl_1_bss_1D8[playerIndex].team = 1;
            lbl_1_bss_1D8[playerIndex].motionSet = -1;
            fn_1_296C(playerIndex, 6);
            CharModelAttrSet(lbl_1_bss_1D8[playerIndex].characterNo, HU3D_MOTATTR_LOOP);
            Hu3DModelCameraSet(lbl_1_bss_1D8[playerIndex].objectModel, 1U);
            if (teamIndex == 0) {
                Hu3DModelLayerSet(lbl_1_bss_1D8[playerIndex].objectModel, 1);
            }
            if (teamIndex == 1) {
                Hu3DModelLayerSet(lbl_1_bss_1D8[playerIndex].objectModel, 2);
            }
            if (teamIndex == 2) {
                Hu3DModelLayerSet(lbl_1_bss_1D8[playerIndex].objectModel, 3);
            }
            lbl_1_bss_1D8[playerIndex].position = lbl_1_data_218[teamIndex];
            lbl_1_bss_1D8[playerIndex].rotation.x =
            lbl_1_bss_1D8[playerIndex].rotation.y =
            lbl_1_bss_1D8[playerIndex].rotation.z = 0.0f;
            lbl_1_bss_1D8[playerIndex].scale.x =
            lbl_1_bss_1D8[playerIndex].scale.y =
            lbl_1_bss_1D8[playerIndex].scale.z = 1.0f;
            Hu3DModelPosSetV(lbl_1_bss_1D8[playerIndex].objectModel,
                             &lbl_1_bss_1D8[playerIndex].position);
            Hu3DModelRotSetV(lbl_1_bss_1D8[playerIndex].objectModel,
                             &lbl_1_bss_1D8[playerIndex].rotation);
            Hu3DModelScaleSetV(lbl_1_bss_1D8[playerIndex].objectModel,
                               &lbl_1_bss_1D8[playerIndex].scale);
            lbl_1_bss_1D8[playerIndex].light = -1;
            if (nightMode == 1) {
                lbl_1_bss_1D8[playerIndex].light = Hu3DLLightCreate(
                    lbl_1_bss_1D8[playerIndex].objectModel, lbl_1_data_278[1].position.x,
                    lbl_1_data_278[1].position.y, lbl_1_data_278[1].position.z,
                    lbl_1_data_278[1].direction.x, lbl_1_data_278[1].direction.y,
                    lbl_1_data_278[1].position.z, lbl_1_data_278[1].color.r,
                    lbl_1_data_278[1].color.g, lbl_1_data_278[1].color.b);
            }
        lbl_1_bss_1D8[playerIndex].particleTimer = (s16)(frand() & 0x7F);
            teamIndex++;
        }
        lbl_1_bss_1D8[playerIndex].stickX =
            lbl_1_bss_1D8[playerIndex].stickY = 0;
        lbl_1_bss_1D8[playerIndex].buttons =
            lbl_1_bss_1D8[playerIndex].buttonsDown = 0;
        CharMotionVoiceOnSet(lbl_1_bss_1D8[playerIndex].characterNo,
                             (s16)lbl_1_data_1E8[1], 0);
        playerIndex++;
    }
    Hu3DZClearLayerSet(2);
    Hu3DZClearLayerSet(3);
    CharEffectLayerSet(5);
    fn_1_EFC4();
    fn_1_11040();
    fn_1_10BD4();
}

/* Called by fn_1_13D30 to advance opening states; returns 1 when all players reach ready. */
s32 fn_1_3E8C(s16 player)
{
    s16 index;
    s16 initialization[1];

    index = 0;
    initialization[0] = 0;
    while (index < 4) {
        if (lbl_1_bss_1D8[index].team == 0) {
            fn_1_3F6C(index, player);
        } else {
            fn_1_4078(index, player);
        }
        index++;
    }
    fn_1_5108();
    index = 0;
    while (index < 4) {
        if (lbl_1_bss_1D8[index].actionState != 29) {
            break;
        }
        index++;
    }
    if (index >= 4) {
        return 1;
    }
    return 0;
}

/* Called by fn_1_3E8C to advance a team 0 player's opening state from start to ready. */
void fn_1_3F6C(s16 index, s16 frame)
{
    switch (lbl_1_bss_1D8[index].actionState) {
    case 17:
        if (frame >= 60) {
            lbl_1_bss_1D8[index].actionState = 18;
        }
        break;
    case 18:
        lbl_1_bss_1D8[index].buttonsDown = PAD_BUTTON_A;
        lbl_1_bss_1D8[index].actionState = 19;
        break;
    case 19:
        lbl_1_bss_1D8[index].actionState = 20;
        break;
    case 20:
        lbl_1_bss_1D8[index].actionState = 21;
        break;
    case 21:
    default:
        lbl_1_bss_1D8[index].actionState = 29;
        break;
    }
}

/* Advances team 1 players through their entrance and starting movement during fn_1_3E8C. */
void fn_1_4078(s16 index, s16 frame)
{
    f32 maximumTime;
    f32 minimumDistance;
    f32 distance;
    s16 nearestIndex;
    s16 scanIndex;

    switch (lbl_1_bss_1D8[index].actionState) {
    case 17:
        if (frame >= 60) {
            lbl_1_bss_1D8[index].actionState = 22;
        }
        break;
    case 22:
        if (frame >= 80) {
            lbl_1_bss_1D8[index].actionState = 23;
        }
        lbl_1_bss_1D8[index].motionCounter = 0;
        return;
    case 23:
        lbl_1_bss_1D8[index].status = 0;
        if (lbl_1_bss_1D8[index].motionSet != 10) {
            lbl_1_bss_1D8[index].position.y -= 0.5f * lbl_1_bss_1D8[index].height;
            Hu3DModelPosSetV(lbl_1_bss_1D8[index].objectModel, &lbl_1_bss_1D8[index].position);
        }
        fn_1_296C(index, 10);
        maximumTime = CharMotionMaxTimeGet(lbl_1_bss_1D8[index].characterNo);
        if ((Hu3DData[lbl_1_bss_1D8[index].objectModel].motWork.speed +
             CharMotionTimeGet(lbl_1_bss_1D8[index].characterNo)) >= maximumTime) {
            CharModelAttrSet(lbl_1_bss_1D8[index].characterNo, HU3D_MOTATTR_PAUSE);
            lbl_1_bss_1D8[index].motionCounter += 1;
            if (lbl_1_bss_1D8[index].motionCounter > 30) {
                lbl_1_bss_1D8[index].actionState = 24;
                lbl_1_bss_1D8[index].motionCounter = 0;
                CharModelAttrReset(lbl_1_bss_1D8[index].characterNo, HU3D_MOTATTR_PAUSE);
                CharModelAttrSet(lbl_1_bss_1D8[index].characterNo, HU3D_MOTATTR_REV);
                return;
            }
        }
        break;
    case 24:
        if (lbl_1_bss_1D8[index].motionSet == 6 &&
            CharMotionShiftIDGet(lbl_1_bss_1D8[index].characterNo) < 0) {
            lbl_1_bss_1D8[index].status = 1;
            lbl_1_bss_1D8[index].actionState = 25;
        }
        if (lbl_1_bss_1D8[index].motionSet != 6 &&
            CharMotionTimeGet(lbl_1_bss_1D8[index].characterNo) <= 1.0f) {
            CharModelAttrReset(lbl_1_bss_1D8[index].characterNo, HU3D_MOTATTR_REV);
            minimumDistance = lbl_1_data_2B8[6].blendTime;
            lbl_1_data_2B8[6].blendTime = 0.0f;
            fn_1_296C(index, 6);
            lbl_1_data_2B8[6].blendTime = minimumDistance;
            lbl_1_bss_1D8[index].position.y += 0.5f * lbl_1_bss_1D8[index].height;
            Hu3DModelPosSetV(lbl_1_bss_1D8[index].objectModel, &lbl_1_bss_1D8[index].position);
            return;
        }
        break;
    case 25:
        lbl_1_bss_1D8[index].stickX =
            lbl_1_bss_1D8[index].stickY = 0;
        lbl_1_bss_1D8[index].buttons =
            lbl_1_bss_1D8[index].buttonsDown = 0;
        if (lbl_1_bss_1D8[index].position.x < 0.0f) {
            lbl_1_bss_1D8[index].stickX = -56;
        } else {
            lbl_1_bss_1D8[index].stickX = 56;
        }
        lbl_1_bss_1D8[index].buttonsDown = PAD_BUTTON_A;
        lbl_1_bss_1D8[index].actionState = 26;
        lbl_1_bss_1D8[index].motionCounter = 0;
        return;
    case 26:
        lbl_1_bss_1D8[index].motionCounter += 1;
        if (lbl_1_bss_1D8[index].motionCounter > 120) {
            lbl_1_bss_1D8[index].actionState = 27;
            return;
        }
        break;
    case 27:
        lbl_1_bss_1D8[index].stickX =
            lbl_1_bss_1D8[index].stickY = 0;
        lbl_1_bss_1D8[index].buttons =
            lbl_1_bss_1D8[index].buttonsDown = 0;
        minimumDistance = 60000.0f;
        scanIndex = 0;
        nearestIndex = 0;
        while (scanIndex < 4) {
            if (lbl_1_bss_1D8[scanIndex].team != 0) {
                distance = lbl_1_bss_1D8[scanIndex].position.x;
                if (distance < 0.0f) {
                    distance *= -1.0f;
                }
                if (minimumDistance >= distance) {
                    minimumDistance = distance;
                    nearestIndex = scanIndex;
                }
            }
            scanIndex += 1;
        }
        if (nearestIndex == index) {
            if (lbl_1_bss_1D8[index].position.x > 0.0f) {
                lbl_1_bss_1D8[index].stickX = -56;
            } else if (lbl_1_bss_1D8[index].position.x < 0.0f) {
                lbl_1_bss_1D8[index].stickX = 56;
            }
            lbl_1_bss_1D8[index].buttonsDown = PAD_BUTTON_A;
        }
        if (((lbl_1_bss_1D8[index].motionSet == 6) &&
             (lbl_1_bss_1D8[index].movedDistance >= 80.0f)) ||
            ((lbl_1_bss_1D8[index].motionSet == 7) &&
             (lbl_1_bss_1D8[index].movedDistance >= 80.0f))) {
            lbl_1_bss_1D8[index].actionState = 28;
            return;
        }
        break;
    case 28:
        lbl_1_bss_1D8[index].stickX =
            lbl_1_bss_1D8[index].stickY = 0;
        lbl_1_bss_1D8[index].buttons =
            lbl_1_bss_1D8[index].buttonsDown = 0;
        /* fall through */
    default:
        lbl_1_bss_1D8[index].actionState = 29;
        break;
    }
}

s16 lbl_1_data_36C[4] = { 0, 0, 0, 0 };

void fn_1_4E60(s16 mode, s16 frameNo)
{
    fn_1_5108();
}

/* Called by fn_1_13E58 each play frame to read controls, advance physics and update swim timers. */
void fn_1_4E80(void)
{
    s16 index;

    fn_1_EB00();
    fn_1_F07C();
    fn_1_5108();
    for (index = 0; index < 4; index++) {
        if ((lbl_1_bss_1D8[index].position.y + lbl_1_bss_1D8[index].velocity.y) >=
            -20.0f) {
            lbl_1_bss_1D8[index].timer += 5;
            if (lbl_1_data_36C[index] > 8) {
                f32 pan = 1200.0f;

                pan = (lbl_1_bss_1D8[index].position.x - -600.0f) / pan;
                pan = 32.0f + (64.0f * pan);
                HuAudFXPanning(HuAudFXPlay(M625_SE_TIMER_TICK), (s16) pan);
            }
            lbl_1_data_36C[index] = 0;
        } else if (lbl_1_bss_1D8[index].status == 1) {
            lbl_1_bss_1D8[index].timer--;
            lbl_1_data_36C[index]++;
            if (lbl_1_data_36C[index] >= 127) {
                lbl_1_data_36C[index] = 126;
            }
        }
        if (lbl_1_bss_1D8[index].timer < 0) {
            lbl_1_bss_1D8[index].timer = 0;
        }
        if (lbl_1_bss_1D8[index].timer > 600) {
            lbl_1_bss_1D8[index].timer = 600;
        }
    }
}

/* Resolves player movement and collisions when setup or gameplay callbacks update the players. */
void fn_1_5108(void)
{
    Point3d position;
    Point3d velocity;
    Point3d rotation;
    Point3d otherPosition;
    Point3d otherVelocity;
    Point3d otherRotation;
    f32 distance;
    f32 reverseDistance;
    s16 playerIndex;
    s16 otherIndex;
    s16 contactCount;
    s16 neighborCount;
    s16 activeCount;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_1D8[playerIndex].team == 0) {
            fn_1_6CE0(playerIndex);
        } else {
            fn_1_860C(playerIndex);
        }
    }
    fn_1_11700();
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_1D8[playerIndex].team != 0 &&
            lbl_1_bss_1D8[playerIndex].status != 14 &&
            lbl_1_bss_1D8[playerIndex].status != 15 &&
            lbl_1_bss_1D8[playerIndex].status != 16) {
            position = lbl_1_bss_1D8[playerIndex].position;
            velocity = lbl_1_bss_1D8[playerIndex].velocity;
            distance = lbl_1_bss_1D8[playerIndex].collisionRadius;
            if (fn_1_12254(position, &velocity, distance) != 0) {
                velocity.z = lbl_1_bss_1D8[playerIndex].velocity.z;
                lbl_1_bss_1D8[playerIndex].velocity = velocity;
                lbl_1_bss_1D8[playerIndex].status = 14;
            }
        }
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_1D8[playerIndex].team != 0 &&
            lbl_1_bss_1D8[playerIndex].status != 14 &&
            lbl_1_bss_1D8[playerIndex].status != 15 &&
            lbl_1_bss_1D8[playerIndex].status != 16) {
            position = lbl_1_bss_1D8[playerIndex].position;
            velocity = lbl_1_bss_1D8[playerIndex].velocity;
            fn_1_2C60(playerIndex, &rotation);
            position.z = velocity.z = 0.0f;
            for (otherIndex = playerIndex; otherIndex < 4; otherIndex++) {
                if (playerIndex != otherIndex && lbl_1_bss_1D8[otherIndex].team != 0 &&
                    lbl_1_bss_1D8[otherIndex].status != 14 &&
                    lbl_1_bss_1D8[otherIndex].status != 15 &&
                    lbl_1_bss_1D8[otherIndex].status != 16) {
                    otherPosition = lbl_1_bss_1D8[otherIndex].position;
                    otherVelocity = lbl_1_bss_1D8[otherIndex].velocity;
                    fn_1_2C60(otherIndex, &otherRotation);
                    otherPosition.z = otherVelocity.z = 0.0f;
                    if (fn_1_137DC(position, rotation, &velocity,
                        lbl_1_bss_1D8[playerIndex].collisionRadius,
                        lbl_1_bss_1D8[playerIndex].height, otherPosition, otherRotation,
                        &otherVelocity, lbl_1_bss_1D8[otherIndex].collisionRadius,
                        lbl_1_bss_1D8[otherIndex].height) != 0) {
                        velocity.z = 0.0f;
                        otherVelocity.z = 0.0f;
                        lbl_1_bss_1D8[playerIndex].velocity = velocity;
                        lbl_1_bss_1D8[otherIndex].velocity = otherVelocity;
                    }
                }
            }
        }
    }
    for (playerIndex = 3; playerIndex >= 0; playerIndex--) {
        if (lbl_1_bss_1D8[playerIndex].team != 0 &&
            lbl_1_bss_1D8[playerIndex].status != 14 &&
            lbl_1_bss_1D8[playerIndex].status != 15 &&
            lbl_1_bss_1D8[playerIndex].status != 16) {
            position = lbl_1_bss_1D8[playerIndex].position;
            velocity = lbl_1_bss_1D8[playerIndex].velocity;
            fn_1_2C60(playerIndex, &rotation);
            position.z = velocity.z = 0.0f;
            for (otherIndex = playerIndex; otherIndex >= 0; otherIndex--) {
                if (playerIndex != otherIndex && lbl_1_bss_1D8[otherIndex].team != 0 &&
                    lbl_1_bss_1D8[otherIndex].status != 14 &&
                    lbl_1_bss_1D8[otherIndex].status != 15 &&
                    lbl_1_bss_1D8[otherIndex].status != 16) {
                    otherPosition = lbl_1_bss_1D8[otherIndex].position;
                    otherVelocity = lbl_1_bss_1D8[otherIndex].velocity;
                    fn_1_2C60(otherIndex, &otherRotation);
                    otherPosition.z = otherVelocity.z = 0.0f;
                    if (fn_1_137DC(position, rotation, &velocity,
                        lbl_1_bss_1D8[playerIndex].collisionRadius,
                        lbl_1_bss_1D8[playerIndex].height, otherPosition, otherRotation,
                        &otherVelocity, lbl_1_bss_1D8[otherIndex].collisionRadius,
                        lbl_1_bss_1D8[otherIndex].height) != 0) {
                        velocity.z = 0.0f;
                        otherVelocity.z = 0.0f;
                        lbl_1_bss_1D8[playerIndex].velocity = velocity;
                        lbl_1_bss_1D8[otherIndex].velocity = otherVelocity;
                    }
                }
            }
        }
    }
    {
        s16 collisions[4][4] = { 0 };

        if (lbl_1_bss_1D4 != 0) {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                if (lbl_1_bss_1D8[playerIndex].team != 0 &&
                    lbl_1_bss_1D8[playerIndex].status != 14 &&
                    lbl_1_bss_1D8[playerIndex].status != 15 &&
                    lbl_1_bss_1D8[playerIndex].status != 16) {
                    position = lbl_1_bss_1D8[playerIndex].position;
                    PSVECAdd(&position, &lbl_1_bss_1D8[playerIndex].velocity, &position);
                    distance = 5.0f + 0.5f * lbl_1_bss_1D8[playerIndex].height;
                    for (otherIndex = 0; otherIndex < 4; otherIndex++) {
                        if (playerIndex != otherIndex &&
                            lbl_1_bss_1D8[otherIndex].team != 0 &&
                            lbl_1_bss_1D8[otherIndex].status != 14 &&
                            lbl_1_bss_1D8[otherIndex].status != 15 &&
                            lbl_1_bss_1D8[otherIndex].status != 16) {
                            otherPosition = lbl_1_bss_1D8[otherIndex].position;
                            PSVECAdd(&otherPosition, &lbl_1_bss_1D8[otherIndex].velocity,
                                     &otherPosition);
                            reverseDistance = 5.0f + 0.5f * lbl_1_bss_1D8[otherIndex].height;
                            if (fn_1_13318(position, distance, otherPosition,
                                reverseDistance) != 0) {
                                collisions[playerIndex][otherIndex] = 1;
                            }
                        }
                    }
                }
            }
            activeCount = 0;
            contactCount = 0;
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                if (lbl_1_bss_1D8[playerIndex].team != 0 &&
                    lbl_1_bss_1D8[playerIndex].status != 14 &&
                    lbl_1_bss_1D8[playerIndex].status != 15 &&
                    lbl_1_bss_1D8[playerIndex].status != 16) {
                    activeCount++;
                    neighborCount = 0;
                    for (otherIndex = 0; otherIndex < 4; otherIndex++) {
                        if (playerIndex != otherIndex &&
                            lbl_1_bss_1D8[otherIndex].team != 0 &&
                            lbl_1_bss_1D8[otherIndex].status != 14 &&
                            lbl_1_bss_1D8[otherIndex].status != 15 &&
                            lbl_1_bss_1D8[otherIndex].status != 16 &&
                            collisions[playerIndex][otherIndex] != 0) {
                            neighborCount++;
                            contactCount++;
                        }
                    }
                    if (neighborCount >= 2) {
                        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                            if (lbl_1_bss_1D8[playerIndex].team != 0 &&
                                lbl_1_bss_1D8[playerIndex].status != 14 &&
                                lbl_1_bss_1D8[playerIndex].status != 15 &&
                                lbl_1_bss_1D8[playerIndex].status != 16) {
                                position = lbl_1_bss_1D8[playerIndex].position;
                                for (otherIndex = 0; otherIndex < 4; otherIndex++) {
                                    if (playerIndex != otherIndex &&
                                        lbl_1_bss_1D8[otherIndex].team != 0 &&
                                        lbl_1_bss_1D8[otherIndex].status != 14 &&
                                        lbl_1_bss_1D8[otherIndex].status != 15 &&
                                        lbl_1_bss_1D8[otherIndex].status != 16) {
                                        otherPosition = lbl_1_bss_1D8[otherIndex].position;
                                        distance = lbl_1_bss_1D8[otherIndex].position.x -
                                                   lbl_1_bss_1D8[playerIndex].position.x;
                                        if (distance > 0.0f) {
                                            lbl_1_bss_1D8[otherIndex].velocity.x += 1.0f;
                                        } else if (distance < 0.0f) {
                                            lbl_1_bss_1D8[otherIndex].velocity.x -= 1.0f;
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    } else if (activeCount == 3 && contactCount == 2) {
                        otherIndex = -1;
                        for (playerIndex = 0; playerIndex < 3; playerIndex++) {
                            if (lbl_1_bss_1D8[playerIndex].team != 0 &&
                                lbl_1_bss_1D8[playerIndex].status != 14 &&
                                lbl_1_bss_1D8[playerIndex].status != 15 &&
                                lbl_1_bss_1D8[playerIndex].status != 16) {
                                if (otherIndex >= 0 &&
                                    (collisions[playerIndex][0] != 0 ||
                                     collisions[playerIndex][1] != 0 ||
                                     collisions[playerIndex][2] != 0 ||
                                     collisions[playerIndex][3] != 0)) {
                                    break;
                                }
                                if (otherIndex < 0 &&
                                    (collisions[playerIndex][0] != 0 ||
                                     collisions[playerIndex][1] != 0 ||
                                     collisions[playerIndex][2] != 0 ||
                                     collisions[playerIndex][3] != 0)) {
                                    otherIndex = playerIndex;
                                }
                            }
                        }
                        distance = lbl_1_bss_1D8[otherIndex].position.x -
                                   lbl_1_bss_1D8[playerIndex].position.x;
                        reverseDistance = lbl_1_bss_1D8[playerIndex].position.x -
                                          lbl_1_bss_1D8[otherIndex].position.x;
                        if (distance > 0.0f) {
                            lbl_1_bss_1D8[otherIndex].velocity.x += 1.0f;
                        } else if (distance < 0.0f) {
                            lbl_1_bss_1D8[otherIndex].velocity.x -= 1.0f;
                        } else {
                            lbl_1_bss_1D8[otherIndex].velocity.x += 1.0f;
                        }
                        if (reverseDistance > 0.0f) {
                            lbl_1_bss_1D8[playerIndex].velocity.x += 1.0f;
                        } else if (reverseDistance < 0.0f) {
                            lbl_1_bss_1D8[playerIndex].velocity.x -= 1.0f;
                        } else {
                            lbl_1_bss_1D8[playerIndex].velocity.x -= 1.0f;
                        }
                        break;
                    }
                }
            }
        }
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_1D8[playerIndex].status != 14 &&
            lbl_1_bss_1D8[playerIndex].status != 15 &&
            lbl_1_bss_1D8[playerIndex].status != 16) {
            position = lbl_1_bss_1D8[playerIndex].position;
            velocity = lbl_1_bss_1D8[playerIndex].velocity;
            distance = lbl_1_bss_1D8[playerIndex].collisionRadius;
            position.z = 0.0f;
            velocity.z = 0.0f;
            otherPosition.x = otherPosition.z = 0.0f;
            otherPosition.y = -550.0f;
            otherVelocity.x = otherVelocity.z = 0.0f;
            otherVelocity.y = 1.0f;
            if (fn_1_133A4(position, &velocity, distance, otherPosition, otherVelocity, 0.0f) !=
                0) {
                velocity.z = lbl_1_bss_1D8[playerIndex].velocity.z;
                lbl_1_bss_1D8[playerIndex].velocity = velocity;
            }
            otherPosition.y = otherPosition.z = 0.0f;
            otherPosition.x = -600.0f;
            otherVelocity.y = otherVelocity.z = 0.0f;
            otherVelocity.x = 1.0f;
            if (fn_1_133A4(position, &velocity, distance, otherPosition, otherVelocity, 0.0f) !=
                0) {
                velocity.z = lbl_1_bss_1D8[playerIndex].velocity.z;
                lbl_1_bss_1D8[playerIndex].velocity = velocity;
            }
            otherPosition.y = otherPosition.z = 0.0f;
            otherPosition.x = 600.0f;
            otherVelocity.y = otherVelocity.z = 0.0f;
            otherVelocity.x = -1.0f;
            if (fn_1_133A4(position, &velocity, distance, otherPosition, otherVelocity, 0.0f) !=
                0) {
                velocity.z = lbl_1_bss_1D8[playerIndex].velocity.z;
                lbl_1_bss_1D8[playerIndex].velocity = velocity;
            }
        }
    }
    fn_1_EE94();
    fn_1_BF54();
    fn_1_12560();
}

/* Called by fn_1_5108 to update a team 0 player's swim input, facing, movement and animation. */
void fn_1_6CE0(s16 playerIndex)
{
    Point3d input = { 0.0f, 0.0f, 0.0f };
    Point3d rotation;
    f32 motionTime;

    switch (lbl_1_bss_1D8[playerIndex].status) {
    case 1: {
        if ((lbl_1_bss_1D8[playerIndex].buttonsDown & PAD_BUTTON_A) != 0) {
            lbl_1_bss_1D8[playerIndex].status = 6;
            motionTime = 1200.0f;
            motionTime = (lbl_1_bss_1D8[playerIndex].position.x - -600.0f) / motionTime;
            motionTime = 32.0f + 64.0f * motionTime;
            HuAudFXPanning(HuAudFXPlay(M625_SE_PLAYER_START), motionTime);
            return;
        }

        input.y = input.z = 0.0f;
        input.x = lbl_1_bss_1D8[playerIndex].stickX / 56.0f;
        input.x *= 7.0f;
        if (input.x < 0.0f && lbl_1_bss_1D8[playerIndex].rotation.y <= -90.0f) {
            lbl_1_bss_1D8[playerIndex].velocity = input;
        }
        if (input.x > 0.0f && lbl_1_bss_1D8[playerIndex].rotation.y >= 90.0f) {
            lbl_1_bss_1D8[playerIndex].velocity = input;
        }

        rotation = lbl_1_bss_1D8[playerIndex].rotation;
        if ((input.x < 0.0f && lbl_1_bss_1D8[playerIndex].turnDirection == 0) ||
            lbl_1_bss_1D8[playerIndex].turnDirection == -5) {
            rotation.y += -9.0f;
            if (rotation.y < -90.0f) {
                rotation.y = -90.0f;
                lbl_1_bss_1D8[playerIndex].turnDirection = 0;
            } else {
                lbl_1_bss_1D8[playerIndex].turnDirection = -5;
            }
        }
        if ((input.x > 0.0f && lbl_1_bss_1D8[playerIndex].turnDirection == 0) ||
            lbl_1_bss_1D8[playerIndex].turnDirection == 5) {
            rotation.y += 9.0f;
            if (rotation.y > 90.0f) {
                rotation.y = 90.0f;
                lbl_1_bss_1D8[playerIndex].turnDirection = 0;
            } else {
                lbl_1_bss_1D8[playerIndex].turnDirection = 5;
            }
        }
        if ((0.0f == input.x && lbl_1_bss_1D8[playerIndex].turnDirection == 0) ||
            lbl_1_bss_1D8[playerIndex].turnDirection == 100) {
            if (rotation.y < -1.0f || 1.0f < rotation.y) {
                rotation.y /= 10.0f;
                lbl_1_bss_1D8[playerIndex].turnDirection = 100;
            } else {
                rotation.y = 0.0f;
                lbl_1_bss_1D8[playerIndex].turnDirection = 0;
            }
        }

        lbl_1_bss_1D8[playerIndex].rotation = rotation;
        if (lbl_1_bss_1D8[playerIndex].motionSet == 8 ||
            lbl_1_bss_1D8[playerIndex].motionSet == 4 ||
            lbl_1_bss_1D8[playerIndex].motionSet == 5) {
            break;
        }
        if (0.0f != input.x) {
            motionTime = 3.0f;
            motionTime *= motionTime;
            if (input.x * input.x < motionTime && lbl_1_bss_1D8[playerIndex].motionCounter >= 20) {
                fn_1_296C(playerIndex, 1);
                lbl_1_bss_1D8[playerIndex].motionCounter = 0;
            }
            if (input.x * input.x >= motionTime && lbl_1_bss_1D8[playerIndex].motionCounter >= 20) {
                fn_1_296C(playerIndex, 2);
                lbl_1_bss_1D8[playerIndex].motionCounter = 0;
            }
        } else if (lbl_1_bss_1D8[playerIndex].motionCounter >= 20) {
            fn_1_296C(playerIndex, 0);
            lbl_1_bss_1D8[playerIndex].motionCounter = 30;
        }
        lbl_1_bss_1D8[playerIndex].motionCounter++;
        if (lbl_1_bss_1D8[playerIndex].motionCounter >= 20) {
            lbl_1_bss_1D8[playerIndex].motionCounter = 20;
            return;
        }
        break;
    }
    case 6: {
        fn_1_296C(playerIndex, 0);
        rotation = lbl_1_bss_1D8[playerIndex].rotation;
        if (rotation.y < -1.0f || rotation.y > 1.0f) {
            rotation.y /= 5.0f;
        } else {
            rotation.y = 0.0f;
            lbl_1_bss_1D8[playerIndex].turnDirection = 0;
            if (lbl_1_bss_1D8[playerIndex].motionSet == 0 &&
                CharMotionShiftIDGet(lbl_1_bss_1D8[playerIndex].characterNo) < 0) {
                fn_1_296C(playerIndex, 3);
                lbl_1_bss_1D8[playerIndex].status = 7;
            }
        }
        lbl_1_bss_1D8[playerIndex].rotation = rotation;
        return;
    }
    case 7: {
        motionTime = CharMotionTimeGet(lbl_1_bss_1D8[playerIndex].characterNo);
        if (9.0f < motionTime && motionTime < 11.0f) {
            lbl_1_bss_1D8[playerIndex].modelScale = 0.0f;
            Hu3DModelScaleSet(lbl_1_bss_1D8[playerIndex].scaleModel,
                              lbl_1_bss_1D8[playerIndex].modelScale,
                              lbl_1_bss_1D8[playerIndex].modelScale,
                              lbl_1_bss_1D8[playerIndex].modelScale);
            input.x = 0.0f;
            input.y = 20.0f;
            input.z = 45.0f;
            PSVECAdd(&lbl_1_bss_1D8[playerIndex].position, &input, &input);
            fn_1_11470(input);
        }
        if (motionTime + Hu3DData[lbl_1_bss_1D8[playerIndex].objectModel].motWork.speed >=
            CharMotionMaxTimeGet(lbl_1_bss_1D8[playerIndex].characterNo)) {
            fn_1_296C(playerIndex, 0);
            lbl_1_bss_1D8[playerIndex].status = 8;
            return;
        }
        break;
    }
    case 8:
        lbl_1_bss_1D8[playerIndex].status = 9;
        return;
    case 9:
        lbl_1_bss_1D8[playerIndex].modelScale += 0.08f;
        if (lbl_1_bss_1D8[playerIndex].modelScale >= 1.0f) {
            lbl_1_bss_1D8[playerIndex].modelScale = 1.0f;
            lbl_1_bss_1D8[playerIndex].status = 10;
        }
        Hu3DModelScaleSet(lbl_1_bss_1D8[playerIndex].scaleModel,
                          lbl_1_bss_1D8[playerIndex].modelScale,
                          lbl_1_bss_1D8[playerIndex].modelScale,
                          lbl_1_bss_1D8[playerIndex].modelScale);
        return;
    case 10:
        lbl_1_bss_1D8[playerIndex].status = 1;
        break;
    }
}

/* Debug hook: empty in the release build; its argument is still evaluated. */
static inline void M625DebugValue(f32 value)
{
}

/* Called by fn_1_5108 to update a team 1 player's swimming, rotation, motion and bubbles. */
void fn_1_860C(s16 playerIndex)
{
    Mtx rotationMatrix;
    Mtx axisMatrix;
    Point3d direction;
    Point3d particlePosition;
    f32 motionTime;
    s16 particleIndex;

    switch (lbl_1_bss_1D8[playerIndex].status) {
    case 1:
        lbl_1_bss_1D8[playerIndex].particleTimer++;
        if (lbl_1_bss_1D8[playerIndex].particleTimer >= 90) {
            Hu3DModelObjPosGet(lbl_1_bss_1D8[playerIndex].objectModel,
                CharHeadObjNameTbl[lbl_1_bss_1D8[playerIndex].characterNo], &direction);
            for (particleIndex = 0; particleIndex < 1; particleIndex++) {
                fn_1_10E90(direction, 8.0f, 3.0f, -10.0f);
            }
            lbl_1_bss_1D8[playerIndex].particleTimer = 0;
        }
        if ((lbl_1_bss_1D8[playerIndex].buttonsDown & PAD_BUTTON_A) != 0 &&
            lbl_1_bss_1D8[playerIndex].timer > 0 &&
            lbl_1_bss_1D8[playerIndex].stickX == 0 &&
            lbl_1_bss_1D8[playerIndex].stickY == 0) {
            lbl_1_bss_1D8[playerIndex].stickX = 0;
            lbl_1_bss_1D8[playerIndex].stickY = 56;
        }
        if ((lbl_1_bss_1D8[playerIndex].buttonsDown & PAD_BUTTON_A) != 0 &&
            lbl_1_bss_1D8[playerIndex].timer > 0 &&
            (lbl_1_bss_1D8[playerIndex].stickX != 0 ||
             lbl_1_bss_1D8[playerIndex].stickY != 0)) {
            if (lbl_1_bss_1D8[playerIndex].motionSet == 6 &&
                lbl_1_bss_1D8[playerIndex].movedDistance >= 80.0f) {
                lbl_1_bss_1D8[playerIndex].targetRotation = fn_1_B530(playerIndex);
                lbl_1_bss_1D8[playerIndex].movedDistance = 0.0f;
            }
            if (lbl_1_bss_1D8[playerIndex].motionSet == 7 &&
                lbl_1_bss_1D8[playerIndex].movedDistance >= 80.0f) {
                lbl_1_bss_1D8[playerIndex].targetRotation = fn_1_B530(playerIndex);
                lbl_1_bss_1D8[playerIndex].movedDistance = 0.0f;
            }
        }
        if ((lbl_1_bss_1D8[playerIndex].buttonsDown & PAD_BUTTON_A) != 0 &&
            lbl_1_bss_1D0 == 0 &&
            (lbl_1_bss_1D8[playerIndex].stickX != 0 ||
             lbl_1_bss_1D8[playerIndex].stickY != 0)) {
            if (lbl_1_bss_1D8[playerIndex].motionSet == 6 &&
                lbl_1_bss_1D8[playerIndex].movedDistance >= 80.0f) {
                lbl_1_bss_1D8[playerIndex].targetRotation = fn_1_B530(playerIndex);
                lbl_1_bss_1D8[playerIndex].movedDistance = 0.0f;
            }
            if (lbl_1_bss_1D8[playerIndex].motionSet == 7 &&
                lbl_1_bss_1D8[playerIndex].movedDistance >= 80.0f) {
                lbl_1_bss_1D8[playerIndex].targetRotation = fn_1_B530(playerIndex);
                lbl_1_bss_1D8[playerIndex].movedDistance = 0.0f;
            }
        }
        if (lbl_1_bss_1D8[playerIndex].position.y + lbl_1_bss_1D8[playerIndex].velocity.y >=
                -5.0f &&
            lbl_1_bss_1D8[playerIndex].targetRotation.x < -30.0f) {
            lbl_1_bss_1D8[playerIndex].targetRotation.x =
                lbl_1_bss_1D8[playerIndex].targetRotation.y =
                lbl_1_bss_1D8[playerIndex].targetRotation.z = 0.0f;
            lbl_1_bss_1D8[playerIndex].movedDistance = 100.0f;
            lbl_1_bss_1D8[playerIndex].moveSpeed = 0.0f;
        }
        fn_1_B958(playerIndex);
        if (lbl_1_bss_1D8[playerIndex].targetRotation.x < -1.0f ||
            1.0f < lbl_1_bss_1D8[playerIndex].targetRotation.x ||
            lbl_1_bss_1D8[playerIndex].targetRotation.y < -1.0f ||
            1.0f < lbl_1_bss_1D8[playerIndex].targetRotation.y) {
            fn_1_296C(playerIndex, 7);
        } else {
            fn_1_296C(playerIndex, 6);
            lbl_1_bss_1D8[playerIndex].movedDistance = 100.0f;
            lbl_1_bss_1D8[playerIndex].targetRotation.x =
                lbl_1_bss_1D8[playerIndex].targetRotation.y =
                lbl_1_bss_1D8[playerIndex].targetRotation.z = 0.0f;
        }
        if (lbl_1_bss_1D8[playerIndex].movedDistance >= 100.0f) {
            lbl_1_bss_1D8[playerIndex].moveSpeed -= 0.1;
        } else {
            lbl_1_bss_1D8[playerIndex].moveSpeed += 0.1;
        }
        if (lbl_1_bss_1D8[playerIndex].moveSpeed <= 0.0f) {
            lbl_1_bss_1D8[playerIndex].moveSpeed = 0.0f;
        }
        if (lbl_1_bss_1D8[playerIndex].moveSpeed >= 5.0f) {
            lbl_1_bss_1D8[playerIndex].moveSpeed = 5.0f;
        }
        direction.x = direction.y = 0.0f;
        direction.z = 1.0f;
        PSMTXRotRad(rotationMatrix, 'x', 0.017453292f * lbl_1_bss_1D8[playerIndex].rotation.x);
        PSMTXRotRad(axisMatrix, 'y', 0.017453292f * lbl_1_bss_1D8[playerIndex].rotation.y);
        PSMTXConcat(axisMatrix, rotationMatrix, rotationMatrix);
        PSMTXRotRad(axisMatrix, 'z', 0.017453292f * lbl_1_bss_1D8[playerIndex].rotation.z);
        PSMTXConcat(axisMatrix, rotationMatrix, rotationMatrix);
        PSMTXMultVecSR(rotationMatrix, &direction, &direction);
        direction.z = 0.0f;
        if (direction.x < -0.00001f || 0.00001f < direction.x ||
            direction.y < -0.00001f || 0.00001f < direction.y) {
            PSVECNormalize(&direction, &direction);
        } else {
            direction.x = direction.y = direction.z = 0.0f;
        }
        PSVECScale(&direction, &lbl_1_bss_1D8[playerIndex].velocity,
                   lbl_1_bss_1D8[playerIndex].moveSpeed);
        motionTime = PSVECMag(&lbl_1_bss_1D8[playerIndex].velocity);
        lbl_1_bss_1D8[playerIndex].movedDistance += motionTime;
        if (lbl_1_bss_1D8[playerIndex].movedDistance >= 100.0f) {
            lbl_1_bss_1D8[playerIndex].movedDistance = 100.0f;
        }
        if (motionTime <= 0.0f) {
            lbl_1_bss_1D8[playerIndex].targetRotation.x =
                lbl_1_bss_1D8[playerIndex].targetRotation.y =
                lbl_1_bss_1D8[playerIndex].targetRotation.z = 0.0f;
        }
        if (lbl_1_bss_1CC == 1 && -10.0f < lbl_1_bss_1D8[playerIndex].targetRotation.y &&
            lbl_1_bss_1D8[playerIndex].targetRotation.y < 10.0f) {
            lbl_1_bss_1D8[playerIndex].velocity.y += 0.5;
        }
        if (lbl_1_bss_1D8[playerIndex].position.y + lbl_1_bss_1D8[playerIndex].velocity.y >=
            -5.0f) {
            lbl_1_bss_1D8[playerIndex].velocity.y = -5.0f - lbl_1_bss_1D8[playerIndex].position.y;
        }
        if (lbl_1_bss_1D0 != 0 && lbl_1_bss_1D8[playerIndex].timer <= 0 &&
            -0.01f < lbl_1_bss_1D8[playerIndex].targetRotation.x &&
            lbl_1_bss_1D8[playerIndex].targetRotation.x < 0.01f &&
            -0.01f < lbl_1_bss_1D8[playerIndex].targetRotation.y &&
            lbl_1_bss_1D8[playerIndex].targetRotation.y < 0.01f) {
            lbl_1_bss_1D8[playerIndex].status = 11;
        }
        break;
    case 11:
        if (lbl_1_bss_1D8[playerIndex].motionSet != 9) {
            lbl_1_bss_1D8[playerIndex].movedDistance = 100.0f;
            lbl_1_bss_1D8[playerIndex].targetRotation.x =
                lbl_1_bss_1D8[playerIndex].targetRotation.y =
                lbl_1_bss_1D8[playerIndex].targetRotation.z = 0.0f;
            lbl_1_bss_1D8[playerIndex].rotation.x *= 0.7f;
            lbl_1_bss_1D8[playerIndex].rotation.y *= 0.7f;
            fn_1_296C(playerIndex, 6);
            if (-1.0f < lbl_1_bss_1D8[playerIndex].rotation.x &&
                lbl_1_bss_1D8[playerIndex].rotation.x < 1.0f &&
                -1.0f < lbl_1_bss_1D8[playerIndex].rotation.y &&
                lbl_1_bss_1D8[playerIndex].rotation.y < 1.0f) {
                lbl_1_bss_1D8[playerIndex].rotation.x =
                    lbl_1_bss_1D8[playerIndex].rotation.y = 0.0f;
                fn_1_296C(playerIndex, 9);
                motionTime = 1200.0f;
                motionTime = (lbl_1_bss_1D8[playerIndex].position.x - -600.0f) / motionTime;
                motionTime = 32.0f + 64.0f * motionTime;
            HuAudFXPanning(HuAudFXPlay(M625_SE_SWIM_START), motionTime);
            }
        } else {
            lbl_1_bss_1D8[playerIndex].targetRotation.x = 90.0f;
            fn_1_B958(playerIndex);
            if (lbl_1_bss_1D8[playerIndex].motionSet == 9 &&
                CharMotionShiftIDGet(lbl_1_bss_1D8[playerIndex].characterNo) < 0) {
                lbl_1_bss_1D8[playerIndex].status = 12;
                lbl_1_bss_1D8[playerIndex].rotation.x =
                    lbl_1_bss_1D8[playerIndex].targetRotation.x;
                motionTime = 1200.0f;
                motionTime = (lbl_1_bss_1D8[playerIndex].position.x - -600.0f) / motionTime;
                motionTime = 32.0f + 64.0f * motionTime;
            HuAudFXPanning(HuAudFXPlay(M625_SE_SWIM_START), motionTime);
            }
        }
        break;
    case 12:
        lbl_1_bss_1D8[playerIndex].particleTimer++;
        if (lbl_1_bss_1D8[playerIndex].particleTimer >= 45) {
            Hu3DModelObjPosGet(lbl_1_bss_1D8[playerIndex].objectModel,
                CharHeadObjNameTbl[lbl_1_bss_1D8[playerIndex].characterNo], &direction);
            for (particleIndex = 0; particleIndex < 20; particleIndex++) {
                particlePosition = direction;
                particlePosition.x += 0.1f * ((f32)(u8)frand() - 128.0f);
                particlePosition.y += 0.1f * ((f32)(u8)frand() - 128.0f);
                fn_1_10E90(particlePosition, 16.0f, 3.0f, -10.0f);
            }
            lbl_1_bss_1D8[playerIndex].particleTimer = 0;
        }
        lbl_1_bss_1D8[playerIndex].velocity.y += 3.0f;
        if (lbl_1_bss_1D8[playerIndex].position.y + lbl_1_bss_1D8[playerIndex].velocity.y >=
            -5.0f) {
            lbl_1_bss_1D8[playerIndex].velocity.y = -5.0f - lbl_1_bss_1D8[playerIndex].position.y;
        }
        if (lbl_1_bss_1D8[playerIndex].position.y + lbl_1_bss_1D8[playerIndex].velocity.y >=
            -15.0f) {
            lbl_1_bss_1D8[playerIndex].status = 13;
            lbl_1_bss_1D8[playerIndex].movedDistance = 100.0f;
            lbl_1_bss_1D8[playerIndex].targetRotation.x =
                lbl_1_bss_1D8[playerIndex].targetRotation.y =
                lbl_1_bss_1D8[playerIndex].targetRotation.z = 0.0f;
        }
        break;
    case 13:
        fn_1_296C(playerIndex, 6);
        lbl_1_bss_1D8[playerIndex].targetRotation.x =
            lbl_1_bss_1D8[playerIndex].targetRotation.y =
            lbl_1_bss_1D8[playerIndex].targetRotation.z = 0.0f;
        fn_1_B958(playerIndex);
        if (lbl_1_bss_1D8[playerIndex].timer > 0 &&
            lbl_1_bss_1D8[playerIndex].motionSet != 9 &&
            CharMotionShiftIDGet(lbl_1_bss_1D8[playerIndex].characterNo) < 0) {
            lbl_1_bss_1D8[playerIndex].status = 1;
            particlePosition.x = particlePosition.y = particlePosition.z = 0.0f;
            lbl_1_bss_1D8[playerIndex].rotation = particlePosition;
            lbl_1_bss_1D8[playerIndex].targetRotation = particlePosition;
        }
        break;
    case 14:
        if (lbl_1_bss_1D8[playerIndex].motionSet != 9) {
            lbl_1_bss_1D8[playerIndex].movedDistance = 100.0f;
            lbl_1_bss_1D8[playerIndex].targetRotation.x =
                lbl_1_bss_1D8[playerIndex].targetRotation.y =
                lbl_1_bss_1D8[playerIndex].targetRotation.z = 0.0f;
            fn_1_B958(playerIndex);
            fn_1_296C(playerIndex, 6);
            if (-3.0f < lbl_1_bss_1D8[playerIndex].rotation.x &&
                lbl_1_bss_1D8[playerIndex].rotation.x < 3.0f &&
                -3.0f < lbl_1_bss_1D8[playerIndex].rotation.x &&
                lbl_1_bss_1D8[playerIndex].rotation.x < 3.0f) {
                fn_1_296C(playerIndex, 9);
                lbl_1_bss_1D8[playerIndex].rotation.x =
                    lbl_1_bss_1D8[playerIndex].rotation.y = 0.0f;
            }
        } else {
            lbl_1_bss_1D8[playerIndex].targetRotation.x = 90.0f;
            fn_1_B958(playerIndex);
            if (lbl_1_bss_1D8[playerIndex].motionSet == 9 &&
                CharMotionShiftIDGet(lbl_1_bss_1D8[playerIndex].characterNo) < 0) {
                fn_1_12D7C(playerIndex);
                lbl_1_bss_1D8[playerIndex].status = 15;
                lbl_1_bss_1D8[playerIndex].rotation.x =
                    lbl_1_bss_1D8[playerIndex].targetRotation.x;
                omVibrate(playerIndex, 20, 20, 0);
                motionTime = 1200.0f;
                motionTime = (lbl_1_bss_1D8[playerIndex].position.x - -600.0f) / motionTime;
                motionTime = 32.0f + 64.0f * motionTime;
                M625DebugValue(motionTime);
            }
        }
        break;
    case 15:
        direction.x = lbl_1_bss_1D8[playerIndex].position.x;
        direction.y = lbl_1_bss_1D8[playerIndex].position.y - 200.0f;
        direction.z = 0.0f;
        if (direction.x < -0.00001f || 0.00001f < direction.x ||
            direction.y < -0.00001f || 0.00001f < direction.y) {
            PSVECNormalize(&direction, &direction);
        } else {
            direction.x = 0.0f;
        }
        direction.y = -1.0f;
        direction.z = 0.0f;
        PSVECScale(&direction, &lbl_1_bss_1D8[playerIndex].velocity, 3.0f);
        PSVECScale(&lbl_1_bss_1D8[playerIndex].storedVelocity,
                   &lbl_1_bss_1D8[playerIndex].storedVelocity, 0.2f);
        PSVECAdd(&lbl_1_bss_1D8[playerIndex].velocity,
                 &lbl_1_bss_1D8[playerIndex].storedVelocity,
                 &lbl_1_bss_1D8[playerIndex].velocity);
        if (lbl_1_bss_1D8[playerIndex].position.z + lbl_1_bss_1D8[playerIndex].velocity.z >=
            150.0f) {
            lbl_1_bss_1D8[playerIndex].velocity.z = 100.0f - lbl_1_bss_1D8[playerIndex].position.z;
        }
        lbl_1_bss_1D8[playerIndex].rotation.z += 6.0f;
        Hu3D3Dto2D(&lbl_1_bss_1D8[playerIndex].position, 1, &direction);
        if (direction.x < -200.0f || 840.0f < direction.x ||
            direction.y < -200.0f || 680.0f < direction.y) {
            fn_1_296C(playerIndex, 6);
            lbl_1_bss_1D8[playerIndex].movedDistance = 100.0f;
            lbl_1_bss_1D8[playerIndex].targetRotation.x =
                lbl_1_bss_1D8[playerIndex].targetRotation.y =
                lbl_1_bss_1D8[playerIndex].targetRotation.z = 0.0f;
            lbl_1_bss_1D8[playerIndex].status = 16;
        }
        break;
    case 16:
        break;
    }
}

s32 lbl_1_data_3E4 = 1;

/* Converts a team 1 player's stick direction into target pitch and yaw for fn_1_860C. */
Point3d fn_1_B530(s16 playerIndex)
{
    Point3d result;
    Point3d normalized;

    result.x = result.y = result.z = 0.0f;
    if (lbl_1_bss_1D8[playerIndex].stickX != 0 ||
        lbl_1_bss_1D8[playerIndex].stickY != 0) {
        normalized.x =
            lbl_1_bss_1D8[playerIndex].stickX / 56.0f;
        normalized.y =
            lbl_1_bss_1D8[playerIndex].stickY / 56.0f;
        if (normalized.x < 0.0f) {
            result.x = -1.0f * normalized.x;
        } else {
            result.x = normalized.x;
        }
        if (normalized.y < 0.0f) {
            result.y = -1.0f * normalized.y;
        } else {
            result.y = normalized.y;
        }
        if (result.x > result.y) {
            normalized.x /= result.x;
            normalized.y /= result.x;
        } else if (result.y > 0.0f) {
            normalized.x /= result.y;
            normalized.y /= result.y;
        } else {
            normalized.x = normalized.y = 0.0f;
        }
        if (-0.5f <= normalized.x &&
            normalized.x <= 0.5f) {
            normalized.x = 0.0f;
        } else if (normalized.x < -0.5f) {
            normalized.x = -1.0f;
        } else if (0.5f < normalized.x) {
            normalized.x = 1.0f;
        }
        if (-0.5f <= normalized.y &&
            normalized.y <= 0.5f) {
            normalized.y = 0.0f;
        } else if (normalized.y < -0.5f) {
            normalized.y = -1.0f;
        } else if (0.5f < normalized.y) {
            normalized.y = 1.0f;
        }
        result.y = 90.0f * normalized.x;
        if ((normalized.x < -0.5 ||
             0.5 < normalized.x) &&
            (normalized.y < -0.5 ||
             0.5 < normalized.y)) {
            result.x = 0.5 *
                       (f64)(-90.0f * normalized.y);
        } else {
            result.x = -90.0f * normalized.y;
        }
    }
    return result;
}

/* Turns a team 1 player toward its stick-derived target angles during fn_1_860C. */
void fn_1_B958(s16 playerIndex)
{
    Point3d targetRotation;
    f32 angleStep;

    targetRotation = lbl_1_bss_1D8[playerIndex].targetRotation;
    angleStep = 4.0f;

    if (targetRotation.y > lbl_1_bss_1D8[playerIndex].rotation.y) {
        lbl_1_bss_1D8[playerIndex].rotation.y += angleStep;
    } else if (targetRotation.y < lbl_1_bss_1D8[playerIndex].rotation.y) {
        lbl_1_bss_1D8[playerIndex].rotation.y -= angleStep;
    }
    if (targetRotation.x > lbl_1_bss_1D8[playerIndex].rotation.x) {
        lbl_1_bss_1D8[playerIndex].rotation.x += angleStep;
    } else if (targetRotation.x < lbl_1_bss_1D8[playerIndex].rotation.x) {
        lbl_1_bss_1D8[playerIndex].rotation.x -= angleStep;
    }
    if (lbl_1_bss_1D8[playerIndex].rotation.y > 90.0f) {
        lbl_1_bss_1D8[playerIndex].rotation.y = 90.0f;
    }
    if (lbl_1_bss_1D8[playerIndex].rotation.y < -90.0f) {
        lbl_1_bss_1D8[playerIndex].rotation.y = -90.0f;
    }
    if (lbl_1_bss_1D8[playerIndex].rotation.x > 90.0f) {
        lbl_1_bss_1D8[playerIndex].rotation.x = 90.0f;
    }
    if (lbl_1_bss_1D8[playerIndex].rotation.x < -90.0f) {
        lbl_1_bss_1D8[playerIndex].rotation.x = -90.0f;
    }
    if (-40.0f > lbl_1_bss_1D8[playerIndex].targetRotation.x &&
        lbl_1_bss_1D8[playerIndex].targetRotation.x > -50.0f &&
        -40.0f > lbl_1_bss_1D8[playerIndex].rotation.x &&
        lbl_1_bss_1D8[playerIndex].rotation.x > -50.0f) {
        lbl_1_bss_1D8[playerIndex].rotation.x = -45.0f;
    }
    if (40.0f < lbl_1_bss_1D8[playerIndex].targetRotation.x &&
        lbl_1_bss_1D8[playerIndex].targetRotation.x < 50.0f &&
        40.0f < lbl_1_bss_1D8[playerIndex].rotation.x &&
        lbl_1_bss_1D8[playerIndex].rotation.x < 50.0f) {
        lbl_1_bss_1D8[playerIndex].rotation.x = 45.0f;
    }
    if (-5.0f < lbl_1_bss_1D8[playerIndex].targetRotation.x &&
        lbl_1_bss_1D8[playerIndex].targetRotation.x < 5.0f &&
        -5.0f < lbl_1_bss_1D8[playerIndex].rotation.x &&
        lbl_1_bss_1D8[playerIndex].rotation.x < 5.0f) {
        lbl_1_bss_1D8[playerIndex].rotation.x = 0.0f;
    }
    if (-5.0f < lbl_1_bss_1D8[playerIndex].targetRotation.y &&
        lbl_1_bss_1D8[playerIndex].targetRotation.y < 5.0f &&
        -5.0f < lbl_1_bss_1D8[playerIndex].rotation.y &&
        lbl_1_bss_1D8[playerIndex].rotation.y < 5.0f) {
        lbl_1_bss_1D8[playerIndex].rotation.y = 0.0f;
    }
}

/* Called by movement and results updates to apply transforms and clear per-frame input and
 * velocity. */
void fn_1_BF54(void)
{
    s16 index;

    for (index = 0; index < 4; index++) {
        lbl_1_bss_1D8[index].position.x += lbl_1_bss_1D8[index].velocity.x;
        lbl_1_bss_1D8[index].position.y += lbl_1_bss_1D8[index].velocity.y;
        lbl_1_bss_1D8[index].position.z += lbl_1_bss_1D8[index].velocity.z;
        lbl_1_bss_1D8[index].storedVelocity = lbl_1_bss_1D8[index].velocity;
            lbl_1_bss_1D8[index].velocity.x = lbl_1_bss_1D8[index].velocity.y =
            lbl_1_bss_1D8[index].velocity.z = 0.0f;
        Hu3DModelPosSetV(lbl_1_bss_1D8[index].objectModel,
                         &lbl_1_bss_1D8[index].position);
        Hu3DModelRotSetV(lbl_1_bss_1D8[index].objectModel,
                         &lbl_1_bss_1D8[index].rotation);
        lbl_1_bss_1D8[index].stickX = lbl_1_bss_1D8[index].stickY = 0;
        lbl_1_bss_1D8[index].buttons = lbl_1_bss_1D8[index].buttonsDown = 0;
    }
}

/* Called by fn_1_13E58 to check team 1's finish count; returns 1 when fewer than three remain
 * active. */
s16 fn_1_C19C(void)
{
    s16 activeCount;
    s16 index;

    index = 0;
    activeCount = 0;
    while (index < 4) {
        if (lbl_1_bss_1D8[index].team != 0 &&
            (lbl_1_bss_1D8[index].status == 14 || lbl_1_bss_1D8[index].status == 15 ||
             lbl_1_bss_1D8[index].status == 16)) {
            activeCount++;
        }
        index++;
    }
    if (activeCount >= 3) {
        return 0;
    }
    return 1;
}

/* Called by fn_1_13EF8 when results begin to award coins and record the winning characters. */
void fn_1_C270(s16 mode)
{
    s16 winnerCharacters[4];
    s16 groupIndex;
    s32 playerIndex;
    s32 alternatePlayerIndex;

    lbl_1_bss_1D4 = 0;
    for (groupIndex = 0; groupIndex < 4; groupIndex++) {
        winnerCharacters[groupIndex] = -1;
    }

    for (groupIndex = 0; groupIndex < 4; groupIndex++) {
        if (mode == 0) {
            if (lbl_1_bss_1D8[groupIndex].team == 0) {
                playerIndex = lbl_1_bss_1D8[groupIndex].playerIndex;
                if (!_CheckFlag(FLAG_MG_PRACTICE)) {
                    GwPlayer[playerIndex].mgCoinBonus = 10;
                }
                winnerCharacters[groupIndex] = lbl_1_bss_1D8[groupIndex].characterNo;
            }
        } else if (lbl_1_bss_1D8[groupIndex].team == 1) {
            alternatePlayerIndex = lbl_1_bss_1D8[groupIndex].playerIndex;
            if (!_CheckFlag(FLAG_MG_PRACTICE)) {
                GwPlayer[alternatePlayerIndex].mgCoinBonus = 10;
            }
            winnerCharacters[groupIndex] = lbl_1_bss_1D8[groupIndex].characterNo;
        }
    }

    MgSeqWinnerSet(winnerCharacters[0], winnerCharacters[1], winnerCharacters[2],
                   winnerCharacters[3]);
}

/* Called by fn_1_13EF8 to move players into result positions; returns when their pose transition is
 * complete. */
s32 fn_1_C438(void) {
    Point3d motionDelta;
    s16 activePlayers[3];
    s16 sequenceZeroOverlay;
    s16 sequenceZeroMotion;
    s16 sequenceThreeOverlay;
    s16 sequenceZeroMotionId;
    s16 sequenceZeroMotionId2;
    s16 sequenceThreeMotionId;
    s16 eligibleCount;
    s16 motionCount;
    s16 index;
    s16 positionIndex;
    s16 playerIndex;

    fn_1_11698();
    if (lbl_1_bss_814 == 0) {
        playerIndex = 0;
for (; playerIndex < 4; playerIndex += 1) {
            if (lbl_1_bss_1D8[playerIndex].team == 0) {
                lbl_1_bss_1D8[playerIndex].status = 1;
                lbl_1_data_2B8[0].overlayMotionId = -1;
                lbl_1_data_2B8[1].overlayMotionId = -1;
                lbl_1_data_2B8[2].overlayMotionId = -1;
                lbl_1_data_2B8[0].modelAttributes = HU3D_MOTATTR_LOOP;
                lbl_1_data_2B8[1].modelAttributes = HU3D_MOTATTR_LOOP;
                lbl_1_data_2B8[2].modelAttributes = HU3D_MOTATTR_LOOP;
                lbl_1_bss_1D8[playerIndex].modelScale -= 0.1f;
                if (lbl_1_bss_1D8[playerIndex].modelScale <= 0.0f) {
                    lbl_1_bss_1D8[playerIndex].modelScale = 0.0f;
                }
                Hu3DModelScaleSet(
                    lbl_1_bss_1D8[playerIndex].scaleModel, lbl_1_bss_1D8[playerIndex].modelScale,
                    lbl_1_bss_1D8[playerIndex].modelScale, lbl_1_bss_1D8[playerIndex].modelScale);
                if (lbl_1_bss_1D8[playerIndex].modelScale <= 0.0f) {
                    if ((s32) lbl_1_data_3E4 != 0) {
                        lbl_1_bss_1D8[playerIndex].motionSet = -1;
                        lbl_1_data_3E4 = 0;
                    } else {
                        index = 0;
                        PSVECSubtract(&lbl_1_data_248[index], &lbl_1_bss_1D8[playerIndex].position,
                                      &motionDelta);
                        if (PSVECMag(&motionDelta) >= 10.0f) {
                            PSVECNormalize(&motionDelta, &motionDelta);
                            PSVECScale(&motionDelta, &motionDelta, 56.0f);

                            lbl_1_bss_1D8[playerIndex].stickX = (s32)motionDelta.x;
                            lbl_1_bss_1D8[playerIndex].stickY = (s32)motionDelta.y;
                        }
                    }
                }
            }}
    } else {
        lbl_1_bss_1D0 = 0;
        playerIndex = 0;
        index = 0;
for (; playerIndex < 4; playerIndex += 1) {
            if (lbl_1_bss_1D8[playerIndex].team == 0) {
                lbl_1_bss_1D8[playerIndex].modelScale -= 0.1f;
                if (lbl_1_bss_1D8[playerIndex].modelScale <= 0.0f) {
                    lbl_1_bss_1D8[playerIndex].modelScale = 0.0f;
                }
                Hu3DModelScaleSet(
                    lbl_1_bss_1D8[playerIndex].scaleModel, lbl_1_bss_1D8[playerIndex].modelScale,
                    lbl_1_bss_1D8[playerIndex].modelScale, lbl_1_bss_1D8[playerIndex].modelScale);
                if (lbl_1_bss_1D8[playerIndex].modelScale <= 0.0f) {
                    lbl_1_bss_1D8[playerIndex].status = 0;
                    if (lbl_1_bss_1D8[playerIndex].motionSet != 3 &&
                        lbl_1_data_2B8[0].overlayMotionId != -1) {
                        fn_1_296C(playerIndex, 3);
                    } else if (lbl_1_data_2B8[0].overlayMotionId != -1) {
                        lbl_1_bss_1D8[playerIndex].motionSet = -1;
                        lbl_1_data_2B8[0].overlayMotionId = -1;
                        lbl_1_data_2B8[0].modelAttributes = HU3D_MOTATTR_LOOP;
                        fn_1_296C(playerIndex, 0);
                    }
                }
            }
            if (lbl_1_bss_1D8[playerIndex].team == 1) {
                (&activePlayers[0])[index] = playerIndex;
                index += 1;
                if (lbl_1_bss_1D8[playerIndex].status == 13 &&
                    lbl_1_bss_1D8[playerIndex].motionSet != 9 &&
                    CharMotionShiftIDGet(lbl_1_bss_1D8[playerIndex].characterNo) < 0) {
                    lbl_1_bss_1D8[playerIndex].status = 1;
                    motionDelta.x = motionDelta.y = motionDelta.z = 0.0f;

                    lbl_1_bss_1D8[playerIndex].rotation = motionDelta;
                    lbl_1_bss_1D8[playerIndex].targetRotation = motionDelta;
                }
                if (lbl_1_bss_1D8[playerIndex].status == 11 ||
                    lbl_1_bss_1D8[playerIndex].status == 12 ||
                    lbl_1_bss_1D8[playerIndex].status == 13) {
                    lbl_1_bss_1D8[playerIndex].status = 13;
                }
            }}
        playerIndex = 0;
for (; playerIndex < 3; playerIndex += 1) {
            index = playerIndex + 1;
for (; index < 3; index += 1) {
    if (lbl_1_bss_1D8[(&activePlayers[0])[playerIndex]].position.x >
        lbl_1_bss_1D8[(&activePlayers[0])[index]].position.x) {
        eligibleCount = (&activePlayers[0])[playerIndex];
        (&activePlayers[0])[playerIndex] = (&activePlayers[0])[index];
        (&activePlayers[0])[index] = eligibleCount;
    }
}
}
        playerIndex = 0;
        eligibleCount = 0;
for (; playerIndex < 3; playerIndex += 1) {
    if (lbl_1_bss_1D8[activePlayers[playerIndex]].status != 14 &&
        lbl_1_bss_1D8[activePlayers[playerIndex]].status != 15 &&
        lbl_1_bss_1D8[activePlayers[playerIndex]].status != 16) {
        eligibleCount += 1;
    } else {
        (&activePlayers[0])[playerIndex] = -1;
    }
}
        if (eligibleCount <= 1) {
            playerIndex = 0;
for (; playerIndex < 3; playerIndex += 1) {
                if (activePlayers[playerIndex] >= 0 && playerIndex != 1) {
                    activePlayers[1] = (&activePlayers[0])[playerIndex];
                    (&activePlayers[0])[playerIndex] = -1;
                }}
        } else if (eligibleCount == 2) {
            playerIndex = 0;
            index = 0;
for (; playerIndex < 3; playerIndex += 1) {
                if (activePlayers[playerIndex] >= 0) {
                    index += 1;
                    if ((playerIndex != 0) && (playerIndex != 2)) {
                        if (index == 1) {
                            activePlayers[0] = (&activePlayers[0])[playerIndex];
                        } else {
                            activePlayers[2] = (&activePlayers[0])[playerIndex];
                        }
                        (&activePlayers[0])[playerIndex] = -1;
                    }
                }}
        }
        playerIndex = 0;
for (; playerIndex < 3; playerIndex += 1) {
            if (activePlayers[playerIndex] >= 0) {
                PSVECSubtract(&lbl_1_data_248[playerIndex + 1],
                              &lbl_1_bss_1D8[(&activePlayers[0])[playerIndex]].position,
                              &motionDelta);
                if (PSVECMag(&motionDelta) > 20.0f) {
                    PSVECNormalize(&motionDelta, &motionDelta);
                    PSVECScale(&motionDelta, &motionDelta, 56.0f);

                    lbl_1_bss_1D8[(&activePlayers[0])[playerIndex]].stickX = motionDelta.x;
                    lbl_1_bss_1D8[(&activePlayers[0])[playerIndex]].stickY = motionDelta.y;
                }
                lbl_1_bss_1D8[(&activePlayers[0])[playerIndex]].buttonsDown = PAD_BUTTON_A;
            }}
    }
    fn_1_5108();
    motionCount = 4;
    if (lbl_1_bss_814 == 0) {
        playerIndex = 0;
for (; playerIndex < 4; playerIndex += 1) {
            index = 0;
            if (lbl_1_bss_1D8[playerIndex].team == 0) {
                PSVECSubtract(&lbl_1_data_248[index], &lbl_1_bss_1D8[playerIndex].position,
                              &motionDelta);
                if (PSVECMag(&motionDelta) < 10.0f) {
                    fn_1_296C(playerIndex, 0);
                    lbl_1_bss_1D8[playerIndex].targetRotation.x =
                    lbl_1_bss_1D8[playerIndex].targetRotation.y =
                    lbl_1_bss_1D8[playerIndex].targetRotation.z = 0.0f;
                    lbl_1_bss_1D8[playerIndex].movedDistance = 100.0f;
                    lbl_1_bss_1D8[playerIndex].moveSpeed = 0.0f;
                    motionCount = 0;
                }
            }}
    } else {
        playerIndex = 0;
for (; playerIndex < 3; playerIndex += 1) {
            if (activePlayers[playerIndex] < 0) {
                motionCount -= 1;
                if (motionCount <= 1) {
                    motionCount = 0;
                }
            } else {
                PSVECSubtract(&lbl_1_data_248[playerIndex + 1],
                              &lbl_1_bss_1D8[(&activePlayers[0])[playerIndex]].position,
                              &motionDelta);
                if (PSVECMag(&motionDelta) <= 20.0f) {
                    lbl_1_bss_1D8[(&activePlayers[0])[playerIndex]].targetRotation.x =
                    lbl_1_bss_1D8[(&activePlayers[0])[playerIndex]].targetRotation.y =
                    lbl_1_bss_1D8[(&activePlayers[0])[playerIndex]].targetRotation.z = 0.0f;
                    lbl_1_bss_1D8[(&activePlayers[0])[playerIndex]].movedDistance = 100.0f;
                    lbl_1_bss_1D8[(&activePlayers[0])[playerIndex]].moveSpeed = 0.0f;
                    motionCount -= 1;
                    if (motionCount <= 1) {
                        motionCount = 0;
                    }
                }
            }}
    }
    if (motionCount == 0) {
        playerIndex = 0;
for (; playerIndex < 4; playerIndex += 1) {
            if (lbl_1_bss_814 == 0) {
                CharModelVoiceFlagSet(lbl_1_bss_1D8[playerIndex].characterNo, 1);
                if (lbl_1_bss_1D8[playerIndex].team == 0) {
                    lbl_1_bss_1D8[playerIndex].rotation.y =
                        0.7f * lbl_1_bss_1D8[playerIndex].rotation.y;
                    if ((-2.0f <= lbl_1_bss_1D8[playerIndex].rotation.y) &&
                        (lbl_1_bss_1D8[playerIndex].rotation.y <= 2.0f)) {
                        lbl_1_bss_1D8[playerIndex].rotation.y = 0.0f;
                        motionCount = -4;
                    }
                }
            }
            if (lbl_1_bss_814 == 1) {
                if (lbl_1_bss_1D8[playerIndex].team == 0) {
                    lbl_1_bss_1D8[playerIndex].rotation.y =
                        0.7f * lbl_1_bss_1D8[playerIndex].rotation.y;
                    if ((-2.0f <= lbl_1_bss_1D8[playerIndex].rotation.y) &&
                        (lbl_1_bss_1D8[playerIndex].rotation.y <= 2.0f)) {
                        lbl_1_bss_1D8[playerIndex].rotation.y = 0.0f;
                        motionCount -= 1;
                    }
                } else if ((-2.0f < lbl_1_bss_1D8[playerIndex].rotation.x) &&
                           (lbl_1_bss_1D8[playerIndex].rotation.x < 2.0f) &&
                           (-2.0f < lbl_1_bss_1D8[playerIndex].rotation.y) &&
                           (lbl_1_bss_1D8[playerIndex].rotation.y < 2.0f)) {
                    motionCount -= 1;
                }
                lbl_1_bss_1D8[playerIndex].targetRotation.x =
                lbl_1_bss_1D8[playerIndex].targetRotation.y =
                lbl_1_bss_1D8[playerIndex].targetRotation.z = 0.0f;
                lbl_1_bss_1D8[playerIndex].movedDistance = 100.0f;
                lbl_1_bss_1D8[playerIndex].moveSpeed = 0.0f;
            }}
        positionIndex = 0;
for (; positionIndex < 4; positionIndex += 1) {
            lbl_1_bss_1D8[positionIndex].position.x += lbl_1_bss_1D8[positionIndex].velocity.x;
            lbl_1_bss_1D8[positionIndex].position.y += lbl_1_bss_1D8[positionIndex].velocity.y;
            lbl_1_bss_1D8[positionIndex].position.z += lbl_1_bss_1D8[positionIndex].velocity.z;
            lbl_1_bss_1D8[positionIndex].storedVelocity = lbl_1_bss_1D8[positionIndex].velocity;
            lbl_1_bss_1D8[positionIndex].velocity.x =
            lbl_1_bss_1D8[positionIndex].velocity.y =
            lbl_1_bss_1D8[positionIndex].velocity.z = 0.0f;
            Hu3DModelPosSetV(lbl_1_bss_1D8[positionIndex].objectModel,
                             &lbl_1_bss_1D8[positionIndex].position);
            Hu3DModelRotSetV(lbl_1_bss_1D8[positionIndex].objectModel,
                             &lbl_1_bss_1D8[positionIndex].rotation);
            lbl_1_bss_1D8[positionIndex].stickX =
            lbl_1_bss_1D8[positionIndex].stickY = 0;
            lbl_1_bss_1D8[positionIndex].buttons =
            lbl_1_bss_1D8[positionIndex].buttonsDown = 0;}
        if (motionCount >= -4) {
            return 1;
        }
    }
    /* Keep the results transition active until the players finish turning into position. */
    return 0;
}

/* Advances victory or defeat motions during the results sequence and commits the player
 * transforms. */
void fn_1_DE68(void)
{
    s16 index;
    f32 maximumTime;

    for (index = 0; index < 4; index++) {
        if (lbl_1_bss_1D8[index].motionSet != 4 &&
            lbl_1_bss_1D8[index].motionSet != 5 &&
            lbl_1_bss_1D8[index].motionSet != 8) {
            if (lbl_1_bss_814 == 0 &&
                lbl_1_bss_1D8[index].team == 0) {
                fn_1_296C(index, 4);
                lbl_1_bss_1D8[index].rotation.y = 0.0f;
            }
            if (lbl_1_bss_814 == 1) {
                if (lbl_1_bss_1D8[index].team == 0) {
                    fn_1_296C(index, 5);
                    lbl_1_bss_1D8[index].rotation.y = 0.0f;
                } else {
                    if (lbl_1_bss_1D8[index].status != 14 &&
                        lbl_1_bss_1D8[index].status != 15 &&
                        lbl_1_bss_1D8[index].status != 16) {
                        fn_1_296C(index, 8);
                    }
                    lbl_1_bss_1D8[index].rotation.x =
                        lbl_1_bss_1D8[index].rotation.y = 0.0f;
                }
            }
        } else {
            if ((lbl_1_bss_1D8[index].motionSet == 4 ||
                 lbl_1_bss_1D8[index].motionSet == 5) &&
                CharMotionShiftIDGet(lbl_1_bss_1D8[index].characterNo) < 0) {
                maximumTime = CharMotionMaxTimeGet(
                    lbl_1_bss_1D8[index].characterNo);
                if (Hu3DData[lbl_1_bss_1D8[index].objectModel].motWork.speed +
                    CharMotionTimeGet(lbl_1_bss_1D8[index].characterNo) >= maximumTime) {
                    CharModelAttrSet(lbl_1_bss_1D8[index].characterNo,
                                      HU3D_MOTATTR_PAUSE);
                }
            }
            if (lbl_1_bss_1D8[index].motionSet == 8) {
                maximumTime = Hu3DMotionMaxTimeGet(
                    lbl_1_bss_1D8[index].objectModel);
                if (1.0f + Hu3DMotionTimeGet(
                    lbl_1_bss_1D8[index].objectModel) >= maximumTime) {
                    Hu3DMotionTimeSet(lbl_1_bss_1D8[index].objectModel,
                                      35.0f);
                }
            }
        }
    }

    fn_1_BF54();
}

/* Copies controller sticks and buttons into player state, applying deadzones and the minigame stick
 * limit. */
void fn_1_EB00(void)
{
    s16 playerIndex;
    s16 xDeadzone;
    s16 yDeadzone;

    xDeadzone = 6;
    yDeadzone = 6;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        lbl_1_bss_1D8[playerIndex].stickX =
            lbl_1_bss_1D8[playerIndex].stickY = 0;
        lbl_1_bss_1D8[playerIndex].buttons =
            lbl_1_bss_1D8[playerIndex].buttonsDown = 0;
        lbl_1_bss_1D8[playerIndex].stickX =
            HuPadStkX[lbl_1_bss_1D8[playerIndex].controllerIndex];
        if (-xDeadzone < lbl_1_bss_1D8[playerIndex].stickX &&
            lbl_1_bss_1D8[playerIndex].stickX < xDeadzone) {
            lbl_1_bss_1D8[playerIndex].stickX = 0;
        }
        if (lbl_1_bss_1D8[playerIndex].stickX > 56) {
            lbl_1_bss_1D8[playerIndex].stickX = 56;
        }
        if (lbl_1_bss_1D8[playerIndex].stickX < -56) {
            lbl_1_bss_1D8[playerIndex].stickX = -56;
        }
        lbl_1_bss_1D8[playerIndex].stickY =
            HuPadStkY[lbl_1_bss_1D8[playerIndex].controllerIndex];
        if (-yDeadzone < lbl_1_bss_1D8[playerIndex].stickY &&
            lbl_1_bss_1D8[playerIndex].stickY < yDeadzone) {
            lbl_1_bss_1D8[playerIndex].stickY = 0;
        }
        if (lbl_1_bss_1D8[playerIndex].stickY > 56) {
            lbl_1_bss_1D8[playerIndex].stickY = 56;
        }
        if (lbl_1_bss_1D8[playerIndex].stickY < -56) {
            lbl_1_bss_1D8[playerIndex].stickY = -56;
        }
        lbl_1_bss_1D8[playerIndex].buttons =
            HuPadBtn[lbl_1_bss_1D8[playerIndex].controllerIndex];
        lbl_1_bss_1D8[playerIndex].buttonsDown =
            HuPadBtnDown[lbl_1_bss_1D8[playerIndex].controllerIndex];
    }
}

/* Updates effect motion flags for each team 0 player's X position and velocity during fn_1_5108. */
void fn_1_EE94(void) {
    s16 playerIndex;
    playerIndex = 0;
    while (playerIndex < 4) {
        if (lbl_1_bss_1D8[playerIndex].team == 0) {
            fn_1_1C58(lbl_1_bss_1D8[playerIndex].position.x,
                      lbl_1_bss_1D8[playerIndex].velocity.x);
        }
        playerIndex += 1;
    }
}

s16 fn_1_EF24(s16 playerIndex) {
    return lbl_1_bss_1D8[playerIndex].timer;
}

void fn_1_EF40(void) {
    lbl_1_bss_1CC = 1;
}

/* Called by fn_1_11040 to request rumble for each team 0 player during the water effect update. */
void fn_1_EF54(void) {
    s16 playerIndex;

    playerIndex = 0;
    while (playerIndex < 4) {
        if (lbl_1_bss_1D8[playerIndex].team == 0) {
            omVibrate(playerIndex, 20, 7, 3);
        }
        playerIndex += 1;
    }
}

/* Called by fn_1_2D48 to clear targets, target flags and selection timers for all players. */
void fn_1_EFC4(void)
{
    s16 index;

    index = 0;
    while (index < 4) {
        lbl_1_bss_1D8[index].target.x =
            lbl_1_bss_1D8[index].target.y =
            lbl_1_bss_1D8[index].target.z = 0.0f;
        lbl_1_bss_1D8[index].hasTarget = 0;
        lbl_1_bss_1D8[index].updateFrames = 0;
        index++;
    }
}

/* Called by fn_1_4E80 to rebuild computer-controlled players' input for the current gameplay
 * frame. */
void fn_1_F07C(void)
{
    s16 index;

    index = 0;
    while (index < 4) {
        if (lbl_1_bss_1D8[index].computerPlayer != 0) {
            lbl_1_bss_1D8[index].stickX = lbl_1_bss_1D8[index].stickY = 0;
            lbl_1_bss_1D8[index].buttons = lbl_1_bss_1D8[index].buttonsDown = 0;
            if (lbl_1_bss_1D8[index].team == 0) {
                fn_1_F174(index);
            } else {
                fn_1_F900(index);
            }
        }
        index++;
    }
}

/* Called by fn_1_F07C to choose team 0 computer-player targets and button presses. */
void fn_1_F174(s16 playerIndex)
{
    f32 targetDistance;
    f32 playerDistance;
    s16 targetChance;
    s16 nearbyChance;
    s16 stopChance;
    s16 closestPlayer;
    s16 candidatePlayer;
    s16 chanceRoll;

    switch (lbl_1_bss_1D8[playerIndex].difficulty) {
    case 0:
        targetChance = 0;
        nearbyChance = 20;
        stopChance = 2;
        break;
    case 1:
        targetChance = 35;
        nearbyChance = 40;
        stopChance = 3;
        break;
    case 2:
        targetChance = 70;
        nearbyChance = 60;
        stopChance = 4;
        break;
    case 3:
        targetChance = 100;
        nearbyChance = 100;
        stopChance = 4;
        break;
    }

    closestPlayer = -1;
    targetDistance = 8000.0f;
    for (candidatePlayer = 0; candidatePlayer < 4; candidatePlayer++) {
        if (lbl_1_bss_1D8[candidatePlayer].team != 0 &&
            (lbl_1_bss_1D8[candidatePlayer].status == 11 ||
             lbl_1_bss_1D8[candidatePlayer].status == 12 ||
             lbl_1_bss_1D8[candidatePlayer].status == 13)) {
            playerDistance = lbl_1_bss_1D8[candidatePlayer].position.x;
            if (playerDistance < 0.0f) {
                playerDistance *= -1.0f;
            }
            if (targetDistance > playerDistance) {
                targetDistance = playerDistance;
                closestPlayer = candidatePlayer;
            }
        }
    }

    chanceRoll = (u8)frandmod(100);
    if (chanceRoll < targetChance) {
        if (closestPlayer >= 0 && lbl_1_bss_1D8[playerIndex].hasTarget == 0) {
            lbl_1_bss_1D8[playerIndex].target =
                lbl_1_bss_1D8[closestPlayer].position;
            lbl_1_bss_1D8[playerIndex].hasTarget = 1;
        }
        if ((lbl_1_bss_1D8[playerIndex].difficulty == 2 ||
             lbl_1_bss_1D8[playerIndex].difficulty == 3) && closestPlayer >= 0) {
            lbl_1_bss_1D8[playerIndex].target =
                lbl_1_bss_1D8[closestPlayer].position;
            lbl_1_bss_1D8[playerIndex].hasTarget = 1;
        }
    }

    if (lbl_1_bss_1D8[playerIndex].hasTarget != 0) {
        targetDistance = lbl_1_bss_1D8[playerIndex].target.x -
                     lbl_1_bss_1D8[playerIndex].position.x;
        if (targetDistance < 0.0f) {
            targetDistance *= -1.0f;
        }
        if (targetDistance < 10.0f) {
            lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
            lbl_1_bss_1D8[playerIndex].hasTarget = 0;
        }
        targetDistance = lbl_1_bss_1D8[playerIndex].target.x -
                     lbl_1_bss_1D8[playerIndex].position.x;
        if (targetDistance < 0.0f) {
            lbl_1_bss_1D8[playerIndex].stickX = -56;
        }
        if (targetDistance > 0.0f) {
            lbl_1_bss_1D8[playerIndex].stickX = 56;
        }
    } else {
        chanceRoll = (u8)frandmod(100);
        if (chanceRoll < nearbyChance) {
            candidatePlayer = 0;
            closestPlayer = -1;
            for (; closestPlayer == -1 && candidatePlayer < 3; candidatePlayer++) {
                closestPlayer = frand() & 0x3;
                if (lbl_1_bss_1D8[closestPlayer].status == 14 ||
                    lbl_1_bss_1D8[closestPlayer].status == 15 ||
                    lbl_1_bss_1D8[closestPlayer].status == 16) {
                    closestPlayer = -1;
                }
            }
            if (closestPlayer >= 0) {
                lbl_1_bss_1D8[playerIndex].target =
                    lbl_1_bss_1D8[closestPlayer].position;
                lbl_1_bss_1D8[playerIndex].hasTarget = 1;
            }
        } else {
            lbl_1_bss_1D8[playerIndex].target.y =
                lbl_1_bss_1D8[playerIndex].target.z = 0.0f;
            if (((u8)frand() & 0x7F) < 64) {
                lbl_1_bss_1D8[playerIndex].target.x =
                    ((u8)frand() & 0xFF) / 255.0f;
                lbl_1_bss_1D8[playerIndex].target.x *= 550.0f;
            } else {
                lbl_1_bss_1D8[playerIndex].target.x =
                    ((u8)frand() & 0xFF) / 255.0f;
                lbl_1_bss_1D8[playerIndex].target.x *= -550.0f;
            }
            lbl_1_bss_1D8[playerIndex].hasTarget = 1;
        }
    }

    lbl_1_bss_1D8[playerIndex].updateFrames++;
    if (lbl_1_bss_1D8[playerIndex].updateFrames >= 120) {
        candidatePlayer = fn_1_12700();
        if (candidatePlayer < stopChance) {
            lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
        }
        lbl_1_bss_1D8[playerIndex].updateFrames = 0;
    }
}

/* Called by fn_1_F07C to steer team 1 computer players around obstacles and team 0 during play. */
void fn_1_F900(s16 playerIndex)
{
    Point3d obstaclePosition;
    Point3d relativePosition;
    Point3d nearestPosition;
    f32 nearestDistance;
    f32 highY;
    f32 lowY;
    f32 lowX;
    f32 highX;
    f32 lowerMotionLimit;
    f32 upperMotionLimit;
    f32 nearLimit;
    f32 farLimit;
    s16 obstacleCount;
    s16 obstacleIndex;
    s16 nearbyPlayer;
    s16 yChance;
    s16 xChance;
    s16 edgeChance;
    s16 nearbyChance;
    s16 foundObstacle;

    switch (lbl_1_bss_1D8[playerIndex].difficulty) {
    case 0:
        edgeChance = 100;
        nearbyChance = 0;
        yChance = 10;
        xChance = 20;
        nearLimit = 10.0f;
        farLimit = 10.0f;
        lowerMotionLimit = 0.3f;
        upperMotionLimit = 0.7f;
        break;
    case 1:
        edgeChance = 100;
        nearbyChance = 100;
        yChance = 20;
        xChance = 40;
        nearLimit = 40.0f;
        farLimit = 40.0f;
        lowerMotionLimit = 0.25f;
        upperMotionLimit = 0.7f;
        break;
    case 2:
        edgeChance = 0;
        nearbyChance = 100;
        yChance = 50;
        xChance = 100;
        nearLimit = 100.0f;
        farLimit = 100.0f;
        lowerMotionLimit = 0.2f;
        upperMotionLimit = 0.7f;
        break;
    case 3:
        edgeChance = 0;
        nearbyChance = 100;
        yChance = 50;
        xChance = 100;
        nearLimit = 100.0f;
        farLimit = 100.0f;
        lowerMotionLimit = 0.2f;
        upperMotionLimit = 0.8f;
        break;
    }

    obstacleCount = frandmod(100);
    if (obstacleCount < nearbyChance) {
        foundObstacle = 0;
        nearestPosition.x = nearestPosition.y = nearestPosition.z = 0.0f;
        nearestDistance = 60000.0f;
        obstacleCount = fn_1_12760();
        for (obstacleIndex = 0; obstacleIndex < obstacleCount; obstacleIndex++) {
            if (0 != fn_1_12768(obstacleIndex)) {
                fn_1_127B0(obstacleIndex, &obstaclePosition);
                PSVECSubtract(&obstaclePosition, &lbl_1_bss_1D8[playerIndex].position,
                              &relativePosition);
                if (nearestDistance > PSVECMag(&relativePosition)) {
                    nearestPosition = relativePosition;
                    nearestDistance = PSVECMag(&relativePosition);
                    foundObstacle = 1;
                }
            }
        }

        if (foundObstacle != 0) {
            if (280.0f >= PSVECMag(&nearestPosition)) {
                if (0.0f < nearestPosition.y && nearestPosition.y < 250.0f) {
                    lbl_1_bss_1D8[playerIndex].stickY = -56;
                    lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
                }
                if (0.0f > nearestPosition.y && nearestPosition.y > -250.0f) {
                    lbl_1_bss_1D8[playerIndex].stickY = 56;
                    lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
                }
                if (0.0f < nearestPosition.x && nearestPosition.x < 250.0f) {
                    lbl_1_bss_1D8[playerIndex].stickX = -56;
                    lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
                }
                if (0.0f > nearestPosition.x && nearestPosition.x > -250.0f) {
                    lbl_1_bss_1D8[playerIndex].stickX = 56;
                    lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
                }
            }
            /* These Y bounds do not overlap, so this branch never steers the CPU. */
            if (-200.0f < nearestPosition.x && nearestPosition.x < 200.0f &&
                nearestPosition.y < -60.0f && 230.0f < nearestPosition.y) {
                if (0.0f < lbl_1_bss_1D8[playerIndex].position.x) {
                    lbl_1_bss_1D8[playerIndex].stickX = -56;
                } else {
                    lbl_1_bss_1D8[playerIndex].stickX = 56;
                }
                lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
            }

            for (obstacleIndex = 0; obstacleIndex < 4; obstacleIndex++) {
                if (lbl_1_bss_1D8[obstacleIndex].team == 0) {
                    nearbyPlayer = obstacleIndex;
                    break;
                }
            }
            if (lbl_1_bss_1D8[nearbyPlayer].position.x < -300.0 ||
                300.0 < lbl_1_bss_1D8[nearbyPlayer].position.x) {
                highY = 200.0f + lbl_1_bss_1D8[playerIndex].position.y;
                lowY = lbl_1_bss_1D8[playerIndex].position.y - 60.0f;
                lowX = lbl_1_bss_1D8[playerIndex].position.x - 140.0f;
                highX = 140.0f + lbl_1_bss_1D8[playerIndex].position.x;
                foundObstacle = 0;
                obstacleCount = fn_1_12760();
                for (obstacleIndex = 0; obstacleIndex < obstacleCount; obstacleIndex++) {
                    if (0 != fn_1_12768(obstacleIndex)) {
                        fn_1_127B0(obstacleIndex, &relativePosition);
                        if (fn_1_13AA8(highY, lowY, lowX, highX, relativePosition) != 0) {
                            foundObstacle++;
                        }
                    }
                }
                if (foundObstacle == 0) {
                    if (0.0f < lbl_1_bss_1D8[nearbyPlayer].position.x) {
                        lbl_1_bss_1D8[playerIndex].stickX = -56;
                        lbl_1_data_3E8[playerIndex] = -1;
                    } else {
                        lbl_1_bss_1D8[playerIndex].stickX = 56;
                        lbl_1_data_3E8[playerIndex] = 1;
                    }
                    lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
                }
            }
        } else {
            if (lbl_1_bss_1D8[playerIndex].difficulty != 3) {
                goto randomSteering;
            }
            for (obstacleIndex = 0; obstacleIndex < 4; obstacleIndex++) {
                if (lbl_1_bss_1D8[obstacleIndex].team == 0) {
                    nearbyPlayer = obstacleIndex;
                    break;
                }
            }
            obstacleCount = frandmod(100);
            if (lbl_1_bss_1D8[nearbyPlayer].position.x <
                    lbl_1_bss_1D8[playerIndex].position.x - 250.0f ||
                250.0f + lbl_1_bss_1D8[playerIndex].position.x <
                    lbl_1_bss_1D8[nearbyPlayer].position.x) {
                lbl_1_bss_1D8[playerIndex].stickY = 56;
                lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
            } else if (obstacleCount < 50) {
                lbl_1_bss_1D8[playerIndex].stickY = -56;
                lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
            }
            obstacleCount = frandmod(100);
            if (obstacleCount < yChance) {
                lbl_1_bss_1D8[playerIndex].stickX = -56;
                lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
            } else if (obstacleCount < xChance) {
                lbl_1_bss_1D8[playerIndex].stickX = 56;
                lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
            } else {
                lbl_1_bss_1D8[playerIndex].stickX = 0;
                lbl_1_bss_1D8[playerIndex].buttonsDown = 0;
            }
        }

        if (lbl_1_data_3E8[playerIndex] < 0) {
            lbl_1_bss_1D8[playerIndex].stickX = -56;
            lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
            lbl_1_data_3E8[playerIndex]--;
            if (lbl_1_data_3E8[playerIndex] < -150) {
                lbl_1_data_3E8[playerIndex] = 0;
            }
        }
        if (lbl_1_data_3E8[playerIndex] > 0) {
            lbl_1_bss_1D8[playerIndex].stickX = 56;
            lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
            lbl_1_data_3E8[playerIndex]++;
            if (lbl_1_data_3E8[playerIndex] > 150) {
                lbl_1_data_3E8[playerIndex] = 0;
            }
        }
    } else {
randomSteering:
        obstacleCount = frandmod(100);
        if (obstacleCount < yChance) {
            lbl_1_bss_1D8[playerIndex].stickY = -56;
            lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
        } else if (obstacleCount < xChance) {
            lbl_1_bss_1D8[playerIndex].stickY = 56;
            lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
        } else {
            lbl_1_bss_1D8[playerIndex].stickY = 0;
            lbl_1_bss_1D8[playerIndex].buttonsDown = 0;
        }
        obstacleCount = frandmod(100);
        if (obstacleCount < yChance) {
            lbl_1_bss_1D8[playerIndex].stickX = -56;
            lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
        } else if (obstacleCount < xChance) {
            lbl_1_bss_1D8[playerIndex].stickX = 56;
            lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
        } else {
            lbl_1_bss_1D8[playerIndex].stickX = 0;
            lbl_1_bss_1D8[playerIndex].buttonsDown = 0;
        }
    }

    if (lbl_1_bss_1D8[playerIndex].difficulty != 3) {
        obstacleCount = frandmod(100);
        if (obstacleCount < nearLimit &&
            lbl_1_bss_1D8[playerIndex].timer < 600.0f * lowerMotionLimit) {
            lbl_1_bss_1D8[playerIndex].stickY = 56;
            lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
        }
        obstacleCount = frandmod(100);
        if (obstacleCount < farLimit &&
            lbl_1_bss_1D8[playerIndex].timer > 600.0f * upperMotionLimit) {
            lbl_1_bss_1D8[playerIndex].stickY = -56;
            lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
        }
    } else {
        for (obstacleIndex = 0; obstacleIndex < 4; obstacleIndex++) {
            if (lbl_1_bss_1D8[obstacleIndex].team == 0) {
                nearbyPlayer = obstacleIndex;
                break;
            }
        }
        if (lbl_1_bss_1D8[nearbyPlayer].position.x <
                lbl_1_bss_1D8[playerIndex].position.x - 350.0f ||
            350.0f + lbl_1_bss_1D8[playerIndex].position.x <
                lbl_1_bss_1D8[nearbyPlayer].position.x) {
            highY = 300.0f + lbl_1_bss_1D8[playerIndex].position.y;
            lowY = lbl_1_bss_1D8[playerIndex].position.y - 60.0f;
            lowX = lbl_1_bss_1D8[playerIndex].position.x - 200.0f;
            highX = 200.0f + lbl_1_bss_1D8[playerIndex].position.x;
            foundObstacle = 0;
            obstacleCount = fn_1_12760();
            for (obstacleIndex = 0; obstacleIndex < obstacleCount; obstacleIndex++) {
                if (0 != fn_1_12768(obstacleIndex)) {
                    fn_1_127B0(obstacleIndex, &relativePosition);
                    if (fn_1_13AA8(highY, lowY, lowX, highX, relativePosition) != 0) {
                        foundObstacle++;
                    }
                }
            }
            if (foundObstacle == 0) {
                lbl_1_bss_1D8[playerIndex].stickY = 56;
                lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
            }
        } else {
            lbl_1_bss_1D8[playerIndex].stickY = -56;
            lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
        }
        if (lbl_1_bss_1D8[playerIndex].timer < 600.0f * lowerMotionLimit) {
            lbl_1_bss_1D8[playerIndex].stickY = 56;
            lbl_1_bss_1D8[playerIndex].buttonsDown = PAD_BUTTON_A;
        }
    }
    if (lbl_1_bss_1D8[playerIndex].position.y >= -250.0f &&
        (obstacleCount = frandmod(100)) < edgeChance &&
        lbl_1_bss_1D8[playerIndex].stickY > 0) {
        lbl_1_bss_1D8[playerIndex].stickY = -56;
    }
}

/* Called by fn_1_2D48 to create the shared bubble particle system during player setup. */
void fn_1_10BD4(void)
{
    ANIMDATA *animation;
    HU3D_PARTICLE *particleSystem;
    HU3D_PARTICLE_DATA *particle;
    s16 index;

    animation =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_effect, 9), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_1C8 = Hu3DParticleCreate(animation, 300);
    Hu3DModelPosSet(lbl_1_bss_1C8, 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(lbl_1_bss_1C8, 1.0f, 1.0f, 1.0f);
    Hu3DModelLayerSet(lbl_1_bss_1C8, 5);
    Hu3DParticleColSet(lbl_1_bss_1C8, 255, 255, 255);
    Hu3DParticleScaleSet(lbl_1_bss_1C8, 1.0f);
    Hu3DParticleHookSet(lbl_1_bss_1C8, fn_1_10D74);
    particleSystem = (HU3D_PARTICLE *)Hu3DData[lbl_1_bss_1C8].hookData;
    particle = particleSystem->data;
    for (particle = particleSystem->data, index = 0; index < particleSystem->maxCnt;
         index++, particle++) {
        particle->scale = 0.0f;
        particle->time = 0;
    }
}

/* Registered by fn_1_10BD4 to move bubbles and expire them when Y reaches the stored limit. */
void fn_1_10D74(HU3D_MODEL *model, HU3D_PARTICLE *particleSystem, Mtx matrix)
{
    HU3D_PARTICLE_DATA *particle;
    s16 index;

    particle = particleSystem->data;
    for (index = 0; index < particleSystem->maxCnt; index++, particle++) {
        if (particle->scale) {
            PSVECAdd(&particle->vel, &particle->pos, &particle->pos);
            particle->vel.x = (f32)sin((3.141592653589793 * (f64)particle->time) /
                                       180.0);
            if (particle->pos.y >= particle->accel.y) {
                particle->scale = 0.0f;
            }
            particle->time += 6;
            if (particle->time >= 360) {
                particle->time = 0;
            }
        }
    }
}

/* Called by swim updates to add a bubble only when its position is at or below Y -10. */
void fn_1_10E90(Point3d position, f32 scale, f32 randomVelocity, f32 verticalAcceleration)
{
    HU3D_PARTICLE *particleSystem;
    HU3D_PARTICLE_DATA *particle;
    s16 index;

    particleSystem = (HU3D_PARTICLE *)Hu3DData[lbl_1_bss_1C8].hookData;
    particle = particleSystem->data;
    index = 0;
    while (index < particleSystem->maxCnt) {
        if (!particle->scale) {
            break;
        }
        index++;
        particle++;
    }
    if (!(position.y > -10.0f) && index < particleSystem->maxCnt) {
        particle->pos = position;
        particle->vel.x = 0.0f;
        particle->vel.y = randomVelocity + (f32)((frand() & 0x1) + 1);
        particle->vel.z = 0.0f;
        particle->accel.x = 0.0f;
        particle->accel.y = verticalAcceleration;
        particle->accel.z = 0.0f;
        particle->color.r = 255;
        particle->color.g = 255;
        particle->color.b = 255;
        particle->scale = scale;
        particle->time = frand() & 0xFC;
        if (particle->time >= 360) {
            particle->time = 0;
        }
    }
}

M625Player lbl_1_bss_1D8[4];
s32 lbl_1_bss_1D4;
s32 lbl_1_bss_1D0;
s32 lbl_1_bss_1CC;
HU3D_MODELID lbl_1_bss_1C8;
