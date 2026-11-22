// Declares the minigame's ground sprite draw hook and visibility update.

#ifndef REL_M673DLL_GROUND_SPRITE_H
#define REL_M673DLL_GROUND_SPRITE_H

#include "REL/framework/ground_sprite.h"

// Draws the ground sprite through its model callback.
void fn_1_8718(HU3D_MODEL *model, Mtx *matrix);

// Shows the sprite when the flag equals 1 and hides it for every other value.
void fn_1_8678(GroundSprite *sprite, u8 reset);

#endif
