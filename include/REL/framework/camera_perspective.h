#ifndef REL_FRAMEWORK_CAMERA_PERSPECTIVE_H
#define REL_FRAMEWORK_CAMERA_PERSPECTIVE_H

#include "dolphin/types.h"

// Display size, fitting margins, and field of view used when a camera is fitted.
struct CameraPerspective {
    s32 width;        // Display width in pixels.
    s32 height;       // Display height in pixels.
    s32 marginLeft;   // Pixels kept clear at the left edge.
    s32 marginRight;  // Pixels kept clear at the right edge.
    s32 marginTop;    // Pixels kept clear at the top edge.
    s32 marginBottom; // Pixels kept clear at the bottom edge.
    f32 fovRadians;   // Vertical field of view in radians.
    CameraPerspective();
    // Preserves clipping planes and applies the stored field of view with integer aspect division.
    void apply(s32 cameraBit);
    // Reads the engine's field of view in degrees into fovRadians, in radians.
    void read(s32 cameraBit);
};

#endif
