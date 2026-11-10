/* Word Herd startup, sequence hooks, vector helpers and matrix transforms. */
#include <math.h>
#include "REL/m669DLL/vector.h"
#include "game/matrix_angle.h"

#include "dolphin/gx.h"

#include "game/main.h"

#include "game/object.h"

#include "game/audio.h"

#include "game/charman.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/gamework.h"

#include "game/memory.h"

#include "game/mg/seqman.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/sprite.h"

#include "game/wipe.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

#include "include/game/mg/actman.h"

#include "include/game/mg/seqman.h"

#include "game/mic.h"

#include "game/hu3d.h"

#include "include/humath.h"

#include "include/game/frand.h"

#include "include/dolphin/mtx/GeoTypes.h"

#include "include/string.h"
#include <string.h>

void fn_1_114(s16 mode, s16 frameNo);
void fn_1_16C(s16 mode, s16 frameNo);
void fn_1_170(s16 mode, s16 frameNo);
void fn_1_174(s16 mode, s16 frameNo);
void fn_1_178(s16 mode, s16 frameNo);
void fn_1_17C(s16 mode, s16 frameNo);
void fn_1_180(s16 mode, s16 frameNo);
void fn_1_184(s16 mode, s16 frameNo);
void fn_1_188(s16 mode, s16 frameNo);

MGSEQ_PARAM lbl_1_data_0 = {
    30, 2, fn_1_114, fn_1_16C, fn_1_170, fn_1_174, fn_1_178,
    fn_1_17C, fn_1_180, fn_1_184, fn_1_188,
};

int lbl_1_data_28[2][6] = {
    {DATANUM(0x800000, 16), DATANUM(0x800000, 18), DATANUM(0x800000, 22), DATANUM(0x800000, 24),
        DATANUM(0x800000, 26), DATANUM(0x800000, 20)},
    {DATANUM(0x800000, 17), DATANUM(0x800000, 19), DATANUM(0x800000, 23), DATANUM(0x800000, 25),
        DATANUM(0x800000, 27), DATANUM(0x800000, 20)}
};

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];

extern const VoidFunc _dtors[];

void ObjectSetup(void);

/* REL entry point: runs the constructor table and starts the minigame actor manager. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    ObjectSetup();
    return 0;
}

/* REL shutdown entry point: runs each function in the destructor table. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

void fn_1_F0(void);

HUPROCESS *lbl_1_bss_0;

/* Creates the actor manager during REL startup and schedules the minigame update process. */
void ObjectSetup(void) {
    lbl_1_bss_0 = fn_80080418();
    fn_8000DAC0(fn_1_F0, 100U, 12288U, 0, lbl_1_bss_0);
}

extern OMOBJMAN *lbl_1_bss_4;




/* Starts the sequence controller, then updates minigame actors once per scheduler pass. */
void fn_1_F0(void)
{
    fn_80070788(&lbl_1_data_0);
    for (;;) {
        fn_800856A8();
        HuPrcVSleep();
    }
}

extern void *fn_1_1A8(s32 priority, u32 bytes, void (*update)(OMOBJ *));

void fn_1_50D8(OMOBJ *object);

/* Initialization hook: prepares microphone and pad-answer input, schedules stage setup and
 * advances. The microphone initialization status is ignored. */
void fn_1_114(s16 mode, s16 frameNo)
{
    HuMCInit(0);
    HuMCSelWinCreate(-10000.0f, -10002.0f);
    fn_1_1A8(100, 8, fn_1_50D8);
    fn_80071A9C();
}

extern s32 lbl_1_data_F18;

extern float lbl_1_bss_1BC;

extern s8 lbl_1_bss_218;

extern s8 lbl_1_bss_219;

extern s8 lbl_1_data_2758;

s32 fn_1_367CC(s32 *player);

/* Fade-in sequence hook; performs no additional work. */
void fn_1_16C(s16 mode, s16 frameNo)
{
}

/* Start-announcement sequence hook; performs no additional work. */
void fn_1_170(s16 mode, s16 frameNo)
{
}

/* Main-play sequence hook; performs no additional work. */
void fn_1_174(s16 mode, s16 frameNo)
{
}

/* Finish-announcement sequence hook; performs no additional work. */
void fn_1_178(s16 mode, s16 frameNo)
{
}

