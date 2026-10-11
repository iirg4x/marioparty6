// Board setup, route markers, NPCs, and player events for Thirsty Gulch.
#include "dolphin.h"
#include "datadir_enum.h"
#include "math.h"
#include "datanum/charmot.h"
#include "game/audio.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/process.h"
#include "game/wipe.h"
#include "game/board/main.h"
#include "game/board/masu.h"
#include "game/board/object.h"

typedef void (*VoidFunc)(void);

typedef struct S01Marker {
    s16 markerModelId;
    s16 markerMasuId;
} S01Marker;

typedef struct S01Npc {
    s16 npcModelId;
    s16 padding;
    s32 eventId;
} S01Npc;

typedef struct S01PosRot {
    HuVecF position;
    float headingDegrees;
} S01PosRot;

typedef struct S01PlayerMoveWork {
    s16 playerMotionId;
    HuVecF currentPosition;
    HuVecF destination;
    HuVecF origin;
} S01PlayerMoveWork;

typedef struct S01MotionPair {
    s16 travelMotionId;
    s16 jumpMotionId;
} S01MotionPair;

typedef struct S01WarpWork {
    // Holds approach displacement, then falling velocity.
    HuVecF movementVector;
    HuVecF currentPosition;
    HuVecF fallPosition;
    HuVecF destination;
    HuVecF playerOrigin;
} S01WarpWork;

enum {
    S01_OBJECT_PRIORITY = 8204,
};

#define S01_MASU_SINGLE_RETURN_EVENT_BIT (1 << 7)
#define S01_MASU_NPC_EVENT_MASK (7 << 4)
#define S01_MASU_MARKER_MOVE_INDEX_MASK 3
#define S01_MASU_MARKER_LOCATION_MASK (3 << 2)

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
extern u32 *lbl_1_bss_40;
extern s16 lbl_1_bss_3C;
extern s16 lbl_1_bss_0;
extern s16 lbl_1_bss_2[3];
extern S01Npc lbl_1_bss_8[5];
extern S01Marker lbl_1_bss_30[3];
extern HuVecF lbl_1_data_0;
extern HuVecF lbl_1_data_C;
extern HuVecF lbl_1_data_18;
extern s32 lbl_1_data_24[3];
extern HuVecF lbl_1_data_30[3];
extern S01PosRot lbl_1_data_54[5];
extern int lbl_1_data_A4[3];
extern s32 lbl_1_data_B0[5];
extern S01PosRot lbl_1_data_C4[3];
extern int lbl_1_data_F4[6];
extern int lbl_1_data_10C[2];
extern char lbl_1_data_114[8];
extern HuVecF lbl_1_data_11C;
extern HuVecF lbl_1_data_128;
#define lbl_1_rodata_78 6500.0f
#define lbl_1_rodata_7C 9800.0f
#define lbl_1_rodata_80 5000.0f
#define lbl_1_rodata_84 70000.0f
#define lbl_1_rodata_88 200.0f
#define lbl_1_rodata_8C 4.0f
#define lbl_1_rodata_90 42.0f
#define lbl_1_rodata_94 100.0f
#define lbl_1_rodata_98 3.5f
#define lbl_1_rodata_9C 2400.0f
#define lbl_1_rodata_A0 -1.0f
#define lbl_1_rodata_A4 120.0f
#define lbl_1_rodata_A8 -16.333334f
#define lbl_1_rodata_AC 0.016666668f
#define lbl_1_rodata_B0 -980.00006f
#define lbl_1_rodata_B4 90.0f
extern s32 lbl_1_bss_44;

void fn_1_A0(void);
void fn_1_F4(void);
void fn_1_270(OMOBJ *boardObject);
void fn_1_274(OMOBJ *boardObject);
int fn_1_2B8(int playerIndex, s16 masuId);
int fn_1_314(int playerIndex, s16 masuId);
int fn_1_374(int playerIndex, s16 masuId);
void fn_1_3E0(int playerIndex);
void fn_1_3E4(int playerIndex);
void fn_1_404(void);
void fn_1_468(void);
void fn_1_46C(BOOL enterF);
void fn_1_4C8(void);
void fn_1_590(int playerIndex, int eventId);
void fn_1_87C(void);
void fn_1_8F0(void);
void fn_1_9B8(int playerIndex, int eventId);
void fn_1_A50(void);
void fn_1_AFC(void);
void fn_1_C38(void);
void fn_1_CEC(void);
void fn_1_DAC(int playerIndex, s16 masuId);

