// Declares the colliding particle group and its emission controls.

#ifndef REL_M673DLL_COLLIDING_PARTICLES_H
#define REL_M673DLL_COLLIDING_PARTICLES_H
#include "REL/m673DLL/particle_group.h"
#include "REL/m673DLL/collision_body.h"
#include <vector>
struct CollidingParticle;

// Spawns and draws colliding particles.
struct BurstEmitter : ParticleGroup {
    Vec offset; // Position of the particle group relative to the actor.
    f32 baseAngle; // Heading used to orient particle emission.
    f32 oscillationAngle; // Angle used for the sweep across the base heading.
    u32 elapsedFrames; // Frames since particle emission started.
    u8 active; // Enables particle updates and timed spawns for the group.
    std::vector<CollidingParticle *> particles; // Particles owned and drawn by the group.
    // Creates the colliding-particle group at the supplied position.
    BurstEmitter(ANIMDATA *animation, int count, int layer, Position3 initialPosition);
    // Destroys the colliding particles and releases their vector.
    virtual ~BurstEmitter();
    // Updates particles and spawns them in sequence while the group is active.
    virtual void update();
    // Draws the active colliding particle billboards.
    virtual void draw(Mtx *matrix);
};

// Stops further spawns and freezes existing particles. Individually active particles
// can still be drawn at their last positions.
void fn_1_406C(BurstEmitter *emitter);
// Starts directional emission of colliding particles if it is inactive.
void fn_1_3F74(BurstEmitter *emitter, f32 angle);

// Returns the colliding particle animation from the scene resources.
ANIMDATA *fn_1_4078();

#endif
