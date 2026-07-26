#include "dolphin.h"
#include "math.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/process.h"
#include "game/board/main.h"
#include "game/board/masu.h"
#include "game/board/object.h"
#include "string.h"

typedef void (*VoidFunc)(void);

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

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
extern s16 lbl_1_bss_3C;
extern s16 lbl_1_bss_0;
extern S01Npc lbl_1_bss_8[5];
extern S01Marker lbl_1_bss_30[3];
extern s32 lbl_1_data_24[3];
extern HuVecF lbl_1_data_30[3];
extern S01PosRot lbl_1_data_54[5];
extern int lbl_1_data_A4[3];
extern s32 lbl_1_data_B0[5];
extern int lbl_1_data_10C[2];
extern char lbl_1_data_114[8];
extern float lbl_1_rodata_10;
extern float lbl_1_rodata_30;
extern float lbl_1_rodata_40;
extern double lbl_1_rodata_60;
extern double lbl_1_rodata_70;
extern float lbl_1_rodata_80;
extern float lbl_1_rodata_84;
extern float lbl_1_rodata_8C;
extern s32 lbl_1_bss_44;
extern float *lbl_1_bss_48;

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
float fn_1_1C54(const HuVecF *a, const HuVecF *b, const HuVecF *c, float t);
float fn_1_1CF8(const HuVecF *a, const HuVecF *b, const HuVecF *c,
    const HuVecF *d, float t);
float fn_1_1DB4(const HuVecF *a, const HuVecF *b, const HuVecF *c,
    const HuVecF *d, float t);
float fn_1_1EF8(float a, float b, float c, float d, float t);
void fn_1_2640(HuVecF *points, int count, HuVecF *out, float t);
float fn_1_27B0(int index, int degree, float t);
void fn_1_2FE0(HuVecF *points, int index, int count, HuVecF *a, HuVecF *b,
    HuVecF *c, HuVecF *d);
void fn_1_31BC(HuVecF *points, int index, int count, HuVecF *a, HuVecF *b,
    HuVecF *c, HuVecF *d);
void fn_1_330C(HuVecF *a, HuVecF *b, HuVecF *out, float t);
void fn_1_3610(void);
s32 fn_1_363C(void);

void mbObjectSetup(s32 boardNo, void (*init)(void), void (*close)(OMOBJ *));
float mbBezierCalcSlope(float a, float b, float c, float t);
float mbHermiteCalcSlope(float a, float b, float c, float d, float t);

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
        mbObjMotionSpeedSet(marker->modelId, lbl_1_rodata_10);
        attr = mbev_MasuAttrGet(i + 1, 0xC);
        marker->masuId = mbMasuFind_MAttrMatchIdGet(-1, attr, 0xC);
    }
}

void fn_1_87C(void)
{
    S01Marker *marker = lbl_1_bss_30;
    s32 i;

    for (i = 0; i < 3; i++, marker++) {
        mbObjMotionSpeedSet(marker->modelId, lbl_1_rodata_10);
        mbObjMotionTimeSet(marker->modelId, lbl_1_rodata_10);
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
            mbObjMotionShiftSet(npc->modelId, 2, lbl_1_rodata_10,
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
            mbObjMotionShiftSet(npc->modelId, 1, lbl_1_rodata_10,
                lbl_1_rodata_8C, 0x40000001);
            mbObjMotionShapeSet(npc->modelId, 1, 0x40000040);
        }
    }
}

void fn_1_C38(void)
{
    s16 *modelId = &lbl_1_bss_0;
    HuVecF pos;

    modelId[0] = mbObjCreate(0xC70010, lbl_1_data_10C, TRUE);
    mbObjMotionSet(modelId[0], 1, 0x40000001);
    mbObjScaleSet(modelId[0], lbl_1_rodata_30, lbl_1_rodata_30,
        lbl_1_rodata_30);
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
    angle = (float)(lbl_1_rodata_70
        * (atan2(delta.x, delta.z) / lbl_1_rodata_60));
    mbObjRotYSet(modelId[0], angle);
    mbObjPosSetV(modelId[0], &targetPos);
}

float fn_1_1C54(const HuVecF *a, const HuVecF *b, const HuVecF *c, float t)
{
    HuVecF slope;

    slope.x = mbBezierCalcSlope(a->x, b->x, c->x, t);
    slope.y = mbBezierCalcSlope(a->y, b->y, c->y, t);
    slope.z = mbBezierCalcSlope(a->z, b->z, c->z, t);
    return PSVECMag(&slope);
}

