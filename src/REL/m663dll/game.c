/* Main gameplay: player actions, end-of-game checks, goal placement, background models and
 * collisions. */
#include "REL/m663dll/transition.h"
#include "math.h"

#include "dolphin/gx.h"

#include "dolphin/types.h"

#include "dolphin/os.h"

#include "dolphin/mtx.h"

#include "string.h"

#include "REL/m663dll/tutorial.h"
#include "REL/m663dll/camera.h"

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

#include "REL/m663dll/score.h"

#include "REL/m663dll/input.h"

#include "REL/m663dll/player.h"

#include "REL/m663dll/models.h"

#include "game/flag.h"

#include "dolphin.h"

#include "humath.h"

#include "messdir_enum.h"

#include "game/hu3d.h"

#include "game/window.h"

#include "game/process.h"

#include "game/armem.h"

#include "game/mgdata.h"

#include "game/printfunc.h"

#include "game/saveload.h"

#include "game/esprite.h"

#include "game/thpmain.h"

#include "game/board/main.h"

#include "game/board/audio.h"

#include "game/board/masu.h"

#include "game/board/player.h"

#include "REL/m663dll/actors.h"

#include "REL/m663dll/motion.h"

extern s16 lbl_1_bss_15A8;

void fn_1_69E8(void);

void fn_1_987C(void);

s32 fn_1_2A00(void);

s16 fn_1_6A20(void);

void fn_1_3588(void);

/* Main phase: on the first frame enables both players, makes the score boxes translucent
 * and scales up the score icons; then runs the frame updates and returns nonzero once the
 * game ends. */
s32 fn_1_6804(void) {
    s16 result = 0;
    s16 index;
    if (lbl_1_bss_15A8 == 0) {
        lbl_1_bss_4F0[1].inputMode = lbl_1_bss_4F0[0].inputMode = 0;
        lbl_1_bss_4F0[1].enabled = lbl_1_bss_4F0[0].enabled = 1;

        lbl_1_bss_120[12].transparency = 0.8f;
        lbl_1_bss_120[13].transparency = 0.8f;
        MgScoreBoxTPLvlSet(lbl_1_bss_120[12].spriteId, lbl_1_bss_120[12].transparency);
        MgScoreBoxTPLvlSet(lbl_1_bss_120[13].spriteId, lbl_1_bss_120[13].transparency);

        lbl_1_bss_350[1].scaleY = lbl_1_bss_350[1].scaleX =
            lbl_1_bss_350[0].scaleY = lbl_1_bss_350[0].scaleX = 1.0f;
        HuSprGrpScaleSet(lbl_1_bss_350[0].groupId,
                         lbl_1_bss_350[0].scaleX, lbl_1_bss_350[0].scaleY);
        HuSprGrpScaleSet(lbl_1_bss_350[1].groupId,
                         lbl_1_bss_350[1].scaleX, lbl_1_bss_350[1].scaleY);

        for (index = 0; index < 5; index++) {
            lbl_1_bss_378[index].delayTimer = 0;
        }
        lbl_1_bss_15A8 = 1;
    }

    fn_1_69E8();
    fn_1_3588();
    fn_1_987C();
    fn_1_2A00();
    if (lbl_1_bss_15A8 != 0) {
        result = fn_1_6A20();
    }
    return result;
}

extern void fn_1_754(void);

extern void fn_1_C744(void);

extern void fn_1_6CB8(void);

extern void fn_1_7EFC(void);

extern void fn_1_900C(void);

extern void fn_1_91D8(void);

/* One gameplay frame: input, COM AI, player actions, background models, trails and score
 * icons. */
void fn_1_69E8(void) {
    fn_1_754();
    fn_1_C744();
    fn_1_6CB8();
    fn_1_7EFC();
    fn_1_900C();
    fn_1_91D8();
    fn_1_DC48();
}

void fn_1_3A68(MGTIMER *timer);

s32 fn_1_3A94(MGTIMER *timer);

/* Ends play when a player's progress reaches 3 (that player wins, with 10 bonus coins unless
 * flag 65551 is set), or declares a draw when the timer started at frame 0x3F48 runs out with
 * both players idle. Locks input and returns 1 when the game ends. */
