/* Board-side math helpers for transforms, camera projection, and motion curves. */
#define _MATH_H
#include "dolphin/math.h"
#include <stddef.h>

#include "game/board/camera.h"
#include "game/board/object.h"
#include "game/disp.h"
#include "game/frand.h"
#include "game/hu3d.h"
#include "game/memory.h"

#include "humath.h"

/* Returns the magnitude of one camera-space coordinate for frustum checks. */
#ifdef __MWERKS__
static inline float MathAbsFloat(register float value)
{
    asm {
        fabs value, value
    }
    return value;
}
#else
static inline float MathAbsFloat(float value)
{
    return (float)fabs((double)value);
}
#endif

#define MB_TRIG_TABLE_COUNT 2048
#define MB_TRIG_TABLE_BYTES (MB_TRIG_TABLE_COUNT * sizeof(float))
#define MB_TRIG_BYTE_MASK (MB_TRIG_TABLE_BYTES - sizeof(float))
#define MB_TRIG_DEG_SCALE (MB_TRIG_TABLE_BYTES / 360.0f)
#define MB_TRIG_RAD_SCALE 1303.7972412109375f
#define MB_TRIG_COS_OFFSET(angle, scale) \
    (((s32)((angle) * (scale)) + 2) & MB_TRIG_BYTE_MASK)
#define MB_TRIG_SIN_OFFSET(angle, scale) \
    (((s32)((angle) * (scale)) - 2046) & MB_TRIG_BYTE_MASK)

static float *cosTab;
static HuVecF objectBBox[8];
static HuVecF objectBBoxView[8];

/* Board setup calls this before angle helpers use the shared cosine lookup table. */
void mbMathInit(void)
{
    s32 i;

    cosTab = HuMemDirectMallocNum(HEAP_HEAP, MB_TRIG_TABLE_BYTES, HU_MEMNUM_OVL);
    for (i = 0; i < MB_TRIG_TABLE_COUNT;) {
        cosTab[i] = HuCos((360.0f / MB_TRIG_TABLE_COUNT) * i);
        i++;
    }
}

/* Board teardown calls this to release the angle lookup table created during setup. */
void mbMathClose(void)
{
    if (cosTab != NULL) {
        HuMemDirectFree(cosTab);
        cosTab = NULL;
    }
}

float mbCosDeg(float deg)
{
    return cosTab[MB_TRIG_COS_OFFSET(deg, MB_TRIG_DEG_SCALE) / sizeof(*cosTab)];
}

float mbCosRad(float rad)
{
    return cosTab[MB_TRIG_COS_OFFSET(rad, MB_TRIG_RAD_SCALE) / sizeof(*cosTab)];
}

float mbSinDeg(float deg)
{
    return cosTab[MB_TRIG_SIN_OFFSET(deg, MB_TRIG_DEG_SCALE) / sizeof(*cosTab)];
}

float mbSinRad(float rad)
{
    return cosTab[MB_TRIG_SIN_OFFSET(rad, MB_TRIG_RAD_SCALE) / sizeof(*cosTab)];
}

#ifdef __MWERKS__

/* Rotates the Y/Z rows of an existing matrix by an X-axis sine/cosine pair. */
void mbMtxRotTrigX(register Mtx mtx, register float sin, register float cos)
{
    asm {
        frsp sin, sin
        frsp cos, cos
        ps_merge00 sin, sin, sin
        ps_merge00 cos, cos, cos
        psq_l fp6, 32(mtx), 0, 0
        psq_l fp7, 40(mtx), 0, 0
        psq_l fp4, 16(mtx), 0, 0
        psq_l fp5, 24(mtx), 0, 0
        ps_mul fp8, sin, fp6
        ps_mul fp9, sin, fp7
        ps_msub fp8, cos, fp4, fp8
        ps_msub fp9, cos, fp5, fp9
        psq_st fp8, 16(mtx), 0, 0
        psq_st fp9, 24(mtx), 0, 0
        ps_mul fp8, sin, fp4
        ps_mul fp9, sin, fp5
        ps_madd fp8, cos, fp6, fp8
        ps_madd fp9, cos, fp7, fp9
        psq_st fp8, 32(mtx), 0, 0
        psq_st fp9, 40(mtx), 0, 0
    }
}

/* Rotates the X/Z rows of an existing matrix by a Y-axis sine/cosine pair. */
void mbMtxRotTrigY(register Mtx mtx, register float sin, register float cos)
{
    asm {
        frsp sin, sin
        frsp cos, cos
        ps_merge00 sin, sin, sin
        ps_merge00 cos, cos, cos
        psq_l fp4, 0(mtx), 0, 0
        psq_l fp5, 8(mtx), 0, 0
        psq_l fp6, 32(mtx), 0, 0
        psq_l fp7, 40(mtx), 0, 0
        ps_mul fp8, cos, fp4
        ps_mul fp9, cos, fp5
        ps_madd fp8, sin, fp6, fp8
        ps_madd fp9, sin, fp7, fp9
        psq_st fp8, 0(mtx), 0, 0
        psq_st fp9, 8(mtx), 0, 0
        ps_mul fp8, sin, fp4
        ps_mul fp9, sin, fp5
        ps_msub fp8, cos, fp6, fp8
        ps_msub fp9, cos, fp7, fp9
        psq_st fp8, 32(mtx), 0, 0
        psq_st fp9, 40(mtx), 0, 0
    }
}

/* Rotates the X/Y rows of an existing matrix by a Z-axis sine/cosine pair. */
void mbMtxRotTrigZ(register Mtx mtx, register float sin, register float cos)
{
    asm {
        frsp sin, sin
        frsp cos, cos
        ps_merge00 sin, sin, sin
        ps_merge00 cos, cos, cos
        psq_l fp6, 16(mtx), 0, 0
        psq_l fp7, 24(mtx), 0, 0
        psq_l fp4, 0(mtx), 0, 0
        psq_l fp5, 8(mtx), 0, 0
        ps_mul fp8, sin, fp6
        ps_mul fp9, sin, fp7
        ps_msub fp8, cos, fp4, fp8
        ps_msub fp9, cos, fp5, fp9
        psq_st fp8, 0(mtx), 0, 0
        psq_st fp9, 8(mtx), 0, 0
        ps_mul fp8, sin, fp4
        ps_mul fp9, sin, fp5
        ps_madd fp8, cos, fp6, fp8
        ps_madd fp9, cos, fp7, fp9
        psq_st fp8, 16(mtx), 0, 0
        psq_st fp9, 24(mtx), 0, 0
    }
}

