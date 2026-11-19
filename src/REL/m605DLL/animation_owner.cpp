#include "REL/m605DLL/particle_objects.h"
#include "dolphin/types.h"
#include <cmath>

extern "C" {
#include "game/data.h"
#include "game/memory.h"
#include "game/sprite.h"
}
#include "REL/m605DLL/module_globals.h"

#include "REL/m605DLL/animation_owner.h"
#include "REL/m605DLL/board_view.h"
#include "REL/m605DLL/particle_effect_object.h"
#include "REL/framework/octtree_man.h"
#include "REL/m605DLL/player_character.h"
#include "REL/framework/tri_colli.h"
#include <functional>
#include <algorithm>

// Loads and locks a sprite animation from its data number.
ANIMDATA *fn_1_FC(unsigned long dataNum)
{
    ANIMDATA *anim = HuSprAnimDataRead(dataNum);
    HuSprAnimLock(anim);
    return anim;
}

// Creates the scene collision tree and loads the four particle animations.
AnimationOwner::AnimationOwner()
    : collisionTree(new Octree(new HuArchive(DATANUM(0x400000, 23))))
{
    lbl_1_bss_2C = collisionTree;

    animations[0] = fn_1_FC(DATANUM(0x400000, 24));
    animations[1] = fn_1_FC(DATANUM(0x400000, 25));
    animations[2] = fn_1_FC(DATANUM(0x400000, 26));
    animations[3] = fn_1_FC(DATANUM(0x400000, 27));
}

// Releases the four locked particle animations.
AnimationOwner::~AnimationOwner()
{
    std::for_each(animations, animations + 4, std::ptr_fun(HuSprAnimKill));
}

// Creates a player with the corresponding configuration and collision tree.
PlayerCharacter *fn_1_5D4(const OwnedPointer<Octree> *collisionTree, s32 playerIndex)
{
    TriangleCollision *collision;
    PlayerConfigSelection *configuration;

    collision = new TriangleCollision(*collisionTree);
    configuration = new PlayerConfigSelection(playerIndex, 2);
    return new PlayerCharacter(configuration, collision, playerIndex);
}

// Creates the particle effect that uses the fourth animation.
OrangeParticles *fn_1_690(const AnimationOwner *source)
{
    OrangeParticles *effect = new OrangeParticles(source->animations[3]);
    return effect;
}

// Creates a particle object at the supplied position.
void fn_1_6F4(Vec *position)
{
    new ParticleGameObject(AnimationOwner::getChecked().animations[0], position);
}

// Creates the paired particle object from the loaded animations.
GameObject *fn_1_794()
{
    AnimationOwner *stateOwner;
    GameObject *resource;
    stateOwner = &AnimationOwner::getChecked();
    resource = new ParticlePairGameObject(stateOwner->animations[1], stateOwner->animations[0]);
    return resource;
}
