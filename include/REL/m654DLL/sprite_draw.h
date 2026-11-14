#ifndef M654_SPRITE_DRAW_H
#define M654_SPRITE_DRAW_H

#include "REL/m654DLL/sprite_parameters.h"
#include "dolphin/gx.h"
void fn_1_8314(SpriteParameters *entry);
void fn_1_81B4();
// The engine sprite header leaves this declaration commented out.
extern "C" void HuSprTexLoad(ANIMDATA *anim, s16 bmpNo, s16 texMapId, GXTexWrapMode wrapS,
                  GXTexWrapMode wrapT, GXTexFilter filter);

#endif
