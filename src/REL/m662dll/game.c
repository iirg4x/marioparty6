/* Player selection, puzzle motion and the main Trick or Tree round. */
#include "dolphin/math.h"
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
#include "REL/m662dll/game.h"

extern s16 lbl_1_bss_C58[4];
void fn_1_984(void), fn_1_AD68(void), fn_1_2670(void);
void fn_1_5A5C(void), fn_1_7DE8(void), fn_1_5884(void), fn_1_887C(void);
s32 fn_1_58B0(void), fn_1_8754(void);
void fn_1_48E8(M662Player *, s16);
s16 fn_1_2DA0(f32 *, f32, f32, f32);
void *fn_1_2EC0(s32, s32, s32, f32, f32);
void fn_1_2F34(void *), fn_1_2F88(void *);
s32 fn_1_2F60(MGTIMER *);
void fn_1_5008(s16, s16, void *);

/* World-space choice and approach points for the four tree slots. */
typedef struct SlotPositions_5A5C { HuVecF position[4]; } SlotPositions_5A5C;
/* The two players begin the choice approach on opposite sides of center. */
typedef struct StartPositions_5A5C { HuVecF position[2]; } StartPositions_5A5C;
/* Alternating horizontal offsets used while climbing a tree. */
typedef struct JumpOffsets_5A5C { f32 offset[2]; } JumpOffsets_5A5C;

/* When a positional effect is updated, changes its pan only while its sound slot and handle are
 * active. */
void fn_1_5468(s16 soundSlot, s16 pan) {
    if (lbl_1_bss_210.soundState[soundSlot] == 1) {
        if (HuAudFXStatusGet(lbl_1_bss_210.soundHandle[soundSlot]) == 2) {
            HuAudFXPanning(lbl_1_bss_210.soundHandle[soundSlot], pan);
        }
    }
}

/* Sound-slot helper: changes pitch only while the selected effect handle is active. */
void fn_1_54F8(s16 soundSlot, s16 pitch) {
    if (lbl_1_bss_210.soundState[soundSlot] == 1) {
        if (HuAudFXStatusGet(lbl_1_bss_210.soundHandle[soundSlot]) == 2) {
            HuAudFXPitchSet(lbl_1_bss_210.soundHandle[soundSlot], pitch);
        }
    }
}

/* Sound-slot helper: changes volume only while the selected effect handle is active. */
void fn_1_5588(s16 soundSlot, s16 volume) {
    if (lbl_1_bss_210.soundState[soundSlot] == 1) {
        if (HuAudFXStatusGet(lbl_1_bss_210.soundHandle[soundSlot]) == 2) {
            HuAudFXVolSet(lbl_1_bss_210.soundHandle[soundSlot], volume);
        }
    }
}

/* Sound-slot shutdown helper: stops a tracked effect when its handle has one of the two accepted
 * active states. */
void fn_1_5618(s16 soundSlot) {
    if ((lbl_1_bss_210.soundState[soundSlot] == 1) &&
        ((HuAudFXStatusGet(lbl_1_bss_210.soundHandle[soundSlot]) == 2) ||
         (HuAudFXStatusGet(lbl_1_bss_210.soundHandle[soundSlot]) == 3))) {
        lbl_1_bss_210.soundState[soundSlot] = -1;
        HuAudFXStop(lbl_1_bss_210.soundHandle[soundSlot]);
    }
}

/* Stops one reusable effect slot through the shared shutdown helper. */
void fn_1_56E0(s16 soundSlot)
{
    fn_1_5618(soundSlot);
}

/* Stops all game audio when requested by the scene shutdown path. */
void fn_1_57A8(void) {
    HuAudAllStop();
}

/* Main sequence callback: starts the four tree motions once, updates the round and returns its
 * completion state. */
s16 fn_1_57C8(void) {
    s32 result = 0;
    s16 modelIndex;

    if (lbl_1_bss_C58[0] == 0) {
        lbl_1_bss_210.mode = 1;
        for (modelIndex = 36; modelIndex <= 39; modelIndex++) {
            Hu3DMotionSpeedSet(
                lbl_1_bss_4B4[modelIndex].modelId,
                1.0f);
        }
        lbl_1_bss_C58[0] = 1;
    }
    fn_1_5884();
    fn_1_887C();
    fn_1_2670();
    result = fn_1_58B0();
    return result;
}

/* Each scene update, refreshes input and computer choices before advancing players and puzzle
 * objects. */
void fn_1_5884(void) {
    fn_1_984();
    fn_1_AD68();
    fn_1_5A5C();
    fn_1_7DE8();
}

/* After round updates, waits for both players to finish climbing, then selects the higher score;
 * ties favor group zero. */
