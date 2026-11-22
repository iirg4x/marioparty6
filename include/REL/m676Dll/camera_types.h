/* Camera, light, fog, and positional-audio state used by the minigame. */
#ifndef REL_M676DLL_CAMERA_TYPES_H
#define REL_M676DLL_CAMERA_TYPES_H

#include "game/object.h"

typedef struct M676CameraRecord {
    /* Camera-motion activity flag, motion ID, and camera-model ID. */
    s32 active, motion, model;
    /* Current and destination camera transforms. */
    OM_CAMERA_VIEW currentPose, targetPose;
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
    s32 unkA4; /* No code names this field. */
    /* Yaw-oscillation phase in degrees and its amplitude blend during course travel. */
    f32 angle, fade;
    u8 unkB0[8]; /* No code names this field. */
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
    u8 unkBA0[8]; /* No code names this field. */
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
    /* Sound handle, camera selection, and enabled controls. */
    s32 handle;
    s32 unk04; /* The game's code never reads this field. */
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
    s32 width, height;
    s32 margin[2];
} M676CameraFitParams;

typedef struct M676CameraFitSphere {
    /* World-space sphere used to fit the camera view. */
    Vec center;
    f32 radius;
} M676CameraFitSphere;
void fn_1_16A0(OMOBJMAN *manager);
void fn_1_1768(OMOBJMAN *manager, Vec *position, Vec *target, GXColor *color);
void fn_1_18C8(void);
void fn_1_1E50(OMOBJMAN *manager, u32 cameras, float fov, float nearPlane, float farPlane,
               float shadowRadius, OM_CAMERA_VIEW *view);
void fn_1_28BC(s32 camera, Vec *direction, Vec *target, float fov, float radius);
Vec *fn_1_2E90(void);
void fn_1_2EF8(u32 cameras, void *data);
void fn_1_2FB0(u32 cameraMask);
double fn_1_34D4(M676CameraFitParams *params, M676CameraFitSphere *spheres, s32 count, Vec *out);
void fn_1_3A1C(HUPROCESS *process, M676AudioConfig *config);
s32 fn_1_3DD0(Vec *position);
void fn_1_4754(s32 effect, Vec *position);
s32 fn_1_49C8(s32 effect, Vec *position, s32 panEnabled, s32 volumeEnabled);
void fn_1_4F08(int effect, int handle, Vec *position, int panEnabled, int volumeEnabled);
int fn_1_50A8(int dataNum);
s32 fn_1_51B0(s32 model, s32 dataNum);
float fn_1_52C0(float start, float end, float factor);
f32 fn_1_53DC(f32 start, f32 end);
float fn_1_5490(HuVecF *from, HuVecF *to);
float fn_1_552C(Vec *first, Vec *second);
void fn_1_5698(Vec *start, Vec *end, f32 factor, Vec *out);
s32 fn_1_56F0(Vec *start, Vec *end, f32 gravity, f32 initialY, Vec *velocity);
void fn_1_A70(f32 x, f32 y, f32 z, f32 w);
s32 fn_1_1480(s32 resource);
s32 fn_1_1498(void);
s32 fn_1_150C(u32 player);
void fn_1_1588(s32 playerIndex, s32 playerSlot, s32 character);
void fn_1_15C0(unsigned int player);
int fn_1_15E0(void);
void fn_1_1630(unsigned int player);
int fn_1_1650(void);


void fn_1_2AAC(void (*callback)(OMOBJ *, u32, s32));
#endif
