#ifndef REL_M646DLL_MODEL_ATTACHMENT_H
#define REL_M646DLL_MODEL_ATTACHMENT_H

#include "REL/minigame_mesh_edges.h"
#include "REL/m646Dll/player_anchor.h"
#include "game/hu3d.h"

typedef struct M646ModelAttachment_s M646ModelAttachment;

struct M646ModelCollider_s {
    u32 kind; /* Registration and active-contact flags. */
    s32 group; /* Group bits used to filter projectile contacts. */
    s32 callbackType; /* Target category selecting points and impact sounds. */
    s32 unusedPlayerIndex; /* Player-index slot unused by stage targets. */
    s32 callbackMode; /* 11 selects model-target collision processing. */
    M646ModelAttachment *owner; /* Target attachment passed to the callbacks. */
    void (*update)(M646ModelAttachment *); /* Unused target-update callback; setup assigns an empty
                                            * function. */
    s32 (*contact)(M646ModelCollider *, void *); /* Handles a projectile contact. */
    u8 unk20[4]; /* No code names this field. */
    MinigameWorldEdge *current; /* World edges used by the current contact pass. */
    MinigameWorldEdge *alternate; /* Spare edge buffer. */
    s32 transformCount; /* Number of transforms applied to the target geometry. */
    HSF_TRANSFORM pose; /* Joint position and scale used for contact checks. */
    Mtx *matrices; /* Matrices for the target collision geometry. */
    MinigameWorldEdge *unk58; /* The game's code never reads this field. */
    s16 category; /* Target group selecting the contact-distance threshold. */
    HU3D_MODELID model; /* Carrier model from which the target joint is sampled. */
    char jointName[64]; /* Carrier joint used to follow this target. */
    HSF_OBJECT *unkA0; /* The game's code never reads this field. */
    s32 assigned; /* Nonzero while a computer player has reserved this target. */
};

struct M646ModelAttachment_s {
    HU3D_MODELID parentModel; /* Hook parent, or -1 after the first projectile contact. */
    HU3D_MODELID model; /* Visible target model. */
    HU3D_MOTIONID motion; /* Hit animation, paused until first contact. */
    s16 state; /* Number of contacts; zero means still attached and unhit. */
    M646ModelCollider collider; /* Contact geometry and callbacks for this target. */
    Vec position; /* World position captured when the target detaches. */
};

#endif