s32 fn_1_58B0(void)
{
    s32 done = 0;
    s32 groupOnePlayerNo;
    s32 groupZeroPlayerNo;

    if ((lbl_1_bss_2F4[0].state == 19) &&
        (lbl_1_bss_2F4[1].state == 19)) {
        lbl_1_bss_210.complete = 1;
    }

    if (lbl_1_bss_210.complete != 0) {
        if (lbl_1_bss_2F4[0].score < lbl_1_bss_2F4[1].score) {
            lbl_1_bss_2F4[1].result = 1;
            lbl_1_bss_210.winnerIndex = 1;
            groupOnePlayerNo = lbl_1_bss_2F4[1].playerNo;
            if (_CheckFlag(FLAG_MG_PRACTICE) == FALSE) {
                GwPlayer[groupOnePlayerNo].mgCoinBonus = 10;
            }
            MgSeqWinnerSet(lbl_1_bss_2F4[1].characterNo, -1, -1, -1);
        } else {
            lbl_1_bss_2F4[0].result = 1;
            lbl_1_bss_210.winnerIndex = 0;
            groupZeroPlayerNo = lbl_1_bss_2F4[0].playerNo;
            if (_CheckFlag(FLAG_MG_PRACTICE) == FALSE) {
                GwPlayer[groupZeroPlayerNo].mgCoinBonus = 10;
            }
            MgSeqWinnerSet(lbl_1_bss_2F4[0].characterNo, -1, -1, -1);
        }
        done = 1;
    }

    return done;
}

/* Each round update, moves players to a tree choice, accepts left/right and A input, then animates
 * their climbs. */
