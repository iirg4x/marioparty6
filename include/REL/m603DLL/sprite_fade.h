#ifndef REL_M603DLL_SPRITE_FADE_H
#define REL_M603DLL_SPRITE_FADE_H

#include "REL/m603DLL/callback_owner.h"
#include "REL/framework/minigame_singleton.h"
extern "C" {
#include "game/sprite.h"
}

struct FadeSpriteGroup {
    HUSPR_GROUPID groupId;
    f32 x, y;
};

class SpriteFade : public CallbackOwner, public MinigameSingleton<SpriteFade> {
public:
    u8 reverse;
    u8 transitionState;
    s32 initializedValue;
    f32 level;
    FadeSpriteGroup groups[4];
    SpriteFade(const s32 *);
    virtual ~SpriteFade();
    virtual void update(unsigned long);
    virtual void extra();
    void hold();
    void advance();
};

#endif
