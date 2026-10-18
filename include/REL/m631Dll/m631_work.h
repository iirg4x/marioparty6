/* Defines scene, player, conveyor and contact state for Conveyor Bolt. */
#ifndef REL_M631DLL_M631_WORK_H
#define REL_M631DLL_M631_WORK_H

#include "game/charman.h"
#include "game/hu3d.h"
#include "game/mg/seqman.h"
#include "game/object.h"

typedef struct M631ActorWork M631ActorWork;
typedef struct M631AnimationNode M631AnimationNode;
typedef struct M631AnimationQueue M631AnimationQueue;
typedef struct M631SceneListNode M631SceneListNode;
typedef struct M631MotionPairNode M631ActorCallbackWork;
typedef struct M631ActorSetupWork M631ActorSetupWork;

typedef struct M631MotionPairNode M631MotionPairNode;
struct M631MotionPairNode {
    HuVecF *position; /* Position whose horizontal contacts are tested. */
    HuVecF previousPosition; /* Position saved before this update's movement callback. */
    f32 deltaX; /* Horizontal X movement and accumulated contact correction. */
    f32 verticalDelta; /* Unused by the horizontal contact update. */
    f32 deltaZ; /* Horizontal Z movement and accumulated contact correction. */
    u8 unusedBytes[12]; /* Unused storage. */
    f32 radius; /* Contact radius in arena units. */
    s32 state; /* Contact kind: airborne, releasing, ground, obstacle, falling, landed, avoidance or
                * response (0 to 7). */
    void *owner; /* Actor, animation node, response node or avoidance controller passed to
                  * callbacks. */
    void (*beforeUpdate)(void *); /* Called before overlap checks. */
    s32 (*afterUpdate)(void *); /* Called after overlap corrections; nonzero unregisters the
                                 * entry. */
    /* Handles one ordered overlap pair and its penetration distance. */
    void (*resolvePair)(M631MotionPairNode *, M631MotionPairNode *, f32);
    M631MotionPairNode *previous; /* Previous active contact entry, or NULL at the head. */
    M631MotionPairNode *next; /* Next active contact entry. */
};

struct M631ActorSetupWork {
    HU3D_MODELID modelId; /* Model to which the airborne character is hooked. */
    s16 releaseModelId; /* Secondary model shown while preparing a release. */
    M631AnimationQueue *animationQueue; /* Pool from which this actor allocates falling objects. */
};

typedef struct M631ActorMotionState {
    HuVecF position; /* Contact-effect position prepared when a ground player is hit. */
    HU3D_MODELID modelId; /* Hidden contact-effect model; shown during elimination. */
} M631ActorMotionState;

typedef struct M631MotionController {
    M631ActorCallbackWork callback; /* Contact entry used to gather computer avoidance
                                     * directions. */
    HuVecF sourcePosition; /* Current actor position used by the avoidance contact entry. */
    HuVecF avoidanceDirection; /* Accumulated horizontal avoidance direction. */
    HuVecF destination; /* World-space destination chosen by airborne computer behavior. */
    s32 state; /* Contact indication cleared before each contact pass. */
    s32 type; /* Role selector: day airborne solo, night ground solo, night airborne team or day
               * ground team. */
    s32 behaviorState; /* Current state of the computer behavior routine. */
    s32 alertState; /* Nearby ground-player indication for airborne release decisions. */
    s32 stateFrames; /* Frames spent in the current timed behavior. */
    s32 finished; /* Reset during actor setup and the finish phase. */
    u8 unusedBytes[8]; /* Unused storage. */
    f32 previousAngle; /* Heading before the current computer movement update, in degrees. */
    f32 scale; /* Difficulty-dependent damping applied to the avoidance direction. */
} M631MotionController;

