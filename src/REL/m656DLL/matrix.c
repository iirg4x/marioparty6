#include "REL/m656/m656.h"

float fn_1_25C(float cosine)
{
    if (-1.0f < cosine) {
        if (cosine < 1.0f) {
            return acos(cosine);
        }
        return 0.0f;
    }
    return 3.1415927f;
}

void fn_1_2E0(Point3d *out, f32 x, f32 y, f32 z)
{
    out->x = x;
    out->y = y;
    out->z = z;
}

float fn_1_2F0(HuVecF *vec)
{
    return sqrtf(vec->z * vec->z + (vec->x * vec->x + vec->y * vec->y));
}

float fn_1_410(Point3d *vec)
{
    return (vec->z * vec->z) + ((vec->x * vec->x) + (vec->y * vec->y));
}

float fn_1_440(HuVecF *a, HuVecF *b)
{
    return PSVECDotProduct(a, b);
}

void fn_1_470(Point3d *out, Point3d *a, Point3d *b)
{
    PSVECCrossProduct(a, b, out);
}

void fn_1_4A8(Point3d *out, Point3d *a, Point3d *b)
{
    PSVECAdd(a, b, out);
}

void fn_1_4E0(Point3d *out, Point3d *a, Point3d *b)
{
    PSVECSubtract(a, b, out);
}

