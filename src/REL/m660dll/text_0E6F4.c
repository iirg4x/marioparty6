/* Builds the route grid and advances the stage's player movement. */
#include "dolphin/math.h"
#include "REL/m660dll/grid.h"
#include "REL/m660dll/particles.h"
#include "dolphin/types.h"
#include "string.h"
#include "game/hu3d.h"
#include "game/gamework.h"
#include "game/flag.h"
#include "game/wipe.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "REL/m660dll/player_slots.h"
#include "game/mg/actman.h"
#include "REL/m660dll/path_data.h"
#include "dolphin/mtx.h"
#include "REL/m660dll/scene.h"

typedef struct RouteModeList_131C8 {
    s16 values[3];
} RouteModeList_131C8;
#include "REL/m660dll/routes.h"
extern HuVecF Center, CRot;
extern f32 CZoom;
extern int printf();
f32 fn_1_3C54(f32 range);
s16 fn_1_2E80(f32 *value, f32 step, f32 start, f32 end);
void fn_1_4004(Vec *origin);
void fn_1_3068(MGTIMER *timer);
void fn_1_67D4(s16 slot, s16 mode, Vec *position);
void fn_1_4B38(M660Model *origin);
void fn_1_E6F4(void);
void fn_1_11F50(void);
s16 fn_1_12030(M660Player *player, s16 advanceState);
s16 fn_1_1265C(M660Player *player);
s16 fn_1_12CA8(M660Player *player);
s16 fn_1_131C8(M660Player *player, s16 selectFarthest, s16 useAlternateMode);
s16 fn_1_13810(M660Player *player, s16 useCenter);
s16 fn_1_13B64(void);
void fn_1_13C6C(M660Player *player);
void fn_1_142F8(s16 group);
void fn_1_15484(s16 selection);
void fn_1_15548(void);

