/* Screen distortion and particle models used by Solo menu animations. */
#ifndef REL_MDSINGDLL_STAGE_EFFECTS_H
#define REL_MDSINGDLL_STAGE_EFFECTS_H

#include "dolphin/gx.h"
#include "dolphin/types.h"
#include "game/hu3d.h"
#include "game/object.h"

typedef struct MdpartyStageTextureWork {
    /* Two rows of the texture-distortion matrix used by the material callback. */
    HuVecF matrixRow0;
    HuVecF matrixRow1;
    /* Texture-combiner color and blend amount, with components in 0-255. */
    GXColor color;
    /* Accumulated vertical texture offset and its increment per material draw. */
    f32 textureScrollY;
    f32 textureScrollStep;
    /* Updates left before the distortion model is hidden; starts at ninety. */
    f32 remainingFrames;
} MDPARTY_STAGE_TEXTURE_WORK;

extern MDPARTY_STAGE_TEXTURE_WORK lbl_1_bss_1428[2];
extern s32 lbl_1_data_CD8[9];
extern ANIMDATA *lbl_1_bss_1478[9];
extern HU3D_MODELID lbl_1_bss_1426;
extern HU3D_MODELID lbl_1_bss_141E[4];
extern HU3D_MODELID lbl_1_bss_141C;
extern HU3D_MODELID lbl_1_bss_1414[4];
extern HU3D_MODELID lbl_1_bss_1412;
extern HU3D_MODELID lbl_1_bss_140E[2];
extern HU3D_MODELID lbl_1_bss_140A[2];
extern HU3D_MODELID lbl_1_bss_13CE[6][5];
extern s16 lbl_1_bss_13CC;
extern void *lbl_1_bss_13C8;
extern ANIMDATA *lbl_1_bss_13C4;
extern ANIMDATA *lbl_1_bss_13C0;
extern OMOBJ *lbl_1_bss_13BC;
extern HUPROCESS *lbl_1_bss_13B8;

#endif