s16 fn_1_6A20(void) {
    s16 finished = 0;
    s16 awardWinner = 0;
    s16 player;

    for (player = 0; player < 2; player++) {
        if (lbl_1_bss_4F0[player].progress >= 3) {
            if (lbl_1_bss_4F0[player].score == 0) {
                s32 coinAwardPlayer;

                lbl_1_bss_4F0[player].mode = 1;
                coinAwardPlayer = lbl_1_bss_4F0[player].playerNo;
                if (!fn_8006A124(65551)) {
                    lbl_80265750[coinAwardPlayer].mgCoinBonus = 10;
                }
                fn_80071C18((s16)lbl_1_bss_4F0[player].characterNo, -1, -1, -1);
                finished = 1;
            }

            lbl_1_bss_4F0[0].inputMode = 1;
            lbl_1_bss_4F0[1].inputMode = 1;
            lbl_1_bss_424.winnerReady = 1;
        }
    }

    {
        if (lbl_1_bss_424.frame++ == 0x3F48) {
        fn_1_3A68(lbl_1_bss_424.timer);
        }
        if (lbl_1_bss_424.frame > 0x3F49 &&
            fn_1_3A94(lbl_1_bss_424.timer) && lbl_1_bss_424.winnerReady == 0) {
            lbl_1_bss_4F0[0].inputMode = 1;
            lbl_1_bss_4F0[1].inputMode = 1;
            if (lbl_1_bss_4F0[0].score == 0 && lbl_1_bss_4F0[1].score == 0) {
                awardWinner = 1;
            }
        }
    }

    if (finished != 0) {
        fn_1_3ABC(lbl_1_bss_424.timer);
    }
    if (awardWinner != 0) {
        lbl_1_bss_424.winnerReady = 1;
        fn_80071C18(-1, -1, -1, -1);
        finished = 1;
    }
    if (finished != 0) {
        lbl_1_bss_4F0[0].inputMode = 1;
        lbl_1_bss_4F0[1].inputMode = 1;
    }
    return finished;
}

void fn_1_8B04(M663PlayerWork *character);

void fn_1_8528(s16 mode);

s32 fn_1_95D0(M663PlayerWork *);

/* Per-player action states: near the goal model, A starts a grab; a successful grab moves the
 * goal to the player, plays sounds and motions, raises progress and respawns the goal. Idle
 * players near the goal rumble, and after the game ends each walks back and turns forward. */
