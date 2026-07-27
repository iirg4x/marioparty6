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

#include "dolphin.h"
#include "humath.h"
#include "math.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/process.h"
#include "game/board/player.h"
#include "string.h"

typedef float (*S01CurveEval)();

extern float *lbl_1_bss_48;

float mbBezierCalcSlope(float a, float b, float c, float t);
float mbHermiteCalcSlope(float a, float b, float c, float d, float t);
float mbAngleLerp(float a, float b, float t);

#pragma push
#pragma section code_type ".text.common"

float fn_1_1EF8(float a, float b, float c, float d, float t);

float fn_1_12CC(S01CurveEval eval, HuVecF *a, HuVecF *b, HuVecF *c,
    HuVecF *d, float t)
{
    int i;
    int div;
    float sampleLength;
    float baseT;
    float sampleT;
    float deltaT;

    div = 10;
    baseT = 0.0f;
    deltaT = (t - baseT) / div;
    sampleT = baseT;
    sampleLength = 0.0f;
    for (i = 0; i < div - 1; i++) {
        sampleT += deltaT;
        sampleLength += eval(a, b, c, d, sampleT);
    }
    sampleLength = deltaT * 0.5
        * (eval(a, b, c, d, baseT) + eval(a, b, c, d, t)
            + (2.0 * sampleLength));
    return sampleLength;
}

float fn_1_1474(S01CurveEval eval, HuVecF *a, HuVecF *b, HuVecF *c,
    HuVecF *d, float t)
{
    int div = 10;
    float baseT = 0.0f;
    float pathLength = 0.0f;
    float deltaT = t - baseT;
    float edgeLength = deltaT
        * (eval(a, b, c, d, baseT) + eval(a, b, c, d, t))
        * 0.5f;
    float sampleLength;
    int j;
    int i;

    for (i = 1; i <= div; i *= 2) {
        sampleLength = 0.0f;
        for (j = 1; j <= i; j++) {
            sampleLength += eval(a, b, c, d,
                baseT + deltaT * (j - 0.5f));
        }
        sampleLength *= deltaT;
        pathLength = (1.0f / 3.0f)
            * (edgeLength + (2.0f * sampleLength));
        deltaT *= 0.5f;
        edgeLength = (edgeLength + sampleLength) * 0.5f;
    }
    return pathLength;
}

float fn_1_1684(S01CurveEval eval, HuVecF *a, HuVecF *b, HuVecF *c,
    HuVecF *d, float t, float distance, int maxStep)
{
    int step;
    float sampleLength;
    float pathLength;
    float oldT;
    float minLength;

    minLength = 0.1f;
    step = 0;
    do {
        pathLength = fn_1_1474(eval, a, b, c, d, t) - distance;
        if (fabs(sampleLength = eval(a, b, c, d, t)) < minLength) {
            sampleLength = 1.0f;
        }
        oldT = t;
        t -= pathLength / sampleLength;
        step++;
    } while (t != oldT && step < maxStep);
    return t;
}

float fn_1_19A0(S01CurveEval eval, HuVecF *a, HuVecF *b, HuVecF *c,
    HuVecF *d, float t, float distance, int maxStep)
{
    int step;
    float sampleLength;
    float pathLength;
    float oldT;
    float minLength;

    minLength = 0.1f;
    step = 0;
    do {
        pathLength = fn_1_12CC(eval, a, b, c, d, t) - distance;
        if (fabs(sampleLength = eval(a, b, c, d, t)) < minLength) {
            sampleLength = 1.0f;
        }
        oldT = t;
        t -= pathLength / sampleLength;
        step++;
    } while (t != oldT && step < maxStep);
    return t;
}

float fn_1_1C54(HuVecF *a, HuVecF *b, HuVecF *c, float t)
{
    HuVecF slope;
    slope.x = mbBezierCalcSlope(a->x, b->x, c->x, t);
    slope.y = mbBezierCalcSlope(a->y, b->y, c->y, t);
    slope.z = mbBezierCalcSlope(a->z, b->z, c->z, t);
    return VECMag(&slope);
}