/* Builds the board route grid and lane paths during fn_1_A3E4 stage setup. */
void fn_1_E6F4(void)
{
    f32 directionTable[8][3] = {
        {0.0f, 0.0f, 1.0f}, {45.0f, 1.0f, 1.0f},
        {90.0f, 1.0f, 0.0f}, {135.0f, 1.0f, -1.0f},
        {180.0f, 0.0f, -1.0f}, {225.0f, -1.0f, -1.0f},
        {270.0f, -1.0f, 0.0f}, {315.0f, -1.0f, 1.0f}
    };
    M660RoutePath savedPath;
    Vec position;
    Vec direction;
    s16 centerModes[4] = {1, 1, 1, 1};
    s16 chances[4] = {100, 100, 100, 100};
    s16 lifetimes[4][2] = {{4, 3}, {8, 5}, {13, 7}, {28, 1}};
    s16 index;
    s16 lane;
    s16 pointIndex;
    s16 directionIndex;
    s16 group;
    f32 angle;
    f32 radius;

    /* The difficulty index is assigned only for CPU slots. A human slot reuses its prior value,
     * which is uninitialized until a CPU slot assigns it. */
    for (index = 0; index < 2; index++) {
        if (lbl_1_bss_2DC[index].type != 0) {
            group = lbl_1_bss_2DC[index].difficulty;
        }
        lbl_1_bss_2DC[index].routeRandomRange = centerModes[(s16)group];
        lbl_1_bss_2DC[index].routeChance = chances[(s16)group];
        lbl_1_bss_2DC[index].routeLifetime =
            lifetimes[(s16)group][0] + frandmod(lifetimes[(s16)group][1]);
        lbl_1_bss_2DC[index].routeLifetime *= 60;
    }
    memset(&lbl_1_bss_3688, 0, 48);
    lbl_1_bss_3688.minimum.x = -750.0f;
    lbl_1_bss_3688.minimum.y = 0.0f;
    lbl_1_bss_3688.minimum.z = -750.0f;
    lbl_1_bss_3688.maximum.x = 750.0f;
    lbl_1_bss_3688.maximum.y = 0.0f;
    lbl_1_bss_3688.maximum.z = 750.0f;
    lbl_1_bss_3688.spacing.x = 100.0f;
    lbl_1_bss_3688.spacing.y = 0.0f;
    lbl_1_bss_3688.spacing.z = 100.0f;
    lbl_1_bss_3688.columns = (s32)((lbl_1_bss_3688.maximum.x -
        lbl_1_bss_3688.minimum.x) / lbl_1_bss_3688.spacing.x) + 1;
    lbl_1_bss_3688.rows = (s32)((lbl_1_bss_3688.maximum.z -
        lbl_1_bss_3688.minimum.z) / lbl_1_bss_3688.spacing.z) + 1;
    lbl_1_bss_3688.count = lbl_1_bss_3688.columns * lbl_1_bss_3688.rows;
    lbl_1_bss_3688.radius = lbl_1_bss_3688.spacing.x / 2;
    memset(lbl_1_bss_2688, 0, lbl_1_bss_3688.count * 16);
    index = 0;
    pointIndex = 0;
    while (index < lbl_1_bss_3688.rows) {
        lane = 0;
        while (lane < lbl_1_bss_3688.columns) {
            lbl_1_bss_2688[pointIndex].flags = 0;
            lbl_1_bss_2688[pointIndex].position.x = lbl_1_bss_3688.minimum.x +
                lbl_1_bss_3688.spacing.x * lane;
            lbl_1_bss_2688[pointIndex].position.y = 0.0f;
            lbl_1_bss_2688[pointIndex].position.z = lbl_1_bss_3688.minimum.z +
                lbl_1_bss_3688.spacing.z * index;
            if (pointIndex == 0 || pointIndex == 1 || pointIndex == 14 || pointIndex == 15 ||
                pointIndex == 16 || pointIndex == 31 || pointIndex == 224 || pointIndex == 239 ||
                pointIndex == 240 || pointIndex == 241 || pointIndex == 254 || pointIndex == 255) {
                lbl_1_bss_2688[pointIndex].flags = 1;
            }
            lane++;
            pointIndex++;
        }
        index++;
    }
    fn_1_142F8(3);
    memset(lbl_1_bss_36B8, 0, 2968);
    for (index = 0; index < 14; index++) {
        for (lane = 0; lane < 4; lane++) {
            lbl_1_bss_36B8[index].paths[lane].direction = -1;
            lbl_1_bss_36B8[index].paths[lane].count = 1;
        }
    }
    lbl_1_bss_36B8[0].paths[1].direction = 1;
    lbl_1_bss_36B8[0].paths[1].count = 2;
    lbl_1_bss_36B8[0].paths[1].directions[0].x = 1.0f;
    lbl_1_bss_36B8[0].paths[1].directions[0].y = 0.0f;
    lbl_1_bss_36B8[0].paths[1].directions[0].z = 1.0f;
    lbl_1_bss_36B8[0].paths[1].points[0].x = -495.0f;
    lbl_1_bss_36B8[0].paths[1].points[0].y = 0.0f;
    lbl_1_bss_36B8[0].paths[1].points[0].z = -386.0f;
    lbl_1_bss_36B8[0].paths[1].directions[1].x = -1.0f;
    lbl_1_bss_36B8[0].paths[1].directions[1].y = 0.0f;
    lbl_1_bss_36B8[0].paths[1].directions[1].z = 1.0f;
    lbl_1_bss_36B8[0].paths[1].points[1].x = -748.0f;
    lbl_1_bss_36B8[0].paths[1].points[1].y = 0.0f;
    lbl_1_bss_36B8[0].paths[1].points[1].z = -415.0f;
    lbl_1_bss_36B8[0].paths[2].direction = 2;
    lbl_1_bss_36B8[0].paths[2].count = 2;
    lbl_1_bss_36B8[0].paths[2].directions[0].x = -1.0f;
    lbl_1_bss_36B8[0].paths[2].directions[0].y = 0.0f;
    lbl_1_bss_36B8[0].paths[2].directions[0].z = 1.0f;
    lbl_1_bss_36B8[0].paths[2].points[0].x = -665.0f;
    lbl_1_bss_36B8[0].paths[2].points[0].y = 0.0f;
    lbl_1_bss_36B8[0].paths[2].points[0].z = 74.0f;
    lbl_1_bss_36B8[0].paths[2].directions[1].x = -1.0f;
    lbl_1_bss_36B8[0].paths[2].directions[1].y = 0.0f;
    lbl_1_bss_36B8[0].paths[2].directions[1].z = -1.0f;
    lbl_1_bss_36B8[0].paths[2].points[1].x = -665.0f;
    lbl_1_bss_36B8[0].paths[2].points[1].y = 0.0f;
    lbl_1_bss_36B8[0].paths[2].points[1].z = -574.0f;
    lbl_1_bss_36B8[0].paths[3].direction = 3;
    lbl_1_bss_36B8[0].paths[3].count = 2;
    lbl_1_bss_36B8[0].paths[3].directions[0].x = 1.0f;
    lbl_1_bss_36B8[0].paths[3].directions[0].y = 0.0f;
    lbl_1_bss_36B8[0].paths[3].directions[0].z = 1.0f;
    lbl_1_bss_36B8[0].paths[3].points[0].x = -611.0f;
    lbl_1_bss_36B8[0].paths[3].points[0].y = 0.0f;
    lbl_1_bss_36B8[0].paths[3].points[0].z = 74.0f;
    lbl_1_bss_36B8[0].paths[3].directions[1].x = 1.0f;
    lbl_1_bss_36B8[0].paths[3].directions[1].y = 0.0f;
    lbl_1_bss_36B8[0].paths[3].directions[1].z = -1.0f;
    lbl_1_bss_36B8[0].paths[3].points[1].x = -611.0f;
    lbl_1_bss_36B8[0].paths[3].points[1].y = 0.0f;
    lbl_1_bss_36B8[0].paths[3].points[1].z = -574.0f;
    lbl_1_bss_36B8[1].paths[0].direction = 0;
    lbl_1_bss_36B8[1].paths[0].count = 2;
    lbl_1_bss_36B8[1].paths[0].directions[0].x = 1.0f;
    lbl_1_bss_36B8[1].paths[0].directions[0].y = 0.0f;
    lbl_1_bss_36B8[1].paths[0].directions[0].z = -1.0f;
    lbl_1_bss_36B8[1].paths[0].points[0].x = -485.0f;
    lbl_1_bss_36B8[1].paths[0].points[0].y = 0.0f;
    lbl_1_bss_36B8[1].paths[0].points[0].z = -80.0f;
    lbl_1_bss_36B8[1].paths[0].directions[1].x = -1.0f;
    lbl_1_bss_36B8[1].paths[0].directions[1].y = 0.0f;
    lbl_1_bss_36B8[1].paths[0].directions[1].z = -1.0f;
    lbl_1_bss_36B8[1].paths[0].points[1].x = -753.0f;
    lbl_1_bss_36B8[1].paths[0].points[1].y = 0.0f;
    lbl_1_bss_36B8[1].paths[0].points[1].z = -80.0f;
    lbl_1_bss_36B8[1].paths[2].direction = 2;
    lbl_1_bss_36B8[1].paths[2].count = 2;
    lbl_1_bss_36B8[1].paths[2].directions[0].x = -1.0f;
    lbl_1_bss_36B8[1].paths[2].directions[0].y = 0.0f;
    lbl_1_bss_36B8[1].paths[2].directions[0].z = 1.0f;
    lbl_1_bss_36B8[1].paths[2].points[0].x = -665.0f;
    lbl_1_bss_36B8[1].paths[2].points[0].y = 0.0f;
    lbl_1_bss_36B8[1].paths[2].points[0].z = 74.0f;
    lbl_1_bss_36B8[1].paths[2].directions[1].x = -1.0f;
    lbl_1_bss_36B8[1].paths[2].directions[1].y = 0.0f;
    lbl_1_bss_36B8[1].paths[2].directions[1].z = -1.0f;
    lbl_1_bss_36B8[1].paths[2].points[1].x = -665.0f;
    lbl_1_bss_36B8[1].paths[2].points[1].y = 0.0f;
    lbl_1_bss_36B8[1].paths[2].points[1].z = -574.0f;
    lbl_1_bss_36B8[1].paths[3].direction = 3;
    lbl_1_bss_36B8[1].paths[3].count = 2;
    lbl_1_bss_36B8[1].paths[3].directions[0].x = 1.0f;
    lbl_1_bss_36B8[1].paths[3].directions[0].y = 0.0f;
    lbl_1_bss_36B8[1].paths[3].directions[0].z = 1.0f;
    lbl_1_bss_36B8[1].paths[3].points[0].x = -611.0f;
    lbl_1_bss_36B8[1].paths[3].points[0].y = 0.0f;
    lbl_1_bss_36B8[1].paths[3].points[0].z = 74.0f;
    lbl_1_bss_36B8[1].paths[3].directions[1].x = 1.0f;
    lbl_1_bss_36B8[1].paths[3].directions[1].y = 0.0f;
    lbl_1_bss_36B8[1].paths[3].directions[1].z = -1.0f;
    lbl_1_bss_36B8[1].paths[3].points[1].x = -611.0f;
    lbl_1_bss_36B8[1].paths[3].points[1].y = 0.0f;
    lbl_1_bss_36B8[1].paths[3].points[1].z = -574.0f;
    lbl_1_bss_36B8[2].paths[2].direction = 2;
    lbl_1_bss_36B8[2].paths[2].directions[0].x = -1.0f;
    lbl_1_bss_36B8[2].paths[2].directions[0].y = 0.0f;
    lbl_1_bss_36B8[2].paths[2].directions[0].z = 0.0f;
    lbl_1_bss_36B8[2].paths[2].points[0].x = 153.0f;
    lbl_1_bss_36B8[2].paths[2].points[0].y = 0.0f;
    lbl_1_bss_36B8[2].paths[2].points[0].z = -514.0f;
    lbl_1_bss_36B8[2].paths[3].direction = 3;
    lbl_1_bss_36B8[2].paths[3].directions[0].x = 1.0f;
    lbl_1_bss_36B8[2].paths[3].directions[0].y = 0.0f;
    lbl_1_bss_36B8[2].paths[3].directions[0].z = 0.0f;
    lbl_1_bss_36B8[2].paths[3].points[0].x = -80.0f;
    lbl_1_bss_36B8[2].paths[3].points[0].y = 0.0f;
    lbl_1_bss_36B8[2].paths[3].points[0].z = -465.0f;
    lbl_1_bss_36B8[3].paths[0].direction = 0;
    lbl_1_bss_36B8[3].paths[0].count = 2;
    lbl_1_bss_36B8[3].paths[0].directions[0].x = 1.0f;
    lbl_1_bss_36B8[3].paths[0].directions[0].y = 0.0f;
    lbl_1_bss_36B8[3].paths[0].directions[0].z = -1.0f;
    lbl_1_bss_36B8[3].paths[0].points[0].x = 174.0f;
    lbl_1_bss_36B8[3].paths[0].points[0].y = 0.0f;
    lbl_1_bss_36B8[3].paths[0].points[0].z = -643.0f;
    lbl_1_bss_36B8[3].paths[0].directions[1].x = -1.0f;
    lbl_1_bss_36B8[3].paths[0].directions[1].y = 0.0f;
    lbl_1_bss_36B8[3].paths[0].directions[1].z = -1.0f;
    lbl_1_bss_36B8[3].paths[0].points[1].x = -268.0f;
    lbl_1_bss_36B8[3].paths[0].points[1].y = 0.0f;
    lbl_1_bss_36B8[3].paths[0].points[1].z = -614.0f;
    lbl_1_bss_36B8[3].paths[1].direction = 1;
    lbl_1_bss_36B8[3].paths[1].directions[0].x = 0.0f;
    lbl_1_bss_36B8[3].paths[1].directions[0].y = 0.0f;
    lbl_1_bss_36B8[3].paths[1].directions[0].z = 1.0f;
    lbl_1_bss_36B8[3].paths[1].points[0].x = 153.0f;
    lbl_1_bss_36B8[3].paths[1].points[0].y = 0.0f;
    lbl_1_bss_36B8[3].paths[1].points[0].z = -514.0f;
    lbl_1_bss_36B8[3].paths[2].direction = 2;
    lbl_1_bss_36B8[3].paths[2].count = 2;
    lbl_1_bss_36B8[3].paths[2].directions[0].x = -1.0f;
    lbl_1_bss_36B8[3].paths[2].directions[0].y = 0.0f;
    lbl_1_bss_36B8[3].paths[2].directions[0].z = 1.0f;
    lbl_1_bss_36B8[3].paths[2].points[0].x = 17.0f;
    lbl_1_bss_36B8[3].paths[2].points[0].y = 0.0f;
    lbl_1_bss_36B8[3].paths[2].points[0].z = -500.0f;
    lbl_1_bss_36B8[3].paths[2].directions[1].x = -1.0f;
    lbl_1_bss_36B8[3].paths[2].directions[1].y = 0.0f;
    lbl_1_bss_36B8[3].paths[2].directions[1].z = -1.0f;
    lbl_1_bss_36B8[3].paths[2].points[1].x = 40.0f;
    lbl_1_bss_36B8[3].paths[2].points[1].y = 0.0f;
    lbl_1_bss_36B8[3].paths[2].points[1].z = -741.0f;
    lbl_1_bss_36B8[3].paths[3].direction = 3;
    lbl_1_bss_36B8[3].paths[3].directions[0].x = 1.0f;
    lbl_1_bss_36B8[3].paths[3].directions[0].y = 0.0f;
    lbl_1_bss_36B8[3].paths[3].directions[0].z = 0.0f;
    lbl_1_bss_36B8[3].paths[3].points[0].x = -80.0f;
    lbl_1_bss_36B8[3].paths[3].points[0].y = 0.0f;
    lbl_1_bss_36B8[3].paths[3].points[0].z = -465.0f;
    lbl_1_bss_36B8[4].paths[0].direction = 0;
    lbl_1_bss_36B8[4].paths[0].directions[0].x = 0.0f;
    lbl_1_bss_36B8[4].paths[0].directions[0].y = 0.0f;
    lbl_1_bss_36B8[4].paths[0].directions[0].z = -1.0f;
    lbl_1_bss_36B8[4].paths[0].points[0].x = 17.0f;
    lbl_1_bss_36B8[4].paths[0].points[0].y = 0.0f;
    lbl_1_bss_36B8[4].paths[0].points[0].z = -362.0f;
    lbl_1_bss_36B8[4].paths[1].direction = 1;
    lbl_1_bss_36B8[4].paths[1].count = 2;
    lbl_1_bss_36B8[4].paths[1].directions[0].x = 1.0f;
    lbl_1_bss_36B8[4].paths[1].directions[0].y = 0.0f;
    lbl_1_bss_36B8[4].paths[1].directions[0].z = 1.0f;
    lbl_1_bss_36B8[4].paths[1].points[0].x = 446.0f;
    lbl_1_bss_36B8[4].paths[1].points[0].y = 0.0f;
    lbl_1_bss_36B8[4].paths[1].points[0].z = -318.0f;
    lbl_1_bss_36B8[4].paths[1].directions[1].x = -1.0f;
    lbl_1_bss_36B8[4].paths[1].directions[1].y = 0.0f;
    lbl_1_bss_36B8[4].paths[1].directions[1].z = 1.0f;
    lbl_1_bss_36B8[4].paths[1].points[1].x = 2.0f;
    lbl_1_bss_36B8[4].paths[1].points[1].y = 0.0f;
    lbl_1_bss_36B8[4].paths[1].points[1].z = -316.0f;
    lbl_1_bss_36B8[4].paths[3].direction = 3;
    lbl_1_bss_36B8[4].paths[3].count = 2;
    lbl_1_bss_36B8[4].paths[3].directions[0].x = 1.0f;
    lbl_1_bss_36B8[4].paths[3].directions[0].y = 0.0f;
    lbl_1_bss_36B8[4].paths[3].directions[0].z = 1.0f;
    lbl_1_bss_36B8[4].paths[3].points[0].x = 160.0f;
    lbl_1_bss_36B8[4].paths[3].points[0].y = 0.0f;
    lbl_1_bss_36B8[4].paths[3].points[0].z = -193.0f;
    lbl_1_bss_36B8[4].paths[3].directions[1].x = 1.0f;
    lbl_1_bss_36B8[4].paths[3].directions[1].y = 0.0f;
    lbl_1_bss_36B8[4].paths[3].directions[1].z = -1.0f;
    lbl_1_bss_36B8[4].paths[3].points[1].x = 121.0f;
    lbl_1_bss_36B8[4].paths[3].points[1].y = 0.0f;
    lbl_1_bss_36B8[4].paths[3].points[1].z = -444.0f;
    lbl_1_bss_36B8[5].paths[0].direction = 0;
    lbl_1_bss_36B8[5].paths[0].count = 2;
    lbl_1_bss_36B8[5].paths[0].directions[0].x = 1.0f;
    lbl_1_bss_36B8[5].paths[0].directions[0].y = 0.0f;
    lbl_1_bss_36B8[5].paths[0].directions[0].z = -1.0f;
    lbl_1_bss_36B8[5].paths[0].points[0].x = 442.0f;
    lbl_1_bss_36B8[5].paths[0].points[0].y = 0.0f;
    lbl_1_bss_36B8[5].paths[0].points[0].z = -153.0f;
    lbl_1_bss_36B8[5].paths[0].directions[1].x = -1.0f;
    lbl_1_bss_36B8[5].paths[0].directions[1].y = 0.0f;
    lbl_1_bss_36B8[5].paths[0].directions[1].z = -1.0f;
    lbl_1_bss_36B8[5].paths[0].points[1].x = 217.0f;
    lbl_1_bss_36B8[5].paths[0].points[1].y = 0.0f;
    lbl_1_bss_36B8[5].paths[0].points[1].z = -141.0f;
    lbl_1_bss_36B8[5].paths[1].direction = 1;
    lbl_1_bss_36B8[5].paths[1].count = 2;
    lbl_1_bss_36B8[5].paths[1].directions[0].x = 1.0f;
    lbl_1_bss_36B8[5].paths[1].directions[0].y = 0.0f;
    lbl_1_bss_36B8[5].paths[1].directions[0].z = 1.0f;
    lbl_1_bss_36B8[5].paths[1].points[0].x = 446.0f;
    lbl_1_bss_36B8[5].paths[1].points[0].y = 0.0f;
    lbl_1_bss_36B8[5].paths[1].points[0].z = -318.0f;
    lbl_1_bss_36B8[5].paths[1].directions[1].x = -1.0f;
    lbl_1_bss_36B8[5].paths[1].directions[1].y = 0.0f;
    lbl_1_bss_36B8[5].paths[1].directions[1].z = 1.0f;
    lbl_1_bss_36B8[5].paths[1].points[1].x = 2.0f;
    lbl_1_bss_36B8[5].paths[1].points[1].y = 0.0f;
    lbl_1_bss_36B8[5].paths[1].points[1].z = -316.0f;
    lbl_1_bss_36B8[5].paths[2].direction = 2;
    lbl_1_bss_36B8[5].paths[2].count = 2;
    lbl_1_bss_36B8[5].paths[2].directions[0].x = -1.0f;
    lbl_1_bss_36B8[5].paths[2].directions[0].y = 0.0f;
    lbl_1_bss_36B8[5].paths[2].directions[0].z = 1.0f;
    lbl_1_bss_36B8[5].paths[2].points[0].x = 316.0f;
    lbl_1_bss_36B8[5].paths[2].points[0].y = 0.0f;
    lbl_1_bss_36B8[5].paths[2].points[0].z = -46.0f;
    lbl_1_bss_36B8[5].paths[2].directions[1].x = -1.0f;
    lbl_1_bss_36B8[5].paths[2].directions[1].y = 0.0f;
    lbl_1_bss_36B8[5].paths[2].directions[1].z = -1.0f;
    lbl_1_bss_36B8[5].paths[2].points[1].x = 324.0f;
    lbl_1_bss_36B8[5].paths[2].points[1].y = 0.0f;
    lbl_1_bss_36B8[5].paths[2].points[1].z = -442.0f;
    lbl_1_bss_36B8[5].paths[3].direction = 3;
    lbl_1_bss_36B8[5].paths[3].directions[0].x = 1.0f;
    lbl_1_bss_36B8[5].paths[3].directions[0].y = 0.0f;
    lbl_1_bss_36B8[5].paths[3].directions[0].z = 0.0f;
    lbl_1_bss_36B8[5].paths[3].points[0].x = 365.0f;
    lbl_1_bss_36B8[5].paths[3].points[0].y = 0.0f;
    lbl_1_bss_36B8[5].paths[3].points[0].z = -46.0f;
    lbl_1_bss_36B8[6].paths[0].direction = 0;
    lbl_1_bss_36B8[6].paths[0].directions[0].x = 0.0f;
    lbl_1_bss_36B8[6].paths[0].directions[0].y = 0.0f;
    lbl_1_bss_36B8[6].paths[0].directions[0].z = -1.0f;
    lbl_1_bss_36B8[6].paths[0].points[0].x = 432.0f;
    lbl_1_bss_36B8[6].paths[0].points[0].y = 0.0f;
    lbl_1_bss_36B8[6].paths[0].points[0].z = 410.0f;
    lbl_1_bss_36B8[6].paths[1].direction = 1;
    lbl_1_bss_36B8[6].paths[1].directions[0].x = 0.0f;
    lbl_1_bss_36B8[6].paths[1].directions[0].y = 0.0f;
    lbl_1_bss_36B8[6].paths[1].directions[0].z = 1.0f;
    lbl_1_bss_36B8[6].paths[1].points[0].x = 519.0f;
    lbl_1_bss_36B8[6].paths[1].points[0].y = 0.0f;
    lbl_1_bss_36B8[6].paths[1].points[0].z = -17.0f;
    lbl_1_bss_36B8[7].paths[1].direction = 1;
    lbl_1_bss_36B8[7].paths[1].directions[0].x = 0.0f;
    lbl_1_bss_36B8[7].paths[1].directions[0].y = 0.0f;
    lbl_1_bss_36B8[7].paths[1].directions[0].z = 1.0f;
    lbl_1_bss_36B8[7].paths[1].points[0].x = 519.0f;
    lbl_1_bss_36B8[7].paths[1].points[0].y = 0.0f;
    lbl_1_bss_36B8[7].paths[1].points[0].z = -17.0f;
    lbl_1_bss_36B8[7].paths[2].direction = 2;
    lbl_1_bss_36B8[7].paths[2].directions[0].x = -1.0f;
    lbl_1_bss_36B8[7].paths[2].directions[0].y = 0.0f;
    lbl_1_bss_36B8[7].paths[2].directions[0].z = 0.0f;
    lbl_1_bss_36B8[7].paths[2].points[0].x = 576.0f;
    lbl_1_bss_36B8[7].paths[2].points[0].y = 0.0f;
    lbl_1_bss_36B8[7].paths[2].points[0].z = 446.0f;
    lbl_1_bss_36B8[7].paths[3].direction = 3;
    lbl_1_bss_36B8[7].paths[3].count = 2;
    lbl_1_bss_36B8[7].paths[3].directions[0].x = 1.0f;
    lbl_1_bss_36B8[7].paths[3].directions[0].y = 0.0f;
    lbl_1_bss_36B8[7].paths[3].directions[0].z = 1.0f;
    lbl_1_bss_36B8[7].paths[3].points[0].x = 624.0f;
    lbl_1_bss_36B8[7].paths[3].points[0].y = 0.0f;
    lbl_1_bss_36B8[7].paths[3].points[0].z = 458.0f;
    lbl_1_bss_36B8[7].paths[3].directions[1].x = 1.0f;
    lbl_1_bss_36B8[7].paths[3].directions[1].y = 0.0f;
    lbl_1_bss_36B8[7].paths[3].directions[1].z = -1.0f;
    lbl_1_bss_36B8[7].paths[3].points[1].x = 615.0f;
    lbl_1_bss_36B8[7].paths[3].points[1].y = 0.0f;
    lbl_1_bss_36B8[7].paths[3].points[1].z = -184.0f;
    lbl_1_bss_36B8[8].paths[0].direction = 0;
    lbl_1_bss_36B8[8].paths[0].count = 2;
    lbl_1_bss_36B8[8].paths[0].directions[0].x = 1.0f;
    lbl_1_bss_36B8[8].paths[0].directions[0].y = 0.0f;
    lbl_1_bss_36B8[8].paths[0].directions[0].z = -1.0f;
    lbl_1_bss_36B8[8].paths[0].points[0].x = 746.0f;
    lbl_1_bss_36B8[8].paths[0].points[0].y = 0.0f;
    lbl_1_bss_36B8[8].paths[0].points[0].z = 317.0f;
    lbl_1_bss_36B8[8].paths[0].directions[1].x = -1.0f;
    lbl_1_bss_36B8[8].paths[0].directions[1].y = 0.0f;
    lbl_1_bss_36B8[8].paths[0].directions[1].z = -1.0f;
    lbl_1_bss_36B8[8].paths[0].points[1].x = 486.0f;
    lbl_1_bss_36B8[8].paths[0].points[1].y = 0.0f;
    lbl_1_bss_36B8[8].paths[0].points[1].z = 314.0f;
    lbl_1_bss_36B8[8].paths[2].direction = 2;
    lbl_1_bss_36B8[8].paths[2].directions[0].x = -1.0f;
    lbl_1_bss_36B8[8].paths[2].directions[0].y = 0.0f;
    lbl_1_bss_36B8[8].paths[2].directions[0].z = 0.0f;
    lbl_1_bss_36B8[8].paths[2].points[0].x = 576.0f;
    lbl_1_bss_36B8[8].paths[2].points[0].y = 0.0f;
    lbl_1_bss_36B8[8].paths[2].points[0].z = 446.0f;
    lbl_1_bss_36B8[8].paths[3].direction = 3;
    lbl_1_bss_36B8[8].paths[3].count = 2;
    lbl_1_bss_36B8[8].paths[3].directions[0].x = 1.0f;
    lbl_1_bss_36B8[8].paths[3].directions[0].y = 0.0f;
    lbl_1_bss_36B8[8].paths[3].directions[0].z = 1.0f;
    lbl_1_bss_36B8[8].paths[3].points[0].x = 624.0f;
    lbl_1_bss_36B8[8].paths[3].points[0].y = 0.0f;
    lbl_1_bss_36B8[8].paths[3].points[0].z = 458.0f;
    lbl_1_bss_36B8[8].paths[3].directions[1].x = 1.0f;
    lbl_1_bss_36B8[8].paths[3].directions[1].y = 0.0f;
    lbl_1_bss_36B8[8].paths[3].directions[1].z = -1.0f;
    lbl_1_bss_36B8[8].paths[3].points[1].x = 615.0f;
    lbl_1_bss_36B8[8].paths[3].points[1].y = 0.0f;
    lbl_1_bss_36B8[8].paths[3].points[1].z = -184.0f;
    lbl_1_bss_36B8[9].paths[0].direction = 0;
    lbl_1_bss_36B8[9].paths[0].count = 2;
    lbl_1_bss_36B8[9].paths[0].directions[0].x = 1.0f;
    lbl_1_bss_36B8[9].paths[0].directions[0].y = 0.0f;
    lbl_1_bss_36B8[9].paths[0].directions[0].z = -1.0f;
    lbl_1_bss_36B8[9].paths[0].points[0].x = -210.0f;
    lbl_1_bss_36B8[9].paths[0].points[0].y = 0.0f;
    lbl_1_bss_36B8[9].paths[0].points[0].z = 320.0f;
    lbl_1_bss_36B8[9].paths[0].directions[1].x = -1.0f;
    lbl_1_bss_36B8[9].paths[0].directions[1].y = 0.0f;
    lbl_1_bss_36B8[9].paths[0].directions[1].z = -1.0f;
    lbl_1_bss_36B8[9].paths[0].points[1].x = -645.0f;
    lbl_1_bss_36B8[9].paths[0].points[1].y = 0.0f;
    lbl_1_bss_36B8[9].paths[0].points[1].z = 320.0f;
    lbl_1_bss_36B8[9].paths[1].direction = 1;
    lbl_1_bss_36B8[9].paths[1].directions[0].x = 0.0f;
    lbl_1_bss_36B8[9].paths[1].directions[0].y = 0.0f;
    lbl_1_bss_36B8[9].paths[1].directions[0].z = 1.0f;
    lbl_1_bss_36B8[9].paths[1].points[0].x = -659.0f;
    lbl_1_bss_36B8[9].paths[1].points[0].y = 0.0f;
    lbl_1_bss_36B8[9].paths[1].points[0].z = 355.0f;
    lbl_1_bss_36B8[9].paths[3].direction = 3;
    lbl_1_bss_36B8[9].paths[3].count = 2;
    lbl_1_bss_36B8[9].paths[3].directions[0].x = 1.0f;
    lbl_1_bss_36B8[9].paths[3].directions[0].y = 0.0f;
    lbl_1_bss_36B8[9].paths[3].directions[0].z = 1.0f;
    lbl_1_bss_36B8[9].paths[3].points[0].x = -519.0f;
    lbl_1_bss_36B8[9].paths[3].points[0].y = 0.0f;
    lbl_1_bss_36B8[9].paths[3].points[0].z = 415.0f;
    lbl_1_bss_36B8[9].paths[3].directions[1].x = 1.0f;
    lbl_1_bss_36B8[9].paths[3].directions[1].y = 0.0f;
    lbl_1_bss_36B8[9].paths[3].directions[1].z = -1.0f;
    lbl_1_bss_36B8[9].paths[3].points[1].x = -543.0f;
    lbl_1_bss_36B8[9].paths[3].points[1].y = 0.0f;
    lbl_1_bss_36B8[9].paths[3].points[1].z = 218.0f;
    lbl_1_bss_36B8[10].paths[0].direction = 0;
    lbl_1_bss_36B8[10].paths[0].count = 2;
    lbl_1_bss_36B8[10].paths[0].directions[0].x = 1.0f;
    lbl_1_bss_36B8[10].paths[0].directions[0].y = 0.0f;
    lbl_1_bss_36B8[10].paths[0].directions[0].z = -1.0f;
    lbl_1_bss_36B8[10].paths[0].points[0].x = -210.0f;
    lbl_1_bss_36B8[10].paths[0].points[0].y = 0.0f;
    lbl_1_bss_36B8[10].paths[0].points[0].z = 320.0f;
    lbl_1_bss_36B8[10].paths[0].directions[1].x = -1.0f;
    lbl_1_bss_36B8[10].paths[0].directions[1].y = 0.0f;
    lbl_1_bss_36B8[10].paths[0].directions[1].z = -1.0f;
    lbl_1_bss_36B8[10].paths[0].points[1].x = -645.0f;
    lbl_1_bss_36B8[10].paths[0].points[1].y = 0.0f;
    lbl_1_bss_36B8[10].paths[0].points[1].z = 320.0f;
    lbl_1_bss_36B8[10].paths[1].direction = 1;
    lbl_1_bss_36B8[10].paths[1].count = 2;
    lbl_1_bss_36B8[10].paths[1].directions[0].x = 1.0f;
    lbl_1_bss_36B8[10].paths[1].directions[0].y = 0.0f;
    lbl_1_bss_36B8[10].paths[1].directions[0].z = 1.0f;
    lbl_1_bss_36B8[10].paths[1].points[0].x = -215.0f;
    lbl_1_bss_36B8[10].paths[1].points[0].y = 0.0f;
    lbl_1_bss_36B8[10].paths[1].points[0].z = 162.0f;
    lbl_1_bss_36B8[10].paths[1].directions[1].x = -1.0f;
    lbl_1_bss_36B8[10].paths[1].directions[1].y = 0.0f;
    lbl_1_bss_36B8[10].paths[1].directions[1].z = 1.0f;
    lbl_1_bss_36B8[10].paths[1].points[1].x = -415.0f;
    lbl_1_bss_36B8[10].paths[1].points[1].y = 0.0f;
    lbl_1_bss_36B8[10].paths[1].points[1].z = 160.0f;
    lbl_1_bss_36B8[10].paths[2].direction = 2;
    lbl_1_bss_36B8[10].paths[2].count = 2;
    lbl_1_bss_36B8[10].paths[2].directions[0].x = -1.0f;
    lbl_1_bss_36B8[10].paths[2].directions[0].y = 0.0f;
    lbl_1_bss_36B8[10].paths[2].directions[0].z = 1.0f;
    lbl_1_bss_36B8[10].paths[2].points[0].x = -313.0f;
    lbl_1_bss_36B8[10].paths[2].points[0].y = 0.0f;
    lbl_1_bss_36B8[10].paths[2].points[0].z = 430.0f;
    lbl_1_bss_36B8[10].paths[2].directions[1].x = -1.0f;
    lbl_1_bss_36B8[10].paths[2].directions[1].y = 0.0f;
    lbl_1_bss_36B8[10].paths[2].directions[1].z = -1.0f;
    lbl_1_bss_36B8[10].paths[2].points[1].x = -337.0f;
    lbl_1_bss_36B8[10].paths[2].points[1].y = 0.0f;
    lbl_1_bss_36B8[10].paths[2].points[1].z = 56.0f;
    lbl_1_bss_36B8[10].paths[3].direction = 3;
    lbl_1_bss_36B8[10].paths[3].count = 2;
    lbl_1_bss_36B8[10].paths[3].directions[0].x = 1.0f;
    lbl_1_bss_36B8[10].paths[3].directions[0].y = 0.0f;
    lbl_1_bss_36B8[10].paths[3].directions[0].z = 0.0f;
    lbl_1_bss_36B8[10].paths[3].points[0].x = -265.0f;
    lbl_1_bss_36B8[10].paths[3].points[0].y = 0.0f;
    lbl_1_bss_36B8[10].paths[3].points[0].z = 17.0f;
    lbl_1_bss_36B8[11].paths[1].direction = 1;
    lbl_1_bss_36B8[11].paths[1].count = 2;
    lbl_1_bss_36B8[11].paths[1].directions[0].x = 1.0f;
    lbl_1_bss_36B8[11].paths[1].directions[0].y = 0.0f;
    lbl_1_bss_36B8[11].paths[1].directions[0].z = 1.0f;
    lbl_1_bss_36B8[11].paths[1].points[0].x = -289.0f;
    lbl_1_bss_36B8[11].paths[1].points[0].y = 0.0f;
    lbl_1_bss_36B8[11].paths[1].points[0].z = 674.0f;
    lbl_1_bss_36B8[11].paths[1].directions[1].x = -1.0f;
    lbl_1_bss_36B8[11].paths[1].directions[1].y = 0.0f;
    lbl_1_bss_36B8[11].paths[1].directions[1].z = 1.0f;
    lbl_1_bss_36B8[11].paths[1].points[1].x = -524.0f;
    lbl_1_bss_36B8[11].paths[1].points[1].y = 0.0f;
    lbl_1_bss_36B8[11].paths[1].points[1].z = 668.0f;
    lbl_1_bss_36B8[11].paths[2].direction = 2;
    lbl_1_bss_36B8[11].paths[2].directions[0].x = -1.0f;
    lbl_1_bss_36B8[11].paths[2].directions[0].y = 0.0f;
    lbl_1_bss_36B8[11].paths[2].directions[0].z = 0.0f;
    lbl_1_bss_36B8[11].paths[2].points[0].x = -403.0f;
    lbl_1_bss_36B8[11].paths[2].points[0].y = 0.0f;
    lbl_1_bss_36B8[11].paths[2].points[0].z = 556.0f;
    lbl_1_bss_36B8[11].paths[3].direction = 3;
    lbl_1_bss_36B8[11].paths[3].directions[0].x = 1.0f;
    lbl_1_bss_36B8[11].paths[3].directions[0].y = 0.0f;
    lbl_1_bss_36B8[11].paths[3].directions[0].z = 0.0f;
    lbl_1_bss_36B8[11].paths[3].points[0].x = -414.0f;
    lbl_1_bss_36B8[11].paths[3].points[0].y = 0.0f;
    lbl_1_bss_36B8[11].paths[3].points[0].z = 530.0f;
    lbl_1_bss_36B8[12].paths[2].direction = 2;
    lbl_1_bss_36B8[12].paths[2].directions[0].x = -1.0f;
    lbl_1_bss_36B8[12].paths[2].directions[0].y = 0.0f;
    lbl_1_bss_36B8[12].paths[2].directions[0].z = 0.0f;
    lbl_1_bss_36B8[12].paths[2].points[0].x = 378.0f;
    lbl_1_bss_36B8[12].paths[2].points[0].y = 0.0f;
    lbl_1_bss_36B8[12].paths[2].points[0].z = 490.0f;
    lbl_1_bss_36B8[12].paths[3].direction = 3;
    lbl_1_bss_36B8[12].paths[3].directions[0].x = 1.0f;
    lbl_1_bss_36B8[12].paths[3].directions[0].y = 0.0f;
    lbl_1_bss_36B8[12].paths[3].directions[0].z = 0.0f;
    lbl_1_bss_36B8[12].paths[3].points[0].x = 42.0f;
    lbl_1_bss_36B8[12].paths[3].points[0].y = 0.0f;
    lbl_1_bss_36B8[12].paths[3].points[0].z = 412.0f;
    lbl_1_bss_36B8[13].paths[0].direction = 0;
    lbl_1_bss_36B8[13].paths[0].directions[0].x = 0.0f;
    lbl_1_bss_36B8[13].paths[0].directions[0].y = 0.0f;
    lbl_1_bss_36B8[13].paths[0].directions[0].z = -1.0f;
    lbl_1_bss_36B8[13].paths[0].points[0].x = 81.0f;
    lbl_1_bss_36B8[13].paths[0].points[0].y = 0.0f;
    lbl_1_bss_36B8[13].paths[0].points[0].z = 587.0f;
    lbl_1_bss_36B8[13].paths[1].direction = 1;
    lbl_1_bss_36B8[13].paths[1].count = 2;
    lbl_1_bss_36B8[13].paths[1].directions[0].x = 1.0f;
    lbl_1_bss_36B8[13].paths[1].directions[0].y = 0.0f;
    lbl_1_bss_36B8[13].paths[1].directions[0].z = 1.0f;
    lbl_1_bss_36B8[13].paths[1].points[0].x = 497.0f;
    lbl_1_bss_36B8[13].paths[1].points[0].y = 0.0f;
    lbl_1_bss_36B8[13].paths[1].points[0].z = 612.0f;
    lbl_1_bss_36B8[13].paths[1].directions[1].x = -1.0f;
    lbl_1_bss_36B8[13].paths[1].directions[1].y = 0.0f;
    lbl_1_bss_36B8[13].paths[1].directions[1].z = 1.0f;
    lbl_1_bss_36B8[13].paths[1].points[1].x = 117.0f;
    lbl_1_bss_36B8[13].paths[1].points[1].y = 0.0f;
    lbl_1_bss_36B8[13].paths[1].points[1].z = 598.0f;
    lbl_1_bss_36B8[13].paths[2].direction = 2;
    lbl_1_bss_36B8[13].paths[2].directions[0].x = -1.0f;
    lbl_1_bss_36B8[13].paths[2].directions[0].y = 0.0f;
    lbl_1_bss_36B8[13].paths[2].directions[0].z = 0.0f;
    lbl_1_bss_36B8[13].paths[2].points[0].x = 378.0f;
    lbl_1_bss_36B8[13].paths[2].points[0].y = 0.0f;
    lbl_1_bss_36B8[13].paths[2].points[0].z = 490.0f;
    lbl_1_bss_36B8[13].paths[3].direction = 3;
    lbl_1_bss_36B8[13].paths[3].count = 2;
    lbl_1_bss_36B8[13].paths[3].directions[0].x = 1.0f;
    lbl_1_bss_36B8[13].paths[3].directions[0].y = 0.0f;
    lbl_1_bss_36B8[13].paths[3].directions[0].z = 1.0f;
    lbl_1_bss_36B8[13].paths[3].points[0].x = 204.0f;
    lbl_1_bss_36B8[13].paths[3].points[0].y = 0.0f;
    lbl_1_bss_36B8[13].paths[3].points[0].z = 734.0f;
    lbl_1_bss_36B8[13].paths[3].directions[1].x = 1.0f;
    lbl_1_bss_36B8[13].paths[3].directions[1].y = 0.0f;
    lbl_1_bss_36B8[13].paths[3].directions[1].z = -1.0f;
    lbl_1_bss_36B8[13].paths[3].points[1].x = 248.0f;
    lbl_1_bss_36B8[13].paths[3].points[1].y = 0.0f;
    lbl_1_bss_36B8[13].paths[3].points[1].z = 508.0f;

    for (index = 0; index < 14; index++) {
        for (lane = 0; lane < 4; lane++) {
            if (lbl_1_bss_36B8[index].paths[lane].direction != -1) {
                for (pointIndex = 0; pointIndex < lbl_1_bss_36B8[index].paths[lane].count;
                     pointIndex++) {
                    position = lbl_1_bss_36B8[index].paths[lane].points[pointIndex];
                    angle = lbl_1_bss_1F4.angle +
                        180.0 * (atan2(position.x, position.z) / 3.141592653589793);
                    radius = sqrt(position.x * position.x + position.z * position.z);
                    position.x = radius * sin((3.141592653589793 * angle) / 180.0);
                    position.z = radius * cos((3.141592653589793 * angle) / 180.0);
                    lbl_1_bss_36B8[index].paths[lane].points[pointIndex] = position;
                    direction = lbl_1_bss_36B8[index].paths[lane].directions[pointIndex];
                    for (directionIndex = 0; directionIndex < 8; directionIndex++) {
                        if (direction.x == directionTable[directionIndex][1] &&
                            direction.z == directionTable[directionIndex][2]) {
                            angle = directionTable[directionIndex][0];
                            break;
                        }
                    }
                    angle += lbl_1_bss_1F4.angle;
                    if (angle >= 360.0f) angle -= 360.0f;
                    directionIndex = angle / 45.0f;
                    direction.x = directionTable[directionIndex][1];
                    direction.z = directionTable[directionIndex][2];
                    lbl_1_bss_36B8[index].paths[lane].directions[pointIndex] = direction;
                }
            }
        }
    }
    directionIndex = lbl_1_bss_1F4.angle / 90.0f;
    for (index = 0; index < directionIndex; index++) {
        for (lane = 0; lane < 14; lane++) {
            savedPath = lbl_1_bss_36B8[lane].paths[2];
            lbl_1_bss_36B8[lane].paths[2] = lbl_1_bss_36B8[lane].paths[0];
            lbl_1_bss_36B8[lane].paths[0] = lbl_1_bss_36B8[lane].paths[3];
            lbl_1_bss_36B8[lane].paths[3] = lbl_1_bss_36B8[lane].paths[1];
            lbl_1_bss_36B8[lane].paths[1] = savedPath;
        }
    }
}

