/* Board setup and the chain event sequence for the S03 board. */
#include "dolphin.h"
#include "math.h"

#include "game/audio.h"
#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/capsule.h"
#include "game/board/main.h"
#include "game/board/masu.h"
#include "game/board/object.h"
#include "game/board/player.h"
#include "game/board/opening.h"
#include "game/board/pause.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/wipe.h"
#include "datanum/charmot.h"
#include "msm_se.h"
#include "string.h"

typedef void (*VoidFunc)(void);
typedef void (*MBHook)(void);

enum {
    S03_BOARD_NO = 8,
    S03_WORK_CLEAR_SIZE = 112,
    S03_SAVE_CLEAR_SIZE = 8,
    S03_MOTION_DATA_COUNT = 16,
    S03_HOOK_NAME_SIZE = 16,
    S03_HOOK_COUNT = 10,
    S03_OBJECT_LAYER = 3,
    S03_OBJECT_PRIORITY = 8204,
    S03_MASU_ATTR_MOVE_START = 0x0002,
    S03_MASU_ATTR_HATENA = 0x0001,
    S03_MASU_LINK_FLAG = 0x2000,
    S03_ROTATE_HALF_TURN = 180,
    S03_ROTATE_MODE = 15,
    S03_MOVE_DURATION = 90,
    S03_MOVE_START_DELAY = 30,
    S03_RING_HIT_TIME = 18,
    S03_VIBRATION_TIME = 20,
    S03_LIFT_FRAMES = 60,
    S03_SWING_FRAMES = 120,
    S03_CHAIN_FRAMES = 240,
    S03_MUSIC_FADE_SPEED = 1000,
    S03_CAPSULE_LOAD_DELAY = 180,
};

typedef struct S03ParticleWork {
    /* Model instance used for this visible chain segment. */
    s32 particleModelId;
    s32 particleIndex;       /* Segment's slot in the particle array. */
    HuVecF pos;              /* Current position of the segment in board units. */
    HuVecF initialPos;       /* Starting position in board units for the chain effect. */
    HuVecF rotation;         /* Current Euler rotation in degrees. */
    HuVecF unusedBaseRotationCopy; /* Unused copy of the segment's base Euler rotation. */
    HuVecF baseRotation;     /* Segment's fixed Euler angle offset in degrees. */
    HuVecF innerChainPoint;  /* Point before this segment's offset is applied. */
    HuVecF outerChainPoint;  /* Point after this segment's offset is applied. */
} S03ParticleWork;

/* Frame-counted Bezier path used to move the player during the hatena-space sequence. */
typedef struct S03MoveWork {
    s32 playerNo;            /* Player being moved by the event path. */
    s32 elapsedFrames;       /* Update frames elapsed along the path. */
    s32 durationFrames;      /* Total update frames for the path. */
    s32 motionChangeFrame;   /* Frame when the player's motion changes. */
    HuVecF startPos;         /* Start point for the board-space Bezier path. */
    HuVecF controlPos;       /* Control point for the board-space Bezier path. */
    HuVecF endPos;           /* End point for the board-space Bezier path. */
} S03MoveWork;

typedef struct S03Work {
    s16 modelId;                  /* Main S03 board model. */
    s16 pathModelId[2];           /* The two animated paths beside the event. */
    s16 chainAttachedModelId;     /* Character model attached to the chain hook. */
    s16 chainModelId;             /* Animated chain model used to carry the player. */
    s16 sourceModelId;            /* Character model supplying the ch_h1 joint transform for the
                                   * carried player; hidden at setup and shown during the chain
                                   * sequence. */
    s16 markerModelId;            /* Marker hooked to the source model's character joint. */
    s16 eventModelId;             /* Animated model shown at the linked masu. */
    HuVecF chainPos;              /* Initial ch_h1 joint position on the source model, in board
                                   * units. */
    HuVecF chainEndPos;           /* Initial kusari_h2 joint position on the source model, in board
                                   * units. */
    s32 heldPlayerNo;              /* Player carried by the chain, or -1 when none is held. */
    s32 playerOffsetCaptured;      /* 0 before the source-joint offset is sampled; then 1. */
    s32 chainMotionState;          /* 0 idle, 1 chain sway, 2 chain sway and growth. */
    HuVecF rotation;               /* Chain sway phase angles in degrees. */
    HuVecF scale;                  /* Per-axis chain sway amplitude in degrees. */
    HuVecF chainHookOffset;        /* kusari_h1 joint offset on the chain model, in board units. */
    OMOBJ *effectObj;              /* Player-follow object active during the event. */
    S03ParticleWork *particleWork; /* Particle model state for the chain effect. */
    float effectAngle;              /* Angle offset used to compute the source model's position
                                     * around the chain path. */
    HuVecF playerOffset;           /* Player offset from ch_h1, in source-model local units. */
    u32 unusedWord;                /* Not read by this board's code. */
} S03Work;

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
extern OMOBJ *lbl_1_bss_0;        /* Active player-path update object. */
extern S03Work lbl_1_bss_4;       /* Main S03 event state. */
extern u32 *lbl_1_bss_78;         /* Board save-data buffer. */
extern BOOL mbSaveNewF;

/* Fixed board-space positions, camera values and model-joint names used by S03. */
extern HuVecF lbl_1_data_0;
extern HuVecF lbl_1_data_C;
extern HuVecF lbl_1_data_18[2];
extern HuVecF lbl_1_data_30;
extern HuVecF lbl_1_data_3C;
extern float lbl_1_data_48;
extern char lbl_1_data_4C[];
extern char lbl_1_data_52[];
extern char lbl_1_data_5C[];
extern char lbl_1_data_62[];
extern HuVecF lbl_1_data_6C[2];
extern char lbl_1_data_84[];
extern HuVecF lbl_1_data_8C[2];

void mbObjectSetup(s32 boardNo, MBHook init, MBHook close);
void mbLightFuncSet(MBHook setHook, MBHook resetHook);
void mbScrollInit(int dataNum);
void mbMapCameraSet(const HuVecF *rot, const HuVecF *pos, float zoom);
void mbMapHookSet(void (*hook)(BOOL enterF));

int _prolog(void);
void _epilog(void);
void fn_1_A0(void);
void fn_1_F4(void);
void fn_1_5EC(void);
void fn_1_5F0(OMOBJ *obj);
void fn_1_634(void);
int fn_1_638(int playerNo, s16 id);
int fn_1_69C(int playerNo, s16 id);
int fn_1_6CC(int playerNo, s16 id);
void fn_1_728(void);
void fn_1_764(void);
void fn_1_768(BOOL enterF);
void fn_1_76C(int playerNo, s16 id);
void fn_1_1238(OMOBJ *obj);
void fn_1_1450(int playerNo, s16 id);
void fn_1_22F8(OMOBJ *obj);
void fn_1_2670(void);
void fn_1_2858(const HuVecF *pos, HuVecF *out);
void fn_1_2A20(OMOBJ *obj);

