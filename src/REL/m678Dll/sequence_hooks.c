/* Block Star: player controls, linked-piece simulation, game sequences, and rendering. */
#include <math.h>

#include "dolphin/types.h"

#include "dolphin/os.h"

#include "dolphin/gx.h"

#include "dolphin/mtx.h"

#include "string.h"

#include "game/main.h"

#include "game/hsfex.h"

#include "game/gamework.h"

#include "game/object.h"

#include "game/pad.h"

#include "REL/m678Dll/player_work.h"

#include "REL/m678Dll/link_entry.h"

#include "dolphin.h"

#include "humath.h"

#include "datadir_enum.h"

#include "messdir_enum.h"

#include "game/hu3d.h"

#include "game/audio.h"

#include "game/sprite.h"

#include "game/memory.h"

#include "game/data.h"

#include "game/window.h"

#include "game/wipe.h"

#include "game/frand.h"

#include "game/charman.h"

#include "game/process.h"

#include "game/gamemes.h"

#include "game/flag.h"

#include "game/armem.h"

#include "game/mgdata.h"

#include "game/printfunc.h"

#include "game/saveload.h"

#include "game/esprite.h"

#include "game/thpmain.h"

#include "game/mg/seqman.h"

#include "game/mg/actman.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/board/object.h"

#include "game/board/main.h"

#include "game/board/audio.h"

#include "game/board/tutorial.h"

#include "game/board/masu.h"

#include "game/board/player.h"

#include "REL/m678Dll.h"

#include "REL/m678Dll/link_group.h"

#include "REL/m678Dll/render.h"
#include "REL/m678Dll/shared_types.h"                                                  /* size = 0x8 */

/* 0.0f */

/* Unidentified constant bytes, .rodata 0x10..0x84. */
const u8 lbl_1_rodata_10[0x74] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x1E,
    0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x46,
    0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x5A, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0xC8,
    0x00, 0x00, 0x01, 0x2C, 0x00, 0x00, 0x01, 0x90, 0x00, 0x00, 0x01, 0xF4, 0x00, 0x00, 0x02, 0x58,
    0x00, 0x00, 0x02, 0xBC, 0x00, 0x00, 0x03, 0x20, 0x00, 0x00, 0x03, 0x84, 0x00, 0x00, 0x03, 0xE8,
    0x00, 0x00, 0x07, 0xD0, 0x00, 0x00, 0x0B, 0xB8, 0x00, 0x00, 0x0F, 0xA0, 0x00, 0x00, 0x13, 0x88,
    0x00, 0x00, 0x17, 0x70, 0x00, 0x00, 0x1B, 0x58, 0x00, 0x00, 0x1F, 0x40, 0x00, 0x00, 0x23, 0x28,
    0x00, 0x00, 0x27, 0x10
};

float fn_1_0(float x, float y)
{
    return sqrtf(x * x + y * y);
}

void fn_1_134(float *x, float *y, float maximum)
{
    float length = fn_1_0(*x, *y);
    if (length > maximum) {
        *x = maximum * (*x / length);
        *y = maximum * (*y / length);
    }
}

void fn_1_2DC(int *x, int *y, float maximum)
{
    float length = fn_1_0(*x, *y);

    if (length > maximum) {
        *x = maximum * (*x / length);
        *y = maximum * (*y / length);
    }
}

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];

extern const VoidFunc _dtors[];

extern void fn_1_129D0(void);

int _prolog(void)
{
    const VoidFunc *ctor = _ctors;

    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }

    fn_1_129D0();
    return 0;
}

