/* Talkie Walkie state layouts shared by its player, map, and effect routines. */
#ifndef REL_M667_RECOVERED_H
#define REL_M667_RECOVERED_H

#include "game/object.h"
#include "game/mg/seqman.h"
#include "game/mg/actman.h"
#include "game/sprite.h"

/* State views shared by Talkie Walkie's scene, player, map, and effect callbacks. */
typedef struct M667SceneView {
    s16 modeTimer; /* Frame counter used by the current scene mode. */
    s16 mode; /* Current scene-sequence mode. */
    s16 modePhase; /* Step within the current scene-sequence mode. */
    s16 outcome; /* 1: hit, 2: lost contact, 3: timeout, 4: escaped. */
    OMOBJ *cameraObject;
    OMOBJ *collisionObject;
    OMOBJ *playerObject;
    OMOBJ *entryObject;
    OMOBJ *mapObject;
    OMOBJ *singlePlayerResultObject;
    OMOBJ *multiplayerResultObject;
    u8 reservedBeforeGroups[4]; /* Reserved bytes between scene objects and group state. */
    s32 groups[4]; /* Assigned group for each of the four players. */
    s32 majorityGroup; /* Group containing the larger player team. */
    struct M667PlayerView *entries[3]; /* Players recorded for the ending sequence. */
    u8 reservedBeforeEntryCount[8]; /* Reserved bytes between entry pointers and entry state. */
    s16 entryCount; /* Number of recorded ending-sequence players, up to three. */
    /* Count of initialized majority-team AI players; scales a stored move interval that
     * their AI callback does not read. */
    s16 aiPlayerIndex;
    /* Set after requesting listener and selection-window setup; cleared at shutdown. */
    s32 micInputRequested;
    s32 micContext; /* Microphone context created during scene setup. */
    s32 controlBits; /* Latest recognized direction bits; player input consumes and clears them. */
    s32 soundHandle; /* Handle of the sound tracked by the scene sequence. */
    s16 soundState; /* Step in the scene's timed sound sequence. */
    s16 soundAge; /* Frame counter used by the timed sound sequence. */
} M667SceneView;

typedef struct M667CameraView {
    s16 mode;
    u8 reservedAfterMode[2]; /* Reserved bytes between the mode and camera timer. */
    s16 timer; /* Cleared on camera mode or motion changes; no camera callback reads or advances
                * this word. */
    u8 reservedBeforeMotionIndex[10]; /* Reserved bytes between the timer and motion index. */
    s32 motionIndex;
    HuVecF position;
    HuVecF target;
} M667CameraView;

/* Each active tile slot holds its grid cell, model, and saved material colors. */
typedef struct M667MapSlot {
    s32 enabled;
    s16 x;
    s16 z;
    u8 reservedBeforeModel[2]; /* Reserved bytes between the grid cell and model handle. */
    HU3D_MODELID model;
    void *data;
} M667MapSlot;

/* Each record tracks one moving map model and its animation progress. */
typedef struct M667MapModel {
    HU3D_MODELID model;
    s16 elapsedFrames;
    s16 index;
    s16 durationFrames;
    u8 reservedAfterFrame[12]; /* Reserved bytes after the moving model's path duration. */
} M667MapModel;

typedef struct M667MapView {
    u8 reservedBeforeGroupCounts[2]; /* Reserved bytes before the group counts. */
    s8 groupCount[4];
    struct M667PlayerView *players[4][4];
    struct M667PlayerView *currentPlayer;
    M667MapSlot slots[4];
    u8 reservedAfterSlots[4]; /* Reserved bytes between map slots and model records. */
    M667MapModel records[2];
    s16 modelIds[4];
    s32 modelCount;
} M667MapView;

/* Entry actors use height = a * distance * distance + b * distance + c for their arc.
 * The resulting value is subtracted from the entry's starting world height. */
typedef struct M667QuadraticView {
    f32 a, b, c; /* Quadratic, linear, and constant height terms, respectively. */
} M667QuadraticView;

/* Each entry carries its owning player index, actor, model, position, and motion state. */
typedef struct M667Entry {
    s16 ownerPlayerIndex;
    s16 mode;
    s16 timer;
    HU3D_MODELID modelId;
    s16 x;
    s16 z;
    f32 radius;
    f32 travelDistance;
    f32 distancePerFrame;
    HuVecF position;
    HuVecF direction;
    MGACTOR *actor;
    M667QuadraticView curve;
    u8 reservedAfterCurve[4]; /* Reserved bytes at the end of each entry actor's state. */
} M667Entry;

