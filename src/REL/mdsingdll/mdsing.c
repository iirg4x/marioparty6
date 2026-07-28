#include "dolphin/mtx.h"
#include "dolphin/os.h"
#include "game/armem.h"
#include "game/charman.h"
#include "game/object.h"
#include "game/process.h"

extern const float lbl_1_rodata_5C;
extern const float lbl_1_rodata_60;
extern s16 lbl_1_bss_34;
extern char lbl_1_data_95A[];
extern char lbl_1_data_96B[];
extern char lbl_1_data_97D[];
extern char lbl_1_data_98F[];
extern char lbl_1_data_99D[];
extern char lbl_1_data_9A3[];
extern char lbl_1_data_9D3[];
extern char lbl_1_data_A02[];

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
