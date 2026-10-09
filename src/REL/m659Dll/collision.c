/* Collision registration and frame-by-frame collision response for Asteroad Rage. */
#include "game/object.h"
#include "game/memory.h"
#include "string.h"
#include "REL/m659/collision.h"

/* Collision-manager object whose data stores the registration table. */
OMOBJ *lbl_1_bss_54;
/* Latch set after an overlap; gates collision response callbacks. */
int lbl_1_bss_50;

/* Both array extents are bounded by the corresponding insertion and lookup
 * functions. The second set's element semantics are not yet reconstructed. */
void fn_1_4280(OMOBJ *obj);
void fn_1_4290(OMOBJ *obj);
#include "REL/m659/collision.h"
/* IEEE-754 binary32 fields used to restore ship positions after NaN coordinates. */
#define M659_FLOAT_EXPONENT_MASK 0x7F800000 /* Binary32 exponent bits 23 through 30. */
/* An all-ones exponent denotes infinity or NaN. */
#define M659_FLOAT_INFINITE_EXPONENT M659_FLOAT_EXPONENT_MASK
#define M659_FLOAT_FRACTION_MASK 0x007FFFFF /* Binary32 fraction bits 0 through 22 distinguish NaN
                                             * from infinity or zero. */
enum {
    M659_FP_NAN = 1, M659_FP_INFINITE = 2,
    M659_FP_ZERO = 3, M659_FP_NORMAL = 4, M659_FP_SUBNORMAL = 5
};

/* The course ignores NaN coordinates and restores the last valid position. */
#include "REL/m659/collision.h"
#include "math.h"
#include "game/object.h"
#include "math.h"

/* The distance helpers below use the course plane's X and Z coordinates only. */

/* Creates the collision-manager object and allocates its registration table during minigame
 * startup. */
void fn_1_41F4(OMOBJMAN *manager)
{
    OMOBJ *obj;
    M659CollisionTable *collisionTable;

    obj = omAddObjEx(manager, 70, 0, 0, -1, fn_1_4280);
    lbl_1_bss_54 = obj;
    collisionTable = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M659CollisionTable), HU_MEMNUM_OVL);
    obj->data = collisionTable;
    memset(collisionTable, 0, sizeof(M659CollisionTable));
}

void fn_1_427C(void)
{
}

/* Collision-manager startup callback that installs its regular frame callback. */
void fn_1_4280(OMOBJ *obj)
{
    obj->objFunc = fn_1_4290;
}

/* Classifies a binary32 coordinate from its IEEE-754 exponent and fraction bits. */
static inline int classifyFloat(float coordinate)
{
    switch (*(int *)&coordinate & M659_FLOAT_EXPONENT_MASK) {
    case M659_FLOAT_INFINITE_EXPONENT:
        if ((*(int *)&coordinate & M659_FLOAT_FRACTION_MASK) != 0) {
            return M659_FP_NAN;
        }
        return M659_FP_INFINITE;
    case 0:
        if ((*(int *)&coordinate & M659_FLOAT_FRACTION_MASK) != 0) {
            return M659_FP_SUBNORMAL;
        }
        return M659_FP_ZERO;
    }
    return M659_FP_NORMAL;
}

/* Restores the saved position when either horizontal coordinate is any NaN value. */
static inline void recoverPosition(void *collisionData)
{
    M659Collision *collision = collisionData;
    if (classifyFloat(collision->position->x) == M659_FP_NAN ||
        classifyFloat(collision->position->z) == M659_FP_NAN) {
        *collision->position = collision->previousPosition;
    }
}

/* Collision-manager frame callback: resolves eligible pairs, restores NaN positions, then updates
 * registered owners. */
void fn_1_4290(OMOBJ *obj)
{
    int firstSlot;
    M659Collision *collisionA;
    M659CollisionTable *collisionTable = obj->data;
    int secondSlot;
    M659Collision *collisionB;
    int overlapOccurred = 0;
    M659Collision *collisionAData;

    for (firstSlot = 0; firstSlot < 200; firstSlot++) {
        collisionA = collisionTable->entries[firstSlot];
        if (collisionA != NULL) {
            collisionAData = collisionA;
            if ((collisionAData->flags & 0x1) && collisionAData->collisionDisabled == 0) {
                for (secondSlot = firstSlot + 1; secondSlot < 200; secondSlot++) {
                    collisionB = collisionTable->entries[secondSlot];
                    if (collisionB != NULL) {
                        M659Collision *collisionBData = collisionB;
                        if ((collisionBData->flags & 0x1) &&
                            collisionBData->collisionDisabled == 0 &&
                            (collisionAData->mask & collisionBData->mask) &&
                            fn_1_450C(collisionA, collisionB) == 1) {
                            overlapOccurred = 1;
                        }
                    }
                }
            }
        }
    }
    if (overlapOccurred == 1) {
        lbl_1_bss_50 = 1;
    }
    for (firstSlot = 0; firstSlot < 200; firstSlot++) {
        collisionA = collisionTable->entries[firstSlot];
        if (collisionA != NULL) {
            collisionAData = collisionA;
            if (collisionAData->collisionDisabled == 0) {
                recoverPosition(collisionA);
            }
            if (collisionAData->update != NULL) {
                collisionAData->update(collisionAData->userData);
            }
        }
    }
}

/* Called for eligible pairs; separates overlapping XZ circles, applies enabled velocity impulses,
 * and invokes response callbacks. */
