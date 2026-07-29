#include "dolphin.h"
#include "game/mgdata.h"

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
extern u32 lbl_1_bss_848[];

s16 fn_1_C324(s16 flag, s16 type)
{
    MGDATA *data = MgDataTbl;
    s16 count = 0;
    s16 index = count;

    while (data->ovl != 0xFFFF) {
        if ((data->flag & flag) && data->type == type) {
            if (lbl_1_bss_8.minigameUnlocked[index] == FALSE) {
                lbl_1_bss_848[count] = 0x43001F;
            } else {
                lbl_1_bss_848[count] = data->nameMes;
            }
            count++;
        }
        index++;
        data++;
    }
    return count;
}
