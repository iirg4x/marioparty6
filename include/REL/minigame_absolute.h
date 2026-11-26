#ifndef REL_MINIGAME_ABSOLUTE_H
#define REL_MINIGAME_ABSOLUTE_H

#include "dolphin/types.h"

/* Reflect negative values around zero. */
static inline f32 absolute(f32 value)
{
    if (value < 0.0f) {
        return -value;
    }
    return value;
}

#endif
