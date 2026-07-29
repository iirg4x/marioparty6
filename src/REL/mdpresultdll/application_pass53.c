#include "game/sprite.h"
#include "humath.h"

extern const float lbl_1_rodata_E70;
extern const float lbl_1_rodata_E74;
extern const float lbl_1_rodata_E78;
extern HUSPR_GROUPID lbl_1_bss_60;

void fn_1_1F3D4(void);

void fn_1_1F7FC(void)
{
    fn_1_1F3D4();
    HuSprAttrReset(lbl_1_bss_60, 0, HUSPR_ATTR_DISPOFF);
}

void fn_1_1F834(void)
{
    HuSprAttrSet(lbl_1_bss_60, 0, HUSPR_ATTR_DISPOFF);
}

void fn_1_1F868(HuVecF *vec, float x, float y, float z)
{
    vec->x = x;
    vec->y = y;
    vec->z = z;
}

float fn_1_1F878(float start, float end, float time, float duration)
{
    if (time <= lbl_1_rodata_E70) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + ((time / duration) * (end - start));
}

float fn_1_1F8BC(float current, float target, float weight)
{
    if (current == target) {
        return target;
    }
    return (target + (current * (weight - lbl_1_rodata_E74))) / weight;
}

float fn_1_1F8EC(float start, float middle, float end, float time)
{
    float inverse = lbl_1_rodata_E74 - time;

    return (end * (time * time))
        + ((start * (inverse * inverse))
            + (lbl_1_rodata_E78 * (middle * (inverse * time))));
}

void fn_1_1F948(
    HuVecF *result, const HuVecF *start, const HuVecF *middle,
    const HuVecF *end, float time)
{
    result->x = fn_1_1F8EC(start->x, middle->x, end->x, time);
    result->y = fn_1_1F8EC(start->y, middle->y, end->y, time);
    result->z = fn_1_1F8EC(start->z, middle->z, end->z, time);
}

void fn_1_1FB50(HuVecF *current, const HuVecF *target, float weight)
{
    current->x = fn_1_1F8BC(current->x, target->x, weight);
    current->y = fn_1_1F8BC(current->y, target->y, weight);
    current->z = fn_1_1F8BC(current->z, target->z, weight);
}
