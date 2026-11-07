/* Minigame sequence callbacks, object creation helpers, and vector math. */
#include "math.h"
#include "float.h"

#include "dolphin/gx.h"

#include "include/game/mg/actman.h"

#include "include/game/mg/seqman.h"

#include "game/object.h"

#include "game/mg/seqman.h"

#include "game/main.h"

#include "game/charman.h"

#include "game/hu3d.h"

#include "game/gamework.h"

#include "game/wipe.h"

#include "game/mg/timer.h"

#include "game/mg/actman.h"

#include "game/sprite.h"

#include "game/memory.h"

#include "game/audio.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

#include "include/humath.h"

#include "humath.h"
#include <string.h>
#include "REL/m615DLL.h"

void fn_1_26C(void);

void fn_1_290(s16 mode, s16 frameNo);

void fn_1_2D0(s16 mode, s16 frameNo);

void fn_1_2D4(s16 mode, s16 frameNo);

void fn_1_2D8(s16 mode, s16 frameNo);

void fn_1_2DC(s16 mode, s16 frameNo);

void fn_1_2E0(s16 mode, s16 frameNo);

void fn_1_2E4(s16 mode, s16 frameNo);

void fn_1_2E8(s16 mode, s16 frameNo);

void fn_1_2EC(s16 mode, s16 frameNo);

MGSEQ_PARAM lbl_1_data_0 = {
    0x1E,
    0,
    fn_1_290,
    fn_1_2D0,
    fn_1_2D4,
    fn_1_2D8,
    fn_1_2DC,
    fn_1_2E0,
    fn_1_2E4,
    fn_1_2E8,
    fn_1_2EC
};

void fn_1_26C(void)
{
    fn_80070788(&lbl_1_data_0);
    for (;;) {
        fn_800856A8();
        HuPrcVSleep();
    }
}

extern void *lbl_1_bss_0;

void *fn_1_2F0(s32 priority, u32 bytes, void (*update)(OMOBJ *));

void fn_1_290(s16 mode, s16 frameNo)
{
    lbl_1_bss_0 = fn_1_2F0(100, 8, fn_1_9620);
    fn_80071A9C();
}

void fn_1_2D0(s16 mode, s16 frameNo)
{
}

void fn_1_2D4(s16 mode, s16 frameNo)
{
}

void fn_1_2D8(s16 mode, s16 frameNo)
{
}

void fn_1_2DC(s16 mode, s16 frameNo)
{
}

void fn_1_2E0(s16 mode, s16 frameNo)
{
}

void fn_1_2E4(s16 mode, s16 frameNo)
{
}

void fn_1_2E8(s16 mode, s16 frameNo)
{
}

void fn_1_2EC(s16 mode, s16 frameNo)
{
}

extern HUPROCESS *lbl_1_bss_4;

void *fn_1_2F0(s32 priority, u32 bytes, void (*update)(OMOBJ *))
{
    OMOBJ *object;

    object = fn_8003AB38(lbl_1_bss_4, priority, 0, 0, 0, update);
    if (bytes != 0) {
        object->data = HuMemDirectMallocNum(HEAP_HEAP, bytes, 0x10000000);
        memset(object->data, 0, bytes);
    } else {
        object->data = NULL;
    }
    return object->data;
}

void fn_1_390(OMOBJ *object, OMOBJ_FUNC callback);

void fn_1_390(OMOBJ *object, OMOBJ_FUNC callback)
{
    object->objFunc = callback;
}

/* Returns the arc cosine of a value clamped to [-1, 1]. */
f32 fn_1_398(f32 value) {
    f32 clampedValue;
    f32 lowerClampedValue;
    if (value > 1.0f)
    {
        clampedValue = 1.0f;
    }
    else
    {
        if (value < -1.0f)
        {
            lowerClampedValue = -1.0f;
        }
        else
        {
            lowerClampedValue = value;
        }
        clampedValue = lowerClampedValue;
    }
    return (f32) acos((f64) clampedValue);
}

double acos(double x);

void fn_1_444(HuVecF *out, f32 x, f32 y, f32 z)
{
    out->x = x;
    out->y = y;
    out->z = z;
}

