#include "REL/framework/game_object_man.h"
/* Collision shapes: every shape built registers itself in one module-wide list. */
#include "dolphin/mtx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "REL/m672DLL/collision_object.h"

std::vector<CollisionShape *> collisionShapes;

// Stores the radius and registers this object in the module-wide collision-shape list.
CollisionObject::CollisionObject(float radius) : radius(radius)
{
    collisionShapes.push_back(this);
}
