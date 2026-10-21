/* Light Up My Night participant selection and movement routines. */
#include "dolphin/math.h"

#include "game/main.h"

#include "REL/m617dll/shared.h"

#include "game/hu3d.h"

#include "game/wipe.h"

#include "game/gamemes.h"

#include "game/sprite.h"

#include "game/mg/timer.h"

#include "game/mg/actman.h"

#include "game/frand.h"

void fn_1_1817C(M617PlayerStateData *work);

typedef struct M617E1ConstantBlock {
    Vec directions[8];
} M617E1ConstantBlock;

extern const M617E1ConstantBlock lbl_1_rodata_CE8;

extern const u16 lbl_1_rodata_D48[4];

extern const u16 lbl_1_rodata_D50[4];

extern const u16 lbl_1_rodata_D58[4];

extern const u16 lbl_1_rodata_D60[4];

extern const u16 lbl_1_rodata_D68[4];

extern const s32 lbl_1_rodata_D70[12];

extern const s16 lbl_1_rodata_DA0[8];

s32 fn_1_24D8(void);

s16 fn_1_5C1C(void);

s32 fn_1_D6B0(void);

s32 fn_1_D6D4(void);

typedef struct M617D6HeaderConstants {
    s32 word[9];
} M617D6HeaderConstants;

extern const M617D6HeaderConstants lbl_1_rodata_C88;

extern f32 CZoom;

extern void fn_1_307C(MGTIMER *timer);

extern s32 fn_1_3CD0(s16 modelId, s16 lightId);

extern s16 fn_1_3BB8(s16 modelId, s16 lightId);

s32 fn_1_D6F8(void);

