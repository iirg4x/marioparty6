#ifndef REL_M672DLL_COLLISION_OBJECT_H
#define REL_M672DLL_COLLISION_OBJECT_H

#include "dolphin/mtx.h"
#include <vector>

/* Three coordinates used for positions, velocities, and directions. */
struct EffectVector {
    Vec value;
    const Vec &getValue() const;
    // Scale the supplied vector into this value.
    EffectVector &setScaled(const Vec *source, f32 scale) {
        PSVECScale(source, &value, scale);
        return *this;
    }
    // Copy the vector coordinates.
    void set(const EffectVector &other) {
        value.x = other.value.x;
        value.y = other.value.y;
        value.z = other.value.z;
    }
    // Copy the vector coordinates.
    void set(double x, double y, double z) {
        value.x = x;
        value.y = y;
        value.z = z;
    }
};

/* A position built from its three coordinates. */
struct EffectPoint : EffectVector {
    // Initialize the three vector coordinates.
    EffectPoint(double x, double y, double z) {
        value.x = x;
        value.y = y;
        value.z = z;
    }
};

/* Base of the collision shapes. */
class CollisionShape {
public:
    EffectVector position;
    int unidentifiedFirst; // Initialized to zero; its meaning is unknown.
    int unidentifiedSecond; // Initialized to -1; its meaning is unknown.
    int unk18; /* No code names this field. */
    int state;
    unsigned char active;
    // Initialize the collision state and activation flag.
    CollisionShape() : unidentifiedFirst(0), unidentifiedSecond(-1), state(-1), active(0) {}
    // Destroy the collision shape.
    virtual ~CollisionShape() {}
    // Return the collision position.
    EffectVector &getPosition() { return position; }
    // Copy the position coordinates.
    void setPosition(const EffectVector &value) {
        EffectVector &destination = position;
        destination.set(value);
    }
};

extern std::vector<CollisionShape *> collisionShapes;

/* A sphere of the given radius. */
class CollisionObject : public CollisionShape {
public:
    float radius;
    CollisionObject(float radius);
    // Destroy the collision sphere.
    virtual ~CollisionObject() {}
};

#endif
