#ifndef REL_MINIGAME_POSITIONAL_AUDIO_H
#define REL_MINIGAME_POSITIONAL_AUDIO_H

#include "game/object.h"

typedef struct MinigameAudioRange {
    u32 cameraMask; /* Cameras used to project the sound position. */
    int panMin, panMax; /* Pan limits at the sides of the camera view. */
    int volumeMin, volumeMiddle, volumeMax; /* Volume levels across the range. */
    f32 distanceFar, distanceMiddle, distanceNear; /* Distance breakpoints in world units. */
} MinigameAudioRange;

typedef struct MinigameAudioRecord {
    int handle, effect; /* Active sound handle and effect resource ID. */
    u32 cameraMask; /* Cameras used for positional audio. */
    int panEnabled, volumeEnabled; /* Whether camera position changes pan or volume. */
    Vec *position; /* World position followed while the sound is active. */
} MinigameAudioRecord;

typedef struct MinigameAudioState {
    MinigameAudioRange *ranges; /* Positional audio settings for the scene. */
    OMOBJ_FUNC callback; /* Per-frame sound position update callback. */
    MinigameAudioRecord records[64]; /* Active positional sound instances. */
} MinigameAudioState;

#endif
