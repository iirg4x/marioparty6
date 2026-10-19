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
    s16 group;
    s16 unknown38;
    s16 padNo;
    u8 unknown3C[4];
    s16 playerType;
    s16 difficulty;
    s16 unknown44;
    s16 playerSlot;
    f32 targetAngle;
    /* Cleared on creation and each update; no other use in this module. */
    f32 clearedValue;
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

extern u32 lbl_1_data_6C0[10];

#endif
