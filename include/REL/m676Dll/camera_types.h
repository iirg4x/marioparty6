/* Camera, light, fog, and positional-audio state used by the minigame. */
#ifndef REL_M676DLL_CAMERA_TYPES_H
#define REL_M676DLL_CAMERA_TYPES_H

#include "game/object.h"

typedef OM_CAMERA_VIEW M676CameraPose;

typedef struct M676CameraRecord {
    /* Camera-motion activity flag, motion ID, and camera-model ID. */
    s32 active, motion, model;
    /* Current and destination camera transforms. */
    M676CameraPose currentPose, targetPose;
    /* Camera position, up direction, and look-at point in world space. */
    Vec position, up, target;
    /* Projection values; nearPlane and farPlane are clipping distances. */
    f32 fov, nearPlane, farPlane, aspect;
    /* Viewport and scissor rectangles in screen coordinates. */
    f32 viewport[4], scissor[4];
    /* Camera state machine phase, elapsed frames, and transition blend. */
    u32 state;
    s32 timer;
    f32 blend;
    /* Unused word retained in the camera record layout. */
    s32 reservedA4;
    /* Yaw-oscillation phase in degrees and its amplitude blend during course travel. */
    f32 angle, fade;
    /* Unused bytes retained at the end of the camera record. */
    u8 reservedB0ToB7[8];
} M676CameraRecord;

typedef struct M676CameraWork {
    /* Camera selection and shadow projection settings. */
    u32 cameras;
    f32 shadowRadius, shadowFov;
    Vec shadowDirection;
    GXColor shadowColor;
    /* Per-frame callback and the sixteen camera slots. */
    void (*callback)(OMOBJ *, u32, s32);
    M676CameraRecord entries[16];
    /* Unused bytes retained at the end of the camera work record. */
    u8 reservedBA0ToBA7[8];
} M676CameraWork;

typedef struct M676AudioConfig {
    /* Cameras used to project sound and bounds for stereo pan and volume. */
    u32 cameras;
    s32 panMin, panMax;
    s32 volumeMin, volumeMiddle, volumeMax;
    /* Distance thresholds in world units, ordered farthest to nearest. */
    f32 distanceFar, distanceMiddle, distanceNear;
} M676AudioConfig;

typedef struct M676FogParam {
    /* Render layer, selected cameras, fog mode, and depth range. */
    s32 layer;
    u32 cameras;
    GXFogType type;
    f32 start, end;
    /* Opaque fog color. */
    GXColor color;
} M676FogParam;

typedef struct M676LightSettings {
    /* Scene light color and fog preset table, terminated by a layer value of -1. */
    GXColor lightColor;
    M676FogParam fog[2];
} M676LightSettings;

typedef struct M676LightWork {
    /* Global light slot and update callback. */
    HU3D_LIGHTID lightId;
    OMOBJ_FUNC callback;
    /* Light position, target, and direction in world space. */
    Vec position, target, direction;
    /* Light color. */
    GXColor color;
} M676LightWork;

typedef struct M676FogWork {
    /* Fog presets, update callback, and current transition factor. */
    M676FogParam *parameters;
    OMOBJ_FUNC callback;
    f32 factor;
} M676FogWork;

typedef struct M676AudioRecord {
    /* Sound handle and effect ID, camera selection, and enabled controls. */
    s32 handle, effect;
    u32 cameraMask;
    s32 panEnabled, volumeEnabled;
    /* World position used to update the sound's pan and volume. */
    Vec *position;
} M676AudioRecord;

typedef struct M676AudioWork {
    /* Shared audio settings, update callback, and active sound records. */
    M676AudioConfig *config;
    OMOBJ_FUNC callback;
    M676AudioRecord records[64];
} M676AudioWork;

typedef struct M676CameraFitParams {
    /* View direction, up direction, field of view in radians, and screen dimensions/margins
     * in pixels. */
    Vec direction, up;
    f32 fov;
    s32 width, height, marginX, marginY;
} M676CameraFitParams;

typedef struct M676CameraFitSphere {
    /* World-space sphere used to fit the camera view. */
    Vec center;
    f32 radius;
} M676CameraFitSphere;

#endif
