#ifndef REL_M603DLL_SPRITE_FADE_H
#define REL_M603DLL_SPRITE_FADE_H

#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
extern "C" {
#include "game/sprite.h"
}

/* Store a sprite-group handle and its screen position. */
struct FadeSpriteGroup {
    HUSPR_GROUPID groupId;
    f32 x, y;
};

/* Fade four sprite groups in response to game requests. */
class SpriteFade : public RequestObject, public MinigameSingleton<SpriteFade> {
public:
    u8 reverse;
    u8 transitionState;
    s32 initializedValue;
    f32 level;
    FadeSpriteGroup groups[4];
    SpriteFade(const s32 *);
    virtual ~SpriteFade();
    virtual void flagEnabled(int);
    virtual void update();
    void hold();
    void advance();
};

#endif
