/* CPU choice and reaction-delay selection used at the start of M616 rounds. */
#include "REL/m616dll.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/pad.h"
#include "game/frand.h"
#include "datadir_enum.h"
#include "datanum/mario.h"
#include <string.h>

/* When CPU choices are prepared for a round, choose 1 to 4 at random in round zero; later, repeat
 * the prior answer with the difficulty's repeat chance or choose a different answer, and choose an
 * input frame from 0 through 269. */
void fn_1_23B0(void)
{
    s32 playerNo;
    u16 choiceId;
    M616CpuParam *cpuSettings;

    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (lbl_80265728[playerNo].type == 1) {
            cpuSettings = &lbl_1_data_2B0[lbl_80265728[playerNo].comDif];
            if (lbl_1_bss_10.roundNo == 0) {
                do {
                    choiceId = frand() % 5;
                } while (choiceId == 0);
            } else if (frand() % 100 < cpuSettings->repeatPercent) {
                choiceId = lbl_1_bss_10.cpuChoices[playerNo];
            } else {
                do {
                    choiceId = frand() % 5;
                } while (choiceId == lbl_1_bss_10.cpuChoices[playerNo] || choiceId == 0);
            }
            lbl_1_bss_10.cpuChoices[playerNo] = choiceId;
            lbl_1_bss_10.cpuInputFrames[playerNo] = frand() % 270;
            OSReport("com %d : btn...%d time ...%d\n", playerNo,
                lbl_1_bss_10.cpuChoices[playerNo], lbl_1_bss_10.cpuInputFrames[playerNo]);
        }
    }
}

/* The round callback reads the CPU's selected answer through this accessor. */
s32 fn_1_2580(s32 playerNo)
{
    return lbl_1_bss_10.cpuChoices[playerNo];
}

/* Data IDs for the 17 camera motions, indexed by cameraIndex. */
s32 lbl_1_data_1F0[17] = {
    DATANUM(0x4b0000, 0), DATANUM(0x4b0000, 1), DATANUM(0x4b0000, 2),
    DATANUM(0x4b0000, 3), DATANUM(0x4b0000, 6), DATANUM(0x4b0000, 4),
    DATANUM(0x4b0000, 7), DATANUM(0x4b0000, 8), DATANUM(0x4b0000, 7),
    DATANUM(0x4b0000, 5), DATANUM(0x4b0000, 11), DATANUM(0x4b0000, 9),
    DATANUM(0x4b0000, 11), DATANUM(0x4b0000, 10), DATANUM(0x4b0000, 11),
    DATANUM(0x4b0000, 9), DATANUM(0x4b0000, 11)
};

/* Data IDs for the six motions loaded onto the linked per-seat display models. */
s32 lbl_1_data_234[6] = {
    DATANUM(0x4b0000, 22), DATANUM(0x4b0000, 23), DATANUM(0x4b0000, 24),
    DATANUM(0x4b0000, 25), DATANUM(0x4b0000, 26), DATANUM(0x4b0000, 27)
};

/* Data IDs for the 13 motions loaded onto each player's base model. */
s32 lbl_1_data_24C[13] = {
    DATANUM(0x4b0000, 28), DATANUM(0x4b0000, 29), DATANUM(0x4b0000, 30),
    DATANUM(0x4b0000, 31), DATANUM(0x4b0000, 33), DATANUM(0x4b0000, 32),
    DATANUM(0x4b0000, 34), DATANUM(0x4b0000, 35), DATANUM(0x4b0000, 36),
    DATANUM(0x4b0000, 37), DATANUM(0x4b0000, 38), DATANUM(0x4b0000, 39),
    DATANUM(0x4b0000, 40)
};

/* Character motion data IDs passed to CharModelMotListCreate for each seat. */
unsigned int lbl_1_data_280[12] = {
    CHARMOT_HSF_c000m1_300, CHARMOT_HSF_c000m1_303, CHARMOT_HSF_c000m1_306,
    CHARMOT_HSF_c000m1_351, CHARMOT_HSF_c000m1_458, CHARMOT_HSF_c000m1_459,
    CHARMOT_HSF_c000m1_304, CHARMOT_HSF_c000m1_346, CHARMOT_HSF_c000m1_348,
    MARIO_HSF_c000m1_618, MARIO_HSF_c000m1_619, 0
};

/* Rows are indexed by CPU difficulty; repeatPercent is the chance of reusing the previous round's
 * answer. */
M616CpuParam lbl_1_data_2B0[4] = {
    { 80, 50 }, { 85, 30 }, { 90, 10 }, { 100, 0 }
};

/* Data IDs for the center gear's three motions. */
s32 lbl_1_data_2D0[3] = {
    DATANUM(0x4b0000, 16), DATANUM(0x4b0000, 17), DATANUM(0x4b0000, 18)
};
