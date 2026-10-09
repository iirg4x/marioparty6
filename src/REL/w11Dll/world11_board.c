/* Sets up the tutorial board, advances its scenes, and samples camera and guide paths. */
#include "dolphin.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/board/tutorial.h"
#include "game/board/player.h"
#include "game/board/camera.h"
#include "game/board/window.h"
#include "game/board/main.h"
#include "humath.h"
#include "datadir_enum.h"
#include "game/board/guide.h"
#include "game/board/masu.h"
#include "game/board/opening.h"
#include "msm_grp.h"
#include "game/hu3d.h"
#include "game/board/status.h"
#include "game/board/audio.h"
#include "game/esprite.h"
#include "messdir_enum.h"

#define TUTORIAL_FALL_SPACE_FLAG (1 << 0)
#define TUTORIAL_NET_SPACE_FLAG (1 << 1)
#define MSM_SE_BRDTT_NET_START 1539
#define MSM_SE_BRDTT_FALL_PREPARE 1540
#define MSM_SE_BRDTT_FALL_DROP 1541

void mbObjDirSet(int dataDirDay, int dataDirNight);
void mbObjPosSet(MBMODELID modelId, float x, float y, float z);
void mbObjCullRadiusSet(MBMODELID modelId, float radius);
void mbObjMotionTimeSet(MBMODELID modelId, float time);
void mbObjMotionShapeTimeSet(MBMODELID modelId, float time);
void mbObjMotionSpeedSet(MBMODELID modelId, float speed);
void mbObjMotionShapeSpeedSet(MBMODELID modelId, float speed);
HU3D_MODELID mbObjModelIDGet(MBMODELID modelId);
void mbObjAttrReset(MBMODELID modelId, u32 attr);
float mbObjMotionShapeTimeGet(MBMODELID modelId);

f32 lbl_1_data_0[3] = { -0.75f, -0.699999988f, -750.0f };

void fn_1_1F4(void);
void fn_1_510(void);
void mbObjectSetup(s32 boardNo, void (*init)(void), void (*close)(void));

/* Module startup selects tutorial rules and installs the board setup and close callbacks. */
void fn_1_A0(void)
{
    GWPartySet(FALSE);
    _SetFlag(FLAG_BOARD_TUTORIAL);
    GwSystem.turnMax = 20;
    GwSystem.tagF = FALSE;
    GWBonusStarSet(FALSE);
    GWMgInstDispSet(TRUE);
    GWMgComDispSet(TRUE);
    GwSystem.mgPack = 0;
    GWVibrateSet(GwCommon.vibrateF);
    GWMessSpeedSet(GW_MESS_SPEED_NORMAL);
    mbObjectSetup(10, fn_1_1F4, fn_1_510);
}

void mbScrollInit(int dataNum);
int mbCapThrowColCreate(int dataNum);
void mbLightFuncSet(void (*setHook)(void), void (*resetHook)(void));
void fn_1_550(OMOBJ *obj);
void fn_1_554(void);
void fn_1_588(void);
int fn_1_58C(int playerNo, s16 spaceId);
void fn_1_918(void);
extern MBMODELID lbl_1_bss_0, lbl_1_bss_2, lbl_1_bss_4;
/* Asset setup tests whether the board is currently in its daytime state. */
static inline BOOL MBTimeDayGet(void)
{
    return GwSystem.curTime == 0;
}

