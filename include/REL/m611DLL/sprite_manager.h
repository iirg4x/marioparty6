#ifndef REL_M611DLL_SPRITE_MANAGER_H
#define REL_M611DLL_SPRITE_MANAGER_H
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"

extern "C" {
#include "game/sprite.h"
}

// Owns the four sprite animations used by the scene.
struct SpriteManager : RequestObject, MinigameSingleton<SpriteManager> {
    SpriteManager();
    virtual ~SpriteManager();
    virtual void flagEnabled(int);
    virtual void update();
    ANIMDATA *animations[4];
};

#endif