void fn_1_5A5C(void)
{
    s16 unfinishedMoves = 0;
    SlotPositions_5A5C slotPositions = { {
        { -400.0f, 0.0f, -250.0f },
        { -135.0f, 0.0f, -250.0f },
        { 135.0f, 0.0f, -250.0f },
        { 400.0f, 0.0f, -250.0f }
    } };
    SlotPositions_5A5C frontPositions = { {
        { -400.0f, 0.0f, -450.0f },
        { -135.0f, 0.0f, -450.0f },
        { 135.0f, 0.0f, -450.0f },
        { 400.0f, 0.0f, -450.0f }
    } };
    StartPositions_5A5C startPositions = { {
        { -150.0f, 0.0f, 0.0f },
        { 150.0f, 0.0f, 0.0f }
    } };
    HuVecF facingOffset;
    JumpOffsets_5A5C jumpOffsets = { { -120.0f, 120.0f } };
    f32 climbStepHeight = 300.0f;
    s16 playerIndex;
    M662Player *player;
    s16 slotStep;
    M662Model *carrier;
    f32 durationFrames;
    f32 startingAngle;
    f32 approachDistance;

    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        player = &lbl_1_bss_2F4[playerIndex];
        fn_1_48E8(player, 6);
        switch (player->state) {
        case 1:
            player->slot = (lbl_1_bss_2F4[playerIndex ^ 1].slot == 3) ? 2 : 3;
            facingOffset.x = player->position.x - slotPositions.position[player->slot].x;
            facingOffset.z = player->position.z - slotPositions.position[player->slot].z;
            player->targetAngle =
                180.0 + 180.0 * (atan2(facingOffset.x, facingOffset.z) / 3.141592653589793);
            lbl_1_bss_3F4[playerIndex].shiftState = 1;
            lbl_1_bss_3F4[playerIndex].shiftMotion = 1;
            lbl_1_bss_3F4[playerIndex].shiftStartFrame = 0.0f;
            lbl_1_bss_3F4[playerIndex].blendDuration = 10.0f;
            lbl_1_bss_3F4[playerIndex].shiftAttributes = HU3D_MOTATTR_LOOP;
            lbl_1_bss_3F4[playerIndex].resetAttributes = 0;
            player->state++;
            break;
        case 2:
            durationFrames = 10.0f;
            fn_1_2DA0(&player->rotation.y, durationFrames, 0.0f, player->targetAngle);
            durationFrames = 80.0f;
            unfinishedMoves = fn_1_2DA0(&player->position.x, durationFrames,
                                        startPositions.position[playerIndex].x,
                                        slotPositions.position[player->slot].x);
            unfinishedMoves += fn_1_2DA0(&player->position.z, durationFrames,
                                         startPositions.position[playerIndex].z,
                                         slotPositions.position[player->slot].z);
            if (unfinishedMoves == 0) {
                player->state++;
            }
            break;
        case 3:
            durationFrames = 10.0f;
            unfinishedMoves =
                fn_1_2DA0(&player->rotation.y, durationFrames, player->targetAngle, 180.0f);
            if (unfinishedMoves == 0) {
                lbl_1_bss_210.timerObject = fn_1_2EC0(0, 10, 0, 288.0f, 64.0f);
                fn_1_2F34(lbl_1_bss_210.timerObject);
                lbl_1_bss_3F4[playerIndex].shiftState = 1;
                lbl_1_bss_3F4[playerIndex].shiftMotion = 0;
                lbl_1_bss_3F4[playerIndex].shiftStartFrame = 0.0f;
                lbl_1_bss_3F4[playerIndex].blendDuration = 10.0f;
                lbl_1_bss_3F4[playerIndex].shiftAttributes = HU3D_MOTATTR_LOOP;
                lbl_1_bss_3F4[playerIndex].resetAttributes = 0;
                omVibrate(player->playerNo, 24, 7, 3);
                player->state++;
            }
            break;
        case 4:
            if (player->delay++ <= 30) {
                break;
            }
            if (fn_1_2F60(lbl_1_bss_210.timerObject) != 0) {
                if (player->counter != 0) {
                    player->input.buttonsDown |= PAD_BUTTON_A;
                    player->input.stickX = 0;
                } else {
                    player->counter++;
                }
            }
            if (player->sideways == 0 && (player->input.buttonsDown & PAD_BUTTON_A) != 0) {
                fn_1_2F88(lbl_1_bss_210.timerObject);
                player->state = 7;
                lbl_1_bss_3F4[playerIndex].shiftState = 1;
                lbl_1_bss_3F4[playerIndex].shiftMotion = 1;
                lbl_1_bss_3F4[playerIndex].shiftStartFrame = 0.0f;
                lbl_1_bss_3F4[playerIndex].blendDuration = 10.0f;
                lbl_1_bss_3F4[playerIndex].shiftAttributes = HU3D_MOTATTR_LOOP;
                lbl_1_bss_3F4[playerIndex].resetAttributes = 0;
                if (playerIndex == 0) {
                    lbl_1_bss_2F4[playerIndex ^ 1].state = 1;
                }
                break;
            }
            /* Only horizontal input beyond +/-40 assigns the slot step here; neutral input does not
             * initialize or reset it before the following checks. */
            if (player->input.stickX > 40) {
                slotStep = 1;
            }
            if (player->input.stickX < -40) {
                slotStep = -1;
            }
            if (player->sideways != 0 && player->lastStep != slotStep) {
                slotStep = 0;
            }
            player->lastStep = slotStep;
            /* Skip the occupied slot while keeping the movement direction. */
            if (player->slot + slotStep == lbl_1_bss_2F4[playerIndex ^ 1].slot) {
                slotStep += slotStep;
            }
            if (player->slot + slotStep > 3) {
                slotStep = 0;
            }
            if (player->slot + slotStep < 0) {
                slotStep = 0;
            }
            player->slot += slotStep;
            if (slotStep != 0) {
                player->startX = player->position.x;
                player->stepSize = (slotStep > 0) ? slotStep : slotStep * -1;
                if (player->stepSize >= 2) {
                    slotStep -= slotStep / 2;
                }
                player->targetAngle = 90.0f * slotStep;
                if (player->sideways == 0) {
                    lbl_1_bss_3F4[playerIndex].shiftState = 1;
                    lbl_1_bss_3F4[playerIndex].shiftMotion = 1;
                    lbl_1_bss_3F4[playerIndex].shiftStartFrame = 0.0f;
                    lbl_1_bss_3F4[playerIndex].blendDuration = 10.0f;
                    lbl_1_bss_3F4[playerIndex].shiftAttributes = HU3D_MOTATTR_LOOP;
                    lbl_1_bss_3F4[playerIndex].resetAttributes = 0;
                    player->rotation.y = (player->targetAngle < 0.0f) ? -180.0f : 180.0f;
                }
                player->state++;
            } else {
                if (player->sideways != 0) {
                    player->sideways = 0;
                    player->state = 6;
                }
                break;
            }
        case 5:
            durationFrames = 15.0f;
            startingAngle = (player->targetAngle < 0.0f) ? -180.0f : 180.0f;
            fn_1_2DA0(&player->rotation.y, durationFrames, startingAngle, player->targetAngle);
            durationFrames = 30.0f;
            if (player->stepSize == 2) {
                durationFrames = 59.0f;
            }
            unfinishedMoves = fn_1_2DA0(&player->position.x, durationFrames, player->startX,
                                  slotPositions.position[player->slot].x);
            if (unfinishedMoves == 0) {
                player->position.x = slotPositions.position[player->slot].x;
                player->sideways = 1;
                player->state = 4;
            }
            break;
        case 6:
            durationFrames = 15.0f;
            startingAngle = (player->targetAngle < 0.0f) ? -180.0f : 180.0f;
            unfinishedMoves =
                fn_1_2DA0(&player->rotation.y, durationFrames, player->targetAngle, startingAngle);
            if (unfinishedMoves == 0) {
                lbl_1_bss_3F4[playerIndex].shiftState = 1;
                lbl_1_bss_3F4[playerIndex].shiftMotion = 0;
                lbl_1_bss_3F4[playerIndex].shiftStartFrame = 0.0f;
                lbl_1_bss_3F4[playerIndex].blendDuration = 10.0f;
                lbl_1_bss_3F4[playerIndex].shiftAttributes = HU3D_MOTATTR_LOOP;
                lbl_1_bss_3F4[playerIndex].resetAttributes = 0;
                player->delay = 15;
                player->state = 4;
            }
            break;
        case 7:
            facingOffset.x = player->position.x - frontPositions.position[player->slot].x;
            facingOffset.z = player->position.z - frontPositions.position[player->slot].z;
            approachDistance =
                sqrt(facingOffset.x * facingOffset.x + facingOffset.z * facingOffset.z);
            if (approachDistance < 125.0f) {
                CharModelStepSet(player->characterNo, 1);
                player->position.y = 15.0f;
            }
            durationFrames = 40.0f;
            unfinishedMoves = fn_1_2DA0(&player->position.z, durationFrames,
                                        slotPositions.position[player->slot].z,
                                        frontPositions.position[player->slot].z);
            if (unfinishedMoves == 0) {
                player->targetAngle = player->rotation.y;
                player->state++;
            }
            break;
        case 8:
            lbl_1_bss_3F4[playerIndex].shiftState = 1;
            lbl_1_bss_3F4[playerIndex].shiftMotion = 0;
            lbl_1_bss_3F4[playerIndex].shiftStartFrame = 0.0f;
            lbl_1_bss_3F4[playerIndex].blendDuration = 10.0f;
            lbl_1_bss_3F4[playerIndex].shiftAttributes = HU3D_MOTATTR_LOOP;
            lbl_1_bss_3F4[playerIndex].resetAttributes = 0;
            player->counter = 0;
            player->state++;
            break;
        case 9:
            if (playerIndex == 1) {
                lbl_1_bss_2F4[0].state = 10;
                lbl_1_bss_2F4[1].state = 10;
            }
            break;
        case 10:
            if (player->counter++ > 30) {
                lbl_1_bss_3F4[playerIndex].shiftState = 1;
                lbl_1_bss_3F4[playerIndex].shiftMotion = 1;
                lbl_1_bss_3F4[playerIndex].shiftStartFrame = 0.0f;
                lbl_1_bss_3F4[playerIndex].blendDuration = 10.0f;
                lbl_1_bss_3F4[playerIndex].shiftAttributes = HU3D_MOTATTR_LOOP;
                lbl_1_bss_3F4[playerIndex].resetAttributes = 0;
                player->counter = 0;
                player->state++;
            }
            break;
        case 11:
            facingOffset.x = player->position.x - frontPositions.position[player->slot].x;
            facingOffset.z = player->position.z - frontPositions.position[player->slot].z;
            approachDistance =
                sqrt(facingOffset.x * facingOffset.x + facingOffset.z * facingOffset.z);
            if (approachDistance > 125.0f) {
                player->position.y = 0.0f;
            }
            durationFrames = 15.0f;
            carrier = &lbl_1_bss_4B4[lbl_1_bss_210.ranks[player->slot] + 36];
            durationFrames = 30.0f;
            unfinishedMoves = 0;
            unfinishedMoves += fn_1_2DA0(&player->position.z, durationFrames,
                                   frontPositions.position[player->slot].z, -600.0f);
            if (unfinishedMoves == 0) {
                lbl_1_bss_3F4[playerIndex].shiftState = 1;
                lbl_1_bss_3F4[playerIndex].shiftMotion = 2;
                lbl_1_bss_3F4[playerIndex].shiftStartFrame = 0.0f;
                lbl_1_bss_3F4[playerIndex].blendDuration = 10.0f;
                lbl_1_bss_3F4[playerIndex].shiftAttributes = 0;
                lbl_1_bss_3F4[playerIndex].resetAttributes = HU3D_MOTATTR_LOOP;
                player->startY = player->position.y;
                player->startZ = player->position.z;
                player->rotation.y = -180.0f;
                player->gravity = -1.0f;
                player->velocityY = 24.0f;
                player->state++;
            }
            break;
        case 12:
            carrier = &lbl_1_bss_4B4[lbl_1_bss_210.ranks[player->slot] + 36];
            durationFrames = 25.0f;
            unfinishedMoves = fn_1_2DA0(&player->position.x, durationFrames,
                                        frontPositions.position[player->slot].x,
                                        carrier->position.x + jumpOffsets.offset[player->score]);
            unfinishedMoves +=
                fn_1_2DA0(&player->position.z, durationFrames, player->startZ, -850.0f);
            player->position.y += player->velocityY;
            player->velocityY += player->gravity;
            if (player->position.y >= player->startY + climbStepHeight) {
                player->position.y = 10.0f + (player->startY + climbStepHeight);
                player->velocityY = 0.0f;
                player->gravity = 0.0f;
            } else {
                unfinishedMoves++;
            }
            if (unfinishedMoves == 0) {
                lbl_1_bss_3F4[playerIndex].shiftState = 1;
                lbl_1_bss_3F4[playerIndex].shiftMotion = 3;
                lbl_1_bss_3F4[playerIndex].shiftStartFrame = 0.0f;
                lbl_1_bss_3F4[playerIndex].blendDuration = 5.0f;
                lbl_1_bss_3F4[playerIndex].shiftAttributes = 0;
                lbl_1_bss_3F4[playerIndex].resetAttributes = HU3D_MOTATTR_LOOP;
                player->score++;
                player->state++;
            }
            break;
        case 13:
            if (lbl_1_bss_3F4[playerIndex].requestedTime >= 0.0f &&
                lbl_1_bss_3F4[playerIndex].motionEnded != 0) {
                lbl_1_bss_3F4[playerIndex].shiftState = 1;
                lbl_1_bss_3F4[playerIndex].shiftMotion = 1;
                lbl_1_bss_3F4[playerIndex].shiftStartFrame = 0.0f;
                lbl_1_bss_3F4[playerIndex].blendDuration = 10.0f;
                lbl_1_bss_3F4[playerIndex].shiftAttributes = HU3D_MOTATTR_LOOP;
                lbl_1_bss_3F4[playerIndex].resetAttributes = 0;
                player->counter = 12;
                player->state++;
            }
            break;
        case 14:
            durationFrames = 15.0f;
            unfinishedMoves = fn_1_2DA0(&player->rotation.y, durationFrames, -180.0f, 0.0f);
            if (unfinishedMoves == 0) {
                if (player->counter++ >= 10) {
                    lbl_1_bss_3F4[playerIndex].shiftState = 1;
                    lbl_1_bss_3F4[playerIndex].shiftMotion = 0;
                    lbl_1_bss_3F4[playerIndex].shiftStartFrame = 0.0f;
                    lbl_1_bss_3F4[playerIndex].blendDuration = 10.0f;
                    lbl_1_bss_3F4[playerIndex].shiftAttributes = 0;
                    lbl_1_bss_3F4[playerIndex].resetAttributes = HU3D_MOTATTR_LOOP;
                    player->state++;
                }
            }
            break;
        case 15:
            if (lbl_1_bss_3F4[playerIndex].requestedTime > 10.0f) {
                lbl_1_bss_3F4[playerIndex].shiftState = 1;
                lbl_1_bss_3F4[playerIndex].shiftMotion = 2;
                lbl_1_bss_3F4[playerIndex].shiftStartFrame = 0.0f;
                lbl_1_bss_3F4[playerIndex].blendDuration = 10.0f;
                lbl_1_bss_3F4[playerIndex].shiftAttributes = 0;
                lbl_1_bss_3F4[playerIndex].resetAttributes = HU3D_MOTATTR_LOOP;
                player->startX = player->position.x;
                player->startY = player->position.y;
                player->gravity = -1.0f;
                player->velocityY = 24.0f;
                /* Track the taller chosen tree with the shared camera. */
            if (lbl_1_bss_210.scores[player->slot] >
                    lbl_1_bss_210.scores[lbl_1_bss_2F4[playerIndex ^ 1].slot]) {
                    lbl_1_bss_210.centerY = Center.y;
                }
                if (player->score >= lbl_1_bss_210.scores[player->slot] - 1) {
                    player->state = 17;
                } else {
                    player->state++;
                }
            }
            break;
        case 16:
            carrier = &lbl_1_bss_4B4[lbl_1_bss_210.ranks[player->slot] + 36];
            durationFrames = 25.0f;
            unfinishedMoves = fn_1_2DA0(&player->position.x, durationFrames, player->startX,
                                  carrier->position.x + jumpOffsets.offset[player->score % 2]);
            player->position.y += player->velocityY;
            player->velocityY += player->gravity;
            if (player->position.y >= player->startY + climbStepHeight) {
                player->position.y = player->startY + climbStepHeight;
                player->velocityY = 0.0f;
                player->gravity = 0.0f;
            } else {
                unfinishedMoves++;
            }
            if (lbl_1_bss_210.scores[player->slot] >
                lbl_1_bss_210.scores[lbl_1_bss_2F4[playerIndex ^ 1].slot]) {
                unfinishedMoves += fn_1_2DA0(&Center.y, durationFrames, lbl_1_bss_210.centerY,
                                       player->startY + climbStepHeight);
            }
            if (unfinishedMoves == 0) {
                lbl_1_bss_3F4[playerIndex].shiftState = 1;
                lbl_1_bss_3F4[playerIndex].shiftMotion = 3;
                lbl_1_bss_3F4[playerIndex].shiftStartFrame = 0.0f;
                lbl_1_bss_3F4[playerIndex].blendDuration = 5.0f;
                lbl_1_bss_3F4[playerIndex].shiftAttributes = 0;
                lbl_1_bss_3F4[playerIndex].resetAttributes = HU3D_MOTATTR_LOOP;
                player->counter = 0;
                player->score++;
                player->state = 14;
            }
            break;
        case 17:
            carrier = &lbl_1_bss_4B4[lbl_1_bss_210.ranks[player->slot] + 36];
            durationFrames = 25.0f;
            unfinishedMoves =
                fn_1_2DA0(&player->position.x, durationFrames, player->startX, carrier->position.x);
            player->position.y += player->velocityY;
            player->velocityY += player->gravity;
            if (player->position.y >= player->startY + climbStepHeight) {
                player->position.y = player->startY + climbStepHeight;
                player->velocityY = 0.0f;
                player->gravity = 0.0f;
            } else {
                unfinishedMoves++;
            }
            if (lbl_1_bss_210.scores[player->slot] >
                lbl_1_bss_210.scores[lbl_1_bss_2F4[playerIndex ^ 1].slot]) {
                unfinishedMoves += fn_1_2DA0(&Center.y, durationFrames, lbl_1_bss_210.centerY,
                                       player->startY + climbStepHeight);
            }
            if (unfinishedMoves == 0) {
                lbl_1_bss_3F4[playerIndex].shiftState = 1;
                lbl_1_bss_3F4[playerIndex].shiftMotion = 3;
                lbl_1_bss_3F4[playerIndex].shiftStartFrame = 0.0f;
                lbl_1_bss_3F4[playerIndex].blendDuration = 5.0f;
                lbl_1_bss_3F4[playerIndex].shiftAttributes = 0;
                lbl_1_bss_3F4[playerIndex].resetAttributes = HU3D_MOTATTR_LOOP;
                player->state++;
            }
            break;
        case 18:
            if (lbl_1_bss_3F4[playerIndex].requestedTime >= 0.0f &&
                lbl_1_bss_3F4[playerIndex].motionEnded != 0) {
                lbl_1_bss_3F4[playerIndex].shiftState = 1;
                lbl_1_bss_3F4[playerIndex].shiftMotion = 0;
                lbl_1_bss_3F4[playerIndex].shiftStartFrame = 0.0f;
                lbl_1_bss_3F4[playerIndex].blendDuration = 10.0f;
                lbl_1_bss_3F4[playerIndex].shiftAttributes = HU3D_MOTATTR_LOOP;
                lbl_1_bss_3F4[playerIndex].resetAttributes = 0;
                player->state++;
            }
            break;
        case 0:
        case 19:
        default:
            break;
        }
    }
}