void mbObjectSetup(s32 boardNo, void (*init)(void), void (*close)(OMOBJ *));
void HuAudSndGrpSetSet(s16 grpSet);
void HuDataDirClose(int dataNum);
void mbLightFuncSet(VoidFunc setHook, VoidFunc resetHook);
void mbMapCameraSet(const HuVecF *rot, const HuVecF *position, float zoom);
void mbMapHookSet(void (*hook)(BOOL enterF));
void mbOpeningViewSet(HuVecF *rot, HuVecF *position, float zoom);
void mbPlayerTurnCloseHookSet(void (*hook)(int playerIndex));
void mbPlayerTurnInitHookSet(void (*hook)(int playerIndex));
void mbScrollInit(int dataNum);
float mbSinDeg(float headingDegrees);
int mbAudFXPlay(int seId);
void mbAudFXStop(int handle);
void mbCameraPlayerViewSetFast(int playerIndex, int viewNo);
void mbCameraShakeSet(int maxTime, float power);
void mbCameraFocusObjSet(int modelId);
void mbCameraMovePlayer(s16 playerIndex, HuVecF *rot, HuVecF *offset,
    float zoom, float fov, s16 maxTime);
void mbCameraMovePos(HuVecF *position, HuVecF *rot, HuVecF *offset,
    float zoom, float fov, s16 maxTime);
void mbMoveNumDispSet(int playerIndex, BOOL display);
void mbPauseDisableSet(BOOL disable);
void mbPlayerColSnapPlayerSet(int playerIndex, BOOL snapF);
int mbPlayerMotionCreate(int playerIndex, int dataNum);
int mbPlayerMotionKill(int playerIndex, int motNo);
void mbPlayerMotionShiftSet(
    int playerIndex, int motNo, float start, float end, u32 attr);
void mbPlayerPosGet(int playerIndex, HuVecF *position);
void mbPlayerPosSetV(int playerIndex, const HuVecF *position);
void mbPlayerRotYSet(int playerIndex, float rotY);
void mbPlayerRotateStart(int playerIndex, s16 endAngle, s16 maxTime);
void mbSingleReturnWrite(void);
void mbWipeCreate(s16 mode, s16 type, s16 time);
void mbWipeDissolveFadeOut(void);
void mbWipeFadeOut(void);
void mbWipeFadeOutTime(int time);
void mbWipeSpecialCreate(int state, int type, int time);
void mbWipeSpecialKill(void);
void mbWipeSpecialWait(void);

#include "REL/s01Dll/common.h"

// Selects single-player mode and registers the Thirsty Gulch board hooks.
void fn_1_A0(void)
{
    GwSystem.partyF = FALSE;
    mbObjectSetup(6, fn_1_F4, fn_1_270);
}

// Loads Thirsty Gulch board assets and installs board, camera, and turn hooks.
void fn_1_F4(void)
{
    s16 *boardModelId = &lbl_1_bss_3C;
    s32 boardNo;

    // The current board number is queried here, but this setup does not use the result.
    boardNo = MBBoardNoGet();

    HuAudSndGrpSetSet(MSM_GRP_SBRD);
    lbl_1_bss_40 = GwSystem.boardWork;
    mbMasuInit(DATANUM(DATA_s01, 0));
    *boardModelId = mbObjCreate(DATANUM(DATA_s01, 2), NULL, FALSE);
    mbObjAttrSet(*boardModelId, HU3D_MOTATTR_LOOP);
    mbScrollInit(DATANUM(DATA_s01, 1));
    mbLightFuncSet(fn_1_404, fn_1_468);
    mbPlayerTurnInitHookSet(fn_1_3E0);
    mbPlayerTurnCloseHookSet(fn_1_3E4);
    mbOpeningViewSet(&lbl_1_data_C, &lbl_1_data_18, lbl_1_rodata_78);
    mbev_MasuMoveEndSet(fn_1_314);
    mbev_MasuMoveStartSet(fn_1_2B8);
    mbev_MasuHatenaSet(fn_1_374);
    mbMapCameraSet(NULL, &lbl_1_data_0, lbl_1_rodata_7C);
    mbMapHookSet(fn_1_46C);
    omAddObjEx(mbObjMan, S01_OBJECT_PRIORITY, 0, 0, -1, fn_1_274);
    fn_1_4C8();
    fn_1_8F0();
    fn_1_AFC();
    fn_1_C38();
    HuDataDirClose(DATANUM(DATA_s01, 0));
}