/* Called by fn_1_7008 after input sampling to update CPU route following when inputDisabled is
 * zero or two. */
void fn_1_11F50(void)
{
    s16 zero = 0;
    s16 index;
    M660Player *player;

    index = 0;
    while (index < 2) {
        player = &lbl_1_bss_2DC[index];
        if (player->type != 0 &&
            (player->inputDisabled == 0 || player->inputDisabled == 2)) {
            player->buttonsWork = 0;
            player->pressedButtonsWork = 0;
            player->stickXWork = 0;
            player->stickYWork = 0;
            switch (player->followState) {
            case 0:
                player->followState++;
            case 1:
                fn_1_13C6C(player);
                break;
            }
        }
        index++;
    }
}

/* Turns and moves a player toward the current path target; movement callbacks use it. */
s16 fn_1_12030(M660Player *player, s16 advanceState)
{
    s16 result = 0;
    s16 chaseResult = 0;
    Vec target;
    Vec delta;
    switch (player->chaseState)
    {
    case 0 :
        {
            f32 angleDegrees;
            f32 adjustment;
            f32 speedOffset;
            f32 distanceSquared;
            MgPlayerComStkOn(lbl_1_bss_1E8.players[player->playerIndex]);
            player->previousChasePosition = player->currentPosition;
            delta.x = player->currentPosition.x - player->position.x;
            delta.z = player->currentPosition.z - player->position.z;
            angleDegrees =
                (f32) (180.0 * ((f64) atan2((f64) delta.x, (f64) delta.z) / 3.141592653589793));
            player->angle = angleDegrees;
            distanceSquared = (delta.x * delta.x) + (delta.z * delta.z);
            player->distance = (f32) sqrt((f64) distanceSquared);
            speedOffset = player->rotation.y;
            if ((angleDegrees > 90.0f &&
                 speedOffset < -90.0f) ||
                (angleDegrees < -90.0f &&
                 speedOffset > 90.0f)) {
                angleDegrees = 0.0f > angleDegrees ?
                    180.0f + angleDegrees : 180.0f - angleDegrees;
                speedOffset = 0.0f > speedOffset ?
                    180.0f + speedOffset : 180.0f - speedOffset;
                adjustment = angleDegrees + speedOffset > 0.0f ?
                    angleDegrees + speedOffset : -1.0f * (angleDegrees + speedOffset);
                if (player->rotation.y > 0.0f)
                {
                    adjustment *= -1.0f;
                }
            } else {
                adjustment = speedOffset - angleDegrees;
            }
            player->steeringX = player->steeringZ = adjustment;
            player->movementOrigin = player->position;
            player->chaseState = 2;
            break;
        }
    case 1 :
        {
            if (player->distance > 5.0f)
            {
                f32 step = 10.0f;
                chaseResult = fn_1_2E80(& player->steeringX, step, player->steeringZ, 0.0f);
                target.x =
                    (f32) ((f64) player->movementOrigin.x +
                           ((f64) player->distance *
                            sin((3.141592653589793 * (f64) (player->angle + player->steeringX)) /
                                180.0)));
                target.z =
                    (f32) ((f64) player->movementOrigin.z +
                           ((f64) player->distance *
                            cos((3.141592653589793 * (f64) (player->angle + player->steeringX)) /
                                180.0)));
                MgPlayerVecChase(lbl_1_bss_1E8.players[player->playerIndex], (HuVecF *) &target,
                                 player->forwardSpeed, player->arrivalRadius);
                if (chaseResult == 0)
                {
                    player->chaseState++;
                }
            }
            else
            {
                player->chaseState++;
            }
            break;
        }
    case 2 :
        {
            player->previousChasePosition = player->currentPosition;
            target = player->previousChasePosition;
            chaseResult =
                MgPlayerVecChase(lbl_1_bss_1E8.players[player->playerIndex], (HuVecF *) &target,
                                 player->forwardSpeed, player->arrivalRadius);
            if (0 != chaseResult)
            {
                if (advanceState)
                {
                    player->chaseState++;
                }
                else
                {
                    result = 1;
                }
            }
            break;
        }
    case 3 :
        {
            MgPlayerComStkOff(lbl_1_bss_1E8.players[player->playerIndex]);
            player->chaseState = 0;
            result = 1;
            break;
        }
    default :
        break;
    }
    return result;
}

