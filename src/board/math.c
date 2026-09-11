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

/* One-instruction native primitive, following the SDK inline-helper style.
 * No fixed-register binding; projection and culling remain ordinary C. */
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

void mbMathInit(void)
{
    s32 i;

    cosTab = HuMemDirectMallocNum(HEAP_HEAP, MB_TRIG_TABLE_BYTES, HU_MEMNUM_OVL);
    for (i = 0; i < MB_TRIG_TABLE_COUNT;) {
        cosTab[i] = HuCos((360.0f / MB_TRIG_TABLE_COUNT) * i);
        i++;
    }
}

void mbMathClose(void)
{
    if (cosTab != NULL) {
        HuMemDirectFree(cosTab);
        cosTab = NULL;
    }
}

float mbCosDeg(float deg)
{
    return *(float *)((char *)cosTab + MB_TRIG_COS_OFFSET(deg, MB_TRIG_DEG_SCALE));
}

float mbCosRad(float rad)
{
    return *(float *)((char *)cosTab + MB_TRIG_COS_OFFSET(rad, MB_TRIG_RAD_SCALE));
}

float mbSinDeg(float deg)
{
    return *(float *)((char *)cosTab + MB_TRIG_SIN_OFFSET(deg, MB_TRIG_DEG_SCALE));
}

float mbSinRad(float rad)
{
    return *(float *)((char *)cosTab + MB_TRIG_SIN_OFFSET(rad, MB_TRIG_RAD_SCALE));
}

#ifdef __MWERKS__
/* MP4/SDK-style native paired-single kernels; portable C follows below. */
#pragma fp_contract on

void mbMtxRotTrigX(register Mtx mtx, register float sin, register float cos)
{
    /* Two paired rows are snapshotted before the in-place rotation. */
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

void mbMtxRotTrigY(register Mtx mtx, register float sin, register float cos)
{
    /* Two paired rows are snapshotted before the in-place rotation. */
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

void mbMtxRotTrigZ(register Mtx mtx, register float sin, register float cos)
{
    /* Two paired rows are snapshotted before the in-place rotation. */
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

#pragma fp_contract off
#else
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

#ifdef __MWERKS__
#pragma fp_contract on
#endif
void mbMtxRotXDeg(Mtx mtx, float angle)
{
    s32 offset = (s32)(angle * MB_TRIG_DEG_SCALE);
    float *table = cosTab;

    mbMtxRotTrigX(mtx,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)));
}

void mbMtxRotXRad(Mtx mtx, float angle)
{
    s32 offset = (s32)(angle * MB_TRIG_RAD_SCALE);
    float *table = cosTab;

    mbMtxRotTrigX(mtx,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)));
}

void mbMtxRotYDeg(Mtx mtx, float angle)
{
    s32 offset = (s32)(angle * MB_TRIG_DEG_SCALE);
    float *table = cosTab;

    mbMtxRotTrigY(mtx,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)));
}

void mbMtxRotYRad(Mtx mtx, float angle)
{
    s32 offset = (s32)(angle * MB_TRIG_RAD_SCALE);
    float *table = cosTab;

    mbMtxRotTrigY(mtx,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)));
}

void mbMtxRotZDeg(Mtx mtx, float angle)
{
    s32 offset = (s32)(angle * MB_TRIG_DEG_SCALE);
    float *table = cosTab;

    mbMtxRotTrigZ(mtx,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)));
}

void mbMtxRotZRad(Mtx mtx, float angle)
{
    s32 offset = (s32)(angle * MB_TRIG_RAD_SCALE);
    float *table = cosTab;

    mbMtxRotTrigZ(mtx,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)));
}

void mbMtxScaleRotXDeg(Mtx mtx, HuVecF *scale, float angle)
{
    s32 offset = (s32)(angle * MB_TRIG_DEG_SCALE);
    float *table = cosTab;

    mbMtxRotTrigScaleX(mtx,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)), scale);
}

void mbMtxScaleRotYDeg(Mtx mtx, float angle, HuVecF *scale)
{
    s32 offset = (s32)(angle * MB_TRIG_DEG_SCALE);
    float *table = cosTab;

    mbMtxRotTrigScaleY(mtx,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)), scale);
}

