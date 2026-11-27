/* Texture distortion, scrolling and lifetime state for minigame-menu scene effects. */
#ifndef REL_MDMINIDLL_LAYER_EFFECT_STATE_H
#define REL_MDMINIDLL_LAYER_EFFECT_STATE_H

#include "game/hu3d.h"

#include "REL/minigame_texture_transition.h"

extern MinigameTextureTransition lbl_1_bss_8E8[2];

/* Functions shared by the module's source files. */
void fn_1_23EC4(s16 effectIndex, HuVecF *position, s16 mode);
void fn_1_24D80(HUPROCESS *process);
void fn_1_25BF4(s16 particleNo, Point3d *position, s16 colorNo);
void fn_1_25EF0(s16 particleNo, HuVecF *position, s16 colorNo, s16 mode);
void fn_1_26394(void);
void fn_1_26454(s16 row, Point3d *position, s16 mode, s16 colorIndex);

#endif