void _epilog(void)
{
    const VoidFunc *dtor = _dtors;

    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

void fn_1_5CC(void)
{
    int zoomIn;

    lbl_80266A4C.y += 20.0 * ((double)(lbl_802BFFEC[0] - lbl_802BFFE8[0]) / 128.0);
    zoomIn = 0;
    if ((lbl_802C0010[0] & PAD_BUTTON_Y) != 0 && lbl_802C028C > 25.0f) {
        zoomIn = 1;
    }
    lbl_802C028C = (float)((double)lbl_802C028C
        + 25.0 * (double)(((lbl_802C0010[0] & PAD_BUTTON_X) != 0) - zoomIn));
}

void fn_1_70C(float *x, float *y, float *z)
{
    HuVecF source;
    HuVecF projected;

    source.x = *x;
    source.y = *y;
    source.z = *z;
    Hu3D3Dto2D(&source, 1, &projected);
    *x = projected.x;
    *y = projected.y;
    *z = projected.z;
}

struct M678LinkGroup {
    s32 index;
    struct M678LinkGroup *parent;
    s32 count;
    u8 unk_0C[8]; /* No code names this field. */
    struct M678LinkEntry *firstEntry;
    struct M678LinkEntry *lastEntry;
};

extern OMOBJ *lbl_1_bss_15C[4];

extern struct _struct_lbl_1_bss_2D0_0x8 lbl_1_bss_2D0[2];

extern struct M678LinkGroup lbl_1_bss_2E0[2][2][0x66];

extern struct M678LinkEntry lbl_1_bss_70A8[2][0x66];

typedef struct M678SoundEntry {
    s32 count;
    s32 handle;
} M678SoundEntry;

extern s32 lbl_1_bss_0;

extern s32 lbl_1_bss_154;

extern struct M678SoundEntry lbl_1_bss_174[0x26];

extern u32 lbl_1_bss_66F0[2];

extern f32 lbl_1_bss_66F8[2][102];

extern f32 lbl_1_bss_6A28[2][102];

extern f32 lbl_1_bss_6D58[2];

extern f32 lbl_1_bss_6D60[2];

extern f32 lbl_1_bss_6D68[2];

extern f32 lbl_1_bss_6D70[2];

extern struct M678LinkEntry *lbl_1_bss_6D78[2][102];

extern s32 lbl_1_bss_8;

extern u32 lbl_1_bss_C;

extern s32 lbl_1_data_130[2];

extern s32 lbl_1_data_80[2];

extern s32 lbl_1_data_88[2];

extern s32 lbl_1_data_90[2];

extern s32 lbl_1_data_A8[2];

extern s32 lbl_1_data_B0[2];

extern s32 lbl_1_data_C0[2];

extern s32 lbl_1_data_F0[2];

typedef struct M678ActionDurations { s32 value[5]; } M678ActionDurations;

/* Scores candidate grid placements and chooses an offset for the selected group. */
s32 fn_1_78C(s32 player, void *gridOutput, s32 pass, s32 groupIndex, s32 *offsetOutput) {
    struct M678LinkEntry *displacedEntries[30];
    int candidateColumn;
    int firstColumn;
    int lastColumn;
    int bestScore;
    int bestOffset;
    int score;
    int preservesOrder;
    M678PlayerSetupWork *playerWork;
    struct M678LinkEntry *previousEntry;
    struct M678LinkEntry *rowEntry;
    int displacedCount;
    int foundGroupEntry;
    int fixedGroup;
    struct M678LinkEntry *resetEntry;
    struct M678LinkEntry *movingEntry;
    int destinationRow;
    struct M678LinkGroup *newGroup;
    struct M678LinkEntry *gridEntry;
    struct M678LinkEntry *firstScoredEntry;
    int scoreBonus;
    int groupScore;
    int height;
    struct M678LinkEntry *targetEntry;
    int isFixedGroup;
    int bothGroupsMissing;
    struct M678LinkGroup *selectedGroup;
    struct M678LinkGroup *existingGroup;
    int groupValue;
    int smallGroupValue;
    int heightInRange;
    int heightValue;
    int lowHeightValue;
    int highHeightValue;
    int elevenHeightValue;
    int tenHeightValue;
    int moderateHeightValue;
    f32 sortPosition;
    int index;
    int row;
    int column;
    struct M678LinkEntry *nextEntry;
    struct M678LinkEntry *currentRowEntry;
    struct M678LinkGroup *scoredGroup;
    struct M678LinkGroup *mergedGroup;
    struct M678LinkEntry *currentEntry;
    struct M678LinkEntry *neighborEntry;
    struct M678LinkEntry *entry;
    struct M678LinkEntry *groupEntry;
    struct M678LinkGroup *neighborGroup;
    struct M678LinkGroup *currentGroup;
    {
        struct M678LinkEntry *grid[6][30] = {{NULL}};
        struct M678LinkEntry *savedGrid[6][30] = {{NULL}};
        struct M678LinkEntry *rowHeads[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
        /* Initialized words with no subsequent access. */
        u32 unknownWords0[0xB4] = {0};
        struct { s32 words[2]; } unusedPairs[3] = {0};
        struct { s32 words[2]; } unusedOtherPairs[3] = {0};
        int rowSlots[6];
        int candidateSlots[6];
        /* Initialized words with no subsequent access. */
        u32 unknownWords1[0x198] = {0};
    firstColumn = 0;
    lastColumn = 0;
    bestScore = -0xC350;
    bestOffset = -0x64;
    preservesOrder = 0;
    playerWork = lbl_1_bss_15C[player]->data;
    lbl_1_bss_2D0[player].counts[pass] = 0;
    while ((pass == 1) && ((u32) lbl_1_bss_2E0[player][0][groupIndex].parent != 0U)) {
        groupIndex = lbl_1_bss_2E0[player][0][groupIndex].parent->index;
    }
    if (pass == 1) {
        groupEntry = lbl_1_bss_2E0[player][0][groupIndex].firstEntry;
        if (((s32) groupEntry->kind == 2) || ((s32) groupEntry->kind == 6) || ((s32) groupEntry->kind == 7)) {
            return -0x30D40;
        }
        firstColumn = 6;
        lastColumn = -1;
        while (groupEntry) {
            firstColumn = (f32)firstColumn > groupEntry->gridRow ? groupEntry->gridRow : (f32)firstColumn;
            lastColumn = (f32)lastColumn < groupEntry->gridRow ? groupEntry->gridRow : (f32)lastColumn;
            groupEntry = groupEntry->nextInGroup[0];
        }
        goto initializeEntryGroups;
    }
initializeEntryGroups:
    index = 0;
    while (index < 0x66) {
        entry = &lbl_1_bss_70A8[player][index];
        nextEntry = NULL;
        previousEntry = NULL;
        entry->group[pass] = 0;
        entry->previousInGroup[pass] =
            entry->nextInGroup[pass] = 0;
        if (pass == 1) {
            if (((s32) entry->kind != 0) && (!rowHeads[(s32) (0.5f + entry->gridRow)])) {
                rowEntry = &lbl_1_bss_70A8[player][index];
                while (rowEntry) {
                    rowHeads[(s32) (0.5f + entry->gridRow)] = rowEntry;
                    rowEntry = rowEntry->previousInRow;
                }
            }
        } else {
            entry->nextInRow = entry->previousInRow = NULL;
            if (((s32) entry->kind != 0) && ((s32) entry->category != 5) && ((s32) entry->fadeTimer <= 0)) {
                sortPosition = entry->category == 6 ? entry->fallSpeed - 1000.0f : entry->gridColumn;
                nextEntry = rowHeads[(s32) entry->gridRow];
                while (nextEntry) {
                    if (!(nextEntry->gridColumn > sortPosition)) {
                        break;
                    }
                    previousEntry = nextEntry;
                    nextEntry = nextEntry->nextInRow;
                }
                entry->nextInRow = nextEntry;
                if (nextEntry) {
                    nextEntry->previousInRow = entry;
                }
                entry->previousInRow = previousEntry;
                if (previousEntry) {
                    previousEntry->nextInRow = entry;
                }
                if (!previousEntry) {
                    rowHeads[(s32) entry->gridRow] = entry;
                }
            }
        }
        index += 1;
    }
    preservesOrder = 1;
    index = 0;
    while (index < 6) {
        displacedCount = 0;
        foundGroupEntry = 0;
        currentRowEntry = rowHeads[index];
        (&rowSlots[0])[index] = 0x1D;
        while (((s32) (&rowSlots[0])[index] > 0) && currentRowEntry) {
            if ((pass == 1) && currentRowEntry->group[0] && currentRowEntry->group[0]->index == groupIndex) {
                (&displacedEntries[0])[displacedCount] = currentRowEntry;
                displacedCount += displacedCount < 0x1E;
                currentRowEntry = currentRowEntry->nextInRow;
                foundGroupEntry = 1;
            } else {
                grid[index][rowSlots[index]] = currentRowEntry;
                (&rowSlots[0])[index] -= (&rowSlots[0])[index] > 0;
                if (foundGroupEntry != 0) {
                    preservesOrder = 0;
                }
                currentRowEntry = currentRowEntry->nextInRow;
            }
        }
        index += 1;
    }
    if (pass == 1) {
        memcpy(&savedGrid, &grid, 0x2D0U);
    }
    if (pass == 0) {
        isFixedGroup = 0;
    } else {
        isFixedGroup = lbl_1_bss_2E0[player][0][groupIndex].firstEntry->kind == 4;
    }
    fixedGroup = isFixedGroup;
    for (candidateColumn = 0; candidateColumn < ((pass == 0) ? 1 : ((fixedGroup != 0) ? 1 : 6 - (lastColumn - firstColumn))); candidateColumn++) {
        if (pass == 1) {
            index = 0;
            while (index < 0x66) {
                resetEntry = &lbl_1_bss_70A8[player][index];
                lbl_1_bss_2D0[player].counts[pass] = 0;
                resetEntry->group[pass] = 0;
                resetEntry->previousInGroup[pass] =
                    resetEntry->nextInGroup[pass] = 0;
                index += 1;
            }
        }
        if (pass == 1) {
            movingEntry = lbl_1_bss_2E0[player][0][groupIndex].firstEntry;
            memcpy(&grid, &savedGrid, 0x2D0U);
            memcpy(&candidateSlots[0], &rowSlots[0], 0x18U);
            if ((fixedGroup == 0) != 0) {
            while (movingEntry) {
                destinationRow = (s32) (0.5f + movingEntry->gridRow);
                destinationRow = candidateColumn + (destinationRow - firstColumn);
                grid[destinationRow][candidateSlots[destinationRow]] = movingEntry;
                (&candidateSlots[0])[destinationRow] -= (&candidateSlots[0])[destinationRow] > 0;
                movingEntry = movingEntry->nextInGroup[0];
            }
            if ((preservesOrder == 0) || (candidateColumn != firstColumn)) {
                goto buildCandidateGroups;
            }
                        continue;
            }
        }
        {
buildCandidateGroups:
            row = 0;
            while (row < 6) {
                column = -0xF;
                while (column < 0xF) {
                    if (grid[row][column + 0xF]) {
                        if ((!grid[row][column + 0xF]->group[pass]) && ((pass != 0) || (column != 0xE))) {
                            newGroup = &lbl_1_bss_2E0[player][pass][lbl_1_bss_2D0[player].counts[pass]];
                            gridEntry = grid[row][column + 0xF];
                            newGroup->parent = 0;
                            newGroup->count = 1;
                            newGroup->firstEntry = gridEntry;
                            newGroup->lastEntry = gridEntry;
                            lbl_1_bss_2D0[player].counts[pass] += lbl_1_bss_2D0[player].counts[pass] < 0x66;
                            gridEntry->previousInGroup[pass] =
                                gridEntry->nextInGroup[pass] = 0;
                            gridEntry->group[pass] = newGroup;
                        }
                        index = 0;
                        while (index < 2) {
                            int rowSteps[2] = { 1, 0 };
                            int columnSteps[2] = { 0, 1 };
                            if (((s32) lbl_1_bss_2D0[player].counts[pass] < 0x66) && ((s32) (row + rowSteps[index]) >= 0) && ((s32) (row + rowSteps[index]) < 6) && ((s32) (column + columnSteps[index]) >= -0xF) && ((s32) (column + columnSteps[index]) < 0xF) && grid[row + rowSteps[index]][column + columnSteps[index] + 0xF]) {
                                currentEntry = grid[row][column + 0xF];
                                neighborEntry = grid[row + rowSteps[index]][column + columnSteps[index] + 0xF];
                                if (((s32) currentEntry->variant != 0) && ((s32) neighborEntry->variant != 0) && ((s32) currentEntry->variant == (s32) neighborEntry->variant) && ((pass == 1) || (((s32) currentEntry->kind == 1) && ((s32) neighborEntry->kind == 1))) && (((u32) currentEntry->group[pass] == 0U) || ((u32) neighborEntry->group[pass] == 0U) || ((u32) currentEntry->group[pass] != (u32) neighborEntry->group[pass]))) {
                                    currentGroup = currentEntry->group[pass];
                                    neighborGroup = neighborEntry->group[pass];
                                    while (currentGroup && currentGroup->parent) {
                                        currentGroup = currentGroup->parent;
                                    }
                                    while (neighborGroup && neighborGroup->parent) {
                                        neighborGroup = neighborGroup->parent;
                                    }
                                    bothGroupsMissing = 0;
                                    if ((!currentGroup) && (!neighborGroup)) {
                                        bothGroupsMissing = 1;
                                    }
                                    if (bothGroupsMissing != 0) {
                                        selectedGroup = NULL;
                                    } else {
                                        if (!currentGroup) {
                                            existingGroup = neighborGroup;
                                        } else {
                                            existingGroup = currentGroup;
                                        }
                                        selectedGroup = existingGroup;
                                    }
                                    mergedGroup = selectedGroup;
                                    if (currentGroup && neighborGroup && (currentGroup != neighborGroup)) {
                                        neighborGroup->parent = currentGroup;
                                        currentGroup->lastEntry->nextInGroup[pass] = neighborGroup->firstEntry;
                                        neighborGroup->firstEntry->previousInGroup[pass] = currentGroup->lastEntry;
                                        currentGroup->lastEntry = neighborGroup->lastEntry;
                                        currentGroup->count = (s32) (currentGroup->count + neighborGroup->count);
                                        neighborGroup->count = 0;
                                    } else if (!mergedGroup) {
                                        if (pass == 1) {
                                            mergedGroup = &lbl_1_bss_2E0[player][pass][lbl_1_bss_2D0[player].counts[pass]];
                                            mergedGroup->parent = 0;
                                            mergedGroup->count = 2;
                                            mergedGroup->firstEntry = currentEntry;
                                            mergedGroup->lastEntry = neighborEntry;
                                            lbl_1_bss_2D0[player].counts[pass] += lbl_1_bss_2D0[player].counts[pass] < 0x66;
                                            currentEntry->nextInGroup[pass] = neighborEntry;
                                            neighborEntry->previousInGroup[pass] = currentEntry;
                                            currentEntry->group[pass] =
                                                neighborEntry->group[pass] = mergedGroup;
                                        }
                                    } else if (mergedGroup == currentGroup) {
                                        currentGroup->count = (s32) (currentGroup->count + 1);
                                        currentGroup->lastEntry->nextInGroup[pass] = neighborEntry;
                                        neighborEntry->previousInGroup[pass] = currentGroup->lastEntry;
                                        neighborEntry->group[pass] = currentGroup;
                                        currentGroup->lastEntry = neighborEntry;
                                    } else if (mergedGroup == neighborGroup) {
                                        neighborGroup->count = (s32) (neighborGroup->count + 1);
                                        neighborGroup->lastEntry->nextInGroup[pass] = currentEntry;
                                        currentEntry->previousInGroup[pass] = neighborGroup->lastEntry;
                                        currentEntry->group[pass] = neighborGroup;
                                        neighborGroup->lastEntry = currentEntry;
                                    }
                                }
                            }
                            index += 1;
                        }
                    }
                    column += 1;
                }
                row += 1;
            }
            if (pass == 1) {
                score = 0;
                index = 0;
                while (index < (s32) lbl_1_bss_2D0[player].counts[1]) {
                    scoredGroup = &lbl_1_bss_2E0[player][1][index];
                    scoreBonus = 0;
                    if ((u32) scoredGroup->parent == 0U) {
                        firstScoredEntry = scoredGroup->firstEntry;
                        if ((s32) scoredGroup->count >= 5) {
                            groupValue = (scoredGroup->count * 0x14) + 0x96 + (scoreBonus * 0x96);
                        } else {
                            if ((s32) scoredGroup->count <= 1) {
                                smallGroupValue = 0;
                            } else {
                                smallGroupValue = (scoredGroup->count * 0x14) + (scoreBonus * 5);
                            }
                            groupValue = smallGroupValue;
                        }
                        score = score + groupValue;
                    }
                    index += 1;
                }
                groupScore = score;
                index = 0;
                while (index < 6) {
                    height = 0x1D - (&candidateSlots[0])[index];
                    heightInRange = 0;
                    if ((height >= -4) && (height <= 6)) {
                        heightInRange = 1;
                    }
                    if (heightInRange != 0) {
                        heightValue = 0x12C;
                    } else {
                        if (height < 4) {
                            lowHeightValue = 0x12C - ((4 - height) * 5);
                        } else {
                            if (height > 0xB) {
                                highHeightValue = -0x5DC - ((height - 0xB) * 0x1F4);
                            } else {
                                if (height > 0xA) {
                                    elevenHeightValue = -0x3E8;
                                } else {
                                    if (height > 9) {
                                        tenHeightValue = -0x12C;
                                    } else {
                                        if (height > 6) {
                                            moderateHeightValue = 0x12C - ((height - 6) * 0xA);
                                        } else {
                                            moderateHeightValue = 0;
                                        }
                                        tenHeightValue = moderateHeightValue;
                                    }
                                    elevenHeightValue = tenHeightValue;
                                }
                                highHeightValue = elevenHeightValue;
                            }
                            lowHeightValue = highHeightValue;
                        }
                        heightValue = lowHeightValue;
                    }
                    score = score + heightValue;
                    index += 1;
                }
                if (((u32) lbl_1_bss_2E0[player][0][groupIndex].firstEntry != 0U) && ((s32) lbl_1_bss_2E0[player][0][groupIndex].firstEntry->kind == 4)) {
                    targetEntry = lbl_1_bss_2E0[player][0][groupIndex].firstEntry;
                    score = (s32) ((f64)score + (10.0 - fabs(playerWork->gridRow - targetEntry->gridRow) - fabs(playerWork->gridColumn - targetEntry->gridColumn)));
                }
                if (bestScore < score) {
                    bestOffset = -firstColumn + candidateColumn;
                    bestScore = score;
                }
            }
        }
    }
    if (gridOutput) {
        memcpy(gridOutput, &grid, 0x2D0U);
    }
    if (offsetOutput) {
        *offsetOutput = bestOffset;
    }
    return bestScore;
    }
}

/* Updates player movement, computer choices, grabbing, and linked-piece release. */
void fn_1_1BC8(OMOBJ *obj) {
    void *connectedEntries[102];
    s32 visitedDirections[102];
    void *playerWorkList[4];
    s32 connectedIndex;
    s32 buttonAHeld;
    s32 *playerIndexPointer;
    s32 buttonBHeld;
    s32 buttonAPressed;
    s32 buttonBPressed;
    s32 horizontalAction;
    s32 verticalAction;
    s32 unusedInputState;
    s32 unusedPlayerIndex;
    s32 rowCount;
    s32 columnCount;
    f32 stickMagnitude;
    s32 bestGroup;
    s32 candidateScore;
    s32 bestScore;
    s32 bestOffset;
    s32 candidateOffset;
    s32 nearestDistance;
    s32 rowSteps;
    s32 columnSteps;
    s32 candidateDistanceScore;
    M678LinkEntry *carriedEntry;
    s32 foundNeighbor;
    s32 connectedCount;
    s32 connectedEntryIndex;
    s32 directionIndex;
    struct M678LinkEntry *nearestEntry;
    s32 fallingRow;
    s32 unusedReleaseState;
    s32 carriedRow;
    f32 releaseSpeed;
    f32 fallFrames;
    f32 fallingColumn;
    f32 rowMovement;
    f32 columnMovement;
    f32 carriedOverlap;
    f32 fallDiscriminant;
    f32 columnInput;
    f32 rowInput;
    s32 workIndex;
    s32 entryIndex;
    s32 playerIndex;
    M678LinkEntry *neighborEntry;
    M678LinkEntry *impactEntry;
    M678LinkEntry *heldEntry;
    M678LinkEntry *releasedEntry;
    M678LinkEntry *grabCandidate;
    M678PlayerSetupWork *playerWork;
    M678ChainGroup *rootGroup;
    M678LinkEntry *candidateEntry;
    M678LinkEntry *grabbedEntry;
    M678LinkEntry *groupEntry;

    playerIndexPointer = obj->data;
    rowInput = 0.0f;
    columnInput = 0.0f;
    buttonAHeld = 0;
    buttonBHeld = 0;
    buttonAPressed = 0;
    buttonBPressed = 0;
    horizontalAction = 0;
    verticalAction = 0;
    unusedInputState = 0;
    unusedPlayerIndex = *playerIndexPointer;
    if ((s32) *playerIndexPointer == 0) {
        lbl_1_bss_154 = fn_80071A8C();
        lbl_1_bss_C = fn_80071DE4();
        for (entryIndex = 0; entryIndex < lbl_1_bss_8; entryIndex++) {
            playerWorkList[entryIndex] = lbl_1_bss_15C[entryIndex]->data;
        }
        playerIndex = 0;
        while (playerIndex < lbl_1_bss_8) {
            playerWork = playerWorkList[playerIndex];
            rowCount = 6;
            columnCount = 0xF;
            rowInput = 0.0f;
            columnInput = 0.0f;
            buttonAHeld = 0;
            buttonBHeld = 0;
            buttonAPressed = 0;
            buttonBPressed = 0;
            horizontalAction = 0;
            verticalAction = 0;
            unusedInputState = 0;
            if (((lbl_1_bss_0 == 2) || (lbl_1_bss_0 == 5)) && ((s32) lbl_1_data_F0[playerIndex] > 0)) {
                if ((s32) playerWork->padNo != -1) {
                    rowInput = lbl_8024C7D8[playerWork->padNo];
                    columnInput = -lbl_8024C7E8[playerWork->padNo];
                    stickMagnitude = sqrtf((rowInput * rowInput) + (columnInput * columnInput));
                    if (stickMagnitude < 0.75f) {
                        if (fabs(rowInput) < fabs(columnInput)) {
                            rowInput = 0.0f;
                        } else {
                            columnInput = 0.0f;
                        }
                    }
                    rowInput = (f32)(rowInput < -0.2f ? -1 : rowInput > 0.2f ? 1 : 0);
                    columnInput = (f32)(columnInput < -0.2f ? -1 : columnInput > 0.2f ? 1 : 0);
                    if ((rowInput == 0.0f) && (columnInput == 0.0f)) {

                        rowInput = (f32) (((s32) (lbl_802C0010[playerWork->padNo] & PAD_BUTTON_RIGHT) != 0 ? 1 : 0) - ((s32) (lbl_802C0010[playerWork->padNo] & PAD_BUTTON_LEFT) != 0 ? 1 : 0));

                        columnInput = (f32) (((s32) (lbl_802C0010[playerWork->padNo] & PAD_BUTTON_DOWN) != 0 ? 1 : 0) - ((s32) (lbl_802C0010[playerWork->padNo] & PAD_BUTTON_UP) != 0 ? 1 : 0));
                    }
                    buttonAHeld = lbl_802C0010[playerWork->padNo] & PAD_BUTTON_A;
                    buttonBHeld = lbl_802C0010[playerWork->padNo] & PAD_BUTTON_B;
                    buttonAPressed = lbl_802C0008[playerWork->padNo] & PAD_BUTTON_A;
                    buttonBPressed = lbl_802C0008[playerWork->padNo] & PAD_BUTTON_B;
                    horizontalAction = (lbl_802C0008[playerWork->padNo] & (PAD_BUTTON_A | PAD_BUTTON_X)) | (lbl_8024C7F8[playerWork->padNo] < -0.5f || lbl_8024C7F8[playerWork->padNo] > 0.5f);
                    verticalAction = (lbl_802C0008[playerWork->padNo] & (PAD_BUTTON_B | PAD_BUTTON_Y)) | (lbl_8024C808[playerWork->padNo] < -0.5f || lbl_8024C808[playerWork->padNo] > 0.5f);
                } else if ((s32) playerWork->padNo == -1) {
                    /* Initialized words with no subsequent access. */
                    u32 unknownWords2[4] = {0x14, 0x14, 0xA, 0};

                    s32 normalWaitByDifficulty[4] = {90, 60, 40, 10};

                    s32 maximumWaitByDifficulty[4] = {90, 60, 50, 20};

                    s32 holdWaitByDifficulty[4] = {30, 20, 16, 13};

                    s32 releaseWaitByDifficulty[4] = {30, 15, 10, 10};

                    switch (playerWork->computerState) {              /* switch 1; irregular */
                    case 0:                         /* switch 1 */
                        playerWork->computerWaitFrames = (s32) (playerWork->computerWaitFrames + 1);
                        entryIndex = 0;
                        workIndex = 0;
                        while (entryIndex < 0x66) {
                            workIndex += lbl_1_bss_70A8[playerIndex][entryIndex].kind != 0 && lbl_1_bss_70A8[playerIndex][entryIndex].fadeTimer == 0;
                            entryIndex += 1;
                        }
                        if (workIndex < 0x18) {
                            buttonBHeld = 1;
                        } else if ((((s32) lbl_1_data_C0[playerIndex] == 0) && ((s32) playerWork->computerWaitFrames >= (s32) normalWaitByDifficulty[playerWork->difficulty])) || ((s32) playerWork->computerWaitFrames >= (s32) maximumWaitByDifficulty[playerWork->difficulty])) {
                            fn_1_78C(playerIndex, NULL, 0, -1, NULL);
                            playerWork->computerState = 1;
                            playerWork->computerWaitFrames = 0;
                        }
                        break;
                    case 1:                         /* switch 1 */
                        bestGroup = -1;
                        bestScore = -0x186A0;
                        candidateOffset = -1;
                        entryIndex = 0;
                        while (entryIndex < (s32) lbl_1_bss_2D0[playerIndex].counts[0]) {
                            if (((u32) lbl_1_bss_2E0[playerIndex][0][entryIndex].parent == 0U) && ((s32) lbl_1_bss_2E0[playerIndex][0][entryIndex].firstEntry->fadeTimer <= 0) && (((s32) lbl_1_bss_2E0[playerIndex][0][entryIndex].firstEntry->kind == 1) || ((s32) lbl_1_bss_2E0[playerIndex][0][entryIndex].firstEntry->kind == 4))) {
                                candidateScore = fn_1_78C(playerIndex, NULL, 1, entryIndex, &candidateOffset);
                                if ((candidateScore + rand8() % 2) > bestScore) {
                                    bestGroup = entryIndex;
                                    bestScore = candidateScore;
                                    bestOffset = candidateOffset;
                                }
                            }
                            entryIndex += 1;
                        }
                        if (bestGroup != -1) {
                            nearestDistance = 0x3E8;
                            playerWork->computerState = 2;
                            playerWork->computerWaitFrames = 0;
                            playerWork->computerRowOffset = bestOffset;
                            candidateEntry = lbl_1_bss_2E0[playerIndex][0][bestGroup].firstEntry;
                            while (candidateEntry) {
                                rowSteps = (s32) fabs(candidateEntry->gridRow - playerWork->gridRow);
                                columnSteps = (s32) fabs(candidateEntry->gridColumn - playerWork->gridColumn);
                                candidateDistanceScore = (rowSteps == 0 ? 0 : rowSteps + 1) + (columnSteps == 0 ? 0 : columnSteps + 1);
                                if ((candidateDistanceScore < nearestDistance) && (candidateEntry->gridColumn < 14.0f)) {
                                    playerWork->computerTarget = candidateEntry;
                                    nearestDistance = candidateDistanceScore;
                                }
                                candidateEntry = candidateEntry->nextInGroup[0];
                            }
                        } else {
                            playerWork->computerState = 0;
                            playerWork->computerWaitFrames = 0;
                        }
                        break;
                    case 2:                         /* switch 1 */
                        if (((s32) playerWork->computerTarget->kind == 0) || ((s32) playerWork->computerTarget->fadeTimer > 0) || (((s32) playerWork->computerTarget->kind != 1) && ((s32) playerWork->computerTarget->kind != 4)) || (playerWork->computerTarget->gridColumn >= 14.0f) || (playerWork->computerTarget->gridRow < 0.0f) || (playerWork->computerTarget->gridRow >= 6.0f)) {
                            playerWork->computerState = 0;
                            playerWork->computerWaitFrames = 0;
                        } else if ((s32) playerWork->computerTarget->category == 0) {
                            playerWork->computerWaitFrames = (s32) (playerWork->computerWaitFrames + 1);
                            rowInput = (f32) (s32) (playerWork->computerTarget->gridRow - playerWork->gridRow);
                            columnInput = (f32) (s32) (playerWork->computerTarget->gridColumn - playerWork->gridColumn);
                            rowInput = rowInput < 0.0f ? -1.0f : rowInput > 0.0f ? 1.0f : rowInput;
                            columnInput = columnInput < 0.0f ? -1.0f : columnInput > 0.0f ? 1.0f : columnInput;
                            if ((rowInput == 0.0f) && (columnInput == 0.0f) && ((s32) playerWork->computerTarget->category == 0)) {
                                buttonAPressed = buttonAHeld = 1;
                                playerWork->computerState = 3;
                                playerWork->computerWaitFrames = 0;
                                playerWork->computerDestinationRow = -1;
                            }
                        }
                        break;
                    case 3:                         /* switch 1 */
                        playerWork->computerWaitFrames = (s32) (playerWork->computerWaitFrames + 1);
                        if ((s32) lbl_1_data_A8[playerIndex] == 0) {
                            playerWork->computerState = 2;
                            playerWork->computerWaitFrames = 0x64;
                        } else {
                            buttonAHeld = 1;
                            if ((s32) playerWork->computerWaitFrames >= (s32) holdWaitByDifficulty[playerWork->difficulty]) {
                                if ((s32) playerWork->computerDestinationRow == -1) {
                                    playerWork->computerDestinationRow = (s32) (playerWork->gridRow + (f32) playerWork->computerRowOffset);
                                }
                                rowInput = (f32) (s32) ((f32) playerWork->computerDestinationRow - playerWork->gridRow);
                                rowInput = rowInput < 0.0f ? -1.0f : rowInput > 0.0f ? 1.0f : rowInput;
                if ((((playerWork->gridRow + lbl_1_bss_6D70[playerIndex]) <= 0.0f) && (rowInput < 0.0f)) || (((playerWork->gridRow + lbl_1_bss_6D68[playerIndex]) <= 6.0f) && (rowInput > 0.0f)) || (rowInput == 0.0f)) {
                                    playerWork->computerState = 4;
                                    playerWork->computerWaitFrames = 0;
                                }
                            }
                        }
                        break;
                    case 4:                         /* switch 1 */
                        playerWork->computerWaitFrames = (s32) (playerWork->computerWaitFrames + 1);
                        if ((s32) lbl_1_data_A8[playerIndex] == 0) {
                            playerWork->computerState = 2;
                            playerWork->computerWaitFrames = 0x64;
                        } else if ((s32) playerWork->computerWaitFrames < (s32) releaseWaitByDifficulty[playerWork->difficulty]) {
                            buttonAHeld = 1;
                        } else {
                            playerWork->computerState = 0;
                            playerWork->computerWaitFrames = 0;
                        }
                        break;
                    }
                }
            }
            if ((buttonBHeld != 0) && ((s32) lbl_1_data_80[playerIndex] == 0) && ((u32) lbl_1_bss_66F0[playerIndex] == 0)) {
                lbl_1_data_80[playerIndex] = 2;
                lbl_1_data_88[playerIndex] = 0;
                if (((s32) lbl_1_data_88[playerIndex] == 0) && ((s32) lbl_1_data_90[playerIndex] > 1) && ((s32) lbl_1_bss_174[playerIndex + 0x1E].count < 3)) {
                    lbl_1_bss_174[playerIndex + 0x1E].count += 1;
                }
            }
            if ((((s32) playerWork->state[6] == 3) || ((s32) playerWork->state[6] == 4)) && ((s32) playerWork->state[7] <= 0xF)) {
                rowInput = columnInput = buttonAHeld = buttonAPressed = 0;
            }
            rowMovement = rowInput <= -0.5f && playerWork->gridRow > 0 ? -1 : (rowInput >= 0.5f && playerWork->gridRow < (f32)(rowCount - 1) ? 1 : 0);
            columnMovement = columnInput <= -0.5f && playerWork->gridColumn > 0 ? -1 : (columnInput >= 0.5f && playerWork->gridColumn < (f32)(columnCount - 1) ? 1 : 0);
            playerWork->state[2] = ((f32) playerWork->state[0] == rowMovement && (f32) playerWork->state[1] == columnMovement) ? playerWork->state[2] + 1 : ((rowMovement != 0.0f || columnMovement != 0.0f) ? 1 : 0);
            playerWork->state[0] = (s32) rowMovement;
            playerWork->state[1] = (s32) columnMovement;
            if (((s32) playerWork->state[2] == 1) || (((s32) playerWork->state[2] >= 0xA) && ((s32) (playerWork->state[2] % 3) == 0))) {
                playerWork->gridRow = (f32) (playerWork->gridRow + rowMovement);
                playerWork->gridColumn = (f32) (playerWork->gridColumn + columnMovement);
            }
            playerWork->gridRow = (f32) (s32) (0.5f + playerWork->gridRow);
            playerWork->gridRow = ((playerWork->gridRow < 0.0f) ? 0.0f : ((playerWork->gridRow > 5.0f) ? 5.0f : playerWork->gridRow));
            playerWork->gridColumn = (f32) (s32) (0.5f + playerWork->gridColumn);
            playerWork->gridColumn = ((playerWork->gridColumn < 0.0f) ? 0.0f : ((playerWork->gridColumn > 13.0f) ? 13.0f : playerWork->gridColumn));
            if (((playerWork->gridRow != playerWork->previousRow) || (playerWork->gridColumn != playerWork->shiftedPosition)) && (lbl_1_bss_154 > 2) && ((s32) lbl_1_bss_174[playerIndex].count < 3)) {
                lbl_1_bss_174[playerIndex].count += 1;
            }
            {
            s32 actionDurations[5] = {255, 30, 30, 15, 15};
            playerWork->state[7] = (s32) (playerWork->state[7] + (playerWork->state[7] < actionDurations[playerWork->state[6]]));
            if (((s32) playerWork->state[7] >= (s32) actionDurations[playerWork->state[6]]) && ((s32) playerWork->state[6] != 0) && ((s32) playerWork->state[6] != 1)) {
                playerWork->state[6] = 0;
                playerWork->state[7] = 0;
            }
            if ((s32) lbl_1_data_A8[playerIndex] > 0) {
                playerWork->gridRow = (((playerWork->gridRow + lbl_1_bss_6D70[playerIndex]) < 0.0f) ? -lbl_1_bss_6D70[playerIndex] : playerWork->gridRow);
                playerWork->gridColumn = (((playerWork->gridColumn + lbl_1_bss_6D60[playerIndex]) < 0.0f) ? -lbl_1_bss_6D60[playerIndex] : playerWork->gridColumn);
                playerWork->gridRow = (((playerWork->gridRow + lbl_1_bss_6D68[playerIndex]) > 6.0f) ? 6.0f - lbl_1_bss_6D68[playerIndex] : playerWork->gridRow);
                playerWork->gridColumn = (((playerWork->gridColumn + lbl_1_bss_6D58[playerIndex]) > 15.0f) ? 15.0f - lbl_1_bss_6D58[playerIndex] : playerWork->gridColumn);
                for (entryIndex = 0; entryIndex < (s32) lbl_1_data_A8[playerIndex]; entryIndex++) {
                    carriedEntry = lbl_1_bss_6D78[playerIndex][entryIndex];
                    carriedEntry->gridRow += playerWork->gridRow - playerWork->previousRow;
                    carriedEntry->gridColumn += playerWork->gridColumn - playerWork->shiftedPosition;
                }
            }
            if ((buttonAHeld != 0) && ((s32) lbl_1_data_A8[playerIndex] == 0) && ((s32) playerWork->state[6] != 3) && ((s32) playerWork->state[6] != 4)) {
                grabbedEntry = NULL;
                connectedCount = 0;
                for (entryIndex = 0; entryIndex < 0x66; entryIndex++) {
                    grabCandidate = &lbl_1_bss_70A8[playerIndex][entryIndex];
                    if (((s32) grabCandidate->kind != 0) && ((s32) grabCandidate->kind == 1) && ((s32) grabCandidate->variant != 0) && ((s32) grabCandidate->category == 0) && ((s32) grabCandidate->age > 0) && ((s32) grabCandidate->fadeTimer == 0) && ((s32) grabCandidate->groupMaximum == 0) && (grabCandidate->gridRow == playerWork->gridRow) && (grabCandidate->gridColumn == playerWork->gridColumn) && (grabCandidate->fallSpeed == 0.0f)) {
                        grabCandidate->connected = 1;
                        grabbedEntry = grabCandidate;
                        connectedEntries[0] = grabbedEntry;
                        visitedDirections[0] = 0;
                        connectedCount = 1;
                        grabCandidate->category = 5;
                        grabCandidate->groupMaximum = 0;
                        lbl_1_bss_6D78[playerIndex][lbl_1_data_A8[playerIndex]] = grabCandidate;
                        lbl_1_data_A8[playerIndex] += 1;
                    } else if ((buttonAPressed != 0) && ((s32) grabCandidate->kind != 0) && ((s32) grabCandidate->kind == 4) && (grabCandidate->gridRow == playerWork->gridRow) && ((grabCandidate->gridColumn - 0.4f) <= playerWork->gridColumn) && (playerWork->gridColumn <= (0.4f + grabCandidate->gridColumn))) {
                        playerWork->state[6] = 3;
                        playerWork->state[7] = 0;
                        playerWork->releaseSpeed = 1.0f;
                        grabbedEntry = NULL;
                        if ((s32) lbl_1_bss_174[playerIndex + 2].count < 3) {
                            lbl_1_bss_174[playerIndex + 2].count += 1;
                        }
                    } else {
                        continue;
                    }
                    break;
                }
                if (grabbedEntry) {
                    rootGroup = grabbedEntry->sequenceGroup;
                    while (rootGroup && rootGroup->parent) {
                        rootGroup = rootGroup->parent;
                    }
                    if (rootGroup) {
                        groupEntry = rootGroup->first;
                        while (groupEntry) {
                            if ((groupEntry != grabbedEntry) && ((s32) groupEntry->kind != 0) && ((s32) groupEntry->kind == 1) && ((s32) groupEntry->variant == (s32) grabbedEntry->variant) && ((s32) groupEntry->category == 0) && ((s32) groupEntry->age > 0) && ((s32) groupEntry->fadeTimer == 0) && ((s32) groupEntry->groupMaximum == 0) && ((groupEntry->gridColumn - (f32) (s32) (1000.0f + groupEntry->gridColumn)) == (grabbedEntry->gridColumn - (f32) (s32) (1000.0f + grabbedEntry->gridColumn))) && (groupEntry->fallSpeed == grabbedEntry->fallSpeed)) {
                                connectedEntries[connectedCount] = groupEntry;
                                visitedDirections[connectedCount] = 0;
                                groupEntry->connected = 0;
                                connectedCount += 1;
                            }
                            groupEntry = groupEntry->sequenceNextInGroup;
                        }
                        do {
                            foundNeighbor = 0;
                            connectedEntryIndex = 0;
                            while (connectedEntryIndex < connectedCount) {
                                groupEntry = connectedEntries[connectedEntryIndex];
                                if (((s32) groupEntry->connected != 0) && ((s32) (visitedDirections[connectedEntryIndex] & 0xF) == 0)) {
                                    directionIndex = 0;
                                    while (directionIndex < 4) {
                                        s32 neighborRowSteps[4] = {-1, 0, 1, 0};

                                        s32 neighborColumnSteps[4] = {0, -1, 0, 1};

                                        if ((s32) ((1 << directionIndex) & visitedDirections[connectedEntryIndex]) == 0) {
                                            visitedDirections[connectedEntryIndex] |= 1 << directionIndex;
                                            for (connectedIndex = 0; connectedIndex < connectedCount; connectedIndex++) {
                                                neighborEntry = connectedEntries[connectedIndex];
                                                if (((s32) neighborEntry->connected == 0) && (neighborEntry->gridRow == (groupEntry->gridRow + (f32) neighborRowSteps[directionIndex])) && (neighborEntry->gridColumn == (groupEntry->gridColumn + (f32) neighborColumnSteps[directionIndex]))) {
                                                    neighborEntry->connected = 1;
                                                    neighborEntry->category = 5;
                                                    neighborEntry->groupMaximum = 0;
                                                    lbl_1_bss_6D78[playerIndex][lbl_1_data_A8[playerIndex]] = neighborEntry;
                                                    lbl_1_data_A8[playerIndex] += 1;
                                                    foundNeighbor = 1;
                                                } else {
                                                    continue;
                                                }
                                                break;
                                            }
                                        }
                                        directionIndex += 1;
                                    }
                                }
                                connectedEntryIndex += 1;
                            }
                        } while (foundNeighbor != 0);
                    }
                    lbl_1_bss_6D70[playerIndex] = lbl_1_bss_6D60[playerIndex] = 65536.0f;
                    lbl_1_bss_6D68[playerIndex] = lbl_1_bss_6D58[playerIndex] = -65536.0f;
                    workIndex = 0;
                    while (workIndex < (s32) lbl_1_data_A8[playerIndex]) {
                        heldEntry = lbl_1_bss_6D78[playerIndex][workIndex];
                        lbl_1_bss_6D70[playerIndex] = (((heldEntry->gridRow - playerWork->gridRow) < lbl_1_bss_6D70[playerIndex]) ? heldEntry->gridRow - playerWork->gridRow : lbl_1_bss_6D70[playerIndex]);
                        lbl_1_bss_6D60[playerIndex] = (((heldEntry->gridColumn - playerWork->gridColumn) < lbl_1_bss_6D60[playerIndex]) ? heldEntry->gridColumn - playerWork->gridColumn : lbl_1_bss_6D60[playerIndex]);
                        lbl_1_bss_6D68[playerIndex] = ((((1.0f + heldEntry->gridRow) - playerWork->gridRow) > lbl_1_bss_6D68[playerIndex]) ? (1.0f + heldEntry->gridRow) - playerWork->gridRow : lbl_1_bss_6D68[playerIndex]);
                        lbl_1_bss_6D58[playerIndex] = ((((1.0f + heldEntry->gridColumn) - playerWork->gridColumn) > lbl_1_bss_6D58[playerIndex]) ? (1.0f + heldEntry->gridColumn) - playerWork->gridColumn : lbl_1_bss_6D58[playerIndex]);
                        lbl_1_bss_6A28[playerIndex][workIndex] = heldEntry->gridRow - playerWork->gridRow;
                        lbl_1_bss_66F8[playerIndex][workIndex] = heldEntry->gridColumn - playerWork->gridColumn;
                        workIndex += 1;
                    }
                    playerWork->state[6] = 1;
                    playerWork->state[7] = 0;
                    if ((s32) lbl_1_bss_174[playerIndex + 2].count < 3) {
                        lbl_1_bss_174[playerIndex + 2].count += 1;
                    }
                }
            } else if (((s32) playerWork->state[6] == 3) || ((s32) playerWork->state[6] == 4)) {
                if ((s32) playerWork->state[7] == 7) {
                    nearestEntry = NULL;
                    entryIndex = 0;
                    while (entryIndex < 0x66) {
                        if (((s32) (impactEntry = &lbl_1_bss_70A8[playerIndex][entryIndex])->kind != 0) && ((s32) impactEntry->kind == 4) && (impactEntry->gridRow == playerWork->gridRow) && ((0.4f + impactEntry->gridColumn) >= playerWork->gridColumn) && (playerWork->gridColumn >= (impactEntry->gridColumn - 0.4f)) && ((!nearestEntry) || (fabs(impactEntry->gridColumn - playerWork->gridColumn) < fabs(nearestEntry->gridColumn - playerWork->gridColumn)))) {
                            nearestEntry = impactEntry;
                        }
                        entryIndex += 1;
                    }
                    if (nearestEntry) {
                        nearestEntry->fadeTimer = (s32) lbl_1_data_130[playerIndex];
                        if ((s32) lbl_1_bss_174[playerIndex + 0x1C].count < 3) {
                            lbl_1_bss_174[playerIndex + 0x1C].count += 1;
                        }
                    }
                }
            } else if (((s32) lbl_1_data_A8[playerIndex] > 0) && (buttonAHeld == 0)) {
                f32 rowVelocities[6];
                f32 rowFloor[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

                s32 fallingCounts[6] = {0, 0, 0, 0, 0, 0};

                for (entryIndex = 0; entryIndex < 6; entryIndex++) {
                    (&rowVelocities[0])[entryIndex] = -65536.0f;
                    rowFloor[entryIndex] = 15.0f;
                }
                workIndex = 0;
                while (workIndex < 3) {
                    entryIndex = 0;
                    while (entryIndex < 0x66) {
                        if (lbl_1_bss_70A8[playerIndex][entryIndex].kind != 0) {
                            if ((workIndex == 0) && (lbl_1_bss_70A8[playerIndex][entryIndex].category == 6)) {
                                fallingCounts[(s32) lbl_1_bss_70A8[playerIndex][entryIndex].gridRow] += 1;
                            }
                            if (((s32) lbl_1_bss_70A8[playerIndex][entryIndex].gridRow >= 0) && ((s32) lbl_1_bss_70A8[playerIndex][entryIndex].gridRow < 6)) {
                                if ((workIndex == 0) && ((lbl_1_bss_70A8[playerIndex][entryIndex].category == 0) || (lbl_1_bss_70A8[playerIndex][entryIndex].category == 4))) {
                                        rowFloor[(s32) lbl_1_bss_70A8[playerIndex][entryIndex].gridRow] = ((rowFloor[(s32) lbl_1_bss_70A8[playerIndex][entryIndex].gridRow] < lbl_1_bss_70A8[playerIndex][entryIndex].gridColumn) ? rowFloor[(s32) lbl_1_bss_70A8[playerIndex][entryIndex].gridRow] : lbl_1_bss_70A8[playerIndex][entryIndex].gridColumn);
                                } else if ((workIndex == 1) && (lbl_1_bss_70A8[playerIndex][entryIndex].category == 3)) {
                                        fallingRow = (s32) lbl_1_bss_70A8[playerIndex][entryIndex].gridRow;

                                        fallFrames = -(&rowVelocities[0])[fallingRow] / ((lbl_1_bss_70A8[playerIndex][entryIndex].category == 6) ? 0.03125f : 0.015625f);

                                        fallingColumn = lbl_1_bss_70A8[playerIndex][entryIndex].gridColumn + (fallFrames * ((lbl_1_bss_70A8[playerIndex][entryIndex].fallSpeed + lbl_1_bss_70A8[playerIndex][entryIndex].fallSpeed + ((fallFrames - 1.0f) * ((lbl_1_bss_70A8[playerIndex][entryIndex].category == 6) ? 0.03125f : 0.015625f))) / 2.0f));
                                        if (workIndex == 1) {
                                            rowFloor[fallingRow] = ((fallingColumn < (rowFloor[fallingRow] - 1.0f)) ? fallingColumn : rowFloor[fallingRow] - 1.0f);
                                        } else {
                                            rowFloor[fallingRow] -= 1.0f;
                                        }
                                    } else if ((workIndex == 2) && ((lbl_1_bss_70A8[playerIndex][entryIndex].category == 6) || (((s32) fallingCounts[(s32) lbl_1_bss_70A8[playerIndex][entryIndex].gridRow] > 0) && (lbl_1_bss_70A8[playerIndex][entryIndex].category == 5)))) {
                                        rowFloor[(s32) lbl_1_bss_70A8[playerIndex][entryIndex].gridRow] -= 1.0f;
                                    }
                                }
                            }
                        entryIndex += 1;
                    }
                    if (workIndex != 1) {
                        entryIndex = 0;
                        while (entryIndex < (s32) lbl_1_data_A8[playerIndex]) {
                            unusedReleaseState = 0;
                            carriedRow = (s32) lbl_1_bss_6D78[playerIndex][entryIndex]->gridRow;

                            carriedOverlap = lbl_1_bss_6D78[playerIndex][entryIndex]->gridColumn - ((rowFloor[carriedRow] - 1.0f) - ((workIndex == 0) ? 0.125f : 0.125f));
                            carriedOverlap = carriedOverlap < 0.0f ? 0.0f : carriedOverlap;
                            (&rowVelocities[0])[carriedRow] = ((carriedOverlap > (&rowVelocities[0])[carriedRow]) ? carriedOverlap : (&rowVelocities[0])[carriedRow]);
                            entryIndex += 1;
                        }
                    }
                    if (workIndex != 1) {
                        entryIndex = 0;
                        while (entryIndex < 6) {
                            fallDiscriminant = 0.0009765625f + (4.0f * (2.0f * (0.03125f * (&rowVelocities[0])[entryIndex])));
                              rowVelocities[entryIndex] = (-1.0f * ((sqrtf(fallDiscriminant) - 0.03125f) / 2.0f)) - 0.03125f;
                            entryIndex += 1;
                        }
                    }
                    workIndex += 1;
                }
                playerWork->releaseSpeed = 0.0f;
                if ((s32) lbl_1_bss_174[playerIndex + 4].count < 3) {
                    lbl_1_bss_174[playerIndex + 4].count += 1;
                }
                entryIndex = 0;
                while (entryIndex < (s32) lbl_1_data_A8[playerIndex]) {
                    releaseSpeed = 0.0f;
                    releasedEntry = lbl_1_bss_6D78[playerIndex][entryIndex];
                    releasedEntry->category = 6;
                    releasedEntry->gridRow = (f32) (s32) (0.5f + releasedEntry->gridRow);
                    releasedEntry->gridColumn = (f32) releasedEntry->gridColumn;
                    releasedEntry->fallSpeed = (f32) (&rowVelocities[0])[(s32) releasedEntry->gridRow];
                    releasedEntry->floorColumnAtRelease =
                        (f32) rowFloor[(s32) releasedEntry->gridRow];
                    releasedEntry->releaseSerial = (u32) lbl_1_data_B0[playerIndex];
                    releaseSpeed = (&rowVelocities[0])[(s32) releasedEntry->gridRow] > -0.075f ? 0.0f : (&rowVelocities[0])[(s32) releasedEntry->gridRow] / -1.0f;
                    playerWork->releaseSpeed = playerWork->releaseSpeed < releaseSpeed ? releaseSpeed : playerWork->releaseSpeed;
                    entryIndex += 1;
                }
                playerWork->state[6] = playerWork->releaseSpeed == 0.0f ? 0 : 2;
                playerWork->state[7] = 0;
                lbl_1_data_B0[playerIndex] += 1;
                lbl_1_data_B0[playerIndex] %= 0x10000;
                lbl_1_data_A8[playerIndex] = 0;
            }
            playerWork->previousRow = (f32) playerWork->gridRow;
            playerWork->shiftedPosition = (f32) playerWork->gridColumn;
            }
            playerIndex += 1;
        }
    }
}

extern s32 lbl_1_bss_10;
extern s32 lbl_1_bss_28;
extern s32 lbl_1_bss_2C;
extern s32 lbl_1_bss_14;
extern u8 lbl_1_data_6A[10];
extern u8 lbl_1_data_74[10];
extern s32 lbl_1_data_AC;
extern s32 lbl_1_data_B4;
extern s32 lbl_1_data_C4;
extern s32 lbl_1_data_C8[2];
extern float lbl_1_data_160[2];
extern MGSEQ_PARAM lbl_1_data_FC;
extern s32 lbl_1_data_CC[4];
extern s32 lbl_1_data_DC[4];
extern s32 lbl_1_data_EC[4];
extern HuVecF lbl_1_data_134;
extern HuVecF lbl_1_data_140;
extern GXColor lbl_1_data_14C;
extern GXColor lbl_1_data_1B4;
extern HuVecF lbl_1_data_1B8;

float fn_1_4DEC(float frame, float rise, float hold, float fall)
{
    float area = fall / 2.0f + (rise / 2.0f + hold);

    return frame < rise
        ? ((1.0f + frame) / 2.0f * frame) / rise / area
        : frame < rise + hold
            ? ((1.0f + rise) / 2.0f * rise) / rise / area
                + (frame - rise) / area
            : frame < fall + (rise + hold) - 1.0f
                ? 1.0f - ((1.0f + (fall + (rise + hold) - frame - 1.0f))
                    / 2.0f * (fall + (rise + hold) - 1.0f - frame - 1.0f))
                    / (fall - 1.0f) / area
                : 1.0f;
}

typedef M678PlayerSetupWork M678PlayerWork;

extern s32 lbl_1_bss_30;

extern s32 lbl_1_bss_34;

extern u32 lbl_1_bss_5BEC;

extern s16 lbl_1_bss_14C;

extern s32 lbl_1_data_110[2];

extern s32 lbl_1_data_1EC;

extern s32 lbl_1_data_1F0;

extern s32 lbl_1_data_1F4;

extern u8 lbl_1_data_48[4];

extern u8 motionBlendFrames[4];

extern s32 lbl_1_data_50[4];

typedef s32 M678UnknownValue;

/* Optional value trace. */
static inline void DebugValue(f32 value) { }

struct _struct_lbl_1_bss_5BF0_0x140 { u8 bytes[0x140]; };

struct _struct_lbl_1_bss_5E70_0x8 { s32 write[2]; };

struct _struct_lbl_1_bss_5E90_0x400 { s32 entries[2][0x80]; };

struct _struct_lbl_1_bss_66C0_0x18 { s32 overflowFrames[6]; };

struct _struct_lbl_1_bss_70A8_0x3960 { M678LinkEntry entries[102]; };

struct _struct_lbl_1_data_138_0x18 { s32 rowCounts[6]; };

struct _struct_lbl_1_data_1AC_0x6 { s8 bytes[0x6]; };

/* size = 0x8 */

/* size = 0x24 */
struct _struct_lbl_1_bss_5E80_0x8 {
    s32 read[2];
};

/* size = 0x8 */
struct _struct_lbl_1_bss_6690_0xC {
    u8 unknown_00[4]; /* No code names this field. */
    s32 rowQueueElapsedFrames;
    s32 fallingQueueElapsedFrames;
};

/* size = 0xC */
struct _struct_lbl_1_bss_66A8_0xC {
    s32 pendingCount[3];
};

extern s32 lbl_1_bss_11C;

extern s32 lbl_1_bss_120;

extern s32 lbl_1_bss_130[2];

extern OMOBJ *lbl_1_bss_158;

extern s32 lbl_1_bss_18;

extern s32 lbl_1_bss_1C;

extern s32 lbl_1_bss_20;

extern MGSCORE *lbl_1_bss_2B4[7];

extern s32 lbl_1_bss_2F80[2];

extern M678LinkGroupSet lbl_1_bss_2F88[2];

extern s32 lbl_1_bss_38;

extern u32 lbl_1_bss_3C;

extern s32 lbl_1_bss_4;

extern s32 lbl_1_bss_40[5];

extern struct _struct_lbl_1_bss_45FC_0x24 lbl_1_bss_45D8;

extern struct _struct_lbl_1_bss_45FC_0x24 lbl_1_bss_45FC[0x14];

typedef struct M678TextureEffect {
    s32 textureIndex;
    f32 x;
    f32 y;
} M678TextureEffect;

struct M678TextureEffectStorage { M678TextureEffect entries[0xCC]; };

extern struct M678TextureEffectStorage lbl_1_bss_48CC[2];

extern s32 lbl_1_bss_54;

extern struct _struct_lbl_1_bss_5BF0_0x140 lbl_1_bss_5BF0[2];

extern s32 lbl_1_bss_5C;

extern struct _struct_lbl_1_bss_66C0_0x18 lbl_1_bss_66C0[2];

extern struct _struct_lbl_1_bss_66A8_0xC lbl_1_bss_66A8[2];

extern struct _struct_lbl_1_bss_6690_0xC lbl_1_bss_6690[2];

extern struct _struct_lbl_1_bss_5E90_0x400 lbl_1_bss_5E90[2];

extern struct _struct_lbl_1_bss_5E80_0x8 lbl_1_bss_5E80[2];

extern struct _struct_lbl_1_bss_5E70_0x8 lbl_1_bss_5E70[2];

extern s32 lbl_1_bss_60;

extern s32 lbl_1_bss_64;

extern s32 lbl_1_bss_68;

extern f32 lbl_1_bss_6C[2];

extern f32 lbl_1_bss_74[2];

extern void *lbl_1_bss_7C[0x28];

extern s32 lbl_1_data_100[2];

extern s32 lbl_1_data_108[2];

extern s32 lbl_1_data_118[2];

extern s32 lbl_1_data_120[2];

extern s32 lbl_1_data_128[2];

extern struct _struct_lbl_1_data_138_0x18 lbl_1_data_138[2];

extern s32 lbl_1_data_168[2][6];

extern f32 lbl_1_data_198;

extern s32 lbl_1_data_19C[2];

extern s32 lbl_1_data_1A4[2];

extern struct _struct_lbl_1_data_1AC_0x6 lbl_1_data_1AC[6];

extern u32 lbl_1_data_1D0[2];

extern s32 lbl_1_data_1D8[2];

extern s32 lbl_1_data_1E0[3];

extern f32 lbl_1_data_254;

extern f32 lbl_1_data_258;

extern f32 lbl_1_data_60[2];

extern f32 lbl_1_data_68[6];

extern s32 lbl_1_data_98[2];

extern s32 lbl_1_data_A0[2];

extern s32 lbl_1_data_B8[2];

extern s32 lbl_1_data_D0[2];

extern s32 lbl_1_data_D8[2];

extern s32 lbl_1_data_E0[2];

extern s32 lbl_1_data_E8[2];

extern s32 lbl_1_data_F8[2];

/* Unidentified initialized bytes, .data 0x0..0x38. */
u8 lbl_1_data_0[0x38] = {
    0x00, 0x90, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0xC2, 0x00, 0x00, 0x00, 0xF5, 0x00, 0x00,
    0x00, 0xEE, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00,
    0x00, 0xD1, 0x00, 0x00, 0x00, 0xB9, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x00, 0x00, 0xB9, 0x00, 0x00,
    0x00, 0xB9, 0x00, 0x00, 0x00, 0xB9, 0x00, 0x00
};
u32 lbl_1_data_38[4] = { 0x930000, 0x930024, 0x930027, 0x930028 };
/* Four byte entries consumed by motion selection through lbl_1_data_48[0..3]. */
u8 lbl_1_data_48[4] = { 0, 0x01, 0x01, 0x01 };
/* Motion transition blend durations. */
u8 motionBlendFrames[4] = { 8, 8, 8, 8 };
s32 lbl_1_data_50[4] = { 0, 0, 0, 0 };
f32 lbl_1_data_60[2] = { 0.0f, 0.0f };
f32 lbl_1_data_68[6] = { 0.0f, 0.0f, -0.00390625f, 0.0f, 0.0f, 0.0f };
s32 lbl_1_data_80[2] = { 0, 0 };
s32 lbl_1_data_88[2] = { 0, 0 };
s32 lbl_1_data_90[2] = { 0, 0 };
s32 lbl_1_data_98[2] = { 0, 0 };
s32 lbl_1_data_A0[2] = { 0, 0 };
s32 lbl_1_data_A8[2] = { 0, 0 };
s32 lbl_1_data_B0[2] = { 0, 0 };
s32 lbl_1_data_B8[2] = { 0, 0 };
s32 lbl_1_data_C0[2] = { 0, 0 };
s32 lbl_1_data_C8[2] = { 0, 0 };
s32 lbl_1_data_D0[2] = { 0, 0 };
s32 lbl_1_data_D8[2] = { 0, 0 };
s32 lbl_1_data_E0[2] = { 0, 0 };
s32 lbl_1_data_E8[2] = { 0, 0 };
s32 lbl_1_data_F0[2] = { 0, 0 };
s32 lbl_1_data_F8[2] = { 0, 0 };
s32 lbl_1_data_100[2] = { 0, 0 };
s32 lbl_1_data_108[2] = { 0, 0 };
s32 lbl_1_data_110[2] = { 0, 0 };
s32 lbl_1_data_118[2] = { 0, 0 };
s32 lbl_1_data_120[2] = { 0, 0 };
s32 lbl_1_data_128[2] = { 0, 0 };
s32 lbl_1_data_130[2] = { 90, 90 };
struct _struct_lbl_1_data_138_0x18 lbl_1_data_138[2] = { { { 0, 0, 0, 0, 0, 0 } }, { { 0, 0, 0, 0, 0, 0 } } };
s32 lbl_1_data_168[2][6] = { { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 } };
f32 lbl_1_data_198 = 1.0f;
s32 lbl_1_data_19C[2] = { 0, 0 };
s32 lbl_1_data_1A4[2] = { 0, 0 };
struct _struct_lbl_1_data_1AC_0x6 lbl_1_data_1AC[6] = { { "" }, { "" }, { "" }, { "" }, { "" }, { "" } };
u32 lbl_1_data_1D0[2] = { 0, 0 };
s32 lbl_1_data_1D8[2] = { 0, 0 };
s32 lbl_1_data_1E0[3] = { 0, 0, -65536 };
s32 lbl_1_data_1EC = -1;
s32 lbl_1_data_1F0 = -1;
s32 lbl_1_data_1F4 = -1;
s32 lbl_1_data_1F8 = 1;
extern void fn_1_12EB4(s16 mode, s16 frameNo);
extern void fn_1_106B8(s16 mode, s16 frameNo);
extern void fn_1_106BC(s16 mode, s16 frameNo);
extern void fn_1_106C0(s16 mode, s16 frameNo);
extern void fn_1_106C4(s16 mode, s16 frameNo);
extern void fn_1_106C8(s16 mode, s16 frameNo);
extern void fn_1_106CC(s16 mode, s16 frameNo);
extern void fn_1_106D0(s16 mode, s16 frameNo);
extern void fn_1_1300C(s16 mode, s16 frameNo);
MGSEQ_PARAM lbl_1_data_1FC = {0, 0, fn_1_12EB4, fn_1_106B8, fn_1_106BC, fn_1_106C0, fn_1_106C4, fn_1_106C8, fn_1_106CC, fn_1_106D0, fn_1_1300C};
/* Unidentified initialized bytes, .data 0x224..0x254. */
u8 lbl_1_data_224[0x30] = {
    0x00, 0x00, 0x00, 0x00, 0x45, 0x9C, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x4C, 0xCC, 0xCD,
    0xBF, 0x80, 0x00, 0x00, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBF, 0x80, 0x00, 0x00,
    0xBF, 0x80, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x80, 0xFF, 0xFF
};
f32 lbl_1_data_254 = 1.5f;
f32 lbl_1_data_258 = 0.75f;

/* Advances falling pieces, matches, incoming queues, effects, and loss conditions. */
void fn_1_4FB0(OMOBJ *hookWork) {

    M678PlayerWork *playerWorks[4];
    s32 averageLevel;
    s32 thresholdIndex;
    s32 clearedState;
    f32 matchedRow;
    f32 matchedColumn;
    s32 rowShiftTrigger;
    s32 initialShiftState;
    s32 loop;
    M678LinkEntry *shiftEntry;
    s32 variantCount;
    M678LinkEntry *newRowEntry;
    s32 totalVariantWeight;
    s32 variantDraw;
    s32 newRowIndex;
    s32 currentVariant;
    s32 variantRunLength;
    s32 longestRunVariant;
    s32 longestRunLength;
    s32 variantRetryCount;
    s32 firstQueueRetryCount;
    s32 entryIndex;
    s32 spawnInterval;
    s32 secondQueueRetryCount;
    M678LinkEntry *spawnedFallingEntry;
    s32 spawnedEntryIndex;
    M678LinkEntry *sortingEntry;
    M678LinkEntry *rowInsertionNext;
    M678LinkEntry *rowInsertionPrevious;
    M678LinkEntry *linkedEntry;
    M678LinkEntry *fallingSortEntry;
    M678LinkEntry *fallingInsertionNext;
    M678LinkEntry *fallingInsertionPrevious;
    M678LinkEntry *rowTail;
    M678LinkEntry *rowGridEntry;
    s32 entryGridRow;
    s32 entryGridColumn;
    M678LinkEntry *gridCandidate;
    M678ChainGroup *mergedGroup;
    M678LinkEntry *groupMember;
    M678LinkEntry *otherMember;
    s32 effectTravelFrames;
    M678ChainGroup *matchedGroup;
    s32 matchedUntimedCount;
    s32 minimumGroupTimer;
    s32 maximumGroupTimer;
    f32 pathDist;
    f32 effectRowDistance;

    f32 effectColumnDistance;

    M678LinkEntry *timerEntry;
    M678ChainGroup *expiringGroup;
    M678LinkEntry *expiringEntry;
    s32 recipientPlayer;
    s32 queuedKindIndex;
    f32 spriteBlendRemainder;
    f32 spriteScale;
    f32 endingSpriteFraction;
    f32 endingSpriteScale;
    s32 resultEffectState;
    f32 resultTravelRemainder;
    f32 resultFadeCurve;
    f32 resultSpriteScale;
    s32 removedCount;
    struct M678TextureEffect *effectRecord;
    M678LinkEntry *followingRowEntry;
    M678LinkEntry *movingEntry;
    M678LinkEntry *fallingEntry;
    M678LinkEntry *entryBelow;
    M678LinkEntry *sameFallEntry;
    M678LinkEntry *integratedEntry;
    s32 repeatResolution;
    s32 collisionCategory;
    s32 collisionTimer;
    M678ChainGroup *fallingGroup;
    M678ChainGroup *otherGroup;
    s32 fadingEntryAhead;
    M678LinkEntry *fadeSearchEntry;
    M678LinkEntry *prevEntry;
    M678LinkEntry *nextEntry;
    f32 scoreScale;
    f32 groupCollisionOffset;
    f32 resultAnimationFrame;
    f32 rowShift;
    f32 effectTravelFraction;
    f32 resultTravelFraction;
    f32 effectFadeFraction;
    f32 resultFadeFraction;
    f32 floorOverlap;
    f32 groupCollisionSpeed;
    f32 scorePulseScale;
    f32 limitOrScale;
    f32 obstacleColumn;
    f32 playerHorizontalOffset;
    s32 motionFrame;
    f32 playerVerticalOffset;
    f32 playerRotationDegrees;
    s32 initialMotionState;
    f32 playerBaseX;
    f32 playerBaseY;
    f32 rotationOffsetX;
    f32 rotationOffsetY;
    s32 rotationAngle;
    struct M678TextureEffect *effectAgePointer;
    s32 audioFadeDivisor;
    M678LinkEntry *endingEntry;
    f32 scorePulse;

    M678ChainGroup *firstGroupRoot;
    s32 scan;
    s32 index;
    s32 playerIndex;

    struct _struct_lbl_1_bss_45FC_0x24 *firstEffectCounter;
    struct _struct_lbl_1_bss_45FC_0x24 *secondEffectCounter;
    struct _struct_lbl_1_bss_45FC_0x24 *newEffect;
    struct _struct_lbl_1_bss_45FC_0x24 *movingEffect;
    struct _struct_lbl_1_bss_5E80_0x8 *firstQueueCursor;
    struct _struct_lbl_1_bss_5E80_0x8 *fallQueueReadCursor;
    struct _struct_lbl_1_bss_5E80_0x8 *rowQueueReadCursor;
    struct _struct_lbl_1_bss_5E80_0x8 *fallQueueWrapCursor;
    struct _struct_lbl_1_bss_6690_0xC *rowQueueTimer;
    struct _struct_lbl_1_bss_6690_0xC *fallQueueTimer;
    struct _struct_lbl_1_bss_66A8_0xC *pendingRowQueue;
    struct _struct_lbl_1_bss_66A8_0xC *pendingFallQueue;
    struct _struct_lbl_1_bss_66A8_0xC *newFallQueue;

    M678LinkEntry *countedEntry;
    M678LinkEntry *connectingEntry;
    M678LinkEntry *hitEntry;

    M678ChainGroup *neighborGroupRoot;
    M678LinkEntry *below;
    M678LinkEntry *collisionGroupEntry;
    M678LinkEntry *rowEntry;
    M678LinkEntry *neighborEntry;
    M678LinkEntry *matchedGroupEntry;

    for (index = 0; index < lbl_1_bss_8; index++) {
        playerWorks[index] = (M678PlayerWork *) lbl_1_bss_15C[index]->data;
    }
    lbl_1_bss_154 = fn_80071A8C();
    lbl_1_bss_C = fn_80071DE4();
    lbl_1_bss_14 += 1;
    lbl_1_bss_14 %= 0xF0000000U;
    lbl_1_bss_28 += lbl_1_bss_28 < 0xFFFF;
    {
    s32 groupChangedFlags[2] = { 0x0, 0x0 };
    s32 movementChangedFlags[2] = { 0x0, 0x0 };
    s32 specialEntryCounts[2] = { 0x0, 0x0 };
    {
    s32 frameThresholds[16] = { 0x0, 0x32, 0x64, 0x96, 0xc8, 0xfa, 0x12c, 0x15e, 0x190, 0x1c2, 0x1f4, 0x226, 0x258, 0x28a, 0x2bc, 0x7fffffff };

    averageLevel = 0;
    thresholdIndex = -1;
    index = 0;
    while (index < lbl_1_bss_8) {
        averageLevel += (s32) lbl_1_data_108[index] / 4;
        index += 1;
    }
    averageLevel = averageLevel / lbl_1_bss_8;
    do {
        thresholdIndex += 1;
    } while (frameThresholds[thresholdIndex + 1] < averageLevel * 8);
    }
    for (playerIndex = 0; playerIndex < lbl_1_bss_8; playerIndex++) {
        {
        M678LinkEntry *entryGrid[6][30] = {{NULL}};

{
M678LinkEntry * rowHeads[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
        {
        M678LinkEntry *rowEntryGrid[6][30] = {{NULL}};

{
s32 rowCount[6] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

{
M678LinkEntry * fallHeads[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
        {
        M678LinkEntry *adjacentEntries[102][4] = {{NULL}};
        clearedState = 0;
        lbl_1_bss_2F80[playerIndex] = 0;
        specialEntryCounts[playerIndex] = lbl_1_data_E8[playerIndex];

        lbl_1_data_128[playerIndex] += ((((s32) lbl_1_data_128[playerIndex] < 0xFFFF) && (lbl_1_bss_0 != 5)));
        rowShiftTrigger = 0;
        switch (lbl_1_data_80[playerIndex]) {                          /* switch 1; irregular */
        case 0:                                     /* switch 1 */
            {
            s32 phaseThresholdValues[100] = {
                0x0000015E, 0x0000012C, 0x0000012C, 0x000000FA, 0x000000FA, 0x000000FA, 0x000000E1, 0x000000E1, 0x000000E1, 0x000000C8,
                0x000000C8, 0x000000C8, 0x00000096, 0x00000096, 0x00000096, 0x000000E1, 0x000000AF, 0x00000096, 0x00000091, 0x0000008C,
                0x0000008C, 0x00000087, 0x00000087, 0x00000082, 0x00000082, 0x0000007D, 0x00000078, 0x00000073, 0x0000006E, 0x00000069,
                0x00000064, 0x00000064, 0x00000064, 0x0000005F, 0x0000005A, 0x00000055, 0x00000050, 0x00000050, 0x00000050, 0x00000050,
                0x00000050, 0x0000004B, 0x0000004B, 0x00000046, 0x00000046, 0x00000046, 0x00000041, 0x0000003C, 0x00000037, 0x00000032,
                0x00000037, 0x0000003C, 0x00000041, 0x00000046, 0x0000004B, 0x00000050, 0x00000050, 0x00000050, 0x00000050, 0x00000050,
                0x00000023, 0x0000001E, 0x00000019, 0x00000019, 0x00000019, 0x00000019, 0x00000019, 0x00000019, 0x00000019, 0x00000019,
                0x00000046, 0x00000041, 0x0000003C, 0x00000037, 0x0000003C, 0x0000002C, 0x00000037, 0x00000024, 0x0000002C, 0x0000001E,
                0x00000023, 0x0000001E, 0x00000019, 0x00000019, 0x00000019, 0x00000019, 0x00000019, 0x00000019, 0x00000019, 0x00000019,
                0x00000046, 0x00000041, 0x0000003C, 0x00000037, 0x0000003C, 0x0000002C, 0x00000037, 0x00000024, 0x0000002C, 0x0000001E
            };
            lbl_1_data_90[playerIndex] += lbl_1_data_90[playerIndex] < 0xFF;
            if ((s32) lbl_1_data_88[playerIndex] == 0) {
                if (lbl_1_bss_8 == 1) {
                    lbl_1_data_98[playerIndex] = phaseThresholdValues[((s32) lbl_1_data_C8[playerIndex] >= 0x64) ? 0x63 : lbl_1_data_C8[playerIndex]];
                } else {

{
s32 phaseDurations[16] = { 0x12C, 0xFA, 0xE1, 0xC8, 0x96, 0x96, 0x7D, 0x70, 0x64, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B };
                    lbl_1_data_98[playerIndex] = phaseDurations[thresholdIndex];
                }
}
                lbl_1_data_88[playerIndex] = 1;
            } else if ((lbl_1_bss_0 == 2) && ((u32) lbl_1_bss_66F0[playerIndex] == 0)) {
                if ((s32) lbl_1_bss_66A8[playerIndex].pendingCount[1] > 0) {
                    lbl_1_data_A0[playerIndex] = lbl_1_data_88[playerIndex];
                    lbl_1_data_88[playerIndex] = lbl_1_data_98[playerIndex];
                } else if ((lbl_1_bss_8 == 1) && ((((s32) lbl_1_data_C8[playerIndex] >= 0x46) && ((s32) lbl_1_data_C8[playerIndex] < 0x50)) || (((s32) lbl_1_data_C8[playerIndex] >= 0x5A) && ((s32) lbl_1_data_C8[playerIndex] < 0x64)))) {
                    lbl_1_data_88[playerIndex] += 1;
                } else {

                    lbl_1_data_88[playerIndex] += (((s32) lbl_1_data_B8[playerIndex] != 0) ? (0) : (1));
                }
            }

            lbl_1_data_60[playerIndex] = (((s32) lbl_1_data_88[playerIndex] > (s32) (lbl_1_data_98[playerIndex] - 0x1E)) ? ((((f32) (lbl_1_data_88[playerIndex] - lbl_1_data_98[playerIndex]) / 30.0f) / 48.0f) * (f32) (((s32) lbl_1_data_88[playerIndex] % 2) != 0 ? 1 : -1)) : 0.0f);
            lbl_1_data_68[playerIndex] = 0.0f;
            if ((s32) lbl_1_data_88[playerIndex] >= (s32) lbl_1_data_98[playerIndex]) {
                if ((lbl_1_bss_8 == 1) && ((((s32) lbl_1_data_C8[playerIndex] >= 0x46) && ((s32) lbl_1_data_C8[playerIndex] < 0x50)) || (((s32) lbl_1_data_C8[playerIndex] >= 0x5A) && ((s32) lbl_1_data_C8[playerIndex] < 0x64)))) {
                    lbl_1_data_80[playerIndex] = 2;
                } else {
                    lbl_1_data_80[playerIndex] = 1;
                }
                lbl_1_data_88[playerIndex] = 0;
            }
            break;
            }
        case 1:                                     /* switch 1 */
            lbl_1_data_90[playerIndex] = 0;

            rowShiftTrigger = (((s32) lbl_1_data_88[playerIndex] == 0) ? (-1) : (0));
            lbl_1_data_88[playerIndex] += 1;
            lbl_1_data_60[playerIndex] = 0.0f;

            lbl_1_data_68[playerIndex] = (((s32) lbl_1_data_88[playerIndex] <= 4) ? ((f32) ((s32) lbl_1_data_88[playerIndex] == 1 ? 0 : 1) - ((f32) lbl_1_data_88[playerIndex] / 4.0f)) : (((f32) (0xA - lbl_1_data_88[playerIndex]) / 6.0f) / 16.0f * (f32) (((s32) lbl_1_data_88[playerIndex] % 2) != 0 ? 1 : -1)));
            if ((s32) lbl_1_data_88[playerIndex] >= 0xA) {
                lbl_1_data_80[playerIndex] = 0;
                lbl_1_data_88[playerIndex] = lbl_1_data_A0[playerIndex];
                lbl_1_data_A0[playerIndex] = 0;
            }
            break;
        case 2:                                     /* switch 1 */
            lbl_1_data_90[playerIndex] = 0;

            rowShiftTrigger = (((s32) lbl_1_data_88[playerIndex] == 0) ? (-1) : (0));
            lbl_1_data_88[playerIndex] += 1;
            lbl_1_data_60[playerIndex] = 0.0f;

            lbl_1_data_68[playerIndex] = (f32) ((s32) lbl_1_data_88[playerIndex] == 1 ? 0 : 1) - ((f32) lbl_1_data_88[playerIndex] / 16.0f);
            if ((s32) lbl_1_data_88[playerIndex] >= 0x10) {
                lbl_1_data_80[playerIndex] = 0;
                lbl_1_data_88[playerIndex] = 0;
                lbl_1_data_A0[playerIndex] = 0;
            }
            break;
        }
        if ((rowShiftTrigger >= 1) || (rowShiftTrigger <= -1)) {
            s32 linkCounts[6][5] = {{0}};

{
s32 rowEntryCounts[6] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

{
M678LinkEntry * lowestColumnEntries[6] = { NULL, NULL, NULL, NULL, NULL, NULL };

{
M678LinkEntry * highestColumnEntries[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
    M678LinkEntry *newRowEntries[6];

            rowShift = (f32) ((rowShiftTrigger >= 1) ? (-1) : (1));
            initialShiftState = -1;
            lbl_1_data_68[playerIndex] += rowShift;
            playerWorks[playerIndex]->gridColumn -= rowShift;
            playerWorks[playerIndex]->shiftedPosition -= rowShift;
            index = 0;
            while (index < 0x66) {
                shiftEntry = &lbl_1_bss_70A8[playerIndex][index];
                if (shiftEntry->kind != 0) {
                    if ((shiftEntry->category != 5) && (((void *) lowestColumnEntries[(s32) shiftEntry->gridRow] == 0) || (lowestColumnEntries[(s32) shiftEntry->gridRow]->gridColumn > shiftEntry->gridColumn))) {
                        lowestColumnEntries[(s32) shiftEntry->gridRow] = shiftEntry;
                    }
                    if ((shiftEntry->category != 5) && (((void *) highestColumnEntries[(s32) shiftEntry->gridRow] == 0) || (highestColumnEntries[(s32) shiftEntry->gridRow]->gridColumn < shiftEntry->gridColumn))) {
                        highestColumnEntries[(s32) shiftEntry->gridRow] = shiftEntry;
                    }
                    if ((shiftEntry->category != 5) && (shiftEntry->kind != 0) && (shiftEntry->variant != 0)) {
                        linkCounts[(s32) shiftEntry->gridRow][shiftEntry->variant - 1] += 1;
                        rowEntryCounts[((s32) ((s32) shiftEntry->gridRow))] += 1;
                    }
                    shiftEntry->gridColumn -= rowShift;
                    shiftEntry->previousColumn -= rowShift;
                }
                index += 1;
            }
            {
            s32 variantCountsByLevel[100] = {
                0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004,
                0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005,
                0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005,
                0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005,
                0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005,
                0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005,
                0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004,
                0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004, 0x00000004,
                0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005,
                0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005, 0x00000005
            };
            if (lbl_1_bss_8 == 1) {

                variantCount = (&variantCountsByLevel[0])[(((s32) lbl_1_data_C8[playerIndex] >= 0x64) ? (0x63) : (lbl_1_data_C8[playerIndex]))];
            } else {
                variantCount = 4;
            }
            index = 0;
            scan = 0;
            while ((index < 0x66) && (scan < 6)) {
                if (lbl_1_bss_70A8[playerIndex][index].kind == 0) {
                    newRowEntry = &lbl_1_bss_70A8[playerIndex][index];

{
s32 variantWeights[5] = { 0x0, 0x0, 0x0, 0x0, 0x0 };
                    totalVariantWeight = 0;
                    variantDraw = rand8();
                    loop = 0;
                    while (loop < variantCount) {
                        variantWeights[loop] = ((((((void *) lowestColumnEntries[scan] && ((s32) (loop + 1) == (s32) lowestColumnEntries[scan]->variant))) || (((void *) highestColumnEntries[scan] && ((s32) (loop + 1) == (s32) highestColumnEntries[scan]->variant)))) != 0) ? (1) : ((((s32) rowEntryCounts[scan] == 0) ? (1) : ((s32) (1.0f + (100.0f * (1.0f - ((f32) linkCounts[scan][loop] / (f32) rowEntryCounts[scan]))))))));
                        totalVariantWeight += variantWeights[loop];
                        loop += 1;
                    }
                    loop = rand8() % variantCount;
do {
                    loop %= variantCount;
                    variantDraw -= variantWeights[loop];
                    if (variantDraw <= 0) break;
                    loop = loop + 1;
                    } while (1);
                    newRowEntry->kind = 1;
                    newRowEntry->variant = (s32) (loop + 1);
                    newRowEntry->gridRow = (f32) scan;
                    newRowEntry->gridColumn = 15.0f;
                    newRowEntry->fallSpeed = 0.0f;
                    newRowEntry->unk10 = -1;
                    newRowEntry->category = 0;
                    newRowEntry->age = 0;
                    newRowEntry->fadeTimer = 0;
                    newRowEntry->groupMaximum = 0;
                    newRowEntry->stillFrames = 0;
                    newRowEntry->previousColumn = (f32) newRowEntry->gridColumn;
                    newRowEntry->releaseSerial = -1;
                    newRowEntry->overlay = 1;
                    newRowEntry->opacityScale = 1.0f;
                newRowEntries[scan] = newRowEntry;
                    scan += 1;
                }
}
                index += 1;
            }
            if (scan == 6) {
                index = 0;
                while (index < 5) {
                    currentVariant = -1;
                    variantRunLength = 0;
                    longestRunVariant = -1;
                    longestRunLength = 0;
                    newRowIndex = 0;
                    while (newRowIndex < 6) {
                if ((newRowEntries[newRowIndex]->variant != 0) && (currentVariant == newRowEntries[newRowIndex]->variant)) {
                            variantRunLength += 1;
                            if (variantRunLength >= longestRunLength) {
                                longestRunVariant = currentVariant;
                                longestRunLength = variantRunLength;
                            }
                        } else {
                    currentVariant = newRowEntries[newRowIndex]->variant;
                            variantRunLength = 1;
                        }
                        newRowIndex += 1;
                    }
                    if (longestRunLength < 4) {
                    break;
                }

                        variantRetryCount = 0;
do {
                        newRowIndex = (s32) (6.0f * ((f32) rand8() / 256.0f));
                        variantRetryCount += 1;
} while ((variantRetryCount < 0x400) && (longestRunVariant != newRowEntries[newRowIndex]->variant));
                        variantRetryCount = 0;
do {
                        currentVariant = (s32) (1.0f + ((f32) variantCount * ((f32) rand8() / 256.0f)));
                        variantRetryCount += 1;
} while ((variantRetryCount < 0x400) && ((longestRunVariant == currentVariant) || (((u32) newRowEntries[newRowIndex]->sequenceNextInRow != 0U) && (currentVariant == (s32) newRowEntries[newRowIndex]->sequenceNextInRow->variant)) || (((u32) newRowEntries[newRowIndex]->sequenceNextInRow != 0U) && ((u32) newRowEntries[newRowIndex]->sequenceNextInRow->sequenceNextInRow != 0U) && (currentVariant == (s32) newRowEntries[newRowIndex]->sequenceNextInRow->sequenceNextInRow->variant))));
                    newRowEntries[newRowIndex]->variant = currentVariant;
                        index += 1;

                    }
            }
            if (scan == 6) {
                lbl_1_data_110[playerIndex] -= lbl_1_data_110[playerIndex] > 0;
            }
            if ((s32) lbl_1_bss_66A8[playerIndex].pendingCount[1] > 0) {
                firstQueueRetryCount = 0;
                index = 0;
                while (index < scan) {
                newRowEntries[index]->kind = 4;
                newRowEntries[index]->variant = 0;
                    index += 1;
                }
                index = 0;
                while ((index < scan) && (index < (s32) lbl_1_bss_5E90[playerIndex].entries[0][lbl_1_bss_5E80[playerIndex].read[0]]) && (firstQueueRetryCount < 0x400)) {
                    entryIndex = rand8() % scan;
                    if (newRowEntries[entryIndex]->kind == 4) {
                        newRowEntries[entryIndex]->kind = 5;
                    } else {
                        index -= 1;
                        firstQueueRetryCount += 1;
                    }
                    index += 1;
                }
                lbl_1_bss_5E80[playerIndex].read[0] += 1;
                lbl_1_bss_5E80[playerIndex].read[0] %= 0x80;
                lbl_1_bss_66A8[playerIndex].pendingCount[1] -= 1;
                lbl_1_bss_6690[playerIndex].rowQueueElapsedFrames = 0;
                if ((s32) lbl_1_bss_174[playerIndex + 0x18].count < 3) {
                    lbl_1_bss_174[playerIndex + 0x18].count += 1;
                }
            } else if ((lbl_1_bss_8 == 1) && (scan == 6)) {
                {
                s32 entryCountsByLevel[100] = {
                0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000002, 0x00000002, 0x00000002,
                0x00000002, 0x00000003, 0x00000003, 0x00000003, 0x00000003, 0x00000001, 0x00000001, 0x00000002, 0x00000002, 0x00000002,
                0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002,
                0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002,
                0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001,
                0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001,
                0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002,
                0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002,
                0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002,
                0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002, 0x00000002
            };
                for (index = 0; (index < entryCountsByLevel[((s32) lbl_1_data_C8[playerIndex] >= 0x64) ? 0x63 : lbl_1_data_C8[playerIndex]]) && ((s32) lbl_1_data_E8[playerIndex] > 0); index++) {
                    if ((s32) lbl_1_data_C8[playerIndex] >= 0x3C) {

                        scan = ((index == 0) ? (0) : (5));
                        goto convertSpecialEntry;
                    }
                    if ((s32) lbl_1_data_C8[playerIndex] >= 0x2D) {
                        scan = index;
                        goto convertSpecialEntry;
                    }
                    loop = 0;
do {
                    scan = (s32) (6.0f * ((f32) rand8() / 256.0f));
                    loop += 1;
} while ((newRowEntries[scan]->kind != 1) && (loop < 0x180));
                    if (loop < 0x180) {
convertSpecialEntry:
                    newRowEntries[scan]->kind = 2;
                        lbl_1_data_E8[playerIndex] -= 1;
                    }
                }
                if (((s32) lbl_1_data_118[playerIndex] == 0) && ((s32) lbl_1_data_120[playerIndex] == 0) && (lbl_1_bss_0 != 5) && ((s32) lbl_1_bss_45D8.state == 0)) {
                    lbl_1_data_120[playerIndex] = 1;
                    lbl_1_data_118[playerIndex] += 0x3C;
                    lbl_1_data_128[playerIndex] = 0;
                    index = 0;
                    while (index < 0x80) {
                        if (newRowEntries[(scan = rand8() % 6)]->kind == 1) {
                        newRowEntries[scan]->kind = 3;
                        break;
                        } else {
                            index += 1;
                            }
                    }
                    if (index == 0x80) {
                        lbl_1_data_110[playerIndex] += 1;
                    } else {
                        lbl_1_data_110[playerIndex] = (rand8() % 3) + 9;
                    }
                }
                            }
} else if ((lbl_1_bss_8 >= 2) && (scan == 6)) {

{
s32 spawnIntervals[16] = { 0x10, 0xC, 0x9, 0x7, 0x6, 0x5, 0x5, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4 };
                lbl_1_data_100[playerIndex] -= 6;
                while ((s32) lbl_1_data_100[playerIndex] < 0) {
                    index = 0;
do {
                    scan = (s32) (6.0f * ((f32) rand8() / 256.0f));
                    index += 1;
} while ((newRowEntries[scan]->kind != 1) && (index < 0x180));

                    newRowEntries[scan]->kind = (((s32) (rand8() % 3) != 0) ? (7) : (6));
                    lbl_1_data_100[playerIndex] += (s32) ((f32) spawnIntervals[thresholdIndex] + (0.5f + (((f32) spawnIntervals[thresholdIndex] * ((f32) rand8() / 256.0f)) - ((f32) spawnIntervals[thresholdIndex] / 2.0f))));
}
            }
}
                    }
}
}
}
}
        if ((lbl_1_bss_8 == 1) && (lbl_1_bss_0 == 2)) {
            {
            s32 levelValues[100] = {
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000014, 0x00000013, 0x00000012, 0x00000011, 0x00000010, 0x0000000F, 0x0000000E, 0x0000000D, 0x0000000C,
                0x00000014, 0x00000014, 0x00000013, 0x00000013, 0x00000012, 0x00000012, 0x00000011, 0x00000011, 0x00000010, 0x00000010,
                0x00000000, 0x00000014, 0x00000013, 0x00000012, 0x00000011, 0x00000010, 0x0000000F, 0x0000000E, 0x0000000D, 0x0000000C,
                0x00000014, 0x00000014, 0x00000013, 0x00000013, 0x00000012, 0x00000012, 0x00000011, 0x00000011, 0x00000010, 0x00000010
            };

            spawnInterval = (&levelValues[0])[(((s32) lbl_1_data_C8[playerIndex] >= 0x64) ? (0x63) : (lbl_1_data_C8[playerIndex]))];
            if (spawnInterval > 0) {
                spawnInterval *= 0xF;
                if ((s32) (lbl_1_bss_4 % spawnInterval) == 0) {
                    lbl_1_bss_66A8[playerIndex].pendingCount[2] += 1;
                }
            }
                    }
}
        if (((s32) lbl_1_data_80[playerIndex] == 0) && ((s32) lbl_1_bss_66A8[playerIndex].pendingCount[2] > 0) && ((s32) lbl_1_data_B8[playerIndex] == 0) && (lbl_1_bss_0 == 2) && ((u32) lbl_1_bss_66F0[playerIndex] == 0)) {
            f32 rowLandingColumns[6];
            M678LinkEntry *spawnedFallingEntries[6];
            index = 0;
            while (index < 6) {
                (&rowLandingColumns[0])[index] = 13.875f;
                index += 1;
            }
            index = 0;
            while (index < 0x66) {
                if (((s32) lbl_1_bss_70A8[playerIndex][index].kind != 0) && ((s32) lbl_1_bss_70A8[playerIndex][index].category != 5)) {
                    (&rowLandingColumns[0])[((s32) ((s32) (0.5f + lbl_1_bss_70A8[playerIndex][index].gridRow)))] -= 1.0f;
                }
                index += 1;
            }

{
s32 chosenRows[6] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

{
s32 spawnRows[3] = { 0x0, 0x0, 0x0 };
            secondQueueRetryCount = 0;
            index = 0;
            while (index < 3) {
                do {
retrySpawnRow:
                    if ((s32) chosenRows[(scan = (s32) (6.0f * ((f32) rand8() / 256.0f)))] != 0) {
                        goto retrySpawnRow;
                    }
                } while (lbl_1_data_1AC[playerIndex].bytes[scan] != 0);
                chosenRows[scan] = lbl_1_data_1AC[playerIndex].bytes[scan] = 1;
                spawnRows[index] = scan;
                scan = 0;
                while (scan < 6) {
                    if (!(lbl_1_data_1AC[playerIndex].bytes[scan] != 0)) {
                        break;
                    }
                        scan += 1;

                }
                if (scan == 6) {
                    scan = 0;
                    while (scan < 6) {
                        lbl_1_data_1AC[playerIndex].bytes[scan] = 0;
                        scan += 1;
                    }
                }
                index += 1;
}
            index = 0;
            scan = 0;
            while ((index < 0x66) && (scan < 3)) {
                if ((s32) lbl_1_bss_70A8[playerIndex][index].kind == 0) {
                    spawnedFallingEntry = &lbl_1_bss_70A8[playerIndex][index];
                    spawnedFallingEntry->kind = 4;
                    spawnedFallingEntry->variant = 0;
                    spawnedFallingEntry->gridRow = (f32) spawnRows[scan];
                    spawnedFallingEntry->gridColumn = 16.0f;
                    spawnedFallingEntry->fallSpeed = (f32) ((-1.0f * ((sqrtf(0.0009765625f + (4.0f * (2.0f * (0.03125f * (4.0f + (spawnedFallingEntry->gridColumn - (&rowLandingColumns[0])[((s32) (spawnRows[scan]))])))))) - 0.03125f) / 2.0f)) - 0.03125f);
                    spawnedFallingEntry->unk10 = -1;
                    spawnedFallingEntry->category = 6;
                    spawnedFallingEntry->age = 0;
                    spawnedFallingEntry->fadeTimer = 0;
                    spawnedFallingEntry->groupMaximum = 0;
                    spawnedFallingEntry->stillFrames = 0;
                    spawnedFallingEntry->previousColumn = (f32) spawnedFallingEntry->gridColumn;
                    spawnedFallingEntry->releaseSerial = -1;
                    spawnedFallingEntry->overlay = 1;
                    spawnedFallingEntry->opacityScale = 1.0f;
                    (&spawnedFallingEntries[0])[scan] = spawnedFallingEntry;
                    scan += 1;
                }
                index += 1;
            }
            index = 0;
            while ((index < scan) && (index < (s32) lbl_1_bss_5E90[playerIndex].entries[1][lbl_1_bss_5E80[playerIndex].read[1]]) && (secondQueueRetryCount < 0x400)) {
                spawnedEntryIndex = rand8() % scan;
                if ((s32) (&spawnedFallingEntries[0])[spawnedEntryIndex]->kind == 4) {
                    (&spawnedFallingEntries[0])[spawnedEntryIndex]->kind = 5;
                } else {
                    index -= 1;
                    secondQueueRetryCount += 1;
                }
                index += 1;
            }
            lbl_1_bss_66A8[playerIndex].pendingCount[2] -= 1;
            lbl_1_bss_6690[playerIndex].fallingQueueElapsedFrames = 0;
            lbl_1_bss_5E80[playerIndex].read[1] += 1;
            lbl_1_bss_5E80[playerIndex].read[1] %= 0x80;
        }
}
}
        lbl_1_bss_6690[playerIndex].rowQueueElapsedFrames += 1;
        lbl_1_bss_6690[playerIndex].fallingQueueElapsedFrames += 1;
        index = 0;
        while (index < 0x66) {
            sortingEntry = &lbl_1_bss_70A8[playerIndex][index];
            rowInsertionNext = NULL;
            rowInsertionPrevious = NULL;
            sortingEntry->sequenceGroup = 0;
            sortingEntry->sequencePreviousInGroup = sortingEntry->sequenceNextInGroup = 0;
            sortingEntry->sequenceNextInRow = sortingEntry->sequencePreviousInRow = NULL;
            if ((s32) sortingEntry->kind != 0) {
                sortingEntry->age += (sortingEntry->age < 0xFFFF);
                if (((s32) sortingEntry->category != 5) && ((s32) sortingEntry->category != 6)) {
        rowInsertionNext = rowHeads[(s32) sortingEntry->gridRow];
                    while (rowInsertionNext) {
                        if (!(rowInsertionNext->gridColumn > sortingEntry->gridColumn)) {
                            break;
                        }
                            rowInsertionPrevious = rowInsertionNext;
                            rowInsertionNext = rowInsertionNext->sequenceNextInRow;

                    }
                    sortingEntry->sequenceNextInRow = rowInsertionNext;
                    if (rowInsertionNext) {
                        rowInsertionNext->sequencePreviousInRow = sortingEntry;
                    }
                    sortingEntry->sequencePreviousInRow = rowInsertionPrevious;
                    if (rowInsertionPrevious) {
                        rowInsertionPrevious->sequenceNextInRow = sortingEntry;
                    }
                    if (rowInsertionPrevious == 0) {
                        rowHeads[(s32) sortingEntry->gridRow] = fallHeads[(s32) sortingEntry->gridRow] = sortingEntry;
                    }
                }
            }
            index += 1;
}
        index = 0;
        while (index < 0x66) {
            linkedEntry = &lbl_1_bss_70A8[playerIndex][index];
            if ((s32) linkedEntry->kind != 0) {
                linkedEntry->previousFalling = linkedEntry->sequencePreviousInRow;
                linkedEntry->nextFalling = linkedEntry->sequenceNextInRow;
            }
            index += 1;
        }
        index = 0;
        while (index < 0x66) {
            fallingSortEntry = &lbl_1_bss_70A8[playerIndex][index];
            fallingInsertionNext = NULL;
            fallingInsertionPrevious = NULL;
            if (((s32) fallingSortEntry->kind != 0) && ((s32) fallingSortEntry->category == 6)) {
                fallingInsertionNext = fallHeads[(s32) fallingSortEntry->gridRow];
                while (fallingInsertionNext) {
                    if (!(fallingInsertionNext->gridColumn > fallingSortEntry->gridColumn)) {
                        break;
                    }
                        fallingInsertionPrevious = fallingInsertionNext;
                        fallingInsertionNext = fallingInsertionNext->nextFalling;

                }
                fallingSortEntry->nextFalling = fallingInsertionNext;
                if (fallingInsertionNext) {
                    fallingInsertionNext->previousFalling = fallingSortEntry;
                }
                fallingSortEntry->previousFalling = fallingInsertionPrevious;
                if (fallingInsertionPrevious) {
                    fallingInsertionPrevious->nextFalling = fallingSortEntry;
                }
                if (fallingInsertionPrevious == 0) {
                    fallHeads[(s32) fallingSortEntry->gridRow] = fallingSortEntry;
                }
            }
            index += 1;
}
        index = 0;
        while (index < 6) {
            rowTail = fallHeads[index];
            while (rowTail && rowTail->nextFalling) {
                rowTail = rowTail->nextFalling;
            }
            fallHeads[index] = rowTail;
            index += 1;
        }
        index = 0;
        while (index < 6) {
            rowGridEntry = rowHeads[index];
            while (rowGridEntry) {
                if ((rowGridEntry->category != 2) && (rowGridEntry->category != 5) && (rowGridEntry->category != 6)) {
                    if (((s32) rowCount[index] >= -0xF) || ((s32) rowCount[index] < 0xF)) {
                        rowEntryGrid[index][rowCount[index] + 0xF] = rowGridEntry;
                    }
                    rowCount[index] += 1;
                }
                rowGridEntry = rowGridEntry->sequenceNextInRow;
            }
            index += 1;
        }
        lbl_1_data_C0[playerIndex] = 0;
        lbl_1_data_B8[playerIndex] = 0;
        index = 0;
        while (index < 0x66) {
            gridCandidate = &lbl_1_bss_70A8[playerIndex][index];
            if ((gridCandidate->kind != 0) && ((gridCandidate->category == 0) || (gridCandidate->category == 1)) && (gridCandidate->category != 5) && (gridCandidate->category != 6) && (gridCandidate->gridColumn == (f32) (s32) gridCandidate->gridColumn)) {
                entryGridRow = (s32) gridCandidate->gridRow;
                entryGridColumn = (s32) (15.0f + gridCandidate->gridColumn) - 0xF;
                if ((entryGridRow >= 0) && (entryGridRow < 6) && (entryGridColumn >= -0xF) && (entryGridColumn < 0xF)) {
                    entryGrid[entryGridRow][entryGridColumn + 0xF] = gridCandidate;
                }
            }
            index += 1;
        }
        scan = 0;
        while (scan < 0x66) {
            connectingEntry = &lbl_1_bss_70A8[playerIndex][scan];
            if (((s32) connectingEntry->kind != 0) && ((s32) connectingEntry->category != 5) && ((s32) connectingEntry->category != 6)) {
                index = 0;
                while (index < 2) {
                    neighborEntry = NULL;
                    if (index == 0) {
                        if ((void *) connectingEntry->sequenceNextInRow && ((s32) connectingEntry->sequenceNextInRow->kind != 0) && (connectingEntry->sequenceNextInRow->gridColumn == (connectingEntry->gridColumn - 1.0f)) && (connectingEntry->sequenceNextInRow->fallSpeed >= connectingEntry->fallSpeed)) {
                            neighborEntry = connectingEntry->sequenceNextInRow;
                        }
                    } else if (connectingEntry->gridRow < 5.0f) {
            neighborEntry = rowHeads[(s32) connectingEntry->gridRow + 1];
                        while (neighborEntry) {
                            if (!((connectingEntry == neighborEntry) || ((1.0f + connectingEntry->gridRow) != neighborEntry->gridRow) || (connectingEntry->gridColumn != neighborEntry->gridColumn) || (connectingEntry->fallSpeed != neighborEntry->fallSpeed))) {
                                break;
                            }
                                neighborEntry = neighborEntry->sequenceNextInRow;

                        }
                    }
                    if (neighborEntry && ((s32) neighborEntry->kind != 0) && ((s32) neighborEntry->category != 5) && ((s32) neighborEntry->category != 6)) {
                        if (index == 0) {
                            adjacentEntries[connectingEntry->index][0] = neighborEntry;
                            adjacentEntries[neighborEntry->index][2] = connectingEntry;
                        } else {
                            adjacentEntries[connectingEntry->index][1] = neighborEntry;
                            adjacentEntries[neighborEntry->index][3] = connectingEntry;
                        }
                        if (((s32) connectingEntry->variant != 0) && ((s32) neighborEntry->variant != 0) && ((s32) connectingEntry->variant == (s32) neighborEntry->variant) && (((void *) connectingEntry->sequenceGroup == 0) || ((void *) neighborEntry->sequenceGroup == 0) || ((void *) connectingEntry->sequenceGroup != (void *) neighborEntry->sequenceGroup))) {
                            firstGroupRoot = connectingEntry->sequenceGroup;
                            neighborGroupRoot = neighborEntry->sequenceGroup;
                            while (firstGroupRoot && firstGroupRoot->parent) {
                                firstGroupRoot = firstGroupRoot->parent;
                            }
                            while (neighborGroupRoot && (void *) neighborGroupRoot->parent) {
                                neighborGroupRoot = neighborGroupRoot->parent;
                            }

                            mergedGroup = (((((firstGroupRoot == 0) && (neighborGroupRoot == 0))) != 0) ? (NULL) : (((firstGroupRoot == 0) ? (neighborGroupRoot) : (firstGroupRoot))));
                            if (firstGroupRoot && neighborGroupRoot && (firstGroupRoot != neighborGroupRoot)) {
                                neighborGroupRoot->parent = firstGroupRoot;
                                firstGroupRoot->last->sequenceNextInGroup = (void *) neighborGroupRoot->first;
                                neighborGroupRoot->first->sequencePreviousInGroup = (void *) firstGroupRoot->last;
                                firstGroupRoot->last = (void *) neighborGroupRoot->last;
                                firstGroupRoot->count += neighborGroupRoot->count;

                                firstGroupRoot->maximum = (((s32) firstGroupRoot->maximum > (s32) neighborGroupRoot->maximum) ? (firstGroupRoot->maximum) : (neighborGroupRoot->maximum));

                                firstGroupRoot->minimum = (((s32) firstGroupRoot->minimum < (s32) neighborGroupRoot->minimum) ? (firstGroupRoot->minimum) : (neighborGroupRoot->minimum));
                            } else if (mergedGroup == 0) {
                                mergedGroup = &lbl_1_bss_2F88[playerIndex].entries[lbl_1_bss_2F80[playerIndex]];
                                mergedGroup->parent = 0;
                                mergedGroup->count = 2;
                                mergedGroup->first = connectingEntry;
                                mergedGroup->last = neighborEntry;

                                mergedGroup->maximum = (((s32) connectingEntry->groupMaximum > (s32) neighborEntry->groupMaximum) ? (connectingEntry->groupMaximum) : (neighborEntry->groupMaximum));
                                mergedGroup->minimum = 0;
                                lbl_1_bss_2F80[playerIndex] += lbl_1_bss_2F80[playerIndex] < 0x66;
                                connectingEntry->sequenceNextInGroup = neighborEntry;
                                neighborEntry->sequencePreviousInGroup = connectingEntry;
                                connectingEntry->sequenceGroup = neighborEntry->sequenceGroup = mergedGroup;

                                mergedGroup->maximum = (((s32) connectingEntry->groupMaximum > (s32) mergedGroup->maximum) ? (connectingEntry->groupMaximum) : ((((s32) neighborEntry->groupMaximum > (s32) mergedGroup->maximum) ? (neighborEntry->groupMaximum) : (mergedGroup->maximum))));
                            } else if (mergedGroup == firstGroupRoot) {
                                groupMember = firstGroupRoot->first;
                                while (groupMember && (groupMember != neighborEntry)) {
                                    groupMember = groupMember->sequenceNextInGroup;
                                }
                                if (groupMember != neighborEntry) {
                                    firstGroupRoot->count += 1;

                                    firstGroupRoot->maximum = (((s32) neighborEntry->groupMaximum > (s32) firstGroupRoot->maximum) ? (neighborEntry->groupMaximum) : (firstGroupRoot->maximum));
                                    firstGroupRoot->last->sequenceNextInGroup = neighborEntry;
                                    neighborEntry->sequencePreviousInGroup = (void *) firstGroupRoot->last;
                                    neighborEntry->sequenceGroup = firstGroupRoot;
                                    firstGroupRoot->last = neighborEntry;
                                }
                            } else if (mergedGroup == neighborGroupRoot) {
                                otherMember = neighborGroupRoot->first;
                                while (otherMember && (otherMember != connectingEntry)) {
                                    otherMember = otherMember->sequenceNextInGroup;
                                }
                                if (otherMember != connectingEntry) {
                                    neighborGroupRoot->count += 1;

                                    neighborGroupRoot->maximum = (((s32) neighborGroupRoot->maximum > (s32) connectingEntry->groupMaximum) ? (neighborGroupRoot->maximum) : (connectingEntry->groupMaximum));
                                    neighborGroupRoot->last->sequenceNextInGroup = connectingEntry;
                                    connectingEntry->sequencePreviousInGroup = (void *) neighborGroupRoot->last;
                                    connectingEntry->sequenceGroup = neighborGroupRoot;
                                    neighborGroupRoot->last = connectingEntry;
                                }
                            }
                        }
                    }
                    index += 1;
}
            }
            scan += 1;
}
        effectTravelFrames = 0x19;
        index = 0;
        while (index < (s32) lbl_1_bss_2F80[playerIndex]) {
            matchedGroup = &lbl_1_bss_2F88[playerIndex].entries[index];
            matchedUntimedCount = 0;
            if (((u32) matchedGroup->parent == 0U) && ((s32) matchedGroup->count >= 5)) {
                matchedGroupEntry = matchedGroup->first;
                minimumGroupTimer = 0xFFFF;
                maximumGroupTimer = 0;

{
s32 matchSound[5] = { 0x0, 0x0, 0x0, 0x0, 0x0 };

{
s32 matchSoundId[5] = { 0x6, 0x8, 0xC, 0x14, 0x12 };
                while (matchedGroupEntry) {
                    if ((s32) matchedGroupEntry->overlay == 1) {
                        if ((lbl_1_bss_8 == 1) && ((s32) matchedGroupEntry->kind == 2)) {
                            lbl_1_data_D0[playerIndex] -= 1;
                            scan = 0;
                            while (scan < 0x14) {
                                if ((s32) lbl_1_bss_45FC[scan].state == 0) {
                                    lbl_1_bss_45FC[scan].state = 1;
                                    lbl_1_bss_45FC[scan].frame = 0;
                                    lbl_1_bss_45FC[scan].x = lbl_1_bss_45FC[scan].startX = matchedGroupEntry->gridRow;
                                    lbl_1_bss_45FC[scan].y = lbl_1_bss_45FC[scan].startY = matchedGroupEntry->gridColumn + lbl_1_data_68[playerIndex];
                                    lbl_1_bss_45FC[scan].arc = lbl_1_bss_45FC[scan].unk20 = 0.0f;
                                    effectRowDistance = lbl_1_bss_45FC[scan].x - 8.3f;
                                    effectColumnDistance = lbl_1_bss_45FC[scan].y - 12.75f;
                                    pathDist = sqrtf(((f32) (effectRowDistance * effectRowDistance)) + ((f32) (effectColumnDistance * effectColumnDistance)));
                                    lbl_1_bss_45FC[scan].duration = effectTravelFrames;

                                    effectTravelFrames = effectTravelFrames + ((effectTravelFrames < 0x20) ? (3) : (2));
                                    break;
                                }
                                scan += 1;
                            }
                        }
                        if ((lbl_1_bss_8 == 1) && ((s32) matchedGroupEntry->kind == 3)) {
                            lbl_1_data_1D8[playerIndex] = 0;
                            lbl_1_bss_45D8.state = 1;
                            lbl_1_bss_45D8.frame = 0;
                            lbl_1_bss_45D8.x = lbl_1_bss_45D8.startX = matchedGroupEntry->gridRow;
                            lbl_1_bss_45D8.y = lbl_1_bss_45D8.startY = matchedGroupEntry->gridColumn + lbl_1_data_68[playerIndex];
                            lbl_1_bss_45D8.arc = lbl_1_bss_45D8.unk20 = 0.0f;
                            lbl_1_data_128[playerIndex] = 0xFFFF;
                        }
                        lbl_1_data_118[playerIndex] -= lbl_1_data_118[playerIndex] > 0;
                        matchSound[0] = 1;
                        switch (matchedGroupEntry->kind) {        /* switch 2; irregular */
                        case 2:                     /* switch 2 */
                            matchSound[1] = 1;
                            break;
                        case 3:                     /* switch 2 */
                            matchSound[2] = 1;
                            break;
                        case 6:                     /* switch 2 */
                            matchSound[3] = 1;
                            break;
                        case 7:                     /* switch 2 */
                            matchSound[4] = 1;
                            break;
                        }
                        matchedGroupEntry->overlay = 0;
                    }
                    if ((s32) matchedGroupEntry->fadeTimer == 0) {
                        matchedRow = matchedGroupEntry->gridRow;
                        matchedColumn = matchedGroupEntry->gridColumn;
                        matchedUntimedCount += 1;
                    }

                    maximumGroupTimer = (((s32) matchedGroupEntry->fadeTimer > maximumGroupTimer) ? (matchedGroupEntry->fadeTimer) : (maximumGroupTimer));
                    if ((s32) matchedGroupEntry->fadeTimer == 0) {
                        playerWorks[playerIndex]->pendingRumble = 1;
                        matchSound[0] = 1;
                    }

                    matchedGroupEntry->fadeTimer = (((s32) matchedGroupEntry->fadeTimer == 0) ? (1) : (matchedGroupEntry->fadeTimer));

                    minimumGroupTimer = ((minimumGroupTimer > (s32) matchedGroupEntry->fadeTimer) ? (matchedGroupEntry->fadeTimer) : (minimumGroupTimer));
                    matchedGroupEntry = matchedGroupEntry->sequenceNextInGroup;
}
                if ((matchSound[0] != 0) && ((s32) lbl_1_bss_174[playerIndex + 6].count < 3)) {
                    lbl_1_bss_174[playerIndex + 6].count += 1;
                }
                if ((matchSound[1] != 0) && ((s32) lbl_1_bss_174[matchSoundId[1] + playerIndex].count < 3)) {
                    lbl_1_bss_174[matchSoundId[1] + playerIndex].count += 1;
                }
                if ((matchSound[2] != 0) && ((s32) lbl_1_bss_174[matchSoundId[2] + playerIndex].count < 3)) {
                    lbl_1_bss_174[matchSoundId[2] + playerIndex].count += 1;
                }
                if ((matchSound[3] != 0) && ((s32) lbl_1_bss_174[matchSoundId[3] + playerIndex].count < 3)) {
                    lbl_1_bss_174[matchSoundId[3] + playerIndex].count += 1;
                }
                if ((matchSound[4] != 0) && ((s32) lbl_1_bss_174[matchSoundId[4] + playerIndex].count < 3)) {
                    lbl_1_bss_174[matchSoundId[4] + playerIndex].count += 1;
                }
                matchedGroupEntry = matchedGroup->first;

                matchedGroup->maximum += ((maximumGroupTimer == 0) ? (1) : (0));
                matchedGroup->minimum = minimumGroupTimer;
                while (matchedGroupEntry) {
                    matchedGroupEntry->fadeTimer = minimumGroupTimer;
                    matchedGroupEntry->groupMaximum = (s32) matchedGroup->maximum;
                    if (minimumGroupTimer == 1) {
                        scan = 0;
                        while (scan < 4) {
                        if (((u32) adjacentEntries[matchedGroupEntry->index][scan] != 0U) && (((s32) adjacentEntries[matchedGroupEntry->index][scan]->kind == 4) || ((s32) adjacentEntries[matchedGroupEntry->index][scan]->kind == 5)) && ((s32) adjacentEntries[matchedGroupEntry->index][scan]->fadeTimer == 0)) {
                            adjacentEntries[matchedGroupEntry->index][scan]->fadeTimer = (s32) lbl_1_data_130[playerIndex];
                                if ((s32) lbl_1_bss_174[playerIndex + 0x1A].count < 3) {
                                    lbl_1_bss_174[playerIndex + 0x1A].count += 1;
                                }
                            }
                            scan += 1;
                        }
                    }
                    matchedGroupEntry = matchedGroupEntry->sequenceNextInGroup;
                }
                groupChangedFlags[playerIndex] = 1;
            }
}
}
            index += 1;
}
        index = 0;
        while (index < 0x66) {
            timerEntry = &lbl_1_bss_70A8[playerIndex][index];
            if (((s32) timerEntry->kind != 0) && ((s32) timerEntry->groupMaximum > 0)) {
                lbl_1_data_B8[playerIndex] = 1;
            }
            if (((s32) timerEntry->kind != 0) && ((s32) timerEntry->fadeTimer > 0)) {
                timerEntry->fadeTimer += 1;
                lbl_1_data_C0[playerIndex] = 1;
            }
            index += 1;
        }
        if (lbl_1_bss_8 >= 2) {
            index = 0;
            while (index < (s32) lbl_1_bss_2F80[playerIndex]) {
                expiringGroup = &lbl_1_bss_2F88[playerIndex].entries[index];
                if (((u32) expiringGroup->parent == 0U) && ((s32) expiringGroup->minimum >= (s32) (lbl_1_data_130[playerIndex] - 1))) {
                    expiringEntry = expiringGroup->first;
                    while (expiringEntry) {
                        if (((s32) expiringEntry->kind >= 6) && ((s32) expiringEntry->kind <= 7)) {

                            recipientPlayer = ((playerIndex + ((((s32) rand8() % 2) != 0) ? (-1) : (1))) + lbl_1_bss_8) % lbl_1_bss_8;
                            queuedKindIndex = expiringEntry->kind - 6;
                            if ((s32) (((s32) ((u32) (lbl_1_bss_5E70[recipientPlayer].write[queuedKindIndex] + 1)) % 0x80)) != (s32) lbl_1_bss_5E80[recipientPlayer].read[queuedKindIndex]) {
                                lbl_1_bss_66A8[recipientPlayer].pendingCount[queuedKindIndex + 1] += 1;
                                lbl_1_bss_5E90[recipientPlayer].entries[queuedKindIndex][lbl_1_bss_5E70[recipientPlayer].write[queuedKindIndex]] = 0;
                                lbl_1_bss_5E70[recipientPlayer].write[queuedKindIndex] += 1;
                                lbl_1_bss_5E70[recipientPlayer].write[queuedKindIndex] %= 0x80;
                            }
                        }
                        expiringEntry = expiringEntry->sequenceNextInGroup;
                    }
                }
                index += 1;
            }
        }
        if (lbl_1_bss_8 == 1) {
            index = 0;
            while (index < 0x14) {
                if ((s32) lbl_1_bss_45FC[index].state == 0) {
                    HuSprAttrSet((s16) lbl_1_bss_120, (s16) index, 4);
                } else {
                    HuSprAttrReset((s16) lbl_1_bss_120, (s16) index, 4);
                }
                if ((s32) lbl_1_bss_45FC[index].state == 1) {
    Point3d spriteWorldPosition;
    Point3d spriteScreenPosition;

                    lbl_1_bss_45FC[index].frame += 1;
                    effectTravelFraction = (f32) lbl_1_bss_45FC[index].frame / (f32) lbl_1_bss_45FC[index].duration;
                    spriteBlendRemainder = 1.0f - effectTravelFraction;
                    lbl_1_bss_45FC[index].x = (8.3f * effectTravelFraction) + (spriteBlendRemainder * lbl_1_bss_45FC[index].startX);
                    lbl_1_bss_45FC[index].y = (12.75f * effectTravelFraction) + (spriteBlendRemainder * lbl_1_bss_45FC[index].startY);
                    lbl_1_bss_45FC[index].arc = 1.0f - (((effectTravelFraction - 0.5f) * (effectTravelFraction - 0.5f)) / 0.25f);
                    lbl_1_bss_45FC[index].y -= 2.5f * lbl_1_bss_45FC[index].arc;
                    spriteScale = 0.666f + lbl_1_bss_45FC[index].arc;

                    spriteWorldPosition.x = ((f32) ((lbl_1_bss_8 <= 1) ? (0) : (-0x15E)) + (60.0f * lbl_1_bss_45FC[index].x)) - 150.0f;
                    spriteWorldPosition.y = -54.0f * (1.5f + ((0.5f + lbl_1_bss_45FC[index].y) - 15.0f));
                    spriteWorldPosition.z = 0.0f;
                    Hu3D3Dto2D(&spriteWorldPosition, 1, &spriteScreenPosition);
                    HuSprPosSet((s16) lbl_1_bss_120, (s16) index, spriteScreenPosition.x, spriteScreenPosition.y);
                    HuSprScaleSet((s16) lbl_1_bss_120, (s16) index, spriteScale, spriteScale);
                    HuSprZRotSet((s16) lbl_1_bss_120, (s16) index, (f32) ((s32) (lbl_1_bss_45FC[index].frame * 0x14)));
                    HuSprPriSet((s16) lbl_1_bss_120, (s16) index, 0x33);
                    HuSprTPLvlSet((s16) lbl_1_bss_120, (s16) index, 0.7f);
                    if ((s32) lbl_1_bss_45FC[index].frame >= (s32) lbl_1_bss_45FC[index].duration) {
                        lbl_1_bss_45FC[index].state = 2;
                        lbl_1_bss_45FC[index].frame = 0;
                        lbl_1_data_D8[playerIndex] -= 1;
                        if ((s32) lbl_1_bss_174[playerIndex + 0xA].count < 3) {
                            lbl_1_bss_174[playerIndex + 0xA].count += 1;
                        }
                    }
                } else if ((s32) lbl_1_bss_45FC[index].state == 2) {
                    lbl_1_bss_45FC[index].frame += 1;
                    effectFadeFraction = (f32) lbl_1_bss_45FC[index].frame / 6.0f;
                    endingSpriteFraction = (0.5f * effectFadeFraction) + (0.5f * (1.0f - ((1.0f - effectFadeFraction) * (1.0f - effectFadeFraction))));
                    endingSpriteScale = 0.666f + 3.0f * endingSpriteFraction;
                    if ((s32) lbl_1_bss_45FC[index].frame <= 6) {
                        HuSprPriSet((s16) lbl_1_bss_120, (s16) index, 0x46);
                        HuSprTPLvlSet((s16) lbl_1_bss_120, (s16) index, 0.75f * (1.0f - effectFadeFraction));
                        HuSprScaleSet((s16) lbl_1_bss_120, (s16) index, endingSpriteScale, endingSpriteScale);
                    } else {
                        HuSprAttrSet((s16) lbl_1_bss_120, (s16) index, 4);
                    }
                    if ((s32) lbl_1_bss_45FC[index].frame > 0xA) {
                        lbl_1_bss_45FC[index].state = 0;
                        lbl_1_bss_45FC[index].frame = 0;
                    }
                }
                index += 1;
            }
        }
        if (lbl_1_bss_8 == 1) {
            resultEffectState = lbl_1_bss_45D8.state;
            if ((s32) lbl_1_bss_45D8.state == 0) {
                HuSprAttrSet((s16) lbl_1_bss_11C, 0, 4);
            } else {
                HuSprAttrReset((s16) lbl_1_bss_11C, 0, 4);
            }
            if ((s32) lbl_1_bss_45D8.state == 1) {
    Point3d resultWorldPosition;
    Point3d resultScreenPosition;

                lbl_1_bss_45D8.frame += 1;
                resultTravelFraction = (f32) lbl_1_bss_45D8.frame / 30.0f;
                resultTravelRemainder = 1.0f - resultTravelFraction;
                lbl_1_bss_45D8.x = (f32) ((-4.38f * resultTravelFraction) + (lbl_1_bss_45D8.startX * resultTravelRemainder));
                lbl_1_bss_45D8.y = (f32) ((4.0f * resultTravelFraction) + (lbl_1_bss_45D8.startY * resultTravelRemainder));
                lbl_1_bss_45D8.arc = (f32) (1.0f - (((resultTravelFraction - 0.5f) * (resultTravelFraction - 0.5f)) / 0.25f));
                lbl_1_bss_45D8.x -= (2.5f * lbl_1_bss_45D8.arc);
                lbl_1_bss_45D8.y += (4.0f * lbl_1_bss_45D8.arc);

                resultWorldPosition.x = ((60.0f * lbl_1_bss_45D8.x) + (f32) ((lbl_1_bss_8 <= 1) ? (0) : (-0x15E))) - 150.0f;
                resultWorldPosition.y = -54.0f * (1.5f + ((0.5f + lbl_1_bss_45D8.y) - 15.0f));
                resultWorldPosition.z = 0.0f;
                Hu3D3Dto2D(&resultWorldPosition, 1, &resultScreenPosition);
                HuSprPosSet((s16) lbl_1_bss_11C, 0, resultScreenPosition.x, resultScreenPosition.y);
                HuSprScaleSet((s16) lbl_1_bss_11C, 0, 1.0f, 1.0f);
                HuSprZRotSet((s16) lbl_1_bss_11C, 0, 360.0f * ((f32) lbl_1_bss_45D8.frame / 30.0f));
                HuSprPriSet((s16) lbl_1_bss_11C, 0, 0x33);
                HuSprTPLvlSet((s16) lbl_1_bss_11C, 0, 1.0f);
                if ((s32) lbl_1_bss_45D8.frame >= 0x1E) {
                    lbl_1_bss_45D8.state = 2;
                    lbl_1_bss_45D8.frame = 0;
                    if (lbl_1_bss_0 != 4) {
                        lbl_1_data_F0[playerIndex] = (((lbl_1_data_F0[playerIndex] + 0x3B) / 60) + 0xF) * 0x3C;

                        lbl_1_data_F0[playerIndex] = (((s32) lbl_1_data_F0[playerIndex] < (s32) lbl_1_data_F8[playerIndex]) ? (lbl_1_data_F0[playerIndex]) : (lbl_1_data_F8[playerIndex]));
                    }
                    if ((s32) lbl_1_bss_174[playerIndex + 0xE].count < 3) {
                        lbl_1_bss_174[playerIndex + 0xE].count += 1;
                    }
                }
            }
            if (resultEffectState == 2) {
                lbl_1_bss_45D8.frame += 1;
                resultFadeFraction = (f32) lbl_1_bss_45D8.frame / 6.0f;
                resultFadeCurve = (0.5f * resultFadeFraction) + (0.5f * (1.0f - ((1.0f - resultFadeFraction) * (1.0f - resultFadeFraction))));
                resultSpriteScale = 0.5f + (3.0f * resultFadeCurve);
                if ((s32) lbl_1_bss_45D8.frame <= 6) {
                    HuSprPriSet((s16) lbl_1_bss_11C, 0, 0x65);
                    HuSprTPLvlSet((s16) lbl_1_bss_11C, 0, 0.6f * (1.0f - resultFadeFraction));
                    HuSprScaleSet((s16) lbl_1_bss_11C, 0, resultSpriteScale, resultSpriteScale);
                } else {
                    HuSprAttrSet((s16) lbl_1_bss_11C, 0, 4);
                }
                if ((s32) lbl_1_bss_45D8.frame > 0xA) {
                    lbl_1_bss_45D8.state = 0;
                    lbl_1_bss_45D8.frame = 0;
                }
            }
        }
        for (index = 0; index < 6; index++) {
            removedCount = 0;
            rowEntry = rowHeads[index];
            while (rowEntry) {
                if (rowEntry->category != 0) {
                    removedCount = 0;
                }
                if ((rowEntry->kind != 0) && (rowEntry->fadeTimer > 0)) {
                    groupChangedFlags[playerIndex] = 1;
                    if (rowEntry->fadeTimer >= lbl_1_data_130[playerIndex]) {
                        rowEntry->fadeTimer = 0;
                        if ((rowEntry->kind == 4) || (rowEntry->kind == 5)) {
                            loop = 0;
                            while (loop < 0xCC) {
                                if ((s32) lbl_1_bss_48CC[playerIndex].entries[loop].textureIndex == 0) {
                                    effectRecord = &lbl_1_bss_48CC[playerIndex].entries[loop];
                                    effectRecord->x = (f32) rowEntry->gridRow;
                                    effectRecord->y = (f32) (rowEntry->gridColumn + lbl_1_data_68[playerIndex]);
                                    effectRecord->textureIndex = 1;
                                    break;
                                }
                                loop += 1;
                            }
                        }

                        rowEntry->groupMaximum = ((((rowEntry->sequenceGroup && ((s32) rowEntry->sequenceGroup->maximum > rowEntry->groupMaximum))) != 0) ? (rowEntry->sequenceGroup->maximum) : (rowEntry->groupMaximum));
                        rowEntry->kind = 0;
                        rowEntry->variant = 0;
                        rowEntry->age = 0;
                        removedCount += 1;
                        lbl_1_data_108[playerIndex] += lbl_1_data_108[playerIndex] < 0x7FFFFFFF;
                        if (rowEntry->sequencePreviousInRow) {
                            rowEntry->sequencePreviousInRow->sequenceNextInRow = rowEntry->sequenceNextInRow;
                        }
                        if (rowEntry->sequenceNextInRow) {
                            followingRowEntry = rowEntry->sequenceNextInRow;
                            while (followingRowEntry) {
                                if ((followingRowEntry->kind == 0) || (followingRowEntry->category != 0) || (followingRowEntry->sequenceGroup && (followingRowEntry->sequenceGroup == rowEntry->sequenceGroup))) break;

                                followingRowEntry->groupMaximum = ((followingRowEntry->groupMaximum > rowEntry->groupMaximum) ? (followingRowEntry->groupMaximum) : (rowEntry->groupMaximum));
                                followingRowEntry = followingRowEntry->sequenceNextInRow;
                            }
                            rowEntry->sequenceNextInRow->sequencePreviousInRow = rowEntry->sequencePreviousInRow;
                        }
                    }
                }
                rowEntry = rowEntry->sequenceNextInRow;
            }
        }

{
M678LinkEntry * fallingRowFronts[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
        index = 0;
        while (index < 6) {
            fallingRowFronts[index] = fallHeads[index];
            while ((void *) fallingRowFronts[index] && ((s32) fallingRowFronts[index]->category == 6) && ((u32) fallingRowFronts[index]->previousFalling != 0U) && ((s32) fallingRowFronts[index]->previousFalling->releaseSerial == (s32) fallingRowFronts[index]->releaseSerial)) {
                fallingRowFronts[index] = fallingRowFronts[index]->previousFalling;
            }
            index += 1;
        }
        index = 0;
        while (index < 0x66) {
            movingEntry = &lbl_1_bss_70A8[playerIndex][index];
            if (((s32) movingEntry->kind != 0) && (((s32) movingEntry->category == 0) || ((s32) movingEntry->category == 3) || ((s32) movingEntry->category == 6) || ((s32) movingEntry->category == 4))) {
                movingEntry->previousColumn = (f32) movingEntry->gridColumn;

                movingEntry->fallSpeed += (((s32) movingEntry->category == 6) ? 0.03125f : 0.015625f);

                movingEntry->fallSpeed = ((movingEntry->fallSpeed >= 0.8f) ? 0.8f : (movingEntry->fallSpeed));
            }
            index += 1;
        }
        index = 0;
        while (index < 0x66) {
            fallingEntry = &lbl_1_bss_70A8[playerIndex][index];
            entryBelow = fallingEntry->previousFalling;
            while (entryBelow && ((s32) entryBelow->category == 6) && ((s32) entryBelow->releaseSerial != -1) && (((s32) entryBelow->releaseSerial > (s32) fallingEntry->releaseSerial) || (((s32) entryBelow->releaseSerial == 0) && ((s32) fallingEntry->releaseSerial == 0xFFFF)))) {
                entryBelow = entryBelow->previousFalling;
            }
            if (((s32) fallingEntry->category == 6) && (fallingEntry->fallSpeed < 0.0f) && ((void *) fallingRowFronts[(s32) fallingEntry->gridRow] == fallingEntry) && ((entryBelow == 0) || ((((s32) entryBelow->category != 6) || ((s32) entryBelow->releaseSerial != (s32) fallingEntry->releaseSerial)) && ((entryBelow->gridColumn - 1.25f) > fallingEntry->gridColumn)))) {
                sameFallEntry = fallingEntry;

                floorOverlap = fallingEntry->gridColumn - (((entryBelow) ? (entryBelow->gridColumn) : 15.0f) - 1.25f);
                floorOverlap = floorOverlap > 0.0f ? 0.0f : -floorOverlap;
                while (sameFallEntry) {
                    if (((s32) sameFallEntry->category == 6) && ((s32) sameFallEntry->releaseSerial == (s32) fallingEntry->releaseSerial)) {
                        sameFallEntry->fallSpeed += (0.03125f * floorOverlap);

                        sameFallEntry->fallSpeed = ((sameFallEntry->fallSpeed > 0.0f) ? 0.0f : (sameFallEntry->fallSpeed));
                    }
                    sameFallEntry = sameFallEntry->nextFalling;
                }
            }
            index += 1;
        }
        index = 0;
        while (index < 0x66) {
            integratedEntry = &lbl_1_bss_70A8[playerIndex][index];
            if (((s32) integratedEntry->kind != 0) && (((s32) integratedEntry->category == 0) || ((s32) integratedEntry->category == 3) || ((s32) integratedEntry->category == 6) || ((s32) integratedEntry->category == 4))) {
                integratedEntry->gridColumn += integratedEntry->fallSpeed;
            }
            index += 1;
        }
        loop = 0;
        do {
            repeatResolution = 0;
            {
            s32 visitedEntries[102] = {
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000
            };
            loop += 1;
            index = 0;
            while (index < 0x66) {
            hitEntry = &lbl_1_bss_70A8[playerIndex][index];
                if ((s32) (&visitedEntries[0])[index] == 0) {
                    (&visitedEntries[0])[index] = 1;
                    if ((loop < 0x32) && ((s32) hitEntry->kind != 0) && ((s32) hitEntry->fadeTimer > 0) && (((s32) hitEntry->category == 0) || ((s32) hitEntry->category == 3)) && hitEntry->sequenceGroup) {
                        groupCollisionOffset = 0.0f;
                        groupCollisionSpeed = hitEntry->fallSpeed;
                        collisionCategory = hitEntry->category;
                        collisionTimer = hitEntry->age;
                        fallingGroup = hitEntry->sequenceGroup;
                        while (fallingGroup && (void *) fallingGroup->parent) {
                            fallingGroup = fallingGroup->parent;
                        }
                        collisionGroupEntry = fallingGroup->first;
                        while (collisionGroupEntry) {
                            otherGroup = NULL;
                            limitOrScale = collisionGroupEntry->sequencePreviousInRow ? collisionGroupEntry->sequencePreviousInRow->gridColumn : 15.0f;
                            if ((void *) collisionGroupEntry->sequencePreviousInRow) {
                                otherGroup = collisionGroupEntry->sequencePreviousInRow->sequenceGroup;
                                while (otherGroup && (void *) otherGroup->parent) {
                                    otherGroup = otherGroup->parent;
                                }
                            }
                            if ((otherGroup != fallingGroup) && (collisionGroupEntry->gridColumn > (limitOrScale - 1.0f)) && (((limitOrScale - 1.0f) - collisionGroupEntry->gridColumn) < groupCollisionOffset)) {
                                groupCollisionOffset = (limitOrScale - 1.0f) - collisionGroupEntry->gridColumn;
                                if ((void *) collisionGroupEntry->sequencePreviousInRow == 0) {
                                    groupCollisionSpeed = 0.0f;
                                } else {

                                    groupCollisionSpeed = ((collisionGroupEntry->sequencePreviousInRow->fallSpeed > collisionGroupEntry->fallSpeed) ? (groupCollisionSpeed) : (collisionGroupEntry->sequencePreviousInRow->fallSpeed));
                                }
                            }
                            (&visitedEntries[0])[collisionGroupEntry->index] = 1;
                            collisionGroupEntry = collisionGroupEntry->sequenceNextInGroup;
                        }
                        collisionGroupEntry = fallingGroup->first;
                        while (collisionGroupEntry) {
                            if (groupCollisionOffset < 0.0f) {
                                if (((s32) collisionGroupEntry->kind == 4) && ((s32) collisionGroupEntry->category == 6) && ((s32) lbl_1_bss_174[playerIndex + 0x16].count < 3)) {
                                    lbl_1_bss_174[playerIndex + 0x16].count += 1;
                                }
                                collisionGroupEntry->category = 0;

                                collisionGroupEntry->age = ((collisionCategory == 0) ? (collisionTimer) : (0));
                                collisionGroupEntry->gridColumn += groupCollisionOffset;
                                collisionGroupEntry->fallSpeed = groupCollisionSpeed;
                                collisionGroupEntry->releaseSerial = -1;
                            } else if ((groupCollisionOffset > 0.0f) && (collisionCategory != 3)) {
                                collisionGroupEntry->category = 3;
                                collisionGroupEntry->age = 0;
                                movementChangedFlags[playerIndex] = 1;
                            }
                            collisionGroupEntry = collisionGroupEntry->sequenceNextInGroup;
                        }
                        if (groupCollisionOffset < 0.0f) {
                            repeatResolution = 1;
                        }
                    } else {
                        if (((s32) hitEntry->kind != 0) && (((s32) hitEntry->category == 0) || ((s32) hitEntry->category == 3) || (((s32) hitEntry->category == 6) && (hitEntry->fallSpeed >= 0.0f)))) {
            below = rowHeads[(s32) hitEntry->gridRow];
                         below = below && (below == hitEntry) ? below->sequenceNextInRow : below;
                        while (below && (below->gridColumn >= hitEntry->gridColumn) && (void *) below->sequenceNextInRow && ((((void *) below->sequenceNextInRow != hitEntry) && (below->sequenceNextInRow->gridColumn >= hitEntry->gridColumn)) || (((void *) below->sequenceNextInRow == hitEntry) && (void *) below->sequenceNextInRow->sequenceNextInRow && (below->sequenceNextInRow->sequenceNextInRow->gridColumn >= hitEntry->gridColumn)))) {

                            below = (((void *) below->sequenceNextInRow != hitEntry) ? (below->sequenceNextInRow) : (below->sequenceNextInRow->sequenceNextInRow));
                        }
                        if (below && (below->gridColumn < hitEntry->gridColumn)) {
                            below = NULL;
                        }
                        obstacleColumn = below ? below->gridColumn : 15.0f;
                        if (hitEntry->gridColumn > (obstacleColumn - 1.0f)) {
                            fadingEntryAhead = 0;
                            if ((s32) hitEntry->category == 6) {

                                fadeSearchEntry = ((below) ? (below->sequenceNextInRow) : (NULL));
                                while (fadeSearchEntry) {
                                    if ((fadeSearchEntry != hitEntry) && ((s32) fadeSearchEntry->category != 6) && ((s32) fadeSearchEntry->fadeTimer > 0)) {
                                        fadingEntryAhead = 1;
                                    }
                                    fadeSearchEntry = fadeSearchEntry->sequenceNextInRow;
                                }
                            }
                            if (fadingEntryAhead != 0) {
                                hitEntry->fallSpeed = (f32) ((-1.0f * ((sqrtf(0.50097656f) - 0.03125f) / 2.0f)) - 0.03125f);
                            } else {
                                hitEntry->gridColumn = (f32) (obstacleColumn - 1.0f);
                                repeatResolution = 1;
                                if (below == 0) {
                                    hitEntry->fallSpeed = 0.0f;
                                } else {

                                    hitEntry->fallSpeed = ((below->fallSpeed > hitEntry->fallSpeed) ? (below->fallSpeed) : (hitEntry->fallSpeed));
                                }
                                if ((s32) hitEntry->category == 6) {
                                    prevEntry = NULL;
            nextEntry = rowHeads[(s32) hitEntry->gridRow];
                                    while (1) {
                                        if ((!nextEntry || nextEntry->gridColumn < hitEntry->gridColumn) && (!prevEntry || hitEntry->gridColumn <= prevEntry->gridColumn)) {
                                    if (prevEntry == 0) {
            rowHeads[(s32) hitEntry->gridRow] = hitEntry;
                                    }
                                    hitEntry->sequenceNextInRow = nextEntry;
                                    hitEntry->sequencePreviousInRow = prevEntry;
                                    if (nextEntry) {
                                        nextEntry->sequencePreviousInRow = hitEntry;
                                    }
                                    if (prevEntry) {
                                        prevEntry->sequenceNextInRow = hitEntry;
                                    }
                                            break;
                                        }
                                        prevEntry = nextEntry;
                                        nextEntry = nextEntry->sequenceNextInRow;
                                    }
                                    repeatResolution = 1;
                                }

                                hitEntry->age = (((s32) hitEntry->category != 0) ? (0) : (hitEntry->age));
                                if (((s32) hitEntry->kind == 4) && ((s32) hitEntry->category == 6) && ((s32) lbl_1_bss_174[playerIndex + 0x16].count < 3)) {
                                    lbl_1_bss_174[playerIndex + 0x16].count += 1;
                                }
                                hitEntry->category = 0;
                                hitEntry->fallSpeed = 0.0f;
                                hitEntry->releaseSerial = -1;
                            }
                        } else if ((hitEntry->gridColumn < (obstacleColumn - 1.0f)) && ((s32) hitEntry->category == 0)) {

                            hitEntry->age = (((s32) hitEntry->category != 3) ? (0) : (hitEntry->age));
                            hitEntry->category = 3;
                            hitEntry->fallSpeed = 0.015625f;
                            movementChangedFlags[playerIndex] = 1;
                        }
                        }
                        hitEntry = hitEntry->sequenceNextInRow;
                    }
                }
                index += 1;
}
                    }
} while (repeatResolution != 0);
        index = 0;
        while (index < 6) {
            lbl_1_data_138[playerIndex].rowCounts[index] = 0;
            lbl_1_data_168[playerIndex][index] = 0;
            index += 1;
        }
        index = 0;
        while (index < 0x66) {
            countedEntry = &lbl_1_bss_70A8[playerIndex][index];
            if ((s32) countedEntry->kind != 0) {
                if (((s32) countedEntry->kind == 2) && ((s32) countedEntry->fadeTimer == 0)) {
                    specialEntryCounts[playerIndex] += 1;
                }
                if (((s32) countedEntry->category != 0) || (countedEntry->gridColumn != countedEntry->previousColumn)) {
                    lbl_1_data_C0[playerIndex] = 1;
                }

                countedEntry->stillFrames = ((countedEntry->gridColumn == countedEntry->previousColumn) ? (countedEntry->stillFrames + 1) : (0));
                if (((s32) countedEntry->category != 5) && ((s32) countedEntry->fadeTimer == 0)) {
                    lbl_1_data_138[playerIndex].rowCounts[(s32) countedEntry->gridRow] += 1;
                }
                if (((s32) countedEntry->category == 0) && ((s32) countedEntry->fadeTimer == 0)) {
                    lbl_1_data_168[playerIndex][(s32) countedEntry->gridRow] += 1;
                }
                if ((countedEntry->fallSpeed == 0.0f) && ((s32) countedEntry->category != 5) && ((s32) countedEntry->category != 6) && (((s32) countedEntry->category == 0) || ((s32) countedEntry->category == 1)) && ((s32) countedEntry->stillFrames >= 2) && ((s32) countedEntry->fadeTimer == 0)) {
                    countedEntry->groupMaximum = 0;
                }
            }
            index += 1;
        }
        motionFrame = (s32) fn_8002D4E0(lbl_1_bss_15C[playerIndex]->mdlId[1]);
        fn_800269E8(lbl_1_bss_15C[playerIndex]->mdlId[1], 0x40000002U);
        fn_80026A44(lbl_1_bss_15C[playerIndex]->mdlId[1], 1U);
        if ((lbl_1_data_1EC != -1) || ((lbl_1_bss_0 == 2) && ((s32) lbl_1_data_F0[playerIndex] == 0) && (lbl_1_bss_14 % 2 != 0))) {
            fn_800269E8(lbl_1_bss_15C[playerIndex]->mdlId[1], 1U);
        }
        switch (((s32) (playerWorks[playerIndex]->state[6]))) {                        /* switch 3; irregular */
        case 0:                                     /* switch 3 */
            motionFrame = 4;
            break;
        case 1:                                     /* switch 3 */

            motionFrame = ((motionFrame > 0) ? (motionFrame - 1) : (motionFrame));
            break;
        case 2:                                     /* switch 3 */

            motionFrame = ((motionFrame < 4) ? (motionFrame + 1) : (motionFrame));
            break;
        case 3:                                     /* switch 3 */
        case 4:                                     /* switch 3 */
            motionFrame = playerWorks[playerIndex]->state[7] + 5;
            break;
        }
        fn_8002D414(lbl_1_bss_15C[playerIndex]->mdlId[1], (f32) motionFrame);

        fn_80026760(lbl_1_bss_15C[playerIndex]->mdlId[1], ((60.0f * ((playerWorks[playerIndex]->gridRow - 0.25f) + lbl_1_data_60[playerIndex])) + (f32) (lbl_1_bss_8 <= 1 ? 0 : (playerIndex == 0 ? -0x15E : 0x15E))) - 150.0f, -54.0f * (1.5f + ((0.6f + playerWorks[playerIndex]->gridColumn + lbl_1_data_68[playerIndex]) - 15.0f)), 20.0f);
        playerHorizontalOffset = 0.0f;
        playerVerticalOffset = 0.0f;
        playerRotationDegrees = 0.0f;
        initialMotionState = 0;

        playerBaseX = ((60.0f * ((playerWorks[playerIndex]->gridRow - 0.25f) + lbl_1_data_60[playerIndex])) + (f32) (lbl_1_bss_8 <= 1 ? 0 : (playerIndex == 0 ? -0x15E : 0x15E))) - 150.0f;
        playerBaseY = -54.0f * (1.5f + ((0.6f + playerWorks[playerIndex]->gridColumn + lbl_1_data_68[playerIndex]) - 15.0f));
        if ((s32) playerWorks[playerIndex]->state[6] == 2) {
            static const f32 lbl_1_rodata_2730[0x1E] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
            static const f32 lbl_1_rodata_27A8[30] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
            static const f32 lbl_1_rodata_2820[30] = { 350.0f, 342.0f, 335.0f, 333.0f, 342.0f, 351.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
            lbl_1_data_254 = 1.5f;
            lbl_1_data_258 = 0.75f;
            playerHorizontalOffset = lbl_1_rodata_2730[playerWorks[playerIndex]->state[7]];
            playerVerticalOffset = lbl_1_rodata_27A8[playerWorks[playerIndex]->state[7]];

            playerRotationDegrees = ((lbl_1_rodata_2820[playerWorks[playerIndex]->state[7]] == 0.0f) ? 0.0f : (playerWorks[playerIndex]->releaseSpeed * (lbl_1_rodata_2820[playerWorks[playerIndex]->state[7]] - 360.0f)));
        } else if (((s32) playerWorks[playerIndex]->state[6] == 3) || ((s32) playerWorks[playerIndex]->state[6] == 4)) {
            static const f32 lbl_1_rodata_2898[30] = { -14.0f, -36.0f, -54.0f, -60.0f, -42.0f, -24.0f, -6.0f, 12.0f, 10.0f, 8.0f, 6.0f, 4.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
            static const f32 lbl_1_rodata_2910[30] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
            lbl_1_data_254 = 1.18f;
            lbl_1_data_258 = -1.0f;
            playerHorizontalOffset = 2.0f * lbl_1_rodata_2898[playerWorks[playerIndex]->state[7]];
            playerVerticalOffset = 2.0f * -lbl_1_rodata_2898[playerWorks[playerIndex]->state[7]];
            playerRotationDegrees = lbl_1_rodata_2910[playerWorks[playerIndex]->state[7]];
        }
        rotationAngle = 0;
        rotationAngle = (s32) playerRotationDegrees;
        rotationOffsetX = (f32) (((f64) lbl_1_data_254 * cos((M_PI * (f64) rotationAngle) / 180.0) - (f64) lbl_1_data_258 * sin((M_PI * (f64) rotationAngle) / 180.0)) - ((f64) lbl_1_data_254 * cos(0.0) - (f64) lbl_1_data_258 * sin(0.0)));
        rotationOffsetY = (f32) (((f64) lbl_1_data_254 * sin((M_PI * (f64) rotationAngle) / 180.0) + (f64) lbl_1_data_258 * cos((M_PI * (f64) rotationAngle) / 180.0)) - ((f64) lbl_1_data_254 * sin(0.0) + (f64) lbl_1_data_258 * cos(0.0)));
        fn_80026760(lbl_1_bss_15C[playerIndex]->mdlId[1], playerBaseX + (60.0f * ((playerHorizontalOffset / 256.0f) - rotationOffsetX)), playerBaseY + (-54.0f * ((playerVerticalOffset / 256.0f) + rotationOffsetY)), 20.0f);
        fn_80026808(lbl_1_bss_15C[playerIndex]->mdlId[1], 0.0f, 0.0f, (f32) -rotationAngle);
        index = 0;
        while (index < 0xCC) {
            effectAgePointer = &lbl_1_bss_48CC[playerIndex].entries[index];
            if ((s32) effectAgePointer->textureIndex != 0) {
                effectAgePointer->textureIndex = effectAgePointer->textureIndex + 1;
                DebugValue(effectAgePointer->x);
                if ((s32) effectAgePointer->textureIndex >= 0x28) {
                    effectAgePointer->textureIndex = 0;
                }
            }
            index += 1;
        }
        lbl_1_data_1A4[playerIndex] += lbl_1_data_1A4[playerIndex] < 0x10000000;
        index = 0;
        scan = 0;
        loop = 0;
        while (index < 6) {
            if ((s32) lbl_1_data_168[playerIndex][index] > 0xB) {
                lbl_1_bss_66C0[playerIndex].overflowFrames[index] += 1;
                scan += 1;
            } else {
                lbl_1_bss_66C0[playerIndex].overflowFrames[index] = 0;
            }
            loop += ((((s32) lbl_1_data_19C[playerIndex] == 0) && ((s32) lbl_1_data_168[playerIndex][index] > (s32) (0xB - ((lbl_1_bss_8 == 1) ? 0 : 1))))) || ((((s32) lbl_1_data_19C[playerIndex] == 1) && ((s32) lbl_1_data_138[playerIndex].rowCounts[index] > (s32) (0xB - ((lbl_1_bss_8 == 1) ? 0 : 3)))));
            index += 1;
        }
        if ((((s32) lbl_1_data_19C[playerIndex] == 0) && (loop != 0)) || (((s32) lbl_1_data_19C[playerIndex] == 1) && (loop == 0))) {
            lbl_1_data_19C[playerIndex] = !lbl_1_data_19C[playerIndex];
            lbl_1_data_1A4[playerIndex] = 0;
        }

        lbl_1_bss_66F0[playerIndex] = ((scan != 0) ? (lbl_1_bss_66F0[playerIndex] + ((u32) lbl_1_bss_66F0[playerIndex] < -1U)) : (0));
        if (((u32) lbl_1_bss_66F0[playerIndex] == 1) && ((s32) lbl_1_bss_174[playerIndex + 0x20].count < 3)) {
            lbl_1_bss_174[playerIndex + 0x20].count += 1;
        }
        if ((s32) playerWorks[playerIndex]->pendingRumble != 0) {
            playerWorks[playerIndex]->pendingRumble = 0;
                fn_8006A030((s16) playerWorks[playerIndex]->groupPlayerIndex, 0x14, 7, 3);
        }
        if (lbl_1_bss_0 == 2) {
            if ((u32) (lbl_1_bss_66F0[playerIndex] % 40) == 1) {
                fn_8006A030((s16) playerWorks[playerIndex]->groupPlayerIndex, 8, 2, 0xA);
            }
            if ((u32) (lbl_1_bss_66F0[playerIndex] % 40) == 0x15) {
                fn_8006A030((s16) playerWorks[playerIndex]->groupPlayerIndex, 8, 2, 0xA);
            }
        }
            }
}
        }
}
}
        }
}
}
    if (lbl_1_bss_154 <= 6) {
        audioFadeDivisor = 0xC;
        if ((lbl_1_bss_8 == 1) && (lbl_1_bss_0 == 5) && (lbl_1_data_C8[0] == 0x2C)) {
                if (lbl_1_bss_4 == 0) {
                    if ((lbl_1_bss_18 == 0) && (lbl_1_bss_1C == 0)) {
                        HuAudSStreamFadeOut(lbl_1_data_1F0, 0x3E8);
                        lbl_1_data_1F0 = -1;
                        if (lbl_1_data_1F4 != -1) {
                            HuAudSStreamStop(lbl_1_data_1F4);
                        }
                        lbl_1_data_1F4 = -1;
                    } else if ((lbl_1_bss_18 == 1) && (lbl_1_bss_1C == 0)) {
                        HuAudSStreamFadeOut(lbl_1_data_1F4, 0x3E8);
                        lbl_1_data_1F4 = -1;
                        HuAudSStreamStop(lbl_1_data_1F0);
                        lbl_1_data_1F0 = -1;
                    } else {
                        HuAudSStreamStop(lbl_1_data_1F0);
                        lbl_1_data_1F0 = -1;
                        HuAudSStreamStop(lbl_1_data_1F4);
                        lbl_1_data_1F4 = -1;
                    }
                    lbl_1_bss_1C = 0;
                }
        } else if ((lbl_1_bss_8 == 1) && (lbl_1_bss_0 == 1) && (lbl_1_data_C8[0] == 0x2D)) {
                        if (lbl_1_bss_4 == 0) {

                            lbl_1_data_1F0 = HuAudSStreamChanPlay((s16) (((((lbl_1_bss_8 == 1) && (lbl_1_data_C8[0] < 0x2D))) != 0) ? (0x53) : (0x2B)), 0);
                            lbl_1_bss_20 = 0;
                        }
        } else {
                        if ((lbl_1_bss_8 == 1) && (lbl_1_bss_18 == 0) && (lbl_1_bss_1C == 0) && (lbl_1_bss_0 == 2) && (((s32) ((lbl_1_data_F0[0] + 0x3B) / 60) <= 0xA) || (((s32) ((lbl_1_data_F0[0] + 0x3B) / 60) <= 0x3C) && ((s32) lbl_1_data_19C[0] == 1) && ((s32) lbl_1_data_1A4[0] > 5)))) {
                            lbl_1_bss_18 = 1;
                            lbl_1_bss_1C = 0x3C / audioFadeDivisor;
                        }
                        if ((lbl_1_bss_8 == 2) && (lbl_1_bss_18 == 0) && (lbl_1_bss_1C == 0) && (lbl_1_bss_0 == 2) && (((s32) lbl_1_data_F0[0] < 0x3C) || ((s32) lbl_1_data_F0[1] < 0x3C) || (((s32) lbl_1_data_19C[0] == 1) && ((s32) lbl_1_data_1A4[0] > 3)) || (((s32) lbl_1_data_19C[1] == 1) && ((s32) lbl_1_data_1A4[1] > 3)) || ((u32) lbl_1_bss_66F0[0] != 0) || ((u32) lbl_1_bss_66F0[1] != 0))) {
                            lbl_1_bss_18 = 1;
                            lbl_1_bss_1C = 0x3C / audioFadeDivisor;
                        }
                        if ((lbl_1_bss_8 == 1) && (lbl_1_bss_18 == 1) && (lbl_1_bss_1C == 0) && ((lbl_1_bss_0 == 5) || (((s32) ((lbl_1_data_F0[0] + 0x3B) / 60) > 0xA) && ((s32) lbl_1_data_19C[0] == 0) && ((s32) lbl_1_data_1A4[0] > 0x3C) && ((f64) lbl_1_bss_20 > 43.5)))) {
                            lbl_1_bss_18 = 0;
                            lbl_1_bss_1C = 0x3C / audioFadeDivisor;
                        }
                        if ((lbl_1_bss_8 == 2) && (lbl_1_bss_18 == 1) && (lbl_1_bss_1C == 0) && ((s32) lbl_1_data_F0[0] >= 0x3C) && ((s32) lbl_1_data_F0[1] >= 0x3C) && ((s32) lbl_1_data_19C[0] == 0) && ((s32) lbl_1_data_1A4[0] > 0x3C) && ((s32) lbl_1_data_19C[1] == 0) && ((s32) lbl_1_data_1A4[1] > 0x3C) && ((f64) lbl_1_bss_20 > 43.5)) {
                            lbl_1_bss_18 = 0;
                            lbl_1_bss_1C = 0x3C / audioFadeDivisor;
                        }
                        if (lbl_1_bss_1C == (s32) (0x3C / audioFadeDivisor)) {
                            if (lbl_1_bss_18 == 0) {
                                HuAudSStreamFadeOut(lbl_1_data_1F4, 0x3E8 / audioFadeDivisor);
                                lbl_1_data_1F4 = -1;
                            } else if (lbl_1_bss_18 == 1) {
                                HuAudSStreamPauseFadeOut(0, 1, 0x3E8 / audioFadeDivisor);
                            }
                        } else if (lbl_1_bss_1C == 1) {
                            if (lbl_1_bss_18 == 0) {
                                HuAudSStreamPauseFadeOut(0, 0, 0x3E8 / audioFadeDivisor);
                                lbl_1_bss_20 = 0;
                            } else if (lbl_1_bss_18 == 1) {

                                lbl_1_data_1F4 = HuAudSStreamChanPlay((s16) (((((lbl_1_bss_8 == 1) && (lbl_1_data_C8[0] >= 0x2D))) != 0) ? (0x2A) : (0xA)), 2);
                                lbl_1_bss_20 = 0;
                            }
                        }
                        lbl_1_bss_1C -= lbl_1_bss_1C > 0;
                        lbl_1_bss_20 += lbl_1_bss_20 < 0x10000000;
        }
    }
    if (lbl_1_bss_8 == 1) {
        playerIndex = 0;
        switch (lbl_1_bss_0) {                /* switch 6; irregular */
        case 3:                                     /* switch 6 */
            break;
        case 1:                                     /* switch 6 */
            if (lbl_1_bss_4 == 0) {
                s32 scoreValues[100] = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x10, 0x11, 0x12, 0x13, 0x14, 0x10, 0x10, 0x12, 0x12, 0x12, 0x12, 0x12, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x10, 0x10, 0x12, 0x12, 0x12, 0x12, 0x12, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14 };
                s32 displayValues[100] = { 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x55, 0x50, 0x4B, 0x46, 0x41, 0x3C, 0x37, 0x32, 0x2D, 0x2D, 0x28, 0x23, 0x1E, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x4D, 0x4A, 0x47, 0x44, 0x44 };
                lbl_1_bss_4 = 1;
                if (lbl_1_data_C8[0] == 0) {
                    lbl_1_data_F0[0] = 0xE10;
                    lbl_1_data_F8[0] = 0xE10;
                }
                if (lbl_1_data_C8[0] == 0) {
                    lbl_1_data_118[0] = 0x12;
                    *lbl_1_data_128 = 0xFFFF;
                } else if ((s32) lbl_1_bss_174[0x11].count < 3) {
                    lbl_1_bss_174[0x11].count += 1;
                }

                lbl_1_data_D0[0] = lbl_1_data_D8[0] = lbl_1_data_E8[0] = lbl_1_data_E0[0] = scoreValues[((lbl_1_data_C8[0] >= 0x64) ? (0x63) : (lbl_1_data_C8[0]))];

                lbl_1_data_130[0] = displayValues[((lbl_1_data_C8[0] >= 0x64) ? (0x63) : (lbl_1_data_C8[0]))];
                lbl_1_data_130[0] = (s32) ((lbl_1_data_130[0] * 0x5A) / 90);
                memset(lbl_1_bss_70A8, 0, 0x3960U);
                memset(lbl_1_bss_2F88, 0, 0xB28U);
                memset(lbl_1_bss_5BF0, 0, 0x140U);
                memset(lbl_1_bss_66A8, 0, 0xCU);
                memset(lbl_1_bss_6690, 0, 0xCU);
                memset(lbl_1_data_1AC, 0, 6U);
                memset(lbl_1_bss_48CC, 0, 0x990U);
                index = 0;
                while (index < 0x66) {
                    lbl_1_bss_70A8[0][index].index = index;
                    index += 1;
                }
                index = 0;
                while (index < 0x66) {
                    lbl_1_bss_2E0[0][1][index].index = index;
                    lbl_1_bss_2E0[0][0][index].index = index;
                    lbl_1_bss_2F88[0].entries[index].index = index;
                    index += 1;
                }
                lbl_1_bss_2F80[0] = 0;
                lbl_1_bss_2D0->counts[0] = lbl_1_bss_2D0->counts[1] = 0;
                lbl_1_data_A8[0] = 0;
                lbl_1_data_B0[0] = 0;
                *lbl_1_data_B8 = 0;
                *lbl_1_data_80 = 0;
                lbl_1_data_88[0] = 0;
                lbl_1_data_90[0] = 0;
                *lbl_1_data_100 = 0;
                lbl_1_data_108[0] = 0;
                *lbl_1_data_120 = 0;

                lbl_1_data_118[0] = (((s32) *lbl_1_data_128 < 0xB4) ? (0) : (lbl_1_data_118[0]));
                *lbl_1_data_1E0 = (s32) (lbl_1_data_F8[0] / 60);
                lbl_1_data_1D8[0] = 0;
                lbl_1_data_198 = 1.0f;
                memset(lbl_1_bss_5E90, 0, 0x400U);
                memset(lbl_1_bss_5E80, 0, 8U);
                memset(lbl_1_bss_5E70, 0, 8U);
                playerWorks[0]->gridRow = 2.0f;
                playerWorks[0]->gridColumn = 14.0f;
                playerWorks[0]->state[6] = playerWorks[0]->state[7] = 0;
                playerWorks[0]->computerState = playerWorks[0]->computerWaitFrames = 0;
                index = 0;
                while (index < 2) {
                    playerWorks[0]->motionState[index] = 0;
                    index += 1;
                }
                fn_800269E8(hookWork->mdlId[7], 0x40000002U);
                fn_8002D414(hookWork->mdlId[7], 0.0f);
            }
            if ((lbl_1_data_C8[0] > 0) && (lbl_1_bss_4 >= 1) && (lbl_1_bss_4 < 7)) {
                lbl_1_data_F0[0] = (((lbl_1_data_F0[0] + 0x3B) / 60) + 5) * 0x3C;

                lbl_1_data_F0[0] = (((s32) lbl_1_data_F0[0] > (s32) lbl_1_data_F8[0]) ? (lbl_1_data_F8[0]) : (lbl_1_data_F0[0]));
            }
            if (WipeCheck() == 0) {
    Point3d cameraScreenPosition;
    Point3d cameraWorldPosition;
    s32 projectedCameraX[2];
    s32 projectedCameraY[2];

                index = 0;
                while (index < 4) {
                    if (lbl_1_bss_4 == (s32) ((index * 0xA) + 5)) {
                        *lbl_1_data_80 = 1;
                        lbl_1_data_88[0] = 0;
                    }
                    index += 1;
                }
                lbl_1_bss_4 += 1;
                if ((lbl_1_bss_4 >= 0x32) && (lbl_1_bss_154 > 3)) {
                    lbl_1_bss_0 = 2;
                    fn_800269E8(hookWork->mdlId[18], 1U);
                }
            }
            break;
        case 2:                                     /* switch 6 */
            lbl_1_bss_4 += 1;

            lbl_1_data_F0[0] -= (((((((s32) lbl_1_data_F0[0] > 0) && ((s32) lbl_1_data_D0[0] > 0))) != 0) && ((s32) lbl_1_bss_45D8.state != 1)));
            if ((s32) lbl_1_data_F0[0] > 0) {
                index = 0;
                scan = 0;
                loop = 0;
                while (index < 6) {
                    scan += lbl_1_data_168[0][index] > 0xB;
                    index += 1;
                }

                lbl_1_data_F0[0] = ((loop != 0) ? (0) : (((scan != 0) ? (lbl_1_data_F0[0] - (lbl_1_data_F0[0] > 0)) : (lbl_1_data_F0[0]))));
                if ((lbl_1_data_1EC == -1) && ((scan != 0) || ((s32) lbl_1_data_F0[0] <= 0x258))) {
                    fn_80026A44(hookWork->mdlId[playerIndex + 3], 0x40000002U);
                    fn_80026A44(hookWork->mdlId[playerIndex + 5], 0x40000002U);
                } else {
                    fn_800269E8(hookWork->mdlId[playerIndex + 3], 0x40000002U);
                    fn_8002D414(hookWork->mdlId[playerIndex + 3], 0.0f);
                    fn_800269E8(hookWork->mdlId[playerIndex + 5], 0x40000002U);
                    fn_8002D414(hookWork->mdlId[playerIndex + 5], 0.0f);
                }
                if ((s32) *lbl_1_data_1E0 < (s32) (lbl_1_data_F0[0] / 60)) {
                    *lbl_1_data_1E0 = (s32) (lbl_1_data_F8[0] / 60);
                }
                if (((s32) *lbl_1_data_1E0 > (s32) (lbl_1_data_F0[0] / 60)) && (((s32) (lbl_1_data_F0[0] / 60) == 0x1D) || ((s32) (lbl_1_data_F0[0] / 60) <= 9))) {
                    *lbl_1_data_1E0 = (s32) (lbl_1_data_F0[playerIndex] / 60);
                    lbl_1_data_1D8[0] = 0x78;
                    if ((s32) lbl_1_bss_174[0x24].count < 3) {
                        lbl_1_bss_174[0x24].count += 1;
                    }
                }

                lbl_1_data_1D8[0] += ((((((s32) lbl_1_data_D0[0] == 0) || (loop != 0))) != 0) ? (-0x1E) : ((((s32) *lbl_1_data_1E0 <= 0xA) ? (-2) : (-1)) * ((scan != 0) ? (2) : (1))));

                lbl_1_data_1D8[0] = (((s32) lbl_1_data_1D8[0] < 0) ? (0) : (lbl_1_data_1D8[0]));
            }
            if (specialEntryCounts[0] == 0) {
                lbl_1_bss_0 = 5;
                lbl_1_bss_4 = 0;
            } else if (((s32) lbl_1_data_F0[0] == 0) && ((s32) *lbl_1_data_C0 == 0) && ((s32) lbl_1_bss_45D8.state == 0)) {
                lbl_1_bss_0 = 4;
                lbl_1_bss_4 = 0;
                if (lbl_1_bss_34 != 0) {
                    fn_8003C80C(7, lbl_1_bss_5BEC);
                }
            }
            if (lbl_1_bss_0 != 2) {
                lbl_1_data_1D8[0] = 0;
            }
            break;
        case 5:                                     /* switch 6 */
            if (lbl_1_bss_4 == 0) {
                fn_800269E8(hookWork->mdlId[3], 0x40000002U);
                fn_8002D414(hookWork->mdlId[3], 0.0f);
                fn_800269E8(hookWork->mdlId[5], 0x40000002U);
                fn_8002D414(hookWork->mdlId[5], 0.0f);
                fn_80026A44(hookWork->mdlId[18], 1U);
                fn_80026A44(hookWork->mdlId[18], 0x40000002U);
                fn_8002D414(hookWork->mdlId[18], 0.0f);
                fn_8002D86C(hookWork->mdlId[18], 1.0f);

                fn_8002CCC0(lbl_1_bss_15C[0]->mdlId[2], lbl_1_bss_15C[0]->mtnId[1], (f32) lbl_1_data_50[1], (f32) motionBlendFrames[1], ((lbl_1_data_48[1] != 0) ? (0) : (0x40000001)));
                espBankSet((s16) lbl_1_bss_130[0], (s16) (((lbl_1_data_C8[0] + 1) / 10) % 10));
                espBankSet((s16) lbl_1_bss_130[1], (s16) ((lbl_1_data_C8[0] + 1) % 10));
                *lbl_1_data_120 = 1;
                if ((s32) lbl_1_bss_174[0x10].count < 3) {
                    lbl_1_bss_174[0x10].count += 1;
                }
            }
            if (lbl_1_bss_4 == 0x1D) {
                lbl_1_data_1D0[0] += (lbl_1_data_F0[0] + 0x3B) / 60;

                lbl_1_data_1D0[0] = ((lbl_1_data_1D0[0] > 0x270FU) ? (0x270F) : (lbl_1_data_1D0[0]));
                if (lbl_1_bss_5BEC < lbl_1_data_1D0[0]) {
                    lbl_1_bss_34 = 1;
                    lbl_1_bss_5BEC = lbl_1_data_1D0[0];
                }
            }
            if (lbl_1_bss_4 == 0x3C) {

                fn_8002CCC0(lbl_1_bss_15C[0]->mdlId[2], lbl_1_bss_15C[0]->mtnId[0], (f32) lbl_1_data_50[0], (f32) motionBlendFrames[0], ((lbl_1_data_48[0] != 0) ? (0) : (0x40000001)));
            }
            if (lbl_1_bss_4 >= 0x1E) {
                index = 0;
                while (index < 0x66) {
                    endingEntry = &lbl_1_bss_70A8[playerIndex][index];
                    if ((s32) endingEntry->kind != 0) {
                        endingEntry->opacityScale = (f32) ((1.0f - ((f32) (lbl_1_bss_4 - 0x1E) / 40.0f)) + (0.02f * endingEntry->gridColumn));

    endingEntry->opacityScale = ((endingEntry->opacityScale > 1.0f) ? 1.0f : (((endingEntry->opacityScale < 0.0f) ? 0.0f : (endingEntry->opacityScale))));
                    }
    index += 1;
}
    lbl_1_data_198 = 0.14f + (1.0f - ((f32) (lbl_1_bss_4 - 0x1E) / 40.0f));

    lbl_1_data_198 = ((lbl_1_data_198 > 1.0f) ? 1.0f : (((lbl_1_data_198 < 0.0f) ? 0.0f : (lbl_1_data_198))));
            }
    lbl_1_bss_4 += 1;
            if ((lbl_1_data_C8[0] == 0x2C) && (lbl_1_bss_4 == 0x3C)) {
    HuAudSStreamAllStop();
    lbl_1_data_1F0 = -1;
            }
            if (lbl_1_bss_4 >= 0x64) {
                fn_8002D86C(hookWork->mdlId[18], -1.0f);
            }
            if (lbl_1_bss_4 >= 0x64) {
                lbl_1_data_C8[0] += lbl_1_data_C8[0] < 0x62;
                lbl_1_bss_0 = 1;
                lbl_1_bss_4 = 0;
            }
            break;
        case 4:                                     /* switch 6 */
            fn_800269E8(hookWork->mdlId[3], 0x40000002U);
            fn_8002D414(hookWork->mdlId[3], 0.0f);
            fn_800269E8(hookWork->mdlId[5], 0x40000002U);
            fn_8002D414(hookWork->mdlId[5], 0.0f);
            if (lbl_1_bss_4 == 0) {
                lbl_1_data_1EC = 4;
                playerWorks[0]->activeFlag = -1;
            }
            lbl_1_bss_4 += 1;
            if (lbl_1_bss_4 == 0x1E) {
                fn_80026A44(hookWork->mdlId[7], 0x40000002U);
            }
            if ((lbl_1_bss_4 == 0x1E) && ((s32) lbl_1_bss_174[0x22].count < 3)) {
                lbl_1_bss_174[0x22].count += 1;
            }
            break;
        }
    } else {
        switch (lbl_1_bss_0) {                /* switch 7; irregular */
        case 3:                                     /* switch 7 */
            break;
        case 1:                                     /* switch 7 */
            if (lbl_1_bss_4 == 0) {
                lbl_1_bss_4 = 1;
                playerIndex = 0;
                while (playerIndex < lbl_1_bss_8) {
                    lbl_1_data_D0[playerIndex] = lbl_1_data_D8[playerIndex] = lbl_1_data_E8[playerIndex] = lbl_1_data_E0[playerIndex] = 0;
                    memset(&lbl_1_bss_70A8[playerIndex], 0, 0x3960U);
                    memset(&lbl_1_bss_2F88[playerIndex], 0, 0xB28U);
                    memset(&lbl_1_bss_5BF0[playerIndex], 0, 0x140U);
                    memset(&lbl_1_bss_66A8[playerIndex], 0, 0xCU);
                    memset(&lbl_1_bss_6690[playerIndex], 0, 0xCU);
                    memset(&lbl_1_data_1AC[playerIndex], 0, 6U);
                    memset(&lbl_1_bss_48CC[playerIndex], 0, 0x990U);
                    index = 0;
                    while (index < 0x66) {
                        lbl_1_bss_70A8[playerIndex][index].index = index;
                        index += 1;
                    }
                    index = 0;
                    while (index < 0x66) {
                        lbl_1_bss_2E0[playerIndex][1][index].index = index;
                        lbl_1_bss_2E0[playerIndex][0][index].index = index;
                        lbl_1_bss_2F88[playerIndex].entries[index].index = index;
                        index += 1;
                    }
                    lbl_1_bss_2F80[playerIndex] = 0;
                    lbl_1_bss_2D0[playerIndex].counts[0] = lbl_1_bss_2D0[playerIndex].counts[1] = 0;
                    lbl_1_data_A8[playerIndex] = 0;
                    lbl_1_data_B0[playerIndex] = 0;
                    lbl_1_data_B8[playerIndex] = 0;
                    lbl_1_data_80[playerIndex] = 0;
                    lbl_1_data_88[playerIndex] = 0;
                    lbl_1_data_90[playerIndex] = 0;
                    lbl_1_data_100[playerIndex] = 0;
                    lbl_1_data_108[playerIndex] = 0;
                    lbl_1_data_130[playerIndex] = 0x5A;
                    lbl_1_data_F0[playerIndex] = lbl_1_data_F8[playerIndex] = 0x12C;
                    lbl_1_data_1E0[playerIndex] = (s32) (lbl_1_data_F8[playerIndex] / 60);
                    lbl_1_data_1D8[playerIndex] = 0;
                    lbl_1_data_198 = 1.0f;
                    memset(&lbl_1_bss_5E90[playerIndex], 0, 0x400U);
                    memset(&lbl_1_bss_5E80[playerIndex], 0, 8U);
                    memset(&lbl_1_bss_5E70[playerIndex], 0, 8U);
                    playerWorks[playerIndex]->gridRow = 2.0f;
                    playerWorks[playerIndex]->gridColumn = 14.0f;
                    playerWorks[playerIndex]->state[6] = playerWorks[playerIndex]->state[7] = 0;
                    playerWorks[playerIndex]->computerState = playerWorks[playerIndex]->computerWaitFrames = 0;
                    index = 0;
                    while (index < 2) {
                        playerWorks[playerIndex]->motionState[index] = 0;
                        index += 1;
                    }
                    fn_800269E8(hookWork->mdlId[playerIndex + 7], 0x40000002U);
                    fn_8002D414(hookWork->mdlId[playerIndex + 7], 0.0f);
                    playerIndex += 1;
                }
            }
            if (WipeCheck() == 0) {
    Point3d cameraScreenPosition;
    Point3d cameraWorldPosition;
    s32 projectedCameraX[2];
    s32 projectedCameraY[2];

                index = 0;
                while (index < 4) {
                    if (lbl_1_bss_4 == (s32) ((index * 0xA) + 5)) {
                        playerIndex = 0;
                        while (playerIndex < lbl_1_bss_8) {
                            lbl_1_data_80[playerIndex] = 1;
                            lbl_1_data_88[playerIndex] = 0;
                            playerIndex += 1;
                        }
                    }
                    index += 1;
                }
                lbl_1_bss_4 += 1;
                if ((lbl_1_bss_4 >= 0x78) && (lbl_1_bss_154 > 3)) {
                    lbl_1_bss_0 = 2;
                }
            }
            break;
        case 2:                                     /* switch 7 */
            lbl_1_bss_4 += 1;
            playerIndex = 0;
            while (playerIndex < lbl_1_bss_8) {
                if ((s32) lbl_1_data_F0[playerIndex] > 0) {
                    index = 0;
                    scan = 0;
                    loop = 0;
                    while (index < 6) {
                        scan += lbl_1_data_168[playerIndex][index] > 0xB;
                        index += 1;
                    }

                    lbl_1_data_F0[playerIndex] = ((loop != 0) ? (0) : (((scan != 0) ? (lbl_1_data_F0[playerIndex] - (lbl_1_data_F0[playerIndex] > 0)) : (lbl_1_data_F0[playerIndex]))));
                    if ((lbl_1_data_1EC == -1) && ((scan != 0) || ((s32) lbl_1_data_F0[playerIndex] <= 0x3C))) {
                        fn_80026A44(hookWork->mdlId[playerIndex + 3], 0x40000002U);
                        fn_80026A44(hookWork->mdlId[playerIndex + 5], 0x40000002U);
                    } else {
                        fn_800269E8(hookWork->mdlId[playerIndex + 3], 0x40000002U);
                        fn_8002D414(hookWork->mdlId[playerIndex + 3], 0.0f);
                        fn_800269E8(hookWork->mdlId[playerIndex + 5], 0x40000002U);
                        fn_8002D414(hookWork->mdlId[playerIndex + 5], 0.0f);
                    }
                    if ((scan != 0) && (((s32) lbl_1_data_1D8[playerIndex] == 0) || ((s32) lbl_1_data_1E0[playerIndex] > (s32) (lbl_1_data_F0[playerIndex] / 60))) && (((s32) (lbl_1_data_F0[playerIndex] / 60) == 0x1E) || ((s32) (lbl_1_data_F0[playerIndex] / 60) <= 0xA))) {
                        lbl_1_data_1E0[playerIndex] = (s32) (lbl_1_data_F0[playerIndex] / 60);
                        lbl_1_data_1D8[playerIndex] = 0x78;
                        if ((s32) lbl_1_bss_174[playerIndex + 0x24].count < 3) {
                            lbl_1_bss_174[playerIndex + 0x24].count += 1;
                        }
                    }

                    lbl_1_data_1D8[playerIndex] += ((scan != 0) ? (-2) : (-0x1E));

                    lbl_1_data_1D8[playerIndex] = (((s32) lbl_1_data_1D8[playerIndex] < 0) ? (0) : (lbl_1_data_1D8[playerIndex]));
                }
                if ((s32) lbl_1_data_F0[playerIndex] == 0) {
                    lbl_1_bss_0 = 4;
                    lbl_1_bss_4 = 0;
                }
                playerIndex += 1;
            }
            playerIndex = 0;
            while (playerIndex < lbl_1_bss_8) {
                if (lbl_1_bss_0 != 2) {
                    lbl_1_data_1D8[playerIndex] = 0;
                }
                if (lbl_1_bss_0 == 4) {

                    playerWorks[playerIndex]->activeFlag = (((s32) lbl_1_data_F0[playerIndex] == 0) ? (-1) : (1));
                }
                playerIndex += 1;
            }
            if (lbl_1_bss_0 == 4) {

                lbl_1_data_1EC = ((((((s32) playerWorks[0]->activeFlag == -1) && ((s32) playerWorks[1]->activeFlag == -1))) != 0) ? (4) : ((((s32) playerWorks[1]->activeFlag == -1) ? (0) : (1))));
            }
            break;
        case 4:                                     /* switch 7 */
            playerIndex = 0;
            while (playerIndex < lbl_1_bss_8) {
                fn_800269E8(hookWork->mdlId[playerIndex + 3], 0x40000002U);
                fn_8002D414(hookWork->mdlId[playerIndex + 3], 0.0f);
                fn_800269E8(hookWork->mdlId[playerIndex + 5], 0x40000002U);
                fn_8002D414(hookWork->mdlId[playerIndex + 5], 0.0f);
                playerIndex += 1;
            }
            lbl_1_bss_4 += 1;
            if (lbl_1_bss_4 == 0x1E) {
                playerIndex = 0;
                while (playerIndex < lbl_1_bss_8) {
                    if ((s32) playerWorks[playerIndex]->activeFlag == -1) {

                        fn_80026A44(hookWork->mdlId[((playerIndex == 0) ? (7) : (8))], 0x40000002U);
                        if ((s32) lbl_1_bss_174[playerIndex + 0x22].count < 3) {
                            lbl_1_bss_174[playerIndex + 0x22].count += 1;
                        }
                    }
                    playerIndex += 1;
                }
            }
            break;
        }
    }
    playerIndex = 0;
    while (playerIndex < lbl_1_bss_8) {

{
s32 modelPositions[2][2] = { -0x190, -0x190, -0x55, 0x55 };

        fn_80026760(hookWork->mdlId[playerIndex + 5], (f32) modelPositions[((lbl_1_bss_8 == 1) ? (0) : (1))][playerIndex], -450.0f * (1.0f - ((f32) ((s32) (lbl_1_data_F0[playerIndex])) / (f32) lbl_1_data_F8[playerIndex])), 150.0f);
        if (((s32) lbl_1_data_80[playerIndex] != 0) || ((s32) lbl_1_data_88[playerIndex] != 0)) {

            fn_8002D86C(hookWork->mdlId[playerIndex + 0xD], (((s32) lbl_1_data_80[playerIndex] == 2) ? 1.0f : (-1.0f)));
        }
        fn_80026A44(hookWork->mdlId[playerIndex + 0xD], 0x40000001U);
        fn_8002D86C(hookWork->mdlId[playerIndex + 0xB], 1.0f);
        if ((lbl_1_bss_8 == 1) && (lbl_1_bss_0 == 5)) {
            if (lbl_1_bss_4 == 1) {
                fn_8002D414(hookWork->mdlId[playerIndex + 0xB], 0.0f);
            }
            fn_80026A44(hookWork->mdlId[playerIndex + 0xB], 0x40000001U);
            fn_80026A44(hookWork->mdlId[playerIndex + 0xB], 0x40000002U);
        } else if ((lbl_1_bss_8 > 1) && (lbl_1_bss_0 == 4) && (lbl_1_data_1EC == playerIndex)) {
            if (lbl_1_bss_4 == 1) {
                fn_8002D414(hookWork->mdlId[playerIndex + 0xB], 0.0f);
            }
            fn_800269E8(hookWork->mdlId[playerIndex + 0xB], 0x40000001U);
        } else if ((lbl_1_bss_0 == 4) && ((lbl_1_bss_8 == 1) || (lbl_1_data_1EC != playerIndex))) {
            fn_8002D414(hookWork->mdlId[playerIndex + 0xB], 180.0f);
            fn_80026A44(hookWork->mdlId[playerIndex + 0xB], 0x40000001U);
            fn_80026A44(hookWork->mdlId[playerIndex + 9], 0x40000002U);
        } else {
            resultAnimationFrame = fn_8002D4E0(hookWork->mdlId[playerIndex + 0xB]);
            resultAnimationFrame = resultAnimationFrame < 100.0f ? 100.0f : (resultAnimationFrame > 120.0f ? resultAnimationFrame - 20.0f : resultAnimationFrame);
            fn_8002D414(hookWork->mdlId[playerIndex + 0xB], resultAnimationFrame);
            fn_800269E8(hookWork->mdlId[playerIndex + 0xB], 0x40000001U);
            fn_80026A44(hookWork->mdlId[playerIndex + 0xB], 0x40000002U);
            fn_8002D86C(hookWork->mdlId[playerIndex + 0xB], 0.3f);
            fn_80026A44(hookWork->mdlId[playerIndex + 9], 0x40000002U);
            fn_800269E8(hookWork->mdlId[playerIndex + 9], 0x40000001U);
        }
        playerIndex += 1;
    }
}
    if (lbl_1_bss_8 == 1) {
{
    f32 scorePositionBases[2][7] = { 480.0f, 480.0f, 480.0f, 450.0f, 510.0f, 102.0f, 0.0f, 480.0f, 480.0f, 480.0f, 450.0f, 510.0f, 248.0f, 328.0f };
{
    f32 scoreScreenY[7] = { 144.0f, 123.0f, 283.0f, 395.0f, 395.0f, 176.0f, 176.0f };
{
    f32 scorePositionOffsets[2][7] = { -24.0f, -24.0f, -8.0f, -8.0f, -8.0f, -17.0f, -17.0f, -30.0f, -30.0f, -8.0f, -8.0f, -8.0f, -10.0f, -8.0f };
        scoreScale = 1.0f;
        MgScoreValueSet(lbl_1_bss_2B4[0], lbl_1_data_1D0[0] % 10000);
        MgScoreValueSet(lbl_1_bss_2B4[1], lbl_1_bss_5BEC % 10000);
        MgScoreValueSet(lbl_1_bss_2B4[2], (lbl_1_data_C8[0] + 1) % 100);
        MgScoreValueSet(lbl_1_bss_2B4[3], lbl_1_data_D8[0] % 100);
        MgScoreValueSet(lbl_1_bss_2B4[4], lbl_1_data_E0[0] % 100);
        MgScoreValueSet(lbl_1_bss_2B4[5], ((lbl_1_data_F0[0] + 0x3B) / 60) % 100);
        index = 0;
        while (index < 0x14) {
            if ((s32) lbl_1_bss_45FC[index].state == 2) {
                scorePulse = 1.0f + (0.333f * (1.0f - ((f32) lbl_1_bss_45FC[index].frame / 10.0f)));

                scoreScale = ((scorePulse > scoreScale) ? (scorePulse) : (scoreScale));
            }
            index += 1;
        }
        index = 3;
        MgScoreScaleSet(lbl_1_bss_2B4[index], scoreScale, scoreScale);

        MgScorePosSet(lbl_1_bss_2B4[index], (16.0f + (scorePositionBases[((lbl_1_bss_8 == 1) ? (0) : (1))][index] + scorePositionOffsets[((lbl_1_bss_8 == 1) ? (0) : (1))][index])) - (16.0f * scoreScale), scoreScreenY[index]);
        index = 5;
        scoreScale = ((s32) lbl_1_bss_45D8.state == 2) ? 1.0f + (0.333f * (1.0f - ((f32) lbl_1_bss_45D8.frame / 10.0f))) : 1.0f;
        MgScoreScaleSet(lbl_1_bss_2B4[index], scoreScale, scoreScale);

        MgScorePosSet(lbl_1_bss_2B4[index], (16.0f + (scorePositionBases[((lbl_1_bss_8 == 1) ? (0) : (1))][index] + scorePositionOffsets[((lbl_1_bss_8 == 1) ? (0) : (1))][index])) - (16.0f * scoreScale), scoreScreenY[index]);
        index = 0;

        scoreScale = (((((((lbl_1_bss_0 == 5) && (lbl_1_bss_4 >= 0x1E))) != 0) && (lbl_1_bss_4 <= 0x21))) != 0) ? 1.0f + (0.05f * (1.0f - ((f32) (lbl_1_bss_4 - 0x1E) / 3.0f))) : 1.0f;
        MgScoreScaleSet(lbl_1_bss_2B4[index], scoreScale, scoreScale);

        MgScorePosSet(lbl_1_bss_2B4[index], (32.0f + (scorePositionBases[((lbl_1_bss_8 == 1) ? (0) : (1))][index] + scorePositionOffsets[((lbl_1_bss_8 == 1) ? (0) : (1))][index])) - (32.0f * scoreScale), scoreScreenY[index]);
        index = 1;
        scoreScale = lbl_1_bss_34 != 0 ? scoreScale : 1.0f;
        if ((lbl_1_bss_154 >= 8) && (lbl_1_bss_34 != 0)) {
            lbl_1_bss_54 += 1;
            lbl_1_bss_54 %= 0x11;
            scorePulseScale = (lbl_1_bss_54 <= 0xA) ? 1.0f + (0.02f * (f32) lbl_1_bss_54) : 1.2f - (0.04f * (f32) (lbl_1_bss_54 - 0xB));
            scorePulseScale = 1.0f + (0.875f * (scorePulseScale - 1.0f));
            MgScoreScaleSet(lbl_1_bss_2B4[index], 1.0f, 1.0f);
            MgScoreDigitScaleSet(lbl_1_bss_2B4[index], scorePulseScale, scorePulseScale);

            MgScorePosSet(lbl_1_bss_2B4[index], (32.0f + (scorePositionBases[((lbl_1_bss_8 == 1) ? (0) : (1))][index] + scorePositionOffsets[((lbl_1_bss_8 == 1) ? (0) : (1))][index])) - 32.0f, scoreScreenY[index]);
        } else {
            MgScoreScaleSet(lbl_1_bss_2B4[index], scoreScale, scoreScale);

            MgScorePosSet(lbl_1_bss_2B4[index], (32.0f + (scorePositionBases[((lbl_1_bss_8 == 1) ? (0) : (1))][index] + scorePositionOffsets[((lbl_1_bss_8 == 1) ? (0) : (1))][index])) - (32.0f * scoreScale), scoreScreenY[index]);
        }
    }
}
}
} else {
        MgScoreValueSet(lbl_1_bss_2B4[5], ((lbl_1_data_F0[0] + 0x3B) / 60) % 100);
        MgScoreValueSet(lbl_1_bss_2B4[6], ((lbl_1_data_F0[1] + 0x3B) / 60) % 100);
    }
    if (WipeCheck() == 0) {
    Point3d cameraScreenPosition;
    Point3d cameraWorldPosition;
    s32 projectedCameraX[2];
    s32 projectedCameraY[2];

        if ((lbl_1_bss_38 < 2) && ((s32) lbl_1_bss_40[0] >= 0x28)) {
            fn_80026A44(lbl_1_bss_158->mdlId[0x2A], 1U);
            fn_800258A8(1, 0);
            fn_800258A8(1, 8);
            lbl_1_bss_38 = 2;
        }
        if (lbl_1_bss_38 < 2) {
            if (lbl_1_bss_38 == 0) {
                index = 0;
                while (index < 2) {

{
s32 cameraTableValue1[4] = { -0x1, 0x1, -0x1, 0x1 };

{
s32 cameraTableValue5[4] = { 0x1, 0x1, -0x1, -0x1 };

                    cameraWorldPosition.x = lbl_80266A4C.x + (f32) (((index == 0) ? (-1) : (1)) * 0x78);

                    cameraWorldPosition.y = lbl_80266A4C.y + (f32) (((index == 0) ? (1) : (-1)) * 0x6C);
                    cameraWorldPosition.z = 0.0f;
                    Hu3D3Dto2D(&cameraWorldPosition, 1, &cameraScreenPosition);
                    cameraScreenPosition.x = (f32) ((s32) ((1.0f + cameraScreenPosition.x / 0.9f) / 2.0f) * 2);
                    cameraScreenPosition.y = (f32) ((s32) ((1.0f + cameraScreenPosition.y) / 2.0f) * 2);
                    (&projectedCameraX[0])[index] = (s32) cameraScreenPosition.x;
                    (&projectedCameraY[0])[index] = (s32) cameraScreenPosition.y;
                    cameraScreenPosition.x *= 0.9f;
                    cameraScreenPosition.z = lbl_802C028C;
                    Hu3D2Dto3D(&cameraScreenPosition, 1, &cameraWorldPosition);
                    cameraWorldPosition.x -= lbl_80266A4C.x;
                    cameraWorldPosition.y -= lbl_80266A4C.y;
                    lbl_1_bss_74[index] = cameraWorldPosition.x;
                    lbl_1_bss_6C[index] = cameraWorldPosition.y;
                    index += 1;
                }
}
}
                lbl_1_bss_68 = projectedCameraX[0];
                lbl_1_bss_64 = projectedCameraY[0];
                lbl_1_bss_60 = projectedCameraX[1] - lbl_1_bss_68;
                lbl_1_bss_5C = projectedCameraY[1] - lbl_1_bss_64;
                lbl_1_bss_3C = GXGetTexBufferSize((u16) lbl_1_bss_60, (u16) lbl_1_bss_5C, 6U, 0U, 0U);
                index = 0;
                while (index < 0x28) {
                    lbl_1_bss_7C[index] = HuMemDirectMallocNum(2, (s32) lbl_1_bss_3C, 0x10000000U);
                    index += 1;
                }
                lbl_1_bss_38 = 1;
                lbl_1_bss_40[0] = -1;
            }
            if ((s32) (lbl_1_bss_40[0] + 1) < 0x28) {
                lbl_1_bss_40[0] += 1;
                fn_80026A44(lbl_1_bss_158->mdlId[0x2A], 1U);
                fn_80026760(lbl_1_bss_158->mdlId[0x2A], lbl_80266A4C.x, lbl_80266A4C.y, 0.0f);
                fn_8002D414(lbl_1_bss_158->mdlId[0x2A], (f32) lbl_1_bss_40[0]);
            }
        }
    }
    }

}

void fn_1_106B8(s16 mode, s16 frameNo)
{
}

void fn_1_106BC(s16 mode, s16 frameNo)
{
}

void fn_1_106C0(s16 mode, s16 frameNo)
{
}

void fn_1_106C4(s16 mode, s16 frameNo)
{
}

void fn_1_106C8(s16 mode, s16 frameNo)
{
}

void fn_1_106CC(s16 mode, s16 frameNo)
{
}

void fn_1_106D0(s16 mode, s16 frameNo)
{
    if (frameNo >= 60) {
        fn_80071A9C();
    }
}

void fn_1_10700(OMOBJ *obj)
{
    M678PlayerWork *playerData[4];
    s32 frameNo;
    s32 player;
    s32 animationPlayer;
    s32 soundHandle;
    s32 pan;

    frameNo = fn_80071DE4();
    lbl_1_bss_154 = fn_80071A8C();
    for (player = 0; player < lbl_1_bss_8; player++) {
        playerData[player] = lbl_1_bss_15C[player]->data;
    }
    player = 0;
    for (; player < 0x26; player++) {
        s32 soundEffectIds[0x26] = {
            2263, 2264, 2265, 2266, 2267, 2268, 2269, 2270,
            2271, 2272, 2273, 2274, 2275, 2276, 2277, 2278,
            2279, 2280, 2281, 2282, 2283, 2284, 2285, 2286,
            2287, 2288, 2289, 2290, 2291, 2292, 2293, 2294,
            2295, 2296, 2297, 2298, 2299, 2300
        };

        if (lbl_1_bss_174[player].count > 1) {
            lbl_1_bss_174[player].count = 1;
        }
        if (lbl_1_bss_174[player].count > 0) {
            if (soundEffectIds[player] != -1) {
                soundHandle = HuAudFXPlay(soundEffectIds[player]);
                lbl_1_bss_174[player].handle = soundHandle;
                if (lbl_1_bss_8 > 1 && player != 0x10 && player != 0x11) {
                    if (player % 2) {
                        pan = 0x50;
                    } else {
                        pan = 0x30;
                    }
                    HuAudFXPanning(soundHandle, pan);
                }
            }
            lbl_1_bss_174[player].count--;
        }
    }

    frameNo = fn_80071DE4();
    lbl_1_bss_154 = fn_80071A8C();
    switch (lbl_1_bss_154) {
    case 0:
    case 1:
    case 3:
    case 4:
        break;
    case 2:
        lbl_1_data_110[0] = 30;
        if (frameNo == 20) {
            lbl_1_data_1F0 = HuAudBGMPlay(MSM_STREAM_SLOW_AND_STEADY);
        }
        if (!WipeCheck()) {
            fn_80071A9C();
        }
        break;
    case 5:
        if (frameNo == 0) {
            lbl_1_bss_30 = 1;
        }
        if (lbl_1_data_1EC != -1) {
            fn_80071A9C();
        }
        break;
    case 6:
        if (GameMesFXPlayCheck(fn_80071DF4()) != 0) {
            if (lbl_1_data_1F0 != -1) {
                HuAudSStreamFadeOut(lbl_1_data_1F0, 100);
                lbl_1_data_1F0 = -1;
            }
            if (lbl_1_data_1F4 != -1) {
                HuAudSStreamFadeOut(lbl_1_data_1F4, 100);
                lbl_1_data_1F4 = -1;
            }
        }
        if (frameNo == 1 && lbl_1_bss_8 != 1) {
            if (lbl_1_data_1EC == 4) {
                fn_80071C18(-1, -1, -1, -1);
            } else {
                s32 winnerCount = 0;
                s32 winnerPlayers[4] = { -1, -1, -1, -1 };
                s32 i;
                s32 playerIndex;
                for (i = 0; i < lbl_1_bss_8; i++) {
                    if (playerData[i]->activeFlag == 1) {
                        winnerPlayers[winnerCount] = playerData[i]->characterNo;
                        winnerCount += winnerCount < 4;
                        playerIndex = playerData[i]->groupPlayerIndex;
                        if (!fn_8006A124(65551)) {
                            lbl_80265750[playerIndex].mgCoinBonus = 10;
                        }
                    }
                }
                fn_80071C18(winnerPlayers[0], winnerPlayers[1],
                    winnerPlayers[2], winnerPlayers[3]);
            }
        }
        break;
    case 7:
        fn_80071A9C();
        break;
    case 8:
        if (frameNo == 1) {
            for (animationPlayer = 0; animationPlayer < lbl_1_bss_8;
                animationPlayer++) {
                player = lbl_1_bss_8 == 1
                    ? (lbl_1_bss_34 != 0 ? 2 : 3)
                    : (playerData[animationPlayer]->activeFlag == 1 ? 2 : 3);
                fn_8002CCC0(lbl_1_bss_15C[animationPlayer]->mdlId[2],
                    lbl_1_bss_15C[animationPlayer]->mtnId[player],
                    (f32)lbl_1_data_50[player],
                    (f32)motionBlendFrames[player],
                    lbl_1_data_48[player] != 0 ? 0 : ((1 << 30) | (1 << 0)));
                if ((lbl_1_bss_8 == 1 || lbl_1_data_1EC == 4) && player == 3) {
                    CharFXPlay((s16)playerData[animationPlayer]->characterNo, (573+12));
                }
            }
            if (lbl_1_bss_8 == 1 && lbl_1_bss_34 != 0) {
                lbl_1_bss_14C = GameMesCreate(GAMEMES_MES_MG_RECORD, lbl_1_bss_5BEC);
                HuAudJinglePlay(MSM_STREAM_SUCCESS);
            } else if (lbl_1_bss_8 == 1) {
                HuAudJinglePlay(MSM_STREAM_LOSING);
            }
        }
        break;
    case 9:
    case 10:
    case 11:
        break;
    default:
        break;
    }
}

void fn_1_10D7C(OMOBJ *obj)
{
    M678PlayerSetupWork *playerWork;
    s32 playerOrder[4] = {0, 1, 2, 3};
    s32 player;
    s32 group;
    s32 playerCount;
    s32 characterIndex;
    s32 padNo;
    s32 difficulty;
    s32 motionAttribute;

    playerCount = 0;
    playerWork = obj->data;
    memset(playerWork, 0, 0x74U);
    for (group = 0; group < 4; group++) {
        for (player = 0; player < 4; player++) {
            if (group == lbl_80265728[player].grpNo) {
                playerOrder[playerCount] = player;
                playerCount += 1;
            }
        }
    }

    playerWork->playerIndex = obj->work[0];
    playerWork->groupPlayerIndex =
        playerOrder[playerWork->playerIndex];
    if (playerWork->groupPlayerIndex == 4) {
        playerWork->groupPlayerIndex = playerWork->playerIndex;
    }
    playerWork->characterNo = lbl_80265728[playerWork->groupPlayerIndex].charNo;
    if (playerWork->characterNo >= 11) {
        characterIndex = 9;
    } else {
        characterIndex = playerWork->characterNo;
    }
    playerWork->characterIndex = characterIndex;
    if (lbl_80265728[playerWork->groupPlayerIndex].type != 0) {
        padNo = -1;
    } else {
        padNo = lbl_80265728[playerWork->groupPlayerIndex].padNo;
    }
    playerWork->padNo = padNo;
    if (playerWork->padNo != -1) {
        difficulty = 0;
    } else {
        difficulty = lbl_80265728[playerWork->groupPlayerIndex].comDif;
    }
    playerWork->difficulty = difficulty;
    playerWork->gridRow = 1.0f;
    playerWork->gridColumn = 1.0f;
    playerWork->currentMotion = 0;
    group = 0;
    for (; group < 2; group++) {
        playerWork->motionState[group] = 0;
    }

    obj->mdlId[1] = fn_80025A04(
        fn_80008DD0(DATANUM(0x890000, 25), 0x10000000, 2));
    fn_80026B8C(obj->mdlId[1], 1);
    fn_80026B8C(obj->mdlId[1], 1);
    fn_800268B0(obj->mdlId[1], 0.825f,
        0.825f, 0.825f);
    fn_800269E8(obj->mdlId[1], ((1 << 30) | (1 << 1)));
    obj->mdlId[2] = fn_80057068((s16)playerWork->characterNo, 2);
    group = 0;
    for (; group < 4; group++) {
        obj->mtnId[group] = fn_8005B0B8(
            (s16)playerWork->characterNo, lbl_1_data_38[group]);
    }
    group = 0;
    if (lbl_1_data_48[group] != 0) {
        motionAttribute = 0;
    } else {
        motionAttribute = ((1 << 30) | (1 << 0));
    }
    fn_8002CCC0(obj->mdlId[2], obj->mtnId[group],
        (f32)lbl_1_data_50[group], (f32)motionBlendFrames[group], motionAttribute);
    fn_80026B8C(obj->mdlId[2], 1);
    fn_800273EC(obj->mdlId[2], 1.0f, 1.0f, 1.0f);
    fn_8005DDEC(1);
    obj->objFunc = fn_1_1BC8;
}

extern s32 lbl_1_bss_124;

extern s32 lbl_1_bss_128;

extern s32 lbl_1_bss_12C;

extern s32 lbl_1_bss_138[2];

extern s32 lbl_1_bss_140;

extern s32 lbl_1_bss_144;

extern s16 lbl_1_bss_2A4[8];

extern HSF_OBJECT *lbl_1_bss_E368[8][5];

/* Optional model consistency check. */
static inline void DebugCheckModels(OMOBJ *obj, s32 enabled)
{
    s32 model;
    s32 count;

    if (enabled) {
        count = 0;
        for (model = 0; model < 0x2C; model++) {
            if (obj->mdlId[model] >= 0) {
                count++;
            }
        }
        if (count == 0 || count == 0x2C) {
            return;
        }
        for (model = 0; model < 0x2C; model++) {
            if (obj->mdlId[model] < 0) {
                count = model;
                break;
            }
        }
        if (count < 0x14) {
            for (model = count; model < 0x14; model++) {
                fn_800269E8(obj->mdlId[model], (1 << 0));
            }
        } else {
            for (model = count; model < 0x2C; model++) {
                fn_80026A44(obj->mdlId[model], (1 << 0));
            }
        }
    }
}

/* Creates the models, sprite controls, and score displays. */
void fn_1_11148(OMOBJ *obj) {
    ANIMDATA *spriteAnimation;
    s16 spriteId;
    s32 side;
    s32 itemIndex;
    s32 modelIndex;
    s32 index;
    M678PlayerSetupWork * playerData[4];
    for (index = 0; index < lbl_1_bss_8; index++)
    {
        playerData[index] = lbl_1_bss_15C[index]->data;
    }
    memset(lbl_1_bss_70A8, 0, 0x72C0U);
    memset(lbl_1_bss_2F88, 0, 0x1650U);
    memset(lbl_1_bss_5BF0, 0, 0x280U);
    memset(lbl_1_bss_45FC, 0, 0x2D0U);
    memset(& lbl_1_bss_45D8.state, 0, 0x24U);
    for (modelIndex = 0; modelIndex < 2; modelIndex++)
    {
        for (index = 0; index < 0x66; index++)
        {
            lbl_1_bss_70A8[modelIndex][index].index = index;
        }
        for (index = 0; index < 0x66; index += 1)
        {
            lbl_1_bss_2F88[modelIndex].entries[index].index = index;
        }
    }
    modelIndex = 0;
    obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(lbl_1_bss_8 == 1 ? DATANUM(0x890000, 9) : DATANUM(0x890000, 10), 0x10000000, 2));
    fn_80026B8C(obj->mdlId[modelIndex], 2);
    fn_800269E8(obj->mdlId[modelIndex], ((1 << 30) | (1 << 0)));
    for (index = 0; index < 3; index++)
    {
        s32 modelIds[3] = { DATANUM(0x890000, 22), DATANUM(0x890000, 23), DATANUM(0x890000, 24) };
        modelIndex = index + 0xF;
        obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(modelIds[index], 0x10000000, 2));
        fn_80026B8C(obj->mdlId[modelIndex], 2);
        fn_800269E8(obj->mdlId[modelIndex], ((1 << 30) | (1 << 0)));
    }
    for (index = 0; index < lbl_1_bss_8; index++)
    {
        s32 playerModelIds[2][7] = { { DATANUM(0x890000, 11), DATANUM(0x890000, 13), DATANUM(0x890000, 14), DATANUM(0x890000, 19), DATANUM(0x890000, 17), DATANUM(0x890000, 20), DATANUM(0x890000, 21) }, { DATANUM(0x890000, 12), DATANUM(0x890000, 15), DATANUM(0x890000, 16), DATANUM(0x890000, 19), DATANUM(0x890000, 18), DATANUM(0x890000, 20), DATANUM(0x890000, 21) } };
        for (itemIndex = 0; itemIndex < 7; itemIndex++)
        {
            s32 playerModelSlots[7] = { 1, 3, 5, 7, 9, 0xB, 0xD };
            s32 pauseModel[7] = { 1, 1, 1, 0, 0, 1, 0 };
            modelIndex = index + playerModelSlots[itemIndex];
            obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(playerModelIds[index][itemIndex], 0x10000000, 2));
            fn_80026B8C(obj->mdlId[modelIndex], 2);
            if (pauseModel[itemIndex] != 0)
            {
                fn_800269E8(obj->mdlId[modelIndex], ((1 << 30) | (1 << 1)));
            }
        }
        fn_800269E8(obj->mdlId[3], ((1 << 30) | (1 << 0)));
        fn_800269E8(obj->mdlId[5], ((1 << 30) | (1 << 0)));
        fn_800269E8(obj->mdlId[4], ((1 << 30) | (1 << 0)));
        fn_800269E8(obj->mdlId[6], ((1 << 30) | (1 << 0)));
    }
    if (lbl_1_bss_8 == 1)
    {
        fn_80026760(obj->mdlId[3], - 400.0f, 0.0f, 150.0f);
        fn_80026760(obj->mdlId[5], - 400.0f, 0.0f, 150.0f);
        modelIndex = 19;
        obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(DATANUM(0x890000, 26), 0x10000000, 2));
        fn_80026B8C(obj->mdlId[modelIndex], 2);
        fn_800269E8(obj->mdlId[modelIndex], ((1 << 30) | (1 << 0)));
    }
    else
    {
        for (index = 0; index < lbl_1_bss_8; index++)
        {
            side = index == 0 ? - 1 : 1;
            fn_80026760(obj->mdlId[index + 1], (f32)(side * 0x15E), 0.0f, 0.0f);
            fn_80026760(obj->mdlId[index + 7], (f32)(side * 0x15E), 0.0f, 0.0f);
            fn_80026760(obj->mdlId[index + 9], (f32)(side * 0x15E), 0.0f, 0.0f);
            fn_80026760(obj->mdlId[index + 0xB], (f32)(side * 0x15E), 0.0f, 0.0f);
            fn_80026760(obj->mdlId[index + 0xD], (f32)(side * 0x15E), 0.0f, 0.0f);
            fn_80026760(obj->mdlId[index + 3], (f32)(side * 0x55), 0.0f, 150.0f);
            fn_80026760(obj->mdlId[index + 5], (f32)(side * 0x55), 0.0f, 150.0f);
        }
    }
    for (index = 0; index < lbl_1_bss_8; index++)
    {
        char hookNames[2][0x64] = { "itemhook1", "itemhook1" };
        if (index < 2)
        {
            fn_8002741C(obj->mdlId[index + 3], hookNames[index], lbl_1_bss_15C[index]->mdlId[2]);
        }
    }
    for (index = 0; index < 8; index++)
    {
        for (itemIndex = 0; itemIndex < ((index < 3) ? 5 : 1); itemIndex++)
        {
            s32 modelOffsets[8] = { 0x14, 0x19, 0x1E, 0x23, 0x25, 0x26, 0x27, 0x28 };
            s32 modelDataIds[2][8] = { { DATANUM(0x890000, 28), DATANUM(0x890000, 33), DATANUM(0x890000, 48), DATANUM(0x890000, 53), DATANUM(0x890000, 57), DATANUM(0x890000, 58), DATANUM(0x890000, 54), DATANUM(0x890000, 55) }, { DATANUM(0x890000, 28), DATANUM(0x890000, 38), DATANUM(0x890000, 43), DATANUM(0x890000, 53), DATANUM(0x890000, 57), DATANUM(0x890000, 58), DATANUM(0x890000, 54), DATANUM(0x890000, 55) } };
            s32 scoreIndexOffsets[2][3] = { { 1, 2, 5 }, { 1, 3, 4 } };
            char modelPrefix[12] = "pazz_block_";
            char colorNames[5][0x1E] = { "red0", "blue0", "yellow0", "green0", "purple0" };
            char numberNames[6][0x1E] = { "0", "1", "2", "3", "4", "5" };
            char nameBuffer[0xFF] = { 0 };
            modelIndex = itemIndex + modelOffsets[index];
            obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(itemIndex + modelDataIds[lbl_1_bss_8 == 1 ? 0 : 1][index], 0x10000000, 2));
            fn_80026B8C(obj->mdlId[modelIndex], 2);
            fn_800269E8(obj->mdlId[modelIndex], (1 << 0));
            if (modelOffsets[index] == 0x26)
            {
                fn_8002D86C(obj->mdlId[modelIndex], 1.375f);
            }
            if (index < 3)
            {
                strcpy(nameBuffer, modelPrefix);
                strcat(nameBuffer, colorNames[itemIndex]);
                strcat(nameBuffer, numberNames[scoreIndexOffsets[lbl_1_bss_8 == 1 ? 0 : 1][index]]);
            }
            else
            {
                char modelNames[5][0x1E] = { "pazz_block_jama", "FX_flame1-FX_flame1", "FX_limit-FX_flame2", "pazz_block_STAR", "pazz_block_Watch" };
                strcpy(nameBuffer, modelNames[index - 3]);
            }
            lbl_1_bss_E368[index][itemIndex] = fn_80026BF4(obj->mdlId[modelIndex], nameBuffer);
        }
    }
    for (index = 0; index < 2; index++)
    {
        s32 modelSlotIndices[2] = { 0x26, 0x25 };
        modelIndex = modelSlotIndices[index];
        fn_800269E8(obj->mdlId[modelIndex], ((1 << 30) | (1 << 0)));
        fn_80026A44(obj->mdlId[modelIndex], (1 << 0));
        fn_80026760(obj->mdlId[modelIndex], 0.0f, 2000.0f, 0.0f);
    }
    modelIndex = 18;
    obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(DATANUM(0x890000, 27), 0x10000000, 2));
    fn_80026B8C(obj->mdlId[modelIndex], 4);
    fn_800269E8(obj->mdlId[modelIndex], (1 << 0));
    modelIndex = 41;
    obj->mdlId[modelIndex] = fn_800260F4(fn_1_1350C);
    fn_80026B8C(obj->mdlId[modelIndex], 3);
    modelIndex = 42;
    obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(DATANUM(0x890000, 56), 0x10000000, 2));
    fn_800269E8(obj->mdlId[modelIndex], (1 << 0));
    fn_80026B8C(obj->mdlId[modelIndex], 0);
    fn_8002583C(1, 0, fn_1_130C4);
    fn_8002583C(1, 8, fn_1_13118);
    modelIndex = 43;
    obj->mdlId[modelIndex] = fn_800260F4(fn_1_1332C);
    fn_80026B8C(obj->mdlId[modelIndex], 5);
    spriteAnimation = HuSprAnimRead(fn_80008DD0(DATANUM(0x890000, 4), 0x10000000, 2));
    lbl_1_bss_120 = (s32) HuSprGrpCreate(0x14);
    index = 0;
    while (index < 0x14)
    {
        spriteId = HuSprCreate(spriteAnimation, 0x33, 0);
        HuSprGrpMemberSet((s16) lbl_1_bss_120, (s16) index, spriteId);
        HuSprAttrSet((s16) lbl_1_bss_120, (s16) index, HUSPR_ATTR_ADDCOL);
        HuSprAttrSet((s16) lbl_1_bss_120, (s16) index, HUSPR_ATTR_DISPOFF);
        index += 1;
    }
    spriteAnimation = HuSprAnimRead(fn_80008DD0(DATANUM(0x890000, 5), 0x10000000, 2));
    lbl_1_bss_11C = (s32) HuSprGrpCreate(4);
    index = 0;
    while (index < 4)
    {
        spriteId = HuSprCreate(spriteAnimation, (s16)(index + 0x34), 0);
        HuSprGrpMemberSet((s16) lbl_1_bss_11C, (s16) index, spriteId);
        HuSprAttrSet((s16) lbl_1_bss_11C, (s16) index, HUSPR_ATTR_DISPOFF);
        HuSprTPLvlSet((s16) lbl_1_bss_11C, (s16) index, 1.0f - ((f32) index / 4.0f));
        index += 1;
    }
    for (index = lbl_1_bss_8 == 1 ? 0 : 5; index <= (lbl_1_bss_8 == 1 ? 5 : 6); index++)
    {
        f32 scoreX[2][7] = { { 480.0f, 480.0f, 480.0f, 450.0f, 510.0f, 102.0f, 0.0f }, { 480.0f, 480.0f, 480.0f, 450.0f, 510.0f, 248.0f, 328.0f } };
        f32 scoreY[7] = { 144.0f, 123.0f, 283.0f, 395.0f, 395.0f, 176.0f, 176.0f };
        f32 boxWidths[7] = { 120.0f, 120.0f, 85.0f, 140.0f, 85.0f, 85.0f, 0.0f };
        s32 maxDigits[7] = { 4, 4, 2, 2, 2, 2, 2 };
        f32 scoreOffsets[2][7] = { { - 24.0f, - 24.0f, - 8.0f, - 8.0f, - 8.0f, - 17.0f, - 17.0f }, { - 30.0f, - 30.0f, - 8.0f, - 8.0f, - 8.0f, - 10.0f, - 8.0f } };
        lbl_1_bss_2B4[index] = MgScoreCreate(DATANUM(0x9b0000, 49), - 1, 1);
        MgScorePosSet(lbl_1_bss_2B4[index], scoreX[lbl_1_bss_8 == 1 ? 0 : 1][index] + scoreOffsets[lbl_1_bss_8 == 1 ? 0 : 1][index], scoreY[index]);
        MgScoreValueSet(lbl_1_bss_2B4[index], 0);
        MgScoreMaxDigitSet(lbl_1_bss_2B4[index], maxDigits[index]);
        if (index == 1)
        {
            MgScoreColorSet(lbl_1_bss_2B4[index], 0x42U, 0xFFU, 0x7AU);
        }
        else
        {
            MgScoreColorSet(lbl_1_bss_2B4[index], 0xFFU, 0xFFU, 0xFFU);
        }
        if (index < 5 && index != 4 && index != 1)
        {
            lbl_1_bss_2A4[index - (index > 4)] = MgScoreBoxCreate((s16) boxWidths[index], index != 0 ? 0x30 : 0x48);
            if (index == 0)
            {
                MgScoreBoxPosSet(lbl_1_bss_2A4[index - (index > 4)], scoreX[lbl_1_bss_8 == 1 ? 0 : 1][index], (scoreY[index] + scoreY[index + 1]) / 2.0f);
            }
            else
            {
                if (index != 3)
                {
                    MgScoreBoxPosSet(lbl_1_bss_2A4[index - (index > 4)], scoreX[lbl_1_bss_8 == 1 ? 0 : 1][index], scoreY[index]);
                }
                else
                {
                    MgScoreBoxPosSet(lbl_1_bss_2A4[index - (index > 4)], (scoreX[lbl_1_bss_8 == 1 ? 0 : 1][index] + scoreX[lbl_1_bss_8 == 1 ? 0 : 1][index + 1]) / 2.0f, scoreY[index]);
                }
            }
        }
    }
    lbl_1_bss_144 = (s32) espEntry(DATANUM(0x890000, 0), 0x400, 0);
    lbl_1_bss_140 = (s32) espEntry(DATANUM(0x890000, 1), 0x63, 0);
    lbl_1_bss_124 = (s32) espEntry(DATANUM(0x9b0000, 69), 0x63, 0);
    index = 0;
    while (index < 2)
    {
        lbl_1_bss_138[index] = (s32) espEntry(DATANUM(0x890000, 8), 0x400, 0);
        lbl_1_bss_130[index] = (s32) espEntry(DATANUM(0x890000, 3), 0x400, 0);
        index += 1;
    }
    espBankSet((s16) lbl_1_bss_138[1], 1);
    lbl_1_bss_12C = (s32) espEntry(DATANUM(0x890000, 6), 0x400, 0);
    lbl_1_bss_128 = (s32) espEntry(DATANUM(0x890000, 7), 0x400, 0);
    espPosSet((s16) lbl_1_bss_140, 482.0f, 394.0f);
    espPosSet((s16) lbl_1_bss_12C, 483.0f, 76.0f);
    espPosSet((s16) lbl_1_bss_128, 483.0f, 238.0f);
    espPosSet((s16) lbl_1_bss_124, 438.0f, 122.0f);
    espDispOff((s16) lbl_1_bss_144);
    if (lbl_1_bss_8 != 1)
    {
        espDispOff((s16) lbl_1_bss_140);
        espDispOff((s16) lbl_1_bss_12C);
        espDispOff((s16) lbl_1_bss_128);
        espDispOff((s16) lbl_1_bss_124);
    }
    obj->objFunc = fn_1_4FB0;
    DebugCheckModels(obj, 0);
}

void fn_1_129B0(OMOBJ *obj)
{
    lbl_1_bss_0 = 1;
    obj->objFunc = fn_1_10700;
}

extern s32 lbl_1_bss_148;

extern HUPROCESS *lbl_1_bss_150;

/* Sets the minigame camera, lighting, shadows, and sequence state. */
void fn_1_129D0(void)
{
    s32 i;
    s16 nightFlag;
    s32 gameMode;

    OSReport("******* M678 ObjectSetup *********\n");
    nightFlag = lbl_802C0278;
    if ((s32)nightFlag == 1) {
        lbl_1_bss_2C = 1;
    }
    lbl_1_bss_5BEC = fn_8003C860(7);
    lbl_802BFF80 = 0;
    for (i = 0; i < 0x26; i++) {
        lbl_1_bss_174[i].count = 0;
        lbl_1_bss_174[i].handle = -1;
    }

    lbl_1_bss_150 = fn_80080418();
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 10.0f, 500.0f, 10000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 640, 480);
    lbl_802C028C = 5900.0f;
    lbl_80266A4C.x = 0.0f;
    lbl_80266A4C.y = 350.0f;
    lbl_80266A4C.z = 0.0f;
    lbl_80266A40.x = 0.0f;
    lbl_80266A40.y = 0.0f;
    lbl_80266A40.z = 0.0f;
    fn_800285BC();

    {
        HuVecF lightPosition = { 0.0f, 5000.0f, 0.0f };
        HuVecF dayDirection = { 0.2f, -1.0f, -0.4f };
        HuVecF nightDirection = { 0.0f, -1.0f, -1.0f };
        HuVecF shadowPosition;
        HuVecF shadowUp;
        HuVecF shadowTarget;
        GXColor lightColor = { 255, 255, 255, 255 };
        GXColor nightColor = { 192, 192, 255, 255 };
        GXColor ambientColor = { 192, 255, 192, 255 };

        lbl_1_bss_148 = fn_800286D0(&lightPosition,
            lbl_1_bss_2C ? &nightDirection : &dayDirection,
            lbl_1_bss_2C ? &lightColor : &lightColor);
        Hu3DGLightStaticSet((s16)lbl_1_bss_148, 1);
        Hu3DGLightInfinitytSet((s16)lbl_1_bss_148);
        shadowPosition.x = -1500.0f;
        shadowPosition.y = 10000.0f;
        shadowPosition.z = 1500.0f;
        shadowUp.x = 0.0f;
        shadowUp.y = 1.0f;
        shadowUp.z = 0.0f;
        shadowTarget.x = 0.0f;
        shadowTarget.y = 0.0f;
        shadowTarget.z = 0.01f;
        fn_8002A3B0(8.0f, 5000.0f, 13000.0f, 1);
        fn_8002A7D8(0.5f, 1);
        fn_8002A674(&shadowPosition, &shadowUp, &shadowTarget, 1);
        fn_8002AA3C(0, 0, 0, 1);
    }
    lbl_1_bss_14C = -1;
    lbl_1_data_1EC = -1;
    gameMode = lbl_80265B70.subGameNo;
    lbl_1_bss_8 = gameMode == 1 ? 2 : 1;
    fn_80070788(&lbl_1_data_1FC);
}

