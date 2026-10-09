/* Collision object records and registration table for Asteroad Rage. */
#ifndef M659_COLLISION_H
#define M659_COLLISION_H
#include "game/object.h"

typedef struct M659Collision M659Collision;
struct M659Collision {
    u32 flags; /* Bit 0 enables collision participation. */
    u32 mask; /* Bit groups that may collide with this object. */
    u32 collisionImpulseEnabled; /* A set flag enables pair velocity impulses in the collision
                                  * pass. */
    u32 unusedCollisionWord; /* Written as zero; no other use is established in this module. */
    s32 collisionDisabled; /* Nonzero skips collision and position recovery. */
    void *userData; /* Owner passed to the update callback. */
    void (*update)(void *); /* Optional per-frame owner update. */
    int (*collide)(M659Collision *, M659Collision *); /* Optional collision response. */
    HuVecF *position; /* Current world position. */
    HuVecF *velocity; /* Velocity modified by collision response. */
    HuVecF previousPosition; /* Position saved at registration and restored if current X or Z is
                              * NaN. */
    float radius; /* Collision radius in world units. */
    float factor; /* Scale applied to the collision impulse. */
};

typedef struct M659CollisionTable {
    M659Collision *entries[200]; /* Collision objects checked by the frame pass. */
    void *secondaryEntries[130]; /* Secondary registrations accessed by index. */
} M659CollisionTable;
extern OMOBJ *lbl_1_bss_54;
extern int lbl_1_bss_50;
int fn_1_450C(M659Collision *firstCollision, M659Collision *secondCollision);
#endif