/* Called by fn_1_93D0 during setup; initializes difficulty tuning and stage routes. */
void fn_1_E1F0(void)
{
    Vec directionVectors[8] = {
        {0.0f, 0.0f, 1.0f}, {45.0f, 1.0f, 1.0f},
        {90.0f, 1.0f, 0.0f}, {135.0f, 1.0f, -1.0f},
        {180.0f, 0.0f, -1.0f}, {225.0f, -1.0f, -1.0f},
        {270.0f, -1.0f, 0.0f}, {315.0f, -1.0f, 1.0f}
    };
    s16 selectionThresholdByDifficulty[4] = {70, 60, 50, 20};
    s16 selectionDurationByDifficulty[4] = {30, 25, 20, 10};
    s16 retryLimitByDifficulty[4] = {10, 10, 10, 10};
    s16 targetChanceByDifficulty[4] = {70, 65, 60, 50};
    s16 chaseSpeedChanceByDifficulty[4] = {45, 50, 55, 70};
    M617RouteEntry savedEntry;
    Vec offsets[4] = {
        {150.0f, 100.0f, 50.0f}, {210.0f, 140.0f, 70.0f},
        {240.0f, 160.0f, 80.0f}, {300.0f, 200.0f, 100.0f}
    };
    Vec position;
    Vec facing;
    s16 durationRanges[4][2] = {{45, 5}, {35, 5}, {1, 1}, {1, 1}};
    s16 recordIndex;
    s16 entryIndex;
    s16 pointIndex;
    s16 directionIndex;
    s16 difficulty;
    f32 angle;
    f32 distance;

    for (recordIndex = 0; recordIndex < 2; recordIndex++) {
        if (lbl_1_bss_AB0[recordIndex].computerControlled != 0) {
            lbl_1_bss_AB0[recordIndex].selectionThreshold =
                selectionThresholdByDifficulty[difficulty =
                    lbl_1_bss_AB0[recordIndex].difficulty];
            lbl_1_bss_AB0[recordIndex].selectionDuration =
                selectionDurationByDifficulty[difficulty];
            lbl_1_bss_AB0[recordIndex].retryLimit = retryLimitByDifficulty[difficulty];
            lbl_1_bss_AB0[recordIndex].targetSelectionChance =
                targetChanceByDifficulty[difficulty];
            lbl_1_bss_AB0[recordIndex].chaseSpeedChance =
                chaseSpeedChanceByDifficulty[difficulty];
            lbl_1_bss_AB0[recordIndex].rangeLimits[0] = offsets[difficulty].x;
            lbl_1_bss_AB0[recordIndex].rangeLimits[1] = offsets[difficulty].y;
            lbl_1_bss_AB0[recordIndex].rangeLimits[2] = offsets[difficulty].z;
            lbl_1_bss_AB0[recordIndex].selectionDelayFrames =
                (durationRanges[difficulty][0] +
                 frandmod(durationRanges[difficulty][1])) * 60;
        }
    }

    memset(lbl_1_bss_38F0, 0, 5088U);
    for (recordIndex = 0; recordIndex < 24; recordIndex++) {
        for (entryIndex = 0; entryIndex < 4; entryIndex++) {
            lbl_1_bss_38F0[recordIndex].entries[entryIndex].state = -1;
            lbl_1_bss_38F0[recordIndex].entries[entryIndex].pointCount = 1;
        }
    }
    if (lbl_1_bss_9C8.layout == 0) {
        lbl_1_bss_38F0[0].entries[1].state = 1;
        lbl_1_bss_38F0[0].entries[1].pointCount = 2;
        lbl_1_bss_38F0[0].entries[1].directions[0].x = 1.0f;
        lbl_1_bss_38F0[0].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[0].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[0].entries[1].positions[0].x = -300.0f;
        lbl_1_bss_38F0[0].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[0].entries[1].positions[0].z = -620.0f;
        lbl_1_bss_38F0[0].entries[1].directions[1].x = -1.0f;
        lbl_1_bss_38F0[0].entries[1].directions[1].y = 0.0f;
        lbl_1_bss_38F0[0].entries[1].directions[1].z = 1.0f;
        lbl_1_bss_38F0[0].entries[1].positions[1].x = -557.0f;
        lbl_1_bss_38F0[0].entries[1].positions[1].y = 0.0f;
        lbl_1_bss_38F0[0].entries[1].positions[1].z = -620.0f;
        lbl_1_bss_38F0[0].entries[2].state = 2;
        lbl_1_bss_38F0[0].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[0].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[0].entries[2].directions[0].z = 0.0f;
        lbl_1_bss_38F0[0].entries[2].positions[0].x = -370.0f;
        lbl_1_bss_38F0[0].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[0].entries[2].positions[0].z = -44.0f;
        lbl_1_bss_38F0[0].entries[3].state = 3;
        lbl_1_bss_38F0[0].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[0].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[0].entries[3].directions[0].z = 0.0f;
        lbl_1_bss_38F0[0].entries[3].positions[0].x = -470.0f;
        lbl_1_bss_38F0[0].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[0].entries[3].positions[0].z = -745.0f;
        lbl_1_bss_38F0[1].entries[0].state = 0;
        lbl_1_bss_38F0[1].entries[0].pointCount = 2;
        lbl_1_bss_38F0[1].entries[0].directions[0].x = 1.0f;
        lbl_1_bss_38F0[1].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[1].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[1].entries[0].positions[0].x = -317.0f;
        lbl_1_bss_38F0[1].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[1].entries[0].positions[0].z = -170.0f;
        lbl_1_bss_38F0[1].entries[0].directions[1].x = -1.0f;
        lbl_1_bss_38F0[1].entries[0].directions[1].y = 0.0f;
        lbl_1_bss_38F0[1].entries[0].directions[1].z = -1.0f;
        lbl_1_bss_38F0[1].entries[0].positions[1].x = -560.0f;
        lbl_1_bss_38F0[1].entries[0].positions[1].y = 0.0f;
        lbl_1_bss_38F0[1].entries[0].positions[1].z = -170.0f;
        lbl_1_bss_38F0[1].entries[2].state = 2;
        lbl_1_bss_38F0[1].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[1].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[1].entries[2].directions[0].z = 0.0f;
        lbl_1_bss_38F0[1].entries[2].positions[0].x = -420.0f;
        lbl_1_bss_38F0[1].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[1].entries[2].positions[0].z = -44.0f;
        lbl_1_bss_38F0[1].entries[3].state = 3;
        lbl_1_bss_38F0[1].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[1].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[1].entries[3].directions[0].z = 0.0f;
        lbl_1_bss_38F0[1].entries[3].positions[0].x = -450.0f;
        lbl_1_bss_38F0[1].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[1].entries[3].positions[0].z = -42.0f;
        lbl_1_bss_38F0[2].entries[0].state = 0;
        lbl_1_bss_38F0[2].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[2].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[2].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[2].entries[0].positions[0].x = -700.0f;
        lbl_1_bss_38F0[2].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[2].entries[0].positions[0].z = -370.0f;
        lbl_1_bss_38F0[2].entries[1].state = 1;
        lbl_1_bss_38F0[2].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[2].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[2].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[2].entries[1].positions[0].x = -700.0f;
        lbl_1_bss_38F0[2].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[2].entries[1].positions[0].z = -420.0f;
        lbl_1_bss_38F0[2].entries[2].state = 2;
        lbl_1_bss_38F0[2].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[2].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[2].entries[2].directions[0].z = 0.0f;
        lbl_1_bss_38F0[2].entries[2].positions[0].x = -370.0f;
        lbl_1_bss_38F0[2].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[2].entries[2].positions[0].z = -44.0f;
        lbl_1_bss_38F0[2].entries[3].state = 3;
        lbl_1_bss_38F0[2].entries[3].pointCount = 2;
        lbl_1_bss_38F0[2].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[2].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[2].entries[3].directions[0].z = -1.0f;
        lbl_1_bss_38F0[2].entries[3].positions[0].x = -610.0f;
        lbl_1_bss_38F0[2].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[2].entries[3].positions[0].z = -550.0f;
        lbl_1_bss_38F0[2].entries[3].directions[1].x = 1.0f;
        lbl_1_bss_38F0[2].entries[3].directions[1].y = 0.0f;
        lbl_1_bss_38F0[2].entries[3].directions[1].z = 1.0f;
        lbl_1_bss_38F0[2].entries[3].positions[1].x = -610.0f;
        lbl_1_bss_38F0[2].entries[3].positions[1].y = 0.0f;
        lbl_1_bss_38F0[2].entries[3].positions[1].z = -256.0f;
        lbl_1_bss_38F0[3].entries[0].state = 0;
        lbl_1_bss_38F0[3].entries[0].pointCount = 2;
        lbl_1_bss_38F0[3].entries[0].directions[0].x = 1.0f;
        lbl_1_bss_38F0[3].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[3].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[3].entries[0].positions[0].x = -15.0f;
        lbl_1_bss_38F0[3].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[3].entries[0].positions[0].z = -650.0f;
        lbl_1_bss_38F0[3].entries[0].directions[1].x = -1.0f;
        lbl_1_bss_38F0[3].entries[0].directions[1].y = 0.0f;
        lbl_1_bss_38F0[3].entries[0].directions[1].z = -1.0f;
        lbl_1_bss_38F0[3].entries[0].positions[1].x = -267.0f;
        lbl_1_bss_38F0[3].entries[0].positions[1].y = 0.0f;
        lbl_1_bss_38F0[3].entries[0].positions[1].z = -650.0f;
        lbl_1_bss_38F0[3].entries[2].state = 2;
        lbl_1_bss_38F0[3].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[3].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[3].entries[2].directions[0].z = 1.0f;
        lbl_1_bss_38F0[3].entries[2].positions[0].x = -120.0f;
        lbl_1_bss_38F0[3].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[3].entries[2].positions[0].z = -550.0f;
        lbl_1_bss_38F0[3].entries[3].state = 3;
        lbl_1_bss_38F0[3].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[3].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[3].entries[3].directions[0].z = 0.0f;
        lbl_1_bss_38F0[3].entries[3].positions[0].x = -180.0f;
        lbl_1_bss_38F0[3].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[3].entries[3].positions[0].z = -550.0f;
        lbl_1_bss_38F0[4].entries[0].state = 0;
        lbl_1_bss_38F0[4].entries[0].pointCount = 2;
        lbl_1_bss_38F0[4].entries[0].directions[0].x = 1.0f;
        lbl_1_bss_38F0[4].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[4].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[4].entries[0].positions[0].x = 635.0f;
        lbl_1_bss_38F0[4].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[4].entries[0].positions[0].z = -430.0f;
        lbl_1_bss_38F0[4].entries[0].directions[1].x = -1.0f;
        lbl_1_bss_38F0[4].entries[0].directions[1].y = 0.0f;
        lbl_1_bss_38F0[4].entries[0].directions[1].z = -1.0f;
        lbl_1_bss_38F0[4].entries[0].positions[1].x = 162.0f;
        lbl_1_bss_38F0[4].entries[0].positions[1].y = 0.0f;
        lbl_1_bss_38F0[4].entries[0].positions[1].z = -430.0f;
        lbl_1_bss_38F0[4].entries[1].state = 1;
        lbl_1_bss_38F0[4].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[4].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[4].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[4].entries[1].positions[0].x = 156.0f;
        lbl_1_bss_38F0[4].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[4].entries[1].positions[0].z = -480.0f;
        lbl_1_bss_38F0[4].entries[3].state = 3;
        lbl_1_bss_38F0[4].entries[3].pointCount = 2;
        lbl_1_bss_38F0[4].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[4].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[4].entries[3].directions[0].z = -1.0f;
        lbl_1_bss_38F0[4].entries[3].positions[0].x = 260.0f;
        lbl_1_bss_38F0[4].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[4].entries[3].positions[0].z = -600.0f;
        lbl_1_bss_38F0[4].entries[3].directions[1].x = 1.0f;
        lbl_1_bss_38F0[4].entries[3].directions[1].y = 0.0f;
        lbl_1_bss_38F0[4].entries[3].directions[1].z = 1.0f;
        lbl_1_bss_38F0[4].entries[3].positions[1].x = 260.0f;
        lbl_1_bss_38F0[4].entries[3].positions[1].y = 0.0f;
        lbl_1_bss_38F0[4].entries[3].positions[1].z = -303.0f;
        lbl_1_bss_38F0[5].entries[0].state = 0;
        lbl_1_bss_38F0[5].entries[0].pointCount = 2;
        lbl_1_bss_38F0[5].entries[0].directions[0].x = 1.0f;
        lbl_1_bss_38F0[5].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[5].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[5].entries[0].positions[0].x = 635.0f;
        lbl_1_bss_38F0[5].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[5].entries[0].positions[0].z = -430.0f;
        lbl_1_bss_38F0[5].entries[0].directions[1].x = -1.0f;
        lbl_1_bss_38F0[5].entries[0].directions[1].y = 0.0f;
        lbl_1_bss_38F0[5].entries[0].directions[1].z = -1.0f;
        lbl_1_bss_38F0[5].entries[0].positions[1].x = 162.0f;
        lbl_1_bss_38F0[5].entries[0].positions[1].y = 0.0f;
        lbl_1_bss_38F0[5].entries[0].positions[1].z = -430.0f;
        lbl_1_bss_38F0[5].entries[1].state = 1;
        lbl_1_bss_38F0[5].entries[1].pointCount = 2;
        lbl_1_bss_38F0[5].entries[1].directions[0].x = 1.0f;
        lbl_1_bss_38F0[5].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[5].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[5].entries[1].positions[0].x = 624.0f;
        lbl_1_bss_38F0[5].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[5].entries[1].positions[0].z = -635.0f;
        lbl_1_bss_38F0[5].entries[1].directions[1].x = -1.0f;
        lbl_1_bss_38F0[5].entries[1].directions[1].y = 0.0f;
        lbl_1_bss_38F0[5].entries[1].directions[1].z = 1.0f;
        lbl_1_bss_38F0[5].entries[1].positions[1].x = 400.0f;
        lbl_1_bss_38F0[5].entries[1].positions[1].y = 0.0f;
        lbl_1_bss_38F0[5].entries[1].positions[1].z = -635.0f;
        lbl_1_bss_38F0[5].entries[2].state = 2;
        lbl_1_bss_38F0[5].entries[2].pointCount = 2;
        lbl_1_bss_38F0[5].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[5].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[5].entries[2].directions[0].z = 1.0f;
        lbl_1_bss_38F0[5].entries[2].positions[0].x = 500.0f;
        lbl_1_bss_38F0[5].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[5].entries[2].positions[0].z = -338.0f;
        lbl_1_bss_38F0[5].entries[2].directions[1].x = -1.0f;
        lbl_1_bss_38F0[5].entries[2].directions[1].y = 0.0f;
        lbl_1_bss_38F0[5].entries[2].directions[1].z = -1.0f;
        lbl_1_bss_38F0[5].entries[2].positions[1].x = 500.0f;
        lbl_1_bss_38F0[5].entries[2].positions[1].y = 0.0f;
        lbl_1_bss_38F0[5].entries[2].positions[1].z = -770.0f;
        lbl_1_bss_38F0[5].entries[3].state = 3;
        lbl_1_bss_38F0[5].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[5].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[5].entries[3].directions[0].z = 0.0f;
        lbl_1_bss_38F0[5].entries[3].positions[0].x = 489.0f;
        lbl_1_bss_38F0[5].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[5].entries[3].positions[0].z = -770.0f;
        lbl_1_bss_38F0[6].entries[0].state = 0;
        lbl_1_bss_38F0[6].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[6].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[6].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[6].entries[0].positions[0].x = -222.0f;
        lbl_1_bss_38F0[6].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[6].entries[0].positions[0].z = 320.0f;
        lbl_1_bss_38F0[6].entries[1].state = 1;
        lbl_1_bss_38F0[6].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[6].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[6].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[6].entries[1].positions[0].x = -412.0f;
        lbl_1_bss_38F0[6].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[6].entries[1].positions[0].z = -47.0f;
        lbl_1_bss_38F0[7].entries[0].state = 0;
        lbl_1_bss_38F0[7].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[7].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[7].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[7].entries[0].positions[0].x = -255.0f;
        lbl_1_bss_38F0[7].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[7].entries[0].positions[0].z = 320.0f;
        lbl_1_bss_38F0[7].entries[1].state = 1;
        lbl_1_bss_38F0[7].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[7].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[7].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[7].entries[1].positions[0].x = -412.0f;
        lbl_1_bss_38F0[7].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[7].entries[1].positions[0].z = -47.0f;
        lbl_1_bss_38F0[8].entries[0].state = 0;
        lbl_1_bss_38F0[8].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[8].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[8].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[8].entries[0].positions[0].x = -270.0f;
        lbl_1_bss_38F0[8].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[8].entries[0].positions[0].z = 280.0f;
        lbl_1_bss_38F0[8].entries[1].state = 1;
        lbl_1_bss_38F0[8].entries[1].pointCount = 2;
        lbl_1_bss_38F0[8].entries[1].directions[0].x = 1.0f;
        lbl_1_bss_38F0[8].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[8].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[8].entries[1].positions[0].x = -292.0f;
        lbl_1_bss_38F0[8].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[8].entries[1].positions[0].z = 120.0f;
        lbl_1_bss_38F0[8].entries[1].directions[1].x = -1.0f;
        lbl_1_bss_38F0[8].entries[1].directions[1].y = 0.0f;
        lbl_1_bss_38F0[8].entries[1].directions[1].z = 1.0f;
        lbl_1_bss_38F0[8].entries[1].positions[1].x = -560.0f;
        lbl_1_bss_38F0[8].entries[1].positions[1].y = 0.0f;
        lbl_1_bss_38F0[8].entries[1].positions[1].z = 120.0f;
        lbl_1_bss_38F0[8].entries[2].state = 2;
        lbl_1_bss_38F0[8].entries[2].pointCount = 2;
        lbl_1_bss_38F0[8].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[8].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[8].entries[2].directions[0].z = -1.0f;
        lbl_1_bss_38F0[8].entries[2].positions[0].x = -400.0f;
        lbl_1_bss_38F0[8].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[8].entries[2].positions[0].z = 5.0f;
        lbl_1_bss_38F0[8].entries[2].directions[1].x = -1.0f;
        lbl_1_bss_38F0[8].entries[2].directions[1].y = 0.0f;
        lbl_1_bss_38F0[8].entries[2].directions[1].z = 1.0f;
        lbl_1_bss_38F0[8].entries[2].positions[1].x = -400.0f;
        lbl_1_bss_38F0[8].entries[2].positions[1].y = 0.0f;
        lbl_1_bss_38F0[8].entries[2].positions[1].z = 419.0f;
        lbl_1_bss_38F0[8].entries[3].state = 3;
        lbl_1_bss_38F0[8].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[8].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[8].entries[3].directions[0].z = 0.0f;
        lbl_1_bss_38F0[8].entries[3].positions[0].x = -437.0f;
        lbl_1_bss_38F0[8].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[8].entries[3].positions[0].z = -66.0f;
        lbl_1_bss_38F0[9].entries[0].state = 0;
        lbl_1_bss_38F0[9].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[9].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[9].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[9].entries[0].positions[0].x = -315.0f;
        lbl_1_bss_38F0[9].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[9].entries[0].positions[0].z = -90.0f;
        lbl_1_bss_38F0[9].entries[1].state = 1;
        lbl_1_bss_38F0[9].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[9].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[9].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[9].entries[1].positions[0].x = -303.0f;
        lbl_1_bss_38F0[9].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[9].entries[1].positions[0].z = -140.0f;
        lbl_1_bss_38F0[9].entries[3].state = 3;
        lbl_1_bss_38F0[9].entries[3].pointCount = 2;
        lbl_1_bss_38F0[9].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[9].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[9].entries[3].directions[0].z = 1.0f;
        lbl_1_bss_38F0[9].entries[3].positions[0].x = -170.0f;
        lbl_1_bss_38F0[9].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[9].entries[3].positions[0].z = -32.0f;
        lbl_1_bss_38F0[9].entries[3].directions[1].x = 1.0f;
        lbl_1_bss_38F0[9].entries[3].directions[1].y = 0.0f;
        lbl_1_bss_38F0[9].entries[3].directions[1].z = -1.0f;
        lbl_1_bss_38F0[9].entries[3].positions[1].x = -170.0f;
        lbl_1_bss_38F0[9].entries[3].positions[1].y = 0.0f;
        lbl_1_bss_38F0[9].entries[3].positions[1].z = -225.0f;
        lbl_1_bss_38F0[10].entries[0].state = 0;
        lbl_1_bss_38F0[10].entries[0].pointCount = 2;
        lbl_1_bss_38F0[10].entries[0].directions[0].x = 1.0f;
        lbl_1_bss_38F0[10].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[10].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[10].entries[0].positions[0].x = 413.0f;
        lbl_1_bss_38F0[10].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[10].entries[0].positions[0].z = -90.0f;
        lbl_1_bss_38F0[10].entries[0].directions[1].x = -1.0f;
        lbl_1_bss_38F0[10].entries[0].directions[1].y = 0.0f;
        lbl_1_bss_38F0[10].entries[0].directions[1].z = -1.0f;
        lbl_1_bss_38F0[10].entries[0].positions[1].x = -274.0f;
        lbl_1_bss_38F0[10].entries[0].positions[1].y = 0.0f;
        lbl_1_bss_38F0[10].entries[0].positions[1].z = -90.0f;
        lbl_1_bss_38F0[10].entries[1].state = 1;
        lbl_1_bss_38F0[10].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[10].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[10].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[10].entries[1].positions[0].x = 410.0f;
        lbl_1_bss_38F0[10].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[10].entries[1].positions[0].z = -140.0f;
        lbl_1_bss_38F0[10].entries[2].state = 2;
        lbl_1_bss_38F0[10].entries[2].pointCount = 2;
        lbl_1_bss_38F0[10].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[10].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[10].entries[2].directions[0].z = 1.0f;
        lbl_1_bss_38F0[10].entries[2].positions[0].x = 280.0f;
        lbl_1_bss_38F0[10].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[10].entries[2].positions[0].z = 1.0f;
        lbl_1_bss_38F0[10].entries[2].directions[1].x = -1.0f;
        lbl_1_bss_38F0[10].entries[2].directions[1].y = 0.0f;
        lbl_1_bss_38F0[10].entries[2].directions[1].z = -1.0f;
        lbl_1_bss_38F0[10].entries[2].positions[1].x = 280.0f;
        lbl_1_bss_38F0[10].entries[2].positions[1].y = 0.0f;
        lbl_1_bss_38F0[10].entries[2].positions[1].z = -246.0f;
        lbl_1_bss_38F0[11].entries[0].state = 0;
        lbl_1_bss_38F0[11].entries[0].pointCount = 2;
        lbl_1_bss_38F0[11].entries[0].directions[0].x = 1.0f;
        lbl_1_bss_38F0[11].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[11].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[11].entries[0].positions[0].x = 413.0f;
        lbl_1_bss_38F0[11].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[11].entries[0].positions[0].z = -90.0f;
        lbl_1_bss_38F0[11].entries[0].directions[1].x = -1.0f;
        lbl_1_bss_38F0[11].entries[0].directions[1].y = 0.0f;
        lbl_1_bss_38F0[11].entries[0].directions[1].z = -1.0f;
        lbl_1_bss_38F0[11].entries[0].positions[1].x = -274.0f;
        lbl_1_bss_38F0[11].entries[0].positions[1].y = 0.0f;
        lbl_1_bss_38F0[11].entries[0].positions[1].z = -90.0f;
        lbl_1_bss_38F0[11].entries[1].state = 1;
        lbl_1_bss_38F0[11].entries[1].pointCount = 2;
        lbl_1_bss_38F0[11].entries[1].directions[0].x = 1.0f;
        lbl_1_bss_38F0[11].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[11].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[11].entries[1].positions[0].x = 195.0f;
        lbl_1_bss_38F0[11].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[11].entries[1].positions[0].z = -300.0f;
        lbl_1_bss_38F0[11].entries[1].directions[1].x = -1.0f;
        lbl_1_bss_38F0[11].entries[1].directions[1].y = 0.0f;
        lbl_1_bss_38F0[11].entries[1].directions[1].z = 1.0f;
        lbl_1_bss_38F0[11].entries[1].positions[1].x = -44.0f;
        lbl_1_bss_38F0[11].entries[1].positions[1].y = 0.0f;
        lbl_1_bss_38F0[11].entries[1].positions[1].z = -300.0f;
        lbl_1_bss_38F0[11].entries[2].state = 2;
        lbl_1_bss_38F0[11].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[11].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[11].entries[2].directions[0].z = 0.0f;
        lbl_1_bss_38F0[11].entries[2].positions[0].x = 80.0f;
        lbl_1_bss_38F0[11].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[11].entries[2].positions[0].z = -444.0f;
        lbl_1_bss_38F0[11].entries[3].state = 3;
        lbl_1_bss_38F0[11].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[11].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[11].entries[3].directions[0].z = 0.0f;
        lbl_1_bss_38F0[11].entries[3].positions[0].x = 37.0f;
        lbl_1_bss_38F0[11].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[11].entries[3].positions[0].z = -459.0f;
        lbl_1_bss_38F0[12].entries[0].state = 0;
        lbl_1_bss_38F0[12].entries[0].pointCount = 2;
        lbl_1_bss_38F0[12].entries[0].directions[0].x = 1.0f;
        lbl_1_bss_38F0[12].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[12].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[12].entries[0].positions[0].x = 267.0f;
        lbl_1_bss_38F0[12].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[12].entries[0].positions[0].z = 350.0f;
        lbl_1_bss_38F0[12].entries[0].directions[1].x = -1.0f;
        lbl_1_bss_38F0[12].entries[0].directions[1].y = 0.0f;
        lbl_1_bss_38F0[12].entries[0].directions[1].z = -1.0f;
        lbl_1_bss_38F0[12].entries[0].positions[1].x = -215.0f;
        lbl_1_bss_38F0[12].entries[0].positions[1].y = 0.0f;
        lbl_1_bss_38F0[12].entries[0].positions[1].z = 390.0f;
        lbl_1_bss_38F0[12].entries[1].state = 1;
        lbl_1_bss_38F0[12].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[12].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[12].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[12].entries[1].positions[0].x = 274.0f;
        lbl_1_bss_38F0[12].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[12].entries[1].positions[0].z = 370.0f;
        lbl_1_bss_38F0[12].entries[2].state = 2;
        lbl_1_bss_38F0[12].entries[2].pointCount = 2;
        lbl_1_bss_38F0[12].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[12].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[12].entries[2].directions[0].z = 1.0f;
        lbl_1_bss_38F0[12].entries[2].positions[0].x = 140.0f;
        lbl_1_bss_38F0[12].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[12].entries[2].positions[0].z = 466.0f;
        lbl_1_bss_38F0[12].entries[2].directions[1].x = -1.0f;
        lbl_1_bss_38F0[12].entries[2].directions[1].y = 0.0f;
        lbl_1_bss_38F0[12].entries[2].directions[1].z = -1.0f;
        lbl_1_bss_38F0[12].entries[2].positions[1].x = 140.0f;
        lbl_1_bss_38F0[12].entries[2].positions[1].y = 0.0f;
        lbl_1_bss_38F0[12].entries[2].positions[1].z = 195.0f;
        lbl_1_bss_38F0[13].entries[0].state = 0;
        lbl_1_bss_38F0[13].entries[0].pointCount = 2;
        lbl_1_bss_38F0[13].entries[0].directions[0].x = 1.0f;
        lbl_1_bss_38F0[13].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[13].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[13].entries[0].positions[0].x = 267.0f;
        lbl_1_bss_38F0[13].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[13].entries[0].positions[0].z = 350.0f;
        lbl_1_bss_38F0[13].entries[0].directions[1].x = -1.0f;
        lbl_1_bss_38F0[13].entries[0].directions[1].y = 0.0f;
        lbl_1_bss_38F0[13].entries[0].directions[1].z = -1.0f;
        lbl_1_bss_38F0[13].entries[0].positions[1].x = -215.0f;
        lbl_1_bss_38F0[13].entries[0].positions[1].y = 0.0f;
        lbl_1_bss_38F0[13].entries[0].positions[1].z = 390.0f;
        lbl_1_bss_38F0[13].entries[1].state = 1;
        lbl_1_bss_38F0[13].entries[1].pointCount = 2;
        lbl_1_bss_38F0[13].entries[1].directions[0].x = 1.0f;
        lbl_1_bss_38F0[13].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[13].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[13].entries[1].positions[0].x = 32.0f;
        lbl_1_bss_38F0[13].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[13].entries[1].positions[0].z = 130.0f;
        lbl_1_bss_38F0[13].entries[1].directions[1].x = -1.0f;
        lbl_1_bss_38F0[13].entries[1].directions[1].y = 0.0f;
        lbl_1_bss_38F0[13].entries[1].directions[1].z = 1.0f;
        lbl_1_bss_38F0[13].entries[1].positions[1].x = -168.0f;
        lbl_1_bss_38F0[13].entries[1].positions[1].y = 0.0f;
        lbl_1_bss_38F0[13].entries[1].positions[1].z = 130.0f;
        lbl_1_bss_38F0[13].entries[2].state = 2;
        lbl_1_bss_38F0[13].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[13].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[13].entries[2].directions[0].z = 0.0f;
        lbl_1_bss_38F0[13].entries[2].positions[0].x = -60.0f;
        lbl_1_bss_38F0[13].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[13].entries[2].positions[0].z = -20.0f;
        lbl_1_bss_38F0[13].entries[3].state = 3;
        lbl_1_bss_38F0[13].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[13].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[13].entries[3].directions[0].z = 0.0f;
        lbl_1_bss_38F0[13].entries[3].positions[0].x = -110.0f;
        lbl_1_bss_38F0[13].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[13].entries[3].positions[0].z = -10.0f;
        lbl_1_bss_38F0[14].entries[0].state = 0;
        lbl_1_bss_38F0[14].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[14].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[14].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[14].entries[0].positions[0].x = 220.0f;
        lbl_1_bss_38F0[14].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[14].entries[0].positions[0].z = 365.0f;
        lbl_1_bss_38F0[14].entries[1].state = 1;
        lbl_1_bss_38F0[14].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[14].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[14].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[14].entries[1].positions[0].x = 468.0f;
        lbl_1_bss_38F0[14].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[14].entries[1].positions[0].z = 40.0f;
        lbl_1_bss_38F0[15].entries[1].state = 1;
        lbl_1_bss_38F0[15].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[15].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[15].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[15].entries[1].positions[0].x = 468.0f;
        lbl_1_bss_38F0[15].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[15].entries[1].positions[0].z = 40.0f;
        lbl_1_bss_38F0[15].entries[2].state = 2;
        lbl_1_bss_38F0[15].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[15].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[15].entries[2].directions[0].z = 0.0f;
        lbl_1_bss_38F0[15].entries[2].positions[0].x = 600.0f;
        lbl_1_bss_38F0[15].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[15].entries[2].positions[0].z = 417.0f;
        lbl_1_bss_38F0[15].entries[3].state = 3;
        lbl_1_bss_38F0[15].entries[3].pointCount = 2;
        lbl_1_bss_38F0[15].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[15].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[15].entries[3].directions[0].z = 1.0f;
        lbl_1_bss_38F0[15].entries[3].positions[0].x = 283.0f;
        lbl_1_bss_38F0[15].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[15].entries[3].positions[0].z = 480.0f;
        lbl_1_bss_38F0[15].entries[3].directions[1].x = 1.0f;
        lbl_1_bss_38F0[15].entries[3].directions[1].y = 0.0f;
        lbl_1_bss_38F0[15].entries[3].directions[1].z = -1.0f;
        lbl_1_bss_38F0[15].entries[3].positions[1].x = 560.0f;
        lbl_1_bss_38F0[15].entries[3].positions[1].y = 0.0f;
        lbl_1_bss_38F0[15].entries[3].positions[1].z = -150.0f;
        lbl_1_bss_38F0[16].entries[0].state = 0;
        lbl_1_bss_38F0[16].entries[0].pointCount = 2;
        lbl_1_bss_38F0[16].entries[0].directions[0].x = 1.0f;
        lbl_1_bss_38F0[16].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[16].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[16].entries[0].positions[0].x = 710.0f;
        lbl_1_bss_38F0[16].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[16].entries[0].positions[0].z = 290.0f;
        lbl_1_bss_38F0[16].entries[0].directions[1].x = -1.0f;
        lbl_1_bss_38F0[16].entries[0].directions[1].y = 0.0f;
        lbl_1_bss_38F0[16].entries[0].directions[1].z = -1.0f;
        lbl_1_bss_38F0[16].entries[0].positions[1].x = 305.0f;
        lbl_1_bss_38F0[16].entries[0].positions[1].y = 0.0f;
        lbl_1_bss_38F0[16].entries[0].positions[1].z = 290.0f;
        lbl_1_bss_38F0[16].entries[1].state = 1;
        lbl_1_bss_38F0[16].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[16].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[16].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[16].entries[1].positions[0].x = 291.0f;
        lbl_1_bss_38F0[16].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[16].entries[1].positions[0].z = 240.0f;
        lbl_1_bss_38F0[16].entries[2].state = 2;
        lbl_1_bss_38F0[16].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[16].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[16].entries[2].directions[0].z = 0.0f;
        lbl_1_bss_38F0[16].entries[2].positions[0].x = 600.0f;
        lbl_1_bss_38F0[16].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[16].entries[2].positions[0].z = 417.0f;
        lbl_1_bss_38F0[16].entries[3].state = 3;
        lbl_1_bss_38F0[16].entries[3].pointCount = 2;
        lbl_1_bss_38F0[16].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[16].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[16].entries[3].directions[0].z = 1.0f;
        lbl_1_bss_38F0[16].entries[3].positions[0].x = 410.0f;
        lbl_1_bss_38F0[16].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[16].entries[3].positions[0].z = 473.0f;
        lbl_1_bss_38F0[16].entries[3].directions[1].x = 1.0f;
        lbl_1_bss_38F0[16].entries[3].directions[1].y = 0.0f;
        lbl_1_bss_38F0[16].entries[3].directions[1].z = -1.0f;
        lbl_1_bss_38F0[16].entries[3].positions[1].x = 410.0f;
        lbl_1_bss_38F0[16].entries[3].positions[1].y = 0.0f;
        lbl_1_bss_38F0[16].entries[3].positions[1].z = 113.0f;
        lbl_1_bss_38F0[17].entries[1].state = 1;
        lbl_1_bss_38F0[17].entries[1].pointCount = 2;
        lbl_1_bss_38F0[17].entries[1].directions[0].x = 1.0f;
        lbl_1_bss_38F0[17].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[17].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[17].entries[1].positions[0].x = 392.0f;
        lbl_1_bss_38F0[17].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[17].entries[1].positions[0].z = 650.0f;
        lbl_1_bss_38F0[17].entries[1].directions[1].x = -1.0f;
        lbl_1_bss_38F0[17].entries[1].directions[1].y = 0.0f;
        lbl_1_bss_38F0[17].entries[1].directions[1].z = 1.0f;
        lbl_1_bss_38F0[17].entries[1].positions[1].x = 176.0f;
        lbl_1_bss_38F0[17].entries[1].positions[1].y = 0.0f;
        lbl_1_bss_38F0[17].entries[1].positions[1].z = 650.0f;
        lbl_1_bss_38F0[17].entries[2].state = 2;
        lbl_1_bss_38F0[17].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[17].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[17].entries[2].directions[0].z = 0.0f;
        lbl_1_bss_38F0[17].entries[2].positions[0].x = 300.0f;
        lbl_1_bss_38F0[17].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[17].entries[2].positions[0].z = 530.0f;
        lbl_1_bss_38F0[17].entries[3].state = 3;
        lbl_1_bss_38F0[17].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[17].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[17].entries[3].directions[0].z = 0.0f;
        lbl_1_bss_38F0[17].entries[3].positions[0].x = 260.0f;
        lbl_1_bss_38F0[17].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[17].entries[3].positions[0].z = 530.0f;
        lbl_1_bss_38F0[18].entries[2].state = 2;
        lbl_1_bss_38F0[18].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[18].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[18].entries[2].directions[0].z = 0.0f;
        lbl_1_bss_38F0[18].entries[2].positions[0].x = 126.0f;
        lbl_1_bss_38F0[18].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[18].entries[2].positions[0].z = 581.0f;
        lbl_1_bss_38F0[18].entries[3].state = 3;
        lbl_1_bss_38F0[18].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[18].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[18].entries[3].directions[0].z = 0.0f;
        lbl_1_bss_38F0[18].entries[3].positions[0].x = -445.0f;
        lbl_1_bss_38F0[18].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[18].entries[3].positions[0].z = 414.0f;
        lbl_1_bss_38F0[19].entries[0].state = 0;
        lbl_1_bss_38F0[19].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[19].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[19].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[19].entries[0].positions[0].x = 126.0f;
        lbl_1_bss_38F0[19].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[19].entries[0].positions[0].z = 581.0f;
        lbl_1_bss_38F0[19].entries[1].state = 1;
        lbl_1_bss_38F0[19].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[19].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[19].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[19].entries[1].positions[0].x = 126.0f;
        lbl_1_bss_38F0[19].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[19].entries[1].positions[0].z = 600.0f;
        lbl_1_bss_38F0[19].entries[3].state = 3;
        lbl_1_bss_38F0[19].entries[3].directions[0].x = -1.0f;
        lbl_1_bss_38F0[19].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[19].entries[3].directions[0].z = 0.0f;
        lbl_1_bss_38F0[19].entries[3].positions[0].x = -495.0f;
        lbl_1_bss_38F0[19].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[19].entries[3].positions[0].z = 414.0f;
        lbl_1_bss_38F0[20].entries[0].state = 0;
        lbl_1_bss_38F0[20].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[20].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[20].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[20].entries[0].positions[0].x = 126.0f;
        lbl_1_bss_38F0[20].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[20].entries[0].positions[0].z = 581.0f;
        lbl_1_bss_38F0[20].entries[1].state = 1;
        lbl_1_bss_38F0[20].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[20].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[20].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[20].entries[1].positions[0].x = 126.0f;
        lbl_1_bss_38F0[20].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[20].entries[1].positions[0].z = 600.0f;
        lbl_1_bss_38F0[20].entries[2].state = 2;
        lbl_1_bss_38F0[20].entries[2].pointCount = 2;
        lbl_1_bss_38F0[20].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[20].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[20].entries[2].directions[0].z = 1.0f;
        lbl_1_bss_38F0[20].entries[2].positions[0].x = -50.0f;
        lbl_1_bss_38F0[20].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[20].entries[2].positions[0].z = 734.0f;
        lbl_1_bss_38F0[20].entries[2].directions[1].x = -1.0f;
        lbl_1_bss_38F0[20].entries[2].directions[1].y = 0.0f;
        lbl_1_bss_38F0[20].entries[2].directions[1].z = -1.0f;
        lbl_1_bss_38F0[20].entries[2].positions[1].x = -50.0f;
        lbl_1_bss_38F0[20].entries[2].positions[1].y = 0.0f;
        lbl_1_bss_38F0[20].entries[2].positions[1].z = 457.0f;
    } else {
        lbl_1_bss_38F0[0].entries[1].state = 1;
        lbl_1_bss_38F0[0].entries[1].pointCount = 2;
        lbl_1_bss_38F0[0].entries[1].directions[0].x = 1.0f;
        lbl_1_bss_38F0[0].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[0].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[0].entries[1].positions[0].x = -315.0f;
        lbl_1_bss_38F0[0].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[0].entries[1].positions[0].z = -634.0f;
        lbl_1_bss_38F0[0].entries[1].directions[1].x = -1.0f;
        lbl_1_bss_38F0[0].entries[1].directions[1].y = 0.0f;
        lbl_1_bss_38F0[0].entries[1].directions[1].z = 1.0f;
        lbl_1_bss_38F0[0].entries[1].positions[1].x = -580.0f;
        lbl_1_bss_38F0[0].entries[1].positions[1].y = 0.0f;
        lbl_1_bss_38F0[0].entries[1].positions[1].z = -634.0f;
        lbl_1_bss_38F0[0].entries[2].state = 2;
        lbl_1_bss_38F0[0].entries[2].pointCount = 2;
        lbl_1_bss_38F0[0].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[0].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[0].entries[2].directions[0].z = 1.0f;
        lbl_1_bss_38F0[0].entries[2].positions[0].x = -400.0f;
        lbl_1_bss_38F0[0].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[0].entries[2].positions[0].z = -288.0f;
        lbl_1_bss_38F0[0].entries[2].directions[1].x = -1.0f;
        lbl_1_bss_38F0[0].entries[2].directions[1].y = 0.0f;
        lbl_1_bss_38F0[0].entries[2].directions[1].z = -1.0f;
        lbl_1_bss_38F0[0].entries[2].positions[1].x = -400.0f;
        lbl_1_bss_38F0[0].entries[2].positions[1].y = 0.0f;
        lbl_1_bss_38F0[0].entries[2].positions[1].z = -739.0f;
        lbl_1_bss_38F0[0].entries[3].state = 3;
        lbl_1_bss_38F0[0].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[0].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[0].entries[3].directions[0].z = 0.0f;
        lbl_1_bss_38F0[0].entries[3].positions[0].x = -470.0f;
        lbl_1_bss_38F0[0].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[0].entries[3].positions[0].z = -748.0f;
        lbl_1_bss_38F0[1].entries[0].state = 0;
        lbl_1_bss_38F0[1].entries[0].pointCount = 2;
        lbl_1_bss_38F0[1].entries[0].directions[0].x = 1.0f;
        lbl_1_bss_38F0[1].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[1].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[1].entries[0].positions[0].x = -482.0f;
        lbl_1_bss_38F0[1].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[1].entries[0].positions[0].z = -170.0f;
        lbl_1_bss_38F0[1].entries[0].directions[1].x = -1.0f;
        lbl_1_bss_38F0[1].entries[0].directions[1].y = 0.0f;
        lbl_1_bss_38F0[1].entries[0].directions[1].z = -1.0f;
        lbl_1_bss_38F0[1].entries[0].positions[1].x = -708.0f;
        lbl_1_bss_38F0[1].entries[0].positions[1].y = 0.0f;
        lbl_1_bss_38F0[1].entries[0].positions[1].z = -170.0f;
        lbl_1_bss_38F0[1].entries[2].state = 2;
        lbl_1_bss_38F0[1].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[1].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[1].entries[2].directions[0].z = 0.0f;
        lbl_1_bss_38F0[1].entries[2].positions[0].x = -619.0f;
        lbl_1_bss_38F0[1].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[1].entries[2].positions[0].z = -101.0f;
        lbl_1_bss_38F0[1].entries[3].state = 3;
        lbl_1_bss_38F0[1].entries[3].pointCount = 2;
        lbl_1_bss_38F0[1].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[1].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[1].entries[3].directions[0].z = 1.0f;
        lbl_1_bss_38F0[1].entries[3].positions[0].x = -601.0f;
        lbl_1_bss_38F0[1].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[1].entries[3].positions[0].z = -53.0f;
        lbl_1_bss_38F0[1].entries[3].directions[1].x = 1.0f;
        lbl_1_bss_38F0[1].entries[3].directions[1].y = 0.0f;
        lbl_1_bss_38F0[1].entries[3].directions[1].z = -1.0f;
        lbl_1_bss_38F0[1].entries[3].positions[1].x = -601.0f;
        lbl_1_bss_38F0[1].entries[3].positions[1].y = 0.0f;
        lbl_1_bss_38F0[1].entries[3].positions[1].z = -572.0f;
        lbl_1_bss_38F0[2].entries[0].state = 0;
        lbl_1_bss_38F0[2].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[2].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[2].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[2].entries[0].positions[0].x = -286.0f;
        lbl_1_bss_38F0[2].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[2].entries[0].positions[0].z = -414.0f;
        lbl_1_bss_38F0[2].entries[1].state = 1;
        lbl_1_bss_38F0[2].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[2].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[2].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[2].entries[1].positions[0].x = -718.0f;
        lbl_1_bss_38F0[2].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[2].entries[1].positions[0].z = -425.0f;
        lbl_1_bss_38F0[2].entries[2].state = 2;
        lbl_1_bss_38F0[2].entries[2].pointCount = 2;
        lbl_1_bss_38F0[2].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[2].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[2].entries[2].directions[0].z = 1.0f;
        lbl_1_bss_38F0[2].entries[2].positions[0].x = -400.0f;
        lbl_1_bss_38F0[2].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[2].entries[2].positions[0].z = -288.0f;
        lbl_1_bss_38F0[2].entries[2].directions[1].x = -1.0f;
        lbl_1_bss_38F0[2].entries[2].directions[1].y = 0.0f;
        lbl_1_bss_38F0[2].entries[2].directions[1].z = -1.0f;
        lbl_1_bss_38F0[2].entries[2].positions[1].x = -400.0f;
        lbl_1_bss_38F0[2].entries[2].positions[1].y = 0.0f;
        lbl_1_bss_38F0[2].entries[2].positions[1].z = -739.0f;
        lbl_1_bss_38F0[2].entries[3].state = 3;
        lbl_1_bss_38F0[2].entries[3].pointCount = 2;
        lbl_1_bss_38F0[2].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[2].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[2].entries[3].directions[0].z = 1.0f;
        lbl_1_bss_38F0[2].entries[3].positions[0].x = -601.0f;
        lbl_1_bss_38F0[2].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[2].entries[3].positions[0].z = -53.0f;
        lbl_1_bss_38F0[2].entries[3].directions[1].x = 1.0f;
        lbl_1_bss_38F0[2].entries[3].directions[1].y = 0.0f;
        lbl_1_bss_38F0[2].entries[3].directions[1].z = -1.0f;
        lbl_1_bss_38F0[2].entries[3].positions[1].x = -601.0f;
        lbl_1_bss_38F0[2].entries[3].positions[1].y = 0.0f;
        lbl_1_bss_38F0[2].entries[3].positions[1].z = -572.0f;
        lbl_1_bss_38F0[3].entries[2].state = 2;
        lbl_1_bss_38F0[3].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[3].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[3].entries[2].directions[0].z = 0.0f;
        lbl_1_bss_38F0[3].entries[2].positions[0].x = 367.0f;
        lbl_1_bss_38F0[3].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[3].entries[2].positions[0].z = -496.0f;
        lbl_1_bss_38F0[3].entries[3].state = 3;
        lbl_1_bss_38F0[3].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[3].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[3].entries[3].directions[0].z = 0.0f;
        lbl_1_bss_38F0[3].entries[3].positions[0].x = -163.0f;
        lbl_1_bss_38F0[3].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[3].entries[3].positions[0].z = -343.0f;
        lbl_1_bss_38F0[4].entries[0].state = 0;
        lbl_1_bss_38F0[4].entries[0].pointCount = 2;
        lbl_1_bss_38F0[4].entries[0].directions[0].x = 1.0f;
        lbl_1_bss_38F0[4].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[4].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[4].entries[0].positions[0].x = -77.0f;
        lbl_1_bss_38F0[4].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[4].entries[0].positions[0].z = -420.0f;
        lbl_1_bss_38F0[4].entries[0].directions[1].x = -1.0f;
        lbl_1_bss_38F0[4].entries[0].directions[1].y = 0.0f;
        lbl_1_bss_38F0[4].entries[0].directions[1].z = -1.0f;
        lbl_1_bss_38F0[4].entries[0].positions[1].x = -282.0f;
        lbl_1_bss_38F0[4].entries[0].positions[1].y = 0.0f;
        lbl_1_bss_38F0[4].entries[0].positions[1].z = -420.0f;
        lbl_1_bss_38F0[4].entries[2].state = 2;
        lbl_1_bss_38F0[4].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[4].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[4].entries[2].directions[0].z = 0.0f;
        lbl_1_bss_38F0[4].entries[2].positions[0].x = -184.0f;
        lbl_1_bss_38F0[4].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[4].entries[2].positions[0].z = -328.0f;
        lbl_1_bss_38F0[4].entries[3].state = 3;
        lbl_1_bss_38F0[4].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[4].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[4].entries[3].directions[0].z = 0.0f;
        lbl_1_bss_38F0[4].entries[3].positions[0].x = -163.0f;
        lbl_1_bss_38F0[4].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[4].entries[3].positions[0].z = -343.0f;
        lbl_1_bss_38F0[5].entries[0].state = 0;
        lbl_1_bss_38F0[5].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[5].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[5].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[5].entries[0].positions[0].x = 345.0f;
        lbl_1_bss_38F0[5].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[5].entries[0].positions[0].z = -564.0f;
        lbl_1_bss_38F0[5].entries[1].state = 1;
        lbl_1_bss_38F0[5].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[5].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[5].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[5].entries[1].positions[0].x = 367.0f;
        lbl_1_bss_38F0[5].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[5].entries[1].positions[0].z = -496.0f;
        lbl_1_bss_38F0[6].entries[0].state = 0;
        lbl_1_bss_38F0[6].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[6].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[6].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[6].entries[0].positions[0].x = 345.0f;
        lbl_1_bss_38F0[6].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[6].entries[0].positions[0].z = -564.0f;
        lbl_1_bss_38F0[6].entries[1].state = 1;
        lbl_1_bss_38F0[6].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[6].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[6].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[6].entries[1].positions[0].x = 367.0f;
        lbl_1_bss_38F0[6].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[6].entries[1].positions[0].z = -496.0f;
        lbl_1_bss_38F0[6].entries[2].state = 2;
        lbl_1_bss_38F0[6].entries[2].pointCount = 2;
        lbl_1_bss_38F0[6].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[6].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[6].entries[2].directions[0].z = 1.0f;
        lbl_1_bss_38F0[6].entries[2].positions[0].x = 220.0f;
        lbl_1_bss_38F0[6].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[6].entries[2].positions[0].z = -404.0f;
        lbl_1_bss_38F0[6].entries[2].directions[1].x = -1.0f;
        lbl_1_bss_38F0[6].entries[2].directions[1].y = 0.0f;
        lbl_1_bss_38F0[6].entries[2].directions[1].z = -1.0f;
        lbl_1_bss_38F0[6].entries[2].positions[1].x = 220.0f;
        lbl_1_bss_38F0[6].entries[2].positions[1].y = 0.0f;
        lbl_1_bss_38F0[6].entries[2].positions[1].z = -712.0f;
        lbl_1_bss_38F0[7].entries[0].state = 0;
        lbl_1_bss_38F0[7].entries[0].pointCount = 2;
        lbl_1_bss_38F0[7].entries[0].directions[0].x = 1.0f;
        lbl_1_bss_38F0[7].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[7].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[7].entries[0].positions[0].x = 604.0f;
        lbl_1_bss_38F0[7].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[7].entries[0].positions[0].z = -343.0f;
        lbl_1_bss_38F0[7].entries[0].directions[1].x = -1.0f;
        lbl_1_bss_38F0[7].entries[0].directions[1].y = 0.0f;
        lbl_1_bss_38F0[7].entries[0].directions[1].z = -1.0f;
        lbl_1_bss_38F0[7].entries[0].positions[1].x = 400.0f;
        lbl_1_bss_38F0[7].entries[0].positions[1].y = 0.0f;
        lbl_1_bss_38F0[7].entries[0].positions[1].z = -343.0f;
        lbl_1_bss_38F0[7].entries[1].state = 1;
        lbl_1_bss_38F0[7].entries[1].pointCount = 2;
        lbl_1_bss_38F0[7].entries[1].directions[0].x = 1.0f;
        lbl_1_bss_38F0[7].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[7].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[7].entries[1].positions[0].x = 602.0f;
        lbl_1_bss_38F0[7].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[7].entries[1].positions[0].z = -532.0f;
        lbl_1_bss_38F0[7].entries[1].directions[1].x = -1.0f;
        lbl_1_bss_38F0[7].entries[1].directions[1].y = 0.0f;
        lbl_1_bss_38F0[7].entries[1].directions[1].z = 1.0f;
        lbl_1_bss_38F0[7].entries[1].positions[1].x = 412.0f;
        lbl_1_bss_38F0[7].entries[1].positions[1].y = 0.0f;
        lbl_1_bss_38F0[7].entries[1].positions[1].z = -532.0f;
        lbl_1_bss_38F0[7].entries[2].state = 2;
        lbl_1_bss_38F0[7].entries[2].pointCount = 2;
        lbl_1_bss_38F0[7].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[7].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[7].entries[2].directions[0].z = 1.0f;
        lbl_1_bss_38F0[7].entries[2].positions[0].x = 517.0f;
        lbl_1_bss_38F0[7].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[7].entries[2].positions[0].z = -229.0f;
        lbl_1_bss_38F0[7].entries[2].directions[1].x = -1.0f;
        lbl_1_bss_38F0[7].entries[2].directions[1].y = 0.0f;
        lbl_1_bss_38F0[7].entries[2].directions[1].z = -1.0f;
        lbl_1_bss_38F0[7].entries[2].positions[1].x = 517.0f;
        lbl_1_bss_38F0[7].entries[2].positions[1].y = 0.0f;
        lbl_1_bss_38F0[7].entries[2].positions[1].z = -675.0f;
        lbl_1_bss_38F0[7].entries[3].state = 3;
        lbl_1_bss_38F0[7].entries[3].pointCount = 2;
        lbl_1_bss_38F0[7].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[7].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[7].entries[3].directions[0].z = 1.0f;
        lbl_1_bss_38F0[7].entries[3].positions[0].x = 478.0f;
        lbl_1_bss_38F0[7].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[7].entries[3].positions[0].z = -226.0f;
        lbl_1_bss_38F0[7].entries[3].directions[1].x = 1.0f;
        lbl_1_bss_38F0[7].entries[3].directions[1].y = 0.0f;
        lbl_1_bss_38F0[7].entries[3].directions[1].z = -1.0f;
        lbl_1_bss_38F0[7].entries[3].positions[1].x = 478.0f;
        lbl_1_bss_38F0[7].entries[3].positions[1].y = 0.0f;
        lbl_1_bss_38F0[7].entries[3].positions[1].z = -675.0f;
        lbl_1_bss_38F0[8].entries[0].state = 0;
        lbl_1_bss_38F0[8].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[8].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[8].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[8].entries[0].positions[0].x = -371.0f;
        lbl_1_bss_38F0[8].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[8].entries[0].positions[0].z = -191.0f;
        lbl_1_bss_38F0[8].entries[1].state = 1;
        lbl_1_bss_38F0[8].entries[1].pointCount = 2;
        lbl_1_bss_38F0[8].entries[1].directions[0].x = 1.0f;
        lbl_1_bss_38F0[8].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[8].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[8].entries[1].positions[0].x = 274.0f;
        lbl_1_bss_38F0[8].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[8].entries[1].positions[0].z = -186.0f;
        lbl_1_bss_38F0[8].entries[1].directions[1].x = -1.0f;
        lbl_1_bss_38F0[8].entries[1].directions[1].y = 0.0f;
        lbl_1_bss_38F0[8].entries[1].directions[1].z = 1.0f;
        lbl_1_bss_38F0[8].entries[1].positions[1].x = -400.0f;
        lbl_1_bss_38F0[8].entries[1].positions[1].y = 0.0f;
        lbl_1_bss_38F0[8].entries[1].positions[1].z = -186.0f;
        lbl_1_bss_38F0[8].entries[3].state = 1;
        lbl_1_bss_38F0[8].entries[3].pointCount = 2;
        lbl_1_bss_38F0[8].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[8].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[8].entries[3].directions[0].z = 1.0f;
        lbl_1_bss_38F0[8].entries[3].positions[0].x = -295.0f;
        lbl_1_bss_38F0[8].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[8].entries[3].positions[0].z = -65.0f;
        lbl_1_bss_38F0[8].entries[3].directions[1].x = 1.0f;
        lbl_1_bss_38F0[8].entries[3].directions[1].y = 0.0f;
        lbl_1_bss_38F0[8].entries[3].directions[1].z = -1.0f;
        lbl_1_bss_38F0[8].entries[3].positions[1].x = -295.0f;
        lbl_1_bss_38F0[8].entries[3].positions[1].y = 0.0f;
        lbl_1_bss_38F0[8].entries[3].positions[1].z = -313.0f;
        lbl_1_bss_38F0[9].entries[0].state = 0;
        lbl_1_bss_38F0[9].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[9].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[9].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[9].entries[0].positions[0].x = 243.0f;
        lbl_1_bss_38F0[9].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[9].entries[0].positions[0].z = -162.0f;
        lbl_1_bss_38F0[9].entries[1].state = 1;
        lbl_1_bss_38F0[9].entries[1].pointCount = 2;
        lbl_1_bss_38F0[9].entries[1].directions[0].x = 1.0f;
        lbl_1_bss_38F0[9].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[9].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[9].entries[1].positions[0].x = 274.0f;
        lbl_1_bss_38F0[9].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[9].entries[1].positions[0].z = -186.0f;
        lbl_1_bss_38F0[9].entries[1].directions[1].x = -1.0f;
        lbl_1_bss_38F0[9].entries[1].directions[1].y = 0.0f;
        lbl_1_bss_38F0[9].entries[1].directions[1].z = 1.0f;
        lbl_1_bss_38F0[9].entries[1].positions[1].x = -400.0f;
        lbl_1_bss_38F0[9].entries[1].positions[1].y = 0.0f;
        lbl_1_bss_38F0[9].entries[1].positions[1].z = -186.0f;
        lbl_1_bss_38F0[9].entries[2].state = 2;
        lbl_1_bss_38F0[9].entries[2].pointCount = 2;
        lbl_1_bss_38F0[9].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[9].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[9].entries[2].directions[0].z = 1.0f;
        lbl_1_bss_38F0[9].entries[2].positions[0].x = 147.0f;
        lbl_1_bss_38F0[9].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[9].entries[2].positions[0].z = -26.0f;
        lbl_1_bss_38F0[9].entries[2].directions[1].x = -1.0f;
        lbl_1_bss_38F0[9].entries[2].directions[1].y = 0.0f;
        lbl_1_bss_38F0[9].entries[2].directions[1].z = -1.0f;
        lbl_1_bss_38F0[9].entries[2].positions[1].x = 147.0f;
        lbl_1_bss_38F0[9].entries[2].positions[1].y = 0.0f;
        lbl_1_bss_38F0[9].entries[2].positions[1].z = -285.0f;
        lbl_1_bss_38F0[10].entries[0].state = 0;
        lbl_1_bss_38F0[10].entries[0].pointCount = 2;
        lbl_1_bss_38F0[10].entries[0].directions[0].x = 1.0f;
        lbl_1_bss_38F0[10].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[10].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[10].entries[0].positions[0].x = 14.0f;
        lbl_1_bss_38F0[10].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[10].entries[0].positions[0].z = 3.0f;
        lbl_1_bss_38F0[10].entries[0].directions[1].x = -1.0f;
        lbl_1_bss_38F0[10].entries[0].directions[1].y = 0.0f;
        lbl_1_bss_38F0[10].entries[0].directions[1].z = -1.0f;
        lbl_1_bss_38F0[10].entries[0].positions[1].x = -179.0f;
        lbl_1_bss_38F0[10].entries[0].positions[1].y = 0.0f;
        lbl_1_bss_38F0[10].entries[0].positions[1].z = 3.0f;
        lbl_1_bss_38F0[10].entries[1].state = 1;
        lbl_1_bss_38F0[10].entries[1].pointCount = 2;
        lbl_1_bss_38F0[10].entries[1].directions[0].x = 1.0f;
        lbl_1_bss_38F0[10].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[10].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[10].entries[1].positions[0].x = 274.0f;
        lbl_1_bss_38F0[10].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[10].entries[1].positions[0].z = -186.0f;
        lbl_1_bss_38F0[10].entries[1].directions[1].x = -1.0f;
        lbl_1_bss_38F0[10].entries[1].directions[1].y = 0.0f;
        lbl_1_bss_38F0[10].entries[1].directions[1].z = 1.0f;
        lbl_1_bss_38F0[10].entries[1].positions[1].x = -400.0f;
        lbl_1_bss_38F0[10].entries[1].positions[1].y = 0.0f;
        lbl_1_bss_38F0[10].entries[1].positions[1].z = -186.0f;
        lbl_1_bss_38F0[10].entries[2].state = 2;
        lbl_1_bss_38F0[10].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[10].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[10].entries[2].directions[0].z = 0.0f;
        lbl_1_bss_38F0[10].entries[2].positions[0].x = -83.0f;
        lbl_1_bss_38F0[10].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[10].entries[2].positions[0].z = 177.0f;
        lbl_1_bss_38F0[10].entries[3].state = 3;
        lbl_1_bss_38F0[10].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[10].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[10].entries[3].directions[0].z = 0.0f;
        lbl_1_bss_38F0[10].entries[3].positions[0].x = -100.0f;
        lbl_1_bss_38F0[10].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[10].entries[3].positions[0].z = 145.0f;
        lbl_1_bss_38F0[11].entries[0].state = 0;
        lbl_1_bss_38F0[11].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[11].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[11].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[11].entries[0].positions[0].x = -257.0f;
        lbl_1_bss_38F0[11].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[11].entries[0].positions[0].z = 217.0f;
        lbl_1_bss_38F0[11].entries[1].state = 1;
        lbl_1_bss_38F0[11].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[11].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[11].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[11].entries[1].positions[0].x = -526.0f;
        lbl_1_bss_38F0[11].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[11].entries[1].positions[0].z = -85.0f;
        lbl_1_bss_38F0[12].entries[0].state = 0;
        lbl_1_bss_38F0[12].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[12].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[12].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[12].entries[0].positions[0].x = -257.0f;
        lbl_1_bss_38F0[12].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[12].entries[0].positions[0].z = 217.0f;
        lbl_1_bss_38F0[12].entries[1].state = 1;
        lbl_1_bss_38F0[12].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[12].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[12].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[12].entries[1].positions[0].x = -267.0f;
        lbl_1_bss_38F0[12].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[12].entries[1].positions[0].z = 209.0f;
        lbl_1_bss_38F0[12].entries[2].state = 2;
        lbl_1_bss_38F0[12].entries[2].pointCount = 2;
        lbl_1_bss_38F0[12].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[12].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[12].entries[2].directions[0].z = 1.0f;
        lbl_1_bss_38F0[12].entries[2].positions[0].x = -362.0f;
        lbl_1_bss_38F0[12].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[12].entries[2].positions[0].z = 323.0f;
        lbl_1_bss_38F0[12].entries[2].directions[1].x = -1.0f;
        lbl_1_bss_38F0[12].entries[2].directions[1].y = 0.0f;
        lbl_1_bss_38F0[12].entries[2].directions[1].z = -1.0f;
        lbl_1_bss_38F0[12].entries[2].positions[1].x = -362.0f;
        lbl_1_bss_38F0[12].entries[2].positions[1].y = 0.0f;
        lbl_1_bss_38F0[12].entries[2].positions[1].z = 76.0f;
        lbl_1_bss_38F0[13].entries[0].state = 0;
        lbl_1_bss_38F0[13].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[13].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[13].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[13].entries[0].positions[0].x = -257.0f;
        lbl_1_bss_38F0[13].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[13].entries[0].positions[0].z = 217.0f;
        lbl_1_bss_38F0[13].entries[1].state = 1;
        lbl_1_bss_38F0[13].entries[1].pointCount = 2;
        lbl_1_bss_38F0[13].entries[1].directions[0].x = 1.0f;
        lbl_1_bss_38F0[13].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[13].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[13].entries[1].positions[0].x = -493.0f;
        lbl_1_bss_38F0[13].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[13].entries[1].positions[0].z = 30.0f;
        lbl_1_bss_38F0[13].entries[1].directions[1].x = -1.0f;
        lbl_1_bss_38F0[13].entries[1].directions[1].y = 0.0f;
        lbl_1_bss_38F0[13].entries[1].directions[1].z = 1.0f;
        lbl_1_bss_38F0[13].entries[1].positions[1].x = -670.0f;
        lbl_1_bss_38F0[13].entries[1].positions[1].y = 0.0f;
        lbl_1_bss_38F0[13].entries[1].positions[1].z = 30.0f;
        lbl_1_bss_38F0[13].entries[2].state = 2;
        lbl_1_bss_38F0[13].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[13].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[13].entries[2].directions[0].z = 0.0f;
        lbl_1_bss_38F0[13].entries[2].positions[0].x = -598.0f;
        lbl_1_bss_38F0[13].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[13].entries[2].positions[0].z = -80.0f;
        lbl_1_bss_38F0[13].entries[3].state = 3;
        lbl_1_bss_38F0[13].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[13].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[13].entries[3].directions[0].z = 0.0f;
        lbl_1_bss_38F0[13].entries[3].positions[0].x = -618.0f;
        lbl_1_bss_38F0[13].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[13].entries[3].positions[0].z = -80.0f;
        lbl_1_bss_38F0[14].entries[0].state = 0;
        lbl_1_bss_38F0[14].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[14].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[14].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[14].entries[0].positions[0].x = 360.0f;
        lbl_1_bss_38F0[14].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[14].entries[0].positions[0].z = 532.0f;
        lbl_1_bss_38F0[14].entries[1].state = 1;
        lbl_1_bss_38F0[14].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[14].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[14].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[14].entries[1].positions[0].x = 309.0f;
        lbl_1_bss_38F0[14].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[14].entries[1].positions[0].z = -81.0f;
        lbl_1_bss_38F0[15].entries[2].state = 2;
        lbl_1_bss_38F0[15].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[15].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[15].entries[2].directions[0].z = 0.0f;
        lbl_1_bss_38F0[15].entries[2].positions[0].x = 543.0f;
        lbl_1_bss_38F0[15].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[15].entries[2].positions[0].z = 529.0f;
        lbl_1_bss_38F0[15].entries[3].state = 3;
        lbl_1_bss_38F0[15].entries[3].pointCount = 2;
        lbl_1_bss_38F0[15].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[15].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[15].entries[3].directions[0].z = 1.0f;
        lbl_1_bss_38F0[15].entries[3].positions[0].x = -588.0f;
        lbl_1_bss_38F0[15].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[15].entries[3].positions[0].z = 515.0f;
        lbl_1_bss_38F0[15].entries[3].directions[1].x = 1.0f;
        lbl_1_bss_38F0[15].entries[3].directions[1].y = 0.0f;
        lbl_1_bss_38F0[15].entries[3].directions[1].z = -1.0f;
        lbl_1_bss_38F0[15].entries[3].positions[1].x = 305.0f;
        lbl_1_bss_38F0[15].entries[3].positions[1].y = 0.0f;
        lbl_1_bss_38F0[15].entries[3].positions[1].z = -195.0f;
        lbl_1_bss_38F0[16].entries[0].state = 0;
        lbl_1_bss_38F0[16].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[16].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[16].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[16].entries[0].positions[0].x = 236.0f;
        lbl_1_bss_38F0[16].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[16].entries[0].positions[0].z = -150.0f;
        lbl_1_bss_38F0[16].entries[1].state = 1;
        lbl_1_bss_38F0[16].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[16].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[16].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[16].entries[1].positions[0].x = 309.0f;
        lbl_1_bss_38F0[16].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[16].entries[1].positions[0].z = -81.0f;
        lbl_1_bss_38F0[16].entries[3].state = 3;
        lbl_1_bss_38F0[16].entries[3].pointCount = 2;
        lbl_1_bss_38F0[16].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[16].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[16].entries[3].directions[0].z = 1.0f;
        lbl_1_bss_38F0[16].entries[3].positions[0].x = 395.0f;
        lbl_1_bss_38F0[16].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[16].entries[3].positions[0].z = 12.0f;
        lbl_1_bss_38F0[16].entries[3].directions[1].x = 1.0f;
        lbl_1_bss_38F0[16].entries[3].directions[1].y = 0.0f;
        lbl_1_bss_38F0[16].entries[3].directions[1].z = -1.0f;
        lbl_1_bss_38F0[16].entries[3].positions[1].x = 417.0f;
        lbl_1_bss_38F0[16].entries[3].positions[1].y = 0.0f;
        lbl_1_bss_38F0[16].entries[3].positions[1].z = -206.0f;
        lbl_1_bss_38F0[17].entries[0].state = 0;
        lbl_1_bss_38F0[17].entries[0].pointCount = 2;
        lbl_1_bss_38F0[17].entries[0].directions[0].x = 1.0f;
        lbl_1_bss_38F0[17].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[17].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[17].entries[0].positions[0].x = 685.0f;
        lbl_1_bss_38F0[17].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[17].entries[0].positions[0].z = 333.0f;
        lbl_1_bss_38F0[17].entries[0].directions[1].x = -1.0f;
        lbl_1_bss_38F0[17].entries[0].directions[1].y = 0.0f;
        lbl_1_bss_38F0[17].entries[0].directions[1].z = -1.0f;
        lbl_1_bss_38F0[17].entries[0].positions[1].x = 471.0f;
        lbl_1_bss_38F0[17].entries[0].positions[1].y = 0.0f;
        lbl_1_bss_38F0[17].entries[0].positions[1].z = 333.0f;
        lbl_1_bss_38F0[17].entries[2].state = 2;
        lbl_1_bss_38F0[17].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[17].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[17].entries[2].directions[0].z = 0.0f;
        lbl_1_bss_38F0[17].entries[2].positions[0].x = 543.0f;
        lbl_1_bss_38F0[17].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[17].entries[2].positions[0].z = 529.0f;
        lbl_1_bss_38F0[17].entries[3].state = 3;
        lbl_1_bss_38F0[17].entries[3].pointCount = 2;
        lbl_1_bss_38F0[17].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[17].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[17].entries[3].directions[0].z = 1.0f;
        lbl_1_bss_38F0[17].entries[3].positions[0].x = 588.0f;
        lbl_1_bss_38F0[17].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[17].entries[3].positions[0].z = 515.0f;
        lbl_1_bss_38F0[17].entries[3].directions[1].x = 1.0f;
        lbl_1_bss_38F0[17].entries[3].directions[1].y = 0.0f;
        lbl_1_bss_38F0[17].entries[3].directions[1].z = -1.0f;
        lbl_1_bss_38F0[17].entries[3].positions[1].x = 305.0f;
        lbl_1_bss_38F0[17].entries[3].positions[1].y = 0.0f;
        lbl_1_bss_38F0[17].entries[3].positions[1].z = -195.0f;
        lbl_1_bss_38F0[18].entries[0].state = 0;
        lbl_1_bss_38F0[18].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[18].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[18].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[18].entries[0].positions[0].x = -269.0f;
        lbl_1_bss_38F0[18].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[18].entries[0].positions[0].z = 325.0f;
        lbl_1_bss_38F0[18].entries[1].state = 1;
        lbl_1_bss_38F0[18].entries[1].pointCount = 2;
        lbl_1_bss_38F0[18].entries[1].directions[0].x = 1.0f;
        lbl_1_bss_38F0[18].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[18].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[18].entries[1].positions[0].x = 408.0f;
        lbl_1_bss_38F0[18].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[18].entries[1].positions[0].z = 318.0f;
        lbl_1_bss_38F0[18].entries[1].directions[1].x = -1.0f;
        lbl_1_bss_38F0[18].entries[1].directions[1].y = 0.0f;
        lbl_1_bss_38F0[18].entries[1].directions[1].z = 1.0f;
        lbl_1_bss_38F0[18].entries[1].positions[1].x = -242.0f;
        lbl_1_bss_38F0[18].entries[1].positions[1].y = 0.0f;
        lbl_1_bss_38F0[18].entries[1].positions[1].z = 311.0f;
        lbl_1_bss_38F0[18].entries[3].state = 3;
        lbl_1_bss_38F0[18].entries[3].pointCount = 2;
        lbl_1_bss_38F0[18].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[18].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[18].entries[3].directions[0].z = 1.0f;
        lbl_1_bss_38F0[18].entries[3].positions[0].x = -139.0f;
        lbl_1_bss_38F0[18].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[18].entries[3].positions[0].z = 444.0f;
        lbl_1_bss_38F0[18].entries[3].directions[1].x = 1.0f;
        lbl_1_bss_38F0[18].entries[3].directions[1].y = 0.0f;
        lbl_1_bss_38F0[18].entries[3].directions[1].z = -1.0f;
        lbl_1_bss_38F0[18].entries[3].positions[1].x = -139.0f;
        lbl_1_bss_38F0[18].entries[3].positions[1].y = 0.0f;
        lbl_1_bss_38F0[18].entries[3].positions[1].z = 164.0f;
        lbl_1_bss_38F0[19].entries[0].state = 0;
        lbl_1_bss_38F0[19].entries[0].directions[0].x = 0.0f;
        lbl_1_bss_38F0[19].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[19].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[19].entries[0].positions[0].x = -269.0f;
        lbl_1_bss_38F0[19].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[19].entries[0].positions[0].z = 325.0f;
        lbl_1_bss_38F0[19].entries[1].state = 1;
        lbl_1_bss_38F0[19].entries[1].pointCount = 2;
        lbl_1_bss_38F0[19].entries[1].directions[0].x = 1.0f;
        lbl_1_bss_38F0[19].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[19].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[19].entries[1].positions[0].x = 408.0f;
        lbl_1_bss_38F0[19].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[19].entries[1].positions[0].z = 318.0f;
        lbl_1_bss_38F0[19].entries[1].directions[1].x = -1.0f;
        lbl_1_bss_38F0[19].entries[1].directions[1].y = 0.0f;
        lbl_1_bss_38F0[19].entries[1].directions[1].z = 1.0f;
        lbl_1_bss_38F0[19].entries[1].positions[1].x = -242.0f;
        lbl_1_bss_38F0[19].entries[1].positions[1].y = 0.0f;
        lbl_1_bss_38F0[19].entries[1].positions[1].z = 311.0f;
        lbl_1_bss_38F0[20].entries[0].state = 0;
        lbl_1_bss_38F0[20].entries[0].pointCount = 2;
        lbl_1_bss_38F0[20].entries[0].directions[0].x = 1.0f;
        lbl_1_bss_38F0[20].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[20].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[20].entries[0].positions[0].x = 377.0f;
        lbl_1_bss_38F0[20].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[20].entries[0].positions[0].z = 474.0f;
        lbl_1_bss_38F0[20].entries[0].directions[1].x = -1.0f;
        lbl_1_bss_38F0[20].entries[0].directions[1].y = 0.0f;
        lbl_1_bss_38F0[20].entries[0].directions[1].z = -1.0f;
        lbl_1_bss_38F0[20].entries[0].positions[1].x = 187.0f;
        lbl_1_bss_38F0[20].entries[0].positions[1].y = 0.0f;
        lbl_1_bss_38F0[20].entries[0].positions[1].z = 498.0f;
        lbl_1_bss_38F0[20].entries[1].state = 1;
        lbl_1_bss_38F0[20].entries[1].pointCount = 2;
        lbl_1_bss_38F0[20].entries[1].directions[0].x = 1.0f;
        lbl_1_bss_38F0[20].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[20].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[20].entries[1].positions[0].x = 408.0f;
        lbl_1_bss_38F0[20].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[20].entries[1].positions[0].z = 318.0f;
        lbl_1_bss_38F0[20].entries[1].directions[1].x = -1.0f;
        lbl_1_bss_38F0[20].entries[1].directions[1].y = 0.0f;
        lbl_1_bss_38F0[20].entries[1].directions[1].z = 1.0f;
        lbl_1_bss_38F0[20].entries[1].positions[1].x = -242.0f;
        lbl_1_bss_38F0[20].entries[1].positions[1].y = 0.0f;
        lbl_1_bss_38F0[20].entries[1].positions[1].z = 311.0f;
        lbl_1_bss_38F0[20].entries[2].state = 2;
        lbl_1_bss_38F0[20].entries[2].pointCount = 2;
        lbl_1_bss_38F0[20].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[20].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[20].entries[2].directions[0].z = 1.0f;
        lbl_1_bss_38F0[20].entries[2].positions[0].x = 281.0f;
        lbl_1_bss_38F0[20].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[20].entries[2].positions[0].z = 632.0f;
        lbl_1_bss_38F0[20].entries[2].directions[1].x = -1.0f;
        lbl_1_bss_38F0[20].entries[2].directions[1].y = 0.0f;
        lbl_1_bss_38F0[20].entries[2].directions[1].z = -1.0f;
        lbl_1_bss_38F0[20].entries[2].positions[1].x = 289.0f;
        lbl_1_bss_38F0[20].entries[2].positions[1].y = 0.0f;
        lbl_1_bss_38F0[20].entries[2].positions[1].z = 187.0f;
        lbl_1_bss_38F0[20].entries[3].state = 3;
        lbl_1_bss_38F0[20].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[20].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[20].entries[3].directions[0].z = 0.0f;
        lbl_1_bss_38F0[20].entries[3].positions[0].x = 262.0f;
        lbl_1_bss_38F0[20].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[20].entries[3].positions[0].z = 631.0f;
        lbl_1_bss_38F0[21].entries[0].state = 0;
        lbl_1_bss_38F0[21].entries[0].pointCount = 2;
        lbl_1_bss_38F0[21].entries[0].directions[0].x = 1.0f;
        lbl_1_bss_38F0[21].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[21].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[21].entries[0].positions[0].x = -218.0f;
        lbl_1_bss_38F0[21].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[21].entries[0].positions[0].z = 625.0f;
        lbl_1_bss_38F0[21].entries[0].directions[1].x = -1.0f;
        lbl_1_bss_38F0[21].entries[0].directions[1].y = 0.0f;
        lbl_1_bss_38F0[21].entries[0].directions[1].z = -1.0f;
        lbl_1_bss_38F0[21].entries[0].positions[1].x = -620.0f;
        lbl_1_bss_38F0[21].entries[0].positions[1].y = 0.0f;
        lbl_1_bss_38F0[21].entries[0].positions[1].z = 605.0f;
        lbl_1_bss_38F0[21].entries[1].state = 1;
        lbl_1_bss_38F0[21].entries[1].directions[0].x = 0.0f;
        lbl_1_bss_38F0[21].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[21].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[21].entries[1].positions[0].x = -167.0f;
        lbl_1_bss_38F0[21].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[21].entries[1].positions[0].z = 591.0f;
        lbl_1_bss_38F0[21].entries[2].state = 2;
        lbl_1_bss_38F0[21].entries[2].pointCount = 2;
        lbl_1_bss_38F0[21].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[21].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[21].entries[2].directions[0].z = 1.0f;
        lbl_1_bss_38F0[21].entries[2].positions[0].x = -283.0f;
        lbl_1_bss_38F0[21].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[21].entries[2].positions[0].z = 727.0f;
        lbl_1_bss_38F0[21].entries[2].directions[1].x = -1.0f;
        lbl_1_bss_38F0[21].entries[2].directions[1].y = 0.0f;
        lbl_1_bss_38F0[21].entries[2].directions[1].z = -1.0f;
        lbl_1_bss_38F0[21].entries[2].positions[1].x = -291.0f;
        lbl_1_bss_38F0[21].entries[2].positions[1].y = 0.0f;
        lbl_1_bss_38F0[21].entries[2].positions[1].z = 505.0f;
        lbl_1_bss_38F0[22].entries[0].state = 0;
        lbl_1_bss_38F0[22].entries[0].pointCount = 2;
        lbl_1_bss_38F0[22].entries[0].directions[0].x = 1.0f;
        lbl_1_bss_38F0[22].entries[0].directions[0].y = 0.0f;
        lbl_1_bss_38F0[22].entries[0].directions[0].z = -1.0f;
        lbl_1_bss_38F0[22].entries[0].positions[0].x = -218.0f;
        lbl_1_bss_38F0[22].entries[0].positions[0].y = 0.0f;
        lbl_1_bss_38F0[22].entries[0].positions[0].z = 625.0f;
        lbl_1_bss_38F0[22].entries[0].directions[1].x = -1.0f;
        lbl_1_bss_38F0[22].entries[0].directions[1].y = 0.0f;
        lbl_1_bss_38F0[22].entries[0].directions[1].z = -1.0f;
        lbl_1_bss_38F0[22].entries[0].positions[1].x = -620.0f;
        lbl_1_bss_38F0[22].entries[0].positions[1].y = 0.0f;
        lbl_1_bss_38F0[22].entries[0].positions[1].z = 605.0f;
        lbl_1_bss_38F0[22].entries[1].state = 1;
        lbl_1_bss_38F0[22].entries[1].pointCount = 2;
        lbl_1_bss_38F0[22].entries[1].directions[0].x = 1.0f;
        lbl_1_bss_38F0[22].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[22].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[22].entries[1].positions[0].x = -437.0f;
        lbl_1_bss_38F0[22].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[22].entries[1].positions[0].z = 404.0f;
        lbl_1_bss_38F0[22].entries[1].directions[1].x = -1.0f;
        lbl_1_bss_38F0[22].entries[1].directions[1].y = 0.0f;
        lbl_1_bss_38F0[22].entries[1].directions[1].z = 1.0f;
        lbl_1_bss_38F0[22].entries[1].positions[1].x = -605.0f;
        lbl_1_bss_38F0[22].entries[1].positions[1].y = 0.0f;
        lbl_1_bss_38F0[22].entries[1].positions[1].z = 436.0f;
        lbl_1_bss_38F0[22].entries[2].state = 2;
        lbl_1_bss_38F0[22].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[22].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[22].entries[2].directions[0].z = 0.0f;
        lbl_1_bss_38F0[22].entries[2].positions[0].x = -537.0f;
        lbl_1_bss_38F0[22].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[22].entries[2].positions[0].z = 311.0f;
        lbl_1_bss_38F0[22].entries[3].state = 3;
        lbl_1_bss_38F0[22].entries[3].pointCount = 2;
        lbl_1_bss_38F0[22].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[22].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[22].entries[3].directions[0].z = 1.0f;
        lbl_1_bss_38F0[22].entries[3].positions[0].x = -501.0f;
        lbl_1_bss_38F0[22].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[22].entries[3].positions[0].z = 333.0f;
        lbl_1_bss_38F0[22].entries[3].directions[1].x = 1.0f;
        lbl_1_bss_38F0[22].entries[3].directions[1].y = 0.0f;
        lbl_1_bss_38F0[22].entries[3].directions[1].z = -1.0f;
        lbl_1_bss_38F0[22].entries[3].positions[1].x = -525.0f;
        lbl_1_bss_38F0[22].entries[3].positions[1].y = 0.0f;
        lbl_1_bss_38F0[22].entries[3].positions[1].z = 742.0f;
        lbl_1_bss_38F0[23].entries[1].state = 1;
        lbl_1_bss_38F0[23].entries[1].pointCount = 2;
        lbl_1_bss_38F0[23].entries[1].directions[0].x = 1.0f;
        lbl_1_bss_38F0[23].entries[1].directions[0].y = 0.0f;
        lbl_1_bss_38F0[23].entries[1].directions[0].z = 1.0f;
        lbl_1_bss_38F0[23].entries[1].positions[0].x = 546.0f;
        lbl_1_bss_38F0[23].entries[1].positions[0].y = 0.0f;
        lbl_1_bss_38F0[23].entries[1].positions[0].z = 642.0f;
        lbl_1_bss_38F0[23].entries[1].directions[1].x = -1.0f;
        lbl_1_bss_38F0[23].entries[1].directions[1].y = 0.0f;
        lbl_1_bss_38F0[23].entries[1].directions[1].z = 1.0f;
        lbl_1_bss_38F0[23].entries[1].positions[1].x = 375.0f;
        lbl_1_bss_38F0[23].entries[1].positions[1].y = 0.0f;
        lbl_1_bss_38F0[23].entries[1].positions[1].z = 625.0f;
        lbl_1_bss_38F0[23].entries[2].state = 2;
        lbl_1_bss_38F0[23].entries[2].directions[0].x = -1.0f;
        lbl_1_bss_38F0[23].entries[2].directions[0].y = 0.0f;
        lbl_1_bss_38F0[23].entries[2].directions[0].z = 0.0f;
        lbl_1_bss_38F0[23].entries[2].positions[0].x = 466.0f;
        lbl_1_bss_38F0[23].entries[2].positions[0].y = 0.0f;
        lbl_1_bss_38F0[23].entries[2].positions[0].z = 525.0f;
        lbl_1_bss_38F0[23].entries[3].state = 3;
        lbl_1_bss_38F0[23].entries[3].directions[0].x = 1.0f;
        lbl_1_bss_38F0[23].entries[3].directions[0].y = 0.0f;
        lbl_1_bss_38F0[23].entries[3].directions[0].z = 0.0f;
        lbl_1_bss_38F0[23].entries[3].positions[0].x = 465.0f;
        lbl_1_bss_38F0[23].entries[3].positions[0].y = 0.0f;
        lbl_1_bss_38F0[23].entries[3].positions[0].z = 518.0f;
    }

    for (recordIndex = 0; recordIndex < 24; recordIndex++) {
        for (entryIndex = 0; entryIndex < 4; entryIndex++) {
            if (lbl_1_bss_38F0[recordIndex].entries[entryIndex].state != -1) {
                for (pointIndex = 0;
                     pointIndex < lbl_1_bss_38F0[recordIndex].entries[entryIndex].pointCount;
                     pointIndex++) {
                    position =
                        lbl_1_bss_38F0[recordIndex].entries[entryIndex].positions[pointIndex];
                    angle = (f32)((f64)lbl_1_bss_9C8.rotation +
                        180.0 * (atan2((f64)position.x, (f64)position.z) / 3.141592653589793));
                    distance = (f32)sqrt((f64)(position.x * position.x + position.z * position.z));
                    position.x =
                        (f32) ((f64) distance * sin((3.141592653589793 * (f64) angle) / 180.0));
                    position.z =
                        (f32) ((f64) distance * cos((3.141592653589793 * (f64) angle) / 180.0));
                    lbl_1_bss_38F0[recordIndex].entries[entryIndex].positions[pointIndex] =
                        position;
                    facing = lbl_1_bss_38F0[recordIndex].entries[entryIndex].directions[pointIndex];
                    for (directionIndex = 0; directionIndex < 8; directionIndex++) {
                        if (facing.x == directionVectors[directionIndex].y &&
                            facing.z == directionVectors[directionIndex].z) {
                            angle = directionVectors[directionIndex].x;
                            break;
                        }
                    }
                    angle += lbl_1_bss_9C8.rotation;
                    if (angle >= 360.0f) angle -= 360.0f;
                    directionIndex = (s16)(angle / 45.0f);
                    facing.x = directionVectors[directionIndex].y;
                    facing.z = directionVectors[directionIndex].z;
                    lbl_1_bss_38F0[recordIndex].entries[entryIndex].directions[pointIndex] = facing;
                }
            }
        }
    }

    directionIndex = (s16)(lbl_1_bss_9C8.rotation / 90.0f);
    /* Rotate the route-entry order once for each quarter-turn of the stage. */
    for (recordIndex = 0; recordIndex < directionIndex; recordIndex++) {
        for (entryIndex = 0; entryIndex < 24; entryIndex++) {
            savedEntry = lbl_1_bss_38F0[entryIndex].entries[2];
            lbl_1_bss_38F0[entryIndex].entries[2] = lbl_1_bss_38F0[entryIndex].entries[0];
            lbl_1_bss_38F0[entryIndex].entries[0] = lbl_1_bss_38F0[entryIndex].entries[3];
            lbl_1_bss_38F0[entryIndex].entries[3] = lbl_1_bss_38F0[entryIndex].entries[1];
            lbl_1_bss_38F0[entryIndex].entries[1] = savedEntry;
        }
    }
}