void fn_1_6CB8(void) {
    Vec distanceDelta;
    f32 distance;
    f32 travelStep;
    f32 step;
    s32 result = 0;
    s16 selectedPlayer;
    s16 modelIndex;
    s16 playerIndex;
    M663PlayerWork *playerState;
    M663ModelWork *model;
    M663ModelWork *goalModel;

    goalModel = &lbl_1_bss_750[0x17];
    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        playerState = &lbl_1_bss_4F0[playerIndex];
        fn_1_584C(playerState, 0);
        distanceDelta.x = goalModel->position.x - playerState->position.x;
        distanceDelta.z = goalModel->position.z - playerState->position.z;
        distance = sqrt((f64)(distanceDelta.x * distanceDelta.x +
                              distanceDelta.z * distanceDelta.z));
        playerState->skipApproach = 0;
        if (distance <= 75.0f) {
            playerState->skipApproach = 1;
        }
        fn_1_8B04(playerState);
        switch (playerState->score) {
            case 0:
                if (lbl_1_bss_424.winnerReady != 0) {
                    playerState->score = 6;
                } else if (((s32) (playerState->input.buttonsDown & 0x100) != 0) && (playerState->inputMode == 0)) {
                    if (playerState->skipApproach != 0) {
                        if (lbl_1_bss_4F0[playerIndex ^ 1].scoreStatus == 0) {
                            playerState->scoreStatus = 1;
                        } else if (lbl_1_bss_4F0[playerIndex ^ 1].carrying == 0) {
                            lbl_1_bss_4F0[playerIndex ^ 1].scoreStatus = 0;
                            selectedPlayer = frandmod(2);
                            lbl_1_bss_4F0[selectedPlayer].scoreStatus = 1;
                        }
                    }
                    playerState->inputMode = 1;
                    playerState->transformMode = 0;
                    fn_80086E3C(lbl_1_bss_418.players[playerIndex]);
                    lbl_1_bss_688[playerIndex].shiftState = 1;
                    lbl_1_bss_688[playerIndex].shiftMotionIndex = 6;
                    lbl_1_bss_688[playerIndex].shiftStartTime = 0.0f;
                    lbl_1_bss_688[playerIndex].shiftDuration = 5.0f;
                    lbl_1_bss_688[playerIndex].shiftAttributes = 0;
                    lbl_1_bss_688[playerIndex].attributesToClear = ((1 << 30) | (1 << 0));
                    playerState->stateTimer = 0;
                    playerState->score++;
                } else {
                    playerState->carrying = 0;
                    if (lbl_1_bss_424.rumbleEnabled != 0) {
                        if ((playerState->skipApproach != 0) && (playerState->rumbleFrames < 0xC) && (lbl_1_bss_4F0[playerIndex ^ 1].scoreStatus == 0)) {
                            fn_8006A030(playerState->playerNo, 0x14, 7, 3);
                            playerState->rumbleFrames++;
                        } else {
                            if (playerState->rumbleFrames++ > 0x2A) {
                                playerState->rumbleFrames = 0;
                            }
                        }
                    }
                }
                break;
            case 1:
                playerState->previousPosition = playerState->position;
                playerState->carrying = 1;
                if (lbl_1_bss_688[playerIndex].motionTime >= 28.9f) {
                    if (playerState->scoreStatus != 0) {
                        for (modelIndex = 0x17; modelIndex <= 0x19; modelIndex++) {
                            fn_80026A44(lbl_1_bss_750[modelIndex].modelId, (1 << 0));
                            fn_8002D86C(lbl_1_bss_750[modelIndex].modelId, 1.0f);
                            fn_8002D414(lbl_1_bss_750[modelIndex].modelId, 0.0f);
                            lbl_1_bss_750[modelIndex].position = lbl_1_bss_750[0x17].position;
                            lbl_1_bss_750[modelIndex].scale = 1.0f;
                        }
    fn_1_6018(2, 0, &playerState->position);
                        lbl_1_bss_688[playerIndex].shiftState = 1;
                        lbl_1_bss_688[playerIndex].shiftMotionIndex = 7;
                        lbl_1_bss_688[playerIndex].shiftStartTime = 0.0f;
                        lbl_1_bss_688[playerIndex].shiftDuration = 10.0f;
                        lbl_1_bss_688[playerIndex].shiftAttributes = 0;
                        lbl_1_bss_688[playerIndex].attributesToClear = ((1 << 30) | (1 << 0));
                        playerState->requestPending = 1;
                        playerState->score++;
                    } else {
                        lbl_1_bss_688[playerIndex].shiftState = 1;
                        lbl_1_bss_688[playerIndex].shiftMotionIndex = 0;
                        lbl_1_bss_688[playerIndex].shiftStartTime = 0.0f;
                        lbl_1_bss_688[playerIndex].shiftDuration = 5.0f;
                        lbl_1_bss_688[playerIndex].shiftAttributes = ((1 << 30) | (1 << 0));
                        lbl_1_bss_688[playerIndex].attributesToClear = 0;
                        playerState->effectFrame = 0.0f;
                        playerState->score = 3;
                    }
                } else if ((lbl_1_bss_688[playerIndex].motionTime >= 14.9f) && (playerState->scoreStatus != 0)) {
                    Hu3DModelObjPosGet(lbl_1_bss_750[playerIndex].modelId, playerState->objectName, &lbl_1_bss_750[0x17].position);
                    goalModel->position.y += 25.0f;
                }
                break;
            case 2:
                if ((s32) playerState->stateTimer == 0x14) {
    fn_1_6018(3, 0, &playerState->position);
                }
                if (playerState->stateTimer++ > 0x5A) {
                    lbl_1_bss_4F0[playerIndex ^ 1].inputMode = 1;
                    lbl_1_bss_4F0[playerIndex ^ 1].transformMode = 0;
                    fn_80086E3C(lbl_1_bss_418.players[playerIndex ^ 1]);
                    lbl_1_bss_688[playerIndex ^ 1].shiftState = 1;
                    lbl_1_bss_688[playerIndex ^ 1].shiftMotionIndex = 5;
                    lbl_1_bss_688[playerIndex ^ 1].shiftStartTime = 0.0f;
                    lbl_1_bss_688[playerIndex ^ 1].shiftDuration = 5.0f;
                    lbl_1_bss_688[playerIndex ^ 1].shiftAttributes = 0;
                    lbl_1_bss_688[playerIndex ^ 1].attributesToClear = ((1 << 30) | (1 << 0));
                    lbl_1_bss_4F0[playerIndex ^ 1].stateTimer = 2;
                    lbl_1_bss_4F0[playerIndex ^ 1].effectFrame = 58.9f;
                    playerState->effectPending = 0;
                    lbl_1_bss_4F0[playerIndex ^ 1].score = 3;
                    lbl_1_bss_688[playerIndex].shiftState = 1;
                    lbl_1_bss_688[playerIndex].shiftMotionIndex = 4;
                    lbl_1_bss_688[playerIndex].shiftStartTime = 0.0f;
                    lbl_1_bss_688[playerIndex].shiftDuration = 5.0f;
                    lbl_1_bss_688[playerIndex].shiftAttributes = 0;
                    lbl_1_bss_688[playerIndex].attributesToClear = ((1 << 30) | (1 << 0));
                    playerState->stateTimer = 2;
                    playerState->effectFrame = 58.9f;
                    playerState->progress++;
                    playerState->effectPending = 1;
                    playerState->score++;
                    for (modelIndex = 0x17; modelIndex <= 0x19; modelIndex++) {
                        if (modelIndex != 0x17) {
                            fn_800269E8(lbl_1_bss_750[modelIndex].modelId, (1 << 0));
                        }
                        fn_8002D86C(lbl_1_bss_750[modelIndex].modelId, 0.0f);
                        fn_8002D414(lbl_1_bss_750[modelIndex].modelId, 0.0f);
                        lbl_1_bss_750[modelIndex].scale = 1.0f;
                    }
                    if (playerState->progress >= 3) {
    fn_1_8528(1);
                    } else {
    fn_1_8528(0);
                    }
                }
                break;
            case 3:
                playerState->previousPosition = playerState->position;
                if (playerState->stateTimer-- < 0) {
                    if ((playerState->effectPending != 0) && (lbl_1_bss_688[playerIndex].motionTime >= 40.0f)) {
                        playerState->effectPending = 0;
                        lbl_1_bss_750[0x4E].animationState = 1;
                        lbl_1_bss_750[0x4E].position = playerState->position;
                        lbl_1_bss_750[0x4E].position.y = 50.0f;
                    }
                    if (lbl_1_bss_688[playerIndex].motionTime >= playerState->effectFrame) {
                        lbl_1_bss_688[playerIndex].shiftState = 1;
                        lbl_1_bss_688[playerIndex].shiftMotionIndex = 0;
                        lbl_1_bss_688[playerIndex].shiftStartTime = 0.0f;
                        lbl_1_bss_688[playerIndex].shiftDuration = 5.0f;
                        lbl_1_bss_688[playerIndex].shiftAttributes = ((1 << 30) | (1 << 0));
                        lbl_1_bss_688[playerIndex].attributesToClear = 0;
                        playerState->score++;
                    }
                }
                break;
            case 4:
                if (lbl_1_bss_688[playerIndex].motionTime >= 0.0f) {
                    playerState->transformMode = 1;
                    fn_80086F00(lbl_1_bss_418.players[playerIndex], &playerState->position);
                    playerState->score++;
                }
                break;
            case 5:
                playerState->rumbleFrames = 0;
                playerState->inputMode = 0;
                playerState->score = 0;
                if (lbl_1_bss_424.winnerReady != 0) {
                    playerState->inputMode = 1;
                }
                break;
            case 6:
                playerState->inputMode = 1;
                break;
            case 7:
                playerState->inputMode = 1;
                playerState->transformMode = 0;
                fn_80086E3C(lbl_1_bss_418.players[playerIndex]);
                playerState->target.x = -150.0f + (300.0f * (f32) playerIndex);
                playerState->target.y = 0.0f;
                playerState->target.z = -100.0f;
                playerState->chaseRadius = 10.1f;
                playerState->chaseSpeed = 10.0f;
                playerState->stateTimer = 0;
                if ((s16)(result = fn_1_95D0(playerState)) == 0) {
                    lbl_1_bss_688[playerIndex].shiftState = 1;
                    lbl_1_bss_688[playerIndex].shiftMotionIndex = 1;
                    lbl_1_bss_688[playerIndex].shiftStartTime = 0.0f;
                    lbl_1_bss_688[playerIndex].shiftDuration = 5.0f;
                    lbl_1_bss_688[playerIndex].shiftAttributes = ((1 << 30) | (1 << 0));
                    lbl_1_bss_688[playerIndex].attributesToClear = 0;
                }
                playerState->score++;
                break;
            case 8:
                if (playerState->chaseTimer > 0x1E) {
                    if (playerIndex == 0) {
                        step = 5.0f;
                    } else {
                        step = -5.0f;
                    }
                    playerState->target.z += step;
                    playerState->stateTimer = 0x14;
                } else {
                    if ((--playerState->stateTimer) <= 0) {
                        playerState->target.x = -150.0f + (300.0f * (f32) playerIndex);
                        playerState->target.z = -100.0f;
                    }
                }
                if (((s16)(result = fn_1_95D0(playerState)) == 0) || playerState->stateTimer > 0) {
                    break;
                }
                {
                    playerState->lastAngle = playerState->rotation.y;
                    playerState->score++;
                }
            case 9:
                    travelStep = 30.0f;
                    if ((s16)(result = fn_1_38D4(&playerState->rotation.y, travelStep, playerState->lastAngle, 0.0f)) == 0) {
                        lbl_1_bss_688[playerIndex].shiftState = 1;
                        lbl_1_bss_688[playerIndex].shiftMotionIndex = 0;
                        lbl_1_bss_688[playerIndex].shiftStartTime = 0.0f;
                        lbl_1_bss_688[playerIndex].shiftDuration = 5.0f;
                        lbl_1_bss_688[playerIndex].shiftAttributes = ((1 << 30) | (1 << 0));
                        lbl_1_bss_688[playerIndex].attributesToClear = 0;
                        playerState->score++;
                    }
                break;
            case 10:
                playerState->previousPosition = playerState->position;
                break;
            }
    }
}

