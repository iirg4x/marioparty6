#ifndef REL_M675DLL_ACTOR_STATE_H
#define REL_M675DLL_ACTOR_STATE_H

#include "game/object.h"

typedef struct M675Collision M675Collision;

struct M675Collision {
    u32 flags; /* Collision enable flags and overlap-channel mask. */
    u32 collisionMask;
    u32 type; /* Event type reported by this collision record. */
    s32 index; /* Owner-supplied record index. */
    s32 shapeType; /* Collision shape selector: 0 sphere, 10 ring, 11 boundary. */
    OMOBJ *owner; /* Object that owns this collision record. */
    void *excludedOwner; /* Other owner ignored when the collision pair is checked. */
    OMOBJ_FUNC update; /* Callback that refreshes the owner's transform. */
    /* shapeType selects which data follows the registration header. */
    union {
        struct {
            s32 (*contact)(M675Collision *, M675Collision *); /* Pair-contact callback. */
            Vec *position, *velocity; /* World position and velocity used by collision response. */
            Vec positionCopy; /* Position snapshot stored when the record is initialized. */
            f32 radius; /* Collision radius. */
            f32 yieldOnContact; /* Nonzero makes this sphere yield to a zero-valued partner. */
            s32 handled; /* Nonzero after another callback handles this event. */
        } sphere;
        struct {
            Vec center;
            f32 bottomOffsetY, height;
            f32 innerRadiusX, innerRadiusZ, outerRadiusX, outerRadiusZ;
        } ring;
        struct {
            Vec center;
            f32 radiusX, radiusZ;
            s32 direction; /* Nonzero keeps the sphere inside the ellipse. */
        } boundary;
    };
};

typedef struct M675ActorState {
    M675Collision collision; /* Player contact data; prop instances leave it disabled. */
    s32 playerMappingIndex; /* Player mapping used by the player gameplay helpers. */
    s32 unk48; /* The game's code never reads this field. */
    s32 formationIndex; /* Current player slot in the formation arrays. */
    s32 active; /* Gates human input, motion-offset easing, result arrival reporting and character
                 * feedback. */
    Vec basePosition; /* Player formation position before a motion offset is applied. */
    Vec position, velocity, rotation; /* World position, per-frame movement and Euler angles. */
    f32 baseYaw; /* Player facing angle before its motion offset is applied. */
    u32 buttonsDown, buttons; /* Buttons pressed this frame and currently held. */
    f32 unk90; /* The game's code never reads this field. */
    f32 unk94; /* The game's code never reads this field. */
    s32 motion, motionFrame; /* Selected motion and transform-update count since its selection. */
    s32 requestedMotion; /* Motion queued by the current frame's behavior. */
    f32 motionSpeed; /* Playback speed for the queued motion. */
} M675ActorState;

#endif
