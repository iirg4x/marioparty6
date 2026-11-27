#ifndef REL_M646DLL_COLLISION_STATE_H
#define REL_M646DLL_COLLISION_STATE_H

#include "REL/minigame_mesh_edges.h"
#include "game/hsfex.h"

/* Collision edge buffers and matrices shared by the moving-stage code. */
typedef struct {
    u32 kind; /* Collision registration and response flags. */
    u32 group; /* Group mask compared with projectile collision groups. */
    u32 callbackType; /* Contact category, set to 12 by stage setup. */
    s32 playerIndex; /* Player-index slot, cleared for stage surfaces. */
    s32 callbackMode; /* Selects the collision manager's stage-surface path. */
    void *unk14; /* The game's code never reads this field. */
    void (*update)(void *); /* Optional update callback, cleared by stage setup. */
    s32 (*contact)(void *, void *); /* Optional contact callback, cleared by stage setup. */
    s32 edgeCount; /* Number of quad edges included in the collision sweep. */
    MinigameWorldEdge *current; /* Edges used by this frame's collision checks. */
    MinigameWorldEdge *alternate; /* Spare edge buffer swapped with current during transform updates. */
    MinigameMeshEdge *original; /* Source-mesh endpoints retained for transforms. */
    s32 transformCount; /* Number of source objects with transform records. */
    HSF_TRANSFORM **transforms; /* Source transforms for the HSF object list. */
    Mtx *matrices; /* Per-object matrices used to transform source edges. */
    MinigameWorldEdge *unk3C; /* The game's code never reads this field. */
} M646CollisionRecord;

#endif