/* Called by fn_1_5DB4 each update to advance eligible computer players' control phases. */
void fn_1_16898(void)
{
    M617PlayerStateData *work;
    s16 playerIndex;

    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        work = &lbl_1_bss_AB0[playerIndex];
        if (work->computerControlled != 0 && (work->motionState == 0 || work->motionState == 2)) {
            /* Clear per-update values before processing this participant's control phase. */
            work->computerButtonsHeld = 0;
            work->computerButtonsDown = 0;
            work->computerStickX = 0;
            work->computerStickY = 0;
            switch (work->controlPhase) {
            case 0:
                work->chaseRadius = 10.1f;
                work->controlPhase++;
                break;
            case 1:
                work->elapsedFrames++;
                fn_1_1817C(work);
                break;
            }
        }
    }
}

#define MOTION_PLAYER(index) (lbl_1_bss_9BC.players[index])

/* Called by fn_1_17538 to start, advance, or finish a participant's chase. */
s16 fn_1_16990(M617PlayerStateData *participant, s16 advanceAfterChase)
{
    HuVecF targetPosition;
    s16 chaseComplete;
    s16 targetReached;

    chaseComplete = 0;
    targetReached = 0;
    switch (participant->chasePhase) {
    case 0:
        MgPlayerComStkOn(MOTION_PLAYER(participant->participantIndex));
        participant->chasePhase++;
        break;
    case 1:
        targetPosition = participant->targetPosition;
        targetReached = MgPlayerVecChase(
            MOTION_PLAYER(participant->participantIndex),
            &targetPosition,
            participant->chaseSpeed,
            participant->chaseRadius);
        if (targetReached != 0) {
            if (advanceAfterChase != 0) {
                participant->chasePhase++;
            } else {
                chaseComplete = 1;
            }
        }
        break;
    case 2:
        MgPlayerComStkOff(MOTION_PLAYER(participant->participantIndex));
        participant->chasePhase = 0;
        chaseComplete = 1;
        break;
    }
    return chaseComplete;
}

