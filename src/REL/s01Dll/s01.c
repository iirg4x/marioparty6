#include "dolphin.h"
#include "math.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/process.h"
#include "game/board/main.h"
#include "game/board/masu.h"
#include "game/board/object.h"

typedef void (*VoidFunc)(void);

static inline int MBBoardNoGet(void)
{
    return GwSystem.boardNo;
}

typedef struct S01Marker {
    s16 modelId;
    s16 masuId;
} S01Marker;

typedef struct S01Npc {
    s16 modelId;
    s16 pad;
    s32 eventNo;
} S01Npc;

typedef struct S01PosRot {
    HuVecF pos;
    float rotY;
} S01PosRot;

typedef struct S01PlayerMoveWork {
    s16 motionId;
    HuVecF pos;
    HuVecF targetPos;
    HuVecF startPos;
} S01PlayerMoveWork;

typedef struct S01MotionPair {
    s16 first;
    s16 second;
} S01MotionPair;

typedef struct S01WarpWork {
    HuVecF delta;
    HuVecF pos;
    HuVecF jumpPos;
    HuVecF targetPos;
    HuVecF playerPos;
} S01WarpWork;

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
extern float lbl_1_rodata_78;
extern float lbl_1_rodata_7C;
extern float lbl_1_rodata_80;
extern float lbl_1_rodata_84;
extern float lbl_1_rodata_88;
extern float lbl_1_rodata_8C;
extern float lbl_1_rodata_90;
extern float lbl_1_rodata_94;
extern float lbl_1_rodata_98;
extern float lbl_1_rodata_9C;
extern float lbl_1_rodata_A0;
extern float lbl_1_rodata_A4;
extern float lbl_1_rodata_A8;
extern float lbl_1_rodata_AC;
extern float lbl_1_rodata_B0;
extern float lbl_1_rodata_B4;
extern s32 lbl_1_bss_44;

void fn_1_A0(void);
void fn_1_F4(void);
void fn_1_270(OMOBJ *obj);
void fn_1_274(OMOBJ *obj);
int fn_1_2B8(int playerNo, s16 id);
int fn_1_314(int playerNo, s16 id);
int fn_1_374(int playerNo, s16 id);
void fn_1_3E0(int playerNo);
void fn_1_3E4(int playerNo);
void fn_1_404(void);
void fn_1_468(void);
void fn_1_46C(BOOL enterF);
void fn_1_4C8(void);
void fn_1_590(int playerNo, int eventNo);
void fn_1_87C(void);
void fn_1_8F0(void);
void fn_1_9B8(int playerNo, int eventNo);
void fn_1_A50(void);
void fn_1_AFC(void);
void fn_1_C38(void);
void fn_1_CEC(void);
void fn_1_DAC(int playerNo, s16 id);

void mbObjectSetup(s32 boardNo, void (*init)(void), void (*close)(OMOBJ *));
void HuAudSndGrpSetSet(s16 grpSet);
void HuDataDirClose(int dataNum);
void mbLightFuncSet(VoidFunc setHook, VoidFunc resetHook);
void mbMapCameraSet(const HuVecF *rot, const HuVecF *pos, float zoom);
void mbMapHookSet(void (*hook)(BOOL enterF));
void mbOpeningViewSet(HuVecF *rot, HuVecF *pos, float zoom);
void mbPlayerTurnCloseHookSet(void (*hook)(int playerNo));
void mbPlayerTurnInitHookSet(void (*hook)(int playerNo));
void mbScrollInit(int dataNum);
float mbSinDeg(float angle);
int mbAudFXPlay(int seId);
void mbAudFXStop(int handle);
void mbCameraPlayerViewSetFast(int playerNo, int viewNo);
void mbCameraShakeSet(int maxTime, float power);
void mbCameraFocusObjSet(int modelId);
void mbCameraMovePlayer(s16 playerNo, HuVecF *rot, HuVecF *offset,
    float zoom, float fov, s16 maxTime);
void mbCameraMovePos(HuVecF *pos, HuVecF *rot, HuVecF *offset,
    float zoom, float fov, s16 maxTime);