#else
/* Applies an X-axis rotation to each column of the existing matrix. */
void mbMtxRotTrigX(Mtx mtx, float sin, float cos)
{
    float y;
    float z;
    s32 i;

    for (i = 0; i < 4; i++) {
        y = mtx[1][i];
        z = mtx[2][i];
        mtx[1][i] = (cos * y) - (sin * z);
        mtx[2][i] = (sin * y) + (cos * z);
    }
}

/* Applies a Y-axis rotation to each column of the existing matrix. */
void mbMtxRotTrigY(Mtx mtx, float sin, float cos)
{
    float x;
    float z;
    s32 i;

    for (i = 0; i < 4; i++) {
        x = mtx[0][i];
        z = mtx[2][i];
        mtx[0][i] = (cos * x) + (sin * z);
        mtx[2][i] = (-sin * x) + (cos * z);
    }
}

/* Applies a Z-axis rotation to each column of the existing matrix. */
void mbMtxRotTrigZ(Mtx mtx, float sin, float cos)
{
    float x;
    float y;
    s32 i;

    for (i = 0; i < 4; i++) {
        x = mtx[0][i];
        y = mtx[1][i];
        mtx[0][i] = (cos * x) - (sin * y);
        mtx[1][i] = (sin * x) + (cos * y);
    }
}

#endif

#ifdef __MWERKS__
/* Builds an X-rotated matrix whose three basis rows carry the supplied scale. */
void mbMtxRotTrigScaleX(register Mtx mtx, register float sin, register float cos,
    register HuVecF *scale)
{
    asm {
        frsp sin, sin
        frsp cos, cos
        lfs fp4, 0(scale)
        psq_l fp5, 4(scale), 0, 0
        fneg fp6, sin
        ps_sub fp7, fp5, fp5
        ps_merge00 fp8, cos, fp6
        ps_merge00 fp9, sin, cos
        ps_mul fp8, fp8, fp5
        ps_mul fp9, fp9, fp5
        stfs fp4, 0(mtx)
        psq_st fp7, 4(mtx), 0, 0
        psq_st fp7, 12(mtx), 0, 0
        psq_st fp8, 20(mtx), 0, 0
        psq_st fp7, 28(mtx), 0, 0
        psq_st fp9, 36(mtx), 0, 0
        stfs fp7, 44(mtx)
    }
}
#else
/* Builds an X-rotated matrix whose three basis rows carry the supplied scale. */
void mbMtxRotTrigScaleX(Mtx mtx, float sin, float cos, HuVecF *scale)
{
    mtx[0][0] = scale->x;
    mtx[0][1] = 0.0f;
    mtx[0][2] = 0.0f;
    mtx[0][3] = 0.0f;
    mtx[1][0] = 0.0f;
    mtx[1][1] = cos * scale->y;
    mtx[1][2] = -sin * scale->z;
    mtx[1][3] = 0.0f;
    mtx[2][0] = 0.0f;
    mtx[2][1] = sin * scale->y;
    mtx[2][2] = cos * scale->z;
    mtx[2][3] = 0.0f;
}

#endif

#ifdef __MWERKS__
/* Builds a Y-rotated matrix whose three basis rows carry the supplied scale. */
void mbMtxRotTrigScaleY(register Mtx mtx, register float sin, register float cos,
    register HuVecF *scale)
{
    asm {
        frsp sin, sin
        frsp cos, cos
        psq_l fp4, 0(scale), 0, 0
        lfs fp6, 8(scale)
        ps_sub fp7, fp4, fp4
        fmuls fp8, cos, fp4
        fmuls fp9, sin, fp6
        ps_merge11 fp5, fp7, fp4
        ps_merge00 fp8, fp8, fp7
        ps_merge00 fp9, fp9, fp7
        psq_st fp5, 16(mtx), 0, 0
        psq_st fp7, 24(mtx), 0, 0
        fneg fp5, sin
        psq_st fp8, 0(mtx), 0, 0
        psq_st fp9, 8(mtx), 0, 0
        fmuls fp8, fp5, fp4
        fmuls fp9, cos, fp6
        ps_merge00 fp8, fp8, fp7
        ps_merge00 fp9, fp9, fp7
        psq_st fp8, 32(mtx), 0, 0
        psq_st fp9, 40(mtx), 0, 0
    }
}
#else
/* Builds a Y-rotated matrix whose three basis rows carry the supplied scale. */
void mbMtxRotTrigScaleY(Mtx mtx, float sin, float cos, HuVecF *scale)
{
    mtx[0][0] = cos * scale->x;
    mtx[0][1] = 0.0f;
    mtx[0][2] = sin * scale->z;
    mtx[0][3] = 0.0f;
    mtx[1][0] = 0.0f;
    mtx[1][1] = scale->y;
    mtx[1][2] = 0.0f;
    mtx[1][3] = 0.0f;
    mtx[2][0] = -sin * scale->x;
    mtx[2][1] = 0.0f;
    mtx[2][2] = cos * scale->z;
    mtx[2][3] = 0.0f;
}
#endif