/* Called by fn_1_1817C to choose or retry a participant's target model. */
s16 fn_1_16AD0(M617PlayerStateData *work)
{
    s16 selectedItems[9];
    M617LightSource *itemRecord;
    s16 targetSelected = 0;
    s16 itemIndex;
    s16 selectedCount;
    s16 retryCount = 0;
    s32 selectionRoll;
    s16 selectedIndex;

    work->selectionChanged = 0;
    itemIndex = 0;
    selectedCount = 0;
    for (; itemIndex < 9; itemIndex++) {
        itemRecord = &lbl_1_bss_458[itemIndex + 2];

        if (itemRecord->enabled != 0) {
            selectedItems[selectedCount] = itemIndex;
            selectedCount++;
        }
    }

    if (selectedCount != 0) {
        selectionRoll = frandmod(100) + 1;
    } else {
        selectionRoll = 101;
    }
    selectedIndex = selectionRoll;

    if ((work->progress >= 4) && (work->selectionDelayFrames > work->elapsedFrames)) {
        selectedIndex = 0;
    }

    if (selectedIndex <= work->targetSelectionChance) {
        selectedIndex = frandmod(selectedCount);
        work->selectionChanged = 1;
        work->selectedModel = selectedItems[selectedIndex] + 46;
        targetSelected = 1;
    } else {
        for (;;) {
            selectedIndex = frandmod(9);
            itemRecord = &lbl_1_bss_458[selectedIndex + 2];
            if (itemRecord->enabled == 0) {
                work->selectedModel = selectedIndex + 46;
                targetSelected = 1;
                break;
            }
            if (retryCount++ > work->retryLimit) {
                work->selectionChanged = 1;
                work->selectedModel = selectedIndex + 46;
                targetSelected = 1;
                break;
            }
        }
    }

    if ((lbl_1_bss_AB0[0].selectedModel == lbl_1_bss_AB0[1].selectedModel) &&
        (lbl_1_bss_AB0[0].computerControlled != 0) &&
        (lbl_1_bss_AB0[1].computerControlled != 0)) {
        /* Reject this selection when both computer players currently name the same target. */
        targetSelected = 0;
    }

    return targetSelected;
}

