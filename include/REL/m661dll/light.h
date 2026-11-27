#ifndef M661DLL_LIGHT_H
#define M661DLL_LIGHT_H

#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "dolphin/gx.h"

/* Point light state initialized during stage setup and read by the light update. */
typedef struct LightConfig {
    u8 beforeLightId[4];
    s16 lightId;
    s16 modelId;
    s16 unk8; /* The game's code never reads this field. */
    s16 modelLightId;
    s16 type;
    Point3d position;
    u8 unk_1C[0x0C]; /* No code names this field. */
    Point3d unk28; /* The game's code never reads this field. */
    Point3d direction;
    GXColor color;
    f32 referenceDistance;
    f32 referenceBrightness;
    f32 cutoff;
} LightConfig;

/* Sets the spotlight cutoff or the point light attenuation. */
static inline void StageLightSetAttenuation(LightConfig *light)
{
    switch (light->type) {
    case 0:
        break;
    case 1:
        fn_80028C1C(light->modelId, light->lightId, light->referenceDistance,
                           light->referenceBrightness, GX_DA_STEEP);
        break;
    case 2:
        fn_80028AA8(light->modelId, light->lightId, light->cutoff, GX_SP_COS);
        break;
    }
}

extern LightConfig lbl_1_bss_1870[3];

#endif
