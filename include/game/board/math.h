#ifndef _GAME_BOARD_MATH_H
#define _GAME_BOARD_MATH_H

#include "dolphin/types.h"
#include "game/memory.h"
#include "humath.h"

void mbMathInit(void);
void mbMathClose(void);
void mbMtxRot(Mtx mtx, float x, float y, float z);
void mbPos3Dto2D(HuVecF *worldPos, HuVecF *screenPos);
void mbPos2Dto3D(HuVecF *screenPos, HuVecF *worldPos);
void mbNormPosto2D(HuVecF *normPos, HuVecF *screenPos);
void mbNormPosto3D(HuVecF *normPos, s16 cameraMask, HuVecF *worldPos);
void mbPos3DtoNorm(HuVecF *worldPos, s16 cameraMask, HuVecF *normPos);
float mbSinDeg(float deg);
float mbCosDeg(float deg);
float mbAngleWrap(float angle);
float mbAngleWrap2(float angle, float startAngle);
float mbAngleLerp(float startAngle, float endAngle, float t);
float mbAngleEaseOut(float startAngle, float endAngle, float t);
BOOL mbAngleAdd(float *rotation, float targetAngle, float speed);
void mbBezierCalcV(HuVecF *start, HuVecF *control, HuVecF *end, HuVecF *dst, float t);
float mbBezierCalc(float start, float control, float end, float t);
float mbHermiteCalc(float start, float end, float startTangent, float endTangent, float t);
float mbCosRad(float rad);
void mbMtxRotYDeg(Mtx mtx, float angle);
void mbMtxRotZDeg(Mtx mtx, float angle);
void mbMtxScaleRotXDeg(Mtx mtx, HuVecF *scale, float angle);
void mbHermiteCalcV(HuVecF *start, HuVecF *end, HuVecF *startTangent, HuVecF *endTangent,
                    HuVecF *dst, float t);
float mbHermiteCalcSlope(float start, float end, float startTangent, float endTangent, float t);

static inline float BoardSinDeg(float deg) {
    return mbSinDeg(deg);
}

static inline float BoardCosDeg(float deg) {
    return mbCosDeg(deg);
}

static inline void *ModelHeapAlloc(s32 size, u32 allocationNo) {
    return HuMemDirectMallocNum(2, size, allocationNo);
}

static inline void MBBoardVecCopy(register const HuVecF *src, register HuVecF *dst)
{
#ifdef __MWERKS__
    register __vec2x32float__ xy;
    register float z;
    asm {
        psq_l xy, 0(src), 0, 0
        lfs z, 8(src)
        psq_st xy, 0(dst), 0, 0
        stfs z, 8(dst)
    }
#else
    HuVecF value = *src;
    *dst = value;
#endif
}

#endif
