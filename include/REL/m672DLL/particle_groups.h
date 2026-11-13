#ifndef REL_M672DLL_PARTICLE_GROUPS_H
#define REL_M672DLL_PARTICLE_GROUPS_H

extern "C" {
#include "dolphin.h"
#include "game/animdata.h"
}
#include "REL/framework/game_object2.h"
#include "REL/m672DLL/collision_object.h"

/* A drawn object made of equal particles that share one texture. */
struct ParticleGroup : RenderObject {
    ANIMDATA *texture;
    EffectVector position;
    int particleCount;
    ParticleGroup(ANIMDATA *value, int count, const EffectVector &point);
    // Performs no additional work when a particle group is destroyed.
    virtual ~ParticleGroup() {}
};

struct RisingParticle {
    EffectVector position;
    EffectVector direction;
    float rotation;
    float width;
    float height;
    float scale;
    float speed;
    float alpha;
    unsigned int countdown;
    int state;
    unsigned char active;
};

struct RisingParticles : ParticleGroup {
    unsigned char stopWhenExpired;
    std::vector<RisingParticle> particles;
    void (RisingParticles::*callback)();
    RisingParticles(ANIMDATA *value, int count, int layer, EffectVector point);
    virtual ~RisingParticles();
    virtual void update();
    virtual void draw(Mtx *);
    void initParticle(RisingParticle *particle);
    // Returns the rising-particle list.
    const std::vector<RisingParticle> &list() const { return particles; }
    // Copies the supplied point into the group's position.
    void setPosition(EffectVector point) {
        position.value.x = point.value.x;
        position.value.y = point.value.y;
        position.value.z = point.value.z;
    }
};

struct CollidingParticle {
    EffectVector position;
    EffectVector velocity;
    float angle;
    float randomAngle;
    float width;
    float height;
    float rotation;
    float rotationStep;
    float speed;
    float alpha;
    unsigned int timer;
    int state;
    unsigned char active;
};

struct CollidingParticles : ParticleGroup {
    Vec offset;
    float emitAngle;
    float angle;
    unsigned char started;
    unsigned char stopped;
    std::vector<CollidingParticle> particles;
    std::vector<CollisionObject *> targets;
    void (CollidingParticles::*callback)();
    // Copies the supplied point into the collision offset.
    void setPosition(EffectVector point) {
        offset.x = point.value.x;
        offset.y = point.value.y;
        offset.z = point.value.z;
    }
    CollidingParticles(ANIMDATA *value, int count, int layer, EffectVector point);
    virtual ~CollidingParticles();
    virtual void update();
    virtual void draw(Mtx *);
    void initParticle(CollidingParticle *particle);
    // Returns the colliding-particle list.
    const std::vector<CollidingParticle> &list() const { return particles; }
};

struct RingParticle {
    Vec position;
    float width;
    float height;
    float scale;
    float bobAngle;
    float orbitAngle;
    float bobStep;
    float orbitStep;
    float orbitRadius;
    float spin;
};

struct RingParticles : ParticleGroup {
    float ringWidth, ringDepth, ringScale, scaleOffset, pulseAngle, ringAngle, alpha;
    unsigned char active, fadingOut;
    int fadeState;
    ANIMDATA *secondTexture;
    std::vector<RingParticle> items;
    RingParticles(ANIMDATA *, ANIMDATA *, int, int, EffectVector);
    virtual ~RingParticles();
    virtual void update();
    virtual void draw(Mtx *);
    // Returns the ring-particle list.
    const std::vector<RingParticle> &list() const { return items; }
};

extern "C" {
void fn_1_3350(CollidingParticles *work);
void fn_1_3568(CollidingParticles *work);
void fn_1_3810(RingParticles *state);
void fn_1_3928(RingParticles *state, RingParticle *vector, int index);
void fn_1_48BC(RingParticles *);
void fn_1_48C8(RingParticles *);
}

#endif
