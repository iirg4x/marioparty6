#ifndef REL_M623DLL_PARTICLE_OBJECT_H
#define REL_M623DLL_PARTICLE_OBJECT_H
#include "REL/framework/list.h"
#include "REL/framework/vector3.h"
// Owns the particles emitted when a player hits an item.
struct ItemHitParticles : ListNode<ItemHitParticles> {
    ItemHitParticles(Vector3 *position, int viewIndex);
    ~ItemHitParticles();
    s16 modelId;
    // The game's code never reads this field.
    int unk0C;
};
#endif
