#ifndef REL_M663DLL_EFFECTS_H
#define REL_M663DLL_EFFECTS_H

#include "dolphin/types.h"
#include "game/hu3d.h"
#include "dolphin/gx.h"

typedef struct M663EffectParameters {
    u32 unk00; /* The game's code never reads this field. */
    f32 unk04[3]; /* The game's code never reads this field. */
    GXColor color;
    u8 unk14[4]; /* The game's code never reads this field. */
    f32 velocityScale[3];
    f32 unk24[2]; /* The game's code never reads this field. */
    f32 verticalPosition;
    f32 verticalScale;
    f32 opacityChange;
    f32 unk38; /* The game's code never reads this field. */
} M663EffectParameters;

s16 fn_1_5094(s16 modelNumber, s16 ownerIndex, Vec position, Vec velocity, f32 alpha, s16 state, M663EffectParameters *parameters);
void fn_1_5294(HU3D_MODEL *model, HU3D_PARTICLE *context, Mtx matrix);
void fn_1_54C8(HU3D_MODEL *model, HU3D_PARTICLE *context, Mtx matrix);
void fn_1_4FA8(void);

#endif