/* The board initializer loads the tutorial scene and installs its lighting and movement hooks. */
void fn_1_1F4(void)
{
    s32 boardNo;

    /* The board number is read even though this scene uses fixed tutorial assets. */
    boardNo = MBBoardNoGet();
    HuAudSndGrpSetSet(MSM_GRP_BRDTT);
    mbObjDirSet(DATA_w11, DATA_w11);
    mbMasuInit(MBTimeDayGet() ? DATA_w11 : DATA_w11);
    /* Both time-of-day branches deliberately select the same tutorial archive. */
    lbl_1_bss_4 = mbObjCreate(DATANUM((MBTimeDayGet() ? DATA_w11 : DATA_w11), 1), NULL, FALSE);
    mbObjAttrSet(lbl_1_bss_4, HU3D_MOTATTR_LOOP);
    mbObjPosSet(lbl_1_bss_4, 0.0f, 0.0f, 0.0f);
    mbObjCullRadiusSet(lbl_1_bss_4, -1.0f);
    mbScrollInit(0);
    mbCapThrowColCreate(-1);
    mbLightFuncSet(fn_1_554, fn_1_588);
    mbOpeningInstHookSet(NULL);
    mbOpeningStarInstHookSet(NULL);
    mbev_MasuMoveEndSet(fn_1_58C);
    lbl_1_bss_2 = mbObjCreate(DATANUM((MBTimeDayGet() ? DATA_w11 : DATA_w11), 2), NULL, FALSE);
    mbObjAttrSet(lbl_1_bss_2, HU3D_MOTATTR_PAUSE);
    lbl_1_bss_0 = mbObjCreate(DATANUM((MBTimeDayGet() ? DATA_w11 : DATA_w11), 3), NULL, FALSE);
    mbObjAttrSet(lbl_1_bss_0, HU3D_MOTATTR_LOOP | HU3D_MOTATTR_SHAPE_LOOP);
    /* Hold the net at frame 20 until the fall scene starts its animation. */
    mbObjMotionTimeSet(lbl_1_bss_0, 20.0f);
    mbObjMotionShapeTimeSet(lbl_1_bss_0, 20.0f);
    mbObjMotionSpeedSet(lbl_1_bss_0, 0.0f);
    mbObjMotionShapeSpeedSet(lbl_1_bss_0, 0.0f);
    HuDataDirClose(MBTimeDayGet() ? DATA_w11 : DATA_w11);
    omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_550);
    fn_1_918();
}

extern MBMODELID lbl_1_bss_4;
void fn_1_5DC(s32 playerNo, s16 spaceId);

/* The board close callback releases the scenery model if its handle is valid. */
void fn_1_510(void)
{
    if (lbl_1_bss_4 >= 0) {
        mbObjKill(lbl_1_bss_4);
    }
}

/* The tutorial's object-manager callback has no per-frame work. */
void fn_1_550(OMOBJ *boardObject)
{
}

/* The board light hook imports the scenery's HSF lights as static global lights. */
void fn_1_554(void)
{
    Hu3DModelLightInfoSet(mbObjModelIDGet(lbl_1_bss_4), TRUE);
}

/* The board light reset hook leaves the tutorial's lighting unchanged. */
void fn_1_588(void)
{
}

/* The movement-end hook starts the fall on marked spaces and returns zero for every space. */
int fn_1_58C(int playerNo, s16 spaceId)
{
    if ((mbMasuMAttrGet(spaceId) & TUTORIAL_FALL_SPACE_FLAG) != 0) {
        fn_1_5DC(playerNo, spaceId);
    }
    return 0;
}

