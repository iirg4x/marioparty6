/* Particle requests, transforms and movement toward the score boxes. */
#ifndef REL_M676DLL_PARTICLE_TYPES_H
#define REL_M676DLL_PARTICLE_TYPES_H

#include "REL/m676Dll/collision_types.h"

typedef struct M676ParticleRecord {
    /* Effect kind, state-machine phase, and flight blend or emission countdown. */
    s32 type;
    s32 state;
    f32 timer;
    /* No module function reads these bytes; remainingEmissions begins at offset 0x10. */
    u8 unk0C, unk0D, unk0E, unk0F;
    /* Remaining type-1 emissions (other types only write it), model slot, player,
     * and lane-sector indices. */
    s32 remainingEmissions;
    s32 modelId;
    s32 playerIndex;
    s32 areaIndex;
    /* Particle position, rotation in degrees, and their per-frame velocities. */
    Vec position;
    Vec rotation;
    Vec velocity;
    Vec rotationVelocity;
    /* Uniform model scale, vertical offset, and effect-specific scalar. */
    f32 scale;
    f32 heightOffset;
    f32 storedValue;
    /* Prior and projected positions, followed by collision state. */
    Vec previousPosition;
    Vec projectedPosition;
    M676CollisionEntry collision;
} M676ParticleRecord;

#endif
