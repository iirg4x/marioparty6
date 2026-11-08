/* COM player difficulty tuning and AI: wander, wait, circle the goal model, then press A. */
#include "dolphin/types.h"
#include "REL/m663dll/models.h"

#include "math.h"

#include "dolphin/mtx.h"

#include "REL/m663dll/player.h"

#include "game/mg/actman.h"

#include "game/memory.h"

#include "REL/m663dll/actors.h"
#include "game/frand.h"

extern M663PlayerWork lbl_1_bss_4F0[2];

extern void fn_1_D2AC(M663PlayerWork *);

/* Sets each COM player's orbit radius, slow-chase chance and timing values from its
 * difficulty. */
void fn_1_C4F8(void) {
    f32 orbitRadii[4] = {550.0f, 450.0f, 400.0f, 350.0f};
    s16 slowChaseChance[4] = {60, 50, 30, 20};
    s16 actionFrames[4] = {60, 50, 30, 30};
    s16 startFrames[4] = {60, 50, 40, 30};
    struct FrameRange {
        s16 start;
        s16 end;
    } frameRanges[4] = {{420, 180}, {240, 120}, {60, 120}, {60, 60}};
    s16 player;

    for (player = 0; player < 2; player++) {
        if (lbl_1_bss_4F0[player].playerType != 0) {
            s16 difficulty = lbl_1_bss_4F0[player].difficulty;
            lbl_1_bss_4F0[player].orbitRadius = orbitRadii[difficulty];
            lbl_1_bss_4F0[player].slowChaseChance = slowChaseChance[difficulty];
            lbl_1_bss_4F0[player].actionFrames = actionFrames[difficulty];
            lbl_1_bss_4F0[player].startFrame = startFrames[difficulty];
            lbl_1_bss_4F0[player].endFrame = frameRanges[difficulty].start;
            lbl_1_bss_4F0[player].endFrame2 = frameRanges[difficulty].end;
        }
    }
}

/* Clears each COM player's stick and button values, then runs its AI sequence (after one
 * startup frame) when its input mode is 0 or 2. */
void fn_1_C744(void) {
    s16 player;
    M663PlayerWork *playerWork;

    for (player = 0; player < 2; player++) {
        playerWork = &lbl_1_bss_4F0[player];
        if (playerWork->playerType != 0 &&
            (playerWork->inputMode == 0 || playerWork->inputMode == 2)) {
            playerWork->resetValues[0] = 0;
            playerWork->resetValues[1] = 0;
            playerWork->resetValues[2] = 0;
            playerWork->resetValues[3] = 0;
            switch (playerWork->step) {
            case 0:
                playerWork->step += 1;
                break;
            case 1:
                fn_1_D2AC(playerWork);
                break;
            }
        }
    }
}

s32 fn_1_C820(M663PlayerWork *move, s16 advance)
{
    Vec target;
    s32 result = 0;
    s16 finished = 0;

    switch (move->communicationState) {
    case 0:
        fn_8007E818(lbl_1_bss_418.players[move->playerIndex]);
        move->communicationState++;
        break;
    case 1:
        target = move->target;
        finished = fn_8007E8B4(
            lbl_1_bss_418.players[move->playerIndex], &target,
            move->chaseSpeed, move->chaseRadius);
        if (finished != 0 || move->chaseTimer > 0x1E) {
            if (advance != 0) {
                move->communicationState++;
            } else {
                result = 1;
            }
            move->chaseTimer = 0;
        }
        break;
    case 2:
        fn_8007E888(lbl_1_bss_418.players[move->playerIndex]);
        move->communicationState = 0;
        result = 1;
        break;
    }
    return result;
}

extern f32 fn_1_46A8(f32 value);

extern s32 fn_1_C974(M663PlayerWork *work);