void fn_1_7EFC(void)
{
    Mtx objectMatrix;
    s32 remaining = 0;
    f32 angleValues[5][2] = {
        {0.0f, 60.0f}, {-10.0f, 50.0f}, {-30.0f, 30.0f},
        {-50.0f, 10.0f}, {-60.0f, 0.0f}
    };
    Vec difference;
    s16 index;
    s16 finishedCount;
    f32 angleStep;

    index = 0;
    finishedCount = 0;
    for (; index < 5; index++) {
        M663TutorialWork *work = &lbl_1_bss_378[index];

        switch (work->phase) {
        case 0:
            if (work->firstMotion == 0) {
                work->motionTimer = frandmod(30);
            } else {
                work->motionTimer = frandmod(60);
            }
            work->delayTimer = 10000;
            work->phase++;
            break;
        case 1: {
            if (work->motionTimer-- < 0) {
                fn_800269E8(lbl_1_bss_750[work->motionSlot].modelId, ((1 << 30) | (1 << 0)));
                fn_8002D86C(lbl_1_bss_750[work->motionSlot].modelId, 1.0f);
                fn_8002D414(lbl_1_bss_750[work->motionSlot].modelId, 0.0f);
                if (work->firstMotion == 0) {
                    work->firstMotion = 1;
                    work->phase++;
                } else {
                    work->phase = 4;
                }
            }
            break;
        }
        case 2: {
            work->startAngle = lbl_1_bss_750[work->motionSlot].rotation.y;
            work->targetAngle = angleValues[index][work->angleIndex];
            work->angleIndex ^= 1;
            work->phase++;
            /* fall through */
        }
        case 3: {
            angleStep = 60.0f;
            remaining = fn_1_38D4(&lbl_1_bss_750[work->motionSlot].rotation.y, angleStep,
                                  work->startAngle, work->targetAngle);
            if (work->delayTimer-- < 0) {
                work->phase++;
            } else if ((s16)remaining == 0) {
                work->phase = 2;
            }
            break;
        }
        case 4: {
            difference.x = lbl_1_bss_750[0x17].position.x - lbl_1_bss_750[work->motionSlot].position.x;
            difference.z = lbl_1_bss_750[0x17].position.z - lbl_1_bss_750[work->motionSlot].position.z;
            work->startAngle = lbl_1_bss_750[work->motionSlot].rotation.y;
            work->targetAngle = (f32)(180.0 *
                (atan2((f64)difference.x, (f64)difference.z) / M_PI));
            work->phase++;
            /* fall through */
        }
        case 5:
            angleStep = 60.0f;
            remaining = fn_1_38D4(&lbl_1_bss_750[work->motionSlot].rotation.y, angleStep,
                                  work->startAngle, work->targetAngle);
            if ((s16)remaining == 0) {
                fn_80026A44(lbl_1_bss_750[work->motionSlot].modelId, ((1 << 30) | (1 << 0)));
                work->phase++;
            }
            break;
        case 6:
            finishedCount++;
            break;
        case 7:
            fn_800269E8(lbl_1_bss_750[work->motionSlot].modelId, ((1 << 30) | (1 << 0)));
            fn_8002D86C(lbl_1_bss_750[work->motionSlot].modelId, 1.0f);
            fn_8002D414(lbl_1_bss_750[work->motionSlot].modelId, 0.0f);
            work->startAngle = lbl_1_bss_750[work->motionSlot].rotation.y;
            work->targetAngle = 0.0f;
            work->phase++;
            /* fall through */
        case 8:
            angleStep = 60.0f;
            remaining = fn_1_38D4(&lbl_1_bss_750[work->motionSlot].rotation.y, angleStep,
                                  work->startAngle, work->targetAngle);
            if ((s16)remaining == 0) {
                fn_80026A44(lbl_1_bss_750[work->motionSlot].modelId, ((1 << 30) | (1 << 0)));
                work->phase++;
            }
            break;
        case 9:
            finishedCount++;
            break;
        }

        {
            if (work->updateTimer-- < 0) {
                fn_8002D86C(lbl_1_bss_750[work->actorSlot].modelId, 1.0f);
                fn_8002D414(lbl_1_bss_750[work->actorSlot].modelId, 0.0f);
                work->updateTimer = frandmod(250) + 50;
                fn_1_6018(1, 0, &lbl_1_bss_750[work->motionSlot].position);
            }
        }

        {
            HU3D_MODEL *actorModel = &lbl_802C01C8[lbl_1_bss_750[work->actorSlot].modelId];

            fn_80023398(lbl_1_bss_750[work->motionSlot].modelId, work->objectName, objectMatrix);
            PSMTXCopy(objectMatrix, actorModel->mtx);
        }
    }

    if (finishedCount >= 5) {
        M663SoundStopCopy(0);
    }
}