void mbMoveNumDispSet(int playerNo, BOOL display);
void mbPauseDisableSet(BOOL disable);
void mbPlayerColSnapPlayerSet(int playerNo, BOOL snapF);
int mbPlayerMotionCreate(int playerNo, int dataNum);
int mbPlayerMotionKill(int playerNo, int motNo);
void mbPlayerMotionShiftSet(
    int playerNo, int motNo, float start, float end, u32 attr);
void mbPlayerPosGet(int playerNo, HuVecF *pos);
void mbPlayerPosSetV(int playerNo, const HuVecF *pos);
void mbPlayerRotYSet(int playerNo, float rotY);
void mbPlayerRotateStart(int playerNo, s16 endAngle, s16 maxTime);
void mbSingleReturnWrite(void);
void mbWipeCreate(s16 mode, s16 type, s16 time);
void mbWipeDissolveFadeOut(void);
void mbWipeFadeOut(void);
void mbWipeFadeOutTime(int time);
void mbWipeSpecialCreate(int state, int type, int time);
void mbWipeSpecialKill(void);
void mbWipeSpecialWait(void);

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

void _epilog(void)
{
    const VoidFunc *dtors = _dtors;

    while (*dtors != 0) {
        (**dtors)();
        dtors++;
    }
}

void fn_1_A0(void)
{
    GwSystem.partyF = FALSE;
    mbObjectSetup(6, fn_1_F4, fn_1_270);
}

void fn_1_F4(void)
{
    s16 *modelId = &lbl_1_bss_3C;
    s32 boardNo;

    boardNo = MBBoardNoGet();

    HuAudSndGrpSetSet(0x1D);
    lbl_1_bss_40 = GwSystem.boardWork;
    mbMasuInit(0xC70000);
    *modelId = mbObjCreate(0xC70002, NULL, FALSE);
    mbObjAttrSet(*modelId, 0x40000001);
    mbScrollInit(0xC70001);
    mbLightFuncSet(fn_1_404, fn_1_468);
    mbPlayerTurnInitHookSet(fn_1_3E0);
    mbPlayerTurnCloseHookSet(fn_1_3E4);
    mbOpeningViewSet(&lbl_1_data_C, &lbl_1_data_18, lbl_1_rodata_78);
    mbev_MasuMoveEndSet(fn_1_314);
    mbev_MasuMoveStartSet(fn_1_2B8);
    mbev_MasuHatenaSet(fn_1_374);
    mbMapCameraSet(NULL, &lbl_1_data_0, lbl_1_rodata_7C);
    mbMapHookSet(fn_1_46C);
    omAddObjEx(mbObjMan, 0x200C, 0, 0, -1, fn_1_274);
    fn_1_4C8();
    fn_1_8F0();
    fn_1_AFC();
    fn_1_C38();
    HuDataDirClose(0xC70000);
}

void fn_1_270(OMOBJ *obj)
{
}

void fn_1_274(OMOBJ *obj)
{
    if (mbExitCheck()) {
        omDelObjEx((OMOBJMAN *)HuPrcCurrentGet(), obj);
    } else {
        fn_1_A50();
        fn_1_CEC();
    }
}

int fn_1_2B8(int playerNo, s16 id)
{
    u32 attr = mbMasuMAttrGet(id);

    if (attr & 0x80) {
        fn_1_DAC(playerNo, id);
    }
    return 0;
}

int fn_1_314(int playerNo, s16 id)
{
    u32 attr = mbMasuMAttrGet(id);

    if (attr & 0x70) {
        fn_1_9B8(playerNo, mbev_MasuBitGet(attr, 0x70));
    }
    return 0;
}

int fn_1_374(int playerNo, s16 id)
{
    u32 attr = mbMasuMAttrGet(id);

    if (attr & 0x3) {
        fn_1_590(playerNo, mbev_MasuBitGet(attr, 0x3) - 1);
        return 0;
    }
    return 0;
}

void fn_1_3E0(int playerNo)
{
}

void fn_1_3E4(int playerNo)
{
    fn_1_87C();
}