float fn_1_1CF8(const HuVecF *a, const HuVecF *b, const HuVecF *c,
    const HuVecF *d, float t)
{
    HuVecF slope;

    slope.x = fn_1_1EF8(a->x, b->x, c->x, d->x, t);
    slope.y = fn_1_1EF8(a->y, b->y, c->y, d->y, t);
    slope.z = fn_1_1EF8(a->z, b->z, c->z, d->z, t);
    return PSVECMag(&slope);
}

float fn_1_1DB4(const HuVecF *a, const HuVecF *b, const HuVecF *c,
    const HuVecF *d, float t)
{
    HuVecF slope;

    slope.x = mbHermiteCalcSlope(a->x, b->x, c->x, d->x, t);
    slope.y = mbHermiteCalcSlope(a->y, b->y, c->y, d->y, t);
    slope.z = mbHermiteCalcSlope(a->z, b->z, c->z, d->z, t);
    return PSVECMag(&slope);
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
        if (t >= lbl_1_bss_48[index]
            && t < lbl_1_bss_48[index + 1]) {
            return lbl_1_rodata_40;
        }
        return lbl_1_rodata_10;
    }
    divisor = lbl_1_bss_48[index + degree] - lbl_1_bss_48[index];
    if (divisor > lbl_1_rodata_10) {
        valueA = ((t - lbl_1_bss_48[index])
                     * fn_1_27B0(index, degree - 1, t))
            / divisor;
    } else {
        valueA = lbl_1_rodata_10;
    }
    divisor = lbl_1_bss_48[index + degree + 1]
        - lbl_1_bss_48[index + 1];
    if (divisor > lbl_1_rodata_10) {
        valueB = ((lbl_1_bss_48[index + degree + 1] - t)
                     * fn_1_27B0(index + 1, degree - 1, t))
            / divisor;
    } else {
        valueB = lbl_1_rodata_10;
    }
    return valueA + valueB;
}

void fn_1_2FE0(HuVecF *points, int index, int count, HuVecF *a, HuVecF *b,
    HuVecF *c, HuVecF *d)
{
    HuVecF *point;
    HuVecF pointB;
    HuVecF pointC;
    HuVecF pointD;

    if (index > count - 1) {
        index = count - 1;
    }
    point = &points[index];
    if (index == count - 1) {
        pointB = point[0];
        pointC = pointB;
        pointD = pointB;
    } else if (index == count - 2) {
        pointB = point[1];
        pointC = pointB;
        pointD = pointB;
    } else if (index == count - 3) {
        pointB = point[1];
        pointC = point[2];
        pointD = pointC;
    } else {
        pointB = point[1];
        pointC = point[2];
        pointD = point[3];
    }
    *a = point[0];
    *b = pointB;
    *c = pointC;
    *d = pointD;
}

void fn_1_31BC(HuVecF *points, int index, int count, HuVecF *a, HuVecF *b,
    HuVecF *c, HuVecF *d)
{
    HuVecF *point;
    HuVecF slopeA;
    HuVecF slopeB;

    if (index > count - 1) {
        index = count - 1;
    }
    point = &points[index];
    if (index == 0) {
        VECSubtract(&point[1], &point[0], &slopeA);
    } else {
        VECSubtract(&point[1], &point[-1], &slopeA);
    }
    if (index == count - 2) {
        VECSubtract(&point[1], &point[0], &slopeB);
    } else {
        VECSubtract(&point[2], &point[0], &slopeB);
    }
    VECScale(&slopeA, &slopeA, lbl_1_rodata_30);
    VECScale(&slopeB, &slopeB, lbl_1_rodata_30);
    *a = point[0];
    *b = point[1];
    *c = slopeA;
    *d = slopeB;
}

void fn_1_330C(HuVecF *a, HuVecF *b, HuVecF *out, float t)
{
    HuVecF delta;

    VECSubtract(b, a, &delta);
    VECScale(&delta, &delta, t);
    VECAdd(a, &delta, out);
}

void fn_1_3610(void)
{
    lbl_1_bss_44 = OSGetTick();
}

s32 fn_1_363C(void)
{
    return OSGetTick() - lbl_1_bss_44;
}