/* Path-step helper: advances one tree along its curved or linear X/Z route and reports when that
 * segment is finished. */
s16 fn_1_795C(M662Puzzle *puzzle)
{
    s16 segmentFinished = 0;
    M662Model *carrier;
    s16 unfinishedAxes = 0;

    carrier = &lbl_1_bss_4B4[puzzle->secondModel];
    if (puzzle->pathIndex != -1) {
        switch (puzzle->pathMode) {
        case 0: {
            M662SinePath *path = &lbl_1_data_150[puzzle->pathIndex];
            carrier->position.x = (f32)((f64)path->centerX +
                (f64)path->amplitude * sin((3.141592653589793 * (f64)puzzle->progress) / 180.0));
            carrier->position.z = (f32)(path->centerZ +
                100.0 * cos((3.141592653589793 * (f64)puzzle->progress) / 180.0));
            unfinishedAxes = fn_1_2DA0(&puzzle->progress, path->duration, path->start, path->end);
            if (unfinishedAxes == 0) {
                segmentFinished = 1;
            }
            break;
        }
        case 1: {
            M662LinearPath *path = &lbl_1_data_32C[puzzle->pathIndex];
            unfinishedAxes = fn_1_2DA0(&carrier->position.x, path->duration, path->startX,
                               path->endX);
            unfinishedAxes += fn_1_2DA0(&carrier->position.z, path->duration, path->startZ,
                                path->endZ);
            if (unfinishedAxes == 0) {
                segmentFinished = 1;
            }
            break;
        }
        }
    } else {
        segmentFinished = 1;
    }
    return segmentFinished;
}