void mbWipeDissolveFadeOut(void);
void mbWipeCreate(s16 mode, s16 type, s16 time);
extern MBMODELID lbl_1_bss_0, lbl_1_bss_2;
/* The marked-space hook drops the player, then keeps them attached to the animated net. */
void fn_1_5DC(s32 playerNo, s16 spaceId)
{
    HuVecF playerPosition;
    HuVecF fallVelocity;
    s32 fallMotions[2];
    s32 netRevealTimer;
    s32 fallFrame;
    f32 previousShapeTime;
    f32 currentShapeTime;
    s32 unusedEventState;

    /* This state is initialized but is not used by the later scene steps. */
    unusedEventState = 0;

    mbMoveNumDispSet(playerNo, FALSE);
    fallMotions[0] = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 23));
    fallMotions[1] = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 34));
    mbPlayerMotionShiftSet(playerNo, fallMotions[0], 0.0f, 4.0f, HU3D_MOTATTR_LOOP);
    HuPrcSleep(120);
    mbObjAttrReset(lbl_1_bss_2, HU3D_MOTATTR_PAUSE);
    mbAudFXPlay(MSM_SE_BRDTT_FALL_PREPARE);
    mbPlayerMotionShiftSet(playerNo, 9, 0.0f, 4.0f, 0);
    HuPrcSleep(30);
    mbCameraFocusReset();
    mbPlayerMotionShiftSet(playerNo, fallMotions[1], 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbAudFXPlay(MSM_SE_BRDTT_FALL_DROP);
    /* Disable collision snapping so the scripted downward acceleration can move the player. */
    fallVelocity.x = fallVelocity.z = 0.0f;
    fallVelocity.y = -6.66666698f;
    for (fallFrame = 0; fallFrame < 60U; fallFrame++) {
        mbPlayerPosGet(playerNo, &playerPosition);
        PSVECAdd(&playerPosition, &fallVelocity, &playerPosition);
        mbPlayerPosSetV(playerNo, &playerPosition);
        fallVelocity.y += -2.72222257f;
        HuPrcVSleep();
    }
    mbTutorialCall(23);
    mbWipeDissolveFadeOut();
    mbStatusDispForceSetAll(FALSE);
    mbStatusMasuDispSet(FALSE);
    mbCameraFocusMasuSet(mbMasuFind_MAttrIdGet(-1, TUTORIAL_NET_SPACE_FLAG));
    mbCameraMoveOnSet(FALSE);
    mbCameraOffsetSet(0.0f, 50.0f, 0.0f);
    mbCameraRotSet(-35.0f, 0.0f, 0.0f);
    mbCameraZoomSet(3500.0f);
    mbObjMotionSpeedSet(lbl_1_bss_0, 1.0f);
    mbObjMotionShapeSpeedSet(lbl_1_bss_0, 1.0f);
    netRevealTimer = 30;
    mbWipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSS_COPY, netRevealTimer);
    netRevealTimer += 2;
    previousShapeTime = -1.0f;
    /* The tutorial process handles the ending while this hook continues following net_hook. */
    for (;;) {
        if (netRevealTimer != 0) {
            netRevealTimer--;
            if (netRevealTimer == 0) {
                mbTutorialCall(23);
            }
        }
        Hu3DModelObjPosGet(mbObjModelIDGet(lbl_1_bss_0), "net_hook", &playerPosition);
        mbPlayerPosSetV(playerNo, &playerPosition);
        currentShapeTime = mbObjMotionShapeTimeGet(lbl_1_bss_0);
        /* Play once each time the looping shape motion passes frame 2. */
        if (currentShapeTime > 2.0f && previousShapeTime <= 2.0f) {
            mbAudFXPlay(MSM_SE_BRDTT_NET_START);
        }
        previousShapeTime = currentShapeTime;
        HuPrcVSleep();
    }
}

s16 lbl_1_bss_4; /* Tutorial scenery model, including the scene's HSF lights. */
s16 lbl_1_bss_2; /* Scene model whose paused motion resumes before the player falls. */
s16 lbl_1_bss_0; /* Animated net model; net_hook supplies the player's attachment position. */

s8 lbl_1_data_15[7] = { 1, 12, 21, 7, 4, 15, -1 };
extern void fn_1_984(void);
extern void fn_1_DF0(void);
extern void fn_1_1200(void);

void fn_1_940(void);

/* Board initialization registers the process that presents the tutorial lessons. */
void fn_1_918(void)
{
    mbTutorialMainFuncSet(fn_1_940);
}

/* The tutorial process shows the opening, movement lesson, and net ending in order. */
void fn_1_940(void)
{
    mbTutorialGuideCreate(lbl_1_data_15, TRUE);
    fn_1_DF0();
    fn_1_984();
    fn_1_1200();
    mbTutorialExitSet();
    /* Remain in the tutorial wait until the exit watcher terminates this process. */
    mbTutorialCallWait(24);
}

void fn_1_127C(f32 x, f32 y, f32 z);
void fn_1_C00(s32 message, s32 illustrationData, s32 iconData, s32 iconBank);