void mbMtxScaleRotZDeg(Mtx mtx, float angle, HuVecF *scale)
{
    s32 offset = (s32)(angle * MB_TRIG_DEG_SCALE);
    float *table = cosTab;

    mbMtxRotTrigScaleZ(mtx,
        *(float *)((char *)table + ((offset - 2046) & MB_TRIG_BYTE_MASK)),
        *(float *)((char *)table + ((offset + 2) & MB_TRIG_BYTE_MASK)), scale);
}

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
#pragma fp_contract off
#endif

/* Native double-precision sums, rounded once by the final single stores. */
#ifdef __MWERKS__
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

u32 mbRandMod(u32 mod)
{
    u32 value = frand();

    value &= MB_RAND_VALUE_MASK;
    if (value % 2 != 0) {
        value |= MB_RAND_HIGH_BIT;
    }
    return ((u64)value * mod) >> 32;
}

float mbVecMagXZ(HuVecF *a, HuVecF *b)
{
    float dx = a->x - b->x;
    float dz = a->z - b->z;

    return HuMagPoint2D(dx, dz);
}

BOOL mbVecMagXZCheck(HuVecF *a, HuVecF *b, float maxDist)
{
    float dist = mbVecMagXZ(a, b);

    if (dist <= maxDist) {
        return TRUE;
    } else {
        return FALSE;
    }
}

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

void mbAngleWrapV(HuVecF *angle)
{
    int i;
    float *dest = (float *)angle;

    for (i = 0; i < 3; i++) {
        *dest = mbAngleWrap(*dest);
        dest++;
    }
}

BOOL mbAngleAdd(float *dest, float angle, float speed)
{
    float wrapAngle = fmod(angle - *dest, 360);
    float diff;

    if (fabs(wrapAngle) < speed) {
        *dest = angle;
        return TRUE;
    }
    if (wrapAngle < 0.0f) {
        wrapAngle += 360.0f;
    }
    if (wrapAngle > 180.0f) {
        diff = -speed;
    } else {
        diff = speed;
    }
    *dest += diff;
    *dest = mbAngleWrap(*dest);
    return FALSE;
}

BOOL mbAngleMoveTo(float *dest, float angle, float speed)
{
    float wrapAngle = fmod(angle - *dest, 360);
    float threshold = 1.0f;

    if (fabs(wrapAngle) < threshold) {
        *dest = angle;
        return TRUE;
    }
    if (wrapAngle < 0.0f) {
        wrapAngle += 360.0f;
    }
    if (wrapAngle > 180.0f) {
        wrapAngle -= 360.0f;
    }
    *dest = fmod(*dest + (speed * wrapAngle), 360.0);
    if (*dest < 0.0f) {
        *dest += 360.0f;
    }
    return FALSE;
}

float mbAngleWrap2(float a, float b)
{
    float angle = fmod(a - b, 360);

    if (angle < 0.0f) {
        angle += 360.0f;
    }
    if (angle >= 180.0f) {
        angle -= 360.0f;
    }
    return angle;
}

BOOL mbVecMagCheck(HuVecF *a, HuVecF *b, float dist)
{
    HuVecF diff;

    VECSubtract(a, b, &diff);
    if (VECSquareMag(&diff) >= dist * dist) {
        return FALSE;
    } else {
        return TRUE;
    }
}

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

void mbPos3Dto2D(HuVecF *src, HuVecF *dst)
{
    MBCAMERA *cameraP = mbCameraGet();
    float tanFov;
    float width;
    float height;
    Mtx lookAt;
    HuVecF pos;

    MTXLookAt(lookAt, &cameraP->eye, &cameraP->up, &cameraP->center);
    MTXMultVec(lookAt, src, &pos);
    tanFov = mbSinDeg(cameraP->fov * 0.5f) / mbCosDeg(cameraP->fov * 0.5f);
    width = HU_DISP_ASPECT * (tanFov * pos.z);
    height = tanFov * pos.z;
    dst->x = HU_DISP_CENTERX + (pos.x * (HU_DISP_CENTERX / -width));
    dst->y = HU_DISP_CENTERY + (pos.y * (HU_DISP_CENTERY / height));
    dst->z = -pos.z;
}

void mbPos3DtoNorm(HuVecF *src, s16 cameraMask, HuVecF *dst)
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
    MTXMultVec(lookAt, src, &pos);
    tanFov = mbSinDeg(cameraP->fov * 0.5f) / mbCosDeg(cameraP->fov * 0.5f);
    height = tanFov * -pos.z;
    width = HU_DISP_ASPECT * height;
    dst->x = pos.x / width;
    dst->y = pos.y / height;
    dst->z = pos.z;
}

