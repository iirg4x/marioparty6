/* Burnstile vector helpers for player movement and rotor tests, plus general matrix helpers. */
#include "math.h"
#include "humath.h"

double acos(double x);

/* Converts the player heading dot product to an angle, returning 0 or pi at the limits. */
float fn_1_220(float x)
{
    if ((-1.0f) < x) {
        if (x < (1.0f)) {
            return (float)acos((double)x);
        }
        return (0.0f);
    }
    return (3.1415927410125732f);
}

/* Writes the three supplied coordinates into a vector used by stage callbacks. */
void fn_1_2A4(HuVecF *out, f32 x, f32 y, f32 z)
{
    out->x = x;
    out->y = y;
    out->z = z;
}

float fn_1_2B4(const HuVecF *vec)
{
    return sqrtf(vec->z * vec->z + (vec->x * vec->x + vec->y * vec->y));
}

f32 fn_1_3D4(const HuVecF *v)
{
    return (v->z * v->z) + ((v->x * v->x) + (v->y * v->y));
}

f32 fn_1_404(const HuVecF *a, const HuVecF *b)
{
    return PSVECDotProduct(a, b);
}
void fn_1_434(HuVecF *dst, const HuVecF *a, const HuVecF *b)
{
    PSVECCrossProduct(a, b, dst);
}
void fn_1_46C(HuVecF *dst, const HuVecF *a, const HuVecF *b)
{
    PSVECAdd(a, b, dst);
}
void fn_1_4A4(HuVecF *dst, const HuVecF *a, const HuVecF *b)
{
    PSVECSubtract(a, b, dst);
}

/* Normalizes src into dst and returns its length; near-zero inputs produce a zero vector and return
 * 0. */
f32 fn_1_4DC(HuVecF *dst, const HuVecF *src)
{
    f32 magnitude2;
    f32 sqrt_result;
    f32 magnitude;

    magnitude2 = (src->z * src->z) + ((src->x * src->x) + (src->y * src->y));
    sqrt_result = sqrtf(magnitude2);
    magnitude = sqrt_result;
    if (magnitude > (0.00000011920928955078125f)) {
        f32 inverse = (1.0f) / magnitude;
        dst->x = src->x * inverse;
        dst->y = src->y * inverse;
        dst->z = src->z * inverse;
    } else {
        magnitude = (0.0f);
        dst->x = (0.0f);
        dst->y = (0.0f);
        dst->z = (0.0f);
    }
    return magnitude;
}

void fn_1_6BC(HuVecF *dst, const HuVecF *src, f32 scale)
{
    PSVECScale(src, dst, scale);
}

/* Multiplies src by the 3x3 rotation basis in mtx and writes the result to dst. */
void fn_1_6F4(HuVecF *dst, const HuVecF *src, Mtx mtx)
{
    dst->x = (src->z * mtx[2][0]) + ((src->x * mtx[0][0]) + (src->y * mtx[1][0]));
    dst->y = (src->z * mtx[2][1]) + ((src->x * mtx[0][1]) + (src->y * mtx[1][1]));
    dst->z = (src->z * mtx[2][2]) + ((src->x * mtx[0][2]) + (src->y * mtx[1][2]));
}

#include <string.h>

extern f64 asin(f64 x);
extern f64 cos(f64 x);
extern f64 atan2(f64 y, f64 x);

/* Builds an orientation basis around a segment, using direction as its preferred up vector. */
s32 fn_1_788(Mtx matrix, const Vec *start, const Vec *end, const Vec *direction)
{
    Vec edge;
    Vec transverse;
    Vec cross;
    f32 length;
    s32 diagonalIndex;

    PSVECSubtract(end, start, &edge);
    length = fn_1_2B4(&edge);
    if (length < (0.000001f)) {
        return 0;
    }
    if ((0.0f) != length) {
        fn_1_6BC(&edge, &edge, (1.0f) / length);
    }
    fn_1_6BC(&transverse, &edge, fn_1_404(direction, &edge));
    PSVECSubtract(direction, &transverse, &transverse);
    length = fn_1_2B4(&transverse);
    if ((0.000001f) > length) {
        /* If direction is parallel to the segment, choose a world axis instead. */
        Vec up = {0.0f, 1.0f, 0.0f};
        fn_1_6BC(&transverse, &edge, edge.y);
        PSVECSubtract(&up, &transverse, &transverse);
        length = fn_1_2B4(&transverse);
        if ((0.000001f) > length) {
            Vec forward = {0.0f, 0.0f, 1.0f};
            fn_1_6BC(&transverse, &edge, edge.z);
            PSVECSubtract(&forward, &transverse, &transverse);
            length = fn_1_2B4(&transverse);
            if ((0.000001f) > length) {
                return 0;
            }
        }
    }
    if ((0.0f) != length) {
        fn_1_6BC(&transverse, &transverse, (1.0f) / length);
    }
    PSVECCrossProduct(&transverse, &edge, &cross);
    /* Only 36 bytes of the 3x4 matrix are cleared; the final translation entry stays untouched. */
    memset(matrix, 0, 36U);
    for (diagonalIndex = 0; diagonalIndex < 3; diagonalIndex++) {
        matrix[diagonalIndex][diagonalIndex] = (1.0f);
    }
    /* Only the X component of the cross-product column is negated; Y and Z retain their signs. */
    matrix[0][0] = -cross.x;
    matrix[0][1] = transverse.x;
    matrix[0][2] = edge.x;
    matrix[1][0] = cross.y;
    matrix[1][1] = transverse.y;
    matrix[1][2] = edge.y;
    matrix[2][0] = cross.z;
    matrix[2][1] = transverse.z;
    matrix[2][2] = edge.z;
    return 1;
}

