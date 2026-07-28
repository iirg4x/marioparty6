#include "dolphin/mtx.h"
#include "dolphin/os.h"
#include "game/armem.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/object.h"
#include "game/process.h"

extern const float lbl_1_rodata_5C;
extern const float lbl_1_rodata_60;
extern const u32 lbl_1_rodata_54[];
extern const s32 lbl_1_rodata_10;
extern const s32 lbl_1_rodata_14[];
extern HUPROCESS *lbl_1_bss_0;
extern s16 lbl_1_bss_30;
extern s16 lbl_1_bss_32;
extern s16 lbl_1_bss_34;
extern s16 lbl_1_bss_1308[][7];
extern s16 lbl_1_bss_1340[];
extern char lbl_1_data_95A[];
extern char lbl_1_data_96B[];
extern char lbl_1_data_97D[];
extern char lbl_1_data_98F[];
extern u32 lbl_1_data_8EC;
extern char lbl_1_data_8F0[];
extern char lbl_1_data_99D[];
extern char lbl_1_data_99F[];
extern char lbl_1_data_9A3[];
extern char lbl_1_data_9D3[];
extern char lbl_1_data_A02[];
extern u32 lbl_1_data_918[];
extern s16 lbl_1_data_912[];
extern char lbl_1_data_928[];

typedef struct MdsingDirectoryPair {
    u32 values[2];
} MdsingDirectoryPair;

typedef struct MdsingSoundTable {
    s32 values[16];
} MdsingSoundTable;

void fn_1_0(s32 unused, u32 sound, s16 slot)
{
    s32 soundMatch[1];
    MdsingSoundTable soundTable;
    s16 i;

    soundMatch[0] = lbl_1_rodata_10;
    soundTable = *(const MdsingSoundTable *)lbl_1_rodata_14;
    slot--;
    OSReport(lbl_1_data_8F0, slot);
    if (lbl_1_data_8EC != sound) {
        lbl_1_data_8EC = sound;
        for (i = 0;; i++) {
            if (soundMatch[i] == -1) {
                HuAudFXPlay(soundTable.values[slot]);
                break;
            }
            if (sound == (u32)soundMatch[i]) {
                if (slot >= 8) {
                    HuAudFXPlayPan(soundTable.values[slot], 0x50);
                } else {
                    HuAudFXPlayPan(soundTable.values[slot], 0x30);
                }
                break;
            }
        }
    }
}

void fn_1_1A4(void)
{
    lbl_1_bss_30 = 0;
    lbl_1_bss_32 = 0;
    if (GWBankFlagGet(2)) {
        lbl_1_bss_30 = 1;
    }
    if (GWBankFlagGet(4)) {
        lbl_1_bss_32 = 1;
    }
    lbl_1_data_912[0] = 1;
    lbl_1_data_912[1] = 1;
    lbl_1_data_912[2] = 1;
}

void fn_1_250(void)
{
    s16 character[5];
    s16 i;
    s32 status;

    character[0] = lbl_1_bss_1308[0][4];
    character[1] = lbl_1_bss_1308[1][4];
    character[2] = 11;
    character[3] = 12;
    character[4] = 13;
    for (i = 0; i < 5; i++) {
        if ((void *)CharMotionAMemPGet(character[i]) == NULL) {
            break;
        }
    }
    if (i == 5) {
        return;
    }

    CharDataClose(-1);
    for (i = 0; i < 5; i++) {
        status = HuDataDirReadAsync(CharDataDirTbl[character[i]][4]);
        if (status != -1) {
            while (!HuDataGetAsyncStat(status)) {
                HuPrcVSleep();
            }
        }
        CharMotionInit(character[i]);
        HuDataDirClose(CharDataDirTbl[character[i]][4]);
    }
}

