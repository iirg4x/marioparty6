/* Camera, fog, and positional audio settings used when the game starts. */
#ifndef REL_M626DLL_SETUP_H
#define REL_M626DLL_SETUP_H
#include "game/object.h"
#include "dolphin/gx.h"

typedef struct M626FogParameters {
    s32 layer; /* Render layer using this fog entry; -1 terminates the table. */
    u32 cameras; /* Camera bit mask used to read that camera's clipping planes. */
    GXFogType type; /* GX fog mode applied to this layer. */
    f32 start; /* Fog start distance in game world units. */
    f32 end; /* Unscaled fog end distance in game world units. */
    GXColor color; /* Fog color for this layer. */
} M626FogParameters;

typedef struct M626AudioParameters {
    u32 cameras; /* Camera bit mask selecting this distance-to-audio range. */
    s32 panMin; /* Minimum sound pan value for the range. */
    s32 panMax; /* Maximum sound pan value for the range. */
    s32 volumeMin; /* Volume at and beyond distanceFar. */
    s32 volumeMiddle; /* Volume at distanceMiddle. */
    s32 volumeMax; /* Volume at and inside distanceNear. */
    f32 distanceFar; /* Far boundary in game world units. */
    f32 distanceMiddle; /* Middle interpolation point in game world units. */
    f32 distanceNear; /* Near boundary in game world units. */
} M626AudioParameters;

extern OM_CAMERA_VIEW lbl_1_data_0;
extern Vec lbl_1_data_1C;
extern Vec lbl_1_data_28;
extern GXColor lbl_1_data_34;
extern M626FogParameters lbl_1_data_38[3];
extern M626FogParameters lbl_1_data_80[3];
extern M626AudioParameters lbl_1_data_C8[2];
#endif
