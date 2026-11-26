#ifndef REL_MINIGAME_CAMERA_H
#define REL_MINIGAME_CAMERA_H

#include "dolphin/mtx.h"

/* Lens, viewport, and pose for a selected minigame camera. */
typedef struct {
    /* Bit mask selecting this stage camera. */
    s32 cameraBit;
    /* Vertical field of view in degrees. */
    f32 fieldOfView;
    /* Near clipping distance in world units. */
    f32 nearPlane;
    /* Far clipping distance in world units. */
    f32 farPlane;
    /* Viewport width divided by height. */
    f32 aspectRatio;
    /* Viewport origin in screen pixels. */
    f32 viewportX;
    /* Viewport origin in screen pixels. */
    f32 viewportY;
    /* Viewport width in screen pixels. */
    f32 viewportWidth;
    /* Viewport height in screen pixels. */
    f32 viewportHeight;
    /* Minimum normalized depth value. */
    f32 minDepth;
    /* Maximum normalized depth value. */
    f32 maxDepth;
    /* Camera position in world units. */
    Point3d position;
    /* Camera up direction. */
    Point3d up;
    /* Point the camera looks toward, in world units. */
    Point3d target;
} MinigameCameraParams;

/* Position, up vector, and target of a minigame camera. */
typedef struct {
    /* Camera position in world units. */
    Point3d position;
    /* Camera up direction. */
    Point3d up;
    /* Point the camera looks toward, in world units. */
    Point3d target;
} MinigameCameraPose;

/* Return the index of the lowest set bit in the camera mask. */
static inline s32 cameraIndex(u32 cameras) {
    s32 index;
    for (index = 0; index < 16; index++, cameras >>= 1) {
        if (cameras & 1) return index;
    }
    return 0;
}

#endif