/* Resolves repeated path collisions and redirects both players when needed; called by
 * fn_1_12CA8. */
s16 fn_1_1265C(M660Player *player)
{
    s16 result = 0;

    switch (player->routeState) {
    case 1:
        break;
    case 0:
    if (player->collisionCount > 10 &&
        lbl_1_bss_2DC[player->playerIndex ^ 1].type != 0) {
        s16 pointIndices[2];
        pointIndices[0] = lbl_1_bss_2DC[0].destinationPointIndex;
        pointIndices[1] = lbl_1_bss_2DC[1].destinationPointIndex;

        if (player->difficulty != lbl_1_bss_2DC[player->playerIndex ^ 1].difficulty &&
            ((lbl_1_bss_2688[pointIndices[0]].flags & 0xFF) != 0 ||
             (lbl_1_bss_2688[pointIndices[1]].flags & 0xFF) != 0)) {
            s16 playerIndex;
            s16 opponentIndex;

            playerIndex = lbl_1_bss_2DC[0].difficulty < lbl_1_bss_2DC[1].difficulty
                              ? 0
                              : 1;
            opponentIndex = playerIndex ^ 1;
            pointIndices[0] = lbl_1_bss_2DC[playerIndex].destinationPointIndex;
            pointIndices[1] = lbl_1_bss_2DC[opponentIndex].destinationPointIndex;
            {
                Vec delta;
                f32 distanceSquared;
                f32 distance;
                f32 scale;

                delta.x = lbl_1_bss_2688[pointIndices[1]].position.x -
                          lbl_1_bss_2DC[opponentIndex].position.x;
                delta.z = lbl_1_bss_2688[pointIndices[1]].position.z -
                          lbl_1_bss_2DC[opponentIndex].position.z;
                distanceSquared =
                    (delta.x * delta.x) + (delta.z * delta.z);
                distance = (f32)sqrt((f64)distanceSquared);
                distance = distance == 0.0f ? 1.0f : distance;
                scale = distance ? 200.0f / distance : 0.0f;

                delta.x *= scale;
                delta.z *= scale;
                lbl_1_bss_2DC[playerIndex].currentPosition.x =
                    delta.x + lbl_1_bss_2DC[opponentIndex].position.x;
                lbl_1_bss_2DC[playerIndex].currentPosition.z =
                    delta.z + lbl_1_bss_2DC[opponentIndex].position.z;
            }
            lbl_1_bss_2DC[playerIndex].destination =
                lbl_1_bss_2DC[playerIndex].currentPosition;
        } else {
            pointIndices[0] = player->destinationPointIndex;
            player->destinationPointIndex =
                lbl_1_bss_2DC[player->playerIndex ^ 1].destinationPointIndex;
            lbl_1_bss_2DC[player->playerIndex ^ 1].destinationPointIndex =
                pointIndices[0];
            player->destination =
                lbl_1_bss_2688[player->destinationPointIndex].position;
            player->currentPosition = player->destination;
            lbl_1_bss_2DC[player->playerIndex ^ 1].currentPosition =
                lbl_1_bss_2DC[player->playerIndex ^ 1].destination =
                    lbl_1_bss_2688[
                        lbl_1_bss_2DC[player->playerIndex ^ 1].destinationPointIndex]
                        .position;
        }
        player->collisionCount = 0;
        lbl_1_bss_2DC[player->playerIndex ^ 1].collisionCount = 0;
        result = 2;
    }
        break;
    default:
        break;
    }
    return result;
}

