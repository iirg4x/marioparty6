/* Light, fog, camera, and positional-audio state shared by the scene utilities. */
#ifndef REL_M674DLL_SCENE_H
#define REL_M674DLL_SCENE_H
#include "game/object.h"
typedef void (*CameraCallback)(OMOBJ *, u32, s32);
typedef struct M674LightWork {
    HU3D_LIGHTID lightId; /* Global light controlled by this object. */
    OMOBJ_FUNC callback;  /* Per-frame light update callback. */
    Vec position;         /* Light position in world coordinates. */
    Vec target;           /* Point the light is aimed toward. */
    Vec direction;        /* Direction supplied to the light setter, which normalizes its own
                           * copy. */
    GXColor color;        /* Light color including alpha. */
} M674LightWork;
typedef struct M674FogParam {
    s32 layer;       /* Render layer receiving this fog. */
    u32 cameras;     /* Camera mask receiving this fog. */
    GXFogType type;  /* Fog curve used by the renderer. */
    f32 start, end;  /* Near and far distances for the fog curve. */
    GXColor color;   /* Fog color. */
} M674FogParam;
typedef struct M674FogWork {
    M674FogParam *parameters; /* Per-layer fog settings. */
    OMOBJ_FUNC callback;      /* Per-frame fog transition callback. */
    f32 factor;               /* Current blend factor between fog states. */
} M674FogWork;
typedef struct M674CameraEntry {
    s32 active, motion, model; /* Use state, active motion, and attached model ID. */
    OM_CAMERA_VIEW view, previousView; /* Current and target camera views. */
    Vec position, up, target; /* Camera position, up direction, and look target. */
    f32 fov, nearPlane, farPlane, aspect; /* Perspective projection settings. */
    f32 viewport[4]; /* Viewport x, y, width, and height in pixels. */
    f32 scissor[4];  /* Scissor x, y, width, and height in pixels. */
    s32 phase, timer; /* Camera transition phase and elapsed frames. */
    u8 unidentifiedCameraData[24]; /* Uninterpreted bytes within this camera record. */
} M674CameraEntry;
typedef struct M674CameraWork {
    u32 cameras; /* Camera mask used by this camera object. */
    f32 shadowRadius, shadowFov; /* Shadow volume radius and camera field of view. */
    Vec shadowDirection; /* Direction used for the scene shadow volume. */
    GXColor shadowColor; /* Color applied to the shadow projection. */
    CameraCallback callback; /* Per-frame camera transition callback. */
    M674CameraEntry entries[16]; /* Transition state indexed by camera slot. */
    s32 shadowInitialized; /* Set after shared shadow settings are installed. */
    u8 unidentifiedCameraData[4]; /* Uninterpreted bytes within the camera work data. */
} M674CameraWork;
typedef struct M674AudioRange {
    u32 cameraMask; /* Cameras used to project the sound position. */
    int panMin, panMax; /* Pan limits at the sides of the camera view. */
    int volumeMin, volumeMiddle, volumeMax; /* Volume levels across the range. */
    f32 distanceFar, distanceMiddle, distanceNear; /* Distance breakpoints in world units. */
} M674AudioRange;
typedef struct M674AudioRecord {
    int handle, effect; /* Active sound handle and effect resource ID. */
    u32 cameraMask; /* Cameras used for positional audio. */
    int panEnabled, volumeEnabled; /* Whether camera position changes pan or volume. */
    Vec *position; /* World position followed while the sound is active. */
} M674AudioRecord;
typedef struct M674AudioState {
    M674AudioRange *ranges; /* Positional audio settings for the scene. */
    OMOBJ_FUNC callback; /* Per-frame sound position update callback. */
    M674AudioRecord records[64]; /* Active positional sound instances. */
} M674AudioState;
typedef struct CameraFitParams {
    Vec direction; /* Direction from the fitted camera toward its subject. */
    Vec up; /* Camera up direction. */
    float fov; /* Vertical field of view in radians. */
    s32 width, height, marginX, marginY; /* View size and edge margins in pixels. */
} CameraFitParams;
/* World-space sphere to fit. */
typedef struct CameraFitSphere { Vec center; float radius; } CameraFitSphere;
void fn_1_F8C(OMOBJMAN *manager);
void fn_1_1054(OMOBJMAN *manager, Vec *position, Vec *target, GXColor *color);
void fn_1_173C(OMOBJMAN *manager, u32 cameras, f32 fov, f32 nearPlane,
    f32 farPlane, f32 shadowRadius, OM_CAMERA_VIEW *view);
void fn_1_2398(CameraCallback callback);
void fn_1_3308(OMOBJMAN *manager, M674AudioRange *ranges);
f32 fn_1_4E18(Vec *first, Vec *second);
void fn_1_4F84(Point3d *start, Point3d *end, f32 factor, Point3d *out);
#endif
