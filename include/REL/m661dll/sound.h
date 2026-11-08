#ifndef M661DLL_SOUND_H
#define M661DLL_SOUND_H

#include "dolphin/types.h"

typedef struct VoicePosition {
    f32 panPosition;
    u8 unknown04[4]; /* No code names this field. */
    f32 volumePosition;
} VoicePosition;

void fn_1_5020(s16 slot, s16 mode, VoicePosition *position);

#endif