int fn_1_450C(M659Collision *firstCollision, M659Collision *secondCollision)
{
    float directionX, directionZ;
    float separationLength;
    HuVecF midpoint;
    HuVecF *firstPosition = firstCollision->position;
    HuVecF *secondPosition = secondCollision->position;
    float impulseRatio, radiusSum;

    directionX = firstPosition->x - secondPosition->x;
    directionZ = firstPosition->z - secondPosition->z;
    separationLength = sqrtf(directionX * directionX + directionZ * directionZ);
    radiusSum = firstCollision->radius + secondCollision->radius;
    if (radiusSum > separationLength) {
        if (separationLength == 0.0) {
            /* Coincident centers use negative Z as the deterministic separation direction. */
            directionX = 0.0f;
            directionZ = -1.0f;
        } else {
            directionX /= separationLength;
            directionZ /= separationLength;
        }
        midpoint.x = firstPosition->x + 0.5 * (secondPosition->x - firstPosition->x);
        midpoint.z = firstPosition->z + 0.5 * (secondPosition->z - firstPosition->z);
        /* Replace the center distance with the half-width used to separate both ships. */
        separationLength = 0.5 * radiusSum;
        firstPosition->x = midpoint.x + directionX * separationLength;
        firstPosition->z = midpoint.z + directionZ * separationLength;
        secondPosition->x = midpoint.x - directionX * separationLength;
        secondPosition->z = midpoint.z - directionZ * separationLength;
        /* The either-object condition is repeated verbatim; one enabled flag is sufficient. */
        if ((firstCollision->collisionImpulseEnabled != 0 ||
             secondCollision->collisionImpulseEnabled != 0) &&
            (secondCollision->collisionImpulseEnabled != 0 ||
             firstCollision->collisionImpulseEnabled != 0)) {
            impulseRatio = 0.1f;
            firstCollision->velocity->x +=
                firstCollision->factor * (impulseRatio * (directionX * separationLength));
            firstCollision->velocity->z +=
                firstCollision->factor * (impulseRatio * (directionZ * separationLength));
            impulseRatio = 0.1f;
            secondCollision->velocity->x -=
                secondCollision->factor * (impulseRatio * (directionX * separationLength));
            secondCollision->velocity->z -=
                secondCollision->factor * (impulseRatio * (directionZ * separationLength));
        }
        /* Once an overlap sets the latch, later response callbacks stay suppressed while
         * separation still runs. */
        if (lbl_1_bss_50 == 0) {
            if (firstCollision->collide != NULL) {
                /* Callback return values are ignored; pair overlap determines this function's
                 * result. */
                firstCollision->collide(firstCollision, secondCollision);
            }
            if (secondCollision->collide != NULL) {
                secondCollision->collide(secondCollision, firstCollision);
            }
        }
        return 1;
    }
    return 0;
}

/* XZ-distance helpers use this to subtract the second position from the first. */
void fn_1_4918(HuVecF positionA, HuVecF positionB, HuVecF *difference)
{
    difference->x = positionA.x - positionB.x;
    difference->z = positionA.z - positionB.z;
}

/* The route planner calls this helper to measure an XZ difference vector. */
double fn_1_493C(HuVecF difference)
{
    return sqrt(difference.x * difference.x + difference.z * difference.z);
}

/* The computer route planner uses this helper to measure spacing between course points. */
double fn_1_4ABC(HuVecF positionA, HuVecF positionB)
{
    HuVecF difference;
    fn_1_4918(positionA, positionB, &difference);
    return fn_1_493C(difference);
}

/* Registers a collision object in the first available collision-table slot and returns that
 * slot. */
int fn_1_4CA4(M659Collision *collision)
{
    M659Collision **collisionEntry;
    int index;
    OMOBJ *obj = lbl_1_bss_54;
    M659CollisionTable *collisionTable = obj->data;

    collisionEntry = collisionTable->entries;
    for (index = 0; index < 200; index++, collisionEntry++) {
        if (*collisionEntry == NULL) {
            *collisionEntry = collision;
            return index;
        }
    }
    return -1;
}

/* Registers a secondary pointer and returns its table index after the collision-object slots. */
int fn_1_4D1C(void *registration)
{
    void **secondaryEntry;
    int index;
    OMOBJ *obj = lbl_1_bss_54;
    M659CollisionTable *collisionTable = obj->data;

    secondaryEntry = collisionTable->secondaryEntries;
    for (index = 0; index < 130; index++, secondaryEntry++) {
        if (*secondaryEntry == NULL) {
            *secondaryEntry = registration;
            return index + 200;
        }
    }
    return -1;
}

/* Clears a collision-table registration by its returned index. */
void fn_1_4D94(int registrationIndex)
{
    M659CollisionTable *collisionTable;
    OMOBJ *obj = lbl_1_bss_54;
    collisionTable = obj->data;
    if (registrationIndex >= 0) {
        if (registrationIndex < 200) {
            collisionTable->entries[registrationIndex] = NULL;
        }
        registrationIndex -= 200;
        if (registrationIndex < 130) {
            collisionTable->secondaryEntries[registrationIndex] = NULL;
        }
    }
}

/* Returns a registered pointer for a valid table index, or NULL outside both tables. */
void *fn_1_4DF8(int registrationIndex)
{
    M659CollisionTable *collisionTable;
    OMOBJ *obj = lbl_1_bss_54;
    collisionTable = obj->data;
    if (registrationIndex < 0) {
        return NULL;
    }
    if (registrationIndex < 200) {
        return collisionTable->entries[registrationIndex];
    }
    registrationIndex -= 200;
    if (registrationIndex < 130) {
        return collisionTable->secondaryEntries[registrationIndex];
    }
    return NULL;
}
