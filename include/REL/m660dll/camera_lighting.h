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

typedef struct M660ShadowCamera {
    Vec cameraPosition;
    f32 azimuth;
    f32 elevation;
    f32 unknown14;
    Vec target;
    u8 unknown24[12];
    Vec cameraUp;
    u8 unknown3C[12];
    f32 orbitDistance;
    f32 shadowTransparency;
} M660ShadowCamera;

extern M660LightState lbl_1_bss_24F4;
extern M660ShadowCamera lbl_1_bss_24A4;
typedef struct M660HalfwordQuartet {
    u16 first;
    u16 second;
    u16 third;
    u16 fourth;
} M660HalfwordQuartet;

#endif