// No close-time returnWork is needed for the board object manager.
void fn_1_270(OMOBJ *boardObject)
{
}

// Board object-manager callback: updates board effects until the board exits.
void fn_1_274(OMOBJ *boardObject)
{
    if (mbExitCheck()) {
        omDelObjEx((OMOBJMAN *)HuPrcCurrentGet(), boardObject);
    } else {
        fn_1_A50();
        fn_1_CEC();
    }
}

// Masu-start hook: starts the return sequence on its special route square.
int fn_1_2B8(int playerIndex, s16 masuId)
{
    u32 masuAttributes = mbMasuMAttrGet(masuId);

    if (masuAttributes & S01_MASU_SINGLE_RETURN_EVENT_BIT) {
        fn_1_DAC(playerIndex, masuId);
    }
    return 0;
}

// Called after a player reaches a square; starts the reactions assigned to its event.
int fn_1_314(int playerIndex, s16 masuId)
{
    u32 masuAttributes = mbMasuMAttrGet(masuId);

    if (masuAttributes & S01_MASU_NPC_EVENT_MASK) {
        fn_1_9B8(playerIndex, mbev_MasuBitGet(masuAttributes, S01_MASU_NPC_EVENT_MASK));
    }
    return 0;
}

// Hatena-square hook: moves the player to the selected route marker.
int fn_1_374(int playerIndex, s16 masuId)
{
    u32 masuAttributes = mbMasuMAttrGet(masuId);

    if (masuAttributes & S01_MASU_MARKER_MOVE_INDEX_MASK) {
        fn_1_590(playerIndex, mbev_MasuBitGet(masuAttributes,
            S01_MASU_MARKER_MOVE_INDEX_MASK) - 1);
        return 0;
    }
    return 0;
}

// Thirsty Gulch has no board-specific player-turn start effect.
void fn_1_3E0(int playerIndex)
{
}

void fn_1_3E4(int playerIndex)
{
    fn_1_87C();
}

// Board light hook: imports the board model's static lights and sets the scene fog.
void fn_1_404(void)
{
    s16 *boardModelId = &lbl_1_bss_3C;

    Hu3DModelLightInfoSet(mbObjModelIDGet(boardModelId[0]), TRUE);
    Hu3DFogSet(lbl_1_rodata_80, lbl_1_rodata_84, 200, 200, 100);
}

// The board does not need a separate light reset when its light hook is removed.
void fn_1_468(void)
{
}

// Map hook: clears fog while entering the map and restores it on exit.
void fn_1_46C(BOOL enteringArea)
{
    if (enteringArea) {
        Hu3DFogClear();
    } else {
        Hu3DFogSet(lbl_1_rodata_80, lbl_1_rodata_84, 200, 200, 100);
    }
}

// Creates the three route markers and records the masu attached to each.
void fn_1_4C8(void)
{
    S01Marker *marker = lbl_1_bss_30;
    u32 masuAttributes;
    s32 markerIndex;

    for (markerIndex = 0; markerIndex < 3; markerIndex++, marker++) {
        marker->markerModelId = mbObjCreate(lbl_1_data_24[markerIndex], NULL, FALSE);
        mbObjPosSetV(marker->markerModelId, &lbl_1_data_30[markerIndex]);
        mbObjMotionSpeedSet(marker->markerModelId, 0.0f);
        masuAttributes = mbev_MasuAttrGet(markerIndex + 1, S01_MASU_MARKER_LOCATION_MASK);
        marker->markerMasuId = mbMasuFind_MAttrMatchIdGet(-1, masuAttributes,
            S01_MASU_MARKER_LOCATION_MASK);
    }
}

