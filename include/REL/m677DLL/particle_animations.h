// Loads and supplies the sprite animations used by particle models.

#ifndef REL_SEER_TERROR_PARTICLE_ANIMATIONS_H
#define REL_SEER_TERROR_PARTICLE_ANIMATIONS_H
#include "REL/framework/singleton.h"
#include "game/animdata.h"
#include <vector>

// Holds the sprite animations used when creating particle effects.
struct ParticleAnimations : MinigameSingleton<ParticleAnimations> {
    std::vector<ANIMDATA *> animations;
    ParticleAnimations();
    ~ParticleAnimations();
    void createGrayImpactParticles(Vec *position, int count, float scaleStart, float velocityRange,
                                   int alphaStep);
    void createYellowBurstParticles(Vec *position, int count, float scaleStart,
                                    float velocityRange);
    void createTanBurstParticles(Vec *position, int count, float scaleStart, float velocityRange,
                                 int alphaStep);
    void createRisingYellowParticles(Vec *position, int count, float peakScale);
};
#endif