void mbPauseDisableSet(BOOL disable);
void mbSingleReturnWrite(void);
void mbWipeDissolveFadeOutTime(int time);
void mbWipeDissolveFadeInTime(int time);
void mbWipeFadeOut(void);
void mbWipeFadeOutTime(int time);
void mbWipeSpecialCreate(int state, int type, int time);
void mbWipeSpecialWait(void);
void mbWipeSpecialKill(void);
int mbBGRead(int dataNum);
void mbBGReadWait(int readStat);
void mbev_CapVecChase(float weight, HuVecF *src, HuVecF *target,
    HuVecF *out);
void mbev_CapBezierGetV(float t, float *a, float *b, float *c, float *out);
float mbev_CapAngleWrap(float a, float b);
float mbev_CapAngleLerp(float a, float b, float t);
OMOBJ *mbev_CapEffRingHitCreate(void);
int mbev_CapEffRingAdd(OMOBJ *obj, HuVecF *pos, HuVecF *rot,
    HuVecF *scale, int kind, int time, int bank, GXColor *color);
void mbev_CapEffRingKill(OMOBJ *obj);
OMOBJ *mbev_CapEffExplodeCreate(void);
void mbev_CapEffDustHeavyAdd(OMOBJ *obj, HuVecF *pos);
int mbev_CapEffExplodeAnimGet(OMOBJ *obj);
void mbev_CapEffExplodeKill(OMOBJ *obj);
void mbev_CapPlayerMotShiftWait(int playerNo, int motionNo, u32 attr,
    BOOL waitF);
void mbev_CapPlayerMotShiftSet(int playerNo, int motionNo, u32 attr,
    BOOL waitF);
void omVibrate(s16 playerNo, s16 time, s16 amp, s16 mode);

/* The REL loader calls this entry point to run constructors and register S03. */
int _prolog(void)
{
    const VoidFunc *ctors = _ctors;

    while (*ctors != 0) {
        (**ctors)();
        ctors++;
    }
    fn_1_A0();
    return 0;
}

/* The REL loader calls this entry point during unload to run registered destructors. */
void _epilog(void)
{
    const VoidFunc *dtors = _dtors;

    while (*dtors != 0) {
        (**dtors)();
        dtors++;
    }
}

/* The REL startup path disables party mode and installs the S03 board callbacks. */
void fn_1_A0(void)
{
    GWPartySet(FALSE);
    mbObjectSetup(S03_BOARD_NO, fn_1_F4, fn_1_5EC);
}

/* Called by mbObjectSetup on board entry; creates the S03 models, paths and event hooks. */
void fn_1_F4(void)
{
    S03Work *work = &lbl_1_bss_4;
    int boardNo = MBBoardNoGet();
    int motData[S03_MOTION_DATA_COUNT];
    Mtx matrix;
    s32 modelId;
    s32 hookModelId;
    int i;
    char name[S03_HOOK_NAME_SIZE];

    HuAudSndGrpSetSet(MSM_GRP_SBRD);
    lbl_1_bss_78 = GwSystem.boardWork;
    mbMasuInit(DATANUM(DATA_s03, 0));
    memset(work, 0, S03_WORK_CLEAR_SIZE);
    work->effectObj = NULL;
    work->particleWork = NULL;

    work->modelId = mbObjCreate(DATANUM(DATA_s03, 2), NULL, FALSE);
    mbObjAttrSet(work->modelId, HU3D_MOTATTR_LOOP);
    mbScrollInit(DATANUM(DATA_s03, 1));
    mbLightFuncSet(fn_1_728, fn_1_764);
    if (mbSaveNewF) {
        memset(lbl_1_bss_78, 0, S03_SAVE_CLEAR_SIZE);
    }

    modelId = mbObjCreate(DATANUM(DATA_s03, 4), NULL, FALSE);
    mbObjMotionSpeedSet(modelId, 0.5f);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    modelId = mbObjCreate(DATANUM(DATA_s03, 5), NULL, FALSE);
    mbObjAttrSet(modelId, (HU3D_MOTATTR_LOOP | HU3D_MOTATTR_SHAPE_LOOP));
    modelId = mbObjCreate(DATANUM(DATA_s03, 6), NULL, FALSE);
    mbObjLayerSet(modelId, S03_OBJECT_LAYER);
    hookModelId = mbObjCreate(DATANUM(DATA_s03, 12), NULL, FALSE);
    mbObjAttrSet(hookModelId, HU3D_MOTATTR_LOOP);
    mbObjLayerSet(hookModelId, S03_OBJECT_LAYER);
    mbObjScaleSet(hookModelId, 1.5f, 1.5f,
        1.5f);
    for (i = 0; i < S03_HOOK_COUNT; i++) {
        sprintf(name, lbl_1_data_4C, i + 1);
        mbObjHookSet(modelId, name, hookModelId);
    }

    for (i = 0; i < 2; i++) {
        modelId = mbObjCreate(DATANUM(DATA_s03, 7), NULL, TRUE);
        work->pathModelId[i] = modelId;
        mbObjPosSetV(modelId, &lbl_1_data_18[i]);
        mbObjMotionSpeedSet(modelId, 0.0f);
    }

    motData[0] = DATANUM(DATA_s03, 14);
    motData[1] = DATANUM(DATA_s03, 15);
    motData[2] = HU_DATANUM_NONE;
    modelId = mbObjCreate(DATANUM(DATA_s03, 13), motData, FALSE);
    work->eventModelId = modelId;
    mbObjDispSet(modelId, FALSE);

    modelId = mbObjCreate(DATANUM(DATA_s03, 11), NULL, FALSE);
    work->chainModelId = modelId;
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    mbObjDispSet(modelId, FALSE);
    Hu3DMotionCalc(mbObjModelIDGet(modelId));
    Hu3DModelObjMtxGet(mbObjModelIDGet(modelId), lbl_1_data_52, matrix);
    work->chainHookOffset.x = matrix[0][3];
    work->chainHookOffset.y = matrix[1][3];
    work->chainHookOffset.z = matrix[2][3];

    modelId = mbObjCreate(DATANUM(DATA_s03, 8), NULL, FALSE);
    work->markerModelId = modelId;
    mbObjPosSetV(modelId, &lbl_1_data_0);

    modelId = mbObjCreate(DATANUM(DATA_s03, 9), NULL, FALSE);
    work->sourceModelId = modelId;
    mbObjDispSet(modelId, FALSE);
    Hu3DMotionCalc(mbObjModelIDGet(work->sourceModelId));
    Hu3DModelObjMtxGet(mbObjModelIDGet(work->sourceModelId), lbl_1_data_5C,
        matrix);
    work->chainPos.x = matrix[0][3];
    work->chainPos.y = matrix[1][3];
    work->chainPos.z = matrix[2][3];
    Hu3DModelObjMtxGet(mbObjModelIDGet(work->sourceModelId), lbl_1_data_62,
        matrix);
    work->chainEndPos.x = matrix[0][3];
    work->chainEndPos.y = matrix[1][3];
    work->chainEndPos.z = matrix[2][3];

    work->heldPlayerNo = -1;
    work->playerOffsetCaptured = 0;
    work->chainMotionState = 0;
    work->rotation.x = work->rotation.y = work->rotation.z =
        0.0f;
    work->scale.x = work->scale.y = work->scale.z = 5.0f;

    fn_1_2670();
    mbev_MasuMoveEndSet(fn_1_69C);
    mbev_MasuMoveStartSet(fn_1_638);
    mbev_MasuHatenaSet(fn_1_6CC);
    mbMapCameraSet(NULL, &lbl_1_data_C, 9800.0f);
    mbMapHookSet(fn_1_768);
    mbOpeningInstHookSet(fn_1_634);
    omAddObjEx(mbObjMan, S03_OBJECT_PRIORITY, 0, 0, OM_GRP_NONE, fn_1_5F0);
    HuDataDirClose(DATANUM(DATA_s03, 0));
    mbOpeningViewSet(&lbl_1_data_30, &lbl_1_data_3C, lbl_1_data_48);
}

