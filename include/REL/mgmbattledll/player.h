/* Character, controller and round state for each Battle Bridge player. */
#ifndef REL_MGMBATTLEDLL_PLAYER_H
#define REL_MGMBATTLEDLL_PLAYER_H

#include "dolphin/mtx.h"

typedef struct MgBattleRecord_s {
    /* No player routine reads or writes these leading bytes. */
    u8 unknownBytesAtStart[2];
    /* Character selected for this player. */
    s16 charNo;
    /* Controller port assigned to this player. */
    s16 padNo;
    /* Team or partner group used for tied-score awards. */
    s16 grpNo;
    /* Human or computer player type from the game settings. */
    s32 playerType;
    /* Computer difficulty from the game settings. */
    s16 comDif;
    /* Battle sequence state for this player. */
    s16 mode;
    /* Character model handle. */
    s16 model;
    /* Loaded character motion IDs, indexed by the battle motion table. */
    s16 motions[12];
    /* Current entry in the battle motion table. */
    s16 currentMotion;
    /* Character position in stage coordinates. */
    Point3d position;
    /* Character Euler rotation in degrees. */
    Point3d rotation;
    /* Character model height in stage units. */
    f32 height;
    /* Stage position approached during the award sequence. */
    Point3d awardPosition;
    /* Horizontal model scale used by character setup. */
    f32 scaleX;
    /* Vertical movement speed in stage units per update. */
    f32 verticalVelocity;
    /* Depth model scale used by character setup. */
    f32 scaleZ;
    /* Per-player state used by the round and character animation flow. */
    s16 state;
} MgBattleRecord;

extern MgBattleRecord lbl_1_bss_4D4[4];

extern Point3d lbl_1_data_278;
extern Point3d lbl_1_data_6A8;
extern Point3d lbl_1_data_6B4;

#endif