extern OMOBJ *lbl_1_bss_16C;

extern OMOBJ *lbl_1_bss_170;

void fn_1_12EB4(s16 mode, s16 frameNo)
{
    OMOBJ *obj;
    s32 player;

    lbl_1_bss_158 = fn_8003AB38(lbl_1_bss_150, 101, 44U, 0U, 1, fn_1_11148);
    player = 0;
    while (player < lbl_1_bss_8) {
        obj = lbl_1_bss_15C[player] =
            fn_8003AB38(lbl_1_bss_150, 100, 3U, 4U, 0, fn_1_10D7C);
        obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M678PlayerSetupWork), 0x10000000U);
        obj->work[0] = (u32)player;
        player += 1;
    }
    lbl_1_bss_16C = fn_8003AB38(lbl_1_bss_150, 32730, 0U, 0U, -1, fn_8003D4FC);
    lbl_1_bss_16C->work[0] = 1;
    lbl_1_bss_170 = fn_8003AB38(lbl_1_bss_150, 102, 0U, 0U, -1, fn_1_129B0);
    fn_80071A9C();
}

void fn_1_1300C(s16 mode, s16 frameNo)
{
    void *discardedData;
    s32 player;

    fn_80071A9C();
    player = 0;
    while (player < lbl_1_bss_8) {
        discardedData = lbl_1_bss_15C[player]->data;
        player += 1;
    }
    if (lbl_1_bss_14C >= 0) {
        GameMesKill(lbl_1_bss_14C);
    }
    GameMesClose();
    HuAudAllStop();
    lbl_802BFF80 = 15;
    fn_8003A5E4((s16)lbl_1_data_1F8, 1);
}