/* Searches the active range for the nearest unclaimed model during selection. */
s32 fn_1_16CC0(M617PlayerStateData *work)
{
    Vec difference;
    f32 distanceSquared;
    f32 distance;
    f32 closestDistance;
    s16 modelIndex;
    s16 itemIndex;
    M617LightSource *candidateItem;
    s32 modelFound;
    M617ModelSetupRecord *model;

    closestDistance = 10000.0f;
    modelFound = 0;
    if (work->progress < 4 || work->selectionDelayFrames <= work->elapsedFrames) {
        modelIndex = 46;
        itemIndex = 2;
        while (modelIndex <= 54) {
            model = &lbl_1_bss_E88[modelIndex];
            candidateItem = &lbl_1_bss_458[itemIndex];
            if (candidateItem->enabled == 0) {
                difference.x =
                    lbl_1_bss_458[work->participantIndex].position.x -
                    model->position.x;
                difference.z =
                    lbl_1_bss_458[work->participantIndex].position.z -
                    model->position.z;
                distanceSquared =
                    (difference.x * difference.x) +
                    (difference.z * difference.z);
                distance = (f32)sqrt((f64)distanceSquared);

                if (distance <= work->rangeLimits[work->rangeIndex] &&
                    closestDistance > distance) {
                    closestDistance = distance;
                    work->selectedModel = modelIndex;
                    modelFound = 1;
                }
            }
            modelIndex++;
            itemIndex++;
        }
    }
    return modelFound;
}