float fn_1_454(const HuVecF *vec)
{
    return sqrtf(vec->z * vec->z + (vec->x * vec->x + vec->y * vec->y));
}

f32 fn_1_574(const HuVecF *v)
{
    return (v->z * v->z) + ((v->x * v->x) + (v->y * v->y));
}

f32 fn_1_5A4(const HuVecF *a, const HuVecF *b);

void fn_1_60C(HuVecF *dst, const HuVecF *a, const HuVecF *b);

#ifndef M615_COLLISION_LAYOUT_DEFINED

#define M615_COLLISION_LAYOUT_DEFINED

typedef struct M615Box {
    HuVecF center;
    HuVecF axis[3];
    union {
        HuVecF halfExtents;
        HuVecF halfExtent;
        HuVecF position;
        f32 halfExtentComponents[3];
    };
} M615Box;

typedef M615Box M615TrianglePosition;

typedef M615Box M615CollisionExtents;

typedef M615Box M615CollisionBox;

#endif

f32 fn_1_5A4(const HuVecF *a, const HuVecF *b)
{
    return PSVECDotProduct(a, b);
}

void fn_1_5D4(HuVecF *dst, const HuVecF *a, const HuVecF *b);

#ifndef M615_COLLISION_LAYOUT_DEFINED

#define M615_COLLISION_LAYOUT_DEFINED

#endif

void fn_1_5D4(HuVecF *dst, const HuVecF *a, const HuVecF *b)
{
    PSVECCrossProduct(a, b, dst);
}

void fn_1_60C(HuVecF *dst, const HuVecF *a, const HuVecF *b)
{
    PSVECAdd(a, b, dst);
}

void fn_1_644(HuVecF *dst, const HuVecF *a, const HuVecF *b);

#ifndef M615_COLLISION_LAYOUT_DEFINED

#define M615_COLLISION_LAYOUT_DEFINED

#endif

void fn_1_644(HuVecF *dst, const HuVecF *a, const HuVecF *b)
{
    PSVECSubtract(a, b, dst);
}

#ifndef M615_COLLISION_LAYOUT_DEFINED

#define M615_COLLISION_LAYOUT_DEFINED

#endif

f32 fn_1_67C(HuVecF *dst, const HuVecF *src)
{
    f32 magnitude2;
    f32 sqrt_result;
    f32 magnitude;

    magnitude2 = (src->z * src->z) + ((src->x * src->x) + (src->y * src->y));
    sqrt_result = sqrtf(magnitude2);
    magnitude = sqrt_result;
    if (magnitude > FLT_EPSILON) {
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

#ifndef M615_COLLISION_LAYOUT_DEFINED

#define M615_COLLISION_LAYOUT_DEFINED

#endif

void fn_1_85C(HuVecF *dst, const HuVecF *src, f32 scale)
{
    PSVECScale(src, dst, scale);
}

static f32 SquareLength_inlined(Point3d *v);

static inline f32 SquareLength_inlined(Point3d *v)
{
    return v->z * v->z + (v->x * v->x + v->y * v->y);
}

f32 fn_1_894(Point3d *point, Point3d *start, Point3d *end)
{
    Point3d difference;
    Point3d segment;
    Point3d projectedSegment;
    f32 dotProduct;
    f32 projection;
    f32 segmentLengthSquared;

    PSVECSubtract(end, start, &segment);
    PSVECSubtract(point, start, &difference);
    dotProduct = PSVECDotProduct(&difference, &segment);
    projection = dotProduct;
    if (projection <= 0.0f) {
        projection = 0.0f;
    } else {
        segmentLengthSquared = SquareLength_inlined(&segment);
        if (projection >= segmentLengthSquared) {
            projection = 1.0f;
            PSVECSubtract(&difference, &segment, &difference);
        } else {
            projection /= segmentLengthSquared;
            PSVECScale(&segment, &projectedSegment, projection);
            PSVECSubtract(&difference, &projectedSegment, &difference);
        }
    }
    return SquareLength_inlined(&difference);
}