void fn_1_130C4(s16 layerNo)
{
    if (lbl_1_bss_38 == 1) {
        GXPixModeSync();
        GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
        GXSetDstAlpha(0, 0);
        GXSetAlphaUpdate(1);
    }
}

extern GXRenderModeObj *RenderMode;

void fn_1_13118(s16 layerNo)
{
    /* Consumed scratch layout: projection uses rows 0..3; the subsequent
     * position matrix uses rows 3..5 after GXSetProjection has consumed it.
     */
    f32 matrices[6][4];
    GXColor clearColor = {0, 0, 0, 0};

    if (lbl_1_bss_38 == 1) {
        C_MTXOrtho(matrices, 0.0f, 480.0f,
                   0.0f, 640.0f,
                   0.0f, -1.0f);
        GXSetProjection(matrices, GX_ORTHOGRAPHIC);
        PSMTXIdentity(&matrices[3]);
        GXLoadPosMtxImm(&matrices[3], GX_PNMTX0);
        GXSetCopyFilter(0U, RenderMode->sample_pattern, 0U, RenderMode->vfilter);
        GXPixModeSync();
        GXSetTexCopySrc((u16)lbl_1_bss_68, (u16)lbl_1_bss_64,
                        (u16)lbl_1_bss_60, (u16)lbl_1_bss_5C);
        GXSetTexCopyDst((u16)lbl_1_bss_60, (u16)lbl_1_bss_5C, GX_TF_RGBA8, 0U);
        GXSetCopyClear(clearColor, 0xFFFFFFU);
        GXCopyTex(lbl_1_bss_7C[lbl_1_bss_40[0]], 1U);
        GXPixModeSync();
        GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
        GXSetCopyFilter(RenderMode->aa, RenderMode->sample_pattern, 1U, RenderMode->vfilter);
    }
}