/* The lesson process explains turns and board spaces at the player's scripted movement events. */
void fn_1_984(void)
{
    s32 windowId;
    fn_1_127C(-0.75f, -0.699999988f, -800.0f);
    mbTutorialCallWait(3);
    windowId = mbTutorialWinCreate(MESSNUM(MESS_BOARD_TUTORIAL, 73));
    mbTutorialWinKeyWait(windowId);
    mbTutorialTurnCall(4);
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 74));
    mbTutorialCallWait(6);
    windowId = mbTutorialWinCreate(MESSNUM(MESS_BOARD_TUTORIAL, 75));
    mbWinInsertMesSet(windowId, mbPlayerNameMesGet(0), 0);
    mbWinInsertMesSet(windowId, (u32)"4", 1);
    mbTutorialWinKeyWait(windowId);
    mbTutorialCallWait(8);
    mbPlayerMotIdleSet(GwSystem.turnPlayerNo);
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 76));
    fn_1_C00(MESSNUM(MESS_BOARD_TUTORIAL, 77), DATANUM(DATA_btutorial, 12),
             DATANUM(DATA_board, 132), 0);
    fn_1_C00(MESSNUM(MESS_BOARD_TUTORIAL, 78), DATANUM(DATA_btutorial, 13),
             DATANUM(DATA_board, 132), 1);
    fn_1_C00(MESSNUM(MESS_BOARD_TUTORIAL, 79), DATANUM(DATA_btutorial, 7), DATANUM(DATA_board, 132),
             2);
    fn_1_C00(MESSNUM(MESS_BOARD_TUTORIAL, 80), DATANUM(DATA_btutorial, 10),
             DATANUM(DATA_board, 132), 3);
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 86));
    mbCameraMovePlayer(GwSystem.turnPlayerNo, NULL, NULL, 1600.0f, -1.0f, 12);
    mbCameraMoveWait();
    mbTutorialTurnCall(5);
    fn_1_C00(MESSNUM(MESS_BOARD_TUTORIAL, 87), DATANUM(DATA_btutorial, 11),
             DATANUM(DATA_board, 132), 4);
    fn_1_C00(MESSNUM(MESS_BOARD_TUTORIAL, 81), DATANUM(DATA_btutorial, 6), DATANUM(DATA_board, 142),
             0);
    mbTutorialCallWait(9);
    mbPlayerMotIdleSet(GwSystem.turnPlayerNo);
    mbTutorialWinMesMasuExec(MESSNUM(MESS_BOARD_TUTORIAL, 84), DATANUM(DATA_btutorial, 2));
    mbTutorialCallWait(9);
    mbPlayerMotIdleSet(GwSystem.turnPlayerNo);
    mbTutorialWinMesMasuExec(MESSNUM(MESS_BOARD_TUTORIAL, 85), DATANUM(DATA_btutorial, 9));
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 88));
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 89));
}

/* Each space lesson shows an illustration and slides in its icon after a movement event. */
void fn_1_C00(s32 message, s32 illustrationData, s32 iconData, s32 iconBank)
{
    s32 animationFrame;
    s32 iconSprite;
    s32 windowId;
    s32 illustrationSprite;
    f32 animationScale;

    mbTutorialCallWait(9);
    mbPlayerMotIdleSet(GwSystem.turnPlayerNo);
    windowId = mbTutorialWinCreate(message);
    illustrationSprite = mbTutorialSprDispOn(mbBoardDataNumGet(illustrationData));
    /* This first wait advances the window transition; its return value is unused. */
    mbTutorialWinWait(windowId);
    mbGuideMotionShiftSet(mbTutorialGuideGet(), 12, TRUE);
    iconSprite = mbTutorialSprCreate(mbBoardDataNumGet(iconData));
    espBankSet(iconSprite, iconBank);
    for (animationFrame = 0; animationFrame < 18U; animationFrame++) {
        animationScale = animationFrame / 18.0f;
        espPosSet(iconSprite, 288.0f,
            -32.0f + 176.0f * mbSinDeg(90.0f * animationScale));
        mbTutorialVSleep();
    }
    mbTutorialWinKeyWait(windowId);
    /* Shrink the illustration while fading the icon after the message finishes. */
    for (animationFrame = 0; animationFrame < 12U; animationFrame++) {
        animationScale = 1.0f - animationFrame / 12.0f;
        espScaleSet(illustrationSprite, animationScale, animationScale);
        espTPLvlSet(iconSprite, animationScale);
        mbTutorialVSleep();
    }
    mbTutorialSprKill(illustrationSprite);
    mbTutorialSprKill(iconSprite);
}

