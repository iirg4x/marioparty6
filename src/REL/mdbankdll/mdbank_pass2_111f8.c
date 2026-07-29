#include <dolphin/mtx/GeoTypes.h>

typedef Vec HuVecF;

extern const float lbl_1_rodata_268;

static inline float fn_1_111B0(float current, float target, float weight)
{
    if (current == target || weight <= lbl_1_rodata_268) {
        return target;
    }
    return (target + (current * (weight - lbl_1_rodata_268))) / weight;
}

void fn_1_111F8(HuVecF *current, const HuVecF *target, float weight)
{
    current->x = fn_1_111B0(current->x, target->x, weight);
    current->y = fn_1_111B0(current->y, target->y, weight);
    current->z = fn_1_111B0(current->z, target->z, weight);
}
