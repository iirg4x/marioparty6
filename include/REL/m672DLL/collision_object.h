#ifndef REL_M672DLL_COLLISION_OBJECT_H
#define REL_M672DLL_COLLISION_OBJECT_H

#include "dolphin/mtx.h"

/* A position in the world. */
struct EffectVector {
    Vec value;
    void set(const EffectVector &other) {
        value.x = other.value.x;
        value.y = other.value.y;
        value.z = other.value.z;
    }
    void set(double x, double y, double z) {
        value.x = x;
        value.y = y;
        value.z = z;
    }
};

/* A position built from its three coordinates. */
struct EffectPoint : EffectVector {
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
    int unknown0C;
    int unknown10;
    int unknown14;
    int state;
    unsigned char active;
    CollisionShape() : unknown0C(0), unknown10(-1), state(-1), active(0) {}
    virtual ~CollisionShape() {}
    EffectVector &getPosition() { return position; }
    void setPosition(const EffectVector &value) {
        EffectVector &destination = position;
        destination.set(value);
    }
};

/* A sphere of the given radius. */
class CollisionObject : public CollisionShape {
public:
    float radius;
    CollisionObject(float radius);
    virtual ~CollisionObject() {}
};

#endif