extern f32 fn_1_46A8(f32 value);

void fn_1_8528(s16 mode)
{
    Vec distanceDelta;
    Vec spawnPosition = { 0.0f, -10.0f, 0.0f };
    s16 index;
    M663ModelWork *character = &lbl_1_bss_750[0x17];

    if (mode == 0) {
        s16 positionClear;

        do {
            f32 radius;
            f32 angle;
            f32 distanceToModel;

            positionClear = 1;
            distanceToModel = fn_1_46A8(425.0f);
            angle = fn_1_46A8(900.0f) / 10.0f;
            angle += 90.0f * fn_1_46A8(4.0f);
            spawnPosition.x = (f32)((f64)distanceToModel * sin((M_PI * (f64)angle) / 180.0));
            spawnPosition.z = (f32)((f64)distanceToModel * cos((M_PI * (f64)angle) / 180.0));
            distanceDelta.x = character->position.x - spawnPosition.x;
            distanceDelta.z = character->position.z - spawnPosition.z;
            radius = sqrt((f64)(distanceDelta.x * distanceDelta.x +
                                distanceDelta.z * distanceDelta.z));
            if (radius <= 300.0f) {
                positionClear = 0;
            }

            for (index = 0; index < 2; index++) {
                distanceDelta.x = lbl_1_bss_4F0[index].position.x - spawnPosition.x;
                distanceDelta.z = lbl_1_bss_4F0[index].position.z - spawnPosition.z;
                radius = sqrt((f64)(distanceDelta.x * distanceDelta.x +
                                    distanceDelta.z * distanceDelta.z));
                if (radius <= 300.0f) {
                    positionClear = 0;
                }
            }
        } while (positionClear == 0);

        character->position = spawnPosition;
        lbl_1_bss_4F0[0].scoreStatus = 0;
        lbl_1_bss_4F0[1].scoreStatus = 0;
        for (index = 0; index < 5; index++) {
            lbl_1_bss_378[index].phase = 0;
        }
        fn_1_6018(0, 5, NULL);
        return;
    }

    for (index = 0; index < 5; index++) {
        lbl_1_bss_378[index].phase = 7;
    }
    fn_1_6018(0, 5, NULL);
    character->position.x = -10000.0f;
}

