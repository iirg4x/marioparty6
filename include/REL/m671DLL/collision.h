#ifndef REL_M671DLL_COLLISION_H
#define REL_M671DLL_COLLISION_H

#include "REL/m671DLL/motion_position.h"

/* Base of the collision shapes. */
class CollisionShape : public MotionPosition {
public:
    int unknown0C;
    int state;
    Mtx *matrix;
    int relation;
    unsigned char active;
    CollisionShape() : unknown0C(0), state(-1), relation(-1), active(0) {}
    virtual ~CollisionShape() {}
    int getState() { return state; }
    int getRelation() { return relation; }
    unsigned char isActive() { return active; }
};

/* A sphere of the given radius; every one built registers itself in the module-wide list. */
class CollisionObject : public CollisionShape {
public:
    float radius;
    CollisionObject(float radius);
    virtual ~CollisionObject() {}
};

void fn_1_148(void);
void fn_1_360(Vec *normal, Vec *point);
void fn_1_40C(void);
void fn_1_C58(void);

#endif
