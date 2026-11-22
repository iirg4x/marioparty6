/* Shared scene, player, grid and effect state used by Cashapult REL code. */
#ifndef REL_M642DLL_WORK_H
#define REL_M642DLL_WORK_H

#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/mg/actman.h"
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

typedef struct  {
    s32 box; /* Score box handle. */
    MGSCORE *display; /* Numeric score display. */
    s32 value, spriteGroup, spriteCount; /* Score value, indicator group and member count. */
    u8 unk14[48]; /* No code names this field. */
} M642ScoreRecord;
typedef struct  {
    s32 resourceId; /* Packed animation resource number; a negative value ends the list. */
    ANIMDATA *animation; /* Loaded sprite animation. */
} M642SpriteAsset;
typedef struct  {
    u32 flags; /* Reuse and initialization state bits. */
    s32 resource, count, blendMode, model; /* Animation resource, particle capacity, blend mode and
                                            * model. */
    ANIMDATA *animation; /* Loaded particle animation. */
    u8 unknown18[4]; /* No code names this field. */
} M642ParticleOwner;
typedef struct  {
    s16 timer, state, phase, round, scoreCount; /* Scene timing, sequence state and round
                                                 * counters. */
    M642ScoreRecord players[4]; /* Score display records, indexed by team. */
    u8 unk11C[4]; /* No code names this field. */
    OMOBJ *scene_objects[8]; /* Scene, player, grid and effect objects. */
    M642ParticleOwner *owners; /* Particle settings for each team. */
    M642SpriteAsset *spriteAssets; /* Loaded score-indicator animations. */
    s32 playerGroups[4]; /* Team assignment for each player number. */
    s16 selection[2]; /* Selected grid row and column. */
    s32 sound; /* Scene sound handle, or -1 when inactive. */
    s16 soundState, soundTimer; /* Intro sound state and elapsed frames. */
    s32 active[2]; /* Per-team activity state used by player callbacks. */
    u8 unk16C[8]; /* No code names this field. */
} M642SceneWork;

typedef struct  {
    OMOBJMAN *manager; /* Actor manager used by this REL's object and process APIs. */
    u32 unk04; /* No code names this field. */
} M642ObjectManagerState;

typedef struct  {
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
    u32 owner; /* Team associated with the cell. */
    float radius; /* World-space collision radius for the cell. */
    float unk0C; /* The game's code never reads this field. */
    Vec position, rotation; /* Cell model's world position and rotation. */
    void (*update)(M642GridCell *); /* Per-frame callback for the cell's current state. */
    void (*collect)(M642PlayerWork *, M642GridCell *); /* Callback when a player reaches the
                                                        * cell. */
};
typedef struct M642PoolNode_s M642PoolNode;
struct M642PoolNode_s {
    struct M642PoolNode_s *prev, *next; /* Neighbor links in the active or free list. */
};
typedef struct M642Pool {
    M642PoolNode *active, *free; /* Heads of the used-item and available-item lists. */
    u8 unknown08[8]; /* No code names this field. */
} M642Pool;

typedef struct M642TrackKeyView {
    s32 frame; /* Animation frame; a negative value terminates the key list. */
    f32 values[3]; /* Up to three values sampled from this key. */
} M642TrackKeyView;

typedef struct M642TrackView {
    s16 kind; /* Interpolation mode used for this track. */
    s16 count; /* Number of values sampled from each key. */
    s16 end; /* Last frame at which the track is active. */
    M642TrackKeyView *keys; /* Ordered animation keys, terminated by a negative frame. */
} M642TrackView;

typedef struct  {
    u32 flags; /* Particle behavior flags, including alternating-size animation. */
    GXColor endColor; /* Initial RGBA at emission and the target RGB for interpolation. */
    GXColor startColor; /* Starting RGB for interpolation. Alpha is updated separately. */
    Vec velocity, decay; /* Initial velocity and per-frame velocity multipliers. */
    float gravity; /* Vertical velocity added each frame. */
    float unk28; /* No code names this field. */
    float scaleDelta; /* Per-frame change to the particle's base size. */
    float alphaDelta; /* Per-frame change to particle transparency. */
    float colorTime; /* Interpolation amount between the configured tint colors. */
} M642ParticleConfig;

struct M642ResourceTableView_s;

typedef struct  {
    M642PoolNode *active, *free; /* Head nodes for used cells and available cells. */
    u8 unknown08[8]; /* No code names this field. */
    struct {
        M642PoolNode links;
        M642GridCell cell;
    } nodes[64]; /* Fixed storage for the grid's pooled cell records. */
} M642CellPoolStorage;
typedef struct  {
    float min, max; /* World-space limits along one grid axis. */
} M642Bounds;
typedef struct  {
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

typedef struct  {
    Vec position, target; /* Camera position and look-at point for this team. */
    M642PlayerWork *active; /* Player currently active for this team's turn. */
} M642TeamOwner;
typedef struct  {
    s16 mode, timer; /* Camera transition state and elapsed frames. */
    u8 unknown04[12]; /* No code names this field. */
    M642TeamOwner teams[2]; /* Active-player state for the two teams. */
} M642RoundControl;

typedef struct  {
    u8 unknown00[4]; /* No code names this field. */
    u8 poolStorage[4112]; /* Backing storage for pooled effect records. */
    M642Pool *pool; /* Active/free lists built over poolStorage. */
} M642EffectWork;

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
void fn_1_5CC(void);
s32 fn_1_64C(void);
s32 fn_1_698(void);
void fn_1_318(MGPLAYER *playerP);
void fn_1_31C(MGPLAYER *playerP);
void fn_1_7878(s32 playerIndex, s16 minigameCoins);
s16 fn_1_7890(void);
void *fn_1_8568(void *work);
M642SpriteAsset *fn_1_859C(M642SpriteAsset *assets);
void *fn_1_8794(M642Pool *pool);
void *fn_1_87D0(void *pool, void *item);
void fn_1_89F8(Point3d *start, Point3d *end, f32 t, Point3d *out);
s32 fn_1_8F9C(Vec *position, s32 camera, float nearPan, float farPan, s32 sound);
s32 fn_1_9008(HuVecF *input, f32 lower, f32 upper, s32 soundEffect);
s32 fn_1_9120(s32 soundId);
void *fn_1_91EC(s32 index);
void *fn_1_9250(s32 index);
M642SpriteAsset *fn_1_92B4(void);

void *fn_1_86B0(M642Pool *pool);
s32 fn_1_8814(M642TrackView **tracks, s32 frame);
void fn_1_8870(M642TrackView **tracks, s32 frame, s32 trackIndex, f32 *outValues);
s32 fn_1_8A50(Point3d *source, Point3d *destination);
s16 fn_1_7D1C(M642ParticleOwner *owner, s16 camera, float x, float y, float z,
              float scale, M642ParticleConfig *config);
OMOBJ *fn_1_84D8(struct M642ResourceTableView_s *capacity);
s16 fn_1_8E68(Vec *position, s32 camera, float nearPan, float farPan);

#endif
