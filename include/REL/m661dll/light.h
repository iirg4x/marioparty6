#ifndef M661DLL_LIGHT_H
#define M661DLL_LIGHT_H

#include "dolphin/types.h"
#include "dolphin/mtx.h"

/* Unobserved intervals separate the fields used by the light update. */
typedef struct LightConfig {
    u8 beforeLightId[4];
    s16 lightId;
    s16 modelId;
    u8 beforeType[4];
    s16 type;
    u8 beforePosition[2];
    Point3d position;
    u8 beforeDirection[0x18];
    Point3d direction;
    u8 beforeReferenceDistance[4];
    f32 referenceDistance;
    f32 referenceBrightness;
    f32 cutoff;
} LightConfig;

extern LightConfig lbl_1_bss_1870[3];

#endif
