/* State shared by the minigame's camera, players, map pieces and collision updates. */
#ifndef REL_M627DLL_GAMEPLAY_H
#define REL_M627DLL_GAMEPLAY_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "game/mg/actman.h"
#include "game/object.h"

typedef struct M627Entry {
    u16 flags; /* Single-piece visibility/collision bits; grouped-piece entries leave this at
                * zero. */
    s16 actorIndex; /* Collision-map slot assigned to this map piece. */
    Vec position; /* World-space center of the piece. */
    s16 modelId; /* Foreground model used for the single or grouped piece. */
    s16 collisionModelId; /* Hidden model stored in the collision-map model list. */
    s16 linkedModelIds[2]; /* Per-camera block-clearing effect models, killed after their motions
                            * end. */
} M627Entry;

typedef struct M627PlayerWork {
    MGPLAYER *player; /* Minigame player and character actor being controlled. */
    Vec position; /* Last accepted world position of the player actor, used by the split-screen edge
                   * clamp. */
    s16 active; /* Set when this player's camera group wins the race. */
    s16 playerIndex; /* Player number in the four-slot configuration table. */
    s16 playerSlot; /* Slot selecting the character's start/result positions and entrance mesh. */
    s16 cameraGroup; /* Split-screen side assigned to this player. */
    s16 characterId; /* Character ID used by character-motion and item-hook helpers. */
    s32 motionSlot; /* Last motion slot tracked by the entrance/result object callbacks. */
    s32 frame; /* Frame counter used by the player's update state. */
    s16 blockClearState; /* Block-clearing action state during mode 3. */
    s16 blockClearTimer; /* Elapsed update count for the block-clearing action. */
    s16 accessoryModelId; /* Model attached to the character during play. */
    f32 motionSpeed; /* Playback-speed multiplier for the character's current motion. */
    s16 playerType; /* Configured human or computer player type. */
    s16 selectionMode; /* Computer difficulty used to choose and follow map entries. */
    M627Entry *activeEntry; /* Map piece currently being approached or selected. */
    Vec followPosition; /* World-space point followed by the player actor. */
    s16 travelTicks; /* Remaining movement ticks before the next approach decision. */
    s16 positionState; /* Consecutive steering checks with too little movement from
                        * lastPositionSample. */
    Vec lastPositionSample; /* Last actor-position sample used to detect stalled computer
                             * steering. */
    s16 waitFrames; /* Delay before the next computer-player movement decision. */
    s16 clearTargetHistoryOnArrival; /* Controls clearing recent target history when the selected
                                      * block is reached. */
    s16 sequenceType; /* Steering stage: 0 targets a block, 1 retreats sideways, and 2 moves
                       * forward. */
    M627Entry *previousEntry; /* First target-history slot; holds the sole recorded target or the
                               * older of two. */
    M627Entry *currentEntry; /* Newest target-history entry once two targets have been recorded;
                              * otherwise NULL. */
} M627PlayerWork;

typedef struct M627BoundaryLine {
    s32 type; /* Boundary classification passed to the collision-line builder. */
    Vec origin; /* World-space point at one end of the boundary. */
    Vec nearPoint; /* Point offset 75 units from origin along the collision normal. */
    Vec farPoint; /* Point offset 175 units from origin along the collision normal. */
    Vec direction; /* Unnormalized vector from origin to the other endpoint along the boundary in
                    * the XZ plane. */
    Vec normal; /* Unit XZ-plane normal used for collision response. */
} M627BoundaryLine;

typedef struct M627MapWork {
    s16 count; /* Number of single map pieces. */
    s16 groupCount; /* Number of two-by-two grouped pieces. */
    M627Entry *entries; /* Single-piece records. */
    M627Entry *groups; /* Grouped-piece records. */
    s16 blockClearModel; /* Hidden source for per-camera block-clearing effect models. */
    OMOBJ *modelsObject; /* Object that owns the map and collision model IDs. */
    s16 timer; /* Frames elapsed during the post-round map cleanup. */
    u8 unusedBytes[6]; /* Allocated work bytes with no reads in this module. */
    s16 boundaryCount; /* Number of playfield boundary lines. */
    M627BoundaryLine *boundaries; /* Playfield edge segments used by actor collision. */
} M627MapWork;

typedef struct M627ActorWork {
    MGACTOR *actor; /* Moving ball actor controlled by this update object. */
    s16 cameraGroup; /* Split-screen side used for visibility and sound panning. */
    s16 modelId; /* Ball model rendered for this actor. */
    s16 motionId; /* Motion used to animate the ball model. */
    s32 unusedCollisionState; /* Initialized to zero but not read in this module. */
    Vec velocity; /* Ball velocity: world space on the flat course, rotated collision space on the
                   * slope. */
    Vec unusedPositionVector; /* Initialized to zero but not read in this module. */
    s16 collisionMode; /* Active collision response mode. */
    s16 soundActive; /* Rolling-cue state; can be set while soundMode suppresses playback. */
    s32 soundHandle; /* Handle of the rolling sound effect. */
    s16 boundarySoundPlayed; /* Set after the boundary-hit sound plays; cleared when the actor
                              * lands. */
    s16 soundMode; /* Enables rolling and impact sounds for this actor. */
} M627ActorWork;

typedef struct M627CameraState {
    Vec cameraPosition[2]; /* World-space camera positions for the opening and split views. */
    Vec cameraTarget[2]; /* World-space look-at points for those views. */
    s32 unusedCameraWord; /* Allocated state word with no reads in this module. */
    s32 elapsedFrames; /* Frames elapsed in the active camera transition. */
    s16 startingColumn; /* Next slot in the five-sample camera trend history. */
    f32 rowValue[2]; /* Tracked Z reference: the first group-0 player's Z for row 0 and the midpoint
                      * of group-1 players' Z range for row 1. */
    f32 values[2][5]; /* Recent per-group Z deltas used to smooth camera tracking. */
} M627CameraState;

typedef struct M627CollisionHit {
    s16 found; /* Set when a collision surface intersects the tested movement. */
    f32 distance; /* Hit fraction from zero to one, or 100.0f when no hit has been selected. */
    Vec normal; /* Surface normal at the selected hit point. */
} M627CollisionHit;

typedef struct M627TaperedCollisionVolume {
    Vec origin; /* Center of the higher end in rotated slope-collision space. */
    Vec lowerCenter; /* Center of the lower end in rotated slope-collision space. */
    f32 originRadius; /* Horizontal radius at origin, in collision-space units. */
    f32 lowerCenterRadius; /* Horizontal radius at lowerCenter, in collision-space units. */
    f32 angle; /* Side slope angle in degrees. */
} M627TaperedCollisionVolume;
typedef struct M627WinnerList {
    s32 characterIds[4]; /* Unused winner slots remain -1. */
} M627WinnerList;
#endif
