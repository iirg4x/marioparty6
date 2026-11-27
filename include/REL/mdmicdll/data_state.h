/* Sprite descriptions and character choices loaded by Mic Mode. */
#ifndef REL_MDMICDLL_DATA_STATE_H
#define REL_MDMICDLL_DATA_STATE_H

#include "REL/minigame_menu_sprite.h"
#include "dolphin/types.h"
#include "humath.h"

typedef struct {
    s16 neighborCharacter[8]; /* Neighbor choices used to move between characters. */
    s16 selectionState; /* -1 makes this character available; 1 reserves it for a player. */
} MDMIC_CHARACTER_CHOICE;

extern s32 lbl_1_data_0;
extern s32 lbl_1_data_C7C;
extern s16 lbl_1_data_E32[3];
extern s32 lbl_1_data_E38[3];
extern s16 lbl_1_data_E64[6];
extern MDMIC_CHARACTER_CHOICE lbl_1_data_E90[11];

extern u32 lbl_1_data_24[32];
extern MinigameMenuSprite lbl_1_data_C8[60];
extern HuVecF lbl_1_data_8BC[80];
extern s16 lbl_1_data_CA2[3];
extern s16 lbl_1_data_CA8[6];
extern u32 lbl_1_data_CB4[3];

#endif