/* Debug hook disabled in release builds. */
static inline void DebugModel(M660Model *object) {}

/* Debug hook disabled in release builds. */
static inline void DebugTimer(s16 timer) {}

/* Advances a player across connected route lanes; called by fn_1_13C6C. */
s16 fn_1_12CA8(M660Player *player)
{
    s16 changed = 0;
    s16 waitTimer;
    s16 routeChanged;
    s16 reached;
    s16 pointIndex;
    s16 moving = 0;
    M660Model *secondObject;
    Vec direction;
    direction.x = (player->destination.x - player->position.x) > 0.0f ? 1.0f : -1.0f;
    direction.y = 0.0f;
    direction.z = (player->destination.z - player->position.z) > 0.0f ? 1.0f : -1.0f;
    player->movementTimer++;
    switch (player->movementState)
    {
    case 0 :
        {
            player->currentPosition = player->destination;
            routeChanged = fn_1_1265C(player);
            if (player->reached == 0 && player->arrived != 0)
            {
                player->movementTimer = 0;
                changed = 1;
                goto done;
            }
            moving = fn_1_12030(player, 0);
            if (routeChanged != 0)
            {
                goto done;
            }
            if (moving != 0 || player->movementTimer > 300)
            {
                player->movementTimer = 0;
                changed = 1;
                goto done;
            }
            if (player->contacts[0] == - 1)
            {
                goto done;
            }
            player->movementState++;
        }
    case 1 :
        {
            s16 routeIndex;
            s16 lane;
            M660Model *firstObject = &lbl_1_bss_60C[player->contacts[0]];
            DebugModel(firstObject);
            secondObject = &lbl_1_bss_60C[player->contacts[1]];
            DebugModel(secondObject);
            routeIndex = player->contacts[0] - 31;
            lane = player->contactEdges[0];
            if (lbl_1_bss_36B8[routeIndex].paths[lane].direction != - 1)
            {
                pointIndex = 0;
                while (pointIndex < lbl_1_bss_36B8[routeIndex].paths[lane].count)
                {
                    reached = 0;
                    if (lbl_1_bss_36B8[routeIndex].paths[lane].directions[pointIndex].x) {
                        if (!direction.x ||
                            lbl_1_bss_36B8[routeIndex].paths[lane].directions[pointIndex].x ==
                                direction.x) {
                            reached++;
                        }
                    } else {
                        reached++;
                    }
                    if (lbl_1_bss_36B8[routeIndex].paths[lane].directions[pointIndex].z) {
                        if (!direction.z ||
                            lbl_1_bss_36B8[routeIndex].paths[lane].directions[pointIndex].z ==
                                direction.z) {
                            reached++;
                        }
                    } else {
                        reached++;
                    }
                    if (reached >= 2)
                    {
                        player->selectedRoute = routeIndex;
                        player->selectedPath = lane;
                        player->selectedPoint = pointIndex;
                        player->waitTimer = 0;
                        player->movementState = 2;
                        break;
                    }
                    pointIndex++;
                }
            }
            if (player->movementState != 2)
            {
                player->movementState = 0;
                break;
            }
            break;
        }
    case 2 :
        {
            s32 samePath;
            s32 sameObject;
            player->currentPosition = lbl_1_bss_36B8[player->selectedRoute]
                                          .paths[player->selectedPath]
                                          .points[player->selectedPoint];
            moving = fn_1_1265C(player);
            if (moving == 2)
            {
                player->movementState = 0;
                goto done;
            }
            samePath = 0;
            sameObject = 0;
            if (player->contacts[0] != - 1 && player->contacts[0] == player->oldContacts[0])
            {
                sameObject = 1;
            }
            if (sameObject != 0 && player->contactEdges[0] == player->oldContactEdges[0])
            {
                samePath = 1;
            }
            if (samePath != 0)
            {
                waitTimer = player->waitTimer++;
                DebugTimer(waitTimer);
            }
            else
            {
                player->waitTimer = waitTimer = 0;
                DebugTimer(waitTimer);
            }
            if (player->waitTimer > 60)
            {
                player->movementState = 0;
                goto done;
            }
            player->arrivalRadius = 10.1f;
            moving = fn_1_12030(player, 0);
            if (moving != 0)
            {
                player->movementState = 0;
            }
            break;
        }
    default :
        break;
    }
    done : return changed;
}

