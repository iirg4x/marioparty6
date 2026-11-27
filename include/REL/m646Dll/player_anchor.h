#ifndef REL_M646DLL_PLAYER_ANCHOR_H
#define REL_M646DLL_PLAYER_ANCHOR_H

#include "game/object.h"

/* Optional model contact and swept position for the collision manager's actor slots. */
typedef struct M646ModelCollider_s M646ModelCollider;
typedef struct M646ActorAnchor_s M646ActorAnchor;
struct M646ActorAnchor_s {
    u32 kind; /* Collision registration and activity flags. */
    s32 group; /* Collision group bits tested against other registered objects. */
    u32 callbackType; /* Actor collision category used by contact handling. */
    s32 playerIndex; /* Collision player tag; projectile setup fixes it at zero. */
    s32 callbackMode; /* Manager mode that selects this anchor's collision path. */
    OMOBJ *owner; /* Object whose work record owns this anchor. */
    void (*update)(OMOBJ *); /* Callback that updates the anchor after collision checks. */
    s32 (*contact)(M646ActorAnchor *, M646ModelCollider *); /* Handles target contact. */
    Vec *position; /* Current actor position in stage units. */
    Vec *direction; /* Per-frame movement vector in stage units. */
    Vec savedPosition; /* Previous position used for swept collision checks. */
    f32 radius; /* Actor collision radius in stage units. */
    f32 responseScale; /* Scales movement added by actor-actor contact. */
};

#endif