void fn_1_8B04(M663PlayerWork *character)
{
    struct {
        f32 x;
        f32 y;
        f32 z;
    } delta;
    f32 distance;
    f32 angle;
    f32 motionTime;

    delta.x = character->position.x - character->previousPosition.x;
    delta.z = character->position.z - character->previousPosition.z;
    distance = sqrt((f64)(delta.x * delta.x + delta.z * delta.z));
    angle = (f32)(180.0 *
                  (atan2((f64)delta.x, (f64)delta.z) / M_PI));

    if (distance > 1.5f) {
        fn_80026A44(lbl_1_bss_750[(s8)character->playerIndex + 0x1A].modelId, (1 << 0));
        fn_8002D86C(lbl_1_bss_750[(s8)character->playerIndex + 0x1A].modelId,
                           1.0f);
        lbl_1_bss_750[(s8)character->playerIndex + 0x1A].position = character->position;
        lbl_1_bss_750[(s8)character->playerIndex + 0x1A].position.y = 50.0f;
        lbl_1_bss_750[(s8)character->playerIndex + 0x1A].rotation.y =
            180.0f + angle;
    } else {
        motionTime = fn_8002D4E0(
            lbl_1_bss_750[(s8)character->playerIndex + 0x1A].modelId);
        if (motionTime >= 2.0f) {
        fn_80026A44(lbl_1_bss_750[(s8)character->playerIndex + 0x1A].modelId, (1 << 0));
        fn_8002D86C(lbl_1_bss_750[(s8)character->playerIndex + 0x1A].modelId,
                           -1.0f);
        } else {
            fn_800269E8(lbl_1_bss_750[(s8)character->playerIndex + 0x1A].modelId, (1 << 0));
            fn_8002D86C(lbl_1_bss_750[(s8)character->playerIndex + 0x1A].modelId,
                               0.0f);
            fn_8002D414(lbl_1_bss_750[(s8)character->playerIndex + 0x1A].modelId,
                              0.0f);
        }
    }

    if (distance > 2.0f) {
        s16 index;

        character->motionCount++;
        if ((character->motionCount & 1U) == 0) {
            character->motionCount = 0;
            for (index = 0x1C; index <= 0x4D; index++) {
                M663ModelWork *candidate = &lbl_1_bss_750[index];

                if (candidate->animationState == 0) {
                    candidate->position = character->position;
                    candidate->position.y = 50.0f;
                    candidate->rotation.y = 180.0f + angle;
                    candidate->animationState = 1;
                    return;
                }
            }
        }
    }
}