/* During the tree shuffle, advances all four trees through each of the twelve path steps. */
s16 fn_1_7AF8(void)
{
    s16 finished = 0;
    s16 finishedCount = 0;
    s16 anyPathActive = 0;
    s16 puzzleIndex;

    for (puzzleIndex = 0; puzzleIndex < 4; puzzleIndex++) {
        M662Puzzle *puzzle = &lbl_1_bss_D0[puzzleIndex];

        switch (puzzle->status) {
        case 0:
            puzzle->pathIndex = puzzle->paths[lbl_1_bss_210.pathStep].index;
            puzzle->pathMode = puzzle->paths[lbl_1_bss_210.pathStep].mode;
            /* The curved-path start is fetched even for index -1, before the inactive-path
             * checks. */
            if (puzzle->pathMode == 0) {
                puzzle->progress = lbl_1_data_150[puzzle->pathIndex].start;
            }
            if (puzzle->pathIndex != -1) {
                anyPathActive = 1;
            }
            puzzle->status++;
            /* Fall through to advance the selected path. */
        case 1:
            finishedCount += fn_1_795C(puzzle);
            break;
        }
    }

    if (anyPathActive != 0) {
        fn_1_5008(1, 5, 0);
    }
    if (finishedCount >= 4) {
        for (puzzleIndex = 0; puzzleIndex < 4; puzzleIndex++) {
            lbl_1_bss_D0[puzzleIndex].status = 0;
        }
        if (++lbl_1_bss_210.pathStep >= 12) {
            finished = 1;
        }
    }
    return finished;
}

