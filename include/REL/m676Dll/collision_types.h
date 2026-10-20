/* Collision registration records and the shapes used by this minigame. */
#ifndef REL_M676DLL_COLLISION_TYPES_H
#define REL_M676DLL_COLLISION_TYPES_H
#include "game/object.h"

typedef struct M676CollisionEntry M676CollisionEntry;
#define M676_COLLISION_OBJECT_COUNT 84
typedef struct M676CollisionHeader {
    /* Registration flags, collision-category mask, and object category; state selects the shape. */
    u32 flags;
    u32 collisionMask;
    u32 type;
    /* Owner ID and current collision state. */
    s32 id;
    s32 state;
    /* Callback context, owning object, and per-frame update callback. */
    void *callbackArgument;
    void *owner;
    void (*updateCallback)(OMOBJ *argument);
} M676CollisionHeader;
typedef struct M676CollisionBounds {
    M676CollisionHeader header;
    /* Axis-aligned world-space limits used by the collision tests. */
    Vec minimum, maximum;
} M676CollisionBounds;

struct M676CollisionEntry {
    M676CollisionHeader header;
    union {
        struct {
            /* Contact handler and live position/velocity vectors for this dynamic collider. */
            s32 (*callback)(M676CollisionEntry *object, M676CollisionEntry *other);
            Vec *position;
            Vec *velocity;
            /* Position sampled for swept tests and collision radius in world units. */
            Vec currentPosition;
            f32 radius;
            /* Nonzero permits this object's position to move; if both values are zero, both
            * collision arguments move. */
            f32 movable;
            /* Contact marker read by the prop and particle updates. */
            s32 active;
        } dynamic;
        struct {
            /* Center and vertical/horizontal extents and radii in world units. */
            Vec center;
            f32 lowerY, verticalSpan, innerRadiusX, innerRadiusZ, outerRadiusX, outerRadiusZ;
        } region;
        /* Center, horizontal radii in world units, and whether the test uses the inside or
         * outside. */
        struct { Vec center; f32 radiusX, radiusZ; s32 direction; } ellipse;
        /* Axis-aligned world-space limits for a static box. */
        struct { Vec minimum, maximum; } bounds;
    } shape;
    /* Player slot copied from the opposing prop on contact. */
    s32 contactId;
};
typedef union M676SceneRegistration {
    M676CollisionBounds bounds;
    M676CollisionEntry entry;
} M676SceneRegistration;
typedef struct M676CollisionWork {
    /* Cleared during setup; no module function reads these leading bytes. */
    u8 unk00, unk01, unk02, unk03, unk04, unk05;
    u8 unk06, unk07;
    /* Registered collision objects and objects tested against them. */
    M676CollisionEntry *objects[M676_COLLISION_OBJECT_COUNT];
    M676CollisionEntry *targets[10];
} M676CollisionWork;
#endif
