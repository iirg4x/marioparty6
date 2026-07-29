#define _MATH_H

#include "dolphin.h"
#include "game/window.h"

typedef struct OptionDecaRecord {
    s16 character;
    s16 finalScore;
    s16 score[10];
} OptionDecaRecord;

typedef struct OptionWork {
    s16 micEnabled;
    s16 micStatus;
    s16 soundMode;
    s16 boardRecord[6];
    s16 boardCharacterRecord[6][11];
    OptionDecaRecord decaRecord[10];
    s32 prizeUnlocked[42];
    s16 prizeId[42];
    u32 prizeName[42];
    u32 prizeDescription[42];
    s32 minigameUnlocked[81];
    u32 record[19];
    u8 consecutiveRecordCount;
    u8 consecutiveRecord[100];
} OptionWork;

extern OptionWork lbl_1_bss_8;
extern s16 lbl_1_bss_6A2[];
extern s16 lbl_1_bss_9EC[7][2];
extern char lbl_1_bss_A08[7][30];
extern char lbl_1_data_486[];
int sprintf(char *buffer, const char *format, int value);

void fn_1_6504(s16 prizeNo)
{
    if (lbl_1_bss_8.prizeUnlocked[prizeNo] == FALSE) {
        HuWinMesSet(lbl_1_bss_6A2[0], 0x190065);
    } else {
        HuWinMesSet(lbl_1_bss_6A2[0],
            lbl_1_bss_8.prizeDescription[prizeNo]);
    }
}

void fn_1_6590(s16 firstPrize)
{
    s16 i;

    for (i = 0; i < 7; i++) {
        if (lbl_1_bss_8.prizeUnlocked[firstPrize + i] == FALSE) {
            HuWinMesSet(lbl_1_bss_9EC[i][0], 0x190032);
            lbl_1_bss_A08[i][0] = 0xE;
            lbl_1_bss_A08[i][1] = 0x13;
            lbl_1_bss_A08[i][2] = 0x10;
            lbl_1_bss_A08[i][3] = 0xC3;
            lbl_1_bss_A08[i][4] = 0xC3;
            lbl_1_bss_A08[i][5] = 0xC3;
            lbl_1_bss_A08[i][6] = 0;
            HuWinMesSet(lbl_1_bss_9EC[i][1], (u32)lbl_1_bss_A08[i]);
        } else {
            HuWinMesSet(lbl_1_bss_9EC[i][0],
                lbl_1_bss_8.prizeName[firstPrize + i]);
            lbl_1_bss_A08[i][0] = 0xE;
            lbl_1_bss_A08[i][1] = 0x13;
            lbl_1_bss_A08[i][2] = 0x10;
            sprintf(&lbl_1_bss_A08[i][3], lbl_1_data_486,
                lbl_1_bss_8.prizeId[firstPrize + i]);
            if (lbl_1_bss_A08[i][2] == '0') {
                lbl_1_bss_A08[i][2] = ' ';
                if (lbl_1_bss_A08[i][3] == '0') {
                    lbl_1_bss_A08[i][3] = ' ';
                }
            }
            HuWinMesSet(lbl_1_bss_9EC[i][1], (u32)lbl_1_bss_A08[i]);
        }
    }
}
