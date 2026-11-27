/* Shared gameplay work records and resource descriptions for T Minus Five. */
#ifndef REL_M658DLL_RECOVERED_H
#define REL_M658DLL_RECOVERED_H

#include "dolphin/types.h"
#include "game/object.h"
struct omObj_s;

typedef struct M658PlayerWork M658PlayerWork;
typedef struct M658Pool M658Pool;

#include "dolphin/mtx/GeoTypes.h"
struct MgPlayer_s;
typedef struct M658HookActor {
    u16 flags; /* Allocation, active state, and release flags. */
    s16 unk2; /* The game's code never reads this field. */
    u8 unk4[2]; /* No code names this field. */
    s16 model;
    u8 unk8[48]; /* No code names this field. */
    s32 hostModel;
    char *hook;
} M658HookActor;

typedef struct M658PlayerChoiceRange {
    s32 minimumElapsedFrames, maximumElapsedFrames, selectionWeight;
} M658PlayerChoiceRange;
struct M658PlayerWork {
    u16 flags, actorFlags;
    s16 playerType;
    s8 state, nextState;
    u8 unk8[4]; /* No code names this field. */
    s32 frame;
    struct MgPlayer_s *player;
    s16 unk14; /* The game's code never reads this field. */
    s16 playerNo, characterNo;
    s8 playerIndex, registryIndex;
    s32 action;
    Vec startPosition;
    s32 motionSlot;
    f32 actorRotation, rotation, spinY, spinZ;
    Vec motionStart, motionEnd;
    u8 unk58[16]; /* No code names this field. */
    M658HookActor *itemEffect; /* Pooled effect attached during the response to a missed row
                                       * selection. */
    void *materialColors; /* Saved actor material RGB bytes for the flash effect. */
    s16 difficulty, attemptCount;
    s16 unk74; /* The game's code never reads this field. */
    s16 inputDelayFrames;
    s16 interval, elapsedInputFrames, firstLimit, secondLimit;
    M658PlayerChoiceRange *choiceRange;
};

typedef struct M658PlayerRow {
    s16 mode;
    s16 timer;
    s16 position;
    s16 step;
    s16 interval;
    s16 models[6];
} M658PlayerRow;

typedef struct M658Rows {
    u8 unk0[2]; /* No code names this field. */
    s8 registryIndex[2];
    u8 unk4[4]; /* No code names this field. */
    M658PlayerWork *entries[2][4];
    u8 unk28[32]; /* No code names this field. */
    M658PlayerRow player[2];
    s32 modelCount;
    s16 collisionModels[4];
    s32 collisionModelCount;
    M658HookActor *transitionObjects[2];
} M658Rows;

typedef struct M658Runtime {
    s16 timer, phase, substate;
    struct omObj_s *objects[14];
    u8 unk40[8]; /* No code names this field. */
    s32 group[4];
    M658PlayerWork *selected;
    M658PlayerWork *other;
    M658PlayerWork *available[4];
    s32 count;
    s16 stateFlags;
    s16 motionActive;
    s16 flags;
    s32 motionEventIndex;
    s32 motionEventSet;
} M658Runtime;

typedef struct M658RegistryEntry {
    void *pool;
    void (*render)(void);
    void (*update)(M658HookActor *item, s32 event);
} M658RegistryEntry;

typedef struct M658RegistryWork {
    u8 unk0[4]; /* No code names this field. */
    M658RegistryEntry entries[7];
    s32 count;
} M658RegistryWork;

typedef struct M658ResourceEntry {
    s32 resourceId, type, modelAttributes;
    s16 linkedModelCount, targetModelIndex;
} M658ResourceEntry;

extern M658RegistryWork *lbl_1_bss_18;
extern s32 lbl_1_bss_8;
extern M658Rows *lbl_1_bss_24;
extern M658Runtime *lbl_1_bss_38;

/* Model resources, effects, and interpolation. */
void fn_1_7048(s32 model, u8 *colors);
void fn_1_70F8(s32 model, const s8 *colors, f32 scale);
void fn_1_6414(OMOBJ *object);
M658ResourceEntry *fn_1_753C(s32 index);
OMOBJ *fn_1_6384(M658ResourceEntry *description);
void fn_1_736C(HuVecF *worldPosition, int soundId);
void *fn_1_6764(M658Pool *pool);
void *fn_1_67A0(M658Pool *pool, void *item);
s32 fn_1_7468(s32 soundId);
void fn_1_571C(s32 player, s16 amount);
void fn_1_7244(s32 modelId, char *objectName, s32 targetModelId);
s8 *fn_1_6F70(s32 model);
s16 fn_1_5770(void);
f32 fn_1_69B8(f32 startValue, f32 endValue, f32 interpolation);
f32 fn_1_6A78(f32 startDegrees, f32 endDegrees, f32 interpolation);
void fn_1_69C8(Point3d *start, Point3d *end, f32 t, Point3d *out);
s32 fn_1_72D0(s32 modulus, s32 count, s32 *weights);
void *fn_1_65E8(u8 *buffer, s32 bytes, s32 itemBytes);
void *fn_1_6680(M658Pool *pool);
void fn_1_55FC(OMOBJ *object);

/* Setup and shared game state. */
void *fn_1_A0(s32 priority, u32 bytes, void (*update)(OMOBJ *));
void ObjectSetup(void);
void fn_1_264(void);
void fn_1_140(OMOBJ *object, OMOBJ_FUNC callback);

/* Player and sequence updates. */
void fn_1_768(void);
s32 fn_1_C38(void);
s32 fn_1_C84(void);
s32 fn_1_CD0(void);
s32 fn_1_D40(void);

#endif
