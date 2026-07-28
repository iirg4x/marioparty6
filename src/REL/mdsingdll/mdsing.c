#include "dolphin/mtx.h"
#include "dolphin/os.h"
#include "game/armem.h"
#include "game/charman.h"
#include "game/gamework.h"
#include "game/object.h"
#include "game/process.h"

extern const float lbl_1_rodata_5C;
extern const float lbl_1_rodata_60;
extern s16 lbl_1_bss_34;
extern s16 lbl_1_bss_1308[];
extern s16 lbl_1_bss_1340[];
extern char lbl_1_data_95A[];
extern char lbl_1_data_96B[];
extern char lbl_1_data_97D[];
extern char lbl_1_data_98F[];
extern char lbl_1_data_99D[];
extern char lbl_1_data_99F[];
extern char lbl_1_data_9A3[];
extern char lbl_1_data_9D3[];
extern char lbl_1_data_A02[];

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
    lbl_1_bss_1308[11] = GwPlayer[1].charNo;
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
    omOvlHisChg(0, history->ovl, 1, lbl_1_bss_1308[11]);
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
