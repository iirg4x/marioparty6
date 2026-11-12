#ifndef REL_M604DLL_ANIMATION_MENU_H
#define REL_M604DLL_ANIMATION_MENU_H
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
extern "C" {
#include "game/sprite.h"
}
struct ParticleAnimations : RequestObject, MinigameSingleton<ParticleAnimations> {
    ANIMDATA *animations[1];
    ParticleAnimations();
    virtual ~ParticleAnimations();
    virtual void flagEnabled(int bit);
    virtual void update();
    ANIMDATA *getAnimation() { return animations[0]; }
};
#endif
