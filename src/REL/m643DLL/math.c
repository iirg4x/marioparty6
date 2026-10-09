/* Vector and angle helpers used by Jump the Gun's gameplay code. */
#include "humath.h"
#include "math.h"

/* Converts cosine to an angle; out-of-range values map to zero or pi. */
float fn_1_214(float cosine)
{
    if ((-1.0f) < cosine) {
        if (cosine < (1.0f)) {
            return (float)acos((double)cosine);
        }
        return (0.0f);
    }
    return (3.1415927410125732f);
}

/* Initializes a vector from its three components for gameplay calculations. */
void fn_1_298(HuVecF *out, f32 x, f32 y, f32 z)
{
    out->x = x;
    out->y = y;
    out->z = z;
}

/* Returns the length of a vector for callers that need its magnitude. */
float fn_1_2A8(const HuVecF *vec)
{
    return sqrtf(vec->z * vec->z + (vec->x * vec->x + vec->y * vec->y));
}

#include "humath.h"

f32 fn_1_3C8(const HuVecF *vector)
{
    return (vector->z * vector->z) + ((vector->x * vector->x) + (vector->y * vector->y));
}

f32 fn_1_3F8(const HuVecF *left, const HuVecF *right)
{
    return PSVECDotProduct(left, right);
}
void fn_1_428(HuVecF *dst, const HuVecF *left, const HuVecF *right)
{
    PSVECCrossProduct(left, right, dst);
}
void fn_1_460(HuVecF *dst, const HuVecF *left, const HuVecF *right)
{
    PSVECAdd(left, right, dst);
}
void fn_1_498(HuVecF *dst, const HuVecF *left, const HuVecF *right)
{
    PSVECSubtract(left, right, dst);
}

