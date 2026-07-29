#define _MATH_H

#include "dolphin.h"
#include "game/mgdata.h"
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
extern char lbl_1_bss_9D8[5][4];
extern s16 lbl_1_bss_9EC[7][2];
extern char lbl_1_data_486[];
int sprintf(char *buffer, const char *format, int value);

void fn_1_B02C(s16 first, s16 count)
{
    s16 i;

    for (i = 0; i < 5; i++) {
        if (first + i >= count) {
            HuWinHomeClear(lbl_1_bss_9EC[i][0]);
            HuWinHomeClear(lbl_1_bss_9EC[i][1]);
        } else {
            sprintf(lbl_1_bss_9D8[i], lbl_1_data_486, first + i + 1);
            HuWinInsertMesSet(lbl_1_bss_9EC[i][0],
                (u32)lbl_1_bss_9D8[i], 0);
            HuWinMesSet(lbl_1_bss_9EC[i][0], 0x43003D);
            HuWinMesSet(lbl_1_bss_9EC[i][1],
                MgDataTbl[lbl_1_bss_8.consecutiveRecord[first + i]].nameMes);
        }
    }
}