/* Pre-winner sequence hook; performs no additional work. */
void fn_1_17C(s16 mode, s16 frameNo)
{
}

/* Winner sequence hook; performs no additional work. */
void fn_1_180(s16 mode, s16 frameNo)
{
}

/* Fade-out sequence hook; performs no additional work. */
void fn_1_184(s16 mode, s16 frameNo)
{
}

/* Close sequence hook: shuts down microphone capture and its input resources. */

void fn_1_188(s16 mode, s16 frameNo) {
    HuMCClose();
}

/* Schedules an object, allocates and clears its optional work, and returns the work pointer. */
void *fn_1_1A8(s32 priority, u32 bytes, void (*update)(OMOBJ *))
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

/* Installs the next object callback; a null callback leaves the object idle. */
void fn_1_248(OMOBJ *object, OMOBJ_FUNC callback)
{
    object->objFunc = callback;
}

void fn_1_164C(HuVecF *dst, Mtx mtx, const HuVecF *src);

u32 fn_1_1684(Mtx dst, Mtx src);

/* Returns a random value between the supplied bounds using the shared game random stream. */
f32 fn_1_250(f32 min, f32 max)
{
    return ((max - min) * frandf()) + min;
}

double acos(double x);

float fn_1_220(float x);

/* Returns acos inside the open interval (-1, 1), with the other branches returning zero or pi. */
float fn_1_298(float x)
{
    if (-1.0f < x) {
        if (x < 1.0f) {
            return (float)acos((double)x);
        }
        return 0.0f;
    }
    return 3.1415927f;
}

/* Writes the three supplied coordinates into a vector. */
void fn_1_31C(HuVecF *out, f32 x, f32 y, f32 z)
{
    out->x = x;
    out->y = y;
    out->z = z;
}

/* Returns the length of a vector for the movement and collision helpers. */
float fn_1_32C(const HuVecF *vec)
{
    return sqrtf(vec->z * vec->z + (vec->x * vec->x + vec->y * vec->y));
}

/* Returns squared vector length for distance and collision comparisons. */
f32 M669VecLengthSquared(const HuVecF *vector)
{
    return (vector->z * vector->z) + ((vector->x * vector->x) + (vector->y * vector->y));
}

/* Returns the SDK dot product of the supplied vectors. */
f32 M669VecDot(const HuVecF *a, const HuVecF *b)
{
    return PSVECDotProduct(a, b);
}

/* Writes the SDK cross product of the supplied vectors. */
void fn_1_4AC(HuVecF *dst, const HuVecF *a, const HuVecF *b)
{
    PSVECCrossProduct(a, b, dst);
}


/* Adds two vectors into the supplied destination. */
void M669VecAdd(HuVecF *dst, const HuVecF *a, const HuVecF *b)
{
    PSVECAdd(a, b, dst);
}

/* Subtracts the second vector from the first into the supplied destination. */
void M669VecSubtract(HuVecF *dst, const HuVecF *a, const HuVecF *b)
{
    PSVECSubtract(a, b, dst);
}

/* Normalizes a vector and returns its original length; very short vectors become zero. */
float fn_1_554(HuVecF *out, HuVecF *vec)
{
    float length;
    float scale;
    length = fn_1_32C(vec);
    if (length > 1.1920929e-7f) {
        scale = 1.0f / length;
        out->x = vec->x * scale;
        out->y = vec->y * scale;
        out->z = vec->z * scale;
    } else {
        length = 0.0f;
        out->x = 0.0f;
        out->y = 0.0f;
        out->z = 0.0f;
    }
    return length;
}

/* Multiplies a vector by the supplied scale. */
void M669VecScale(HuVecF *dst, const HuVecF *src, f32 scale)
{
    PSVECScale(src, dst, scale);
}

/* Applies the three matrix columns to a vector without using translation. */
void fn_1_76C(HuVecF *dst, const HuVecF *src, Mtx mtx)
{
    dst->x = (src->z * mtx[2][0]) + ((src->x * mtx[0][0]) + (src->y * mtx[1][0]));
    dst->y = (src->z * mtx[2][1]) + ((src->x * mtx[0][1]) + (src->y * mtx[1][1]));
    dst->z = (src->z * mtx[2][2]) + ((src->x * mtx[0][2]) + (src->y * mtx[1][2]));
}

