/* Texture distortion, scrolling and lifetime state for minigame-menu scene effects. */
#ifndef REL_MDMINIDLL_LAYER_EFFECT_STATE_H
#define REL_MDMINIDLL_LAYER_EFFECT_STATE_H

#include "dolphin/types.h"

/* The six matrix coefficients are read together by the distortion material hook. */
typedef struct MDMinidllLayerEffectStateView {
    f32 warpSFromS; /* S-input coefficient for the warped S texture coordinate. */
    f32 warpSFromT; /* T-input coefficient for the warped S texture coordinate. */
    f32 warpSOffset; /* Offset coefficient for the warped S texture coordinate. */
    f32 warpTFromS; /* S-input coefficient for the warped T texture coordinate. */
    f32 warpTFromT; /* T-input coefficient for the warped T texture coordinate. */
    f32 warpTOffset; /* Offset coefficient for the warped T texture coordinate. */
    u8 tintChannels[4]; /* Red, green, blue and alpha channels passed to the blend stages. */
    f32 scrollY; /* Vertical warp-texture translation, in texture coordinates. */
    f32 scrollStep; /* Vertical translation added on each material-hook call. */
    f32 remainingUpdates; /* Starts at 90; a negative value ends the distortion effect. */
} MDMinidllLayerEffectStateView;

/* Two state slots are available; the object update callback advances only the first. */
extern MDMinidllLayerEffectStateView lbl_1_bss_8E8[2];

#endif
