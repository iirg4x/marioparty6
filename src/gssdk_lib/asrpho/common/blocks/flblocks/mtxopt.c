#include "gssdk/mtx.h"

#include <string.h>

u32 QrPreMult(FloatMatrix *matrix, FloatMatrix *multipliers, FloatMatrix *vector)
{
    f32 *multiplier;
    f32 *matrixValue;
    f32 *matrixCursor;
    f32 *vectorValue;
    f32 *vectorCursor;
    u32 row;
    u32 column;

    column = 0;
    vectorValue = vector->values;
    multiplier = multipliers->values;
    matrixValue = matrix->values;

    while (column++ < matrix->columns) {
        f32 sum;

        matrixValue += column;
        sum = *vectorValue++;
        matrixCursor = matrixValue;
        vectorCursor = vectorValue;
        for (row = column; row < matrix->rows; row++) {
            sum += *vectorCursor++ * *matrixCursor++;
        }

        sum *= *multiplier++;
        vectorValue[-1] -= sum;
        vectorCursor = vectorValue;
        for (row = column; row < matrix->rows; row++) {
            *vectorCursor++ -= sum * *matrixValue++;
        }
    }

    return 0;
}

void mtxFillCopy(FloatMatrix *destination, const FloatMatrix *source)
{
    memcpy(
        destination->values, source->values,
        destination->elementCount * sizeof(f32));
}

void mtxFillX(FloatMatrix *matrix, f32 value)
{
    u32 elementCount = matrix->elementCount;
    f32 *cursor = matrix->values;
    u32 i;

    for (i = 0; i < elementCount; i++) {
        *cursor++ = value;
    }
}
