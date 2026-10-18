/* Reward and minigame result rows, with their numeric sprite displays. */
#ifndef REL_MDSINGDLL_SCORE_H
#define REL_MDSINGDLL_SCORE_H

#include "game/hu3d.h"
#include "game/sprite.h"

typedef struct MdsingScoreEntry {
    /* Message ID for this row; -1 leaves it blank. */
    s32 message;
    /* Numeric reward value; -1 suppresses the number. */
    s16 value;
    /* 1 marks a category heading; other values use the normal text color. */
    s16 color;
} MDSING_SCORE_ENTRY;

typedef struct MdsingResultModel {
    /* Rotating reward model beside this row or the result total. */
    HU3D_MODELID modelId;
    /* Digit group: four populated sprites per row, six for the total. */
    HUSPR_GROUPID groupId;
    /* Number prefix and digit sprites belonging to the group. */
    HUSPRID spriteId[6];
} MDSING_RESULT_MODEL;

extern MDSING_SCORE_ENTRY lbl_1_bss_480[128];
extern MDSING_SCORE_ENTRY lbl_1_bss_880[128];
extern MDSING_RESULT_MODEL lbl_1_bss_3E4[9];
extern s16 lbl_1_bss_474;
extern ANIMDATA *lbl_1_bss_478;
extern s16 lbl_1_bss_47C[2];
extern ANIMDATA *lbl_1_bss_3E0;

#endif
