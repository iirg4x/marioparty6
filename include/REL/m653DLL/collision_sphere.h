#ifndef REL_M653DLL_COLLISION_SPHERE_H
#define REL_M653DLL_COLLISION_SPHERE_H

#include "dolphin/mtx.h"
#include "REL/framework/sphere.h"

// Sets a sphere's centre.
static inline void setCenter(Sphere *sphere, f32 x, f32 y, f32 z)
{
    sphere->center.x = x;
    sphere->center.y = y;
    sphere->center.z = z;
}

/* A sphere on the module's collision list. Each pair that overlaps is reported with both categories and owners. */
struct CollisionSphere : Sphere {
    s32 category;
    void *owner;
    u8 active;
    CollisionSphere *next;
    CollisionSphere(s32 category, void *owner);
    ~CollisionSphere();
    void deactivate();
    void activate();
};

// Called for each overlapping pair of active spheres, the lower category first.
typedef void (*CollisionCallback)(s32 firstCategory, void *firstOwner, s32 secondCategory,
                                  void *secondOwner);

extern "C" {
extern CollisionSphere *lbl_1_bss_30;
void fn_1_4EFC(CollisionCallback callback);
}

#endif