/* Called by mbObjectSetup on board exit; this board has no additional close action. */
void fn_1_5EC(void)
{
}

/* Runs each object-manager frame and removes the board watcher after the board exits. */
void fn_1_5F0(OMOBJ *obj)
{
    if (mbExitCheck()) {
        omDelObjEx(mbObjMan, obj);
        return;
    }
}

/* Registered as the opening-instruction hook; S03 adds no instruction here. */
void fn_1_634(void)
{
}

/* Called when a player starts moving; the marked masu launches the S03 chain event. */
int fn_1_638(int playerNo, s16 id)
{
    u32 mAttr = mbMasuMAttrGet(id);

    if (mAttr & S03_MASU_ATTR_MOVE_START) {
        mbPauseDisableSet(TRUE);
        fn_1_1450(playerNo, id);
    }
    return 0;
}

/* Called when a player finishes moving; S03 has no extra move-end action. */
int fn_1_69C(int playerNo, s16 id)
{
    u32 mAttr = mbMasuMAttrGet(id);

    /* The move-end callback reads the masu attributes but performs no S03 action. */
    return 0;
}

/* Called for the board's hatena-space callback and starts its event on marked spaces. */
int fn_1_6CC(int playerNo, s16 id)
{
    u32 mAttr = mbMasuMAttrGet(id);

    if (mAttr & S03_MASU_ATTR_HATENA) {
        fn_1_76C(playerNo, id);
    }
    return 0;
}

/* Called by the board light setup hook to enable light information on the main model. */
void fn_1_728(void)
{
    S03Work *work = &lbl_1_bss_4;

    Hu3DModelLightInfoSet(mbObjModelIDGet(work->modelId), TRUE);
}

/* Called by the board light reset hook; S03 has no extra light reset action. */
void fn_1_764(void)
{
}

/* Called when the map changes; S03 does not add a map-transition action. */
void fn_1_768(BOOL enterF)
{
}

/* Called for marked hatena spaces; moves the event model from the linked masu toward the triggering
 * masu, animates the player away from the triggering masu, then relocates the player to the board's
 * start masu. */