/* Each scene update, advances the tree shuffle or falling models, orders tree depth and updates
 * attached model positions. */
void fn_1_7DE8(void)
{
    s16 depthOrder[4];
    s16 sequenceFinished = 0;
    s16 index;
    s16 compareIndex;
    s16 swappedIndex;
    M662Puzzle *puzzle;
    f32 depthOffset;

    switch (lbl_1_bss_210.mode) {
    case 0:
        break;
    case 1:
        sequenceFinished = fn_1_7AF8();
        if (sequenceFinished != 0) {
            for (index = 0; index < 4; index++) {
                puzzle = &lbl_1_bss_D0[index];
                Hu3DModelAttrReset(lbl_1_bss_4B4[puzzle->secondModel].modelId, HU3D_MOTATTR_LOOP);
            }
            for (index = 32; index <= 35; index++) {
                Hu3DModelAttrReset(lbl_1_bss_4B4[index].modelId, HU3D_ATTR_DISPOFF);
            }
            for (index = 0; index < 4; index++) {
                lbl_1_bss_210.ranks[index] = index;
            }
            for (index = 0; index < 4; index++) {
                for (compareIndex = 0; compareIndex < 4; compareIndex++) {
                    if (lbl_1_bss_4B4[lbl_1_bss_210.ranks[index] + 36].position.x <
                        lbl_1_bss_4B4[lbl_1_bss_210.ranks[compareIndex] + 36].position.x) {
                        swappedIndex = lbl_1_bss_210.ranks[index];
                        lbl_1_bss_210.ranks[index] = lbl_1_bss_210.ranks[compareIndex];
                        lbl_1_bss_210.ranks[compareIndex] = swappedIndex;
                    }
                }
            }
            for (index = 0; index < 4; index++) {
                if (lbl_1_bss_210.ranks[index] == 0) {
                    lbl_1_bss_210.scores[index] = 6;
                }
                if (lbl_1_bss_210.ranks[index] == 1) {
                    lbl_1_bss_210.scores[index] = 5;
                }
                if (lbl_1_bss_210.ranks[index] == 2) {
                    lbl_1_bss_210.scores[index] = 4;
                }
                if (lbl_1_bss_210.ranks[index] == 3) {
                    lbl_1_bss_210.scores[index] = 3;
                }
            }
            lbl_1_bss_2F4[0].state = 1;
            lbl_1_bss_210.mode++;
        }
        break;
    case 2:
        fn_1_8754();
        break;
    }

    for (index = 0; index < 4; index++) {
        depthOrder[index] = index;
    }
    for (index = 0; index < 4; index++) {
        for (compareIndex = 0; compareIndex < 4; compareIndex++) {
            if (lbl_1_bss_4B4[depthOrder[index] + 36].position.z >
                lbl_1_bss_4B4[depthOrder[compareIndex] + 36].position.z) {
                swappedIndex = depthOrder[index];
                depthOrder[index] = depthOrder[compareIndex];
                depthOrder[compareIndex] = swappedIndex;
            }
        }
    }
    /* Separate crossing trees slightly in Z according to their depth order. */
    for (index = 0, depthOffset = 0.0f; index < 4; index++, depthOffset -= 5.0f) {
        puzzle = &lbl_1_bss_D0[depthOrder[index]];
        puzzle->depthOffset = depthOffset;
    }
    for (index = 0; index < 4; index++) {
        puzzle = &lbl_1_bss_D0[index];
        if (lbl_1_bss_210.mode == 2) {
            if (puzzle->countdown-- < 0) {
                Hu3DMotionSpeedSet(lbl_1_bss_4B4[puzzle->firstModel].modelId, 1.0f);
                Hu3DMotionTimeSet(lbl_1_bss_4B4[puzzle->firstModel].modelId, 0.0f);
                fn_1_5008(2, 6, &lbl_1_bss_4B4[puzzle->secondModel].position);
                puzzle->countdown = frandmod(250) + 50;
            }
        }
        Hu3DModelPosSet(lbl_1_bss_4B4[puzzle->secondModel].modelId,
                        lbl_1_bss_4B4[puzzle->secondModel].position.x,
                        lbl_1_bss_4B4[puzzle->secondModel].position.y,
                        puzzle->depthOffset + lbl_1_bss_4B4[puzzle->secondModel].position.z);
        Hu3DModelRotSet(lbl_1_bss_4B4[puzzle->secondModel].modelId,
                        lbl_1_bss_4B4[puzzle->secondModel].rotation.x,
                        lbl_1_bss_4B4[puzzle->secondModel].rotation.y,
                        lbl_1_bss_4B4[puzzle->secondModel].rotation.z);
        Hu3DModelObjPosGet(lbl_1_bss_4B4[puzzle->secondModel].modelId, puzzle->objName,
                           &lbl_1_bss_4B4[puzzle->firstModel].position);
    }
}