// Animates a player along the board route to the marker selected by its event.
void fn_1_590(int playerIndex, int eventId)
{
    S01Marker *marker = &lbl_1_bss_30[eventId];
    S01PlayerMoveWork routeMove;
    int soundHandle;
    s32 elapsedFrame;
    float progress;

    routeMove.playerMotionId = mbPlayerMotionCreate(playerIndex, CHARMOT_HSF_c000m1_344);
    mbPlayerRotateStart(playerIndex, 0, 15);
    mbCameraPlayerViewSetFast(playerIndex, 2);
    mbCameraShakeSet(48, lbl_1_rodata_88);
    soundHandle = mbAudFXPlay(MSM_SE_SBRD_08);
    HuPrcSleep(48);
    mbPlayerMotionShiftSet(playerIndex, 9, 0.0f,
        lbl_1_rodata_8C, 0);
    mbMasuPosGet(GwPlayer[playerIndex].masuId, &routeMove.origin);
    mbMasuPosGet(marker->markerMasuId, &routeMove.destination);
    GwPlayer[playerIndex].masuIdNext = marker->markerMasuId;
    mbMasuNextSet(marker->markerMasuId);
    mbPlayerColSnapPlayerSet(playerIndex, FALSE);

    // The player follows a linear route with a short sine-shaped hop above it.
    for (elapsedFrame = 0; (u32)elapsedFrame <= 42; elapsedFrame++) {
        progress = elapsedFrame / lbl_1_rodata_90;
        if ((u32)elapsedFrame == 0) {
            mbObjMotionSpeedSet(marker->markerModelId, 1.0f);
            mbAudFXPlay(MSM_SE_SBRD_09);
            mbAudFXStop(soundHandle);
        } else if ((u32)elapsedFrame == 24) {
            mbPlayerMotionShiftSet(playerIndex, routeMove.playerMotionId,
                0.0f, 8.0f, 0);
        }
        routeMove.currentPosition.x = routeMove.origin.x
            + progress * (routeMove.destination.x - routeMove.origin.x);
        routeMove.currentPosition.y = routeMove.origin.y
            + progress * (routeMove.destination.y - routeMove.origin.y)
            + lbl_1_rodata_94
                * (lbl_1_rodata_98 * mbSinDeg(180.0f * progress));
        routeMove.currentPosition.z = routeMove.origin.z
            + progress * (routeMove.destination.z - routeMove.origin.z);
        mbPlayerPosSetV(playerIndex, &routeMove.currentPosition);
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerIndex, 6, 0.0f,
        lbl_1_rodata_8C, HU3D_MOTATTR_LOOP);
    HuPrcSleep(60);
    mbWipeFadeOut();
    mbPlayerColSnapPlayerSet(playerIndex, TRUE);
    mbPlayerMotionKill(playerIndex, routeMove.playerMotionId);
    GwPlayer[playerIndex].masuId = marker->markerMasuId;
}

// Stops each route marker after a player's turn closes.
void fn_1_87C(void)
{
    S01Marker *marker = lbl_1_bss_30;
    s32 markerIndex;

    for (markerIndex = 0; markerIndex < 3; markerIndex++, marker++) {
        mbObjMotionSpeedSet(marker->markerModelId, 0.0f);
        mbObjMotionTimeSet(marker->markerModelId, 0.0f);
    }
}

// Creates and places the five Thirsty Gulch NPCs during board setup.
void fn_1_8F0(void)
{
    S01Npc *npc = lbl_1_bss_8;
    s32 npcIndex;

    for (npcIndex = 0; npcIndex < 5; npcIndex++, npc++) {
        npc->npcModelId = mbObjCreate(DATANUM(DATA_s01, 6), lbl_1_data_A4, FALSE);
        mbObjPosSetV(npc->npcModelId, &lbl_1_data_54[npcIndex].position);
        mbObjRotYSet(npc->npcModelId, lbl_1_data_54[npcIndex].headingDegrees);
        mbObjMotionSet(npc->npcModelId, 1, HU3D_MOTATTR_LOOP);
        npc->eventId = lbl_1_data_B0[npcIndex];
    }
}