void fn_1_404(void)
{
    s16 *modelId = &lbl_1_bss_3C;

    Hu3DModelLightInfoSet(mbObjModelIDGet(modelId[0]), TRUE);
    Hu3DFogSet(lbl_1_rodata_80, lbl_1_rodata_84, 200, 200, 100);
}

void fn_1_468(void)
{
}

void fn_1_46C(BOOL enterF)
{
    if (enterF) {
        Hu3DFogClear();
    } else {
        Hu3DFogSet(lbl_1_rodata_80, lbl_1_rodata_84, 200, 200, 100);
    }
}

void fn_1_4C8(void)
{
    S01Marker *marker = lbl_1_bss_30;
    u32 attr;
    s32 i;

    for (i = 0; i < 3; i++, marker++) {
        marker->modelId = mbObjCreate(lbl_1_data_24[i], NULL, FALSE);
        mbObjPosSetV(marker->modelId, &lbl_1_data_30[i]);
        mbObjMotionSpeedSet(marker->modelId, 0.0f);
        attr = mbev_MasuAttrGet(i + 1, 0xC);
        marker->masuId = mbMasuFind_MAttrMatchIdGet(-1, attr, 0xC);
    }
}

#include "src/REL/s01Dll/s01_common.inc"

void fn_1_590(int playerNo, int eventNo)
{
    S01Marker *marker = &lbl_1_bss_30[eventNo];
    S01PlayerMoveWork move;
    int sound;
    s32 time;
    float t;

    move.motionId = mbPlayerMotionCreate(playerNo, 0x930022);
    mbPlayerRotateStart(playerNo, 0, 15);
    mbCameraPlayerViewSetFast(playerNo, 2);
    mbCameraShakeSet(48, lbl_1_rodata_88);
    sound = mbAudFXPlay(0x60D);
    HuPrcSleep(48);
    mbPlayerMotionShiftSet(playerNo, 9, 0.0f,
        lbl_1_rodata_8C, 0);
    mbMasuPosGet(GwPlayer[playerNo].masuId, &move.startPos);
    mbMasuPosGet(marker->masuId, &move.targetPos);
    GwPlayer[playerNo].masuIdNext = marker->masuId;
    mbMasuNextSet(marker->masuId);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);

    for (time = 0; (u32)time <= 42; time++) {
        t = time / lbl_1_rodata_90;
        if ((u32)time == 0) {
            mbObjMotionSpeedSet(marker->modelId, 1.0f);
            mbAudFXPlay(0x60E);
            mbAudFXStop(sound);
        } else if ((u32)time == 24) {
            mbPlayerMotionShiftSet(playerNo, move.motionId,
                0.0f, 8.0f, 0);
        }
        move.pos.x = move.startPos.x
            + t * (move.targetPos.x - move.startPos.x);
        move.pos.y = move.startPos.y
            + t * (move.targetPos.y - move.startPos.y)
            + lbl_1_rodata_94
                * (lbl_1_rodata_98 * mbSinDeg(180.0f * t));
        move.pos.z = move.startPos.z
            + t * (move.targetPos.z - move.startPos.z);
        mbPlayerPosSetV(playerNo, &move.pos);
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 6, 0.0f,
        lbl_1_rodata_8C, 0x40000001);
    HuPrcSleep(60);
    mbWipeFadeOut();
    mbPlayerColSnapPlayerSet(playerNo, TRUE);
    mbPlayerMotionKill(playerNo, move.motionId);
    GwPlayer[playerNo].masuId = marker->masuId;
}

void fn_1_87C(void)
{
    S01Marker *marker = lbl_1_bss_30;
    s32 i;

    for (i = 0; i < 3; i++, marker++) {
        mbObjMotionSpeedSet(marker->modelId, 0.0f);
        mbObjMotionTimeSet(marker->modelId, 0.0f);
    }
}

