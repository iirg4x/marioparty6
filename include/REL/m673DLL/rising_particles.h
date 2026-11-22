// Declares the rising particles and their renderer.

#ifndef REL_M673DLL_RISING_PARTICLES_H
#define REL_M673DLL_RISING_PARTICLES_H
#include "REL/m673DLL/particle_group.h"
#include <vector>

// Stores the position, velocity, and lifetime state of a rising particle.
struct EmitterParticle {
    Vec position; // Current particle position.
    Vec direction; // Current movement direction.
    f32 rotation; // Rotation applied to the quad.
    f32 width; // Unscaled width of the quad.
    f32 height; // Unscaled height of the quad.
    f32 scale; // Multiplier applied to the quad dimensions.
    f32 speed; // Distance traveled per update.
    f32 alpha; // Opacity before the group multiplier.
    u32 countdown; // Waiting frames before drawing and fade-in begin.
    // 0 waits, 1 fades in without moving, 2 rises after alpha reaches 255,
    // and 3 is finished and inactive.
    u32 state;
    u8 active; // Controls whether the particle is drawn.
};

// Updates and draws the rising particles as one group.
struct ParticleEmitter : ParticleGroup {
    u8 stopWhenExpired; // Keeps expired particles from restarting.
    f32 alphaScale; // Multiplier applied to each particle's opacity.
    std::vector<EmitterParticle> particles; // Particle records updated and drawn by the group.
    u8 enabled; // Enables drawing for the group.
    void (ParticleEmitter::*callback)(); // The constructor clears this callback; no code calls it.
    // Creates the rising-particle group and assigns each particle its first state.
    ParticleEmitter(ANIMDATA *texture, s32 particleCount, s32 layer,
                    const Position3 &position);
    // Destroys the rising particles and releases their vector.
    virtual ~ParticleEmitter();
    // Resets a particle for reuse by the rising particle group.
    static void initParticle(EmitterParticle *particle);
    // Advances particle delays, motion, and recycling.
    virtual void update();
    // Draws the enabled particles as textured billboards.
    virtual void draw(Mtx *matrix);
};

// Randomizes the particle position and rotation and sets its initial motion and size.
void fn_1_2B34(ParticleEmitter *, EmitterParticle *parameters);

// Sets the rising particle group's enabled flag.
void fn_1_3588(ParticleEmitter *emitter, u8 enabled);

#endif