float fn_1_518(HuVecF *out, HuVecF *vec)
{
    float length;
    float scale;
    length = fn_1_2F0(vec);
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

void fn_1_6F8(HuVecF *out, HuVecF *vec, float scale)
{
    PSVECScale(vec, out, scale);
}

void fn_1_730(Point3d *out, Point3d *vec, Mtx44 matrix)
{
    out->x = (vec->z * matrix[2][0]) + ((vec->x * matrix[0][0]) + (vec->y * matrix[1][0]));
    out->y = (vec->z * matrix[2][1]) + ((vec->x * matrix[0][1]) + (vec->y * matrix[1][1]));
    out->z = (vec->z * matrix[2][2]) + ((vec->x * matrix[0][2]) + (vec->y * matrix[1][2]));
}

s32 fn_1_7C4(Mtx out, HuVecF *start, HuVecF *end, HuVecF *up)
{
    HuVecF forward, vertical, side;
    float length;
    int i;
    PSVECSubtract(end, start, &forward);
    length = fn_1_2F0(&forward);
    if (length < 0.000001f) return 0;
    if (length != 0.0f) fn_1_6F8(&forward, &forward, 1.0f / length);
    fn_1_6F8(&vertical, &forward, fn_1_440(up, &forward));
    PSVECSubtract(up, &vertical, &vertical);
    length = fn_1_2F0(&vertical);
    if (0.000001f > length) {
        HuVecF axis = { 0.0f, 1.0f, 0.0f };
        fn_1_6F8(&vertical, &forward, forward.y);
        PSVECSubtract(&axis, &vertical, &vertical);
        length = fn_1_2F0(&vertical);
        if (0.000001f > length) {
            HuVecF axis = { 0.0f, 0.0f, 1.0f };
            fn_1_6F8(&vertical, &forward, forward.z);
            PSVECSubtract(&axis, &vertical, &vertical);
            length = fn_1_2F0(&vertical);
            if (0.000001f > length) return 0;
        }
    }
    if (length != 0.0f) fn_1_6F8(&vertical, &vertical, 1.0f / length);
    PSVECCrossProduct(&vertical, &forward, &side);
    /* Retail clears nine floats, then initializes this 3x4 matrix's diagonal. */
    memset(out, 0, 9 * sizeof(float));
    for (i = 0; i < 3; i++) out[i][i] = 1.0f;
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

static inline float M656GetAngleXY(float x, float y)
{
    if (y == 0.0f) {
        if (x >= 0.0f) return M_PI / 2;
        else return -(M_PI / 2);
    }
    return atan2f(x, y);
}

s32 fn_1_EBC(HuVecF *out, Mtx matrix)
{
    float cosY;
    float scaleX, scaleY, scaleZ;
    float scaleX2, scaleY2, scaleZ2;
    float angleY, dot;
    scaleX2 = matrix[2][0] * matrix[2][0] +
        (matrix[0][0] * matrix[0][0] + matrix[1][0] * matrix[1][0]);
    scaleX = sqrtf(scaleX2);
    if (!(scaleX < 0.00000001f)) {
        scaleY2 = matrix[2][1] * matrix[2][1] +
            (matrix[0][1] * matrix[0][1] + matrix[1][1] * matrix[1][1]);
        scaleY = sqrtf(scaleY2);
        if (!(scaleY < 0.00000001f)) {
            scaleZ2 = matrix[2][2] * matrix[2][2] +
                (matrix[0][2] * matrix[0][2] + matrix[1][2] * matrix[1][2]);
            scaleZ = sqrtf(scaleZ2);
            if (!(scaleZ < 0.00000001f)) {
                dot = -matrix[2][0] / scaleX;
                if (dot >= 1.0f) angleY = M_PI / 2;
                else if (dot <= -1.0f) angleY = -(M_PI / 2);
                else angleY = asinf(dot);
                out->y = -angleY;
                cosY = cos(out->y);
                if (cosY > 0.00000001f) {
                    out->x = -M656GetAngleXY(matrix[2][1] / scaleY, matrix[2][2] / scaleZ);
                    out->z = -M656GetAngleXY(matrix[1][0], matrix[0][0]);
                } else {
                    out->x = -M656GetAngleXY(matrix[0][1], matrix[1][1]);
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

void fn_1_1548(Mtx out, f64 angle)
{
    f32 temp_f31;
    f32 temp_f30;

    temp_f31 = (f32) sin(angle);
    temp_f30 = (f32) cos(angle);
    out[0][0] = temp_f30;
    out[0][1] = 0.0f;
    out[0][2] = -temp_f31;
    out[1][0] = 0.0f;
    out[1][1] = 1.0f;
    out[1][2] = 0.0f;
    out[2][0] = temp_f31;
    out[2][1] = 0.0f;
    out[2][2] = temp_f30;
}

void fn_1_1620(Mtx out, const Mtx input, float scale)
{
    int column;
    int row;
    for (row = 0; row < 3; row++) {
        for (column = 0; column < 3; column++) {
            out[row][column] = scale * input[row][column];
        }
    }
}

void fn_1_1688(Point3d *out, Mtx matrix, Point3d *vec)
{
    PSMTXMultVec(matrix, vec, out);
}

void fn_1_16C0(Mtx out, Mtx input)
{
    PSMTXInverse(input, out);
}

void fn_1_16F0(M656Vec4 *out, Mtx44 matrix, HuVecF *vec)
{
    M656Vec4 result;
    result.x = matrix[3][0] + ((matrix[2][0] * vec->z) + ((matrix[0][0] * vec->x) + (matrix[1][0] * vec->y)));
    result.y = matrix[3][1] + ((matrix[2][1] * vec->z) + ((matrix[0][1] * vec->x) + (matrix[1][1] * vec->y)));
    result.z = matrix[3][2] + ((matrix[2][2] * vec->z) + ((matrix[0][2] * vec->x) + (matrix[1][2] * vec->y)));
    result.w = 1.0f;
    *out = result;
}
s32 fn_1_17D4(Mtx44 out, HuVecF *start, HuVecF *end, HuVecF *up)
{
    HuVecF forward, vertical, side;
    float dot;
    PSVECSubtract(end, start, &forward);
    fn_1_518(&forward, &forward);
    dot = fn_1_440(up, &forward);
    vertical.x = up->x - dot * forward.x;
    vertical.y = up->y - dot * forward.y;
    vertical.z = up->z - dot * forward.z;
    fn_1_518(&vertical, &vertical);
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
    out[3][0] = -fn_1_440(start, &side);
    out[3][1] = -fn_1_440(start, &vertical);
    out[3][2] = -fn_1_440(start, &forward);
    out[3][3] = 1.0f;
    return 1;
}

s32 fn_1_1D48(Mtx44 out, f32 angle, f32 aspect)
{
    float half;
    float x;
    float y;
    half = 0.5f * angle;
    x = aspect * cos(half) / sin(half);
    y = cos(half) / sin(half);
    memset(out, 0, sizeof(Mtx44));
    out[0][0] = x;
    out[1][1] = y;
    out[2][2] = 1.0f;
    out[2][3] = 1.0f;
    out[3][2] = 0.0f;
    return 1;
}

void fn_1_1E58(M656MatrixElements *out, M656MatrixElements *a, M656MatrixElements *b)
{
    M656MatrixElements result;
    float *left = a->elements;
    float *right = b->elements;
    result.elements[0] = left[3]*right[12] + (left[2]*right[8] + (left[0]*right[0] + left[1]*right[4]));
    result.elements[1] = left[3]*right[13] + (left[2]*right[9] + (left[0]*right[1] + left[1]*right[5]));
    result.elements[2] = left[3]*right[14] + (left[2]*right[10] + (left[0]*right[2] + left[1]*right[6]));
    result.elements[3] = left[3]*right[15] + (left[2]*right[11] + (left[0]*right[3] + left[1]*right[7]));
    result.elements[4] = left[7]*right[12] + (left[6]*right[8] + (left[4]*right[0] + left[5]*right[4]));
    result.elements[5] = left[7]*right[13] + (left[6]*right[9] + (left[4]*right[1] + left[5]*right[5]));
    result.elements[6] = left[7]*right[14] + (left[6]*right[10] + (left[4]*right[2] + left[5]*right[6]));
    result.elements[7] = left[7]*right[15] + (left[6]*right[11] + (left[4]*right[3] + left[5]*right[7]));
    result.elements[8] = left[11]*right[12] + (left[10]*right[8] + (left[8]*right[0] + left[9]*right[4]));
    result.elements[9] = left[11]*right[13] + (left[10]*right[9] + (left[8]*right[1] + left[9]*right[5]));
    result.elements[10] = left[11]*right[14] + (left[10]*right[10] + (left[8]*right[2] + left[9]*right[6]));
    result.elements[11] = left[11]*right[15] + (left[10]*right[11] + (left[8]*right[3] + left[9]*right[7]));
    result.elements[12] = left[15]*right[12] + (left[14]*right[8] + (left[12]*right[0] + left[13]*right[4]));
    result.elements[13] = left[15]*right[13] + (left[14]*right[9] + (left[12]*right[1] + left[13]*right[5]));
    result.elements[14] = left[15]*right[14] + (left[14]*right[10] + (left[12]*right[2] + left[13]*right[6]));
    result.elements[15] = left[15]*right[15] + (left[14]*right[11] + (left[12]*right[3] + left[13]*right[7]));
    *out = result;
}