void fn_1_8F0(void)
{
    S01Npc *npc = lbl_1_bss_8;
    s32 i;

    for (i = 0; i < 5; i++, npc++) {
        npc->modelId = mbObjCreate(0xC70006, lbl_1_data_A4, FALSE);
        mbObjPosSetV(npc->modelId, &lbl_1_data_54[i].pos);
        mbObjRotYSet(npc->modelId, lbl_1_data_54[i].rotY);
        mbObjMotionSet(npc->modelId, 1, 0x40000001);
        npc->eventNo = lbl_1_data_B0[i];
    }
}

void fn_1_9B8(int playerNo, int eventNo)
{
    S01Npc *npc = lbl_1_bss_8;
    s32 i;

    for (i = 0; i < 5; i++, npc++) {
        if (eventNo == npc->eventNo) {
            mbObjMotionShiftSet(npc->modelId, 2, 0.0f,
                lbl_1_rodata_8C, 0);
            mbObjMotionShapeSet(npc->modelId, 2, 0);
        }
    }
}

void fn_1_A50(void)
{
    S01Npc *npc = lbl_1_bss_8;
    s32 i;

    for (i = 0; i < 5; i++, npc++) {
        if (mbObjMotionGet(npc->modelId) != 1
            && mbObjMotionEndCheck(npc->modelId)) {
            mbObjMotionShiftSet(npc->modelId, 1, 0.0f,
                lbl_1_rodata_8C, 0x40000001);
            mbObjMotionShapeSet(npc->modelId, 1, 0x40000040);
        }
    }
}

void fn_1_AFC(void)
{
    s16 *modelId = lbl_1_bss_2;
    s32 motion[16];
    s32 motionCount = 5;
    s32 randomA;
    s32 randomB;
    s32 temp;
    s32 i;

    for (i = 0; i < motionCount; i++) {
        motion[i] = i + 1;
    }
    for (i = 0; i < 100; i++) {
        randomA = mbRandMod(motionCount);
        randomB = mbRandMod(motionCount);
        temp = motion[randomA];
        motion[randomA] = motion[randomB];
        motion[randomB] = temp;
    }
    for (i = 0; i < 3; i++, modelId++) {
        *modelId = mbObjCreate(0xC70009, lbl_1_data_F4, FALSE);
        mbObjPosSetV(*modelId, &lbl_1_data_C4[i].pos);
        mbObjRotYSet(*modelId, lbl_1_data_C4[i].rotY);
        mbObjMotionSet(*modelId, motion[i], 0x40000001);
    }
}

void fn_1_C38(void)
{
    s16 *modelId = &lbl_1_bss_0;
    HuVecF pos;

    modelId[0] = mbObjCreate(0xC70010, lbl_1_data_10C, TRUE);
    mbObjMotionSet(modelId[0], 1, 0x40000001);
    mbObjScaleSet(modelId[0], 0.5f, 0.5f, 0.5f);
    Hu3DModelObjPosGet(mbObjModelIDGet(lbl_1_bss_3C), lbl_1_data_114, &pos);
    mbObjPosSetV(modelId[0], &pos);
}

void fn_1_CEC(void)
{
    s16 *modelId = &lbl_1_bss_0;
    HuVecF objectPos;
    HuVecF targetPos;
    HuVecF delta;
    float angle;

    mbObjPosGet(modelId[0], &objectPos);
    Hu3DModelObjPosGet(mbObjModelIDGet(lbl_1_bss_3C), lbl_1_data_114,
        &targetPos);
    PSVECSubtract(&targetPos, &objectPos, &delta);
    angle = (float)(180.0
        * (atan2(delta.x, delta.z) / 3.141592653589793));
    mbObjRotYSet(modelId[0], angle);
    mbObjPosSetV(modelId[0], &targetPos);
}

