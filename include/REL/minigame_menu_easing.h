#ifndef REL_MINIGAME_MENU_EASING_H
#define REL_MINIGAME_MENU_EASING_H

#include "dolphin/types.h"
#include "math.h"

/* Returns a clamped sine-eased coordinate for the guide-linked effect's movement. */
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

/* Raises and lowers a guide-linked effect coordinate over its flight. */
static inline f32 MinigameMenuHalfSine(f32 start, f32 end, f32 elapsed, f32 duration)
{
    if ((elapsed <= 0.0f) || (elapsed >= duration)) {
        return start;
    }
    return (f32)((f64)start + (f64)(end - start) *
        sin((M_PI * (f64)((180.0f / duration) * elapsed)) / 180.0));
}

#endif