void mbPos2Dto3D(HuVecF *src, HuVecF *dst)
{
    MBCAMERA *cameraP = mbCameraGet();
    float tanFov = mbSinDeg(cameraP->fov * 0.5f) / mbCosDeg(cameraP->fov * 0.5f);
    float height = 2.0f * (tanFov * src->z);
    float width = HU_DISP_ASPECT * height;
    float normX = src->x / HU_DISP_WIDTH;
    float normY = src->y / HU_DISP_HEIGHT;
    Mtx lookAt;

    dst->x = (normX - 0.5) * width;
    dst->y = -(normY - 0.5) * height;
    dst->z = -src->z;
    mbCameraLookAtInvGet(lookAt);
    MTXMultVec(lookAt, dst, dst);
}

void mbNormPosto3D(HuVecF *src, s16 cameraMask, HuVecF *dst)
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
    absoluteDepth = MathAbsFloat(src->z);
    fovTan = mbSinDeg(halfFov) / cosine;
    depth = fovTan * absoluteDepth;
    dst->x = src->x * (HU_DISP_ASPECT * depth);
    dst->y = src->y * depth;
    dst->z = src->z;
    MTXLookAt(lookAt, &cameraP->pos, &cameraP->up, &cameraP->target);
    MTXInverse(lookAt, lookAtInv);
    MTXMultVec(lookAtInv, dst, dst);
}

void mbNormPosto2D(HuVecF *src, HuVecF *dst)
{
    dst->x = HU_DISP_CENTERX * (1.0f + src->x);
    dst->y = -HU_DISP_CENTERY * (src->y - 1.0f);
    dst->z = src->z;
}

float mbBezierCalc(float a, float b, float c, float t)
{
    float invTime = 1.0f - t;

    return (t * t * c) + ((invTime * invTime * a) + (b * ((2.0f * invTime) * t)));
}

void mbBezierCalcV(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *dst, float t)
{
    dst->x = mbBezierCalc(a->x, b->x, c->x, t);
    dst->y = mbBezierCalc(a->y, b->y, c->y, t);
    dst->z = mbBezierCalc(a->z, b->z, c->z, t);
}

void mbBezierCalcVList(HuVecF *src, HuVecF *dst, float t)
{
    dst->x = mbBezierCalc(src[0].x, src[1].x, src[2].x, t);
    dst->y = mbBezierCalc(src[0].y, src[1].y, src[2].y, t);
    dst->z = mbBezierCalc(src[0].z, src[1].z, src[2].z, t);
}

float mbBezierCalcSlope(float a, float b, float c, float t)
{
    return 2.0f * ((-a + b) + (t * (c + (a - (2.0f * b)))));
}

void mbBezierCalcSlopeV(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *dst, float t)
{
    dst->x = mbBezierCalcSlope(a->x, b->x, c->x, t);
    dst->y = mbBezierCalcSlope(a->y, b->y, c->y, t);
    dst->z = mbBezierCalcSlope(a->z, b->z, c->z, t);
}

float mbHermiteCalc(float a, float b, float c, float d, float t)
{
    float tt = t * t;
    float ttt = t * t * t;
    float aCoef = 1.0f + ((2.0f * ttt) - (3.0f * tt));
    float bCoef = (-2.0f * ttt) + (3.0f * tt);
    float cCoef = t + (ttt - (2.0f * tt));
    float dCoef = ttt - tt;

    return (aCoef * a) + (bCoef * b) + (cCoef * c) + (dCoef * d);
}

void mbHermiteCalcV(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, HuVecF *dst, float t)
{
    dst->x = mbHermiteCalc(a->x, b->x, c->x, d->x, t);
    dst->y = mbHermiteCalc(a->y, b->y, c->y, d->y, t);
    dst->z = mbHermiteCalc(a->z, b->z, c->z, d->z, t);
}

float mbHermiteCalcSlope(float a, float b, float c, float d, float t)
{
    float tt = t * t;
    float aCoef = (6.0f * tt) - (6.0f * t);
    float bCoef = (-6.0f * tt) + (6.0f * t);
    float cCoef = 1.0f + ((3.0f * tt) - (4.0f * t));
    float dCoef = (3.0f * tt) - (2.0f * t);

    return (aCoef * a) + (bCoef * b) + (cCoef * c) + (dCoef * d);
}

