#include "dolphin/mtx.h"

extern const float lbl_1_rodata_5C;
extern const float lbl_1_rodata_60;

float fn_1_1230(float start, float control, float end, float weight)
{
    float inverse = lbl_1_rodata_5C - weight;

    return (end * (weight * weight)) +
        ((start * (inverse * inverse)) +
            (lbl_1_rodata_60 * (control * (inverse * weight))));
}

void fn_1_128C(Vec *out, const Vec *start, const Vec *control,
    const Vec *end, float weight)
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
    return (target + (current * (weight - lbl_1_rodata_5C))) / weight;
}

void fn_1_14C4(Vec *current, const Vec *target, float weight)
{
    current->x = fn_1_1494(current->x, target->x, weight);
    current->y = fn_1_1494(current->y, target->y, weight);
    current->z = fn_1_1494(current->z, target->z, weight);
}