// Starts reactions for each board character assigned to the reached square's event.
void fn_1_9B8(int playerIndex, int eventId)
{
    S01Npc *npc = lbl_1_bss_8;
    s32 npcIndex;

    for (npcIndex = 0; npcIndex < 5; npcIndex++, npc++) {
        if (eventId == npc->eventId) {
            mbObjMotionShiftSet(npc->npcModelId, 2, 0.0f,
                lbl_1_rodata_8C, 0);
            mbObjMotionShapeSet(npc->npcModelId, 2, 0);
        }
    }
}

// Board update: returns finished NPC reactions to their idle animation.
void fn_1_A50(void)
{
    S01Npc *npc = lbl_1_bss_8;
    s32 npcIndex;

    for (npcIndex = 0; npcIndex < 5; npcIndex++, npc++) {
        if (mbObjMotionGet(npc->npcModelId) != 1
            && mbObjMotionEndCheck(npc->npcModelId)) {
            mbObjMotionShiftSet(npc->npcModelId, 1, 0.0f,
                lbl_1_rodata_8C, HU3D_MOTATTR_LOOP);
            mbObjMotionShapeSet(npc->npcModelId, 1, HU3D_MOTATTR_SHAPE_LOOP);
        }
    }
}

// Creates three route characters with shuffled looping animations.
void fn_1_AFC(void)
{
    s16 *routeModelId = lbl_1_bss_2;
    s32 motionIds[16];
    s32 motionOptionCount = 5;
    s32 firstRandomIndex;
    s32 secondRandomIndex;
    s32 displacedMotion;
    s32 motionIndex;

    for (motionIndex = 0; motionIndex < motionOptionCount; motionIndex++) {
        motionIds[motionIndex] = motionIndex + 1;
    }
    for (motionIndex = 0; motionIndex < 100; motionIndex++) {
        firstRandomIndex = mbRandMod(motionOptionCount);
        secondRandomIndex = mbRandMod(motionOptionCount);
        displacedMotion = motionIds[firstRandomIndex];
        motionIds[firstRandomIndex] = motionIds[secondRandomIndex];
        motionIds[secondRandomIndex] = displacedMotion;
    }
    for (motionIndex = 0; motionIndex < 3; motionIndex++, routeModelId++) {
        *routeModelId = mbObjCreate(DATANUM(DATA_s01, 9), lbl_1_data_F4, FALSE);
        mbObjPosSetV(*routeModelId, &lbl_1_data_C4[motionIndex].position);
        mbObjRotYSet(*routeModelId, lbl_1_data_C4[motionIndex].headingDegrees);
        mbObjMotionSet(*routeModelId, motionIds[motionIndex], HU3D_MOTATTR_LOOP);
    }
}

// Creates the small sign model and places it at the board's "jyango" point.
void fn_1_C38(void)
{
    s16 *signModelId = &lbl_1_bss_0;
    HuVecF position;

    signModelId[0] = mbObjCreate(DATANUM(DATA_s01, 16), lbl_1_data_10C, TRUE);
    mbObjMotionSet(signModelId[0], 1, HU3D_MOTATTR_LOOP);
    mbObjScaleSet(signModelId[0], 0.5f, 0.5f, 0.5f);
    Hu3DModelObjPosGet(mbObjModelIDGet(lbl_1_bss_3C), lbl_1_data_114, &position);
    mbObjPosSetV(signModelId[0], &position);
}

// Board update: turns the sign toward its target point and follows that point.
void fn_1_CEC(void)
{
    s16 *signModelId = &lbl_1_bss_0;
    HuVecF signPosition;
    HuVecF signTarget;
    HuVecF signDirection;
    float headingDegrees;

    mbObjPosGet(signModelId[0], &signPosition);
    Hu3DModelObjPosGet(mbObjModelIDGet(lbl_1_bss_3C), lbl_1_data_114,
        &signTarget);
    PSVECSubtract(&signTarget, &signPosition, &signDirection);
    headingDegrees = (float)(180.0
        * (atan2(signDirection.x, signDirection.z) / 3.141592653589793));
    mbObjRotYSet(signModelId[0], headingDegrees);
    mbObjPosSetV(signModelId[0], &signTarget);
}