float mbAngleLerp(float a, float b, float t)
{
    float diff = fmod(b - a, 360);
    float ret;

    if (diff < 0.0f) {
        diff += 360.0f;
    }
    if (diff > 180.0f) {
        diff -= 360.0f;
    }
    ret = fmod(a + (t * diff), 360);
    if (ret < 0.0f) {
        ret += 360.0f;
    }
    return ret;
}

float mbAngleEaseOut(float a, float b, float t)
{
    return mbAngleLerp(a, b, HuSin(t * 90.0f));
}

float mbAngleEaseIn(float a, float b, float t)
{
    return mbAngleLerp(a, b, 1.0f - HuCos(t * 90.0f));
}

float mbMathDistScale(HuVecF *src, float scale, HuVecF *dst)
{
    MBCAMERA *cameraP = mbCameraGet();
    HuVecF pos;
    float tanFov;
    float depth;
    float z;

    mbPos3Dto2D(src, &pos);
    tanFov = HuSin(cameraP->fov * 0.5f) / HuCos(cameraP->fov * 0.5f);
    depth = pos.z * tanFov;
    z = (depth / scale) / tanFov;
    pos.z = z;
    mbPos2Dto3D(&pos, dst);
    return 0.0f;
}

static void ObjectCullUpdate(HSF_OBJECT *object, Mtx mtx)
{
    HU3D_CAMERA *cameraP;
    HuVecF center;
    HuVecF centerView;
    ROMtx cullMtx;
    float fov;
    float fovTan;
    float near;
    float aspect;
    float cameraH;
    float aspectInv;
    s32 i;
    BOOL cullF;
    BOOL verticalCullF;

    object->flags &= ~HSF_MATERIAL_DISPOFF;
    if (shadowModelDrawF == FALSE) {
        cameraP = &Hu3DCamera[Hu3DCameraNo];
        fov = cameraP->fov;
        near = cameraP->near;
        aspect = cameraP->aspect;
    } else {
        fov = Hu3DShadow->fov;
        near = Hu3DShadow->near;
        aspect = 1.0f;
    }
    fovTan = -1.0f * (mbSinDeg(fov * 0.5f) / mbCosDeg(fov * 0.5f));
    PSVECAdd(&object->mesh.mesh.min, &object->mesh.mesh.max, &center);
    PSVECScale(&center, &center, 0.5f);
    PSMTXMultVec(mtx, &center, &centerView);
    cameraH = MathAbsFloat(centerView.z * fovTan);
    if (MathAbsFloat(centerView.x) <= cameraH * aspect
        && MathAbsFloat(centerView.y) <= cameraH
        && centerView.y < -near) {
        return;
    }

    PSMTXReorder(mtx, cullMtx);
    aspectInv = 1.0f / aspect;
    cullMtx[0][0] *= aspectInv;
    cullMtx[1][0] *= aspectInv;
    cullMtx[2][0] *= aspectInv;
    cullMtx[3][0] *= aspectInv;
    cullMtx[0][2] *= fovTan;
    cullMtx[1][2] *= fovTan;
    cullMtx[2][2] *= fovTan;
    cullMtx[3][2] *= fovTan;

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
        if (objectBBoxView[i].z > near) {
            break;
        }
    }
    if (i >= 8) {
        object->flags |= HSF_MATERIAL_DISPOFF;
        return;
    }

    cullF = FALSE;
    if (centerView.x >= 0.0f) {
        for (i = 0; i < 8; i++) {
            if (objectBBoxView[i].x < objectBBoxView[i].z) {
                break;
            }
        }
        if (i >= 8) {
            cullF = TRUE;
        }
    } else {
        for (i = 0; i < 8; i++) {
            if (objectBBoxView[i].x > -objectBBoxView[i].z) {
                break;
            }
        }
        if (i >= 8) {
            cullF = TRUE;
        }
    }
    if (cullF) {
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
        object->flags |= HSF_MATERIAL_DISPOFF;
    }
}

#pragma dont_inline on
#ifdef __MWERKS__
/* Native paired-single extrema kernel, with offsets derived from the HSF ABI. */
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
#pragma dont_inline reset

#ifdef __MWERKS__
#pragma fp_contract on
#endif
/* Set only the translation column, after snapshotting the three inputs. */
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

#ifdef __MWERKS__
#pragma fp_contract off
#endif

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
