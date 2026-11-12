/* Sprite texture upload shared by sprite and model renderers. */
#ifndef GAME_SPRPUT_H
#define GAME_SPRPUT_H
#include "game/animdata.h"
#include "dolphin/gx.h"
#ifdef __cplusplus
extern "C" {
#endif
void HuSprTexLoad(ANIMDATA *anim, s16 bmpNo, s16 texMapId, GXTexWrapMode wrapS,
                  GXTexWrapMode wrapT, GXTexFilter filter);
#ifdef __cplusplus
}
#endif
#endif