extern char lbl_1_data_2B0[15];

void fn_1_1332C(HU3D_MODEL *modelP, Mtx *mtx)
{
    Mtx matrix;
    HuVecF position;
    s32 spriteIds[4];
    s32 i;

    spriteIds[0] = lbl_1_bss_138[0];
    spriteIds[1] = lbl_1_bss_130[0];
    spriteIds[2] = lbl_1_bss_130[1];
    spriteIds[3] = lbl_1_bss_138[1];
    GXLoadPosMtxImm(*mtx, GX_PNMTX0);
    fn_80023398(lbl_1_bss_158->mdlId[18], lbl_1_data_2B0, matrix);
    i = 0;
    while (i < 4) {
        espDispOff((s16)spriteIds[i]);
        i += 1;
    }
    if (0.0f != matrix[0][0]) {
        i = 0;
        while (i < 4) {
            float positions[4] = {-86.0f, 128.0f, 188.0f, 235.0f};
            HuVecF projected;
            HuVecF source;
            espDispOn((s16)spriteIds[i]);
            position.x = positions[i];
            position.y = 0.0f;
            position.z = 0.0f;
            PSMTXMultVec(matrix, &position, &position);
            source.x = position.x;
            source.y = position.y;
            source.z = position.z;
            Hu3D3Dto2D(&source, 1, &projected);
            position.x = projected.x;
            position.y = projected.y;
            position.z = projected.z;
            espPosSet((s16)spriteIds[i], position.x, position.y);
            espScaleSet((s16)spriteIds[i], matrix[0][0], matrix[1][1]);
            i += 1;
        }
    }
}