/* Selects an eligible grid point as a player's destination; called by fn_1_13C6C. */
s16 fn_1_131C8(M660Player *player, s16 selectFarthest, s16 useAlternateMode)
{
    f32 bestDistance;
    f32 minimumDistance;
    f32 nearbyRadius;
    Vec delta;
    f32 distanceSquared;
    f32 distance;
    f32 otherDistanceSquared;
    f32 otherDistance;
    Vec selectedPosition;
    s16 selectedPoint;
    s16 candidateCount;
    s16 routeMode;
    s16 found = 0;
    u16 routeModes[3] = {45, 50, 53};
    s16 index;

    bestDistance = selectFarthest != 0 ? -1.0f : 99999.0f;
    minimumDistance = 100.0f;
    nearbyRadius = lbl_1_bss_60C[((s16 *)routeModes)[lbl_1_bss_1F4.effectIndex]].bounds.x;
    /* Alternate mode forces the nearest-point search even when selectFarthest was requested. */
    if (useAlternateMode != 0) {
        routeMode = 16;
        selectFarthest = 0;
        bestDistance = 99999.0f;
    } else {
        routeMode = 0;
    }

    index = 0;
    candidateCount = 0;
    while (index < lbl_1_bss_3688.count) {
        M660GridPoint *candidate = &lbl_1_bss_2688[index];

        if (candidate->flags == routeMode) {
            candidateCount++;
            delta.x = player->position.x - candidate->position.x;
            delta.z = player->position.z - candidate->position.z;
            distanceSquared = (delta.x * delta.x) + (delta.z * delta.z);
            distance = (f32)sqrt((f64)distanceSquared);
            if (lbl_1_bss_2DC[player->playerIndex ^ 1].type != 0 &&
                player->playerIndex == 1) {
                delta.x =
                    lbl_1_bss_2DC[player->playerIndex ^ 1].destination.x - candidate->position.x;
                delta.z =
                    lbl_1_bss_2DC[player->playerIndex ^ 1].destination.z - candidate->position.z;
                otherDistanceSquared = (delta.x * delta.x) +
                                       (delta.z * delta.z);
                otherDistance = (f32)sqrt((f64)otherDistanceSquared);
            } else {
                otherDistance = 9999.0f;
            }
            if (selectFarthest == 0) {
                if (bestDistance > distance && otherDistance > 150.0f &&
                    frandmod(player->routeRandomRange) == 0) {
                    found = 1;
                    bestDistance = distance;
                    selectedPosition = candidate->position;
                    selectedPoint = index;
                }
            } else if (bestDistance < distance && otherDistance > 100.0f &&
                       minimumDistance < distance && nearbyRadius > distance) {
                found = 1;
                bestDistance = distance;
                selectedPosition = candidate->position;
                selectedPoint = index;
            }
        }
        index++;
    }

    if (found != 0) {
        player->destination = selectedPosition;
        player->destinationPointIndex = selectedPoint;
        lbl_1_bss_2688[selectedPoint].flags |= 0x100;
    }
    return found;
}

