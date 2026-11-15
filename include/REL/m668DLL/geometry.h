#ifndef REL_M668DLL_GEOMETRY_H
#define REL_M668DLL_GEOMETRY_H

#include "humath.h"

typedef struct {
    HuVecF xyz; /* Bound center or normal vector, in world coordinates. */
    f32 margin; /* Plane offset or bound radius used by the projection tests. */
} M668VecBound;

s32 fn_1_1844(const M668VecBound *, const M668VecBound *);

#endif