/* Called by fn_1_17538 to orbit the target position or nudge its X/Z coordinates while blocked. */
/* The orbit test always compares participant 0's and participant 1's headings, regardless of
 * work. */
s16 fn_1_16F88(M617PlayerStateData *work)
{
    f32 angle;
    f32 secondParticipantHeading;
    Vec delta;
    f32 distanceSquared;
    s16 orbitResult;

    orbitResult = 0;
    angle = lbl_1_bss_AB0[0].rotation.y < 0.0f ?
        360.0f + lbl_1_bss_AB0[0].rotation.y :
        lbl_1_bss_AB0[0].rotation.y;
    secondParticipantHeading = lbl_1_bss_AB0[1].rotation.y < 0.0f ?
        360.0f + lbl_1_bss_AB0[1].rotation.y :
        lbl_1_bss_AB0[1].rotation.y;
    angle = angle - secondParticipantHeading > 0.0f ?
        angle - secondParticipantHeading : -1.0f * (angle - secondParticipantHeading);
    angle -= 180.0f;

    if (((work->orbitCounter > 60 &&
          angle < 30.0f &&
          work->orbitActive == 0) ||
         (work->orbitCounter > 300 && work->orbitActive == 0))) {
        work->orbitAngleStep = 0.0f;
        work->orbitDelay = 0;
        work->orbitFrame = 0;
        work->orbitActive = 1;
        work->orbitOrigin = work->position;
    }

    if (work->orbitActive != 0) {
        f32 radius;

        delta.x = work->targetPosition.x - work->orbitOrigin.x;
        delta.z = work->targetPosition.z - work->orbitOrigin.z;
        angle = (f32)(180.0 *
            (atan2((f64)delta.x, (f64)delta.z) / 3.141592653589793));
        distanceSquared =
            (delta.x * delta.x) + (delta.z * delta.z);
        radius = (f32)sqrt((f64)distanceSquared);
        angle += work->orbitAngleStep;

        work->targetPosition.x = (f32)((f64)work->orbitOrigin.x +
            ((f64)radius * sin((3.141592653589793 * (f64)angle) /
                               180.0)));
        work->targetPosition.z = (f32)((f64)work->orbitOrigin.z +
            ((f64)radius * cos((3.141592653589793 * (f64)angle) /
                               180.0)));
        orbitResult = 1;
        if (work->orbitCounter >= 1) {
            work->orbitDelay = 0;
            work->orbitAngleStep += 4.0f;
        } else if (work->orbitCounter == 0) {
            work->orbitDelay++;
        }

        if (work->orbitDelay > (work->participantIndex * 90 + 30)) {
            orbitResult = 2;
            work->orbitActive = 0;
        } else if (work->orbitFrame++ > 300 && work->participantIndex == 1) {
            work->targetPosition.x = work->position.x;
            work->targetPosition.z = work->position.z;
        }
    } else {
        work->targetStepX = work->targetObstructed != 0 ?
            (work->targetStepX += 1.0f) : 0.0f;
        work->targetStepZ = work->targetObstructed != 0 ?
            (work->targetStepZ += 1.0f) : 0.0f;
        work->targetPosition.x += work->targetStepX;
        work->targetPosition.z += work->targetStepZ;
    }
    return orbitResult;
}