/* Called by fn_1_13C6C; resets to center or attempts an off-center destination by assigning both
* calculated coordinates to destination.x. */
s16 fn_1_13810(
    M660Player *player, s16 useCenter)
{
    s16 success = 0;
    s16 attempt = 0;

    if (useCenter == 0) {
        for (;;) {
            f32 offset[3];
            f32 angle = fn_1_3C54(360.0f);
            f32 radius = fn_1_3C54(300.0f);

            offset[0] = (f32)((f64)player->position.x -
                              ((f64)radius *
                               sin((3.141592653589793 * angle) /
                                   180.0)));
            offset[2] = (f32)((f64)player->position.z -
                              ((f64)radius *
                               cos((3.141592653589793 * angle) /
                                   180.0)));
            radius = (f32)sqrt((offset[0] * offset[0]) +
                               (offset[2] * offset[2]));
            if (radius > 200.0f) {
                player->destination.x =
                    (f32)((f64)radius *
                          sin((3.141592653589793 * angle) /
                              180.0));
                player->destination.y = 0.0f;
                player->destination.x =
                    (f32)((f64)radius *
                          cos((3.141592653589793 * angle) /
                              180.0));
                success = 1;
                break;
            }
            if (++attempt > 100) {
                break;
            }
        }
    } else {
        player->destination.x = 0.0f;
        player->destination.y = 0.0f;
        player->destination.z = 0.0f;
        success = 1;
    }

    return success;
}

/* Checks whether an active stage slot lies within the central board area for fn_1_13C6C. */
s16 fn_1_13B64(void)
{
    M660Slot *player;
    s16 found = 0;
    s16 index = 0;
    M660Model *object;

    while (index < 9) {
        player = &lbl_1_bss_80[index];
        object = &lbl_1_bss_60C[index + 45];
        if (player->active != 0 &&
            object->position.x < 500.0f + object->bounds.x &&
            object->position.x > -500.0f - object->bounds.x &&
            object->position.z < 500.0f + object->bounds.x &&
            object->position.z > -500.0f - object->bounds.x) {
            found = 1;
        }
        index++;
    }

    return found;
}

/* Chooses the next destination and advances a player's route transition from fn_1_11F50. */
void fn_1_13C6C(M660Player *player)
{
    s16 avoidOccupied;
    s16 roll;
    s16 success = 0;

    if (player->external != 0) {
        player->arrived = 1;
    }
    player->routeLifetime--;

    switch (player->transitionState) {
    case 0:
    {
        if (lbl_1_bss_1F4.count < 100 && player->routeLifetime > 0) {
            roll = frandmod(100);
            success = roll > player->routeChance ? 0 : fn_1_13B64();
            success = success == 0 ? fn_1_13810(player, player->routeActive) : 0;
            if (success != 0 && player->routeActive != 0) {
                player->routeActive = 0;
            }
            player->reached = 0;
        } else {
            s16 useCenter = (player->routeLifetime < 0) ? 1 : 0;
            avoidOccupied = player->external != 0 ? 0 : 1;
            player->reached = player->external != 0 ? 1 : 0;
            success = fn_1_131C8(player, avoidOccupied, useCenter);
            if (useCenter != 0) {
                player->reached = 1;
            }
            if (player->external != 0) {
                player->routeActive = 1;
                player->external = 0;
            }
        }
        if (success != 0) {
            player->arrived = 0;
            player->transitionState++;
        }
        break;
    }
    case 1:
    {
        player->forwardSpeed = 10.0f;
        player->arrivalRadius = player->reached != 0 ? 10.1f : 200.0f;
        success = fn_1_12CA8(player);
        if (success != 0) {
            player->transitionState = player->arrived != 0 ? 0 : 2;
            return;
        }
        break;
    }
    case 2:
    {
        if (player->movementCounter++ > 30 || player->arrived != 0) {
            player->movementCounter = 0;
            player->transitionState = 0;
        }
        break;
    }
    default:
        break;
    }
}

/* Marks grid points covered by static obstacle bounds for group 3 or by moving-object path
 * corridors for groups 0-2, during setup and effect changes. */