/* Helpers for extracting Euler angles from a matrix's column basis. */
extern f64 asin(f64 x);
extern f64 cos(f64 x);
extern f64 atan2(f64 y, f64 x);

static inline f32 M668Asinf(f32 x)
{
    return (f32)asin((f64)x);
}

static inline f32 M668Atan2f(f32 y, f32 x)
{
    return (f32)atan2((f64)y, (f64)x);
}

/* Keeps matrix roundoff within asin's domain while extracting the Y angle. */
static inline f32 M668ClampedAsin(f32 value)
{
    /* Keep out-of-range matrix values at the nearest quarter-turn endpoint. */
    if (value >= (1.0f)) return (1.5707963705062866f);
    if (value <= (-1.0f)) return (-1.5707963705062866f);
    return M668Asinf(value);
}

/* Used by matrix angle extraction; returns a quarter-turn when x is zero. */
static inline f32 M668GuardedAtan2(f32 y, f32 x)
{
    /* With x zero, return +pi/2 for y >= 0 (including zero), otherwise -pi/2. */
    if ((0.0f) == x) {
        if (y >= (0.0f)) return (1.5707963705062866f);
        return (-1.5707963705062866f);
    }
    return M668Atan2f(y, x);
}

/* Extracts XYZ angles from a rotation matrix; clears the result if a column is degenerate. */
s32 fn_1_E80(Vec *angles, Mtx matrix)
{
    f32 xLength;
    f32 yLength;
    f32 zLength;
    f32 cosine;

    xLength = sqrtf(matrix[2][0] * matrix[2][0] +
                       (matrix[0][0] * matrix[0][0] + matrix[1][0] * matrix[1][0]));
    if (xLength < (0.00000001f)) goto fail;
    yLength = sqrtf(matrix[2][1] * matrix[2][1] +
                       (matrix[0][1] * matrix[0][1] + matrix[1][1] * matrix[1][1]));
    if (yLength < (0.00000001f)) goto fail;
    zLength = sqrtf(matrix[2][2] * matrix[2][2] +
                       (matrix[0][2] * matrix[0][2] + matrix[1][2] * matrix[1][2]));
    if (zLength < (0.00000001f)) goto fail;
    angles->y = -M668ClampedAsin(-matrix[2][0] / xLength);
    cosine = (f32)cos((f64)angles->y);
    if (cosine > (0.00000001f)) {
        angles->x = -M668GuardedAtan2(matrix[2][1] / yLength, matrix[2][2] / zLength);
        angles->z = -M668GuardedAtan2(matrix[1][0], matrix[0][0]);
    } else {
        /* Near a vertical Y angle, use the alternate extraction and set Z to pi. */
        angles->x = -M668GuardedAtan2(matrix[0][1], matrix[1][1]);
        angles->z = (3.1415927410125732f);
    }
    return 1;
fail:
    angles->x = (0.0f);
    angles->y = (0.0f);
    angles->z = (0.0f);
    return 0;
}

#include "dolphin/mtx/GeoTypes.h"

double sin(double);
double cos(double);

/* Writes the 3x3 Y-axis rotation basis for an angle in radians; leaves translation untouched. */
void fn_1_150C(Mtx out, double angle)
{
    float sine = (float)sin(angle);
    float cosine = (float)cos(angle);

    out[0][0] = cosine;
    out[0][1] = (0.0f);
    out[0][2] = -sine;
    out[1][0] = (0.0f);
    out[1][1] = (1.0f);
    out[1][2] = (0.0f);
    out[2][0] = sine;
    out[2][1] = (0.0f);
    out[2][2] = cosine;
}

/* Scales each element of a 3x3 matrix when a caller adjusts a transform basis. */
void fn_1_15E4(Mtx dst, Mtx src, float scale)
{
    int column;
    int row;
    for (row = 0; row < 3; row++) {
        for (column = 0; column < 3; column++) {
            dst[row][column] = scale * src[row][column];
        }
    }
}

#include "game/frand.h"
void fn_1_164C(HuVecF *dst, Mtx mtx, const HuVecF *src)
{
    PSMTXMultVec(mtx, src, dst);
}
u32 fn_1_1684(Mtx dst, Mtx src)
{
    return PSMTXInverse(src, dst);
}

f32 fn_1_16B4(f32 min, f32 max)
{
    return ((max - min) * frandf()) + min;
}