void fn_1_900C(void)
{
    s16 objectIndex;
    M663ModelWork *object;

    for (objectIndex = 0x1C; objectIndex <= 0x4D; objectIndex++) {
        object = &lbl_1_bss_750[objectIndex];
        switch (object->animationState) {
        case 0:
            break;
        case 1:
            fn_80026A44(object->modelId, (1 << 0));
            fn_8002CF70(object->modelId, 1.1f);
            fn_8002CF98(object->modelId, 0.0f);
            object->animationState++;
            /* fall through */
        case 2:
            if (fn_8002CFE4(object->modelId) - 2.0f <=
                lbl_802C01C8[object->modelId].motShapeWork.time) {
                fn_800269E8(object->modelId, (1 << 0));
                fn_8002CF70(object->modelId, 0.0f);
                object->animationState = 0;
            }
            break;
        default:
            break;
        }
    }

    object = &lbl_1_bss_750[0x4E];
    switch (object->animationState) {
    case 0:
        break;
    case 1:
        fn_80026A44(object->modelId, (1 << 0));
        fn_8002D86C(object->modelId, 1.1f);
        fn_8002D414(object->modelId, 0.0f);
        object->animationState++;
        /* fall through */
    case 2:
        if (fn_8002D75C(object->modelId)) {
            fn_800269E8(object->modelId, (1 << 0));
            fn_8002D86C(object->modelId, 0.0f);
            object->animationState = 0;
        }
        break;
    default:
        break;
    }
}

static inline void DebugValue(f32 value) { }

