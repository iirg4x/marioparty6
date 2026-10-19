/* Shared camera, lighting, fog, and positional-audio state. */
#ifndef M626_SUPPORT_H
#define M626_SUPPORT_H
#include "REL/m626Dll/setup.h"
#include "game/hu3d.h"

typedef void (*M626CameraCallback)(OMOBJ *, u32, s32);

typedef struct M626LightWork_s {
    HU3D_LIGHTID lightId;
    OMOBJ_FUNC callback;
    Vec position, target, direction;
    GXColor color;
} M626LightWork;

typedef struct M626FogWork_s {
    M626FogParameters *parameters;
    OMOBJ_FUNC callback;
    f32 factor;
} M626FogWork;

typedef struct M626CameraEntry_s {
    s32 active, motion, model;
    OM_CAMERA_VIEW view, previousView;
    Vec position, up, target;
    f32 fov, nearPlane, farPlane, aspect;
    f32 viewportX, viewportY, viewportWidth, viewportHeight;
    u8 unknown_88_to_A7[32];
} M626CameraEntry;

typedef struct M626CameraWork_s {
    u32 cameras;
    f32 shadowRadius, shadowFov;
    Vec shadowDirection;
    GXColor shadowColor;
    M626CameraCallback callback;
    M626CameraEntry entries[16];
} M626CameraWork;

typedef struct M626CameraFitParams_s {
    Vec direction, up;
    f32 fov;
    s32 width, height, marginX, marginY;
} M626CameraFitParams;

typedef struct M626CameraFitSphere_s {
    Vec center;
    f32 radius;
} M626CameraFitSphere;

typedef struct M626AudioEntry_s {
    s32 handle, effect;
    u32 cameras;
    s32 updatePan, updateVolume;
    Vec *position;
} M626AudioEntry;

typedef struct M626AudioWork_s {
    M626AudioParameters *config;
    OMOBJ_FUNC callback;
    M626AudioEntry entries[64];
} M626AudioWork;

extern s32 lbl_1_bss_38[80];
extern s32 lbl_1_bss_178[80];
extern OMOBJ *lbl_1_bss_2B8, *lbl_1_bss_2BC, *lbl_1_bss_2C0, *lbl_1_bss_2C4, *lbl_1_bss_2C8;
extern Vec lbl_1_data_138;
extern GXColor lbl_1_data_144;
#endif
