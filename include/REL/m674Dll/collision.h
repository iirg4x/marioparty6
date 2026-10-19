/* Registration records and the geometry used by the collision manager. */
#ifndef REL_M674DLL_COLLISION_H
#define REL_M674DLL_COLLISION_H
#include "game/object.h"
typedef struct M674Entry M674Entry;
typedef struct M674ActorShape {
    s32 (*callback)(M674Entry *self, M674Entry *other); /* Called after contact resolution; its
                                                         * return value is ignored. */
    Vec *position; /* Actor center in world coordinates. */
    Vec *velocity; /* Actor velocity; vertical velocity is cleared on landing. */
    Vec initialPosition; /* Position captured when the actor is registered. */
    f32 radius; /* Radius for the 3D actor-pair test and horizontal boundary corrections. */
    f32 collisionFlag; /* Pair correction moves this actor if its flag is nonzero or the other's is
                        * zero. */
    M674Entry *contactRegion; /* Region entry recorded when the actor lands on a boundary. */
} M674ActorShape;
typedef struct M674RegionShape {
    Vec center; /* World-space center used by boundary and height calculations. */
    f32 lowerY; /* Offset from center.y to the lower bound of the collision volume. */
    f32 verticalSpan; /* Height from the lower bound to the upper landing surface. */
    f32 innerRadiusX; /* Inner ellipse radius along X, in world units. */
    f32 innerRadiusZ; /* Inner ellipse radius along Z, in world units. */
    f32 outerRadiusX; /* Outer ellipse radius along X, in world units. */
    f32 outerRadiusZ; /* Outer ellipse radius along Z, in world units. */
} M674RegionShape;
typedef struct M674EllipseShape {
    Vec center; /* World-space center of the elliptical boundary. */
    f32 radiusX; /* Ellipse radius along X, in world units. */
    f32 radiusZ; /* Ellipse radius along Z, in world units. */
    s32 direction; /* Selects whether correction applies inside or outside the ellipse. */
} M674EllipseShape;
struct M674Entry {
    u32 flags; /* Collision-entry state flags. */
    u32 mask; /* Entry mask compared with other entries. */
    u32 type; /* Entry category used by registry lookups. */
    s32 id; /* Entry ID within its category. */
    s32 state; /* State value checked by collision pair processing. */
    void *primaryRelation; /* Object associated with this entry. */
    void *secondaryRelation; /* Additional object or region associated with this entry. */
    OMOBJ_FUNC invoke; /* Object callback run after every collision pass for a registered primary
                        * entry. */
    union {
        M674ActorShape actor;
        M674RegionShape region;
        M674EllipseShape ellipse;
    } shape;
};
typedef struct M674EntryWork {
    s32 state; /* Collision-manager processing state. */
    s32 timer; /* Frame counter for the collision manager's warm-up. */
    M674Entry *entries[31]; /* Primary collision-entry slots. */
    M674Entry *secondaryEntries[10]; /* Secondary entries checked against the primary set. */
} M674EntryWork;
#endif
