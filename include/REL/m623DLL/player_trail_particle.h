#ifndef REL_M623DLL_PLAYER_TRAIL_PARTICLE_H
#define REL_M623DLL_PLAYER_TRAIL_PARTICLE_H

#include "REL/framework/list.h"
#include "REL/framework/vector3.h"

// Owns the trail particles emitted by a player.
struct PlayerTrailParticles : ListNode<PlayerTrailParticles> {
    PlayerTrailParticles(const Vector3 *position, int viewIndex);
    ~PlayerTrailParticles();
    s16 modelId;
    // The game's code never reads this field.
    s32 unk0C;
};

#endif
