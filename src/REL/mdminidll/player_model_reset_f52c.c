#define _MATH_H
#include "game/hu3d.h"
#include "game/object.h"
#include "game/sprite.h"
#include "REL/mdminidll/model_animation.h"
#include "REL/mdminidll/player_config.h"
#include "REL/mdminidll/readonly_scalars.h"

typedef struct MDMinidllSelectionView {
    u8 unknown_00[16];
    s16 word_10;
} MDMinidllSelectionView;

extern MDMinidllSelectionView lbl_1_data_89A[];
extern s16 lbl_1_data_86E[];
extern OMOBJ *lbl_1_bss_1C;
extern OMOBJ *lbl_1_bss_24;
extern HUSPR_GROUPID lbl_1_bss_752[];

void fn_1_F52C(OMOBJ *arg0)
{
    MDMinidllPlayerConfig *config;
    s16 playerIndex;

    playerIndex = 0;
    config = lbl_1_bss_7CC;
    while (playerIndex < 4) {
        if (config->comF == 1) {
            if (config->unknown_00 == 0) {
                OMOBJ *models;

                lbl_1_data_89A[config->charNo].word_10 = -1;
                models = lbl_1_bss_1C;
                Hu3DModelAttrSet(models->mdlId[playerIndex], 1U);
                HuSprScaleSet(lbl_1_bss_752[3], (s16)playerIndex,
                              lbl_1_rodata_1C4, lbl_1_rodata_1C4);
                HuSprAttrSet(lbl_1_bss_752[3], (s16)playerIndex, 4);
            } else if (config->unknown_00 == 1) {
                HuSprAttrSet(lbl_1_bss_752[2], 4, 4);
                lbl_1_data_86E[4] = 0;
                HuSprAttrSet(lbl_1_bss_752[2], 5, 4);
                lbl_1_data_86E[5] = 0;
            } else {
                goto next_player;
            }
            break;
        } else {
        next_player:
            playerIndex += 1;
            config += 1;
            continue;
        }
    }

    playerIndex = 0;
    while (playerIndex < 11) {
        MDMinidllModelAnimationRecord *entry;

        entry = &lbl_1_bss_230[playerIndex];
        Hu3DModelRotSet(entry->model, lbl_1_rodata_74,
                        lbl_1_rodata_74, lbl_1_rodata_74);
        playerIndex += 1;
    }

    arg0->objFunc = NULL;
    *lbl_1_bss_24->mtnId = 1;
}