/* Called by fn_1_1817C to choose a target and follow its route points. */
s16 fn_1_17538(M617PlayerStateData *work)
{
    M617ModelSetupRecord *target;
    M617ModelSetupRecord *secondTarget;
    M617ModelSetupRecord *firstTarget;
    Vec direction;
    Vec difference;
    f32 distanceSquared;
    f32 targetDistance;
    f32 targetAngle;
    f32 currentAngle;
    s16 routeStatus;
    s16 playerMotionResult;
    s16 routeIndex;
    s16 matchingAxisCount;
    s16 matchedAxes;
    s16 candidateIndex;
    s16 candidatePoint;
    s16 variantIndex;
    s16 selectionRoll;
    s16 orbitStatus;
    s16 itemIndex;

    routeStatus = 0;
    playerMotionResult = 0;
    if (work->selectionFrames > 60) {
        work->selectionChanged = 1;
        work->routeMode = 0;
        /* Test the old index: advancing target 54 selects model 55; a later retry can wrap to
         * 46. */
        if (work->selectedModel++ > 54) {
            work->selectedModel = 46;
        }
        work->selectionFrames = 0;
    }

    target = &lbl_1_bss_E88[work->selectedModel];
    direction.x = target->position.x - work->position.x > 0.0f ?
        1.0f : -1.0f;
    direction.y = 0.0f;
    direction.z = target->position.z - work->position.z > 0.0f ?
        1.0f : -1.0f;

    switch (work->routeMode) {
    case 0:
        itemIndex = work->selectedModel - 44;
                    if (lbl_1_bss_458[itemIndex].enabled != 0 && work->selectionChanged == 0) {
            routeStatus = 2;
            break;
        }

        work->targetPosition = target->position;
        difference.x = work->targetPosition.x - work->position.x;
        difference.z = work->targetPosition.z - work->position.z;
        distanceSquared = (difference.x * difference.x) +
                          (difference.z * difference.z);
        targetDistance = (f32)sqrt((f64)distanceSquared);
        targetAngle = (f32)(180.0 *
            (atan2((f64)difference.x, (f64)difference.z) /
             3.141592653589793));
        currentAngle = work->rotation.y;

        if (((targetAngle > 90.0f) && (currentAngle < -90.0f)) ||
            ((targetAngle < -90.0f) && (currentAngle > 90.0f))) {
            targetAngle = targetAngle < 0.0f ?
                180.0f + targetAngle : 180.0f - targetAngle;
            currentAngle = currentAngle < 0.0f ?
                180.0f + currentAngle : 180.0f - currentAngle;
            targetAngle = targetAngle + currentAngle > 0.0f ?
                targetAngle + currentAngle :
                -1.0f * (targetAngle + currentAngle);
        } else {
            targetAngle = targetAngle - currentAngle > 0.0f ?
                targetAngle - currentAngle :
                -1.0f * (targetAngle - currentAngle);
        }

        if (targetDistance < (5.0f + (50.0f + lbl_1_bss_9C8.limit)) &&
            targetAngle < 40.0f) {
            work->targetPosition = work->position;
        }

        orbitStatus = fn_1_16F88(work);
        playerMotionResult = fn_1_16990(work, 0);
        if (orbitStatus == 0) {
            if (playerMotionResult != 0) {
                routeStatus = lbl_1_bss_458[itemIndex].enabled == 0 ? 1 : 2;
            } else if (work->collisionModels[0] != -1) {
                work->routeMode++;
    case 1:
                firstTarget = &lbl_1_bss_E88[work->collisionModels[0]];
                secondTarget = &lbl_1_bss_E88[work->collisionModels[1]];
                routeIndex = work->collisionModels[0] - 55;
                variantIndex = work->collisionEdges[0];
                if (lbl_1_bss_38F0[routeIndex].entries[variantIndex].state != -1) {
                    candidateIndex = 0;
                    while (candidateIndex <
                            lbl_1_bss_38F0[routeIndex].entries[variantIndex].pointCount) {
                        matchingAxisCount = 0;
                        if (lbl_1_bss_38F0[routeIndex]
                                .entries[variantIndex]
                                .directions[candidateIndex]
                                .x) {
                            if (!direction.x || lbl_1_bss_38F0[routeIndex]
                                                        .entries[variantIndex]
                                                        .directions[candidateIndex]
                                                        .x == direction.x) {
                                matchingAxisCount++;
                            }
                        } else {
                            matchingAxisCount++;
                        }
                        if (lbl_1_bss_38F0[routeIndex]
                                .entries[variantIndex]
                                .directions[candidateIndex]
                                .z) {
                            if (!direction.z || lbl_1_bss_38F0[routeIndex]
                                                        .entries[variantIndex]
                                                        .directions[candidateIndex]
                                                        .z == direction.z) {
                                matchingAxisCount++;
                            }
                        } else {
                            matchingAxisCount++;
                        }
                        if (matchingAxisCount >= 2) {
                            work->routeIndex = routeIndex;
                            work->routePoint = variantIndex;
                            work->routeElement = candidateIndex;
                            work->routeFrame = 0;
                            work->routeMode = 2;
                            break;
                        } else {
                            candidateIndex++;
                        }
                    }
                }
                if (work->routeMode != 2) {
                    work->routeMode = 0;
                }
            }
        }
        break;
    case 2:
        work->targetPosition = lbl_1_bss_38F0[work->routeIndex]
                                   .entries[work->routePoint]
                                   .positions[work->routeElement];
        playerMotionResult = fn_1_16F88(work);
        if (playerMotionResult == 2) {
            work->routeMode = 0;
        } else {
            (((work->collisionModels[0] != -1 &&
                work->collisionModels[0] == work->previousCollisionModels[0]) != 0) &&
                work->collisionEdges[0] == work->previousCollisionEdges[0]) != 0 ?
                    work->routeFrame++ : (work->routeFrame = 0);
            if (work->routeFrame > 240) {
                work->routeMode = 0;
            } else {
                playerMotionResult = fn_1_16990(work, 0);
                if (playerMotionResult == 0 && work->stageFrame > 60) {
                    playerMotionResult = 1;
                }
                if (playerMotionResult != 0) {
                    selectionRoll = frandmod(100) + 1;
                    if (selectionRoll > work->selectionThreshold) {
                        work->routeMode = 0;
                    } else {
                        work->routeStateFrame = 0;
                        work->routeMode++;
                    }
                }
            }
        }
        break;
    case 3:
        if (work->routeStateFrame++ >= work->selectionDuration) {
            work->routeMode = 0;
        }
        break;
    }
    return routeStatus;
}

