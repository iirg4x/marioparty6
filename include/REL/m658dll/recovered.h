/* Shared gameplay work records and resource descriptions for T Minus Five. */
#ifndef REL_M658DLL_RECOVERED_H
#define REL_M658DLL_RECOVERED_H

#include "dolphin/types.h"
struct omObj_s;

typedef struct M658PlayerWork M658PlayerWork;

#include "dolphin/mtx/GeoTypes.h"
struct MgPlayer_s;
typedef struct M658PlayerChoiceRange {
    s32 minimumElapsedFrames, maximumElapsedFrames, selectionWeight;
} M658PlayerChoiceRange;
struct M658PlayerWork {
    u16 flags, actorFlags;
    s16 playerType;
    s8 state, nextState;
    s32 reserved08, frame; /* Only frame is used after the player work is cleared. */
    struct MgPlayer_s *player;
    s16 unused; /* Cleared on creation; the player's state callbacks never read it. */
    s16 playerNo, characterNo;
    s8 playerIndex, registryIndex;
    s32 action;
    Vec startPosition;
    s32 motionSlot;
    f32 actorRotation, rotation, spinY, spinZ;
    Vec motionStart, motionEnd;
    u8 reserved58[16]; /* Cleared at player setup and unused by the state callbacks. */
    struct M658HookActor *itemEffect; /* Pooled effect attached during the response to a missed row
                                       * selection. */
    void *materialColors; /* Saved actor material RGB bytes for the flash effect. */
    s16 difficulty, attemptCount, inputState, inputDelayFrames;
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
    u8 reserved00[2]; /* Cleared at row setup; row updates leave these bytes untouched. */
    s8 registryIndex[2];
    u8 reserved04[4]; /* Cleared at row setup; row updates leave these bytes untouched. */
    M658PlayerWork *entries[2][4];
    u8 reserved28[32]; /* Cleared at row setup; row updates leave these bytes untouched. */
    M658PlayerRow player[2];
    s32 modelCount;
    s16 collisionModels[4];
    s32 collisionModelCount;
    u16 *transitionObjects[2];
} M658Rows;

typedef struct M658Runtime {
    s16 timer, phase, substate;
    struct omObj_s *objects[14];
    u8 reserved40[8]; /* Cleared at runtime setup and unused by the sequence callbacks. */
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
    void (*update)(void *item, s32 event);
} M658RegistryEntry;

typedef struct M658RegistryWork {
    void *reserved00; /* Cleared at registry setup and unused by entry processing. */
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

#endif
