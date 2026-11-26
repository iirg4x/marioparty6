#ifndef REL_MINIGAME_FOG_H
#define REL_MINIGAME_FOG_H

#include "dolphin/gx.h"

/* Fog type, depth range, and color applied to the selected cameras and layer. */
typedef struct MinigameFogParams {
    s32 layer; /* Render layer using this fog entry; -1 terminates the table. */
    u32 cameras; /* Camera bit mask used to read that camera's clipping planes. */
    GXFogType type; /* GX fog mode applied to this layer. */
    f32 start; /* Fog start distance in game world units. */
    f32 end; /* Unscaled fog end distance in game world units. */
    GXColor color; /* Fog color for this layer. */
} MinigameFogParams;

#endif
