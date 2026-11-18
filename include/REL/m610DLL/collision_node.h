#ifndef REL_M610DLL_COLLISION_NODE_H
#define REL_M610DLL_COLLISION_NODE_H

extern "C"
{
#include "dolphin/types.h"
}

#include "REL/framework/sphere.h"

// A collision sphere in the module's list of live spheres.
struct CollisionNode : Sphere
{
    s32 category;
    void *owner;
    u8 active;
    CollisionNode *next;

    CollisionNode(s32 category, void *owner);
    ~CollisionNode();
    void fn_1_BBC0();
    void fn_1_BBCC();
};

#endif