void fn_1_76C(int playerNo, s16 id)
{
    S03Work *work;
    OMOBJ *ringObj;
    OMOBJ *explodeObj;
    HuVecF movePos;
    HuVecF masuPos;
    HuVecF pathPos0;
    HuVecF pathPos1;
    HuVecF ringRot;
    HuVecF ringScale;
    HuVecF ringPosArg;
    HuVecF ringRotArg;
    HuVecF ringScaleArg;
    HuVecF dustPos;
    int masuId;
    GXColor ringColor = { 255, 255, 127, 255 };
    GXColor color;
    int linkedMasu;
    GXColor *colorPtr;
    HuVecF *ringScalePtr;
    HuVecF *ringRotPtr;
    HuVecF *ringPosPtr;
    HuVecF *dustPosPtr;
    int startMasu;
    int pathIndex;
    int time;
    float t;
    Mtx resetMatrix;
    Mtx matrixX;
    Mtx matrixY;
    S03MoveWork *moveWork;

    work = &lbl_1_bss_4;
    ringObj = mbev_CapEffRingHitCreate();

    mbObjPosGet(work->pathModelId[0], &pathPos0);
    mbObjPosGet(work->pathModelId[1], &pathPos1);
    mbMasuPosGet(id, &masuPos);
    PSVECSubtract(&pathPos0, &masuPos, &pathPos0);
    PSVECSubtract(&pathPos1, &masuPos, &pathPos1);
    if (fabs(pathPos0.y) < fabs(pathPos1.y)) {
        pathIndex = 0;
    } else {
        pathIndex = 1;
    }

    masuId = id;
    linkedMasu = mbMasuAttrFindLink(masuId, S03_MASU_LINK_FLAG);
    mbMasuPosGet(masuId, &pathPos0);
    mbMasuPosGet(linkedMasu, &pathPos1);
    mbObjPosSetV(work->eventModelId, &pathPos1);
    mbObjDispSet(work->eventModelId, TRUE);
    mbObjMotionSet(work->eventModelId, 1, HU3D_MOTATTR_LOOP);

    for (time = 1; time < mbMasuNumGet(); time++) {
        if ((mbMasuAttrGet(time) & MASU_FLAG_START) != 0) {
            break;
        }
    }
    startMasu = time;

    omVibrate((s16)playerNo, S03_VIBRATION_TIME, 4, 4);
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f,
        8.0f, HU3D_MOTATTR_LOOP);
    masuPos.x = masuPos.z = 0.0f;
    masuPos.y = 100.0f;
    mbCameraMovePlayer((s16)playerNo, NULL, &masuPos,
        1200.0f, -1.0f, 30);
    mbCameraMoveWait();
    mbAudFXPlay(MSM_SE_SBRD_13);
    mbObjMotionTimeSet(work->pathModelId[pathIndex], 0.0f);
    mbObjMotionSpeedSet(work->pathModelId[pathIndex], 1.0f);
    mbPlayerRotateStart(playerNo, S03_ROTATE_HALF_TURN, S03_ROTATE_MODE);
    HuPrcSleep(10);
    mbAudFXPlay(MSM_SE_GUIDE_77);
    mbObjMotionShiftSet(work->eventModelId, 2, 0.0f,
        8.0f, HU3D_MOTATTR_LOOP);

    for (time = 1; (float)time <= 30.0f; time++) {
        t = (float)time / 30.0f;
        mbev_CapVecChase(t, &pathPos1, &pathPos0, &masuPos);
        masuPos.y += 0.5
            * (100.0
                * sin((M_PI * (180.0f * t))
                    / 180.0));
        mbObjPosSetV(work->eventModelId, &masuPos);
        if (time == 6 || time == 12 || time == 18 || time == 24) {
            mbAudFXPlay(MSM_SE_SBRD_14);
        }
        if (time == 24) {
            lbl_1_bss_0 = omAddObjEx(mbObjMan, S03_OBJECT_PRIORITY, 0, 0,
                OM_GRP_NONE, fn_1_1238);
            mbPlayerMotionSet(playerNo, 9, HU3D_MOTATTR_NONE);
            moveWork = lbl_1_bss_0->data = HuMemDirectMallocNum(
                HEAP_HEAP, sizeof(*moveWork), HU_MEMNUM_OVL);
            memset(moveWork, 0, sizeof(*moveWork));
            moveWork->playerNo = playerNo;
            moveWork->elapsedFrames = 0;
            moveWork->durationFrames = S03_MOVE_DURATION;
            moveWork->motionChangeFrame = S03_MOVE_START_DELAY;
            moveWork->startPos = pathPos0;
            moveWork->controlPos.x = moveWork->startPos.x;
            moveWork->controlPos.y = moveWork->startPos.y
                + 1700.0f;
            moveWork->controlPos.z = moveWork->startPos.z
                + 700.0f;
            moveWork->endPos.x = moveWork->startPos.x;
            moveWork->endPos.y = moveWork->startPos.y
                - 1000.0f;
            moveWork->endPos.z = moveWork->startPos.z
                + 1400.0f;

            masuPos.x = pathPos0.x;
            masuPos.y = 100.0f + pathPos0.y;
            masuPos.z = 120.00001f + pathPos0.z;
            ringRot.x = ringRot.y = ringRot.z = 0.0f;
            ringScale.x = 0.5f;
            ringScale.y = 3.0f;
            ringScale.z = 100.0f
                * (1.0f
                    + (0.25f * MBCapsuleEffRandF()));
            color = ringColor;
            colorPtr = &color;
            ringScaleArg = ringScale;
            ringScalePtr = &ringScaleArg;
            ringRotArg = ringRot;
            ringRotPtr = &ringRotArg;
            ringPosArg = masuPos;
            ringPosPtr = &ringPosArg;
            mbev_CapEffRingAdd(ringObj, ringPosPtr, ringRotPtr,
                ringScalePtr, 1, S03_RING_HIT_TIME, 2, colorPtr);
            mbAudFXPlay(MSM_SE_SBRD_15);
            omVibrate((s16)playerNo, S03_VIBRATION_TIME, 7, 3);
        }
        HuPrcVSleep();
    }

    mbObjMotionShiftSet(work->eventModelId, 1, 0.0f,
        8.0f, HU3D_MOTATTR_LOOP);
    while (lbl_1_bss_0 != NULL) {
        HuPrcVSleep();
    }
    mbWipeDissolveFadeOutTime(1);
    masuPos.x = masuPos.z = 0.0f;
    masuPos.y = 100.0f;
    mbCameraMoveMasu(startMasu, NULL, &masuPos, 1200.0f,
        -1.0f, -1);
    mbCameraMoveWait();
    mbMasuPosGet(startMasu, &pathPos0);
    pathPos1.x = pathPos0.x;
    pathPos1.y = pathPos0.y + 800.0f;
    pathPos1.z = pathPos0.z - 200.0f;
    mbWipeDissolveFadeInTime(S03_MOVE_START_DELAY);
    mbPlayerDispSet(playerNo, TRUE);
    for (time = 1; (float)time < 30.0f; time++) {
        t = (float)time / 30.0f;
        t = (float)cos((M_PI * (90.0f * t))
            / 180.0);
        mbev_CapVecChase(t, &pathPos0, &pathPos1, &masuPos);
        mbPlayerPosSetV(playerNo, &masuPos);
        MTXRotDeg(matrixX, 'X', 180.0f);
        MTXRotDeg(matrixY, 'Y', 720.0f * t);
        PSMTXConcat(matrixX, matrixY, matrixX);
        mbPlayerMtxSet(playerNo, &matrixX);
        HuPrcVSleep();
    }
    omVibrate((s16)playerNo, S03_VIBRATION_TIME, 20, 0);
    mbAudFXPlay(MSM_SE_GUIDE_71);
    {
        explodeObj = mbev_CapEffExplodeCreate();

        mbMasuPosGet(startMasu, &pathPos0);
        dustPos = pathPos0;
        dustPosPtr = &dustPos;
        mbev_CapEffDustHeavyAdd(explodeObj, dustPosPtr);
        mbMasuPosGet(startMasu, &movePos);
        mbPlayerPosSetV(playerNo, &movePos);
        mbPlayerRotSet(playerNo, 0.0f, 0.0f,
            0.0f);
        PSMTXIdentity(resetMatrix);
        mbPlayerMtxSet(playerNo, &resetMatrix);
        GwPlayer[playerNo].masuId = startMasu;
        HuPrcSleep(S03_MOVE_START_DELAY);
        while (mbev_CapEffExplodeAnimGet(explodeObj) > 0) {
            HuPrcVSleep();
        }
        mbev_CapEffExplodeKill(explodeObj);
    }
    mbWipeFadeOut();
    mbMasuPosGet(startMasu, &movePos);
    mbPlayerPosSetV(playerNo, &movePos);
    mbPlayerRotSet(playerNo, 0.0f, 0.0f,
        0.0f);
    PSMTXIdentity(resetMatrix);
    mbPlayerMtxSet(playerNo, &resetMatrix);
    GwPlayer[playerNo].masuId = startMasu;
    mbPlayerDispSet(playerNo, TRUE);
    mbObjMotionTimeSet(work->pathModelId[pathIndex], 0.0f);
    mbObjMotionSpeedSet(work->pathModelId[pathIndex], 0.0f);
    mbObjDispSet(work->eventModelId, FALSE);
    mbev_CapEffRingKill(ringObj);
}

