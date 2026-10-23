/* Score boxes, per-player totals, and per-entry totals for Mole-it!. */
#include "math.h"

#include "dolphin/gx.h"

#include "game/main.h"

#include "game/object.h"

#include "game/audio.h"

#include "game/charman.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/gamework.h"

#include "game/memory.h"

#include "game/mg/seqman.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/sprite.h"

#include "game/wipe.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

#include "dolphin/types.h"

#include "REL/m641Dll/m641Dll.h"

static const float lbl_1_rodata_1C0 = 0.800000011920929f;

static const float lbl_1_rodata_1C4 = 10.0f;

GXColor lbl_1_data_480[2] = {{255, 0, 30, 0}, {0, 50, 250, 0}};
typedef struct M641ScorePositionTable {
    Point3d positions[2];
    u8 unknownTail[16];
} M641ScorePositionTable;
M641ScorePositionTable lbl_1_data_488 = {
    {{48.0f, 60.0f, 0.0f}, {48.0f, 262.0f, 0.0f}},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

/* fn_1_1433C calls this at sequence startup to create score displays and clear entry totals. */
void fn_1_13D10(void) {
    s16 index;
    s16 unusedLocal;
    index = 0;
    while (index < 2)
    {
        lbl_1_bss_7C8[index].scoreValue = 0;
        lbl_1_bss_7C8[index].boxHandle = MgScoreBoxCreate(64, 40);
        MgScoreBoxColorSet(
            lbl_1_bss_7C8[index].boxHandle,
            lbl_1_data_480[index].r, lbl_1_data_480[index].g, lbl_1_data_480[index].b);
        MgScoreBoxTPLvlSet(
            lbl_1_bss_7C8[index].boxHandle,
            lbl_1_rodata_1C0);
        MgScoreBoxPosSet(
            lbl_1_bss_7C8[index].boxHandle,
            lbl_1_data_488.positions[index].x,
            lbl_1_data_488.positions[index].y);
        MgScoreBoxDispSet(
            lbl_1_bss_7C8[index].boxHandle, 0);
        lbl_1_bss_7C8[index].score = MgScoreCreate(DATANUM(DATA_mgconst, 49), -1, 1);
        MgScoreMaxDigitSet(lbl_1_bss_7C8[index].score, 2);
        MgScoreDispOff(lbl_1_bss_7C8[index].score);
        MgScorePosSet(
            lbl_1_bss_7C8[index].score,
            lbl_1_data_488.positions[index].x - lbl_1_rodata_1C4,
            lbl_1_data_488.positions[index].y);
        MgScoreColorSet(
            lbl_1_bss_7C8[index].score, 255U,
            216U, 21U);
        MgScorePriSet(lbl_1_bss_7C8[index].score, 10);
        index += 1;
    }
    index = 0;
    while (index < 4)
    {
        lbl_1_bss_7C0[index] = 0;
        index += 1;
    }
}
/* fn_1_144B8 enables both score boxes and score readouts when scoring begins. */
void fn_1_13FC4(void) {
    s16 groupIndex;

    groupIndex = 0;
    while (groupIndex < 2) {
        MgScoreBoxDispSet(lbl_1_bss_7C8[groupIndex].boxHandle, 1);
        MgScoreDispOn(lbl_1_bss_7C8[groupIndex].score);
        groupIndex += 1;
    }
}

/* Adds hit points to group and player totals; clamps only the group total to 0 through 99. */
void fn_1_1403C(s16 groupIndex, s16 playerIndex, s32 amount) {
    if ((groupIndex >= 0) && (groupIndex < 2)) {
        lbl_1_bss_7C8[groupIndex].scoreValue += amount;
        lbl_1_bss_7C0[playerIndex] += amount;
        if (lbl_1_bss_7C8[groupIndex].scoreValue > 99) {
            lbl_1_bss_7C8[groupIndex].scoreValue = 99;
        }
        if (lbl_1_bss_7C8[groupIndex].scoreValue < 0) {
            lbl_1_bss_7C8[groupIndex].scoreValue = 0;
        }
    }
}

/* Sequence callbacks fn_1_144F8 and fn_1_1453C refresh the displayed totals. */
void fn_1_14118(void) {
    s16 groupIndex;

    groupIndex = 0;
    while (groupIndex < 2) {
        MgScoreValueSet(lbl_1_bss_7C8[groupIndex].score,
                        (s32)lbl_1_bss_7C8[groupIndex].scoreValue);
        groupIndex += 1;
    }
}

/* Returns a group total when route logic checks its score. */
s16 fn_1_14188(s16 groupIndex) {
    return lbl_1_bss_7C8[groupIndex].scoreValue;
}

/* Return one entry's accumulated total when board logic checks it. */
s16 fn_1_141A4(s16 index) {
    return lbl_1_bss_7C0[index];
}

/* fn_1_14598 selects the winner at round end; return -1 when the scores tie. */
s32 fn_1_141C0(void) {
    if (lbl_1_bss_7C8[0].scoreValue > lbl_1_bss_7C8[1].scoreValue) {
        return 0;
    }
    if (lbl_1_bss_7C8[0].scoreValue < lbl_1_bss_7C8[1].scoreValue) {
        return 1;
    }
    return -1;
}

M641ScoreEntry lbl_1_bss_7C8[2];
s16 lbl_1_bss_7C0[4];
