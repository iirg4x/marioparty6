/* Camera lens, viewport, and stored camera-pose data for Sink or Swim. */
#ifndef REL_M625DLL_CAMERA_H
#define REL_M625DLL_CAMERA_H

#include "game/main.h"

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
} M625CameraParams;

typedef struct {
    /* Camera position in world units. */
    Point3d position;
    /* Camera up direction. */
    Point3d up;
    /* Point the camera looks toward, in world units. */
    Point3d target;
} M625CameraPose;

extern M625CameraParams lbl_1_data_0;
extern M625CameraPose lbl_1_data_50;
extern M625CameraPose lbl_1_data_74[2];
extern M625CameraPose lbl_1_data_BC[2];

#endif
