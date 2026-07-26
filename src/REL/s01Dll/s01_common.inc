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
