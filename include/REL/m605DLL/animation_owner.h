#ifndef REL_M605DLL_ANIMATION_OWNER_H
#define REL_M605DLL_ANIMATION_OWNER_H

#include "dolphin/mtx.h"
#include "REL/framework/hu_archive.h"
#include "REL/framework/singleton.h"
struct Octree;

extern "C" {
#include "game/sprite.h"
}

#include "REL/framework/owned_pointer.h"

// Owns the minigame's shared effect animations and their archive.
struct AnimationOwner : MinigameSingleton<AnimationOwner> {
    OwnedPointer<Octree> collisionTree;
    ANIMDATA *animations[4];

    AnimationOwner();
    ~AnimationOwner();
};

class OrangeParticles;
OrangeParticles *fn_1_690(const AnimationOwner *source);

class PlayerCharacter;

PlayerCharacter *fn_1_5D4(const OwnedPointer<Octree> *collisionTree, s32 playerIndex);

class GameObject;
void fn_1_6F4(Vec *position);
GameObject *fn_1_794();

#endif
