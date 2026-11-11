#ifndef REL_M672DLL_PARTICLE_GROUPS_H
#define REL_M672DLL_PARTICLE_GROUPS_H

extern "C" {
#include "dolphin.h"
#include "game/animdata.h"
}
#include "REL/framework/game_object2.h"
#include "REL/m672DLL/collision_object.h"
#include "REL/m672DLL/position_bf64.h"


/* A drawn object made of equal particles that share one texture. */
struct ParticleGroup : RenderObject {
    ANIMDATA *texture;
    EffectVector position;
    int particleCount;
    ParticleGroup(ANIMDATA *value, int count, const EffectVector &point);
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
    RisingParticles(ANIMDATA *value, int count, int layer, WorldPosition point);
    virtual ~RisingParticles();
    virtual void update();
    virtual void draw(Mtx *);
    void initParticle(RisingParticle *particle);
    const std::vector<RisingParticle> &list() const { return particles; }
    void setPosition(WorldPosition point) {
        position.value.x = point.value.value.x;
        position.value.y = point.value.value.y;
        position.value.z = point.value.value.z;
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
    void setPosition(WorldPosition point) {
        offset.x = point.value.value.x;
        offset.y = point.value.value.y;
        offset.z = point.value.value.z;
    }
    CollidingParticles(ANIMDATA *value, int count, int layer, WorldPosition point);
    virtual ~CollidingParticles();
    virtual void update();
    virtual void draw(Mtx *);
    void initParticle(CollidingParticle *particle);
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
    RingParticles(ANIMDATA *, ANIMDATA *, int, int, WorldPosition);
    virtual ~RingParticles();
    virtual void update();
    virtual void draw(Mtx *);
    const std::vector<RingParticle> &list() const { return items; }
};

#endif