/* Normalizes a direction for lighting and camera calculations; near-zero vectors become zero. */
f32 fn_1_4D0(HuVecF *dst, const HuVecF *src)
{
    f32 squaredMagnitude;
    f32 measuredMagnitude;
    f32 magnitude;

    squaredMagnitude = (src->z * src->z) + ((src->x * src->x) + (src->y * src->y));
    measuredMagnitude = sqrtf(squaredMagnitude);
    magnitude = measuredMagnitude;
    if (magnitude > (1.1920928955078125e-07f)) {
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

/* Scales a vector when gameplay needs a chosen direction length. */
void fn_1_6B0(HuVecF *dst, const HuVecF *src, f32 scale)
{
    PSVECScale(src, dst, scale);
}

/* Transforms a direction vector by the matrix's 3x3 rotation portion. */
void fn_1_6E8(HuVecF *dst, const HuVecF *src, Mtx matrix)
{
    dst->x = (src->z * matrix[2][0]) + ((src->x * matrix[0][0]) + (src->y * matrix[1][0]));
    dst->y = (src->z * matrix[2][1]) + ((src->x * matrix[0][1]) + (src->y * matrix[1][1]));
    dst->z = (src->z * matrix[2][2]) + ((src->x * matrix[0][2]) + (src->y * matrix[1][2]));
}

#include <string.h>
extern const Vec lbl_1_rodata_34;
extern const Vec lbl_1_rodata_40;
extern f64 asin(f64 x);
extern f64 cos(f64 x);
extern f64 atan2(f64 y, f64 x);
/* Returns vector length for the projectile and rescue orientation builder below. */
static inline f32 Magnitude(const Vec *vec)
{
    return sqrtf(vec->z * vec->z + (vec->x * vec->x + vec->y * vec->y));
}

/* Returns the dot product used to project the reference direction onto the travel direction. */
static inline f32 Dot(const Vec *a, const Vec *b)
{
    return PSVECDotProduct(a, b);
}

/* Scales basis vectors while building projectile and rescue travel orientation. */
static inline void Scale(Vec *dst, const Vec *src, f32 scale)
{
    PSVECScale(src, dst, scale);
}

const Vec lbl_1_rodata_34 = {0.0f, 1.0f, 0.0f};
const Vec lbl_1_rodata_40 = {0.0f, 0.0f, 1.0f};

/* Flying-shot and rescue-approach callbacks build a travel orientation from consecutive
 * positions and a reference direction; degenerate directions return failure. */
s32 fn_1_77C(Mtx matrix, const Vec *start, const Vec *end, const Vec *direction)
{
    Vec edge;
    Vec transverse;
    Vec cross;
    f32 length;
    s32 axisIndex;

    PSVECSubtract(end, start, &edge);
    length = Magnitude(&edge);
    if (length < (9.999999974752427e-07f)) {
        return 0;
    }
    if ((0.0f) != length) {
        Scale(&edge, &edge, (1.0f) / length);
    }
    Scale(&transverse, &edge, Dot(direction, &edge));
    PSVECSubtract(direction, &transverse, &transverse);
    length = Magnitude(&transverse);
    if ((9.999999974752427e-07f) > length) {
        /* Use world up, then world forward, when the supplied direction is parallel. */
        Vec up = lbl_1_rodata_34;
        Scale(&transverse, &edge, edge.y);
        PSVECSubtract(&up, &transverse, &transverse);
        length = Magnitude(&transverse);
        if ((9.999999974752427e-07f) > length) {
            Vec forward = lbl_1_rodata_40;
            Scale(&transverse, &edge, edge.z);
            PSVECSubtract(&forward, &transverse, &transverse);
            length = Magnitude(&transverse);
            if ((9.999999974752427e-07f) > length) {
                return 0;
            }
        }
    }
    if ((0.0f) != length) {
        Scale(&transverse, &transverse, (1.0f) / length);
    }
    PSVECCrossProduct(&transverse, &edge, &cross);
    memset(matrix, 0, 36U);
    for (axisIndex = 0; axisIndex < 3; axisIndex++) {
        matrix[axisIndex][axisIndex] = (1.0f);
    }
    /* The first basis column negates only the cross product's X component. */
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
/* Computes single-precision arcsine for projectile and rescue orientation extraction. */
static inline f32 Asinf(f32 x)
{
    return (f32)asin((f64)x);
}

/* Computes atan2 for projectile and rescue orientation extraction. */
static inline f32 Atan2f(f32 y, f32 x)
{
    return (f32)atan2((f64)y, (f64)x);
}

/* Keeps angle extraction defined when rounding moves sine outside its range. */
static inline f32 ClampedAsin(f32 value)
{
    if (value >= (1.0f)) return (1.5707963705062866f);
    if (value <= (-1.0f)) return (-1.5707963705062866f);
    return Asinf(value);
}

/* Angle extraction returns +/-pi/2 when X is zero, including +pi/2 for the pair (0, 0). */
static inline f32 GuardedAtan2(f32 y, f32 x)
{
    if ((0.0f) == x) {
        if (y >= (0.0f)) return (1.5707963705062866f);
        return (-1.5707963705062866f);
    }
    return Atan2f(y, x);
}

/* Flying-shot and rescue-approach callbacks extract model rotation angles in radians
 * from the travel-orientation matrix. */
s32 fn_1_E74(Vec *angles, Mtx matrix)
{
    f32 xLength;
    f32 yLength;
    f32 zLength;
    f32 cosine;

    xLength = sqrtf(matrix[2][0] * matrix[2][0] +
                       (matrix[0][0] * matrix[0][0] + matrix[1][0] * matrix[1][0]));
    if (xLength < (9.99999993922529e-09f)) goto fail;
    yLength = sqrtf(matrix[2][1] * matrix[2][1] +
                       (matrix[0][1] * matrix[0][1] + matrix[1][1] * matrix[1][1]));
    if (yLength < (9.99999993922529e-09f)) goto fail;
    zLength = sqrtf(matrix[2][2] * matrix[2][2] +
                       (matrix[0][2] * matrix[0][2] + matrix[1][2] * matrix[1][2]));
    if (zLength < (9.99999993922529e-09f)) goto fail;
    angles->y = -ClampedAsin(-matrix[2][0] / xLength);
    cosine = (f32)cos((f64)angles->y);
    if (cosine > (9.99999993922529e-09f)) {
        angles->x = -GuardedAtan2(matrix[2][1] / yLength, matrix[2][2] / zLength);
        angles->z = -GuardedAtan2(matrix[1][0], matrix[0][0]);
    } else {
        /* When the cosine of the Y angle is near zero, force the Z angle to pi. */
        angles->x = -GuardedAtan2(matrix[0][1], matrix[1][1]);
        angles->z = (3.1415927410125732f);
    }
    return 1;
fail:
    /* A near-zero matrix column yields zero angles and a failure result. */
    angles->x = (0.0f);
    angles->y = (0.0f);
    angles->z = (0.0f);
    return 0;
}

/* Builds a Y-axis rotation matrix; the minigame's gameplay does not call this helper. */
void fn_1_1500(Mtx out, double angle)
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

/* Scales the nine entries of a 3x3 matrix; the minigame's gameplay does not call this helper. */
void fn_1_15D8(Mtx dst, Mtx src, float scale)
{
    int column;
    int row;
    for (row = 0; row < 3; row++) {
        for (column = 0; column < 3; column++) {
            dst[row][column] = scale * src[row][column];
        }
    }
}