void fn_1_91D8(void)
{
    s16 index;
    s32 preset;
    s16 modelIndex;
    f32 progress;
    s16 presetHeader[2];

    presetHeader[1] = 0;
    preset = 0;
    presetHeader[0] = 0;
    {
    f32 presets[6][5][6] = {
    {
        {300.0f, 170.0f, 9.0f, 0.0f, 0.9f, 0.0f},
        {300.0f, 9.0f, -149.0f, 0.9f, 0.0f, 0.0f},
        {300.0f, -149.0f, 9.0f, 0.0f, 0.9f, 0.0f},
        {300.0f, 9.0f, 169.0f, 0.9f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
    },
    {
        {300.0f, 62.0f, 219.0f, 0.0f, 0.9f, 0.0f},
        {300.0f, 219.0f, 378.0f, 0.9f, 0.0f, 0.0f},
        {300.0f, 378.0f, 221.0f, 0.0f, 0.9f, 0.0f},
        {300.0f, 221.0f, 63.0f, 0.9f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
    },
    {
        {300.0f, 1184.0f, 585.0f, 0.5f, 0.9f, 0.0f},
        {300.0f, 585.0f, -20.0f, 0.9f, 0.5f, 0.0f},
        {300.0f, -20.0f, -618.0f, 0.5f, 0.9f, 0.0f},
        {300.0f, -618.0f, -1219.0f, 0.9f, 0.5f, 0.0f},
        {-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
    },
    {
        {300.0f, 19.0f, -695.0f, 0.5f, 0.9f, 0.0f},
        {300.0f, -695.0f, -1404.0f, 0.9f, 0.5f, 0.0f},
        {300.0f, 1447.0f, 733.0f, 0.5f, 0.9f, 0.0f},
        {300.0f, 733.0f, 24.0f, 0.9f, 0.5f, 0.0f},
        {-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
    },
    {
        {300.0f, -1817.0f, -1111.0f, 0.9f, 0.4f, 0.0f},
        {300.0f, -1111.0f, -401.0f, 0.4f, 0.9f, 0.0f},
        {300.0f, -401.0f, 306.0f, 0.9f, 0.4f, 0.0f},
        {300.0f, 306.0f, 1012.0f, 0.4f, 0.9f, 0.0f},
        {-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
    },
    {
        {300.0f, 136.0f, 945.0f, 0.9f, 0.4f, 0.0f},
        {300.0f, 945.0f, 1754.0f, 0.4f, 0.9f, 0.0f},
        {300.0f, -1478.0f, -672.0f, 0.9f, 0.4f, 0.0f},
        {300.0f, -672.0f, 131.0f, 0.4f, 0.9f, 0.0f},
        {-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
    }
    };

    for (modelIndex = 0x50, index = 0; modelIndex <= 0x55; modelIndex++, index++) {
        M663ModelWork *model = &lbl_1_bss_750[modelIndex];
        M663TransitionControl *control = &lbl_1_bss_0[index];

        switch (control->mode) {
        case 0:
            if (presets[control->playerIndex][control->presetIndex][0] < 0.0f) {
                control->presetIndex = 0;
                lbl_1_bss_424.transitionCount = 0;
            }
            progress = presets[control->playerIndex][control->presetIndex][0];
            DebugValue(progress);
            progress = 300.0f;
            control->positionX = progress;
            control->positionY = presets[control->playerIndex][control->presetIndex][1];
            control->positionZ = presets[control->playerIndex][control->presetIndex][2];
            control->targetY = presets[control->playerIndex][control->presetIndex][3];
            control->targetZ = presets[control->playerIndex][control->presetIndex][4];
            control->progress = presets[control->playerIndex][control->presetIndex][3];
            model->position.x = presets[control->playerIndex][control->presetIndex][1];
            control->mode++;
            /* fall through */
        case 1:
            preset = fn_1_38D4(&model->position.x, control->positionX,
                               control->positionY, control->positionZ);
            if (control->state == 0) {
                preset += fn_1_38D4(&control->progress, control->positionX,
                                   control->targetY, control->targetZ);
                if ((s16)preset == 0) {
                    control->presetIndex++;
                    control->mode = 0;
                }
            } else {
            if ((control->progress -= 0.05f) < 0.0f) {
                    control->progress = 0.0f;
                    control->mode++;
                }
            }
            break;
        case 2:
            break;
        default:
            break;
        }
        fn_80026CBC(model->modelId, control->progress);
    }
    }
    lbl_1_bss_424.transitionCount++;
}

extern void printf(const char *format);

s32 fn_1_94E0(COL_NARROW_PARAM *first, COL_NARROW_PARAM *request)
{
    switch (request->type) {
    case 0:
        lbl_1_bss_4F0[first->paramB].pairedIndex = (s16)request->paramB;
        lbl_1_bss_4F0[request->paramB].pairedIndex = (s16)first->paramB;
        lbl_1_bss_4F0[first->paramB].chaseTimer += 1;
        printf("kiteruyo\n");
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    default:
        break;
    }
    return 0;
}

void fn_1_959C(MGACTOR *slot)
{
    s32 playerIndex = slot->param;
    lbl_1_bss_4F0[playerIndex].motionIndex = (s16)slot->colMesh;
}

/* Moves the player one chase-speed step toward its target in the XZ plane and faces it;
 * returns 1 once it is within the chase radius. */
s32 fn_1_95D0(M663PlayerWork *motion) {
    s32 reached = 0;
    f32 distance;
    struct {
        f32 x;
        f32 y;
        f32 z;
    } delta;

    delta.x = motion->target.x - motion->position.x;
    delta.z = motion->target.z - motion->position.z;
    distance = sqrt((f64)(delta.x * delta.x + delta.z * delta.z));

    if (distance > motion->chaseRadius) {
        f32 angle = 180.0 *
                    (atan2((f64)delta.x, (f64)delta.z) / M_PI);
        motion->position.x += motion->chaseSpeed *
                           sin((M_PI * (f64)angle) / 180.0);
        motion->position.z += motion->chaseSpeed *
                           cos((M_PI * (f64)angle) / 180.0);
        motion->rotation.y = angle;
    } else {
        reached = 1;
    }
    return reached;
}

void fn_1_987C(void)
{
    f32 distance;
    f32 scale;
    s16 collided = 0;
    s16 playerIndex;
    M663PlayerWork *player;

    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        Vec position;

        player = &lbl_1_bss_4F0[playerIndex];

        if (player->enabled != 0) {
            if ((lbl_1_bss_4F0[playerIndex ^ 1].score >= 1 && lbl_1_bss_424.winnerReady == 0) ||
                lbl_1_bss_4F0[playerIndex ^ 1].score >= 9 ||
                (lbl_1_bss_4F0[playerIndex ^ 1].score == 8 && player->score == 8)) {

            position.x = player->position.x - lbl_1_bss_4F0[playerIndex ^ 1].position.x;
            position.z = player->position.z - lbl_1_bss_4F0[playerIndex ^ 1].position.z;
            if ((distance = sqrt((f64)(position.x * position.x +
                                       position.z * position.z))) <= 100.0f) {
                distance = (0.0f == distance) ? 1.0f : distance;
                scale = 100.0f / distance;
                position.x *= scale;
                position.z *= scale;
                player->position.x = position.x + lbl_1_bss_4F0[playerIndex ^ 1].position.x;
                player->position.z = position.z + lbl_1_bss_4F0[playerIndex ^ 1].position.z;
                if (lbl_1_bss_4F0[playerIndex ^ 1].score >= 8 &&
                    player->score >= 8) {
                    player->chaseTimer++;
                }
                collided = 1;
            }
            }

            {

                position.x = player->position.x;
                position.z = player->position.z;
                if ((distance = sqrt((f64)(position.x * position.x +
                                           position.z * position.z))) >= 475.0f) {

                    distance = (0.0f == distance) ? 1.0f : distance;
                    scale = 475.0f / distance;
                    position.x *= scale;
                    position.z *= scale;
                    player->position.x = position.x;
                    player->position.z = position.z;
                }
            }
        }
    }

    if (lbl_1_bss_4F0[playerIndex ^ 1].score >= 8 &&
        player->score >= 8 && collided == 0) {
        lbl_1_bss_4F0[0].chaseTimer = 0;
        lbl_1_bss_4F0[1].chaseTimer = 0;
    }
}