typedef struct M667Entries {
    u8 reservedBeforeWord[2]; /* Unused leading bytes before the initialized word and entry
                                     * array. */
    s16 reservedWord; /* Word initialized to zero before the entry array. */
    M667Entry entries[12];
} M667Entries;

/* Effect records hold lifecycle state, age, an optional model, and world position. */
typedef struct M667ModelEffectView {
    u16 flags;
    s16 age;
    u8 reservedAfterAge[2]; /* Reserved bytes between effect age and model handle. */
    s16 model;
    s16 stopped; /* Trails stop on the floor contact after their third bounce,
                  * or below Y -500 while descending. */
    s16 pointCount; /* Number of sampled positions and colors in a line strip. */
    union {
        s16 bounceCount; /* Floor reflections completed by a trail. */
        s16 childCount; /* Trails created by a particle controller. */
    } counts;
    u8 reservedBeforeAlpha[6]; /* Reserved bytes between child count and trail opacity. */
    f32 alpha; /* Trail opacity, reduced on each update. */
    u8 reservedAfterAlpha[12]; /* Reserved bytes between trail opacity and world position. */
    HuVecF position;
    HuVecF velocity; /* Trail displacement applied on each update. */
    u8 reservedAfterVelocity[4]; /* Reserved bytes between velocity and the trail point list. */
    struct M667ParticlePointView *points; /* Newest point first, followed by older points. */
    struct M667ModelEffectView **children; /* Trails emitted by a particle controller. */
} M667ModelEffectView;

/* Each effect registry entry pairs a pool with its draw and update callbacks. */
typedef struct M667RegistryEntryView {
    void *pool; /* Active and free lists for one effect type. */
    void (*render)(void); /* Shared drawing setup for this effect type, if present. */
    void (*update)(M667ModelEffectView *item, s32 event); /* Per-item lifecycle callback. */
} M667RegistryEntryView;

typedef struct M667RegistryView {
    u8 reservedBeforeEntries[4]; /* Reserved bytes before the effect registry entries. */
    M667RegistryEntryView entries[8];
    s32 count; /* Number allocated from the final pool during registry setup. */
    s32 model; /* Model handle whose draw hook processes the registered effects. */
} M667RegistryView;

/* Sprite-animation lists end at a negative resource ID. */
typedef struct M667ParticleAnimEntry {
    s32 resourceId; /* Data number to load, or a negative list terminator. */
    ANIMDATA *anim; /* Loaded sprite-animation reference. */
} M667ParticleAnimEntry;

/* Pool nodes link the active and available effect lists. */
typedef struct M667PoolNode {
    struct M667PoolNode *prev; /* Previous node in the linked list. */
    struct M667PoolNode *next; /* Next node in the linked list. */
} M667PoolNode;

typedef struct M667Pool {
    M667PoolNode *active; /* Head of the items allocated from this pool. */
    M667PoolNode *free; /* Head of the items available for reuse. */
    u8 reservedAfterLists[8]; /* Pool initialization clears these bytes; list operations leave
                               * them unchanged. */
} M667Pool;

/* Each track key supplies a frame number and one value for each animated channel. */
typedef struct M667TrackKey {
    s32 frame; /* Key frame; a negative value terminates the key list. */
    f32 *values; /* Channel values at this key frame. */
} M667TrackKey;

typedef struct M667Track {
    s16 kind; /* Zero selects linear interpolation; other kinds leave samples unchanged. */
    s16 count; /* Number of values sampled from each key. */
    s16 end; /* Last frame considered active by the completion query. */
    M667TrackKey *keys; /* Frame-ordered keys ending at a negative frame number. */
} M667Track;

/* Model-object setup reads these descriptors until kind 8 terminates the list. */
typedef struct M667ModelDescription {
    s32 dataId; /* Resource data number; negative values skip the resource read. */
    s32 kind; /* 0/2: model, 1: joint motion, 3: camera motion, 5: skipped, 8: end. */
    s32 attr; /* Model or motion attributes applied when loading a joint motion. */
    s16 count; /* Model copies requested by kinds 0/2; nonpositive values create one. */
    s16 modelIndex; /* Model slot receiving kind 1's motion; negative skips attachment. */
} M667ModelDescription;

/* AI rows hold decision chance and the weighted choices used by computer players. */
typedef struct M667AIView {
    s32 actionCadence;
    u32 chance;
    s32 directionWeights[3];
    s32 attackWeights[3];
    s32 unusedValues[5]; /* Additional integer table entries not read by the AI callbacks. */
} M667AIView;

