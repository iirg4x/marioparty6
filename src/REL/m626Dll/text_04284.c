/* Sets up player groups and interpolates points for Snow Brawl objects. */
#include "REL/m626Dll/objects.h"
#include "dolphin/mtx.h"
#include "dolphin/types.h"
#include "game/object.h"
#include "game/gamework.h"
#include "game/process.h"

/* Interpolates a point along a segment when a caller needs an intermediate position. */
void fn_1_4284(Point3d *startPoint, Point3d *endPoint, Point3d *interpolatedPoint, f32 t)
{
    interpolatedPoint->x = startPoint->x + (t * (endPoint->x - startPoint->x));
    interpolatedPoint->y = startPoint->y + (t * (endPoint->y - startPoint->y));
    interpolatedPoint->z = startPoint->z + (t * (endPoint->z - startPoint->z));
}

extern OMOBJ *lbl_1_bss_304;
typedef struct M626GroupCounts_s {
    s32 playersPerGroup[2];
} M626GroupCounts;

typedef struct M626PlayerSlots_s {
    s32 nextSlotPerGroup[2];
} M626PlayerSlots;

static const M626GroupCounts lbl_1_rodata_C8 = {{0, 0}};
static const M626PlayerSlots lbl_1_rodata_D0 = {{0, 1}};

extern void fn_1_44F8(OMOBJ *object);
extern void fn_1_4548(OMOBJ *object);
extern void fn_1_4EA8(OMOBJ *object);
extern s32 fn_1_960(s32 player, u32 group, s16 character);

/* Called by fn_1_A0 during startup to create players and assign fallback teams. */
void fn_1_42DC(HUPROCESS *process)
{
    s32 fallbackGroup;
    s32 playerGroups[4];
    M626GroupCounts groupCounts;
    M626PlayerSlots playerSlots;
    s32 anyLargeGroup;
    s32 player;
    OMOBJ *object;
    s32 group;

    groupCounts = lbl_1_rodata_C8;
    playerSlots = lbl_1_rodata_D0;
    anyLargeGroup = 0;
    lbl_1_bss_304 = omAddObjEx(process, 14, 0, 0, -1, fn_1_4548);
    fn_1_44F8(lbl_1_bss_304);

    for (player = 0; player < 4; player++) {
        if ((playerGroups[player] =
                (group = GwPlayerConf[player].grpNo)) >= 2) {
            anyLargeGroup = 1;
        } else {
            groupCounts.playersPerGroup[group]++;
        }
    }
    if (groupCounts.playersPerGroup[0] != 1 || groupCounts.playersPerGroup[1] != 3) {
        anyLargeGroup = 1;
    }
    if (anyLargeGroup != 0) {
        /* Invalid side assignments use player zero against the other three players. */
        for (player = 0; player < 4; player++) {
            if (player == 0) {
                fallbackGroup = 0;
            } else {
                fallbackGroup = 1;
            }
            playerGroups[player] = fallbackGroup;
        }
    }

    omMakeGroupEx(process, 0, 4);
    for (player = 0; player < 4; player++) {
        object = (lbl_1_bss_2F4[player] =
            omAddObjEx(process, 10, 1, 10, 0, fn_1_4EA8));
        object->work[0] = player;
        group = playerGroups[player];
        object->work[1] = group;
        object->work[2] = playerSlots.nextSlotPerGroup[group]++;
        fn_1_960(player, group, GwPlayerConf[player].charNo);
    }
}