/* Builds a basis from start, end and up; returns zero if it cannot form usable forward and vertical
 * directions. */
s32 fn_1_800(Mtx out, HuVecF *start, HuVecF *end, HuVecF *up)
{
    HuVecF forward, vertical, side;
    float length;
    int diagonal;
    PSVECSubtract(end, start, &forward);
    length = fn_1_32C(&forward);
    if (length < 0.000001f) return 0;
    if (length != 0.0f) M669VecScale(&forward, &forward, 1.0f / length);
    M669VecScale(&vertical, &forward, M669VecDot(up, &forward));
    PSVECSubtract(up, &vertical, &vertical);
    length = fn_1_32C(&vertical);
    if (0.000001f > length) {
        HuVecF axis = { 0.0f, 1.0f, 0.0f };
        M669VecScale(&vertical, &forward, forward.y);
        PSVECSubtract(&axis, &vertical, &vertical);
        length = fn_1_32C(&vertical);
        if (0.000001f > length) {
            HuVecF axis = { 0.0f, 0.0f, 1.0f };
            M669VecScale(&vertical, &forward, forward.z);
            PSVECSubtract(&axis, &vertical, &vertical);
            length = fn_1_32C(&vertical);
            if (0.000001f > length) return 0;
        }
    }
    if (length != 0.0f) M669VecScale(&vertical, &vertical, 1.0f / length);
    PSVECCrossProduct(&vertical, &forward, &side);
/* The clear covers nine contiguous floats, including out[0][3] and out[1][3].
 * The translation entry out[2][3] is left unchanged. */
    memset(out, 0, 9 * sizeof(float));
    for (diagonal = 0; diagonal < 3; diagonal++) out[diagonal][diagonal] = 1.0f;
    out[0][0] = -side.x;
    out[0][1] = vertical.x;
    out[0][2] = forward.x;
    out[1][0] = side.y;
    out[1][1] = vertical.y;
    out[1][2] = forward.y;
    out[2][0] = side.z;
    out[2][1] = vertical.z;
    out[2][2] = forward.z;
    return 1;
}

/* Extracts rotation angles in radians after measuring column scales; degenerate columns produce
 * zero angles. */
s32 fn_1_EF8(HuVecF *out, Mtx matrix)
{
    float cosY;
    float scaleX, scaleY, scaleZ;
    float scaleXSquared, scaleYSquared, scaleZSquared;
    float angleY, dot;
    scaleXSquared = matrix[2][0] * matrix[2][0] +
        (matrix[0][0] * matrix[0][0] + matrix[1][0] * matrix[1][0]);
    scaleX = sqrtf(scaleXSquared);
    if (!(scaleX < 0.00000001f)) {
        scaleYSquared = matrix[2][1] * matrix[2][1] +
            (matrix[0][1] * matrix[0][1] + matrix[1][1] * matrix[1][1]);
        scaleY = sqrtf(scaleYSquared);
        if (!(scaleY < 0.00000001f)) {
            scaleZSquared = matrix[2][2] * matrix[2][2] +
                (matrix[0][2] * matrix[0][2] + matrix[1][2] * matrix[1][2]);
            scaleZ = sqrtf(scaleZSquared);
            if (!(scaleZ < 0.00000001f)) {
                dot = -matrix[2][0] / scaleX;
                if (dot >= 1.0f) angleY = M_PI / 2;
                else if (dot <= -1.0f) angleY = -(M_PI / 2);
                else angleY = asinf(dot);
                out->y = -angleY;
                cosY = cos(out->y);
                if (cosY > 0.00000001f) {
                    out->x = -HuAngleXY(matrix[2][1] / scaleY, matrix[2][2] / scaleZ);
                    out->z = -HuAngleXY(matrix[1][0], matrix[0][0]);
                } else {
                    /* Use the alternate X angle near a right-angle Y rotation, fixing Z to pi. */
                    out->x = -HuAngleXY(matrix[0][1], matrix[1][1]);
                    out->z = M_PI;
                }
                return 1;
            }
        }
    }
    out->x = 0.0f;
    out->y = 0.0f;
    out->z = 0.0f;
    return 0;
}

double sin(double);

double cos(double);