// Runs the return-square animation, then exits the single-player board
// and discards pending results.
void fn_1_DAC(int playerIndex, s16 masuId)
{
    S01Marker *marker = &lbl_1_bss_30[2];
    S01MotionPair playerMotions;
    S01WarpWork returnWork;
    s16 currentMasuId;
    int soundHandle;
    s32 elapsedFrame;
    float progress;
    float headingDegrees;

    mbPauseDisableSet(TRUE);
    playerMotions.travelMotionId = mbPlayerMotionCreate(playerIndex, CHARMOT_HSF_c000m1_323);
    playerMotions.jumpMotionId = mbPlayerMotionCreate(playerIndex, CHARMOT_HSF_c000m1_344);
    mbMoveNumDispSet(playerIndex, FALSE);
    mbCameraMovePlayer((s16)playerIndex, &lbl_1_data_11C, NULL,
        lbl_1_rodata_9C, lbl_1_rodata_A0, 120);
    currentMasuId = GwPlayer[playerIndex].masuId;
    mbMasuPosGet(currentMasuId, &returnWork.playerOrigin);
    mbMasuPosGet(masuId, &returnWork.destination);
    PSVECSubtract(&returnWork.destination, &returnWork.playerOrigin, &returnWork.movementVector);
    headingDegrees = (float)(180.0
        * (atan2(returnWork.movementVector.x, returnWork.movementVector.z) / 3.141592653589793));
    mbPlayerRotYSet(playerIndex, headingDegrees);
    mbPlayerMotionShiftSet(playerIndex, 2, 0.0f,
        lbl_1_rodata_8C, HU3D_MOTATTR_LOOP);

    for (elapsedFrame = 0; (u32)elapsedFrame <= 120; elapsedFrame++) {
        progress = elapsedFrame / lbl_1_rodata_A4;
        returnWork.currentPosition.x =
            returnWork.playerOrigin.x + progress * returnWork.movementVector.x;
        returnWork.currentPosition.y =
            returnWork.playerOrigin.y + progress * returnWork.movementVector.y;
        returnWork.currentPosition.z =
            returnWork.playerOrigin.z + progress * returnWork.movementVector.z;
        mbPlayerPosSetV(playerIndex, &returnWork.currentPosition);
        if (elapsedFrame % 30 == 0) {
            omVibrate((s16)playerIndex, 20, 7, 3);
        }
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerIndex, playerMotions.travelMotionId, 0.0f,
        lbl_1_rodata_8C, HU3D_MOTATTR_LOOP);
    HuPrcSleep(30);
    mbCameraShakeSet(60, lbl_1_rodata_88);
    soundHandle = mbAudFXPlay(MSM_SE_SBRD_08);
    HuPrcSleep(60);
    mbObjMotionSpeedSet(marker->markerModelId, 1.0f);
    omVibrate((s16)playerIndex, 20, 20, 0);
    mbAudFXPlay(MSM_SE_SBRD_09);
    mbAudFXStop(soundHandle);
    HuPrcSleep(12);
    mbPlayerMotionShiftSet(playerIndex, 9, 0.0f,
        lbl_1_rodata_8C, 0);
    mbCameraFocusObjSet(-1);
    returnWork.movementVector.x = returnWork.movementVector.y = returnWork.movementVector.z = 0.0f;
    mbPlayerPosGet(playerIndex, &returnWork.fallPosition);

    // The player accelerates downward for 72 updates before the outgoing frame is captured.
    for (elapsedFrame = 0; (u32)elapsedFrame < 72; elapsedFrame++) {
        returnWork.movementVector.y += lbl_1_rodata_A8;
        returnWork.fallPosition.y += lbl_1_rodata_AC * returnWork.movementVector.y;
        mbPlayerPosSetV(playerIndex, &returnWork.fallPosition);
        if ((u32)elapsedFrame == 18) {
            mbPlayerMotionShiftSet(playerIndex, playerMotions.jumpMotionId,
                0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        }
        HuPrcVSleep();
    }
    mbWipeDissolveFadeOut();
    mbCameraMovePos(NULL, &lbl_1_data_128, NULL, lbl_1_rodata_88,
        lbl_1_rodata_A0, -1);
    mbObjDispSet(marker->markerModelId, FALSE);
    // Reset to the approach height and begin another fall with downward velocity.
    returnWork.fallPosition.y = returnWork.playerOrigin.y;
    returnWork.movementVector.y = lbl_1_rodata_B0;
    mbPlayerPosSetV(playerIndex, &returnWork.fallPosition);
    mbWipeCreate(1, WIPE_TYPE_CROSS_COPY, 30);
    mbMusFadeOutSpeed(0, 1000);
    mbWipeSpecialCreate(1, 6, 90);
    mbAudFXPlay(MSM_SE_SBRD_10);

    // The following loop computes progress but does not use it.
    for (elapsedFrame = 0; (u32)elapsedFrame <= 90; elapsedFrame++) {
        progress = elapsedFrame / lbl_1_rodata_B4;
        returnWork.movementVector.y += lbl_1_rodata_A8;
        returnWork.fallPosition.y += lbl_1_rodata_AC * returnWork.movementVector.y;
        mbPlayerPosSetV(playerIndex, &returnWork.fallPosition);
        HuPrcVSleep();
    }
    mbWipeSpecialWait();
    mbWipeFadeOutTime(1);
    mbWipeSpecialKill();
    mbPlayerMotionKill(playerIndex, playerMotions.travelMotionId);
    mbPlayerMotionKill(playerIndex, playerMotions.jumpMotionId);
    mbSingleReturnWrite();
}

float *lbl_1_bss_48;
s32 lbl_1_bss_44;
u32 *lbl_1_bss_40;
s16 lbl_1_bss_3C;
S01Marker lbl_1_bss_30[3];
S01Npc lbl_1_bss_8[5];
s16 lbl_1_bss_2[3];
s16 lbl_1_bss_0;

HuVecF lbl_1_data_0 = {0.0f, 0.0f, 300.0f};
HuVecF lbl_1_data_C = {-28.0f, 0.0f, 0.0f};
HuVecF lbl_1_data_18 = {0.0f, 0.0f, 0.0f};
s32 lbl_1_data_24[3] = {DATANUM(DATA_s01, 3), DATANUM(DATA_s01, 4), DATANUM(DATA_s01, 5)};
HuVecF lbl_1_data_30[3] = {
    {-150.0f, 566.0f, -375.0f},
    {191.0f, 828.0f, -1272.0f},
    {-806.0f, 887.0f, -1481.0f}
};
S01PosRot lbl_1_data_54[5] = {
    {{-556.0f, -8.0f, 756.0f}, 41.0f},
    {{190.0f, 219.0f, 160.0f}, -15.0f},
    {{-1204.0f, 384.0f, -465.0f}, 37.0f},
    {{-1123.0f, 338.0f, -548.0f}, 22.0f},
    {{1480.0f, 661.0f, -1111.0f}, -40.0f}
};
int lbl_1_data_A4[3] = {DATANUM(DATA_s01, 7), DATANUM(DATA_s01, 8), -1};
s32 lbl_1_data_B0[5] = {1, 2, 3, 3, 4};
S01PosRot lbl_1_data_C4[3] = {
    {{-203.0f, -11.0f, 857.0f}, 50.0f},
    {{523.0f, 99.0f, 411.0f}, -55.0f},
    {{-801.0f, 653.0f, -870.0f}, 45.0f}
};
int lbl_1_data_F4[6] = {
    DATANUM(DATA_s01, 10), DATANUM(DATA_s01, 11), DATANUM(DATA_s01, 12), DATANUM(DATA_s01, 13),
    DATANUM(DATA_s01, 14), -1
};
int lbl_1_data_10C[2] = {DATANUM(DATA_s01, 17), -1};
char lbl_1_data_114[8] = "jyango";
HuVecF lbl_1_data_11C = {-15.0f, 0.0f, 0.0f};
HuVecF lbl_1_data_128 = {-90.0f, 0.0f, 0.0f};
