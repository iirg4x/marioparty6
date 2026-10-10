// Chooses the computer's branch toward the star on Clockwork Castle.
#include "dolphin.h"
#include "dolphin/math.h"
#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/opening.h"
#include "game/board/player.h"
#include "game/board/window.h"
#include "game/wipe.h"
#include "messdir_enum.h"

int mbSNpcMasuGet(void);

#define W06_BRANCH_NEAREST_STEP_INITIAL_VALUE 10000

// The branch chooser calls this when a computer needs a route toward the star.
// It favors the closest route by day and the farthest route by night.
s32 fn_1_E3EC(s32 playerNo, s32 linkCount, const s16 *spaceIds)
{
    s16 stepsToNpc[12];
    s32 npcSpaceId;
    s32 minimumStep;
    s32 maximumStep;
    s32 minimumIndex;
    s32 maximumIndex;
    s32 index;

    npcSpaceId = (s16)mbSNpcMasuGet();
    // Measure every candidate branch space from the NPC's current space.
    for (index = 0; index < linkCount; index++) {
        stepsToNpc[index] = mbMasuFind_IdStepGet2(spaceIds[index], npcSpaceId, FALSE, TRUE);
    }

    if (GwSystem.curTime == FALSE) {
        minimumStep = W06_BRANCH_NEAREST_STEP_INITIAL_VALUE;
        minimumIndex = -1;
        for (index = 0; index < linkCount; index++) {
            if (minimumStep > stepsToNpc[index]) {
                minimumStep = stepsToNpc[index];
                minimumIndex = index;
            }
        }
        return minimumIndex;
    }

    maximumStep = -1;
    maximumIndex = -1;
    for (index = 0; index < linkCount; index++) {
        if (maximumStep < stepsToNpc[index]) {
            maximumStep = stepsToNpc[index];
            maximumIndex = index;
        }
    }
    return maximumIndex;
}