void HuSprTexLoad(ANIMDATA *, s16, s16, GXTexWrapMode, GXTexWrapMode, GXTexFilter);

/* Sprite animation references and sprite group members. */
typedef struct M678SpriteAnimation {
    u32 dataNum;
    u16 useCnt;
    ANIMDATA *anim;
} M678SpriteAnimation;
typedef struct M678SpriteEntry {
    s16 memberNo;
    s16 animNo;
} M678SpriteEntry;
extern M678SpriteAnimation espanim[];

extern M678SpriteEntry esprite[];

/* Optional entry consistency check. */
static inline void DebugCheckEntries(const f32 *horizontalOffset, s32 enabled)
{
    s32 index;
    if (enabled) {
        if (*horizontalOffset != 0.0f) {
            return;
        }
        for (index = 0; index < 0x66; index++) {
            if (lbl_1_bss_70A8[0][index].kind < 0 || lbl_1_bss_70A8[0][index].kind > 7 ||
                lbl_1_bss_70A8[1][index].kind < 0 || lbl_1_bss_70A8[1][index].kind > 7) {
                return;
            }
        }
    }
}

/* Draws the piece models, overlays, texture effects, and numeric displays. */
void fn_1_1350C(HU3D_MODEL *modelP, f32 (*mtx)[3][4]) {
    f32 entryX[2][0x66];
    f32 entryY[2][0x66];
    f32 entryZ[2][0x66];
    M678PlayerSetupWork *playerData[4];
    s32 cornerX[4] = { 0, 1, 1, 0 };
    s32 cornerY[4] = { 0, 0, 1, 1 };
    s32 entryOpacity[2][0x66] = {{0}};

    Mtx entryMatrices[2][0x66];
    M678LinkEntry *drawEntries[2][0x66] = {{NULL}};

    Mtx scaledMatrix;
    Mtx effectMatrix;
    s32 drawPaused;
    HSF_DATA * entryData;
    f32 drawValue;
    f32 entryPositionY;
    f32 entryDepth;
    s32 pulseValue;
    s32 columnIndex;
    s32 shakeFrame;
    HU3D_MODEL * entryModel;
    s32 modelVariant;
    s32 highlightAlpha;
    s32 overlayModelIndex;
    s32 overlayObjectIndex;
    HU3D_MODEL * overlayModel;
    HSF_DATA * overlayData;
    s32 overlayOpacity;
    s32 effectAlpha;
    f32 shakeY;
    f32 shakeX;
    f32 textureX;
    f32 textureY;
    f32 displayLeft;
    f32 displayRight;
    f32 displayTop;
    f32 displayBottom;
    s32 digit;

    f32 vertexX;
    f32 vertexY;
    f32 textureS;
    f32 textureT;
    f32 displayAlpha;
    f32 digitX;
    f32 digitY;

    s32 drawOverlay;

    HSF_MATERIAL * firstMaterial;
    s32 drawIndex;

    f32 entryShift;
    f32 digitScale;

    s32 opacity;
    s32 shakeLevel;

    s32 drawPass;
    s32 entryIndex;
    s32 playerIndex;

    M678LinkEntry * overlayEntry;
    M678LinkEntry * entry;
    {
        drawPaused = fn_8003B748() != 0 && lbl_1_bss_8 == 1;
        for (entryIndex = 0; entryIndex < lbl_1_bss_8; entryIndex++)
        {
            playerData[entryIndex] = lbl_1_bss_15C[entryIndex]->data;
        }
        GXLoadPosMtxImm(*mtx, GX_PNMTX0);
        GXSetNumTevStages(1U);
        GXSetNumTexGens(1U);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0U, GX_PTIDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
        GXSetNumChans(1U);
        GXSetChanCtrl(GX_COLOR0A0, 0U, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXSetAlphaCompare(GX_ALWAYS, 0U, GX_AOP_AND, GX_ALWAYS, 0U);
        GXSetZMode(1U, GX_LEQUAL, 1U);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0U);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0U);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0U);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0U);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0U);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_SRCALPHA, GX_LO_NOOP);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0U);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0U);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        playerIndex = 0;
        while (playerIndex < lbl_1_bss_8)
        {
            drawPass = 0;
            while (drawPass < 2)
            {
                entryIndex = 0;
                while (entryIndex < 0x66)
                {
                    if ((lbl_1_bss_70A8[playerIndex][entryIndex].kind != 0) && (((drawPass == 0) && (lbl_1_bss_70A8[playerIndex][entryIndex].category != 5) && (lbl_1_bss_70A8[playerIndex][entryIndex].category != 6)) || ((drawPass == 1) && ((lbl_1_bss_70A8[playerIndex][entryIndex].category == 5) || (lbl_1_bss_70A8[playerIndex][entryIndex].category == 6)))))
                    {
                        {
                            s32 renderTable[24] = { 128, 128, 128, 128, 16, 16, 124, 124, 8, 16, 96, 16, 16, 96, 128, 32, 8, 120, 104, 32, 104, 96, 96, 96 };
                        }
                        entry = &lbl_1_bss_70A8[playerIndex][entryIndex];
                        drawValue = ((60.0f * entry->gridRow) + (f32)(lbl_1_bss_8 <= 1 ? 0 : playerIndex == 0 ? - 0x15E : 0x15E)) - 150.0f;
                        entryPositionY = - 54.0f * (1.5f + (entry->gridColumn - 15.0f));
                        entryDepth = 0.0f;
                        opacity = (s32)(entry->fadeTimer > 0 ? (entry->fadeTimer % 2 != 0 ? 0.0f : 255.0f * (1.0f - (f32) entry->fadeTimer / lbl_1_data_130[playerIndex])) : 255.0f);
                        opacity = (s32)((f32)(u8) opacity * entry->opacityScale);
                        drawValue += 60.0f * lbl_1_data_60[playerIndex];
                        entryPositionY += - 54.0f * lbl_1_data_68[playerIndex];
                        if ((entry->category == 5) && (playerData[playerIndex]->state[6] == 1))
                        {
                            entryShift = 0.0f;
                            entryShift = 0.125f * (playerData[playerIndex]->state[7] > 3 ? 1.0f : (f32)playerData[playerIndex]->state[7] / 3.0f);
                            drawValue -= 60.0f * entryShift;
                            entryPositionY += - 54.0f * entryShift;
                            entryDepth = 8.0f;
                        }
                        else
                        {
                            if (entry->category == 6)
                            {
                                entryDepth = 4.0f;
                            }
                        }
                        if ((lbl_1_bss_0 == 2) && (lbl_1_data_1EC == - 1) && (lbl_1_data_168[playerIndex][(s32)(0.5f + entry->gridRow)] >= 8) && ((entry->category == 0) || (entry->category == 3)))
                        {
                            shakeX = 0.0f;
                            shakeY = 0.0f;
                            columnIndex = (s32)(0.5f + entry->gridRow);
                            {
                                s32 shakeDirections[4] = { - 1, 0, 1, 0 };
                                s32 shakePeriods[5] = { 0x28, 0x14, 0xC, 4, 4 };
                                f32 shakeAmounts[5] = { 0.0f, 0.0f, 1.5f, 1.5f, 1.5f };
                                shakeFrame = lbl_1_bss_14;
                                shakeLevel = lbl_1_data_168[playerIndex][columnIndex] - 8;
                                shakeLevel = shakeLevel > 4 ? 4 : shakeLevel;
                                shakeY = shakeAmounts[shakeLevel] * ((f32)(- 0x36 * (shakeDirections[shakeFrame % 4] * (((s32)(shakeFrame % shakePeriods[shakeLevel]) < 4) ? 1 : 0))) / 32.0f);
                                drawValue += shakeX;
                                entryPositionY += shakeY;
                            }
                        }
                        pulseValue = (s32)((entry->kind == 2 && lbl_1_data_E8[playerIndex] <= 0) ? 96.0f * ((f32)(0x3C - (lbl_1_bss_14 % 60)) / 60.0f) : 0.0f);
                        pulseValue = 0;
                        entryX[playerIndex][entryIndex] = drawValue;
                        entryY[playerIndex][entryIndex] = entryPositionY;
                        entryZ[playerIndex][entryIndex] = entryDepth;
                        entryOpacity[playerIndex][entryIndex] = (u8) opacity;
                        drawEntries[playerIndex][entryIndex] = entry;
                        {
                            s32 modelOffsets[8] = { - 1, 0x14, 0x19, 0x1E, 0x23, - 1, 0x19, 0x1E };
                            s32 objectIndices[8] = { - 1, 0, 1, 2, 3, - 1, 1, 2 };
                            modelVariant = entry->variant == 0 ? 0 : entry->variant - 1 + (((entry->variant >= 3 && entry->variant < 5) && lbl_1_bss_8 == 1) && (lbl_1_data_C8[0] >= 0x3C && lbl_1_data_C8[0] < 0x50));
                            entryModel = & lbl_802C01C8[lbl_1_bss_158->mdlId[modelVariant + modelOffsets[entry->kind]]];
                            entryData = entryModel->hsf;
                            {
                                firstMaterial = entryData->material;
                                drawIndex = 0;
                                while (drawIndex < entryData->materialNum)
                                {
                                    firstMaterial->invAlpha = 1.0f - ((f32)(u8) opacity / 255.0f);
                                    if ((u8) opacity != 0xFF)
                                    {
                                        firstMaterial->pass = (firstMaterial->pass & 0xF0) | 1;
                                    }
                                    else
                                    {
                                        firstMaterial->pass &= 0xF0;
                                    }
                                    drawIndex += 1;
                                    firstMaterial += 1;
                                }
                            }
                            PSMTXTrans(entryMatrices[playerIndex][entryIndex], drawValue, - 27.0f + entryPositionY, entryDepth);
                            PSMTXConcat(* mtx, entryMatrices[playerIndex][entryIndex], entryMatrices[playerIndex][entryIndex]);
                            fn_8002472C();
                            if ((drawPaused == 0) && ((entry->variant != 0) || (lbl_1_bss_14 % 2 != 0) || ((modelOffsets[entry->kind] != 0x14) && (modelOffsets[entry->kind] != 0x19) && (modelOffsets[entry->kind] != 0x1E))))
                            {
                                fn_80024850(lbl_1_bss_158->mdlId[modelVariant + modelOffsets[entry->kind]], lbl_1_bss_E368[objectIndices[entry->kind]][modelVariant], entryMatrices[playerIndex][entryIndex]);
                            }
                        }
                    }
                    entryIndex += 1;
                }
                drawPass += 1;
            }
            playerIndex += 1;
        }
        GXSetZMode(1U, GX_LEQUAL, 0U);
        playerIndex = 0;
        while (playerIndex < lbl_1_bss_8)
        {
            highlightAlpha = (s32)(192.0f + (63.0f * ((f32)(lbl_1_bss_14 % 30) / 30.0f)));
            entryIndex = 0;
            while (entryIndex < 0x66)
            {
                overlayEntry = &lbl_1_bss_70A8[playerIndex][entryIndex];
                if ((overlayEntry->kind != 0) && (entryOpacity[playerIndex][entryIndex] > 0) && ((overlayEntry->kind == 2) || (overlayEntry->kind == 3) || (overlayEntry->kind == 6) || (overlayEntry->kind == 7)))
                {
                    for (drawPass = !(overlayEntry->overlay != 0 && (overlayEntry->kind == 2 || overlayEntry->kind == 3)); drawPass < 2; drawPass++)
                    {
                        overlayModelIndex = drawPass == 0 ? (overlayEntry->kind == 2 ? 0x27 : 0x28) : 0x25;
                        overlayObjectIndex = drawPass == 0 ? (overlayEntry->kind == 2 ? 6 : 7) : 4;
                        overlayModel = & lbl_802C01C8[lbl_1_bss_158->mdlId[overlayModelIndex]];
                        overlayData = overlayModel->hsf;
                        {
                            HSF_MATERIAL *secondMaterial;
                            secondMaterial = overlayData->material;
                            overlayOpacity = entryOpacity[playerIndex][entryIndex];
                            drawIndex = 0;
                            while (drawIndex < overlayData->materialNum)
                            {
                                secondMaterial->invAlpha = 1.0f - ((f32) overlayOpacity / 255.0f);
                                if (overlayOpacity != 0xFF)
                                {
                                    secondMaterial->pass = (secondMaterial->pass & 0xF0) | 1;
                                }
                                else
                                {
                                    secondMaterial->pass &= 0xF0;
                                }
                                drawIndex += 1;
                                secondMaterial += 1;
                            }
                        }
                        fn_8002472C();
                        if (drawPaused == 0)
                        {
                            if (drawPass == 0)
                            {
                                PSMTXCopy(entryMatrices[playerIndex][entryIndex], scaledMatrix);
                                PSMTXScaleApply(scaledMatrix, scaledMatrix, 0.925f, 0.925f, 1.0f);
                                scaledMatrix[0][3] = entryMatrices[playerIndex][entryIndex][0][3];
                                scaledMatrix[1][3] = entryMatrices[playerIndex][entryIndex][1][3];
                                scaledMatrix[2][3] = 1.0f + entryMatrices[playerIndex][entryIndex][2][3];
                                fn_80024850(lbl_1_bss_158->mdlId[overlayModelIndex], lbl_1_bss_E368[overlayObjectIndex][0], scaledMatrix);
                            }
                            else
                            {
                                fn_80024850(lbl_1_bss_158->mdlId[overlayModelIndex], lbl_1_bss_E368[overlayObjectIndex][0], entryMatrices[playerIndex][entryIndex]);
                            }
                        }
                    }
                }
                entryIndex += 1;
            }
            playerIndex += 1;
        }
        if ((lbl_1_bss_0 == 2) && (lbl_1_data_1EC == - 1))
        {
            playerIndex = 0;
            while (playerIndex < lbl_1_bss_8)
            {
                effectAlpha = (s32)(192.0f + (63.0f * ((f32)(lbl_1_bss_14 % 30) / 30.0f)));
                entryIndex = 0;
                while (entryIndex < 0x66)
                {
                    if ((entryOpacity[playerIndex][entryIndex] != 0) && ((! drawEntries[playerIndex][entryIndex]) || (drawEntries[playerIndex][entryIndex]->category != 5)) && (lbl_1_data_168[playerIndex][(s32)(0.5f + lbl_1_bss_70A8[playerIndex][entryIndex].gridRow)] > 0xB))
                    {
                        PSMTXTrans(effectMatrix, entryX[playerIndex][entryIndex], - 27.0f + entryY[playerIndex][entryIndex], entryZ[playerIndex][entryIndex] + 2.0f);
                        PSMTXConcat(*mtx, effectMatrix, effectMatrix);
                        fn_8002472C();
                        if (drawPaused == 0)
                        {
                            fn_80024850(lbl_1_bss_158->mdlId[38], lbl_1_bss_E368[5][0], effectMatrix);
                        }
                    }
                    entryIndex += 1;
                }
                playerIndex += 1;
            }
        }
        GXSetZMode(1U, GX_LEQUAL, 1U);
        GXLoadPosMtxImm(*mtx, GX_PNMTX0);
        GXSetNumTevStages(1U);
        GXSetNumTexGens(1U);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0U, GX_PTIDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
        GXSetNumChans(1U);
        GXSetChanCtrl(GX_COLOR0A0, 0U, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXSetAlphaCompare(GX_ALWAYS, 0U, GX_AOP_AND, GX_ALWAYS, 0U);
        GXSetZMode(1U, GX_LEQUAL, 1U);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXSetNumTevStages(1U);
        GXSetNumTexGens(1U);
        GXClearVtxDesc();
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0U);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0U);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0U);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        playerIndex = 0;
        while (playerIndex < lbl_1_bss_8)
        {
            entryIndex = 0;
            while (entryIndex < 0xCC)
            {
                GXTexObj textureObject;
                M678TextureEffect *textureEntry;
                textureEntry = &lbl_1_bss_48CC[playerIndex].entries[entryIndex];
                if (((s32) textureEntry->textureIndex != 0) && ((s32) textureEntry->textureIndex <= 0x28))
                {
                    textureX = ((60.0f * textureEntry->x) + (f32)(lbl_1_bss_8 <= 1 ? 0 : playerIndex == 0 ? - 0x15E : 0x15E)) - 150.0f;
                    textureY = - 54.0f * (1.5f + ((0.5f + textureEntry->y) - 15.0f));
                    GXInitTexObj(& textureObject, lbl_1_bss_7C[textureEntry->textureIndex - 1], (u16) lbl_1_bss_60, (u16) lbl_1_bss_5C, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, 0U);
                    GXLoadTexObj(& textureObject, GX_TEXMAP0);
                    if (drawPaused == 0)
                    {
                        GXBegin(GX_QUADS, GX_VTXFMT0, 4U);
                        drawIndex = 0;
                        while (drawIndex < 4)
                        {
                    vertexY = textureY + lbl_1_bss_6C[cornerY[drawIndex]];
                    vertexX = textureX + lbl_1_bss_74[cornerX[drawIndex]];
                    GXPosition3f32(vertexX, vertexY, 0.0f);
                            GXColor4u8(255, 255, 255, (u8)(255.0f * lbl_1_data_198));
                            textureT = (f32) cornerY[drawIndex];
                            textureS = (f32) cornerX[drawIndex];
                            GXTexCoord2f32(textureS, textureT);
                            drawIndex += 1;
                        }
                    }
                }
                entryIndex += 1;
            }
            playerIndex += 1;
        }
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0U);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0U);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0U);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0U);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0U);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
        GXSetZMode(1U, GX_ALWAYS, 1U);
        GXSetZMode(1U, GX_LEQUAL, 1U);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0U);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0U);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0U);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0U);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0U);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0U);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        HuSprTexLoad(espanim[esprite[lbl_1_bss_144].animNo].anim, 0, 0, 0, 0, 1);
        playerIndex = 0;
        while (playerIndex < lbl_1_bss_8)
        {
            if (lbl_1_data_1D8[playerIndex] > 0)
            {
                s32 displayValue;
                displayValue = lbl_1_data_1E0[playerIndex] + 1;
                displayAlpha = (236.0f * ((f32) lbl_1_data_1D8[playerIndex] / 120.0f)) / 255.0f;
                displayLeft = (f32)(lbl_1_bss_8 <= 1 ? 0 : playerIndex == 0 ? - 0x15E : 0x15E) - 150.0f;
                displayRight = (f32)((lbl_1_bss_8 <= 1 ? 0 : playerIndex == 0 ? - 0x15E : 0x15E) + 0x168) - 150.0f;
                displayTop = 513.0f;
                displayBottom = - 81.0f;
                digitScale = displayValue >= 0xA ? 0.5f : 1.0f;
                drawIndex = 0;
                while (drawIndex < (s32)((displayValue >= 0xA) + 1))
                {
                    digit = digitScale == 1.0f ? displayValue : drawIndex == 0 ? (displayValue / 10) % 10 : displayValue % 10;
                    GXBegin(GX_QUADS, GX_VTXFMT0, 4U);
                    entryIndex = 0;
                    while (entryIndex < 4)
                    {
                        digitY = cornerY[entryIndex] == 0 ? displayTop : displayBottom;
                        digitX = ((f32)(lbl_1_bss_8 <= 1 ? 0 : playerIndex == 0 ? - 0x15E : 0x15E) + (60.0f * ((6.0f * (digitScale == 1.0f ? (f32)cornerX[entryIndex] : (((0.5f * (f32)drawIndex) + (0.7f * (f32)cornerX[entryIndex])) - 0.1f))) - 0.5f))) - 150.0f;
                        GXPosition3f32(digitX, digitY, 5.0f);
                        GXColor4u8((u8)(255.0f * displayAlpha), (u8)(232.0f * displayAlpha), (u8)(232.0f * displayAlpha), (u8)(232.0f * displayAlpha));
                    GXTexCoord2f32((f32)(((digit % 5) + cornerX[entryIndex]) * 0x50) / 400.0f, (128.0f * (0.5f + ((f32)(digit / 5) + (0.5f * (f32)(cornerY[entryIndex] != 0 ? 1 : - 1))))) / 256.0f);
                        entryIndex += 1;
                    }
                    drawIndex += 1;
                }
            }
            playerIndex += 1;
        }
    }
    DebugCheckEntries(&shakeX, 0);
}

