#ifndef REL_MINIGAME_MENU_EASING_H
#define REL_MINIGAME_MENU_EASING_H

#include "dolphin/types.h"
#include "math.h"

/* Interpolates between two coordinates using a clamped sine curve. */
static inline f32 MinigameMenuEaseOut(f32 start, f32 end, f32 elapsed, f32 duration)
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

/* Uses a half sine wave to move from start toward end and back. */
static inline f32 MinigameMenuHalfSine(f32 start, f32 end, f32 elapsed, f32 duration)
{
    if ((elapsed <= 0.0f) || (elapsed >= duration)) {
        return start;
    }
    return (f32)((f64)start + (f64)(end - start) *
        sin((M_PI * (f64)((180.0f / duration) * elapsed)) / 180.0));
}

#endif