/* Hatena-sequence update that advances and rotates the player along a Bezier path, switches to
 * motion 6 at frame 30, hides the player at the endpoint, and removes the updater on the following
 * update. */
void fn_1_1238(OMOBJ *obj)
{
    S03MoveWork *moveWork = obj->data;
    HuVecF playerPos;
    Mtx matrixX;
    Mtx matrixY;
    float t;

    if (mbExitCheck() || moveWork->elapsedFrames >= moveWork->durationFrames) {
        lbl_1_bss_0 = NULL;
        omDelObjEx(mbObjMan, obj);
        return;
    }
    t = (float)++moveWork->elapsedFrames / (float)moveWork->durationFrames;
    mbev_CapBezierGetV(t, (float *)&moveWork->startPos,
        (float *)&moveWork->controlPos, (float *)&moveWork->endPos,
        (float *)&playerPos);
    mbPlayerPosSetV(moveWork->playerNo, &playerPos);
    MTXRotDeg(matrixX, 'X',
        180.0 * -sin((M_PI * (90.0f * t)) / 180.0));
    MTXRotDeg(matrixY, 'Y', 720.0f * t);
    PSMTXConcat(matrixX, matrixY, matrixX);
    mbPlayerMtxSet(moveWork->playerNo, &matrixX);
    if (moveWork->elapsedFrames == moveWork->motionChangeFrame) {
        mbPlayerMotionShiftSet(moveWork->playerNo, 6, 0.0f,
            8.0f, HU3D_MOTATTR_LOOP);
    }
    if (t >= 1.0f) {
        mbPlayerDispSet(moveWork->playerNo, FALSE);
    }
}

/* Runs when a player starts moving from the marked masu: lifts the player to the chain, swings them
 * through the event, then writes the single-event return state. */
