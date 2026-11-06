#ifndef M669_VECTOR_H
#define M669_VECTOR_H

#include "humath.h"

f32 M669VecLengthSquared(const HuVecF *vector);
f32 M669VecDot(const HuVecF *a, const HuVecF *b);
void M669VecAdd(HuVecF *dst, const HuVecF *a, const HuVecF *b);
void M669VecSubtract(HuVecF *dst, const HuVecF *a, const HuVecF *b);
void M669VecScale(HuVecF *dst, const HuVecF *src, f32 scale);
float fn_1_298(float x);
void fn_1_31C(HuVecF *out, f32 x, f32 y, f32 z);
float fn_1_32C(const HuVecF *vec);
void fn_1_4AC(HuVecF *dst, const HuVecF *a, const HuVecF *b);
float fn_1_554(HuVecF *out, HuVecF *vec);

#endif
