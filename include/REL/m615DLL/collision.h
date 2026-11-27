#ifndef M615DLL_COLLISION_H
#define M615DLL_COLLISION_H

#include "REL/m615DLL/geometry.h"

typedef struct M615CollisionData {
    s32 type;
    MinigameTriangle triangle;
    HuVecF normal;
    M615Bounds bounds;
} M615CollisionData;

typedef struct M615CollisionPool {
    s32 count;
    M615CollisionData **entries;
} M615CollisionPool;

void fn_1_9140(M615CollisionPool **poolResult, s32 count);
M615CollisionData *fn_1_9228(M615CollisionPool *pool, s32 index);
M615CollisionData *fn_1_9238(M615CollisionPool *pool, M615Capsule *capsule);

#endif
