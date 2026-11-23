// Loads and supplies the sprite animations used by particle models.

#include "REL/m677DLL/particle_animations.h"
#include "REL/m677DLL/particle_model_effect.h"
#include "datadir_enum.h"

extern "C" {
#include "game/data.h"
#include "game/sprite.h"
}

// Reads and locks a sprite animation for a particle effect.
static ANIMDATA *loadSpriteAnimation(unsigned long dataNum)
{
    ANIMDATA *animation = HuSprAnimDataRead(dataNum);
    HuSprAnimLock(animation);
    return animation;
}

// Loads the five particle animations from the data numbers below.
ParticleAnimations::ParticleAnimations()
{
    static const unsigned long dataNums[] = {
        DATANUM(_m677 << 16, 0x31), DATANUM(_m677 << 16, 0x35), DATANUM(_m677 << 16, 0x33),
        DATANUM(_m677 << 16, 0x34), DATANUM(_m677 << 16, 0x32)
    };
    for (int index = 0; index < 5; ++index) {
        animations.push_back(loadSpriteAnimation(dataNums[index]));
    }
}

// Releases the vector storage and clears the singleton.
ParticleAnimations::~ParticleAnimations()
{
}

// Creates the effect driven by the first animation.
void ParticleAnimations::createGrayImpactParticles(Vec *position, int count,
    float scaleStart, float velocityRange,
                                   int alphaStep)
{
    new GrayImpactParticles(animations[0], *position, count, scaleStart, velocityRange, alphaStep);
}

// Creates the effect driven by the second animation.
void ParticleAnimations::createYellowBurstParticles(Vec *position, int count,
    float scaleStart, float velocityRange)
{
    new YellowBurstParticles(animations[1], *position, count, scaleStart, velocityRange);
}

// Creates the alternate effect driven by the first animation.
void ParticleAnimations::createTanBurstParticles(Vec *position, int count,
    float scaleStart, float velocityRange,
                                   int alphaStep)
{
    new TanBurstParticles(animations[0], *position, count, scaleStart, velocityRange, alphaStep);
}

// Creates the effect driven by the last animation.
void ParticleAnimations::createRisingYellowParticles(Vec *position, int count, float peakScale)
{
    new RisingYellowParticles(animations[4], *position, count, peakScale);
}