void fn_1_1450(int playerNo, s16 id)
{
    S03Work *work = &lbl_1_bss_4;
    int playerMotion[16];
    HuVecF playerPos;
    HuVecF targetPos;
    HuVecF movePos;
    HuVecF delta;
    HuVecF modelPos;
    HuVecF pos;
    HuVecF curveA;
    HuVecF curveB;
    HuVecF curveC;
    HuVecF dustPos;
    OMOBJ *moveObj;
    OMOBJ *explodeObj;
    s32 readStat;
    int currentMasu;
    HuVecF *dustPosPtr;
    int time;
    float t;
    float scale;
    float angle;

    if (GwPlayer[playerNo].charNo == 2 || GwPlayer[playerNo].charNo == 5) {
        playerMotion[0] = mbPlayerMotionCreate(playerNo,
            CHARMOT_HSF_c000m1_399);
    } else {
        playerMotion[0] = mbPlayerMotionCreate(playerNo,
            CHARMOT_HSF_c000m1_318);
    }
    playerMotion[1] = mbPlayerMotionCreate(playerNo,
        CHARMOT_HSF_c000m1_355);
    playerMotion[2] = mbPlayerMotionCreate(playerNo,
        CHARMOT_HSF_c000m1_345);
    playerMotion[3] = mbPlayerMotionCreate(playerNo,
        CHARMOT_HSF_c000m1_368);

    mbMoveNumDispSet(playerNo, FALSE);
    mbCameraMovePlayer((s16)playerNo, &lbl_1_data_6C[0], NULL,
        900.0f, -1.0f, S03_SWING_FRAMES);
    currentMasu = GwPlayer[playerNo].masuId;
    mbMasuPosGet((s16)currentMasu, &playerPos);
    mbMasuPosGet(id, &targetPos);
    PSVECSubtract(&targetPos, &playerPos, &delta);
    angle = (float)(180.0
        * (atan2(delta.x, delta.z) / M_PI));
    mbPlayerRotYSet(playerNo, angle);
    mbPlayerMotionShiftSet(playerNo, 2, 0.0f,
        8.0f, HU3D_MOTATTR_LOOP);

    for (time = 0; (u32)time <= S03_LIFT_FRAMES; time++) {
        scale = 0.4f * ((float)time / 60.0f);
        movePos.x = playerPos.x + scale * delta.x;
        movePos.y = playerPos.y;
        movePos.z = playerPos.z + scale * delta.z;
        mbPlayerPosSetV(playerNo, &movePos);
        if (time == 0 || time == S03_MOVE_START_DELAY || time == S03_LIFT_FRAMES
            || time == S03_MOVE_DURATION || time == S03_SWING_FRAMES) {
            omVibrate((s16)playerNo, S03_VIBRATION_TIME, 7, 3);
        }
        HuPrcVSleep();
    }

    mbPlayerMotionShiftSet(playerNo, 4, 0.0f,
        8.0f, HU3D_MOTATTR_NONE);
    for (time = 0; (u32)time <= S03_MOVE_START_DELAY; time++) {
        t = (float)time / 30.0f;
        scale = 0.4f + 0.4f * t;
        movePos.x = playerPos.x + scale * delta.x;
        movePos.y = playerPos.y + t * delta.y
            + (2.0
                * (100.0
                    * sin((M_PI * (180.0f * t))
                        / 180.0)));
        movePos.z = playerPos.z + scale * delta.z;
        mbPlayerPosSetV(playerNo, &movePos);
        if (time == S03_VIBRATION_TIME) {
            mbPlayerMotionShiftSet(playerNo, 5, 0.0f,
                8.0f, HU3D_MOTATTR_NONE);
        }
        if (time == 0 || time == S03_MOVE_START_DELAY || time == S03_LIFT_FRAMES
            || time == S03_MOVE_DURATION || time == S03_SWING_FRAMES) {
            omVibrate((s16)playerNo, S03_VIBRATION_TIME, 7, 3);
        }
        HuPrcVSleep();
    }

    mbPlayerMotionShiftSet(playerNo, 2, 0.0f,
        8.0f, HU3D_MOTATTR_LOOP);
    for (time = 0; (u32)time <= S03_MOVE_START_DELAY; time++) {
        t = (float)time / 30.0f;
        scale = 0.8f + 0.2f * t;
        movePos.x = playerPos.x + scale * delta.x;
        movePos.y = playerPos.y + delta.y;
        movePos.z = playerPos.z + scale * delta.z;
        mbPlayerPosSetV(playerNo, &movePos);
        if (time == 0 || time == S03_MOVE_START_DELAY || time == S03_LIFT_FRAMES
            || time == S03_MOVE_DURATION || time == S03_SWING_FRAMES) {
            omVibrate((s16)playerNo, S03_VIBRATION_TIME, 7, 3);
        }
        HuPrcVSleep();
    }

    mbPlayerMotionShiftSet(playerNo, playerMotion[1],
        0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    HuPrcSleep(S03_SWING_FRAMES);
    mbCameraMoveWait();
    mbObjDispSet(work->sourceModelId, TRUE);
    for (time = 1; (u32)time <= S03_MOVE_START_DELAY; time++) {
        scale = (float)time / 30.0f;
        modelPos = lbl_1_data_0;
        modelPos.y += 5.0
            * (100.0
                * cos((M_PI * (90.0f * scale))
                    / 180.0));
        PSVECSubtract(&modelPos, &work->chainPos, &modelPos);
        mbObjPosSetV(work->sourceModelId, &modelPos);
        HuPrcVSleep();
    }
    mbObjHookSet(work->sourceModelId, lbl_1_data_5C,
        work->markerModelId);
    mbAudFXPlay(MSM_SE_SBRD_16);
    {
        explodeObj = mbev_CapEffExplodeCreate();

        dustPos = targetPos;
        dustPosPtr = &dustPos;
        mbev_CapEffDustHeavyAdd(explodeObj, dustPosPtr);
        omVibrate((s16)playerNo, S03_VIBRATION_TIME, 20, 0);
        mbev_CapPlayerMotShiftWait(playerNo, 9, HU3D_MOTATTR_NONE, TRUE);
        mbCameraFocusObjSet(HU3D_MODELID_NONE);
        mbPlayerMotionShiftSet(playerNo, playerMotion[2],
            0.0f, 8.0f, HU3D_MOTATTR_LOOP);

        moveObj = omAddObjEx(mbObjMan, S03_OBJECT_PRIORITY, 0, 0, OM_GRP_NONE,
            fn_1_22F8);
        work->effectObj = moveObj;
        moveObj->work[0] = playerNo;
        moveObj->work[1] = 0;
        moveObj->work[2] = 0;
        moveObj->work[3] = 0;
        moveObj->trans.x = moveObj->scale.x = targetPos.x;
        moveObj->trans.y = moveObj->scale.y = targetPos.y;
        moveObj->trans.z = moveObj->scale.z = targetPos.z;
        moveObj->rot.x = 0.0f;
        moveObj->rot.y = angle;
        moveObj->rot.z = 0.0f;

        readStat = mbBGRead(DATANUM(DATA_capsulechar1, 0));
        HuPrcSleep(S03_CAPSULE_LOAD_DELAY);
        mbBGReadWait(readStat);
        work->chainAttachedModelId = mbObjCreate(DATANUM(DATA_capsulechar1, 0), NULL,
            FALSE);
        mbObjDispSet(work->chainAttachedModelId, FALSE);
        HuPrcVSleep();
        mbObjMotionCreate(work->chainAttachedModelId, DATANUM(DATA_capsulechar1, 1));
        HuPrcVSleep();
        mbObjMotionCreate(work->chainAttachedModelId, DATANUM(DATA_capsulechar1, 3));
        HuPrcVSleep();
        HuDataDirClose(DATANUM(DATA_capsulechar1, 0));
        mbObjMotionSet(work->chainAttachedModelId, 1, HU3D_MOTATTR_LOOP);
        mbObjHookSet(work->chainModelId, lbl_1_data_84, work->chainAttachedModelId);
        mbObjPosGet(work->sourceModelId, &modelPos);
        fn_1_2858(&modelPos, &pos);
        mbObjPosSetV(work->chainModelId, &pos);
        mbObjDispSet(work->chainModelId, TRUE);
        mbObjDispSet(work->chainAttachedModelId, TRUE);
        moveObj->work[2] = 1;
        while (work->effectObj) {
            HuPrcVSleep();
        }
        mbPlayerRotateStart(playerNo, 0, S03_ROTATE_MODE);
        while (!mbPlayerRotateCheck(playerNo)) {
            HuPrcVSleep();
        }
        mbPlayerMotionShiftSet(playerNo, playerMotion[3],
            0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        HuPrcSleep(S03_LIFT_FRAMES);
        mbAudFXPlay(MSM_SE_SBRD_17);
        modelPos.x = modelPos.z = 0.0f;
        modelPos.y = 600.0f;
        mbCameraMoveMasu(id, &lbl_1_data_6C[0], &modelPos,
            3000.0f, -1.0f, S03_MOVE_DURATION);
        work->heldPlayerNo = playerNo;
        HuPrcSleep(S03_MOVE_START_DELAY);
        mbAudFXDelaySet(S03_MOVE_START_DELAY);
        mbAudFXPlay(MSM_SE_GUIDE_47);
        mbev_CapPlayerMotShiftSet(work->chainAttachedModelId, 2, HU3D_MOTATTR_NONE,
            TRUE);
        mbObjMotionShiftSet(work->chainAttachedModelId, 1, 0.0f,
            8.0f, HU3D_MOTATTR_LOOP);
        mbPlayerMotionShiftSet(playerNo, playerMotion[0], 0,
            8.0f, HU3D_MOTATTR_NONE);
        omVibrate((s16)playerNo, S03_VIBRATION_TIME, 7, 3);
        work->chainMotionState++;
    }

    for (time = 1; (u32)time < S03_LIFT_FRAMES; time++) {
        t = (float)time / S03_LIFT_FRAMES;
        modelPos.x = pos.x;
        modelPos.y = pos.y + (2.0
            * (100.0
                * sin((M_PI * (90.0f * t))
                    / 180.0)));
        modelPos.z = pos.z;
        mbObjPosSetV(work->chainModelId, &modelPos);
        HuPrcVSleep();
    }
    mbObjPosGet(work->chainModelId, &pos);
    for (time = 1; (u32)time < S03_SWING_FRAMES; time++) {
        t = (float)time / S03_SWING_FRAMES;
        modelPos.x = pos.x;
        modelPos.y = pos.y + (0.1f
            * (100.0
                * sin((M_PI * (720.0f * t))
                    / 180.0)));
        modelPos.z = pos.z;
        mbObjPosSetV(work->chainModelId, &modelPos);
        mbObjRotSet(work->chainModelId, 0.0f,
            (float)(180.0
                * sin((M_PI * (90.0f * t))
                    / 180.0)),
            0.0f);
        HuPrcVSleep();
    }
    mbObjPosGet(work->chainModelId, &curveA);
    curveB.x = curveA.x + 1500.0f;
    curveB.y = curveA.y;
    curveB.z = curveA.z;
    curveC.x = curveA.x;
    curveC.y = curveA.y - 500.0f;
    curveC.z = curveA.z - 3000.0f;
    work->chainMotionState++;
    omVibrate((s16)playerNo, S03_CHAIN_FRAMES, 4, 4);
    for (time = 1; (u32)time < S03_CHAIN_FRAMES; time++) {
        float curvePhase;

        t = (float)time / S03_CHAIN_FRAMES;
        curvePhase = (float)time / S03_SWING_FRAMES;
        scale = (float)sin((M_PI * (90.0f * (t * t))) / 180.0);
        mbev_CapBezierGetV(
            scale,
            (float *)&curveA, (float *)&curveB, (float *)&curveC,
            (float *)&pos);
        pos.y += 0.1f
            * (100.0
                * sin((M_PI * (720.0f * t))
                    / 180.0));
        mbObjPosSetV(work->chainModelId, &pos);
        if (time == S03_SWING_FRAMES) {
            mbWipeSpecialCreate(WIPE_MODE_IN, WIPE_TYPE_WHITE,
                S03_SWING_FRAMES);
        }
        if (time == S03_CAPSULE_LOAD_DELAY) {
            mbMusFadeOutSpeed(MB_MUS_CHAN_BG, S03_MUSIC_FADE_SPEED);
            mbMusFadeOutSpeed(MB_MUS_CHAN_FG, S03_MUSIC_FADE_SPEED);
        }
        HuPrcVSleep();
    }
    mbWipeSpecialWait();
    mbWipeFadeOutTime(1);
    mbWipeSpecialKill();
    for (time = 0; time < 4; time++) {
        mbPlayerMotionKill(playerNo, playerMotion[time]);
    }
    mbev_CapEffExplodeKill(explodeObj);
    mbSingleReturnWrite();
}

/* Registered by fn_1_1450 as an object-manager callback; moves the player to a random nearby point
 * and back to the event point, then ends the effect. Each update faces the player toward the
 * current point and advances toward it, slowing as the yaw gap grows. */
void fn_1_22F8(OMOBJ *obj)
{
    S03Work *work = &lbl_1_bss_4;
    HuVecF basePos;
    HuVecF playerPos;
    HuVecF objPos;
    HuVecF delta;
    HuVecF playerRot;
    float targetYaw;
    float wrappedYaw;
    float angleRate;
    int playerNo;

    if (mbExitCheck() || obj->work[3] != 0) {
        work->effectObj = NULL;
        omDelObjEx(mbObjMan, obj);
        return;
    }

    playerNo = (int)obj->work[0];
    mbPlayerPosGet(playerNo, &playerPos);
    objPos.x = obj->trans.x;
    objPos.y = obj->trans.y;
    objPos.z = obj->trans.z;
    basePos.x = obj->scale.x;
    basePos.y = obj->scale.y;
    basePos.z = obj->scale.z;
    PSVECSubtract(&objPos, &playerPos, &delta);
    if (PSVECMag(&delta) < 10.0f) {
        switch ((int)obj->work[2]) {
        case 0:
            /* The first arrival sends the player to a random point within 50 board units on X and
             * Z. */
            obj->trans.x = basePos.x
                + (100.0f
                    * (-0.5f + MBCapsuleEffRandF()));
            obj->trans.y = basePos.y;
            obj->trans.z = basePos.z
                + (100.0f
                    * (-0.5f + MBCapsuleEffRandF()));
            break;
        case 1:
            /* After the random point, return to the original event point. */
            obj->trans.x = basePos.x;
            obj->trans.y = basePos.y;
            obj->trans.z = basePos.z;
            obj->work[2]++;
            break;
        case 2:
            /* Reaching the event point again lets the next callback update delete this effect
             * object. */
            obj->work[3] = 1;
            break;
        }
        objPos.x = obj->trans.x;
        objPos.y = obj->trans.y;
        objPos.z = obj->trans.z;
        PSVECSubtract(&objPos, &playerPos, &delta);
    }

    mbPlayerRotGet(playerNo, &playerRot);
    targetYaw = (float)(180.0
        * (atan2(delta.x, delta.z) / M_PI));
    wrappedYaw = mbev_CapAngleWrap(targetYaw, playerRot.y);
    angleRate = (float)(10.0
        * (1.0
            - (fabs(wrappedYaw) / 180.0)));
    playerRot.y = mbev_CapAngleLerp(targetYaw, playerRot.y,
        10.0f);
    if (PSVECMag(&delta) > 0.0f) {
        PSVECNormalize(&delta, &delta);
    }
    PSVECScale(&delta, &delta, angleRate);
    PSVECAdd(&playerPos, &delta, &playerPos);
    mbPlayerPosSetV(playerNo, &playerPos);
    mbPlayerRotSetV(playerNo, &playerRot);
}

/* Called during board setup to allocate and initialize the chain particle model. */
void fn_1_2670(void)
{
    S03Work *work = &lbl_1_bss_4;
    S03ParticleWork *particleBase;
    S03ParticleWork *particle;
    int i;

    particleBase = HuMemDirectMallocNum(HEAP_HEAP,
        sizeof(*work->particleWork), HU_MEMNUM_OVL);
    work->particleWork = particleBase;
    particle = particleBase;
    memset(work->particleWork, 0, sizeof(*work->particleWork));
    for (i = 0; i < 1; i++, particle++) {
        particle->particleModelId = mbObjCreate(DATANUM(DATA_s03, 10), NULL,
            TRUE);
        mbObjDispSet(particle->particleModelId, FALSE);
        particle->particleIndex = i;

        particle->pos.x = particle->pos.y = particle->pos.z
            = 0.0f;
        particle->initialPos = particle->pos;

        particle->baseRotation.x = particle->baseRotation.z = 0.0f;
        particle->baseRotation.y = 90.0f * (float)(i + 1);
        particle->rotation = particle->baseRotation;
        particle->unusedBaseRotationCopy = particle->baseRotation;
        particle->innerChainPoint = particle->pos;
        particle->outerChainPoint = particle->pos;
    }
    work->effectAngle = 90.0f * (float)(4 - (i % 4));
}

/* Called as the chain enters the event; positions its particle and starts its frame updater. */
void fn_1_2858(const HuVecF *pos, HuVecF *out)
{
    S03Work *work = &lbl_1_bss_4;
    S03ParticleWork *particle = work->particleWork;
    int i;

    for (i = 0; i < 1; i++) {
        particle = &work->particleWork[-i];
        particle->pos = *pos;
        particle->pos.y += -25.0f
            + (50.0f * (float)(i + 1));
        particle->initialPos = particle->pos;

        particle->innerChainPoint = particle->pos;
        particle->innerChainPoint.y += 25.0f;
        particle->outerChainPoint = particle->pos;
        particle->outerChainPoint.y -= 25.0f;

        mbObjDispSet(particle->particleModelId, TRUE);
        mbObjPosSetV(particle->particleModelId, &particle->pos);
        mbObjRotSetV(particle->particleModelId, &particle->rotation);
    }
    *out = particle->innerChainPoint;
    PSVECSubtract(out, &work->chainHookOffset, out);
    omAddObjEx(mbObjMan, S03_OBJECT_PRIORITY, 0, 0, OM_GRP_NONE, fn_1_2A20);
}

/* Runs each object-manager frame to animate chain sway and growth, pose the chain particle and
 * source model from the chain path, and align the carried player to the source model joint. */
void fn_1_2A20(OMOBJ *obj)
{
    S03Work *work = &lbl_1_bss_4;
    S03ParticleWork *particle = work->particleWork;
    HuVecF chainPos;
    HuVecF targetPos;
    HuVecF chainRot;
    HuVecF offset;
    HuVecF playerPos;
    HuVecF particlePos;
    HuVecF rot;
    Mtx matrix;
    int i;

    mbObjPosGet(work->chainModelId, &chainPos);
    PSVECAdd(&chainPos, &work->chainHookOffset, &targetPos);
    mbObjRotGet(work->chainModelId, &chainRot);
    switch (work->chainMotionState) {
    case 0:
        break;
    case 1:
        if ((work->rotation.y += 3.0f * MBCapsuleEffRandF()) > 360.0f) {
            work->rotation.y -= 360.0f;
        }
        chainRot.y += work->scale.y
            * sin((M_PI * work->rotation.y)
                / 180.0);
        if ((work->rotation.z += 3.0f
                + (2.0f * MBCapsuleEffRandF())) > 360.0f) {
            work->rotation.z -= 360.0f;
        }
        chainRot.z += work->scale.z
            * sin((M_PI * work->rotation.z)
                / 180.0);
        break;
    case 2:
        work->scale.x += 0.05f;
        work->scale.y += 0.05f;
        work->scale.z += 0.05f;
        if ((work->rotation.x += 3.0f
                + (2.0f * MBCapsuleEffRandF())) > 360.0f) {
            work->rotation.x -= 360.0f;
        }
        chainRot.x += work->scale.x
            * sin((M_PI * work->rotation.x)
                / 180.0);
        if ((work->rotation.y += 3.0f * MBCapsuleEffRandF()) > 360.0f) {
            work->rotation.y -= 360.0f;
        }
        chainRot.y += work->scale.y
            * sin((M_PI * work->rotation.y)
                / 180.0);
        if ((work->rotation.z += 3.0f
                + (2.0f * MBCapsuleEffRandF())) > 360.0f) {
            work->rotation.z -= 360.0f;
        }
        chainRot.z += work->scale.z
            * sin((M_PI * work->rotation.z)
                / 180.0);
        break;
    }

    particlePos = targetPos;
    particlePos.y += 25.0f;
    rot = chainRot;
    for (i = 0; i < 1; i++, particle++) {
        PSVECAdd(&rot, &particle->baseRotation, &particle->rotation);
        mtxRot(matrix, rot.x, rot.y, rot.z);
        PSMTXMultVec(matrix, &lbl_1_data_8C[0], &offset);
        particle->innerChainPoint = particlePos;
        PSVECAdd(&particlePos, &offset, &particle->pos);
        PSVECAdd(&particle->pos, &offset, &particlePos);
        particle->outerChainPoint = particlePos;
        mbObjPosSetV(particle->particleModelId, &particle->pos);
        mbObjRotSetV(particle->particleModelId, &particle->rotation);
    }

    mtxRot(matrix, rot.x, rot.y + work->effectAngle, rot.z);
    PSMTXMultVec(matrix, &lbl_1_data_8C[0], &offset);
    PSVECAdd(&particlePos, &offset, &particlePos);
    mbObjPosSetV(work->sourceModelId, &particlePos);
    mbObjRotSetV(work->sourceModelId, &rot);

    if (work->heldPlayerNo >= 0) {
        if (work->playerOffsetCaptured == 0) {
            /* This sample reads player 0's position even when another player is held. */
            mbPlayerPosGet(work->playerOffsetCaptured, &work->playerOffset);
            Hu3DModelObjMtxGet(mbObjModelIDGet(work->sourceModelId),
                lbl_1_data_5C, matrix);
            work->playerOffset.x -= matrix[0][3];
            work->playerOffset.y -= matrix[1][3];
            work->playerOffset.z -= matrix[2][3];
            work->playerOffsetCaptured = 1;
        }
        Hu3DMotionCalc(mbObjModelIDGet(work->sourceModelId));
        Hu3DModelObjMtxGet(mbObjModelIDGet(work->sourceModelId),
            lbl_1_data_5C, matrix);
        PSMTXMultVec(matrix, &work->playerOffset, &playerPos);
        matrix[0][3] = matrix[1][3] = matrix[2][3] = 0.0f;
        mbPlayerMtxSet(work->heldPlayerNo, &matrix);
        mbPlayerPosSetV(work->heldPlayerNo, &playerPos);
        mbPlayerRotSet(work->heldPlayerNo, 0.0f,
            0.0f, 0.0f);
        mbPlayerScaleSet(work->heldPlayerNo, 1.0f,
            1.0f, 1.0f);
    }
}

HuVecF lbl_1_data_0 = { 327.0f, 5221.0f, 320.0f };
HuVecF lbl_1_data_C = { 0.0f, 0.0f, 300.0f };
HuVecF lbl_1_data_18[2] = {
    { 0.0f, 4500.0f, 636.0f },
    { 0.0f, 2500.0f, 636.0f },
};
HuVecF lbl_1_data_30 = { -51.0f, 0.0f, 0.0f };
HuVecF lbl_1_data_3C = { 1.0f, 7419.0f, 3655.0f };
float lbl_1_data_48 = 3898.0f;
char lbl_1_data_4C[] = "b_h%d";
char lbl_1_data_52[] = "kusari_h1";
char lbl_1_data_5C[] = "ch_h1";
char lbl_1_data_62[] = "kusari_h2";
HuVecF lbl_1_data_6C[2] = {
    { -15.0f, 0.0f, 0.0f },
    { -90.0f, 0.0f, 0.0f },
};
char lbl_1_data_84[] = "koopa_h";
HuVecF lbl_1_data_8C[2] = {
    { 0.0f, -25.0f, 0.0f },
    { 0.0f, 50.0f, 0.0f },
};

u32 *lbl_1_bss_78;
S03Work lbl_1_bss_4;
OMOBJ *lbl_1_bss_0;
