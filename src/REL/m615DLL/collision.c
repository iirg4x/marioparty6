/* Collision pools: their allocation, entries and capsule lookups. */
#include "dolphin/types.h"
#include "math.h"
#include "humath.h"
#include "game/memory.h"
#include "include/dolphin/types.h"
#include "game/object.h"
#include "game/gamework.h"
#include "game/mg/actman.h"
#include "game/mg/seqman.h"
#include "game/charman.h"
#include "game/audio.h"
#include "game/gamemes.h"
#include "game/flag.h"
#include "include/game/audio.h"
#include "include/game/data.h"
#include "include/game/memory.h"
#include "include/datadir_enum.h"
#include "include/game/frand.h"
#include "include/string.h"
#include "include/game/gamework.h"
#include "include/game/gamemes.h"

typedef struct M615Bounds {
    HuVecF min;
    HuVecF max;
} M615Bounds;

typedef struct M615Segment {
    HuVecF start;
    HuVecF delta;
} M615Segment;

typedef struct M615Capsule {
    M615Segment segment;
    f32 radius;
} M615Capsule;

typedef struct M615Sphere {
    HuVecF center;
    f32 radius;
} M615Sphere;

typedef struct M615Triangle {
    HuVecF start;
    HuVecF edge1;
    HuVecF edge2;
} M615Triangle;

typedef struct M615CollisionData {
    s32 type;
    M615Triangle triangle;
    HuVecF normal;
    M615Bounds bounds;
} M615CollisionData;

typedef struct M615CollisionPool {
    s32 count;
    M615CollisionData **entries;
} M615CollisionPool;

extern s32 fn_1_1478(M615Bounds *a, M615Bounds *b);

extern f32 fn_1_14E8(HuVecF *point, M615Triangle *triangle, f32 *barycentricS, f32 *barycentricT);

extern f32 fn_1_2544(M615Segment *segment, M615Triangle *triangle, f32 *segmentTOut, f32 *triangleSOut, f32 *triangleTOut);

extern f32 fn_1_42CC(HuVecF *point, M615Triangle *triangle, f32 *barycentricS, f32 *barycentricT);

extern f32 fn_1_4C9C(M615Segment *segment, M615Triangle *triangle, f32 *segmentTOut, f32 *triangleSOut, f32 *triangleTOut);

extern f32 fn_1_5A4(const HuVecF *a, const HuVecF *b);

extern void fn_1_A0C(M615Bounds *bounds, M615Capsule *capsule);

void fn_1_9140(M615CollisionPool **poolResult, s32 count)
{
    s32 index;
    M615CollisionPool *pool;

    pool = HuMemDirectMallocNum(HEAP_HEAP, 8, 0x10000000U);
    memset(pool, 0, 8);
    pool->count = count;
    pool->entries = HuMemDirectMallocNum(HEAP_HEAP, count * 4, 0x10000000U);
    index = 0;
    while (index < count) {
        pool->entries[index] = HuMemDirectMallocNum(HEAP_HEAP, 0x4C, 0x10000000U);
        memset(pool->entries[index], 0, 0x4C);
        pool->entries[index]->type = -1;
        index += 1;
    }
    if (poolResult) {
        *poolResult = pool;
    }
}

M615CollisionData *fn_1_9228(M615CollisionPool *pool, s32 index)
{
    return pool->entries[index];
}

M615CollisionData *fn_1_9238(M615CollisionPool *pool, M615Capsule *capsule)
{
    M615Bounds capsuleBounds;
    M615CollisionData *selectedCollision;
    M615CollisionData *firstCollision;
    M615CollisionData *collision;
    f32 bestDistance;
    f32 sphereDistance;
    f32 segmentDistance;
    s32 collisionIndex;

    selectedCollision = NULL;
    fn_1_A0C(&capsuleBounds, capsule);
    collisionIndex = 0;
    while (collisionIndex < pool->count) {
        firstCollision = pool->entries[collisionIndex];
        if ((firstCollision->type != -1) &&
            (fn_1_1478(&capsuleBounds, &firstCollision->bounds) != 0) &&
            !(fn_1_5A4(&firstCollision->normal, &capsule->segment.delta) >= 0.0f)) {
            if (firstCollision->type == 0) {
                if (fn_1_2544(&capsule->segment, &firstCollision->triangle, NULL, NULL, NULL) <=
                    capsule->radius * capsule->radius) {
                    selectedCollision = firstCollision;
                    bestDistance = fn_1_14E8(&capsule->segment.start, &firstCollision->triangle, NULL, NULL);
                    break;
                }
            } else if ((firstCollision->type == 1) &&
                       (fn_1_4C9C(&capsule->segment, &firstCollision->triangle, NULL, NULL, NULL) <=
                        capsule->radius * capsule->radius)) {
                selectedCollision = firstCollision;
                bestDistance = fn_1_42CC(&capsule->segment.start, &firstCollision->triangle, NULL, NULL);
                break;
            }
        }
        collisionIndex++;
    }
    while (collisionIndex < pool->count) {
        collision = pool->entries[collisionIndex];
        if ((collision->type != -1) &&
            (fn_1_1478(&capsuleBounds, &collision->bounds) != 0) &&
            !(fn_1_5A4(&collision->normal, &capsule->segment.delta) >= 0.0f)) {
            if (collision->type == 0) {
                if (fn_1_2544(&capsule->segment, &collision->triangle, NULL, NULL, NULL) <=
                    capsule->radius * capsule->radius) {
                    sphereDistance = fn_1_14E8(&capsule->segment.start, &collision->triangle, NULL, NULL);
                    if (bestDistance > sphereDistance) {
                        selectedCollision = collision;
                        bestDistance = sphereDistance;
                    }
                }
            } else if ((collision->type == 1) &&
                       (fn_1_4C9C(&capsule->segment, &collision->triangle, NULL, NULL, NULL) <=
                        capsule->radius * capsule->radius)) {
                segmentDistance = fn_1_42CC(&capsule->segment.start,
                                            &collision->triangle, NULL, NULL);
                if (bestDistance > segmentDistance) {
                    selectedCollision = collision;
                    bestDistance = segmentDistance;
                }
            }
        }
        collisionIndex++;
    }
    return selectedCollision;
}
