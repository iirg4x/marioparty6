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
#include <string.h>


typedef struct M616CpuParam {
    u32 unk_00;
    u32 repeatPercent;
} M616CpuParam;
extern MGSEQ_PARAM lbl_1_data_0;
extern s32 lbl_1_data_1F0[17];
extern s32 lbl_1_data_234[6];
extern s32 lbl_1_data_24C[13];
extern unsigned int lbl_1_data_280[12];
extern M616CpuParam lbl_1_data_2B0[4];
extern s32 lbl_1_data_2D0[3];
void fn_1_23B0(void)
{
    s32 player;
    u16 choice;
    M616CpuParam *param;

    for (player = 0; player < 4; player++) {
        if (GwPlayerConf[player].type == 1) {
            param = &lbl_1_data_2B0[GwPlayerConf[player].comDif];
            if (lbl_1_bss_10.roundNo == 0) {
                do {
                    choice = frand() % 5;
                } while (choice == 0);
            } else if (frand() % 100 < param->repeatPercent) {
                choice = lbl_1_bss_10.cpuChoices[player];
            } else {
                do {
                    choice = frand() % 5;
                } while (choice == lbl_1_bss_10.cpuChoices[player] || choice == 0);
            }
            lbl_1_bss_10.cpuChoices[player] = choice;
            lbl_1_bss_10.cpuInputFrames[player] = frand() % 270;
            OSReport("com %d : btn...%d time ...%d\n", player,
                lbl_1_bss_10.cpuChoices[player], lbl_1_bss_10.cpuInputFrames[player]);
        }
    }
}

s32 fn_1_2580(s32 playerNo)
{
    return lbl_1_bss_10.cpuChoices[playerNo];
}
s32 lbl_1_data_1F0[17] = {
    DATANUM(DATA_m616, 0), DATANUM(DATA_m616, 1), DATANUM(DATA_m616, 2),
    DATANUM(DATA_m616, 3), DATANUM(DATA_m616, 6), DATANUM(DATA_m616, 4),
    DATANUM(DATA_m616, 7), DATANUM(DATA_m616, 8), DATANUM(DATA_m616, 7),
    DATANUM(DATA_m616, 5), DATANUM(DATA_m616, 11), DATANUM(DATA_m616, 9),
    DATANUM(DATA_m616, 11), DATANUM(DATA_m616, 10), DATANUM(DATA_m616, 11),
    DATANUM(DATA_m616, 9), DATANUM(DATA_m616, 11)
};

s32 lbl_1_data_234[6] = {
    DATANUM(DATA_m616, 22), DATANUM(DATA_m616, 23), DATANUM(DATA_m616, 24),
    DATANUM(DATA_m616, 25), DATANUM(DATA_m616, 26), DATANUM(DATA_m616, 27)
};

s32 lbl_1_data_24C[13] = {
    DATANUM(DATA_m616, 28), DATANUM(DATA_m616, 29), DATANUM(DATA_m616, 30),
    DATANUM(DATA_m616, 31), DATANUM(DATA_m616, 33), DATANUM(DATA_m616, 32),
    DATANUM(DATA_m616, 34), DATANUM(DATA_m616, 35), DATANUM(DATA_m616, 36),
    DATANUM(DATA_m616, 37), DATANUM(DATA_m616, 38), DATANUM(DATA_m616, 39),
    DATANUM(DATA_m616, 40)
};

unsigned int lbl_1_data_280[12] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 3), DATANUM(DATA_mariomot, 6),
    DATANUM(DATA_mariomot, 40), DATANUM(DATA_mariomot, 111), DATANUM(DATA_mariomot, 112),
    DATANUM(DATA_mariomot, 4), DATANUM(DATA_mariomot, 36), DATANUM(DATA_mariomot, 37),
    DATANUM(DATA_mario, 154), DATANUM(DATA_mario, 155), 0
};



M616CpuParam lbl_1_data_2B0[4] = {
    { 80, 50 }, { 85, 30 }, { 90, 10 }, { 100, 0 }
};

s32 lbl_1_data_2D0[3] = {
    DATANUM(DATA_m616, 16), DATANUM(DATA_m616, 17), DATANUM(DATA_m616, 18)
};
