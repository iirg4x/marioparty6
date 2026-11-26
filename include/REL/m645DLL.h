#ifndef M645DLL_H
#define M645DLL_H

#include "REL/m645DLL/shared_types.h"

void ObjectSetup(void);
void fn_1_2ABC(OMOBJ *object);
void fn_1_144(s16 mode, s16 frameNo);
void fn_1_148(s16 mode, s16 frameNo);
void fn_1_14C(s16 mode, s16 frameNo);
void fn_1_150(s16 mode, s16 frameNo);
void fn_1_154(s16 mode, s16 frameNo);
void fn_1_158(s16 mode, s16 frameNo);
void fn_1_15C(s16 mode, s16 frameNo);
void fn_1_160(s16 mode, s16 frameNo);
void *fn_1_164(s32 priority, u32 bytes, void (*update)(OMOBJ *));
void fn_1_204(OMOBJ *object, OMOBJ_FUNC callback);
f32 fn_1_20C(f32 angle);
void fn_1_2B8(HuVecF *out, f32 x, f32 y, f32 z);
float fn_1_2C8(const HuVecF *vec);
f32 fn_1_3E8(const HuVecF *v);
f32 fn_1_418(const HuVecF *a, const HuVecF *b);
void fn_1_480(HuVecF *dst, const HuVecF *a, const HuVecF *b);
void fn_1_4B8(HuVecF *dst, const HuVecF *a, const HuVecF *b);
f32 fn_1_4F0(HuVecF *dst, const HuVecF *src);
void fn_1_6D0(HuVecF *dst, const HuVecF *src, f32 scale);
void fn_1_708(M645Bounds *out, M645Capsule *capsule);
void fn_1_8C0(M645Bounds *bounds, MinigameTriangle *triangle);
void fn_1_B34(M645Bounds *out, M645Sphere *sphere);
s32 fn_1_BC8(M645Bounds *a, M645Bounds *b);
float fn_1_E40(void *point, void *triangle, float *barycentricS, float *barycentricT);
f32 fn_1_F6C(M645Sphere *sphere, MinigameTriangle *triangle, f32 *barycentricS, f32 *barycentricT);
f32 fn_1_1D70(M645Segment *queryPoint, M645Segment *triangle,
             f32 *barycentricSOut, f32 *barycentricTOut);
f32 fn_1_2178(M645Sphere *movingSphere, const M645Segment *lineSegment);

f32 fn_1_2518(M645Sphere *firstSphere, M645Sphere *secondSphere);

#endif