/* After the shuffle, starts six delayed falling models and stops each motion once it has fallen
 * below the scene. */
s32 fn_1_8754(void)
{
    s32 finished = 1;
    s16 modelIndex = 40;
    M662Model *fallingModel;

    while (modelIndex <= 45) {
        fallingModel = &lbl_1_bss_4B4[modelIndex];
        switch (fallingModel->motionState) {
        case 0: {
            if ((fallingModel->motionFrame)-- < 0) {
                Hu3DModelAttrReset(fallingModel->modelId, HU3D_ATTR_DISPOFF);
                Hu3DMotionSpeedSet(fallingModel->modelId, 1.0f);
                fallingModel->motionState += 1;
            }
            finished = 0;
            break;
        }
        case 1: {
            if ((fallingModel->position.y -= 2.0f) <
                -300.0f) {
                Hu3DMotionSpeedSet(fallingModel->modelId, 0.0f);
                fallingModel->motionState += 1;
            }
            finished = 0;
            break;
        }
        case 2:
            break;
        }
        modelIndex++;
    }
    return finished;
}

/* During the round, restarts a finished scenery motion when an animated tree overlaps its
 * horizontal activation range. */
void fn_1_887C(void)
{
    M662Player *playerRecord[1];
    s16 sceneIndex;
    s16 carrierIndex;
    M662Model *carrier;
    M662Bounds *activationBounds;

    /* These player-pointer assignments are unused by the scenery checks below. */
    for (sceneIndex = 0; sceneIndex < 2; sceneIndex++) {
        playerRecord[0] = &lbl_1_bss_2F4[sceneIndex];
    }

    for (sceneIndex = 0; sceneIndex < 13; sceneIndex++) {
        if (Hu3DMotionEndCheck(
                lbl_1_bss_4B4[sceneIndex + 19]
                    .modelId) != 0) {
            activationBounds = &lbl_1_bss_0[sceneIndex];
            for (carrierIndex = 0; carrierIndex < 4; carrierIndex++) {
                carrier = &lbl_1_bss_4B4
                              [carrierIndex + 36];
                if (Hu3DMotionEndCheck(
                        carrier->modelId) == 0 &&
                    (((activationBounds->centerX + activationBounds->radius >=
                       50.0f + carrier->position.x) &&
                      (activationBounds->centerX - activationBounds->radius <=
                       50.0f + carrier->position.x)) ||
                     ((activationBounds->centerX + activationBounds->radius >=
                       carrier->position.x - 50.0f) &&
                      (activationBounds->centerX - activationBounds->radius <=
                       carrier->position.x - 50.0f)) ||
                     ((activationBounds->centerX + activationBounds->radius <=
                       50.0f + carrier->position.x) &&
                      (activationBounds->centerX + activationBounds->radius >=
                       carrier->position.x - 50.0f)) ||
                     ((activationBounds->centerX - activationBounds->radius <=
                       50.0f + carrier->position.x) &&
                      (activationBounds->centerX - activationBounds->radius >=
                       carrier->position.x - 50.0f)))) {
                    Hu3DMotionTimeSet(
                        lbl_1_bss_4B4
                            [sceneIndex + 19]
                                .modelId,
                        0.0f);
                    Hu3DMotionSpeedSet(
                        lbl_1_bss_4B4
                            [sceneIndex + 19]
                                .modelId,
                        1.0f);
                }
            }
        }
    }
}

