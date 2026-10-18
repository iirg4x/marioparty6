/* Sprite descriptions and character choices loaded by Mic Mode. */
#ifndef REL_MDMICDLL_DATA_STATE_H
#define REL_MDMICDLL_DATA_STATE_H

#include "dolphin/types.h"
#include "humath.h"

typedef struct {
    s16 groupNo; /* Sprite group receiving this entry. */
    s16 memberNo; /* Member slot within the group. */
    s16 animNo; /* Animation data entry for the member. */
    s16 priority; /* Sprite priority passed when the sprite is created. */
    s16 bank; /* Initial animation bank. */
    HuVec2f pos; /* Initial screen position. */
    HuVec2f scale; /* Horizontal and vertical sprite scale. */
    f32 zRot; /* Rotation passed to the sprite's Z-rotation setter. */
} MDMIC_SPRITE_DESC;

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
extern MDMIC_SPRITE_DESC lbl_1_data_C8[60];
extern HuVecF lbl_1_data_8BC[80];
extern s16 lbl_1_data_CA2[3];
extern s16 lbl_1_data_CA8[6];
extern u32 lbl_1_data_CB4[3];

#endif