s32 fn_1_C974(M663PlayerWork *work)
{
    f32 result = 0.0f;
    Vec approachPosition;
    f32 orbitRadius;
    f32 chaseFinished = 0.0f;
    M663ModelWork *orbitPlayer;

    orbitPlayer = &lbl_1_bss_750[0x17];

    if (work->skipApproach != 0 && work->stopTimer != 0) {
        if (work->actionTimer-- < 0) {
            result = 1.0f;
        }
    } else {
        switch (work->state) {
        case 0:
            work->actionTimer = frandmod(work->actionFrames);
            work->callTimer = work->endFrame +
                                       frandmod(work->endFrame2);
            work->stopTimer = 0;
            work->state++;
            /* fall through */
        case 1: {
            s16 randomValue;
            f32 angle = fn_1_46A8(360.0f);

            orbitRadius = fn_1_46A8(work->orbitRadius);
            work->target.x = (f32)((f64)orbitPlayer->position.x +
                                    (f64)orbitRadius *
                                        sin((M_PI * angle) / 180.0));
            work->target.z = (f32)((f64)orbitPlayer->position.z +
                                    (f64)orbitRadius *
                                        cos((M_PI * angle) / 180.0));
            approachPosition.x = work->target.x;
            approachPosition.z = work->target.z;
            orbitRadius = (f32)sqrt((f64)(approachPosition.x * approachPosition.x +
                                          approachPosition.z * approachPosition.z));
            if (orbitRadius < 475.0f) {
                randomValue = frandmod(100);
                if (randomValue < work->slowChaseChance) {
                    work->chaseSpeed = 5.0f;
                    work->chaseRadius = 5.1f;
                } else {
                    work->chaseSpeed = 10.0f;
                    work->chaseRadius = 10.1f;
                }
                work->state++;
            }
            break;
        }
        case 2: {
            if (work->callTimer-- < 0) {
                work->stopTimer = 1;
            }
            chaseFinished = (f32)(s16)fn_1_C820(work, 1);
            if (chaseFinished) {
                work->state = 1;
            }
            break;
        }
        }
    }
    return result;
}

s32 fn_1_CEB4(M663PlayerWork *move)
{
    s32 result = 0;
    s32 phaseFinished = 0;
    switch (move->targetState)
    {
    case 0 :
        {
            f32 angle = fn_1_46A8(360.0f);
            f32 radius = fn_1_46A8(475.0f);
            Vec delta;
            move->target.x = radius * sin((M_PI * angle) / 180.0);
            move->target.z = radius * cos((M_PI * angle) / 180.0);
            delta.x = move->target.x - move->position.x;
            delta.z = move->target.z - move->position.z;
            radius = sqrt((f64)(delta.x * delta.x + delta.z * delta.z));
            if (radius > 150.0f)
            {
                move->chaseSpeed = 10.0f;
                move->chaseRadius = 10.1f;
                move->targetState++;
            }
            break;
        }
    case 1 :
        phaseFinished = fn_1_C820(move, 1);
        if ((s16)phaseFinished)
        {
            move->targetState = 0;
            result = 1;
        }
        break;
    }
    return result;
}

void fn_1_D2AC(M663PlayerWork *work)
{
    s32 result = 0;
    s16 state;
    if (lbl_1_bss_4F0[0].scoreStatus != 0 || lbl_1_bss_4F0[1].scoreStatus != 0)
    {
        work->sequenceState = 0;
    }
    switch (work->sequenceState)
    {
    case 0 :
        {
            state = frandmod(100);
            work->sequenceState = state < work->startFrame ? 1 : 3;
            if (work->sequenceState == 3)
            {
                work->state = 0;
            }
            if (work->sequenceState == 1)
            {
                state = frandmod(100);
                if (state < 50)
                {
                    work->sequenceState = 2;
                    work->waitTimer = frandmod(30) + 30;
                }
            }
            break;
        }
    case 1 :
        {
            result = fn_1_CEB4(work);
            if ((s16) result != 0)
            {
                work->sequenceState = 0;
                return;
            }
            break;
        }
    case 2 :
        {
            if (work->waitTimer-- < 0)
            {
                work->sequenceState = 0;
                return;
            }
            break;
        }
    case 3 :
        {
            result = fn_1_C974(work);
            if ((s16) result != 0)
            {
                work->sequenceState++;
                return;
            }
            break;
        }
    case 4 :
        if (work->skipApproach != 0)
        {
            work->input.buttonsDown |= PAD_BUTTON_A;
            return;
        }
        if (lbl_1_bss_4F0[0].score == 0 && lbl_1_bss_4F0[1].score == 0)
        {
            work->sequenceState = 0;
        }
        break;
    }
}
