/* Quaternion arcs align one game direction with another. */
#ifndef REL_FRAMEWORK_MISC_ROTARC_H
#define REL_FRAMEWORK_MISC_ROTARC_H

extern "C" {
#include "dolphin/math.h"
#include "dolphin/mtx.h"
}

#include "REL/framework/tri_colli.h"

/* Writes the rotation from start to end after normalizing both directions. */
void makeRotationArc(Quaternion *rotation, const Vec *start, const Vec *end);

#endif