void fn_1_3A0(void)
{
    MdsingDirectoryPair aramDirectory =
        *(const MdsingDirectoryPair *)lbl_1_rodata_54;
    s16 character[5];
    s16 i;
    s16 directory;
    s32 motionStatus;
    s32 dataStatus;

    character[0] = lbl_1_bss_1308[0][4];
    character[1] = lbl_1_bss_1308[1][4];
    character[2] = 11;
    character[3] = 12;
    character[4] = 13;
    for (i = 0; i < 5; i++) {
        if ((void *)CharMotionAMemPGet(character[i]) == NULL) {
            break;
        }
    }
    if (i != 5) {
        CharDataClose(-1);
        for (i = 0; i < 5; i++) {
            motionStatus = HuDataDirReadAsync(CharDataDirTbl[character[i]][4]);
            if (motionStatus != -1) {
                while (!HuDataGetAsyncStat(motionStatus)) {
                    HuPrcVSleep();
                }
            }
            CharMotionInit(character[i]);
            HuDataDirClose(CharDataDirTbl[character[i]][4]);
        }
    }
    for (directory = 0; directory < 2; directory++) {
        dataStatus = HuDataDirReadAsync(aramDirectory.values[directory]);
        if (dataStatus != -1) {
            while (!HuDataGetAsyncStat(dataStatus)) {
                HuPrcVSleep();
            }
        }
        HuAR_MRAMtoARAM(aramDirectory.values[directory]);
        while (HuARDMACheck() != 0) {
            HuPrcVSleep();
        }
        HuDataDirClose(aramDirectory.values[directory]);
    }
    dataStatus = HuDataDirReadAsync(lbl_1_data_918[lbl_1_bss_1340[1]]);
    if (dataStatus != -1) {
        while (!HuDataGetAsyncStat(dataStatus)) {
            HuPrcVSleep();
        }
    }
    lbl_1_bss_34 = 1;
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

void fn_1_5E8(void)
{
    lbl_1_bss_34 = 0;
    OSReport(lbl_1_data_928);
    OSReport(lbl_1_data_95A, 0x21);
    OSReport(lbl_1_data_96B, 0x24);
    OSReport(lbl_1_data_97D, 0x9B);
    OSReport(lbl_1_data_98F, 0xF2);
    HuAMemDump();
    OSReport(lbl_1_data_99D);
    HuDataDirClose(0x9A0000);
    HuPrcChildCreate(fn_1_3A0, 0x100, 0x4000, 0, lbl_1_bss_0);
}

void fn_1_6B4(void)
{
    s16 character = 0;
    s16 i;

    character = lbl_1_bss_1308[0][4];
    GwCommon.storyMgPack = lbl_1_bss_1340[3];
    for (i = 0; i < 1; i++) {
        GwPlayer[i].comF = 0;
        GwPlayer[i].comDif = 0;
        GwPlayer[i].charNo = lbl_1_bss_1308[i][4];
        GwPlayer[i].padNo = 0;
        GwPlayer[i].team = 0;
    }
    for (i = 1; i < 4; i++) {
        GwPlayer[i].comF = 1;
        GwPlayer[i].comDif = 0;
        character++;
        if (character >= 14) {
            character -= 14;
        }
        GwPlayer[i].charNo = character;
        GwPlayer[i].padNo = i;
        GwPlayer[i].team = 0;
    }
    for (i = 0; i < 4; i++) {
        GwPlayerConf[i].grpNo = 0;
        GwPlayerConf[i].type = GwPlayer[i].comF;
        GwPlayerConf[i].comDif = GwPlayer[i].comDif;
        GwPlayerConf[i].charNo = GwPlayer[i].charNo;
        GwPlayerConf[i].padNo = GwPlayer[i].padNo;
        GwPlayerConf[i].grpNo = GwPlayer[i].team;
    }
    for (i = 0; i < 4; i++) {
        OSReport(lbl_1_data_99F, GwPlayerConf[i].charNo);
    }
    GwCommon.confSingleDiff = lbl_1_bss_1340[2];
    GwCommon.storyMgPack = lbl_1_bss_1340[3];
    _ClearFlag(0x1000E);
    GWSingleDataInit();
    GWSingleMgRecordNumSet(0);
    GWSingleMgWinNumSet(0);
    GwSystem.turnPlayerNo = 0;
    for (i = 0; i < 4; i++) {
        GwCommon.singleMgWinNum[i] = 0;
    }
    mbSaveInit(lbl_1_bss_1340[1] + 6);
    mbSaveStoryInit(lbl_1_bss_1308[1][4], lbl_1_bss_1340[3],
        lbl_1_bss_1340[2]);
}

void fn_1_B34(void)
{
    s16 i;

    for (i = 0; i < 1; i++) {
        GwPlayer[i].comF = 1;
        GwPlayer[i].comDif = 0;
        GwPlayer[i].charNo = 0;
        GwPlayer[i].padNo = 0;
        GwPlayer[i].team = 0;
    }
    for (i = 1; i < 4; i++) {
        GwPlayer[i].comF = 1;
        GwPlayer[i].comDif = 0;
        GwPlayer[i].charNo = i;
        GwPlayer[i].padNo = i;
        GwPlayer[i].team = 0;
    }
    for (i = 0; i < 4; i++) {
        GwPlayerConf[i].grpNo = 0;
        GwPlayerConf[i].type = GwPlayer[i].comF;
        GwPlayerConf[i].comDif = GwPlayer[i].comDif;
        GwPlayerConf[i].charNo = GwPlayer[i].charNo;
        GwPlayerConf[i].padNo = GwPlayer[i].padNo;
        GwPlayerConf[i].grpNo = GwPlayer[i].team;
    }
    lbl_1_bss_1308[1][4] = GwPlayer[1].charNo;
    for (i = 0; i < 4; i++) {
        OSReport(lbl_1_data_99F, GwPlayerConf[i].charNo);
    }
    lbl_1_bss_1340[1] = 3;
    mbSaveInit(10);
}

void fn_1_EA0(void)
{
    OMOVLHIS *history;

    do {
        HuPrcVSleep();
    } while (lbl_1_bss_34 == 0);

    history = omOvlHisGet(0);
    omOvlHisChg(0, history->ovl, 1, lbl_1_bss_1308[1][4]);
    OSReport(lbl_1_data_9A3);
    OSReport(lbl_1_data_95A, 0x21);
    OSReport(lbl_1_data_96B, 0x24);
    OSReport(lbl_1_data_97D, 0x9B);
    OSReport(lbl_1_data_98F, 0xF2);
    HuAMemDump();
    OSReport(lbl_1_data_99D);
    switch (lbl_1_bss_1340[1]) {
        case 0:
            omOvlCallEx(0x72, 1, 0, 0);
            break;
        case 1:
            omOvlCallEx(0x73, 1, 0, 0);
            break;
        case 2:
            omOvlCallEx(0x74, 1, 0, 0);
            break;
    }
}

void fn_1_FEC(void)
{
    OMOVLHIS *history;

    do {
        HuPrcVSleep();
    } while (lbl_1_bss_34 == 0);

    history = omOvlHisGet(0);
    omOvlHisChg(0, history->ovl, 2, 0);
    OSReport(lbl_1_data_9A3);
    OSReport(lbl_1_data_95A, 0x21);
    OSReport(lbl_1_data_96B, 0x24);
    OSReport(lbl_1_data_97D, 0x9B);
    OSReport(lbl_1_data_98F, 0xF2);
    HuAMemDump();
    OSReport(lbl_1_data_99D);
    omOvlCallEx(0x82, 1, 0, 0);
}

void fn_1_10D0(void)
{
    CharDataClose(-1);
    HuARDirFree(0x50000);
    HuARDirFree(0x60000);
    HuARDirFree(0xC0000);
    OSReport(lbl_1_data_9D3);
    OSReport(lbl_1_data_95A, 0x21);
    OSReport(lbl_1_data_96B, 0x24);
    OSReport(lbl_1_data_97D, 0x9B);
    OSReport(lbl_1_data_98F, 0xF2);
    HuAMemDump();
    OSReport(lbl_1_data_99D);
}

void fn_1_1180(void)
{
    CharDataClose(-1);
    HuARDirFree(0x50000);
    HuARDirFree(0x60000);
    HuARDirFree(0xC0000);
    OSReport(lbl_1_data_A02);
    OSReport(lbl_1_data_95A, 0x21);
    OSReport(lbl_1_data_96B, 0x24);
    OSReport(lbl_1_data_97D, 0x9B);
    OSReport(lbl_1_data_98F, 0xF2);
    HuAMemDump();
    OSReport(lbl_1_data_99D);
}

float fn_1_1230(float start, float control, float end, float weight)
{
    float inverse = lbl_1_rodata_5C - weight;

    return (end * (weight * weight)) +
        ((start * (inverse * inverse)) +
            (lbl_1_rodata_60 * (control * (inverse * weight))));
}

void fn_1_128C(Vec *out, const Vec *start, const Vec *control,
    const Vec *end, float weight)
{
    out->x = fn_1_1230(start->x, control->x, end->x, weight);
    out->y = fn_1_1230(start->y, control->y, end->y, weight);
    out->z = fn_1_1230(start->z, control->z, end->z, weight);
}

float fn_1_1494(float current, float target, float weight)
{
    if (current == target) {
        return target;
    }
    return (target + (current * (weight - lbl_1_rodata_5C))) / weight;
}

void fn_1_14C4(Vec *current, const Vec *target, float weight)
{
    current->x = fn_1_1494(current->x, target->x, weight);
    current->y = fn_1_1494(current->y, target->y, weight);
    current->z = fn_1_1494(current->z, target->z, weight);
}
