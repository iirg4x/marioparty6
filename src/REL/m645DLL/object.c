/* Object allocation, callbacks and minigame sequence hooks for Clean Team. */
#include "humath.h"

#include "math.h"

#include "game/main.h"

#include "game/object.h"

#include "game/charman.h"

#include "game/hu3d.h"

#include "game/gamework.h"

#include "game/wipe.h"

#include "game/mg/timer.h"

#include "game/mg/seqman.h"

#include "game/mg/actman.h"

#include "game/sprite.h"

extern HUPROCESS *lbl_1_bss_0;

#include "game/memory.h"

#include <string.h>

void fn_1_144(s16 mode, s16 frameNo)
{
}

void fn_1_148(s16 mode, s16 frameNo)
{
}

void fn_1_14C(s16 mode, s16 frameNo)
{
}

void fn_1_150(s16 mode, s16 frameNo)
{
}

void fn_1_154(s16 mode, s16 frameNo)
{
}

void fn_1_158(s16 mode, s16 frameNo)
{
}

void fn_1_15C(s16 mode, s16 frameNo)
{
}

void fn_1_160(s16 mode, s16 frameNo)
{
}

void *fn_1_164(s32 priority, u32 bytes, void (*update)(OMOBJ *))
{
    OMOBJ *object;

    object = fn_8003AB38(lbl_1_bss_0, priority, 0, 0, 0, update);
    if (bytes != 0) {
        object->data = HuMemDirectMallocNum(HEAP_HEAP, bytes, 0x10000000);
        memset(object->data, 0, bytes);
    } else {
        object->data = NULL;
    }
    return object->data;
}

void fn_1_204(OMOBJ *object, OMOBJ_FUNC callback)
{
    object->objFunc = callback;
}

/* Clamp the angle to the supported range before computing its arccosine. */
f32 fn_1_20C(f32 angle)
{
    f32 clampedAngle;
    f32 boundedAngle;

    if (angle > 1.0f) {
        clampedAngle = 1.0f;
    } else {
        if (angle < -1.0f) {
            boundedAngle = -1.0f;
        } else {
            boundedAngle = angle;
        }
        clampedAngle = boundedAngle;
    }
    return (f32)acos((f64)clampedAngle);
}

void fn_1_2B8(HuVecF *out, f32 x, f32 y, f32 z)
{
    out->x = x;
    out->y = y;
    out->z = z;
}

float fn_1_2C8(const HuVecF *vec)
{
    return sqrtf(vec->z * vec->z + (vec->x * vec->x + vec->y * vec->y));
}

f32 fn_1_3E8(const HuVecF *v)
{
    return (v->z * v->z) + ((v->x * v->x) + (v->y * v->y));
}

f32 fn_1_418(const HuVecF *a, const HuVecF *b)
{
    return PSVECDotProduct(a, b);
}

void fn_1_448(HuVecF *dst, const HuVecF *a, const HuVecF *b)
{
    PSVECCrossProduct(a, b, dst);
}

void fn_1_480(HuVecF *dst, const HuVecF *a, const HuVecF *b)
{
    PSVECAdd(a, b, dst);
}

void fn_1_4B8(HuVecF *dst, const HuVecF *a, const HuVecF *b)
{
    PSVECSubtract(a, b, dst);
}

f32 fn_1_4F0(HuVecF *dst, const HuVecF *src)
{
    f32 magnitude2;
    f32 sqrt_result;
    f32 magnitude;

    magnitude2 = (src->z * src->z) + ((src->x * src->x) + (src->y * src->y));
    sqrt_result = sqrtf(magnitude2);
    magnitude = sqrt_result;
    if (magnitude > 1.1920929e-07f) {
        f32 inverse = 1.0f / magnitude;
        dst->x = src->x * inverse;
        dst->y = src->y * inverse;
        dst->z = src->z * inverse;
    } else {
        magnitude = 0.0f;
        dst->x = 0.0f;
        dst->y = 0.0f;
        dst->z = 0.0f;
    }
    return magnitude;
}
