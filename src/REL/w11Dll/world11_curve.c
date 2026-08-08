#include "dolphin.h"

typedef struct W11CurveWork {
    s32 unk_00;
    s32 unk_04;
    s32 unk_08;
    f32 unk_0C;
    f32 unk_10;
    f32 unk_14;
} W11CurveWork;

extern f32 mbHermiteCalc(f32 a, f32 b, f32 c, f32 d, f32 t);

f32 fn_1_1454(const u8 *values, s32 stride, W11CurveWork *work)
{
    f32 lower;
    f32 upper;
    f32 delta;
    f32 slopeIn;
    f32 slopeOut;

    lower = *(const f32 *)(values + stride * work->unk_08);
    upper = *(const f32 *)(values + stride * (work->unk_08 + 1));
    delta = upper - lower;
    if (work->unk_00 != 0) {
        slopeIn = delta;
    } else {
        slopeIn = work->unk_10
            * (delta + (lower
                - *(const f32 *)(values + stride * (work->unk_08 - 1))));
    }
    if (work->unk_04 != 0) {
        slopeOut = delta;
    } else {
        slopeOut = work->unk_14
            * (delta + (*(const f32 *)(values + stride * (work->unk_08 + 2)) - upper));
    }
    return mbHermiteCalc(lower, upper, slopeIn, slopeOut, work->unk_0C);
}
