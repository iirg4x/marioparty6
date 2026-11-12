/* Owns the particle model and animation used by a player's effect. */
#ifndef M604_PLAYER_PARTICLE_EFFECT_H
#define M604_PLAYER_PARTICLE_EFFECT_H

#include "dolphin/types.h"
#include "dolphin/mtx.h"
extern "C" {
#include "game/hu3d.h"
#include "game/sprite.h"
}

struct PlayerParticleEffect {
    s16 modelId;
    ANIMDATA *animation;
    HU3D_PARTICLE *particle;
    Vec position;   // where new particles start
    Vec storedVector;  // set, never read
    u8 enabled;     // the particles are drawn and renewed
    u8 finishing;   // no particle is renewed: the model hides once all are gone

    PlayerParticleEffect(const Vec *position, s32 cameraMask);
    ~PlayerParticleEffect();
};

#endif
