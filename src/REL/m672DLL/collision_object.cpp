#include "REL/framework/game_object_man.h"
/* Collision shapes: every shape built registers itself in one module-wide list. */
#include "dolphin/mtx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "REL/m672DLL/collision_object.h"

extern "C" {
std::vector<CollisionShape *> lbl_1_bss_C;
}

CollisionObject::CollisionObject(float radius) : radius(radius)
{
    lbl_1_bss_C.push_back(this);
}