extern void fn_1_F20(f32 time);
extern void fn_1_10A0(f32 time);
extern void mbWipeFadeIn(void);
extern void mbWipeFadeOut(void);

/* The tutorial process plays the opening camera and guide paths before the first turn. */
void fn_1_DF0(void)
{
    OMOBJ *guideObject;
    f32 pathTime;

    pathTime = 0.0f;
    mbTutorialCallWait(0);
    guideObject = mbTutorialGuideGet();
    mbGuideScreenSet(guideObject, FALSE);
    mbObjDispSet(mbGuideModelGet(guideObject), TRUE);
    mbGuideMotionShiftSet(guideObject, 15, FALSE);
    /* Clear the guide's motion-completion tracking; its animation continues playing. */
    mbGuideMotionStop(guideObject);
    mbCameraFocusReset();
    mbCameraMoveOnSet(FALSE);
    fn_1_F20(pathTime);
    fn_1_10A0(pathTime);
    mbWipeFadeIn();
    /* Path time advances by 1.2 units per frame, independently of animation playback. */
    while (pathTime < 600.0f) {
        fn_1_F20(pathTime);
        fn_1_10A0(pathTime);
        mbTutorialVSleep();
        pathTime += 1.20000005f;
    }
    mbWipeFadeOut();
    mbGuideScreenSet(guideObject, TRUE);
    mbObjRotSet(mbGuideModelGet(guideObject), 0.0f, 0.0f, 0.0f);
    mbGuideMotionSet(guideObject, 1, FALSE);
}

/* Shared segment state for the opening's time/value tables, whose time ends with -1. */
typedef struct W11CurveWork {
    s32 startOneSided; /* Nonzero when there is no earlier sample for the start tangent. */
    s32 endOneSided; /* Nonzero when there is no later sample for the end tangent. */
    s32 segmentIndex; /* Index of the segment's first sample. */
    f32 progress; /* Time within the selected segment, from 0.0 to 1.0. */
    f32 startSlopeScale; /* Segment duration divided by the span across the start sample. */
    f32 endSlopeScale; /* Segment duration divided by the span across the end sample. */
} W11CurveWork;

/* Position paths store one time and three world-space coordinates per sample. */
typedef struct W11PositionSample {
    f32 time; /* Path time; a negative value terminates the table. */
    HuVecF position; /* Camera center or guide position at this time. */
} W11PositionSample;

/* Scalar paths store one camera or guide setting at each path time. */
typedef struct W11ScalarSample {
    f32 time; /* Path time; a negative value terminates the table. */
    f32 value; /* Camera distance or camera/guide angle, as described by the table. */
} W11ScalarSample;

extern W11PositionSample lbl_1_data_2C[8];
extern W11ScalarSample lbl_1_data_AC[8];
extern W11ScalarSample lbl_1_data_EC[5];
extern W11ScalarSample lbl_1_data_114[8];
extern void fn_1_12FC(const u8 *values, s32 stride, W11CurveWork *work, f32 time);
extern f32 fn_1_1454(const u8 *values, s32 stride, W11CurveWork *work);