#ifdef __MWERKS__
/* Builds a Z-rotated matrix whose three basis rows carry the supplied scale. */
void mbMtxRotTrigScaleZ(register Mtx mtx, register float sin, register float cos,
    register HuVecF *scale)
{
    asm {
        frsp sin, sin
        frsp cos, cos
        psq_l fp4, 0(scale), 0, 0
        lfs fp5, 8(scale)
        fneg fp6, sin
        ps_sub fp7, fp4, fp4
        ps_merge00 fp8, cos, fp6
        ps_merge00 fp9, sin, cos
        ps_merge00 fp5, fp5, fp7
        ps_mul fp8, fp8, fp4
        ps_mul fp9, fp9, fp4
        psq_st fp8, 0(mtx), 0, 0
        psq_st fp7, 8(mtx), 0, 0
        psq_st fp9, 16(mtx), 0, 0
        psq_st fp7, 24(mtx), 0, 0
        psq_st fp7, 32(mtx), 0, 0
        psq_st fp5, 40(mtx), 0, 0
    }
}
#else
/* Builds a Z-rotated matrix whose three basis rows carry the supplied scale. */
void mbMtxRotTrigScaleZ(Mtx mtx, float sin, float cos, HuVecF *scale)
{
    mtx[0][0] = cos * scale->x;
    mtx[0][1] = -sin * scale->y;
    mtx[0][2] = 0.0f;
    mtx[0][3] = 0.0f;
    mtx[1][0] = sin * scale->x;
    mtx[1][1] = cos * scale->y;
    mtx[1][2] = 0.0f;
    mtx[1][3] = 0.0f;
    mtx[2][0] = 0.0f;
    mtx[2][1] = 0.0f;
    mtx[2][2] = scale->z;
    mtx[2][3] = 0.0f;
}

#endif

/* Looks up sine and cosine in the shared table, then rotates about the selected axis in degrees. */
void mbMtxRotAxisDeg(Mtx mtx, u8 axis, float angle)
{
    float *table;
    s32 offset;

    offset = (s32)(angle * MB_TRIG_DEG_SCALE);
    table = cosTab;
    MTXRotTrig(mtx, axis,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)));
}

/* Looks up sine and cosine in the shared table, then rotates about the selected axis in radians. */
void mbMtxRotAxisRad(Mtx mtx, u8 axis, float angle)
{
    float *table;
    s32 offset;

    offset = (s32)(angle * MB_TRIG_RAD_SCALE);
    table = cosTab;
    MTXRotTrig(mtx, axis,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)));
}

/* Applies an X-axis matrix rotation using an angle measured in degrees. */
void mbMtxRotXDeg(Mtx mtx, float angle)
{
    s32 offset = (s32)(angle * MB_TRIG_DEG_SCALE);
    float *table = cosTab;

    mbMtxRotTrigX(mtx,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)));
}

/* Applies an X-axis matrix rotation using an angle measured in radians. */
void mbMtxRotXRad(Mtx mtx, float angle)
{
    s32 offset = (s32)(angle * MB_TRIG_RAD_SCALE);
    float *table = cosTab;

    mbMtxRotTrigX(mtx,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)));
}

/* Applies a Y-axis matrix rotation using an angle measured in degrees. */
void mbMtxRotYDeg(Mtx mtx, float angle)
{
    s32 offset = (s32)(angle * MB_TRIG_DEG_SCALE);
    float *table = cosTab;

    mbMtxRotTrigY(mtx,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)));
}

/* Applies a Y-axis matrix rotation using an angle measured in radians. */
void mbMtxRotYRad(Mtx mtx, float angle)
{
    s32 offset = (s32)(angle * MB_TRIG_RAD_SCALE);
    float *table = cosTab;

    mbMtxRotTrigY(mtx,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)));
}

/* Applies a Z-axis matrix rotation using an angle measured in degrees. */
void mbMtxRotZDeg(Mtx mtx, float angle)
{
    s32 offset = (s32)(angle * MB_TRIG_DEG_SCALE);
    float *table = cosTab;

    mbMtxRotTrigZ(mtx,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)));
}

/* Applies a Z-axis matrix rotation using an angle measured in radians. */
void mbMtxRotZRad(Mtx mtx, float angle)
{
    s32 offset = (s32)(angle * MB_TRIG_RAD_SCALE);
    float *table = cosTab;

    mbMtxRotTrigZ(mtx,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)));
}

/* Builds a scaled X-axis rotation matrix from a degree angle and scale vector. */
void mbMtxScaleRotXDeg(Mtx mtx, HuVecF *scale, float angle)
{
    s32 offset = (s32)(angle * MB_TRIG_DEG_SCALE);
    float *table = cosTab;

    mbMtxRotTrigScaleX(mtx,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)), scale);
}

/* Builds a scaled Y-axis rotation matrix from a degree angle and scale vector. */
void mbMtxScaleRotYDeg(Mtx mtx, float angle, HuVecF *scale)
{
    s32 offset = (s32)(angle * MB_TRIG_DEG_SCALE);
    float *table = cosTab;

    mbMtxRotTrigScaleY(mtx,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)), scale);
}

/* Builds a scaled Z-axis rotation matrix from a degree angle and scale vector. */
void mbMtxScaleRotZDeg(Mtx mtx, float angle, HuVecF *scale)
{
    s32 offset = (s32)(angle * MB_TRIG_DEG_SCALE);
    float *table = cosTab;

    mbMtxRotTrigScaleZ(mtx,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)), scale);
}

/* Starts with an X rotation (or identity) and appends the nonzero Y and Z rotations. */
void mbMtxRot(Mtx mtx, float x, float y, float z)
{
    if (x != 0.0f) {
        mbMtxRotAxisDeg(mtx, 'x', x);
    } else {
        MTXIdentity(mtx);
    }
    if (y != 0.0f) {
        mbMtxRotYDeg(mtx, y);
    }
    if (z != 0.0f) {
        mbMtxRotZDeg(mtx, z);
    }
}

