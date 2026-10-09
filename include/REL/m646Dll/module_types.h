/* Shared work records for the Hyper Sniper sequence, players, collisions, and stage. */
#ifndef REL_M646DLL_MODULE_TYPES_H
#define REL_M646DLL_MODULE_TYPES_H

#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/player.h"
#include "game/board/window.h"
#include "game/window.h"
#include "game/printfunc.h"
#include "string.h"
#include "stdio.h"

/* Tracks the empty camera object and the separate model playing either sequence motion. */
typedef struct M646CameraData_s {
    OMOBJ *object;                   /* Object created during camera setup. */
    HU3D_MODELID model;              /* Camera model playing the selected motion, or -1. */
    HU3D_MOTIONID motions[2];        /* Opening and alternate camera motions. */
} M646CameraData;

/* Sequence state shared by setup and phase callbacks. */
typedef struct M646MainData_s {
    OMOBJMAN *objectManager;          /* Manager used to create the minigame's objects. */
    M646CameraData camera;            /* Camera object, current model, and loaded motions. */
    s16 lightId;                      /* Global light created for the stage. */
    s32 resetValue;                   /* Cleared at sequence setup; not otherwise used. */
    s32 streamId;                     /* Sequence music handle, or -1 before music starts. */
    s16 nightFlag;                    /* Snapshot of the minigame's day/night setting. */
    s32 unknown20;                    /* Cleared at sequence setup; not otherwise used. */
} M646MainData;

/* Character motion and result-facing turn state shared by the player callbacks. */
typedef struct M646PlayerWork_s {
    s16 playerIndex;                  /* Player slot, 0 through 3. */
    s16 character;                    /* Character selected in the player configuration. */
    s16 state;                        /* Index of the current character motion. */
    HU3D_MOTIONID motions[7];          /* Idle, result, opening, firing, and turning motions. */
    Point3d translation;              /* Character position in world units. */
    Point3d rotation;                 /* Rotation vector also passed to the collision anchor. */
    Point3d setupRotation;            /* Zeroed rotation vector used when positioning the player
                                       * object at setup. */
    s32 transitionCommand;            /* Human/computer update phase for play and results. */
    s32 motionState;                  /* Nonzero selects the winning result motion. */
    s32 rotationActive;               /* Nonzero while the result-facing turn is advancing. */
    f32 rotationStart;                /* Heading captured before the result turn, in degrees. */
    f32 rotationPhase;                /* Result-turn elapsed frames, advanced toward 30. */
} M646PlayerWork;

typedef struct M646PlayerTicket_s {
    OMOBJ *playerObject;              /* Character object queued by target-selection setup. */
    s32 playerKind;                   /* Zero for a human player; nonzero for a computer player. */
} M646PlayerTicket;

/* Character pose, input, and computer aim shared with projectile and targeting callbacks. */
typedef struct M646PlayerRuntimeWork_s {
    M646PlayerWork motion;             /* Character motion and turn-transition state. */
    f32 forceAngle;                    /* Forced head-joint Z rotation, in degrees. */
    Vec modelPosition;                 /* Aim-model position; shots aim 120 world units farther in
                                        * Z. */
    Vec projectileDirection;           /* Flight displacement per frame, with length 40 world
                                        * units. */
    Vec projectilePosition;            /* Gun-hook position where a shot starts. */
    Vec itemPosition;                  /* Detached gun position during the finish animation. */
    Vec itemRotation;                  /* Detached gun rotation in degrees. */
    f32 itemPhase;                     /* Frames elapsed in the detached-gun animation. */
    f32 itemDuration;                  /* Detached-gun animation duration in frames. */
    u8 unusedPlayerStorage[12];        /* Cleared at player creation; item, input, and target
                                       * callbacks leave these bytes untouched. */
    s16 padIndex;                      /* Controller port used by this player. */
    f32 stickOffsetX;                  /* Horizontal aim displacement this frame, in world units. */
    f32 stickOffsetY;                  /* Vertical aim displacement this frame, in world units. */
    u32 buttons;                       /* Buttons pressed this frame. */
    s32 playerKind;                    /* Zero for a human player; nonzero for a computer player. */
    s16 comDifficulty;                 /* Computer difficulty, 0-3; -1 for a human player. */
    s32 targetReady;                   /* Nonzero while a computer player's target is active. */
    s32 motionReady;                   /* Nonzero while computer aim is moving toward its target. */
    void *targetData;                  /* Chosen attachment, or NULL when aiming at stored
                                        * coordinates. */
    f32 aimError;                      /* Extra arrival tolerance in world units. */
    f32 randomX;                       /* Horizontal error added to the target, in world units. */
    f32 randomY;                       /* Vertical error added to the target, in world units. */
    s16 targetDelayTicks;              /* Remaining target-selection attempts to wait; not a frame
                                        * count. */
    s16 shotCount;                     /* Number of shots fired by the computer player. */
    f32 targetX;                       /* Horizontal fallback aim coordinate in world units. */
    f32 targetY;                       /* Vertical fallback aim coordinate in world units. */
} M646PlayerRuntimeWork;

/* World positions sampled from the moving stage for stage-boundary checks. */
typedef struct M646PositionTable_s {
    Point3d points[4];                 /* Four sampled stage positions in world units. */
} M646PositionTable;

/* Collision header and edge buffers registered for a moving-stage surface. */
typedef struct M646AnchorRecord_s {
    s32 kind;                         /* Collision registration and activity flags. */
    s32 group;                        /* Group mask compared with projectile collision groups. */
    s32 callbackType;                 /* Contact category; stage setup uses 12. */
    s32 unknown0C;                    /* Player-tag slot cleared for stage surfaces. */
    s32 callbackMode;                 /* 10 selects the stage-surface collision path. */
    OMOBJ *owner;                     /* Moving-stage object that owns the surface. */
    s32 unknown18;                    /* Update-callback slot cleared for stage surfaces. */
    s32 unknown1C;                    /* Contact-callback slot cleared for stage surfaces. */
    s32 unknown20;                    /* Edge count set by the collision-mesh setup helper. */
    Point3d *pointBuffer;             /* Active edge buffer exchanged by the transform updater. */
    Point3d *transformedBuffer;        /* Spare edge buffer exchanged by the transform updater. */
} M646AnchorRecord;

#endif
