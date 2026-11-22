// Declares the common renderer for particle groups.

#ifndef REL_M673DLL_PARTICLE_GROUP_H
#define REL_M673DLL_PARTICLE_GROUP_H
#include "REL/framework/game_object2.h"
#include "REL/m673DLL/collision_body.h"
#include "game/animdata.h"

// Provides the shared render object for a group of particles.
struct ParticleGroup : RenderObject {
    ANIMDATA *texture; // Texture shared by this group.
    Vec position; // Origin of the group.
    s32 particleCount; // Number of particles in the group.
    // Initializes the render layer, animation resource, position, and item count.
    ParticleGroup(ANIMDATA *value, s32 count, const Position3 &initial);
    // Releases the base particle render object.
    virtual ~ParticleGroup();
};

#endif
