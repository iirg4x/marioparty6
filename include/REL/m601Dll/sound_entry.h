#ifndef M601_SOUND_ENTRY_H
#define M601_SOUND_ENTRY_H

#include "dolphin/types.h"

typedef struct M601SoundEntry {
    s32 count;
    f32 x[3];
    f32 z[3];
    s32 handle;
} M601SoundEntry;

#endif