typedef struct M667PlayerView {
    u16 flags;
    u16 flags2;
    s16 controlMode;
    s8 actionMode;
    s8 nextActionMode;
    s8 actionStep;
    MGPLAYER *player;
    s16 actionTimer;
    s16 rotationTimer;
    s16 reservedWord;
    s16 index;
    s16 charNo;
    s8 group;
    s8 slot;
    u8 reservedBeforePosition[4]; /* Reserved bytes between player identity and position. */
    HuVecF position;
    s32 motion;
    f32 rotationStartY;
    f32 rotationCurrentY;
    f32 rotationTargetY;
    u8 reservedBeforeStartPosition[8]; /* Reserved bytes between rotation state and map path. */
    /* Map movement stores its previous and next world positions here. */
    HuVecF startPos;
    HuVecF targetPos;
    u8 reservedBeforeVelocity[12]; /* Reserved bytes between map path and velocity. */
    HuVecF velocity;
    u8 reservedBeforeMapCoordinates[8]; /* Reserved bytes between velocity and map coordinates. */
    s16 mapX;
    s16 mapZ;
    s16 mapSlotIndex;
    /* Entry handle used while carrying an entry actor; -1 means none is held. */
    s16 entryId;
    char *itemHook;
    u8 reservedBeforeDifficulty[2]; /* Reserved bytes between item hook and AI difficulty. */
    s16 difficulty;
    u8 reservedBeforeAiIntervals[2]; /* Reserved bytes between difficulty and AI timing. */
    s16 aiMoveInterval;
    s16 aiActionInterval;
    s16 aiIntervalVariance;
    s16 aiMoveTimer;
    s16 aiAttackTimer;
    M667AIView *ai;
    s32 controlBits;
} M667PlayerView;

extern HUPROCESS *lbl_1_bss_0;
extern M667Entries *lbl_1_bss_8;
extern M667RegistryView *lbl_1_bss_C;
extern M667CameraView *lbl_1_bss_10;
extern M667MapView *lbl_1_bss_18;
extern M667SceneView *lbl_1_bss_2C;
extern MGSEQ_PARAM lbl_1_data_0;
extern s32 lbl_1_data_28;
extern void *lbl_1_data_AB8[7];
extern void *lbl_1_data_AD4[7];

void fn_1_19C(void);
void fn_1_66AC(M667ModelEffectView *item, s32 event);
void fn_1_5C0(M667SceneView *work);
u16 *fn_1_2098(u32 index, HuVecF *position);
void fn_1_140(OMOBJ *object, OMOBJ_FUNC callback);
void fn_1_D6C(s32 index, HuVecF *position);
void fn_1_1554(s32 index);
void fn_1_1258(s32 index);
s8 *fn_1_895C(s32 model);
void fn_1_16B8(s32 index, void *color);
void fn_1_1628(s32 index);
void fn_1_1810(s32 index, s32 x, s32 z);
s32 fn_1_1AD4(s32 index, s32 x, s32 z);
void fn_1_1338(s32 index);
void fn_1_1E5C(M667PlayerView *state, s32 mode);
s32 fn_1_1EB8(M667PlayerView *work, s16 frames);
s16 fn_1_6C44(void);
void *fn_1_A0(s32 priority, u32 bytes, void (*update)(OMOBJ *));
void fn_1_2B68(OMOBJ *object);
void fn_1_768(void);
void fn_1_7A4(void);
s32 fn_1_B90(void);
s32 fn_1_BDC(void);
s32 fn_1_C28(void);
void fn_1_551C(OMOBJ *object);
void fn_1_5580(OMOBJ *object);
void fn_1_5898(OMOBJ *object);
void fn_1_2644(s16 entryIndex);
void fn_1_21CC(s16 index, s32 mode);
M667Entry *fn_1_2230(s16 index);
f32 fn_1_80AC(HuVecF *start, HuVecF *end);
s32 fn_1_8544(HuVecF *source, HuVecF *destination);
void fn_1_8254(M667QuadraticView *out, HuVecF *start, HuVecF *peak, HuVecF *end);
f32 fn_1_84E8(M667QuadraticView *curve, f32 x);
void fn_1_7664(void *work);
void fn_1_77D0(M667ParticleAnimEntry *entries);
void fn_1_7350(void *description, s32 *models, s32 *motions);
void fn_1_7424(OMOBJ *object, M667ModelDescription *description);
OMOBJ *fn_1_7868(void *description);
void fn_1_7AC8(void *item);
void *fn_1_7B10(u8 *buffer, s32 bytes, s32 itemBytes);
void *fn_1_7BA8(M667Pool *pool);
void fn_1_7C1C(M667Pool *pool, void *item);
void *fn_1_7C8C(void *pool);
void *fn_1_7CC8(void *pool, void *item);
s16 fn_1_8FC4(void);

#endif