/* The opening loop samples camera center, distance, and X/Y angles at its current path time. */
void fn_1_F20(f32 pathTime)
{
    W11CurveWork curveWork;
    HuVecF cameraCenter;
    f32 zoomOrPitch;

    fn_1_12FC((const u8 *)&lbl_1_data_2C[0].time, 16, &curveWork, pathTime);
    cameraCenter.x = fn_1_1454((const u8 *)&lbl_1_data_2C[0].position.x, 16, &curveWork);
    cameraCenter.y = fn_1_1454((const u8 *)&lbl_1_data_2C[0].position.y, 16, &curveWork);
    cameraCenter.z = fn_1_1454((const u8 *)&lbl_1_data_2C[0].position.z, 16, &curveWork);
    /* Setting the center also removes any board focus targets. */
    mbCameraCenterSetV(&cameraCenter);

    fn_1_12FC((const u8 *)&lbl_1_data_AC[0].time, 8, &curveWork, pathTime);
    zoomOrPitch = fn_1_1454((const u8 *)&lbl_1_data_AC[0].value, 8, &curveWork);
    mbCameraZoomSet(zoomOrPitch);

    fn_1_12FC((const u8 *)&lbl_1_data_EC[0].time, 8, &curveWork, pathTime);
    zoomOrPitch = fn_1_1454((const u8 *)&lbl_1_data_EC[0].value, 8, &curveWork);
    {
        f32 cameraYaw;
        fn_1_12FC((const u8 *)&lbl_1_data_114[0].time, 8, &curveWork, pathTime);
        cameraYaw = fn_1_1454((const u8 *)&lbl_1_data_114[0].value, 8, &curveWork);
        mbCameraRotSet(zoomOrPitch, cameraYaw, 0.0f);
    }
}

extern W11PositionSample lbl_1_data_154[11];
extern W11ScalarSample lbl_1_data_204[8];
extern W11ScalarSample lbl_1_data_244[12];
extern void fn_1_12FC(const u8 *values, s32 stride, W11CurveWork *work, f32 time);
extern f32 fn_1_1454(const u8 *values, s32 stride, W11CurveWork *work);

/* The opening loop samples the guide's world position and X/Y rotation from its path tables. */
void fn_1_10A0(f32 pathTime)
{
    W11CurveWork curveWork;
    HuVecF guidePosition;
    s16 guideModelId;
    f32 guidePitch;
    f32 guideYaw;
    guideModelId = mbGuideModelGet(mbTutorialGuideGet());

    fn_1_12FC((const u8 *)&lbl_1_data_154[0].time, 16, &curveWork, pathTime);
    guidePosition.x = fn_1_1454((const u8 *)&lbl_1_data_154[0].position.x, 16, &curveWork);
    guidePosition.y = fn_1_1454((const u8 *)&lbl_1_data_154[0].position.y, 16, &curveWork);
    guidePosition.z = fn_1_1454((const u8 *)&lbl_1_data_154[0].position.z, 16, &curveWork);
    mbObjPosSetV(guideModelId, &guidePosition);

    fn_1_12FC((const u8 *)&lbl_1_data_204[0].time, 8, &curveWork, pathTime);
    guidePitch = fn_1_1454((const u8 *)&lbl_1_data_204[0].value, 8, &curveWork);
    fn_1_12FC((const u8 *)&lbl_1_data_244[0].time, 8, &curveWork, pathTime);
    guideYaw = fn_1_1454((const u8 *)&lbl_1_data_244[0].value, 8, &curveWork);
    mbObjRotSet(guideModelId, guidePitch, guideYaw, 0.0f);
}

extern void mbWipeSpecialCreate(s16 mode, s16 type, s16 maxTime);
extern void mbWipeSpecialWait(void);
extern void mbWipeFadeOutTime(s16 maxTime);
extern void mbWipeSpecialKill(void);

/* The tutorial process presents the net-scene messages and plays its final exit transition. */
void fn_1_1200(void)
{
    mbTutorialCallWait(23);
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 90));
    mbTutorialCallWait(23);
    mbTutorialCallEnd();
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 91));
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 92));
    /* Suppress START handling and the watcher's extra wipe during this scripted ending. */
    mbTutorialExitOnSet(FALSE);
    mbWipeSpecialCreate(WIPE_MODE_IN, 6, 120);
    mbWipeSpecialWait();
    mbWipeFadeOutTime(1);
    mbWipeSpecialKill();
}

extern void mbNormPosto3D(HuVecF *src, s16 cameraMask, HuVecF *dst);

