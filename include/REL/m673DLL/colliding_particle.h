// Declares the colliding particle and its update states.

#ifndef REL_M673DLL_COLLIDING_PARTICLE_H
#define REL_M673DLL_COLLIDING_PARTICLE_H
#include "REL/m673DLL/collision_body.h"
#include "REL/m673DLL/follow_effect.h"
struct CollidingParticle;

// Callback type for a colliding particle's state update.
typedef void (CollidingParticle::*ParticleCallback)();

// Updates a colliding particle and its collision body.
struct CollidingParticle {
    // Creates a collision body and a quad for a colliding particle.
    CollidingParticle(s32, s32);
    // Advances the particle state and updates the collision body and attached quad.
    void fn_1_48C0();
    // Waits for the particle countdown, then activates collision and the next state.
    void fn_1_4980();
    // Leaves the colliding particle unchanged in its idle state.
    void fn_1_49F0();
    // Sets half opacity and enters the moving state.
    void fn_1_49F4();
    // Advances the particle offset along its direction and updates its position.
    void fn_1_4AE8();
    // Releases the collision body and the particle quad.
    ~CollidingParticle();
    // Resets the particle's motion parameters for the supplied angle.
    void fn_1_4334(f32 angle);
    ParticleCallback state; // Current update callback.
    Vec origin; // Position at which the particle starts.
    Vec offset; // Randomized offset applied at spawn.
    Vec direction; // Movement direction of the particle.
    CollisionBody collisionBody; // Body registered for particle collision checks.
    f32 angle; // Initial direction about the vertical axis.
    f32 rotation; // Rotation applied to the particle quad.
    f32 width; // Unscaled width of the quad.
    f32 height; // Unscaled height of the quad.
    f32 scale; // Multiplier applied to both quad dimensions.
    f32 scaleStep; // Increase in scale at each update.
    f32 speed; // Distance advanced along the current direction per update.
    f32 alpha; // Alpha applied to the particle's quad.
    u8 active; // Controls whether the particle is drawn.
    u32 countdown; // Frames remaining before the next state change.
    s32 index; // Index used to stagger particle starts.
    FollowEffect followEffect; // Quad attached to the colliding particle.
    Vec effectPosition; // Position read by the attached quad.
};

// Starts a particle at the supplied position and angle.
void fn_1_4570(CollidingParticle *particle, f32 angle, const Vec *position);

#endif