float fn_1_1CF8(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, float t)
{
    HuVecF slope;
    slope.x = fn_1_1EF8(a->x, b->x, c->x, d->x, t);
    slope.y = fn_1_1EF8(a->y, b->y, c->y, d->y, t);
    slope.z = fn_1_1EF8(a->z, b->z, c->z, d->z, t);
    return VECMag(&slope);
}

float fn_1_1DB4(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, float t)
{
    HuVecF slope;
    slope.x = mbHermiteCalcSlope(a->x, b->x, c->x, d->x, t);
    slope.y = mbHermiteCalcSlope(a->y, b->y, c->y, d->y, t);
    slope.z = mbHermiteCalcSlope(a->z, b->z, c->z, d->z, t);
    return VECMag(&slope);
}

float fn_1_1E70(float a, float b, float c, float d, float t)
{
    float invT = 1.0f - t;
    return (a * (invT * invT * invT))
        + (b * (3.0f * t * invT * invT))
        + (c * (3.0f * t * t * invT))
        + (d * (t * t * t));
}

float fn_1_1EF8(float a, float b, float c, float d, float t)
{
    float t2 = t * t;
    return (a * ((-3.0f * t2) - (6.0f * t) - 3.0f))
        + (b * ((9.0f * t2) - (12.0f * t) + 3.0f))
        + (c * ((-9.0f * t2) + (6.0f * t)))
        + ((3.0f * t2) * d);
}

void fn_1_1FD0(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, HuVecF *out,
    float t)
{
    out->x = fn_1_1E70(a->x, b->x, c->x, d->x, t);
    out->y = fn_1_1E70(a->y, b->y, c->y, d->y, t);
    out->z = fn_1_1E70(a->z, b->z, c->z, d->z, t);
}

void fn_1_2290(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, HuVecF *out,
    float t)
{
    out->x = fn_1_1EF8(a->x, b->x, c->x, d->x, t);
    out->y = fn_1_1EF8(a->y, b->y, c->y, d->y, t);
    out->z = fn_1_1EF8(a->z, b->z, c->z, d->z, t);
}

void fn_1_2640(HuVecF *points, int count, HuVecF *out, float t)
{
    HuVecF *allocTbl = HuMemDirectMallocNum(
        HEAP_HEAP, count * sizeof(HuVecF), HU_MEMNUM_OVL);
    HuVecF *bezierTbl = allocTbl;
    int i;
    int j;
    memcpy(bezierTbl, points, count * sizeof(HuVecF));
    for (i = 1; i < count; i++) {
        for (j = 0; j < count - i; j++) {
            bezierTbl[j].x = bezierTbl[j].x
                + (t * (bezierTbl[j + 1].x - bezierTbl[j].x));
            bezierTbl[j].y = bezierTbl[j].y
                + (t * (bezierTbl[j + 1].y - bezierTbl[j].y));
            bezierTbl[j].z = bezierTbl[j].z
                + (t * (bezierTbl[j + 1].z - bezierTbl[j].z));
        }
    }
    *out = bezierTbl[0];
    HuMemDirectFree(bezierTbl);
}

float fn_1_27B0(int index, int degree, float t)
{
    float valueA;
    float valueB;
    float divisor;
    if (degree == 0) {
        if (t >= lbl_1_bss_48[index] && t < lbl_1_bss_48[index + 1]) {
            return 1.0f;
        }
        return 0.0f;
    }
    divisor = lbl_1_bss_48[index + degree] - lbl_1_bss_48[index];
    if (divisor > 0.0f) {
        valueA = ((t - lbl_1_bss_48[index])
                     * fn_1_27B0(index, degree - 1, t)) / divisor;
    } else {
        valueA = 0.0f;
    }
    divisor = lbl_1_bss_48[index + degree + 1] - lbl_1_bss_48[index + 1];
    if (divisor > 0.0f) {
        valueB = ((lbl_1_bss_48[index + degree + 1] - t)
                     * fn_1_27B0(index + 1, degree - 1, t)) / divisor;
    } else {
        valueB = 0.0f;
    }
    return valueA + valueB;
}

