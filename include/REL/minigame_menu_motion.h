#ifndef REL_MINIGAME_MENU_MOTION_H
#define REL_MINIGAME_MENU_MOTION_H

/* Evaluate the quadratic path between menu positions. */
static inline float menuQuadraticPosition(float startValue, float controlValue, float endValue,
                                      float progress)
{
    float remaining = 1.0f - progress;

    return (endValue * (progress * progress))
        + ((startValue * (remaining * remaining))
            + ((controlValue * (remaining * progress) * 2.0f)));
}

#endif
