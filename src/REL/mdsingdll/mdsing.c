#include "humath.h"

#define sind(x) sin((M_PI * (x)) / 180.0)

const s32 lbl_1_rodata_10 = -1;

const s32 lbl_1_rodata_14[16] = {
    0x3B5, 0x3B6, 0x3B7, 0x3B8, 0x3B9, 0x3BA, 0x3BB, -1,
    0x3AD, 0x3AE, 0x3AF, 0x3B0, 0x3B1, 0x3B2, 0x3B3, -1,
};

const s32 lbl_1_rodata_54[2] = { 0x60000, 0xC0000 };

float fn_1_1230(float start, float control, float end, float weight)
{
    float inverse = 1.0f - weight;

    return (end * (weight * weight)) +
        ((start * (inverse * inverse)) +
            (2.0f * (control * (inverse * weight))));
}

void fn_1_128C(HuVecF *out, const HuVecF *start, const HuVecF *control,
    const HuVecF *end, float weight)
{
    out->x = fn_1_1230(start->x, control->x, end->x, weight);
    out->y = fn_1_1230(start->y, control->y, end->y, weight);
    out->z = fn_1_1230(start->z, control->z, end->z, weight);
}

float fn_1_1494(float current, float target, float weight)
{
    if (current == target) {
        return target;
    }
    return (target + (current * (weight - 1.0f))) / weight;
}

void fn_1_14C4(HuVecF *current, const HuVecF *target, float weight)
{
    current->x = fn_1_1494(current->x, target->x, weight);
    current->y = fn_1_1494(current->y, target->y, weight);
    current->z = fn_1_1494(current->z, target->z, weight);
}

float fn_1_1608(float start, float end, float time, float duration)
{
    if (time <= 0.0f) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + ((end - start) * sind((90.0f / duration) * time));
}

float fn_1_16F0(float start, float end, float time, float duration)
{
    if (time <= 0.0f) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + ((time / duration) * (end - start));
}