#ifdef __MWERKS__
/* Adds a position offset to the translation column of an existing matrix. */
void mbMtxTransCat(register Mtx mtx, register float x, register float y, register float z)
{
    asm {
        lfs fp4, 12(mtx)
        lfs fp5, 28(mtx)
        lfs fp6, 44(mtx)
        fadd fp4, fp4, x
        fadd fp5, fp5, y
        fadd fp6, fp6, z
        stfs fp4, 12(mtx)
        stfs fp5, 28(mtx)
        stfs fp6, 44(mtx)
    }
}
#else
/* Adds a position offset to the translation column of an existing matrix. */
void mbMtxTransCat(Mtx mtx, float x, float y, float z)
{
    double tx = mtx[0][3];
    double ty = mtx[1][3];
    double tz = mtx[2][3];

    tx += x;
    ty += y;
    tz += z;
    mtx[0][3] = (float)tx;
    mtx[1][3] = (float)ty;
    mtx[2][3] = (float)tz;
}
#endif

#define MB_RAND_HIGH_BIT (1U << 31)
#define MB_RAND_VALUE_MASK (MB_RAND_HIGH_BIT - 1U)

/* Maps a frand() value into the half-open integer range [0, mod). */
u32 mbRandMod(u32 mod)
{
    u32 value = frand();

    value &= MB_RAND_VALUE_MASK;
    if (value % 2 != 0) {
        value |= MB_RAND_HIGH_BIT;
    }
    return ((u64)value * mod) >> 32;
}

/* Board movement uses this to measure horizontal separation, ignoring height. */
float mbVecMagXZ(HuVecF *positionA, HuVecF *positionB)
{
    float dx = positionA->x - positionB->x;
    float dz = positionA->z - positionB->z;

    return HuMagPoint2D(dx, dz);
}