/* The movement lesson positions the screen-space guide using normalized coordinates and depth. */
void fn_1_127C(float screenX, float screenY, float depth)
{
    HuVecF guidePosition;
    OMOBJ *guideObject;
    s16 guideModelId;

    guidePosition.x = screenX;
    guidePosition.y = screenY;
    guidePosition.z = depth;
    guideObject = mbTutorialGuideGet();
    guideModelId = mbGuideModelGet(guideObject);
    mbNormPosto3D(&guidePosition, HU3D_CAM2, &guidePosition);
    mbObjPosSetV(guideModelId, &guidePosition);
}

/* Camera and guide path updates select a time segment from byte-strided records for
 * interpolation. */
void fn_1_12FC(const u8 *timeSamples, s32 sampleStride, W11CurveWork *curveWork, f32 pathTime)
{
    s32 upperSampleIndex;
    s32 lowerSampleIndex;
    f32 startTime;
    f32 endTime;
    f32 duration;
    f32 segmentProgress;

    upperSampleIndex = 1;
    while (pathTime > *(const f32 *)(timeSamples + sampleStride * upperSampleIndex)) {
        /* A negative time terminates the table; clamp the path at its last valid sample. */
        if (0.0f > *(const f32 *)(timeSamples + sampleStride * upperSampleIndex)) {
            upperSampleIndex--;
            pathTime = *(const f32 *)(timeSamples + sampleStride * upperSampleIndex);
            break;
        }
        upperSampleIndex++;
    }
    lowerSampleIndex = upperSampleIndex - 1;
    startTime = *(const f32 *)(timeSamples + sampleStride * lowerSampleIndex);
    endTime = *(const f32 *)(timeSamples + sampleStride * upperSampleIndex);
    pathTime -= startTime;
    duration = endTime - startTime;
    segmentProgress = pathTime / duration;
    curveWork->segmentIndex = lowerSampleIndex;
    curveWork->progress = segmentProgress;
    curveWork->startOneSided = curveWork->endOneSided = 1;
    curveWork->startSlopeScale = curveWork->endSlopeScale = 0.0f;
    if (lowerSampleIndex > 0) {
        curveWork->startOneSided = 0;
        curveWork->startSlopeScale = duration
            / (endTime - *(const f32 *)(timeSamples + sampleStride * (lowerSampleIndex - 1)));
    }
    if (*(const f32 *)(timeSamples + sampleStride * (upperSampleIndex + 1)) >= 0.0f) {
        curveWork->endOneSided = 0;
        curveWork->endSlopeScale = duration
            / (*(const f32 *)(timeSamples + sampleStride * (upperSampleIndex + 1)) - startTime);
    }
}

extern f32 mbHermiteCalc(f32 a, f32 b, f32 c, f32 d, f32 t);

/* Camera and guide updates interpolate one value channel using the selected time segment. */
f32 fn_1_1454(const u8 *valueSamples, s32 sampleStride, W11CurveWork *curveWork)
{
    f32 startValue;
    f32 endValue;
    f32 valueDelta;
    f32 startTangent;
    f32 endTangent;

    startValue = *(const f32 *)(valueSamples + sampleStride * curveWork->segmentIndex);
    endValue = *(const f32 *)(valueSamples + sampleStride * (curveWork->segmentIndex + 1));
    valueDelta = endValue - startValue;
    /* Endpoints use the segment difference; interior tangents also use neighboring samples. */
    if (curveWork->startOneSided != 0) {
        startTangent = valueDelta;
    } else {
        startTangent = curveWork->startSlopeScale
            * (valueDelta + (startValue
                - *(const f32 *)(valueSamples + sampleStride * (curveWork->segmentIndex - 1))));
    }
    if (curveWork->endOneSided != 0) {
        endTangent = valueDelta;
    } else {
        endTangent =
            curveWork->endSlopeScale *
            (valueDelta +
             (*(const f32 *) (valueSamples + sampleStride * (curveWork->segmentIndex + 2)) -
              endValue));
    }
    return mbHermiteCalc(startValue, endValue, startTangent, endTangent, curveWork->progress);
}