void fn_1_142F8(s16 group)
{
    Vec corners[4];
    Vec normals[4];
    Vec edges[4];
    Vec minimum;
    Vec maximum;
    Vec projected;
    Vec point;
    Vec difference;
    f32 angles[4];
    s16 edgeIndices[4][2] = {{0, 3}, {1, 2}, {0, 1}, {2, 3}};
    s16 opposite[4] = {1, 3, 2, 0};
    s16 ranges[4][2] = {{45, 49}, {50, 52}, {53, 53}, {31, 44}};
    s16 edgeIndex;
    s16 pointIndex;
    s16 objectIndex;
    s16 inside;
    s32 flag;
    f32 angle;
    f32 direction;
    f32 radius;
    f32 edgeSquared;
    f32 numerator;
    f32 factor;
    f32 distanceSquared;
    f32 secondTurn;
    f32 angleDifference;
    f32 distance;
    M660Model *object;
    M660Slot *slot;
    M660GridPoint *pathPoint;

    for (objectIndex = ranges[group][0]; objectIndex <= ranges[group][1]; objectIndex++) {
        object = &lbl_1_bss_60C[objectIndex];
        if (group == 3) {
            corners[0].x = 50.0f + (object->position.x + object->bounds.x);
            corners[0].z = 50.0f + (object->position.z + object->bounds.z);
            corners[1].x = 50.0f + (object->position.x + object->bounds.x);
            corners[1].z = (object->position.z - object->bounds.z) - 50.0f;
            corners[2].x = (object->position.x - object->bounds.x) - 50.0f;
            corners[2].z = (object->position.z - object->bounds.z) - 50.0f;
            corners[3].x = (object->position.x - object->bounds.x) - 50.0f;
            corners[3].z = 50.0f + (object->position.z + object->bounds.z);
        } else {
            slot = &lbl_1_bss_80[objectIndex - 45];
            angle = lbl_1_data_100[slot->pathIndex].angles[0];
            radius = 50.0f + object->bounds.x;
            corners[0].x = lbl_1_data_100[slot->pathIndex].points[0].x +
                radius * sin((3.141592653589793 * (90.0f + angle)) / 180.0);
            corners[0].z = lbl_1_data_100[slot->pathIndex].points[0].z +
                radius * cos((3.141592653589793 * (90.0f + angle)) / 180.0);
            corners[1].x = lbl_1_data_100[slot->pathIndex].points[1].x +
                radius * sin((3.141592653589793 * (90.0f + angle)) / 180.0);
            corners[1].z = lbl_1_data_100[slot->pathIndex].points[1].z +
                radius * cos((3.141592653589793 * (90.0f + angle)) / 180.0);
            corners[2].x = lbl_1_data_100[slot->pathIndex].points[1].x +
                radius * sin((3.141592653589793 * (angle - 90.0f)) / 180.0);
            corners[2].z = lbl_1_data_100[slot->pathIndex].points[1].z +
                radius * cos((3.141592653589793 * (angle - 90.0f)) / 180.0);
            corners[3].x = lbl_1_data_100[slot->pathIndex].points[0].x +
                radius * sin((3.141592653589793 * (angle - 90.0f)) / 180.0);
            corners[3].z = lbl_1_data_100[slot->pathIndex].points[0].z +
                radius * cos((3.141592653589793 * (angle - 90.0f)) / 180.0);
        }
        for (edgeIndex = 0; edgeIndex < 4; edgeIndex++) {
            edges[edgeIndex].x = corners[edgeIndices[edgeIndex][0]].x -
                                 corners[edgeIndices[edgeIndex][1]].x;
            edges[edgeIndex].y = 0.0f;
            edges[edgeIndex].z = corners[edgeIndices[edgeIndex][0]].z -
                                 corners[edgeIndices[edgeIndex][1]].z;
            edgeSquared = edges[edgeIndex].x * edges[edgeIndex].x +
                          edges[edgeIndex].z * edges[edgeIndex].z;
            numerator = -1.0f * (edges[edgeIndex].x *
                (corners[edgeIndices[edgeIndex][0]].x - corners[opposite[edgeIndex]].x) +
                edges[edgeIndex].z *
                (corners[edgeIndices[edgeIndex][0]].z - corners[opposite[edgeIndex]].z));
            factor = edgeSquared ? numerator / edgeSquared : 0.0f;
            projected.x = factor * edges[edgeIndex].x + corners[edgeIndices[edgeIndex][0]].x;
            projected.z = factor * edges[edgeIndex].z + corners[edgeIndices[edgeIndex][0]].z;
            normals[edgeIndex].x = corners[opposite[edgeIndex]].x - projected.x;
            normals[edgeIndex].y = 0.0f;
            normals[edgeIndex].z = corners[opposite[edgeIndex]].z - projected.z;
            PSVECNormalize(&normals[edgeIndex], &normals[edgeIndex]);
            angles[edgeIndex] = 180.0 * (atan2(normals[edgeIndex].x,
                normals[edgeIndex].z) / 3.141592653589793);
        }
        maximum = corners[0];
        minimum = corners[0];
        for (pointIndex = 1; pointIndex < 4; pointIndex++) {
            if (maximum.x < corners[pointIndex].x) maximum.x = corners[pointIndex].x;
            if (maximum.z < corners[pointIndex].z) maximum.z = corners[pointIndex].z;
            if (minimum.x > corners[pointIndex].x) minimum.x = corners[pointIndex].x;
            if (minimum.z > corners[pointIndex].z) minimum.z = corners[pointIndex].z;
        }
        maximum.x += lbl_1_bss_3688.radius;
        maximum.z += lbl_1_bss_3688.radius;
        minimum.x -= lbl_1_bss_3688.radius;
        minimum.z -= lbl_1_bss_3688.radius;
        for (pointIndex = 0; pointIndex < lbl_1_bss_3688.count; pointIndex++) {
            pathPoint = &lbl_1_bss_2688[pointIndex];
            point = pathPoint->position;
            if (maximum.x > pathPoint->position.x && minimum.x < pathPoint->position.x &&
                maximum.z > pathPoint->position.z && minimum.z < pathPoint->position.z) {
                edgeIndex = 0;
                inside = 1;
                for (; edgeIndex < 4; edgeIndex++) {
                    edgeSquared = edges[edgeIndex].x * edges[edgeIndex].x +
                                  edges[edgeIndex].z * edges[edgeIndex].z;
                    numerator = -1.0f * (edges[edgeIndex].x *
                        (corners[edgeIndices[edgeIndex][0]].x - point.x) +
                        edges[edgeIndex].z * (corners[edgeIndices[edgeIndex][0]].z - point.z));
                    factor = edgeSquared ? numerator / edgeSquared : 0.0f;
                    projected.x =
                        factor * edges[edgeIndex].x + corners[edgeIndices[edgeIndex][0]].x;
                    projected.z =
                        factor * edges[edgeIndex].z + corners[edgeIndices[edgeIndex][0]].z;
                    difference.x = point.x - projected.x;
                    difference.y = 0.0f;
                    difference.z = point.z - projected.z;
                    distanceSquared = difference.x * difference.x + difference.z * difference.z;
                    distance = sqrt(distanceSquared);
                    if (distance < lbl_1_bss_3688.radius) break;
                    PSVECNormalize(&difference, &difference);
                    direction = 180.0 * (atan2(difference.x, difference.z) / 3.141592653589793);
                    if ((angles[edgeIndex] > 90.0f && direction < -90.0f) ||
                        (angles[edgeIndex] < -90.0f && direction > 90.0f)) {
                        angle = 0.0f > angles[edgeIndex] ?
                            180.0f + angles[edgeIndex] : 180.0f - angles[edgeIndex];
                        secondTurn = 0.0f > direction ? 180.0f + direction : 180.0f - direction;
                        angleDifference = angle + secondTurn > 0.0f ?
                            angle + secondTurn : -1.0f * (angle + secondTurn);
                    } else {
                        angleDifference = angles[edgeIndex] - direction > 0.0f ?
                            angles[edgeIndex] - direction : -1.0f * (angles[edgeIndex] - direction);
                    }
                    if (angleDifference > 1.0f) {
                        inside = 0;
                        break;
                    }
                }
                if (inside == 1) {
                    if (group == 3) flag = 1;
                    else flag = 1 << 4;
                    pathPoint->flags |= flag;
                }
            }
        }
    }
}

/* Retains grid flags for the selected route effect before fn_1_142F8 marks its bounds. */
void fn_1_15484(s16 selection)
{
    s16 *entry;
    s16 index;
    s16 mask;

    switch (selection) {
    case 0:
    case 1:
    case 2:
        mask = (1 << 8) | 1;
        break;
    case 3:
        mask = (1 << 8) | (1 << 4);
        printf("!!!!!kiteru\n");
        break;
    case 4:
        mask = (1 << 4) | 1;
        break;
    }

    index = 0;
    while (index < lbl_1_bss_3688.count) {
        entry = &lbl_1_bss_2688[index].flags;
        *entry &= mask;
        index++;
    }
}

/* Prints nearby route-effect grid points; prints "nonhit" when none meet the distance test. */
void fn_1_15548(void)
{
    M660Player *players = lbl_1_bss_2DC;
    s16 index = 0;
    s16 foundObject = 0;
    M660GridPoint *object;
    struct {
        f32 x;
        f32 y;
        f32 z;
    } delta;
    f32 distance;

    while (index < lbl_1_bss_3688.count) {
        object = &lbl_1_bss_2688[index];
        delta.x = players[0].position.x - object->position.x;
        delta.z = players[0].position.z - object->position.z;
        distance = sqrt((f64)((delta.x * delta.x) + (delta.z * delta.z)));

        if ((distance <= (50.0f + lbl_1_bss_3688.radius)) &&
            ((object->flags & 0x10) != 0)) {
            foundObject = 1;
            printf("%d %d\n", (s16)object->position.x, (s16)object->position.z);
        }
        index++;
    }

    if (foundObject == 0) {
        printf("nonhit\n");
    }
}

M660Route lbl_1_bss_36B8[14];
M660Grid lbl_1_bss_3688;
M660GridPoint lbl_1_bss_2688[256];
