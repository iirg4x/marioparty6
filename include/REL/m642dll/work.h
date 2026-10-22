/* Shared scene, player, grid and effect state used by Cashapult REL code. */
#ifndef REL_M642DLL_WORK_H
#define REL_M642DLL_WORK_H

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

typedef struct M642ScoreRecord_s {
    s32 box; /* Score box handle. */
    MGSCORE *display; /* Numeric score display. */
    s32 value, spriteGroup, spriteCount; /* Score value, indicator group and member count. */
    u8 unreadData[48]; /* Remaining team record bytes are not read by Cashapult. */
} M642ScoreRecord;
typedef struct M642SpriteAsset_s {
    u32 unreadWord; /* Asset record word not read by the Cashapult source. */
    ANIMDATA *animation; /* Loaded sprite animation. */
} M642SpriteAsset;
typedef struct M642ParticleOwner_s {
    u32 flags; /* Reuse and initialization state bits. */
    s32 resource, count, blendMode, model; /* Animation resource, particle capacity, blend mode and
                                            * model. */
    ANIMDATA *animation; /* Loaded particle animation. */
    u8 unknown18[4]; /* No Cashapult code reads these bytes of the particle-owner record. */
} M642ParticleOwner;
typedef struct M642SceneWork_s {
    s16 timer, state, phase, round, scoreCount; /* Scene timing, sequence state and round
                                                 * counters. */
    M642ScoreRecord players[4]; /* Two team's score displays and related record data. */
    u8 alignmentPadding[4]; /* Aligns the following object pointers. */
    OMOBJ *scene_objects[8]; /* Scene, player, grid and effect objects. */
    M642ParticleOwner *owners; /* Particle settings for each team. */
    M642SpriteAsset *spriteAssets; /* Loaded score-indicator animations. */
    s32 playerGroups[4]; /* Team assignment for each player number. */
    s16 selection[2]; /* Selected grid row and column. */
    s32 sound; /* Background stream handle, or -1 when inactive. */
    s16 soundState, soundTimer; /* Background sound state and elapsed frames. */
    s32 active[2]; /* Per-team activity state used by player callbacks. */
    u8 trailingData[8]; /* Scene record bytes not read by Cashapult. */
} M642SceneWork;

typedef struct M642ObjectManagerState_s {
    OMOBJMAN *manager; /* Actor manager used by this REL's object and process APIs. */
    u32 reserved; /* Unread word following the manager pointer. */
} M642ObjectManagerState;

typedef struct M642PlayerWork_s {
    u16 flags; /* Player behavior flags used by the movement callbacks. */
    s16 computer; /* Nonzero when this player is controlled by the computer. */
    s8 state, nextState; /* Current movement state and the state queued for the next update. */
    s32 sound, timer; /* Active sound handle and frames elapsed in the current state. */
    MGPLAYER *player; /* Minigame player and its character actor. */
    s16 initialized, playerNo, charNo; /* Setup state, player index and character model index. */
    s8 team, slot; /* Team index and position within that team's player list. */
    Vec position; /* Current world position used by the movement callbacks. */
    s32 motion; /* Current motion slot, or the value used before one is selected. */
    float gravity; /* Vertical acceleration applied during a jump. */
    Vec velocity, direction; /* Movement velocity and input/facing direction. */
    float baseRotation, rotation; /* Board-facing angle and current character rotation. */
    Vec startingPosition, targetPosition; /* World-space endpoints for movement. */
    s16 difficulty, aiState, aiFrames, steeringFrames; /* CPU level and AI movement state timers. */
    s32 jumps; /* Number of jump actions taken by this player. */
    Vec nextStep, savedTarget; /* Current AI steering direction and saved world-space target. */
    s32 *aiParams; /* Difficulty-specific CPU movement parameters. */
} M642PlayerWork;