/* Writes a Y-axis rotation from a radian angle into the upper three-by-three matrix entries. */
void fn_1_1584(Mtx out, double angle)
{
    float sine = (float)sin(angle);
    float cosine = (float)cos(angle);

    out[0][0] = cosine;
    out[0][1] = 0.0f;
    out[0][2] = -sine;
    out[1][0] = 0.0f;
    out[1][1] = 1.0f;
    out[1][2] = 0.0f;
    out[2][0] = sine;
    out[2][1] = 0.0f;
    out[2][2] = cosine;
}


/* Scales the upper three-by-three matrix entries and leaves translation unchanged. */
void fn_1_165C(Mtx dst, Mtx src, float scale)
{
    int column;
    int row;
    for (row = 0; row < 3; row++) {
        for (column = 0; column < 3; column++) {
            dst[row][column] = scale * src[row][column];
        }
    }
}

/* Transforms a vector with the supplied affine matrix using the SDK routine. */
void fn_1_16C4(HuVecF *dst, Mtx mtx, const HuVecF *src)
{
    PSMTXMultVec(mtx, src, dst);
}

/* Returns the SDK inversion result for the supplied affine matrix. */
u32 fn_1_16FC(Mtx dst, Mtx src)
{
    return PSMTXInverse(src, dst);
}

typedef Vec HuVecF;

typedef struct {
    float x, y, z, w;
} M669Vector4;

/* Transforms a point using matrix columns and the translation row, forcing the result w to one. */
void fn_1_172C(M669Vector4 *out, Mtx44 matrix, HuVecF *vec)
{
    M669Vector4 result;
    result.x = matrix[3][0] +
               ((matrix[2][0] * vec->z) + ((matrix[0][0] * vec->x) + (matrix[1][0] * vec->y)));
    result.y = matrix[3][1] +
               ((matrix[2][1] * vec->z) + ((matrix[0][1] * vec->x) + (matrix[1][1] * vec->y)));
    result.z = matrix[3][2] +
               ((matrix[2][2] * vec->z) + ((matrix[0][2] * vec->x) + (matrix[1][2] * vec->y)));
    result.w = 1.0f;
    *out = result;
}

/* Builds a four-by-four basis and translation matrix from start, end and up, with fixed Z sign
 * changes. */
s32 fn_1_1810(Mtx44 out, HuVecF *start, HuVecF *end, HuVecF *up)
{
    HuVecF forward, vertical, side;
    float dot;
    PSVECSubtract(end, start, &forward);
    fn_1_554(&forward, &forward);
    dot = M669VecDot(up, &forward);
    vertical.x = up->x - dot * forward.x;
    vertical.y = up->y - dot * forward.y;
    vertical.z = up->z - dot * forward.z;
    fn_1_554(&vertical, &vertical);
    PSVECCrossProduct(&vertical, &forward, &side);
    PSVECScale(&side, &side, -1.0f);
    out[0][0] = side.x;
    out[0][1] = vertical.x;
    out[0][2] = forward.x;
    out[0][3] = 0.0f;
    out[1][0] = side.y;
    out[1][1] = vertical.y;
    out[1][2] = forward.y;
    out[1][3] = 0.0f;
    out[2][0] = side.z;
    out[2][1] = -vertical.z;
    out[2][2] = -forward.z;
    out[2][3] = 0.0f;
    out[3][0] = -M669VecDot(start, &side);
    out[3][1] = -M669VecDot(start, &vertical);
    out[3][2] = -M669VecDot(start, &forward);
    out[3][3] = 1.0f;
    return 1;
}

/* Builds a projection matrix from a radian angle and a horizontal scale multiplier. */
s32 fn_1_1D84(Mtx44 out, f32 angle, f32 aspect)
{
    float halfAngle;
    float horizontalScale;
    float verticalScale;
    halfAngle = 0.5f * angle;
    horizontalScale = aspect * cos(halfAngle) / sin(halfAngle);
    verticalScale = cos(halfAngle) / sin(halfAngle);
    memset(out, 0, sizeof(Mtx44));
    out[0][0] = horizontalScale;
    out[1][1] = verticalScale;
    out[2][2] = 1.0f;
    out[2][3] = 1.0f;
    out[3][2] = 0.0f;
    return 1;
}