char lbl_1_data_2B0[15] = "1P_ALL-kanban1";



HSF_OBJECT *lbl_1_bss_E368[8][5];
struct M678LinkEntry lbl_1_bss_70A8[2][0x66];
struct M678LinkEntry *lbl_1_bss_6D78[2][102];
f32 lbl_1_bss_6D70[2];
f32 lbl_1_bss_6D68[2];
f32 lbl_1_bss_6D60[2];
f32 lbl_1_bss_6D58[2];
f32 lbl_1_bss_6A28[2][102];
f32 lbl_1_bss_66F8[2][102];
u32 lbl_1_bss_66F0[2];
struct _struct_lbl_1_bss_66C0_0x18 lbl_1_bss_66C0[2];
struct _struct_lbl_1_bss_66A8_0xC lbl_1_bss_66A8[2];
struct _struct_lbl_1_bss_6690_0xC lbl_1_bss_6690[2];
struct _struct_lbl_1_bss_5E90_0x400 lbl_1_bss_5E90[2];
struct _struct_lbl_1_bss_5E80_0x8 lbl_1_bss_5E80[2];
struct _struct_lbl_1_bss_5E70_0x8 lbl_1_bss_5E70[2];
struct _struct_lbl_1_bss_5BF0_0x140 lbl_1_bss_5BF0[2];
u32 lbl_1_bss_5BEC;
struct M678TextureEffectStorage lbl_1_bss_48CC[2];
struct _struct_lbl_1_bss_45FC_0x24 lbl_1_bss_45FC[0x14];
struct _struct_lbl_1_bss_45FC_0x24 lbl_1_bss_45D8;
M678LinkGroupSet lbl_1_bss_2F88[2];
s32 lbl_1_bss_2F80[2];
struct M678LinkGroup lbl_1_bss_2E0[2][2][0x66];
struct _struct_lbl_1_bss_2D0_0x8 lbl_1_bss_2D0[2];
MGSCORE *lbl_1_bss_2B4[7];
s16 lbl_1_bss_2A4[8];
struct M678SoundEntry lbl_1_bss_174[0x26];
OMOBJ *lbl_1_bss_170;
OMOBJ *lbl_1_bss_16C;
OMOBJ *lbl_1_bss_15C[4];
OMOBJ *lbl_1_bss_158;
s32 lbl_1_bss_154;
HUPROCESS *lbl_1_bss_150;
s16 lbl_1_bss_14C;
s32 lbl_1_bss_148;
s32 lbl_1_bss_144;
s32 lbl_1_bss_140;
s32 lbl_1_bss_138[2];
s32 lbl_1_bss_130[2];
s32 lbl_1_bss_12C;
s32 lbl_1_bss_128;
s32 lbl_1_bss_124;
s32 lbl_1_bss_120;
s32 lbl_1_bss_11C;
void *lbl_1_bss_7C[0x28];
f32 lbl_1_bss_74[2];
f32 lbl_1_bss_6C[2];
s32 lbl_1_bss_68;
s32 lbl_1_bss_64;
s32 lbl_1_bss_60;
s32 lbl_1_bss_5C;
u8 lbl_1_bss_58[0x4]; /* Unnamed target interval, .bss 0x58..0x5C. */
s32 lbl_1_bss_54;
s32 lbl_1_bss_40[5];
u32 lbl_1_bss_3C;
s32 lbl_1_bss_38;
s32 lbl_1_bss_34;
s32 lbl_1_bss_30;
s32 lbl_1_bss_2C;
s32 lbl_1_bss_28;
u8 lbl_1_bss_24[0x4]; /* Unnamed target interval, .bss 0x24..0x28. */
s32 lbl_1_bss_20;
s32 lbl_1_bss_1C;
s32 lbl_1_bss_18;
s32 lbl_1_bss_14;
s32 lbl_1_bss_10;
u32 lbl_1_bss_C;
s32 lbl_1_bss_8;
s32 lbl_1_bss_4;
s32 lbl_1_bss_0;