typedef struct M642GridCell_s M642GridCell;
struct M642GridCell_s {
    u16 flags; /* Cell state flags checked by grid and player callbacks. */
    HU3D_MODELID model; /* Model displayed for this grid cell. */
    u32 owner; /* Team or player ownership value associated with the cell. */
    float radius, height; /* World-space collision dimensions for the cell. */
    Vec position, rotation; /* Cell model's world position and rotation. */
    void (*update)(u8 *); /* Per-frame callback for the cell's current state. */
    void (*collect)(M642PlayerWork *, M642GridCell *); /* Callback when a player reaches the
                                                        * cell. */
};
typedef struct M642PoolNode_s {
    struct M642PoolNode_s *prev, *next; /* Neighbor links in the active or free list. */
} M642PoolNode;
typedef struct M642CellPoolStorage_s {
    M642PoolNode *active, *free; /* Head nodes for used cells and available cells. */
    u8 unknown08[8]; /* Header bytes whose purpose is not established by this source. */
    struct {
        M642PoolNode links;
        M642GridCell cell;
    } nodes[64]; /* Fixed storage for the grid's pooled cell records. */
} M642CellPoolStorage;
typedef struct M642Bounds_s {
    float min, max; /* World-space limits along one grid axis. */
} M642Bounds;
typedef struct M642GridWork_s {
    u32 flags; /* Grid-wide state flags. */
    M642PlayerWork *players[2][2]; /* Up to two player work records for each team. */
    u8 count[2]; /* Number of registered players on each team. */
    s8 active; /* Team-list slot selected for the current turn. */
    M642CellPoolStorage cellStorage[2]; /* Backing cell pools for the two grid sides. */
    void *pools[2]; /* Active/free cell pools for each grid side. */
    M642GridCell *grid[2][9][9]; /* Cell lookup by side, row and column. */
    s32 motion, step, row; /* Active grid motion, animation step and current row. */
    M642Bounds bounds[2]; /* World-space horizontal limits for each side. */
    s32 cellModels, effectModels; /* Model slots consumed by cells and grid effects. */
} M642GridWork;

typedef struct M642TeamOwner_s {
    Vec position, target; /* Camera position and look-at point for this team. */
    M642PlayerWork *active; /* Player currently active for this team's turn. */
} M642TeamOwner;
typedef struct M642RoundControl_s {
    s16 mode, timer; /* Camera transition state and elapsed frames. */
    u8 unknown04[12]; /* No Cashapult code reads these bytes before the team camera records. */
    M642TeamOwner teams[2]; /* Active-player state for the two teams. */
} M642RoundControl;

struct M667Pool;
typedef struct M642EffectWork_s {
    u8 unknown00[4]; /* No Cashapult code reads these bytes before the effect-item pool buffer. */
    u8 poolStorage[4112]; /* Backing storage for pooled effect records. */
    struct M667Pool *pool; /* Active/free lists built over poolStorage. */
} M642EffectWork;

typedef struct M642WorkView_s {
    u8 unidentifiedPrefix[24]; /* Bytes before the first large work region. */
    u8 workBufferA[3600]; /* First fixed-size work region. */
    u8 workBufferB[3600]; /* Second fixed-size work region. */
    void *resourcePointer; /* Pointer stored after the two work regions. */
    void *auxiliaryPointer; /* Adjacent pointer field. */
    u8 unidentifiedTail[648]; /* Bytes before the final word. */
    s32 finalWord; /* Final word in this otherwise unused work view. */
} M642WorkView;

extern M642SceneWork *lbl_1_bss_2C;
extern M642GridWork *lbl_1_bss_18;
extern M642PlayerWork *lbl_1_bss_1C[4];
extern M642RoundControl *lbl_1_bss_14;
extern M642EffectWork *lbl_1_bss_C;
extern M642ObjectManagerState lbl_1_bss_0;
extern void *lbl_1_bss_8;
extern void *lbl_1_bss_10;

void *fn_1_A0(s32 priority, u32 bytes, void (*update)(OMOBJ *));
void fn_1_31CC(OMOBJ *object);
void fn_1_140(OMOBJ *object, OMOBJ_FUNC callback);
void *fn_1_8618(void *buffer, s32 bytes, s32 itemBytes);
void fn_1_11E4(s32 index, s32 value);
void fn_1_5080(OMOBJ *object);

#endif
