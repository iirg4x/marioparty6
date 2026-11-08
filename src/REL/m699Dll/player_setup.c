/* Dunk Bros. player grouping and object creation. */
#include "math.h"
#include "dolphin/mtx.h"
#include "game/gamework.h"
#include "game/object.h"
#include "REL/m699Dll.h"

extern s32 lbl_1_bss_444[25];
extern OMOBJ *lbl_1_bss_4A8[4];
extern OMOBJ *lbl_1_bss_4B8;
extern void fn_1_59CC(OMOBJ *object);
extern void fn_1_597C(OMOBJ *object);
extern void fn_1_62CC(OMOBJ *object);

void fn_1_5760(HUPROCESS *process)
{
    s32 playerGroups[4];
    s32 teamCounts[2] = { 0, 0 };
    s32 playerCounts[2] = { 0, 2 };
    s32 invalidGroup;
    s32 playerIndex;
    OMOBJ *object;
    s32 group;

    invalidGroup = 0;
    lbl_1_bss_4B8 = fn_8003AB38(process, 0xE, 0, 0, -1, fn_1_59CC);
    fn_1_597C(lbl_1_bss_4B8);

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        playerGroups[playerIndex] = group = lbl_80265728[playerIndex].grpNo;
        if (group >= 2) {
            invalidGroup = 1;
        } else {
            teamCounts[group] += 1;
        }
    }

    if (teamCounts[0] != 2 || teamCounts[1] != 2) {
        invalidGroup = 1;
    }
    if (invalidGroup != 0) {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            playerGroups[playerIndex] = playerIndex >> 1;
        }
    }

    omMakeGroupEx(process, 0, 4);
    lbl_1_bss_444[0] = 0;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        object = lbl_1_bss_4A8[playerIndex] =
            fn_8003AB38(process, 0xA, 1, 0x10, 0, fn_1_62CC);
        object->work[0] = playerIndex;
        group = playerGroups[playerIndex];
        object->work[1] = group;
        object->work[2] = playerCounts[group]++;
        fn_1_1210(playerIndex, group, lbl_80265728[playerIndex].charNo);
    }
}
