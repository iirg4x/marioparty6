// Implements the particle models and their per-frame updates.

#ifndef REL_SEER_TERROR_PARTICLE_MODEL_EFFECT_H
#define REL_SEER_TERROR_PARTICLE_MODEL_EFFECT_H

#include "REL/framework/game_object.h"

extern "C" {
#include "game/hu3d.h"
}

// Owns the particle model and dispatches its update hooks.
class ParticleModelEffect : public GameObject {
  public:
    ParticleModelEffect(ANIMDATA *animation, const Vec &position, int capacity);
    virtual ~ParticleModelEffect();
    virtual void update();
    virtual void initializeParticle(HU3D_PARTICLE_DATA *entry);
    virtual void updateParticles(HU3D_MODEL *model, HU3D_PARTICLE *list, Mtx matrix);
    void frameCallback();

    void fn_1_434C();

    HU3D_MODELID modelId;
    unsigned char unk12[6]; /* No code names this field. */
    // Engine particle list attached to the model.
    HU3D_PARTICLE *particle;
    // The game's code never reads this field.
    Vec unk1C;
    // Whether the update should delete the effect.
    unsigned char finished;
    // The game's code never reads this field.
    float unk2C;
    // The game's code never reads this field.
    int unk30;
    // Counter used by the derived effect for frames or finished particles.
    int counter;
};

// Moves and fades a burst of gray particles.
// Created by Golden Koopa, Dossun falls, Togezoh falls, Meteoric fall, Bubble falls, Bomb-hey
// falls, and Koopa-bomb.
class GrayImpactParticles : public ParticleModelEffect {
  public:
    GrayImpactParticles(ANIMDATA *animation, const Vec &position, int count, float initialScale,
                        float launchSpeedRange, int fadeStep);
    virtual ~GrayImpactParticles();
    virtual void update();
    virtual void initializeParticle(HU3D_PARTICLE_DATA *entry);
    virtual void updateParticles(HU3D_MODEL *model, HU3D_PARTICLE *list, Mtx matrix);
    static void updateModelParticles(HU3D_MODEL *model, HU3D_PARTICLE *list, Mtx matrix);

    int particleCount;
    unsigned char unk3C[0x14]; /* No code names this field. */
    float scaleStart;
    float velocityRange;
    unsigned char unk58[8]; /* No code names this field. */
    int alphaStep;
    unsigned char unk64[4]; /* No code names this field. */
    void (ParticleModelEffect::*callback)();
};

// Moves, grows and fades a burst of yellow particles.
// Created by Tarai falls and Togezoh falls.
class YellowBurstParticles : public ParticleModelEffect {
  public:
    YellowBurstParticles(ANIMDATA *animation, const Vec &position, int count, float initialScale,
                         float launchSpeedRange);
    virtual ~YellowBurstParticles();
    virtual void update();
    virtual void initializeParticle(HU3D_PARTICLE_DATA *entry);
    virtual void updateParticles(HU3D_MODEL *model, HU3D_PARTICLE *list, Mtx matrix);
    static void updateModelParticles(HU3D_MODEL *model, HU3D_PARTICLE *list, Mtx matrix);

    unsigned char unk38[4]; /* No code names this field. */
    unsigned char unk3C[0x14]; /* No code names this field. */
    float scaleStart;
    float velocityRange;
    unsigned char unk58[8]; /* No code names this field. */
    unsigned char unk60[4]; /* No code names this field. */
    unsigned char unk64[4]; /* No code names this field. */
    void (ParticleModelEffect::*callback)();
};

// Moves and fades tan particles with a wider vertical velocity range.
// Created by Wan-wan chase and Coins stolen.
class TanBurstParticles : public ParticleModelEffect {
  public:
    TanBurstParticles(ANIMDATA *animation, const Vec &position, int count, float initialScale,
                      float launchSpeedRange, int fadeStep);
    virtual ~TanBurstParticles();
    virtual void update();
    virtual void initializeParticle(HU3D_PARTICLE_DATA *entry);
    virtual void updateParticles(HU3D_MODEL *model, HU3D_PARTICLE *list, Mtx matrix);
    static void updateModelParticles(HU3D_MODEL *model, HU3D_PARTICLE *list, Mtx matrix);

    int particleCount;
    unsigned char unk3C[0x14]; /* No code names this field. */
    float scaleStart;
    float velocityRange;
    unsigned char unk58[8]; /* No code names this field. */
    int alphaStep;
    unsigned char unk64[4]; /* No code names this field. */
    void (ParticleModelEffect::*callback)();
};

// Grows and shrinks rising particles as they age.
// Created by Kinoko get.
class RisingYellowParticles : public ParticleModelEffect {
  public:
    RisingYellowParticles(ANIMDATA *animation, const Vec &position, int count, float peakScale);
    virtual ~RisingYellowParticles();
    virtual void update();
    virtual void initializeParticle(HU3D_PARTICLE_DATA *entry);
    virtual void updateParticles(HU3D_MODEL *model, HU3D_PARTICLE *list, Mtx matrix);
    static void updateModelParticles(HU3D_MODEL *model, HU3D_PARTICLE *list, Mtx matrix);

    int particleCount;
    unsigned char unk3C[0x14]; /* No code names this field. */
    float maximumScale;
    unsigned char unk54[4]; /* No code names this field. */
    unsigned char unk58[8]; /* No code names this field. */
    unsigned char unk60[4]; /* No code names this field. */
    unsigned char unk64[4]; /* No code names this field. */
    void (ParticleModelEffect::*callback)();
};

#endif
