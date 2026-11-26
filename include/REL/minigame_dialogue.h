#ifndef REL_MINIGAME_DIALOGUE_H
#define REL_MINIGAME_DIALOGUE_H

#include "game/window.h"

typedef struct MinigameWindow {
    HUWINID windowId; /* Handle for the message window. */
    f32 width; /* Window width in pixels. */
    f32 height; /* Window height in pixels. */
} MinigameWindow;

typedef struct MinigameMessageSound {
    u32 messageId; /* Message containing the sound callback marker. */
    s16 callbackCode; /* Code selecting the cue within the message. */
    s32 soundEffectId; /* Sound effect ID. A negative value suppresses playback. */
} MinigameMessageSound;

#endif