f32 lbl_1_data_20[3] = { -0.75f, -0.699999988f, -750.0f };

/* Camera-center samples: path time followed by world-space X, Y, and Z. */
W11PositionSample lbl_1_data_2C[8] = {
    { 0.0f, { 0.0f, 0.0f, 1000.0f } },
    { 60.0f, { 0.0f, 0.0f, 100.0f } },
    { 120.0f, { 0.0f, 500.0f, 0.0f } },
    { 180.0f, { 0.0f, 1000.0f, 0.0f } },
    { 240.0f, { 0.0f, 2000.0f, 0.0f } },
    { 360.0f, { 0.0f, 2600.0f, 0.0f } },
    { 480.0f, { 0.0f, 2600.0f, 0.0f } },
    { -1.0f, { 0.0f, 0.0f, 0.0f } },
};

/* Camera-distance samples: path time followed by distance from the camera center. */
W11ScalarSample lbl_1_data_AC[8] = {
    { 0.0f, 3000.0f },
    { 60.0f, 3000.0f },
    { 120.0f, 2500.0f },
    { 180.0f, 3000.0f },
    { 300.0f, 2000.0f },
    { 420.0f, 6000.0f },
    { 480.0f, 6000.0f },
    { -1.0f, 0.0f },
};

/* Camera X-angle samples: path time followed by degrees. */
W11ScalarSample lbl_1_data_EC[5] = {
    { 0.0f, -30.0f },
    { 60.0f, -30.0f },
    { 240.0f, -10.0f },
    { 360.0f, -60.0f },
    { -1.0f, 0.0f },
};

/* Camera Y-angle samples: path time followed by degrees. */
W11ScalarSample lbl_1_data_114[8] = {
    { 0.0f, 0.0f },
    { 60.0f, 0.0f },
    { 120.0f, 30.0f },
    { 180.0f, -20.0f },
    { 270.0f, 20.0f },
    { 360.0f, 0.0f },
    { 480.0f, 0.0f },
    { -1.0f, 0.0f },
};

/* Guide-position samples: path time followed by world-space X, Y, and Z. */
W11PositionSample lbl_1_data_154[11] = {
    { 0.0f, { -500.0f, 2000.0f, 4000.0f } },
    { 60.0f, { 500.0f, 100.0f, 300.0f } },
    { 90.0f, { 0.0f, 300.0f, 100.0f } },
    { 120.0f, { 300.0f, 700.0f, 300.0f } },
    { 180.0f, { 100.0f, 1200.0f, 400.0f } },
    { 240.0f, { -200.0f, 2000.0f, 200.0f } },
    { 360.0f, { 100.0f, 2600.0f, 0.0f } },
    { 420.0f, { 0.0f, 4000.0f, 500.0f } },
    { 480.0f, { 300.0f, 5000.0f, 1500.0f } },
    { 540.0f, { -100.0f, 8000.0f, 3000.0f } },
    { -1.0f, { 0.0f, 0.0f, 0.0f } },
};

/* Guide X-angle samples: path time followed by degrees. */
W11ScalarSample lbl_1_data_204[8] = {
    { 0.0f, 40.0f },
    { 60.0f, 20.0f },
    { 120.0f, 0.0f },
    { 180.0f, -10.0f },
    { 240.0f, -30.0f },
    { 420.0f, -40.0f },
    { 480.0f, -60.0f },
    { -1.0f, 0.0f },
};

/* Guide Y-angle samples: path time followed by degrees, including complete turns. */
W11ScalarSample lbl_1_data_244[12] = {
    { 0.0f, -972.0f },
    { 48.0f, -900.0f },
    { 66.0f, -828.0f },
    { 90.0f, -720.0f },
    { 180.0f, -540.0f },
    { 240.0f, -360.0f },
    { 300.0f, -180.0f },
    { 348.0f, 0.0f },
    { 390.0f, 180.0f },
    { 432.0f, 432.000031f },
    { 540.0f, 324.0f },
    { -1.0f, 0.0f },
};

f32 lbl_1_data_2A4[3] = { -0.75f, -0.699999988f, -750.0f };