void fn_1_DAC(int playerNo, s16 id)
{
    S01Marker *marker = &lbl_1_bss_30[2];
    S01MotionPair motions;
    S01WarpWork work;
    s16 currentMasu;
    int sound;
    s32 time;
    float t;
    float angle;

    mbPauseDisableSet(TRUE);
    motions.first = mbPlayerMotionCreate(playerNo, 0x930017);
    motions.second = mbPlayerMotionCreate(playerNo, 0x930022);
    mbMoveNumDispSet(playerNo, FALSE);
    mbCameraMovePlayer((s16)playerNo, &lbl_1_data_11C, NULL,
        lbl_1_rodata_9C, lbl_1_rodata_A0, 120);
    currentMasu = GwPlayer[playerNo].masuId;
    mbMasuPosGet(currentMasu, &work.playerPos);
    mbMasuPosGet(id, &work.targetPos);
    PSVECSubtract(&work.targetPos, &work.playerPos, &work.delta);
    angle = (float)(180.0
        * (atan2(work.delta.x, work.delta.z) / 3.141592653589793));
    mbPlayerRotYSet(playerNo, angle);
    mbPlayerMotionShiftSet(playerNo, 2, 0.0f,
        lbl_1_rodata_8C, 0x40000001);

    for (time = 0; (u32)time <= 120; time++) {
        t = time / lbl_1_rodata_A4;
        work.pos.x = work.playerPos.x + t * work.delta.x;
        work.pos.y = work.playerPos.y + t * work.delta.y;
        work.pos.z = work.playerPos.z + t * work.delta.z;
        mbPlayerPosSetV(playerNo, &work.pos);
        if (time % 30 == 0) {
            omVibrate((s16)playerNo, 20, 7, 3);
        }
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, motions.first, 0.0f,
        lbl_1_rodata_8C, 0x40000001);
    HuPrcSleep(30);
    mbCameraShakeSet(60, lbl_1_rodata_88);
    sound = mbAudFXPlay(0x60D);
    HuPrcSleep(60);
    mbObjMotionSpeedSet(marker->modelId, 1.0f);
    omVibrate((s16)playerNo, 20, 20, 0);
    mbAudFXPlay(0x60E);
    mbAudFXStop(sound);
    HuPrcSleep(12);
    mbPlayerMotionShiftSet(playerNo, 9, 0.0f,
        lbl_1_rodata_8C, 0);
    mbCameraFocusObjSet(-1);
    work.delta.x = work.delta.y = work.delta.z = 0.0f;
    mbPlayerPosGet(playerNo, &work.jumpPos);

    for (time = 0; (u32)time < 72; time++) {
        work.delta.y += lbl_1_rodata_A8;
        work.jumpPos.y += lbl_1_rodata_AC * work.delta.y;
        mbPlayerPosSetV(playerNo, &work.jumpPos);
        if ((u32)time == 18) {
            mbPlayerMotionShiftSet(playerNo, motions.second,
                0.0f, 8.0f, 0x40000001);
        }
        HuPrcVSleep();
    }
    mbWipeDissolveFadeOut();
    mbCameraMovePos(NULL, &lbl_1_data_128, NULL, lbl_1_rodata_88,
        lbl_1_rodata_A0, -1);
    mbObjDispSet(marker->modelId, FALSE);
    work.jumpPos.y = work.playerPos.y;
    work.delta.y = lbl_1_rodata_B0;
    mbPlayerPosSetV(playerNo, &work.jumpPos);
    mbWipeCreate(1, 0x81, 30);
    mbMusFadeOutSpeed(0, 1000);
    mbWipeSpecialCreate(1, 6, 90);
    mbAudFXPlay(0x60F);

    for (time = 0; (u32)time <= 90; time++) {
        t = time / lbl_1_rodata_B4;
        work.delta.y += lbl_1_rodata_A8;
        work.jumpPos.y += lbl_1_rodata_AC * work.delta.y;
        mbPlayerPosSetV(playerNo, &work.jumpPos);
        HuPrcVSleep();
    }
    mbWipeSpecialWait();
    mbWipeFadeOutTime(1);
    mbWipeSpecialKill();
    mbPlayerMotionKill(playerNo, motions.first);
    mbPlayerMotionKill(playerNo, motions.second);
    mbSingleReturnWrite();
}

#pragma push
#pragma section code_type ".text.after_common"

void fn_1_3610(void)
{
    lbl_1_bss_44 = OSGetTick();
}

s32 fn_1_363C(void)
{
    return OSGetTick() - lbl_1_bss_44;
}

#pragma pop
