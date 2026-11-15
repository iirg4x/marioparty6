#ifndef REL_M634DLL_PARTICLE_UPDATE_LAYOUT_H
#define REL_M634DLL_PARTICLE_UPDATE_LAYOUT_H
#include "dolphin/types.h"
#include "game/hu3d.h"
typedef struct {

    u8 unknown00[0x5C]; /* No code names this field. */

    s32 liveCount;
    u8 enabled;
} ParticleUpdateOwner634;
void fn_1_12FD8(ParticleUpdateOwner634 *owner, HU3D_PARTICLE_DATA *particle);
void fn_1_1315C(ParticleUpdateOwner634 *owner, HU3D_PARTICLE_DATA *particle);
#endif
