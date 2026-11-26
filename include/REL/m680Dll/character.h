/* Strawberry Shortfuse: player model, movement and result state. */
#ifndef REL_M680DLL_CHARACTER_H
#define REL_M680DLL_CHARACTER_H

#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "game/object.h"

/* Ranking records carry a player index through the turn-order sort. */
typedef struct M680PlayerRankEntry { s32 rank, player; } M680PlayerRankEntry;

typedef struct M680CharacterState {
    OMOBJ *object;
    Vec position;
    Vec rotation;
    s16 modelId;
    s16 motionIds[10];
    s16 playerIndex;
    s16 character;
    s16 unk36; /* The game's code never reads this field. */
    s16 unk38; /* No code names this field. */
    s16 padNo;
    u8 unk3C[4]; /* No code names this field. */
    s16 playerType;
    s16 unk42; /* The game's code never reads this field. */
    s16 unk44; /* No code names this field. */
    s16 playerSlot;
    f32 targetAngle;
    f32 unk4C; /* The game's code never reads this field. */
    s32 state;
    s32 frame;
    Vec origin;
    Vec destination;
    f32 endAngle;
    s32 travelFrames;
    /* Nonzero after this player leaves the active turn rotation. */
    s32 eliminated;
} M680CharacterState;

void fn_1_AB70(s32 count, M680PlayerRankEntry *entries);
void fn_1_6538(M680CharacterState *work, s32 player);
void fn_1_AD38(OMOBJ *object);
void fn_1_B844(M680CharacterState *work, s32 targetIndex, s32 speedIndex);
void fn_1_BAF8(M680CharacterState *state);
void ObjectSetup(void);
void fn_1_9600(void);

extern u32 lbl_1_data_6C0[10];

#endif