/* Curved X/Z shuffle segments; their interpolation angles are in degrees. */
M662SinePath lbl_1_data_150[17] = {
    { 30.0f, -300.0f, { 0 }, -900.0f, -90.0f, 90.0f, 150.0f },
        { 30.0f, 300.0f, { 0 }, -900.0f, 90.0f, -90.0f, 150.0f },
        { 30.0f, 0.0f, { 0 }, -900.0f, 90.0f, 270.0f, 150.0f },
        { 30.0f, 0.0f, { 0 }, -900.0f, -90.0f, 90.0f, 150.0f },
        { 30.0f, -300.0f, { 0 }, -900.0f, 90.0f, -90.0f, 150.0f },
        { 30.0f, 300.0f, { 0 }, -900.0f, -90.0f, 90.0f, 150.0f },
        { 30.0f, -300.0f, { 0 }, -900.0f, 90.0f, 270.0f, 150.0f },
        { 30.0f, 300.0f, { 0 }, -900.0f, -90.0f, -270.0f, 150.0f },
        { 30.0f, -150.0f, { 0 }, -900.0f, -90.0f, -270.0f, 300.0f },
        { 30.0f, -150.0f, { 0 }, -900.0f, 90.0f, -90.0f, 300.0f },
        { 30.0f, 150.0f, { 0 }, -900.0f, -90.0f, -270.0f, 300.0f },
        { 30.0f, 150.0f, { 0 }, -900.0f, 90.0f, -90.0f, 300.0f },
        { 30.0f, 0.0f, { 0 }, -900.0f, -90.0f, -270.0f, 450.0f },
        { 30.0f, 0.0f, { 0 }, -900.0f, 90.0f, -90.0f, 450.0f },
        { 30.0f, 0.0f, { 0 }, -900.0f, 90.0f, 270.0f, 450.0f },
        { 30.0f, 150.0f, { 0 }, -900.0f, 90.0f, 270.0f, 300.0f },
        { 30.0f, 300.0f, { 0 }, -900.0f, 90.0f, 270.0f, 150.0f }
};

/* Straight X/Z shuffle segments, each lasting thirty updates. */
M662LinearPath lbl_1_data_32C[10] = {
    { 30.0f, -450.0f, { 0 }, 0.0f, -500.0f, { 0 }, -900.0f },
        { 30.0f, -150.0f, { 0 }, 0.0f, -500.0f, { 0 }, -900.0f },
        { 30.0f, 150.0f, { 0 }, 0.0f, 500.0f, { 0 }, -900.0f },
        { 30.0f, 450.0f, { 0 }, 0.0f, 500.0f, { 0 }, -900.0f },
        { 30.0f, -500.0f, { 0 }, 0.0f, 0.0f, { 0 }, -900.0f },
        { 30.0f, 500.0f, { 0 }, 0.0f, 0.0f, { 0 }, -900.0f },
        { 30.0f, 0.0f, { 0 }, 0.0f, -450.0f, { 0 }, -900.0f },
        { 30.0f, 0.0f, { 0 }, 0.0f, -150.0f, { 0 }, -900.0f },
        { 30.0f, 0.0f, { 0 }, 0.0f, 150.0f, { 0 }, -900.0f },
        { 30.0f, 0.0f, { 0 }, 0.0f, 450.0f, { 0 }, -900.0f }
};