/* Board checks use this for an inclusive horizontal-distance threshold. */
BOOL mbVecMagXZCheck(HuVecF *positionA, HuVecF *positionB, float maxDist)
{
    float distance = mbVecMagXZ(positionA, positionB);

    if (distance <= maxDist) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/* Keeps a board rotation angle in the signed range [-180, 180] degrees. */
float mbAngleWrap(float angle)
{
    angle = fmod(angle, 360);
    if (angle < -180.0f) {
        angle += 360.0f;
    } else if (angle > 180.0f) {
        angle -= 360.0f;
    }
    return angle;
}

/* Normalizes all three rotation components before board objects use them. */
void mbAngleWrapV(HuVecF *rotation)
{
    int i;
    float *component = (float *)rotation;

    for (i = 0; i < 3; i++) {
        *component = mbAngleWrap(*component);
        component++;
    }
}

/* Advances a board rotation by a fixed degree step toward its target each update. */
BOOL mbAngleAdd(float *rotation, float targetAngle, float speed)
{
    float wrappedDelta = fmod(targetAngle - *rotation, 360);
    float step;

    if (fabs(wrappedDelta) < speed) {
        *rotation = targetAngle;
        return TRUE;
    }
    if (wrappedDelta < 0.0f) {
        wrappedDelta += 360.0f;
    }
    if (wrappedDelta > 180.0f) {
        step = -speed;
    } else {
        step = speed;
    }
    *rotation += step;
    *rotation = mbAngleWrap(*rotation);
    return FALSE;
}

/* Gate and board motion use this to approach a target by a fraction per update. */
BOOL mbAngleMoveTo(float *rotation, float targetAngle, float speed)
{
    float wrappedDelta = fmod(targetAngle - *rotation, 360);
    float threshold = 1.0f;

    if (fabs(wrappedDelta) < threshold) {
        *rotation = targetAngle;
        return TRUE;
    }
    if (wrappedDelta < 0.0f) {
        wrappedDelta += 360.0f;
    }
    if (wrappedDelta > 180.0f) {
        wrappedDelta -= 360.0f;
    }
    *rotation = fmod(*rotation + (speed * wrappedDelta), 360.0);
    if (*rotation < 0.0f) {
        *rotation += 360.0f;
    }
    return FALSE;
}

/* Returns the signed shortest angular difference from startAngle to angle. */
float mbAngleWrap2(float angle, float startAngle)
{
    float wrappedDelta = fmod(angle - startAngle, 360);

    if (wrappedDelta < 0.0f) {
        wrappedDelta += 360.0f;
    }
    if (wrappedDelta >= 180.0f) {
        wrappedDelta -= 360.0f;
    }
    return wrappedDelta;
}

/* Board collision checks use a strict 3D radius test (the boundary is outside). */
BOOL mbVecMagCheck(HuVecF *positionA, HuVecF *positionB, float radius)
{
    HuVecF diff;

    VECSubtract(positionA, positionB, &diff);
    if (VECSquareMag(&diff) >= radius * radius) {
        return FALSE;
    } else {
        return TRUE;
    }
}

/* Builds the orientation rows used by board camera-facing transforms. */
void mbMtxLookAtCalc(Mtx dest, HuVecF *eye, HuVecF *up, HuVecF *target)
{
    HuVecF f;
    HuVecF u;
    HuVecF s;

    f.x = eye->x - target->x;
    f.y = eye->y - target->y;
    f.z = eye->z - target->z;
    VECNormalize(&f, &f);
    VECCrossProduct(up, &f, &u);
    VECNormalize(&u, &u);
    VECCrossProduct(&f, &u, &s);
    dest[0][0] = u.x;
    dest[0][1] = u.y;
    dest[0][2] = u.z;
    dest[0][3] = 0.0f;
    dest[1][0] = s.x;
    dest[1][1] = s.y;
    dest[1][2] = s.z;
    dest[1][3] = 0.0f;
    dest[2][0] = f.x;
    dest[2][1] = f.y;
    dest[2][2] = f.z;
    dest[2][3] = 0.0f;
}

/* Board sprites use this to project a world position into display pixels. */
void mbPos3Dto2D(HuVecF *worldPos, HuVecF *screenPos)
{
    MBCAMERA *cameraP = mbCameraGet();
    float tanFov;
    float width;
    float height;
    Mtx lookAt;
    HuVecF pos;

    MTXLookAt(lookAt, &cameraP->eye, &cameraP->up, &cameraP->center);
    MTXMultVec(lookAt, worldPos, &pos);
    tanFov = mbSinDeg(cameraP->fov * 0.5f) / mbCosDeg(cameraP->fov * 0.5f);
    width = HU_DISP_ASPECT * (tanFov * pos.z);
    height = tanFov * pos.z;
    screenPos->x = HU_DISP_CENTERX + (pos.x * (HU_DISP_CENTERX / -width));
    screenPos->y = HU_DISP_CENTERY + (pos.y * (HU_DISP_CENTERY / height));
    screenPos->z = -pos.z;
}

/* Board effects use this to project world positions into a selected camera's normalized view. */
void mbPos3DtoNorm(HuVecF *worldPos, s16 cameraMask, HuVecF *normPos)
{
    HU3D_CAMERA *cameraP;
    float tanFov;
    float height;
    float width;
    Mtx lookAt;
    HuVecF pos;
    s32 cameraNo;

    for (cameraNo = 0; cameraNo < HU3D_CAM_MAX; cameraNo++) {
        if (cameraMask & (1 << cameraNo)) {
            break;
        }
    }
    cameraP = &Hu3DCamera[cameraNo];
    MTXLookAt(lookAt, &cameraP->pos, &cameraP->up, &cameraP->target);
    MTXMultVec(lookAt, worldPos, &pos);
    tanFov = mbSinDeg(cameraP->fov * 0.5f) / mbCosDeg(cameraP->fov * 0.5f);
    height = tanFov * -pos.z;
    width = HU_DISP_ASPECT * height;
    normPos->x = pos.x / width;
    normPos->y = pos.y / height;
    normPos->z = pos.z;
}

/* Converts display-pixel coordinates and camera depth back to a world position. */
void mbPos2Dto3D(HuVecF *screenPos, HuVecF *worldPos)
{
    MBCAMERA *cameraP = mbCameraGet();
    float tanFov = mbSinDeg(cameraP->fov * 0.5f) / mbCosDeg(cameraP->fov * 0.5f);
    float height = 2.0f * (tanFov * screenPos->z);
    float width = HU_DISP_ASPECT * height;
    float normX = screenPos->x / HU_DISP_WIDTH;
    float normY = screenPos->y / HU_DISP_HEIGHT;
    Mtx lookAt;

    worldPos->x = (normX - 0.5) * width;
    worldPos->y = -(normY - 0.5) * height;
    worldPos->z = -screenPos->z;
    mbCameraLookAtInvGet(lookAt);
    MTXMultVec(lookAt, worldPos, worldPos);
}

/* Converts normalized camera-view coordinates and depth to a world position. */
void mbNormPosto3D(HuVecF *normPos, s16 cameraMask, HuVecF *worldPos)
{
    HU3D_CAMERA *cameraP;
    float depth;
    float halfFov;
    float cosine;
    float absoluteDepth;
    float fovTan;
    Mtx lookAt;
    Mtx lookAtInv;
    s32 cameraNo;

    for (cameraNo = 0; cameraNo < HU3D_CAM_MAX; cameraNo++) {
        if (cameraMask & (1 << cameraNo)) {
            break;
        }
    }
    cameraP = &Hu3DCamera[cameraNo];
    halfFov = cameraP->fov * 0.5f;
    cosine = mbCosDeg(halfFov);
    absoluteDepth = MathAbsFloat(normPos->z);
    fovTan = mbSinDeg(halfFov) / cosine;
    depth = fovTan * absoluteDepth;
    worldPos->x = normPos->x * (HU_DISP_ASPECT * depth);
    worldPos->y = normPos->y * depth;
    worldPos->z = normPos->z;
    MTXLookAt(lookAt, &cameraP->pos, &cameraP->up, &cameraP->target);
    MTXInverse(lookAt, lookAtInv);
    MTXMultVec(lookAtInv, worldPos, worldPos);
}

/* Converts normalized screen coordinates to pixels while preserving depth. */
void mbNormPosto2D(HuVecF *normPos, HuVecF *screenPos)
{
    screenPos->x = HU_DISP_CENTERX * (1.0f + normPos->x);
    screenPos->y = -HU_DISP_CENTERY * (normPos->y - 1.0f);
    screenPos->z = normPos->z;
}

/* Board path animation uses a quadratic Bezier value for normalized progress t. */
float mbBezierCalc(float start, float control, float end, float t)
{
    float invTime = 1.0f - t;

    return (t * t * end) + ((invTime * invTime * start) + (control * ((2.0f * invTime) * t)));
}

/* Evaluates the same quadratic Bezier independently for each vector component. */
void mbBezierCalcV(HuVecF *start, HuVecF *control, HuVecF *end, HuVecF *dst, float t)
{
    dst->x = mbBezierCalc(start->x, control->x, end->x, t);
    dst->y = mbBezierCalc(start->y, control->y, end->y, t);
    dst->z = mbBezierCalc(start->z, control->z, end->z, t);
}

/* Evaluates a quadratic Bezier from three consecutive points in a vector list. */
void mbBezierCalcVList(HuVecF *controlPoints, HuVecF *dst, float t)
{
    dst->x = mbBezierCalc(controlPoints[0].x, controlPoints[1].x, controlPoints[2].x, t);
    dst->y = mbBezierCalc(controlPoints[0].y, controlPoints[1].y, controlPoints[2].y, t);
    dst->z = mbBezierCalc(controlPoints[0].z, controlPoints[1].z, controlPoints[2].z, t);
}

/* Returns the derivative of a quadratic Bezier curve at normalized progress t. */
float mbBezierCalcSlope(float start, float control, float end, float t)
{
    return 2.0f * ((-start + control) + (t * (end + (start - (2.0f * control)))));
}

/* Evaluates the quadratic Bezier derivative for each vector component. */
void mbBezierCalcSlopeV(HuVecF *start, HuVecF *control, HuVecF *end, HuVecF *dst, float t)
{
    dst->x = mbBezierCalcSlope(start->x, control->x, end->x, t);
    dst->y = mbBezierCalcSlope(start->y, control->y, end->y, t);
    dst->z = mbBezierCalcSlope(start->z, control->z, end->z, t);
}

/* Board animation uses cubic Hermite interpolation between endpoint values and tangents. */
float mbHermiteCalc(float start, float end, float startTangent, float endTangent, float t)
{
    float tt = t * t;
    float ttt = t * t * t;
    float aCoef = 1.0f + ((2.0f * ttt) - (3.0f * tt));
    float bCoef = (-2.0f * ttt) + (3.0f * tt);
    float cCoef = t + (ttt - (2.0f * tt));
    float dCoef = ttt - tt;

    return (aCoef * start) + (bCoef * end) + (cCoef * startTangent) + (dCoef * endTangent);
}

/* Evaluates cubic Hermite interpolation for each vector component. */
void mbHermiteCalcV(HuVecF *start, HuVecF *end, HuVecF *startTangent, HuVecF *endTangent,
    HuVecF *dst, float t)
{
    dst->x = mbHermiteCalc(start->x, end->x, startTangent->x, endTangent->x, t);
    dst->y = mbHermiteCalc(start->y, end->y, startTangent->y, endTangent->y, t);
    dst->z = mbHermiteCalc(start->z, end->z, startTangent->z, endTangent->z, t);
}

/* Returns the derivative of the cubic Hermite curve at normalized progress t. */
float mbHermiteCalcSlope(float start, float end, float startTangent, float endTangent,
    float t)
{
    float tt = t * t;
    float aCoef = (6.0f * tt) - (6.0f * t);
    float bCoef = (-6.0f * tt) + (6.0f * t);
    float cCoef = 1.0f + ((3.0f * tt) - (4.0f * t));
    float dCoef = (3.0f * tt) - (2.0f * t);

    return (aCoef * start) + (bCoef * end) + (cCoef * startTangent) + (dCoef * endTangent);
}

/* Board camera and character motion interpolate along the shortest degree arc. */
float mbAngleLerp(float startAngle, float endAngle, float t)
{
    float angleDelta = fmod(endAngle - startAngle, 360);
    float result;

    if (angleDelta < 0.0f) {
        angleDelta += 360.0f;
    }
    if (angleDelta > 180.0f) {
        angleDelta -= 360.0f;
    }
    result = fmod(startAngle + (t * angleDelta), 360);
    if (result < 0.0f) {
        result += 360.0f;
    }
    return result;
}

/* Starts angle interpolation quickly, then eases toward the end angle. */
float mbAngleEaseOut(float startAngle, float endAngle, float t)
{
    return mbAngleLerp(startAngle, endAngle, HuSin(t * 90.0f));
}

/* Starts angle interpolation slowly, then eases toward the end angle. */
float mbAngleEaseIn(float startAngle, float endAngle, float t)
{
    return mbAngleLerp(startAngle, endAngle, 1.0f - HuCos(t * 90.0f));
}

/* Moves a projected board position along camera depth by the requested scale. */
float mbMathDistScale(HuVecF *worldPos, float scale, HuVecF *scaledWorldPos)
{
    MBCAMERA *cameraP = mbCameraGet();
    HuVecF pos;
    float tanFov;
    float depth;
    float z;

    mbPos3Dto2D(worldPos, &pos);
    tanFov = HuSin(cameraP->fov * 0.5f) / HuCos(cameraP->fov * 0.5f);
    depth = pos.z * tanFov;
    z = (depth / scale) / tanFov;
    pos.z = z;
    mbPos2Dto3D(&pos, scaledWorldPos);
    /* The scaled position is written through the output pointer; the scalar return is always
     * zero. */
    return 0.0f;
}

/* Called by the object's transform hook to set its renderer-visible cull flag. */
static void ObjectCullUpdate(HSF_OBJECT *object, Mtx mtx)
{
    HU3D_CAMERA *cameraP;
    HuVecF center;
    HuVecF centerView;
    ROMtx cullMtx;
    float fov;
    float negativeFovTan;
    float nearPlaneDistance;
    float aspect;
    float frustumHalfHeight;
    float reciprocalAspect;
    s32 i;
    BOOL horizontalCullF;
    BOOL verticalCullF;

    object->flags &= ~HSF_MATERIAL_DISPOFF;
    if (shadowModelDrawF == FALSE) {
        cameraP = &Hu3DCamera[Hu3DCameraNo];
        fov = cameraP->fov;
        nearPlaneDistance = cameraP->near;
        aspect = cameraP->aspect;
    } else {
        fov = Hu3DShadow->fov;
        nearPlaneDistance = Hu3DShadow->near;
        aspect = 1.0f;
    }
    negativeFovTan = -1.0f * (mbSinDeg(fov * 0.5f) / mbCosDeg(fov * 0.5f));
    PSVECAdd(&object->mesh.mesh.min, &object->mesh.mesh.max, &center);
    PSVECScale(&center, &center, 0.5f);
    PSMTXMultVec(mtx, &center, &centerView);
    frustumHalfHeight = MathAbsFloat(centerView.z * negativeFovTan);
    if (MathAbsFloat(centerView.x) <= frustumHalfHeight * aspect
        && MathAbsFloat(centerView.y) <= frustumHalfHeight
        && centerView.y < -nearPlaneDistance) {
        /* A center inside the view bounds is sufficient to keep this object enabled. */
        return;
    }

    /* Transform all eight local bounding-box corners into the camera's cull coordinates. */
    PSMTXReorder(mtx, cullMtx);
    reciprocalAspect = 1.0f / aspect;
    cullMtx[0][0] *= reciprocalAspect;
    cullMtx[1][0] *= reciprocalAspect;
    cullMtx[2][0] *= reciprocalAspect;
    cullMtx[3][0] *= reciprocalAspect;
    cullMtx[0][2] *= negativeFovTan;
    cullMtx[1][2] *= negativeFovTan;
    cullMtx[2][2] *= negativeFovTan;
    cullMtx[3][2] *= negativeFovTan;

    objectBBox[0].x = object->mesh.mesh.max.x;
    objectBBox[0].y = object->mesh.mesh.max.y;
    objectBBox[0].z = object->mesh.mesh.max.z;
    objectBBox[1].x = object->mesh.mesh.max.x;
    objectBBox[1].y = object->mesh.mesh.max.y;
    objectBBox[1].z = object->mesh.mesh.min.z;
    objectBBox[2].x = object->mesh.mesh.max.x;
    objectBBox[2].y = object->mesh.mesh.min.y;
    objectBBox[2].z = object->mesh.mesh.max.z;
    objectBBox[3].x = object->mesh.mesh.max.x;
    objectBBox[3].y = object->mesh.mesh.min.y;
    objectBBox[3].z = object->mesh.mesh.min.z;
    objectBBox[4].x = object->mesh.mesh.min.x;
    objectBBox[4].y = object->mesh.mesh.max.y;
    objectBBox[4].z = object->mesh.mesh.max.z;
    objectBBox[5].x = object->mesh.mesh.min.x;
    objectBBox[5].y = object->mesh.mesh.max.y;
    objectBBox[5].z = object->mesh.mesh.min.z;
    objectBBox[6].x = object->mesh.mesh.min.x;
    objectBBox[6].y = object->mesh.mesh.min.y;
    objectBBox[6].z = object->mesh.mesh.max.z;
    objectBBox[7].x = object->mesh.mesh.min.x;
    objectBBox[7].y = object->mesh.mesh.min.y;
    objectBBox[7].z = object->mesh.mesh.min.z;
    PSMTXROMultVecArray(cullMtx, objectBBox, objectBBoxView, 8);

    for (i = 0; i < 8; i++) {
        if (objectBBoxView[i].z > nearPlaneDistance) {
            break;
        }
    }
    if (i >= 8) {
        /* No corner lies beyond the near plane, so the box is wholly behind it. */
        object->flags |= HSF_MATERIAL_DISPOFF;
        return;
    }

    horizontalCullF = FALSE;
    if (centerView.x >= 0.0f) {
        for (i = 0; i < 8; i++) {
            if (objectBBoxView[i].x < objectBBoxView[i].z) {
                break;
            }
        }
        if (i >= 8) {
            horizontalCullF = TRUE;
        }
    } else {
        for (i = 0; i < 8; i++) {
            if (objectBBoxView[i].x > -objectBBoxView[i].z) {
                break;
            }
        }
        if (i >= 8) {
            horizontalCullF = TRUE;
        }
    }
    if (horizontalCullF) {
        /* All corners fall outside the same horizontal side of the view. */
        object->flags |= HSF_MATERIAL_DISPOFF;
        return;
    }

    verticalCullF = FALSE;
    if (centerView.y >= 0.0f) {
        for (i = 0; i < 8; i++) {
            if (objectBBoxView[i].y < objectBBoxView[i].z) {
                break;
            }
        }
        if (i >= 8) {
            verticalCullF = TRUE;
        }
    } else {
        for (i = 0; i < 8; i++) {
            if (objectBBoxView[i].y > -objectBBoxView[i].z) {
                break;
            }
        }
        if (i >= 8) {
            verticalCullF = TRUE;
        }
    }
    if (verticalCullF) {
        /* All corners fall outside the same vertical side of the view. */
        object->flags |= HSF_MATERIAL_DISPOFF;
    }
}

#ifdef __MWERKS__
/* HSF object offsets used to read vertex data and store its local bounds. */
enum {
    BBOX_VERTEX = offsetof(HSF_OBJECT, mesh.vertex),
    BBOX_DATA = offsetof(HSF_BUFFER, data),
    BBOX_COUNT = offsetof(HSF_BUFFER, count),
    BBOX_MAX_X = offsetof(HSF_OBJECT, mesh.mesh.max.x),
    BBOX_MAX_Y = offsetof(HSF_OBJECT, mesh.mesh.max.y),
    BBOX_MAX_Z = offsetof(HSF_OBJECT, mesh.mesh.max.z),
    BBOX_MIN_X = offsetof(HSF_OBJECT, mesh.mesh.min.x),
    BBOX_MIN_Y = offsetof(HSF_OBJECT, mesh.mesh.min.y),
    BBOX_MIN_Z = offsetof(HSF_OBJECT, mesh.mesh.min.z)
};
static const float bboxMaxInitial = -1000000.0f;
static const float bboxMinInitial = 1000000.0f;

/* Computes the local-space minimum and maximum of the model's vertex positions. */
static asm void ObjectBBoxUpdate(register HSF_OBJECT *object)
{
    nofralloc
    lwz r4, BBOX_VERTEX(object)
    lwz r6, BBOX_DATA(r4)
    lwz r0, BBOX_COUNT(r4)
    lfs fp6, bboxMaxInitial
    stfs fp6, BBOX_MAX_Y(object)
    stfs fp6, BBOX_MAX_X(object)
    lfs fp6, bboxMinInitial
    stfs fp6, BBOX_MIN_Y(object)
    stfs fp6, BBOX_MIN_X(object)
    subi r6, r6, 4
    mtctr r0
    psq_l fp0, BBOX_MAX_X(object), 0, 0
    psq_l fp2, BBOX_MIN_X(object), 0, 0
    ps_mr fp1, fp0
    ps_mr fp3, fp2
vertex_loop:
    psq_l fp4, 4(r6), 0, 0
    psq_lu fp5, 12(r6), 1, 0
    ps_cmpo0 cr0, fp0, fp4
    bge max_x_unchanged
    ps_cmpo1 cr0, fp0, fp4
    bge max_x_only
    ps_mr fp0, fp4
    b max_xy_done
max_x_only:
    ps_merge01 fp0, fp4, fp0
    b max_xy_done
max_x_unchanged:
    ps_cmpo1 cr0, fp0, fp4
    bge max_xy_done
    ps_merge01 fp0, fp0, fp4
max_xy_done:
    ps_cmpo0 cr0, fp1, fp5
    bge max_z_done
    ps_mr fp1, fp5
max_z_done:
    ps_cmpo0 cr0, fp2, fp4
    ble min_x_unchanged
    ps_cmpo1 cr0, fp2, fp4
    ble min_x_only
    ps_mr fp2, fp4
    b min_xy_done
min_x_only:
    ps_merge01 fp2, fp4, fp2
    b min_xy_done
min_x_unchanged:
    ps_cmpo1 cr0, fp2, fp4
    ble min_xy_done
    ps_merge01 fp2, fp2, fp4
min_xy_done:
    ps_cmpo0 cr0, fp3, fp5
    ble min_z_done
    ps_mr fp3, fp5
min_z_done:
    bdnz vertex_loop
    psq_st fp0, BBOX_MAX_X(object), 0, 0
    psq_st fp1, BBOX_MAX_Z(object), 1, 0
    psq_st fp2, BBOX_MIN_X(object), 0, 0
    psq_st fp3, BBOX_MIN_Z(object), 1, 0
    blr
}
#else
/* Computes the local-space minimum and maximum of the model's vertex positions. */
static void ObjectBBoxUpdate(HSF_OBJECT *object)
{
    HuVecF *vertex = object->mesh.vertex->data;
    s32 count = object->mesh.vertex->count;
    s32 i;

    object->mesh.mesh.max.x = object->mesh.mesh.max.y = -1000000.0f;
    object->mesh.mesh.min.x = object->mesh.mesh.min.y = 1000000.0f;
    object->mesh.mesh.max.z = object->mesh.mesh.max.x;
    object->mesh.mesh.min.z = object->mesh.mesh.min.x;
    for (i = 0; i < count; i++, vertex++) {
        if (object->mesh.mesh.max.x < vertex->x) {
            object->mesh.mesh.max.x = vertex->x;
        }
        if (object->mesh.mesh.max.y < vertex->y) {
            object->mesh.mesh.max.y = vertex->y;
        }
        if (object->mesh.mesh.max.z < vertex->z) {
            object->mesh.mesh.max.z = vertex->z;
        }
        if (object->mesh.mesh.min.x > vertex->x) {
            object->mesh.mesh.min.x = vertex->x;
        }
        if (object->mesh.mesh.min.y > vertex->y) {
            object->mesh.mesh.min.y = vertex->y;
        }
        if (object->mesh.mesh.min.z > vertex->z) {
            object->mesh.mesh.min.z = vertex->z;
        }
    }
}
#endif

/* ObjectCullHook calls this while building the object's culling transform; it writes the
 * object's position into the translation column after scale and rotation are set. */
#ifdef __MWERKS__
static inline void MathMtxTranslationSet(register Mtx mtx, register const HuVecF *pos)
{
    asm {
        lfs fp4, 0(pos)
        lfs fp5, 4(pos)
        lfs fp6, 8(pos)
        stfs fp4, 12(mtx)
        stfs fp5, 28(mtx)
        stfs fp6, 44(mtx)
    }
}
#else
/* ObjectCullHook calls this while building the object's culling transform; it writes the
 * object's position into the translation column after scale and rotation are set. */
static inline void MathMtxTranslationSet(Mtx mtx, const HuVecF *pos)
{
    float x = pos->x;
    float y = pos->y;
    float z = pos->z;
    mtx[0][3] = x;
    mtx[1][3] = y;
    mtx[2][3] = z;
}
#endif

/* Composes the object's scale/rotation/translation with its parent before culling. */
static void ObjectCullHook(HSF_OBJECT *object, HSF_TRANSFORM *transform,
    Mtx *prevMtx, Mtx *currMtx)
{
    Mtx objectMtx;
    BOOL rotF = FALSE;

    if (transform->rot.x != 0.0f) {
        rotF = TRUE;
        mbMtxScaleRotXDeg(objectMtx, &transform->scale, transform->rot.x);
    }
    if (transform->rot.y != 0.0f) {
        s32 offset = (s32)(transform->rot.y * MB_TRIG_DEG_SCALE);
        float *table = cosTab;
        float sine = *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK));
        float cosine = *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK));

        if (rotF == FALSE) {
            rotF = TRUE;
            mbMtxRotTrigScaleY(objectMtx, sine, cosine, &transform->scale);
        } else {
            mbMtxRotTrigY(objectMtx, sine, cosine);
        }
    }
    if (transform->rot.z != 0.0f) {
        s32 offset = (s32)(transform->rot.z * MB_TRIG_DEG_SCALE);
        float *table = cosTab;
        float sine = *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK));
        float cosine = *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK));

        if (rotF == FALSE) {
            rotF = TRUE;
            mbMtxRotTrigScaleZ(objectMtx, sine, cosine, &transform->scale);
        } else {
            mbMtxRotTrigZ(objectMtx, sine, cosine);
        }
    }
    if (rotF == FALSE) {
        PSMTXScale(objectMtx, transform->scale.x, transform->scale.y,
            transform->scale.z);
    }
    MathMtxTranslationSet(objectMtx, &transform->pos);
    PSMTXConcat(*prevMtx, objectMtx, *currMtx);
    ObjectCullUpdate(object, *currMtx);
}

/* Called during board-model creation to cache bounds and install per-object cull hooks. */
void mbObjCullInit(MBMODELID modelId)
{
    HSF_DATA *hsf;
    s16 i;
    HSF_OBJECT *object;
    BOOL cullF = FALSE;

    hsf = Hu3DData[mbObjModelIDGet(modelId)].hsf;
    mbObjModelIDGet(modelId);
    object = hsf->object;
    for (i = 0; i < hsf->objectNum; i++, object++) {
        if (object->mesh.cenvNum == 0 && object->constData != NULL) {
            cullF = TRUE;
            ObjectBBoxUpdate(object);
            ((HSF_CONSTDATA *)object->constData)->hook = ObjectCullHook;
        }
    }
    if (cullF == FALSE) {
        Hu3DModelAttrSet(mbObjModelIDGet(modelId), HU3D_ATTR_NOCULL);
    }
}
