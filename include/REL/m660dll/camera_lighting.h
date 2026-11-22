/* Camera and light records shared by the Boo'd Off the Stage module. */
#ifndef REL_M660_CAMERA_LIGHTING_H
#define REL_M660_CAMERA_LIGHTING_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "dolphin/gx.h"

typedef struct M660LightState {
    s16 id;
    Vec position;
    Vec orientation;
    Vec aim;
    Vec direction;
    f32 range;
    GXColor color;
} M660LightState;

/* Shadow camera transform and orbit settings. */
typedef struct M660ShadowCamera {
    Vec cameraPosition;
    f32 elevation;
    f32 azimuth;
    f32 unk14; /* The game's code never reads this field. */
    Vec target;
    u8 unk24[12]; /* No code names this field. */
    Vec cameraUp;
    u8 unk3C[12]; /* No code names this field. */
    f32 orbitDistance;
    f32 shadowOpacity;
} M660ShadowCamera;

extern M660LightState lbl_1_bss_24F4;
extern M660ShadowCamera lbl_1_bss_24A4;

#endif