/* Called by fn_1_1817C's final selection phase to update the action state. */
s16 fn_1_180E8(M617PlayerStateData *state)
{
    s16 actionComplete;

    actionComplete = 0;
    switch (state->actionPhase) {
    case 0:
        if (state->completionState == 0) {
            state->computerButtonsDown |= 1 << 8;
            state->input.buttonsDown |= 1 << 8;
        } else if (state->completionState == 1) {
            state->actionPhase++;
        }
        break;
    case 1:
        if (state->completionState == 4) {
            state->actionPhase = 0;
            actionComplete = 1;
        }
        break;
    }
    return actionComplete;
}

s16 fn_1_16AD0(M617PlayerStateData *work);

s16 fn_1_17538(M617PlayerStateData *work);

s32 frandmod(s32 limit);

/* Used by fn_1_1817C to find the nearest available model inside the active range. */
static inline s16 m617FindSelectionModel(M617PlayerStateData *work)
{
    Vec difference;
    f32 distance;
    f32 closestDistance;
    s16 modelIndex;
    s16 itemIndex;
    M617LightSource *candidateItem;
    s16 modelFound;
    M617ModelSetupRecord *model;

    closestDistance = 10000.0f;
    modelFound = 0;
    if (work->progress < 4 ||
        work->selectionDelayFrames <= work->elapsedFrames) {
        modelIndex = 46;
        itemIndex = 2;
        while (modelIndex <= 54) {
            model = &lbl_1_bss_E88[modelIndex];
            candidateItem = &lbl_1_bss_458[itemIndex];
            if (candidateItem->enabled == 0) {
                difference.x =
                    lbl_1_bss_458[work->participantIndex].position.x -
                    model->position.x;
                difference.z =
                    lbl_1_bss_458[work->participantIndex].position.z -
                    model->position.z;
                distance = (f32)sqrt(
                    (difference.x * difference.x) +
                    (difference.z * difference.z));
                if (distance <= work->rangeLimits[work->rangeIndex] &&
                    closestDistance > distance) {
                    closestDistance = distance;
                    work->selectedModel = modelIndex;
                    modelFound = 1;
                }
            }
            modelIndex++;
            itemIndex++;
        }
    }
    return modelFound;
}

/* Called by fn_1_16898 to advance target selection and the chase/action phases. */
void fn_1_1817C(M617PlayerStateData *work)
{
    s16 chaseSpeedRoll;
    s16 selectionStatus;

    selectionStatus = 0;
    if (work->selectionTimer++ > 180) {
        work->targetSelectionChance = work->targetSelectionChance > 0 ?
            work->targetSelectionChance - 1 : 0;
        work->selectionTimer = 0;
    }

    switch (work->selectionPhase) {
    case 0:
        work->selectionPhase++;
        break;
    case 1:
        selectionStatus = fn_1_16AD0(work);
        if (selectionStatus != 0) {
            chaseSpeedRoll = (s16)(frandmod(100) + 1);
            work->chaseSpeed = chaseSpeedRoll < work->chaseSpeedChance ?
                9.1f : 5.0f;
            work->routeMode = 0;
            work->selectionPhase++;
        }
        break;
    case 2:
        selectionStatus = fn_1_17538(work);
        if (selectionStatus == 1) {
            work->selectionPhase = 4;
            break;
        }
        if (selectionStatus == 2) {
            work->selectionPhase = 0;
            break;
        }

        selectionStatus = m617FindSelectionModel(work);
        if (selectionStatus != 0) {
            work->selectionPhase = 3;
        }
        break;
    case 3:
        work->chaseSpeed = 9.1f;
        selectionStatus = fn_1_17538(work);
        if (selectionStatus == 1) {
            work->selectionPhase = 4;
        } else if (selectionStatus == 2) {
            work->selectionPhase = 0;
        }
        break;
    case 4:
        selectionStatus = fn_1_180E8(work);
        if (selectionStatus != 0) {
            work->selectionPhase = 0;
        }
        break;
    }
}

#include "dolphin/types.h"