void fn_1_2D24(HuVecF *points, int count, HuVecF *out, float t)
{
    int knotNo;
    int degree = 3;
    int i;
    HuVecF pos;
    float value;
    float *freeTbl;
    float *knotTbl;
    if (t < 0.0f) { *out = points[0]; return; }
    if (t >= 1.0f) { *out = points[count - 1]; return; }
    knotTbl = HuMemDirectMallocNum(HEAP_HEAP,
        (count + degree + 1) * sizeof(float), HU_MEMNUM_OVL);
    lbl_1_bss_48 = knotTbl;
    knotNo = 0;
    for (i = 0; i <= degree; i++) lbl_1_bss_48[knotNo++] = 0.0f;
    for (i = 0; i < (count - degree) - 1; i++)
        lbl_1_bss_48[knotNo++] = (float)(i + 1) / (count - degree);
    for (i = 0; i <= degree; i++) lbl_1_bss_48[knotNo++] = 1.0f;
    pos.x = pos.y = pos.z = 0.0f;
    for (i = 0; i < count; i++) {
        value = fn_1_27B0(i, degree, t);
        pos.x += value * points[i].x;
        pos.y += value * points[i].y;
        pos.z += value * points[i].z;
    }
    freeTbl = lbl_1_bss_48;
    HuMemDirectFree(freeTbl);
    *out = pos;
}

void fn_1_2FE0(HuVecF *points, int index, int count, HuVecF *a, HuVecF *b,
    HuVecF *c, HuVecF *d)
{
    HuVecF *point;
    HuVecF pointB, pointC, pointD;
    if (index > count - 1) index = count - 1;
    point = &points[index];
    if (index == count - 1) { pointB = point[0]; pointC = pointB; pointD = pointB; }
    else if (index == count - 2) { pointB = point[1]; pointC = pointB; pointD = pointB; }
    else if (index == count - 3) { pointB = point[1]; pointC = point[2]; pointD = pointC; }
    else { pointB = point[1]; pointC = point[2]; pointD = point[3]; }
    *a = point[0]; *b = pointB; *c = pointC; *d = pointD;
}

void fn_1_31BC(HuVecF *points, int index, int count, HuVecF *a, HuVecF *b,
    HuVecF *c, HuVecF *d)
{
    HuVecF *point;
    HuVecF slopeA, slopeB;
    if (index > count - 1) index = count - 1;
    point = &points[index];
    if (index == 0) VECSubtract(&point[1], &point[0], &slopeA);
    else VECSubtract(&point[1], &point[-1], &slopeA);
    if (index == count - 2) VECSubtract(&point[1], &point[0], &slopeB);
    else VECSubtract(&point[2], &point[0], &slopeB);
    VECScale(&slopeA, &slopeA, 0.5f);
    VECScale(&slopeB, &slopeB, 0.5f);
    *a = point[0]; *b = point[1]; *c = slopeA; *d = slopeB;
}

void fn_1_330C(HuVecF *a, HuVecF *b, HuVecF *out, float t)
{
    HuVecF delta;
    VECSubtract(b, a, &delta);
    VECScale(&delta, &delta, t);
    VECAdd(a, &delta, out);
}

void fn_1_3370(int playerNo, HuVecF *dstPos, float dstRotY,
    float jumpHeight, int maxTime)
{
    HuVecF srcPos, pos, delta;
    int time;
    float srcRotY, t, rotT;
    mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
    mbPlayerPosGet(playerNo, &srcPos);
    srcRotY = mbPlayerRotYGet(playerNo);
    VECSubtract(dstPos, &srcPos, &delta);
    for (time = 0; time <= maxTime; time++) {
        t = time / (float)maxTime;
        if ((u32)time == maxTime - 6)
            mbPlayerMotionShiftSet(playerNo, 5, 0.0f, 2.0f, HU3D_MOTATTR_NONE);
        pos.x = srcPos.x + (t * delta.x);
        pos.y = srcPos.y + (t * delta.y) + (jumpHeight * HuSin(180.0f * t));
        pos.z = srcPos.z + (t * delta.z);
        mbPlayerPosSetV(playerNo, &pos);
        rotT = time / 6.0f;
        if (rotT > 1.0f) rotT = 1.0f;
        mbPlayerRotYSet(playerNo, mbAngleLerp(srcRotY, dstRotY, rotT));
        mbPlayerWorkGet(playerNo)->_unk08 = maxTime - time;
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 2.0f, HU3D_MOTATTR_LOOP);
}

#pragma pop

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