struct M631ActorWork {
    OMOBJ *object; /* Object-manager entry for this player. */
    HuVecF position; /* World position in arena units. */
    HuVecF rotation; /* Character model rotation in degrees. */
    HuVecF direction; /* Airborne inertia or elimination velocity, in units per update. */
    M631ActorCallbackWork callbackWork; /* Player's contact entry. */
    M631ActorSetupWork setupWork; /* Airborne attachment models and falling-object pool. */
    M631MotionController motionController; /* Computer behavior state and avoidance contacts. */
    M631ActorWork *target; /* Ground player currently selected by airborne computer behavior. */
    M631ActorMotionState hitEffect; /* Contact effect used when this ground player is
                                           * eliminated. */
    s32 dispatchEnabled; /* Nonzero while normal player input and behavior are enabled. */
    f32 behaviorAngle; /* Requested horizontal heading in degrees. */
    f32 movementStrength; /* Ground movement strength, clamped to zero through one. */
    f32 randomOffset; /* Phase of the airborne vertical bob, in degrees. */
    HU3D_MODELID characterModelId; /* Character model. */
    HU3D_MOTIONID idleMotionId; /* Idle and default character motion. */
    HU3D_MOTIONID slowMoveMotionId; /* Character motion used at low ground movement strength. */
    HU3D_MOTIONID fastMoveMotionId; /* Character motion used at higher ground movement strength. */
    HU3D_MOTIONID airborneMotionId; /* Motion used by the character while attached to its airborne
                                 * model. */
    HU3D_MOTIONID eliminatedMotionId; /* Motion used while an eliminated player rises. */
    HU3D_MOTIONID entranceMotionId; /* Ground player's entrance motion. */
    HU3D_MOTIONID winMotionId; /* Winning player's results motion. */
    HU3D_MOTIONID loseMotionId; /* Losing player's results motion. */
    s16 playerNo; /* Player slot, zero through three. */
    s16 charNo; /* Character selection. */
    s16 groupSelector; /* Configuration team: zero for the solo player, nonzero for the other
                        * three. */
    s16 teamIndex; /* Index within this player's role group. */
    s16 padNo; /* Controller port used for human input. */
    u8 unusedBytes[4]; /* Unused storage. */
    s16 isComputer; /* Nonzero for computer-controlled players. */
    s16 difficulty; /* Computer difficulty index, zero through three. */
    s16 isGroundPlayer; /* Zero for airborne players, nonzero for ground players. */
    s16 hit; /* Nonzero once a ground player has been hit. */
    s16 releaseCooldown; /* Release cooldown counter; zero permits a new release. */
    s16 releaseFrames; /* Frames spent preparing the current falling-object release. */
    s16 accelerationFrames; /* Airborne acceleration counter, capped at 120 updates. */
    s16 movementRequested; /* Movement animation requested during this update. */
    s16 releaseRequested; /* Release requested by human input or computer behavior. */
    s16 isNight; /* Nonzero for the night variant. */
};

struct M631AnimationNode {
    M631AnimationQueue *queueOwner; /* Pool to which this falling object returns. */
    M631AnimationNode *previous; /* Previous active falling object, or NULL at the head. */
    M631AnimationNode *next; /* Next active or free falling object. */
    M631ActorCallbackWork callbackWork; /* Contact entry for the falling object. */
    HuVecF position; /* Main model's world position. */
    HuVecF targetPosition; /* Contact-effect model's world position. */
    s32 animationState; /* State of the growth, fall, lifetime or conveyor-exit animation. */
    s32 timer; /* Frames spent in the current timed animation state. */
    s32 effectHeightIndex; /* Effect height selection; zero lets the effect follow the conveyor. */
    HU3D_MODELID modelId; /* Main falling-object model. */
    HU3D_MODELID effectModelId; /* Secondary contact-effect model. */
    s32 soundId; /* Looping effect sound handle, or -1 when silent. */
};

struct M631AnimationQueue {
    M631AnimationNode nodes[32]; /* Reusable falling objects. */
    M631AnimationNode *head; /* First active falling object. */
    M631AnimationNode *freeHead; /* First free falling object. */
};

struct M631SceneListNode {
    M631SceneListNode *previous; /* Previous active contact-response node. */
    M631SceneListNode *next; /* Next active or free contact-response node. */
    HuVecF position; /* World position of this response. */
    M631ActorCallbackWork callbackWork; /* Contact entry describing this response region. */
};

