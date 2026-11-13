#ifndef REL_MENU_INTERPOLATION_H
#define REL_MENU_INTERPOLATION_H

#include "math.h"
#include "humath.h"

static inline float fn_1_13A8_inlined(float startValue, float controlValue, float endValue,
                                      float progress)
{
    float remaining = 1.0f - progress;

    return (endValue * (progress * progress))
        + ((startValue * (remaining * remaining))
            + ((controlValue * (remaining * progress) * 2.0f)));
}

static inline f32 fn_1_4C78_ease90_inlined(f32 start, f32 end, f32 elapsed, f32 duration)
{
    if (elapsed <= 0.0f) {
        return start;
    }
    if (elapsed >= duration) {
        return end;
    }
    return (f32)((f64)start + (f64)(end - start) *
        sin((M_PI * (f64)((90.0f / duration) * elapsed)) / 180.0));
}

static inline f32 fn_1_4C78_ease180_inlined(f32 start, f32 end, f32 elapsed, f32 duration)
{
    if ((elapsed <= 0.0f) || (elapsed >= duration)) {
        return start;
    }
    return (f32)((f64)start + (f64)(end - start) *
        sin((M_PI * (f64)((180.0f / duration) * elapsed)) / 180.0));
}

#endif