typedef struct M631ActionNode M631ActionNode;
typedef struct M631ActionQueue M631ActionQueue;
struct M631ActionNode {
    M631ActionQueue *owner; /* Pool to which this conveyor obstacle returns. */
    M631ActionNode *previous; /* Previous active obstacle. */
    M631ActionNode *next; /* Next active or free obstacle. */
    M631ActorCallbackWork callbackWork; /* Conveyor obstacle's contact entry. */
    HuVecF position; /* World position on the conveyor. */
    s32 modelIndex; /* Conveyor lane index, zero through four. */
    s32 completed; /* Nonzero after the closing animation finishes. */
    s16 unusedSlot; /* Unused storage. */
    s16 state; /* Opening, travel or closing state. */
    s16 frame; /* Frames spent playing the opening or closing animation. */
    HU3D_MODELID modelId; /* Conveyor obstacle model. */
};
struct M631ActionQueue {
    M631ActionNode nodes[16]; /* Reusable conveyor obstacles. */
    M631ActionNode *activeHead; /* First active conveyor obstacle. */
    M631ActionNode *freeHead; /* First available conveyor obstacle. */
    s32 stage; /* Timed spawning stage, cycling through single and paired obstacles. */
    s32 timer; /* Frames elapsed in the current spawning stage. */
    s32 modelCounts[5]; /* Active obstacle count in each of the five lanes. */
};
typedef struct M631SceneModelPosition {
    HuVecF position; /* Base world position of a background prop. */
    f32 motionTime; /* Phase of the prop's sine movement, in degrees. */
    HU3D_MODELID model; /* Background prop model. */
} M631SceneModelPosition;

typedef struct M631ActorMotionGroup {
    M631ActorWork *actorSlots[3]; /* Players sharing the airborne or ground role. */
    s32 actorCount; /* Number of players in the role: one or three. */
} M631ActorMotionGroup;

typedef struct M631SceneWork {
    M631ActorWork *actorSlots[4]; /* All players, indexed by player number. */
    M631ActorMotionGroup motionGroups[2]; /* Airborne group followed by ground group. */
    M631ActionQueue actionQueue; /* Conveyor obstacle pool and spawn timing. */
    M631AnimationQueue animationQueue; /* Falling-object pool. */
    s16 sceneModels[30]; /* Shared model IDs indexed with the scene resource table; -1 means not
                          * loaded. */
    s16 unusedSlots[1]; /* Slot initialized to -1 and otherwise unused. */
    u8 unusedBytes[6]; /* Unused storage. */
    M631SceneModelPosition indexedWorks[5]; /* Background prop placements and movement phases. */
    M631SceneListNode *sceneListHead; /* First active contact-response node. */
    M631SceneListNode *sceneListFreeHead; /* First free contact-response node. */
    M631SceneListNode sceneListNodes[32]; /* Reusable contact-response nodes. */
    f32 primaryRotation; /* First rotating background model's heading, in degrees. */
    f32 secondaryRotation; /* Second rotating background model's heading, in degrees. */
    f32 cameraAngle; /* Camera bob phase in degrees. */
    s32 cameraActive; /* Nonzero after the opening camera animation ends. */
    s32 endingScene; /* Selects the results camera and player arrangement. */
    s16 playPhase; /* State of the sequence's play phase. */
    s16 openingPhase; /* State of the opening camera and entrance phase. */
    s16 startPhase; /* State of the music-start phase. */
    s16 resultsTransitionPhase; /* State of the results transition and wipes. */
    s16 finishPhase; /* State of the finish phase and winner selection. */
    s16 resultsPhase; /* State of the results animation and coin award phase. */
    s16 isNight; /* Nonzero for the night variant. */
    s16 eliminatedCount; /* Number of ground players counted out. */
    s16 winnerGroup; /* Winning role group, or -1 before winner selection. */
    s16 conveyorActive; /* Nonzero while the conveyor and obstacle spawning run. */
    s32 openingFrames; /* Frames elapsed in the opening phase. */
    s16 pursuitCount; /* Shared pursuit counter; independent chase-exit checks can decrement it more
                       * than once per update. */
    s16 unused; /* Unused storage. */
} M631SceneWork;

extern OMOBJ *lbl_1_bss_8;

#endif
