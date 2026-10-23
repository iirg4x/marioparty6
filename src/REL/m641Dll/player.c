/* Character setup, motion, input, and movement for Mole-it! */
#include "math.h"

#include "dolphin/gx.h"

#include "game/main.h"

#include "game/flag.h"

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

#include "include/game/mg/actman.h"

#include "include/game/audio.h"

#include "include/game/gamemes.h"

#include "game/hu3d.h"

#include "REL/m641Dll/m641Dll.h"

typedef struct {
    s16 motionId;       /* Character motion-table index, or -1 when unused. */
    s16 padding;        /* Unused two-byte padding between the motion ID and blend duration. */
    f32 blendTime;      /* Motion blend duration in frames. */
    u32 attributes;     /* Character-model attributes applied with this motion. */
    s32 voiceEnabled;   /* Nonzero enables character voice and effect cues. */
} M641MotionInfo;

#define M641_MOTION_COUNT 14

#define M641_CHARACTER_MOTION_COUNT 13

#define M641_MOVEMENT_STATE_HIT_REACTION 10

#define M641_MOVEMENT_STATE_COLLISION_LAUNCH 11

#define M641_MOVEMENT_STATE_COLLISION_AIRBORNE 12

#define M641_MOVEMENT_STATE_COLLISION_LANDING 13

#define M641_MOTION_INDEX_WIN 12

#define M641_MOTION_INDEX_LOSS 13

#define M641_STICK_LIMIT 56

#define M641_FADE_IN_STATE_INITIAL_WAIT 18

#define M641_FADE_IN_STATE_FIRST_INPUT 19

#define M641_FADE_IN_STATE_SECOND_INPUT 20

#define M641_FADE_IN_STATE_FINAL_WAIT 21

#define M641_FADE_IN_STATE_COMPLETE 22

#define M641_FADE_IN_STICK_INPUT 44

#define M641_RESULT_SEQUENCE_STATE_WAIT_FOR_MOVEMENT 14

#define M641_RESULT_SEQUENCE_STATE_POSITION_WITH_TEAMMATE 15

#define M641_RESULT_SEQUENCE_STATE_AFTER_POSITIONING 16

#define M641_RESULT_SEQUENCE_STATE_FINISHED 17

#define M641_COLLISION_RECOVERY_STATE_RESULT_SEQUENCE 23

u32 lbl_1_data_2A8[13] = {
    DATANUM(DATA_mariomot, 0),  DATANUM(DATA_mariomot, 1),  DATANUM(DATA_mariomot, 2),
    DATANUM(DATA_mariomot, 3),  DATANUM(DATA_mariomot, 4),  DATANUM(DATA_mariomot, 8),
    DATANUM(DATA_mariomot, 10), DATANUM(DATA_mariomot, 9),  DATANUM(DATA_mariomot, 20),
    DATANUM(DATA_mariomot, 22), DATANUM(DATA_mario, 28),    DATANUM(DATA_mariomot, 6),
    DATANUM(DATA_mariomot, 40)
};

Point3d lbl_1_data_2DC[2][4] = {
    {{-140.0f, 0.0f, 0.0f}, {140.0f, 0.0f, 0.0f}, {-140.0f, 0.0f, 0.0f}, {140.0f, 0.0f, 0.0f}},
    {{-280.0f, 0.0f, 0.0f}, {280.0f, 0.0f, 0.0f}, {-280.0f, 0.0f, 0.0f}, {280.0f, 0.0f, 0.0f}}
};

f32 lbl_1_data_33C[2] = {-140.0f, 140.0f};

M641MotionInfo lbl_1_data_344[14] = {
    {0, 0, 0.0f, HU3D_MOTATTR_LOOP, 1},
    {0, 0, 8.0f, HU3D_MOTATTR_LOOP, 1},
    {1, 0, 8.0f, HU3D_MOTATTR_LOOP, 1},
    {2, 0, 8.0f, HU3D_MOTATTR_LOOP, 1},
    {3, 0, 8.0f, HU3D_MOTATTR_LOOP, 1},
    {4, 0, 0.0f, HU3D_MOTATTR_LOOP, 1},
    {5, 0, 0.0f, HU3D_MOTATTR_LOOP, 1},
    {6, 0, 0.0f, HU3D_MOTATTR_LOOP, 1},
    {7, 0, 0.0f, HU3D_MOTATTR_LOOP, 1},
    {8, 0, 8.0f, HU3D_MOTATTR_LOOP, 0},
    {9, 0, 8.0f, HU3D_MOTATTR_LOOP, 1},
    {10, 0, 4.0f, HU3D_MOTATTR_LOOP, 1},
    {11, 0, 8.0f, HU3D_MOTATTR_LOOP, 1},
    {12, 0, 8.0f, HU3D_MOTATTR_LOOP, 1}
};

s16 lbl_1_data_424[4] = {0, 0, 0, 0};

extern s16 lbl_1_bss_7DE;

extern s16 lbl_1_bss_7DC;

extern s16 lbl_1_data_228[6][2];

/* The init, fade-out, and winner hooks call this to select a character motion and apply its blend,
 * model-attribute, and voice settings. */
void fn_1_9F7C(s16 playerIndex, s16 motionIndex) {
    s16 characterMotionIndex;

    if ((playerIndex < 0) || (playerIndex >= 4) || (motionIndex < 0) ||
        (motionIndex >= M641_MOTION_COUNT)) {
        return;
    }
    if (motionIndex != lbl_1_bss_590[playerIndex].currentMotion) {
        characterMotionIndex = lbl_1_data_344[motionIndex].motionId;
        if (characterMotionIndex != -1) {
            if (lbl_1_data_344[motionIndex].blendTime <= 1.0f) {
                CharMotionSet(lbl_1_bss_590[playerIndex].characterNo,
                              lbl_1_bss_590[playerIndex].motionIds[characterMotionIndex]);
                Hu3DModelAttrReset(
                    lbl_1_bss_590[playerIndex].modelId,
                    Hu3DModelMotionAttrGet(lbl_1_bss_590[playerIndex].modelId) | HU3D_MOTATTR);
                CharModelAttrSet(
                    lbl_1_bss_590[playerIndex].characterNo, lbl_1_data_344[motionIndex].attributes);
            } else {
                Hu3DModelAttrSet(
                    lbl_1_bss_590[playerIndex].modelId, HU3D_MOTATTR_PAUSE);
                CharMotionShiftSet(
                    lbl_1_bss_590[playerIndex].characterNo,
                    lbl_1_bss_590[playerIndex].motionIds[characterMotionIndex],
                    0.0f,
                    lbl_1_data_344[motionIndex].blendTime,
                    lbl_1_data_344[motionIndex].attributes);
            }
            CharModelVoiceFlagSet(lbl_1_bss_590[playerIndex].characterNo,
                                  lbl_1_data_344[motionIndex].voiceEnabled);
        }
        lbl_1_bss_590[playerIndex].currentMotion = motionIndex;
    }
}

void fn_1_19C(void);

void fn_1_1C0(s16 mode, s16 frameNo);

void fn_1_1F4(s16 mode, s16 frameNo);

void fn_1_1F8(s16 mode, s16 frameNo);

void fn_1_254(s16 mode, s16 frameNo);

void fn_1_280(s16 mode, s16 frameNo);

void fn_1_2F0(s16 mode, s16 frameNo);

void fn_1_31C(s16 mode, s16 frameNo);

void fn_1_320(s16 mode, s16 frameNo);

void fn_1_10278(void);

void fn_1_1403C(s16 groupIndex, s16 playerIndex, s32 amount);

f32 fn_1_1D80();

f32 fn_1_1DB4();

f32 fn_1_1DC4();

s32 fn_1_540(Point3d *first, f32 firstRadius, f32 firstExtent, Point3d *firstDisplacement,
             Point3d *second, f32 secondRadius, f32 secondExtent, Point3d *secondDisplacement);

s16 fn_1_8400(s16 boardIndex, s16 participant, Point3d *position, Point3d *displacement, f32 radius,
              f32 height, s16 *kind, Point3d *contact);

void fn_1_9EC8(s16 effectId, Point3d *position);

void fn_1_C368(s16 playerIndex);

s16 fn_1_A204(s16 playerIndex) {
    return lbl_1_bss_590[playerIndex].groupNo;
}
void fn_1_B108(void);

void fn_1_11BAC(void);

void fn_1_1155C(void);

static inline void M641DebugSound(s32 handle) { }

extern void fn_1_B108(void);

extern void fn_1_11AD8(void);

extern f32 lbl_1_bss_20;

/* The init hook builds the four player records, character models, motions, render layers, and
 * lights. */
void fn_1_A220(void) {
    Point3d lightPosition;
    Point3d lightDirection;
    GXColor lightColor;
    s16 boardPlayerCount;
    s16 guestPlayerCount;
    s16 boardPositionIndex;
    s16 guestPositionIndex;
    s32 playerIndex;
    s32 motionIndex;
    s16 lightId;

    if (lbl_1_bss_7DC == 0) {
        Hu3DZClearLayerSet(3);
    } else {
        Hu3DZClearLayerSet(2);
        Hu3DZClearLayerSet(3);
    }
    boardPositionIndex = guestPositionIndex = boardPlayerCount = guestPlayerCount = 0;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        lbl_1_bss_590[playerIndex].playerSlot = playerIndex;
        lbl_1_bss_590[playerIndex].characterNo = GwPlayerConf[playerIndex].charNo;
        lbl_1_bss_590[playerIndex].padNo = GwPlayerConf[playerIndex].padNo;
        lbl_1_bss_590[playerIndex].groupNo = GwPlayerConf[playerIndex].grpNo;
        if (lbl_1_bss_590[playerIndex].groupNo == 0) {
            lbl_1_bss_590[playerIndex].groupNo = 0;
        } else {
            lbl_1_bss_590[playerIndex].groupNo = 1;
        }
        lbl_1_bss_590[playerIndex].inputSamplingGate = GwPlayerConf[playerIndex].type;
        lbl_1_bss_590[playerIndex].computerDifficulty = GwPlayerConf[playerIndex].comDif;
        lbl_1_bss_590[playerIndex].movementState = 1;
        lbl_1_bss_590[playerIndex].fadeInState = M641_FADE_IN_STATE_INITIAL_WAIT;
        lbl_1_bss_590[playerIndex].resultSequenceState = -1;
        lbl_1_bss_590[playerIndex].velocity.x =
            lbl_1_bss_590[playerIndex].velocity.y =
                lbl_1_bss_590[playerIndex].velocity.z = 0.0f;
        lbl_1_bss_590[playerIndex].jumpVelocity = lbl_1_bss_590[playerIndex].velocity;
        lbl_1_bss_590[playerIndex].tiltState = 0;
        lbl_1_bss_590[playerIndex].jumpDescentDelayFrames = 0;
        lbl_1_bss_590[playerIndex].collisionRecoveryState =
            M641_COLLISION_RECOVERY_STATE_RESULT_SEQUENCE;
        lbl_1_bss_590[playerIndex].modelId =
            CharModelCreate(lbl_1_bss_590[playerIndex].characterNo, 8);
        for (motionIndex = 0; motionIndex < M641_CHARACTER_MOTION_COUNT; motionIndex++) {
            lbl_1_bss_590[playerIndex].motionIds[motionIndex] = CharMotionCreate(
                lbl_1_bss_590[playerIndex].characterNo, lbl_1_data_2A8[motionIndex]);
        }
        CharMotionDataClose(lbl_1_bss_590[playerIndex].characterNo);
        lbl_1_bss_590[playerIndex].currentMotion = -1;
        fn_1_9F7C(playerIndex, 0);
        lbl_1_bss_590[playerIndex].movementTimer = 0;
        if (lbl_1_bss_590[playerIndex].groupNo == 0) {
            Hu3DModelCameraSet(lbl_1_bss_590[playerIndex].modelId, 1);
        } else {
            Hu3DModelCameraSet(lbl_1_bss_590[playerIndex].modelId, 2);
        }
        if (lbl_1_bss_7DC == 0) {
            if (lbl_1_bss_590[playerIndex].groupNo == 0) {
                if (boardPlayerCount == 0) {
                    Hu3DModelLayerSet(lbl_1_bss_590[playerIndex].modelId, 2);
                } else {
                    Hu3DModelLayerSet(lbl_1_bss_590[playerIndex].modelId, 3);
                }
                boardPlayerCount++;
            } else {
                if (guestPlayerCount == 0) {
                    Hu3DModelLayerSet(lbl_1_bss_590[playerIndex].modelId, 2);
                } else {
                    Hu3DModelLayerSet(lbl_1_bss_590[playerIndex].modelId, 3);
                }
                guestPlayerCount++;
            }
        } else {
            if (lbl_1_bss_590[playerIndex].groupNo == 0) {
                if (boardPlayerCount == 0) {
                    Hu3DModelLayerSet(lbl_1_bss_590[playerIndex].modelId, 2);
                } else {
                    Hu3DModelLayerSet(lbl_1_bss_590[playerIndex].modelId, 3);
                }
                boardPlayerCount++;
            } else {
                if (guestPlayerCount == 0) {
                    Hu3DModelLayerSet(lbl_1_bss_590[playerIndex].modelId, 2);
                } else {
                    Hu3DModelLayerSet(lbl_1_bss_590[playerIndex].modelId, 3);
                }
                guestPlayerCount++;
            }
        }
        if (lbl_1_bss_7DC == 0) {
            Hu3DModelShadowSet(lbl_1_bss_590[playerIndex].modelId);
        }
        lbl_1_bss_590[playerIndex].collisionRadius = 40.0f;
        lbl_1_bss_590[playerIndex].characterHeight =
            CharModelHeightGet(lbl_1_bss_590[playerIndex].characterNo);
        if (lbl_1_bss_7DC == 0) {
            motionIndex = 0;
        } else {
            motionIndex = 1;
        }
        if (lbl_1_bss_590[playerIndex].groupNo == 0) {
            lbl_1_bss_590[playerIndex].position = lbl_1_data_2DC[motionIndex][boardPositionIndex];
            boardPositionIndex++;
        } else {
            lbl_1_bss_590[playerIndex].position =
                lbl_1_data_2DC[motionIndex][guestPositionIndex + 2];
            guestPositionIndex++;
        }
        lbl_1_bss_590[playerIndex].position.y = lbl_1_bss_20;
        Hu3DModelPosSetV(lbl_1_bss_590[playerIndex].modelId, &lbl_1_bss_590[playerIndex].position);
        lbl_1_bss_590[playerIndex].rotation.x =
            lbl_1_bss_590[playerIndex].rotation.y =
                lbl_1_bss_590[playerIndex].rotation.z = 0.0f;
        Hu3DModelRotSetV(lbl_1_bss_590[playerIndex].modelId, &lbl_1_bss_590[playerIndex].rotation);
        lbl_1_bss_590[playerIndex].stickX = lbl_1_bss_590[playerIndex].stickY = 0;
        lbl_1_bss_590[playerIndex].buttons = lbl_1_bss_590[playerIndex].buttonsDown = 0;
        lbl_1_bss_590[playerIndex].heldButtonFlags =
            lbl_1_bss_590[playerIndex].jumpInputFlags = 0;
    }
    if (lbl_1_bss_7DC == 0) {
        CharEffectLayerSet(4);
    } else {
        CharEffectLayerSet(4);
    }
    if (lbl_1_bss_7DC != 0) {
        lightPosition.x = 0.0f;
        lightPosition.y = -10000.0f;
        lightPosition.z = -2000.0f;
        lightDirection.x = 0.0f;
        lightDirection.y = 1.0f;
        lightDirection.z = 0.2f;
        lightColor.r = lightColor.g = 224;
        lightColor.b = 0;
        lightColor.a = 255;
        lightId = CharLightCreateV(&lightPosition, &lightDirection, &lightColor);
    }
    fn_1_11AD8();
}

/* The fade-in hook advances night-mode player states, updates movement, and reports when all four
 * players finish; day mode marks them finished immediately. */
s32 fn_1_ACE0(s16 frame) {
    s16 positiveCount;
    s16 negativeCount;
    s16 playerIndex;

    positiveCount = negativeCount = 0;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_7DC == 0) {
            lbl_1_bss_590[playerIndex].fadeInState = M641_FADE_IN_STATE_COMPLETE;
        } else {
            switch (lbl_1_bss_590[playerIndex].fadeInState) {
            case M641_FADE_IN_STATE_INITIAL_WAIT:
                if (frame >= 60) {
                    lbl_1_bss_590[playerIndex].fadeInState = M641_FADE_IN_STATE_FIRST_INPUT;
                }
                break;
            case M641_FADE_IN_STATE_FIRST_INPUT:
                if (lbl_1_bss_590[playerIndex].groupNo == 0) {
                    if (positiveCount == 0) {
                        lbl_1_bss_590[playerIndex].stickX = M641_FADE_IN_STICK_INPUT;
                    } else {
                        lbl_1_bss_590[playerIndex].stickX = -M641_FADE_IN_STICK_INPUT;
                    }
                    positiveCount++;
                } else {
                    if (negativeCount == 0) {
                        lbl_1_bss_590[playerIndex].stickX = M641_FADE_IN_STICK_INPUT;
                    } else {
                        lbl_1_bss_590[playerIndex].stickX = -M641_FADE_IN_STICK_INPUT;
                    }
                    negativeCount++;
                }
                if (frame >= 68) {
                    lbl_1_bss_590[playerIndex].buttonsDown = PAD_BUTTON_A;
                    lbl_1_bss_590[playerIndex].buttons = PAD_BUTTON_A;
                    lbl_1_bss_590[playerIndex].fadeInState = M641_FADE_IN_STATE_SECOND_INPUT;
                }
                break;
            case M641_FADE_IN_STATE_SECOND_INPUT:
                if (lbl_1_bss_590[playerIndex].groupNo == 0) {
                    if (positiveCount == 0) {
                        lbl_1_bss_590[playerIndex].stickX = M641_FADE_IN_STICK_INPUT;
                    } else {
                        lbl_1_bss_590[playerIndex].stickX = -M641_FADE_IN_STICK_INPUT;
                    }
                    positiveCount++;
                } else {
                    if (negativeCount == 0) {
                        lbl_1_bss_590[playerIndex].stickX = M641_FADE_IN_STICK_INPUT;
                    } else {
                        lbl_1_bss_590[playerIndex].stickX = -M641_FADE_IN_STICK_INPUT;
                    }
                    negativeCount++;
                }
                lbl_1_bss_590[playerIndex].buttons = PAD_BUTTON_A;
                if (frame >= 90) {
                    lbl_1_bss_590[playerIndex].buttonsDown = PAD_BUTTON_A;
                    lbl_1_bss_590[playerIndex].fadeInState = M641_FADE_IN_STATE_FINAL_WAIT;
                }
                break;
            case M641_FADE_IN_STATE_FINAL_WAIT:
                if (frame >= 150) {
                    lbl_1_bss_590[playerIndex].fadeInState = M641_FADE_IN_STATE_COMPLETE;
                }
                break;
            case M641_FADE_IN_STATE_COMPLETE:
            default:
                lbl_1_bss_590[playerIndex].fadeInState = M641_FADE_IN_STATE_COMPLETE;
                break;
            }
        }
    }
    fn_1_B108();
    playerIndex = 0;
    while (playerIndex < 4) {
        if (lbl_1_bss_590[playerIndex].fadeInState != M641_FADE_IN_STATE_COMPLETE) {
            break;
        }
        playerIndex++;
    }
    if (playerIndex >= 4) {
        return 1;
    }
    return 0;
}

void fn_1_B0C0(s16 mode, s16 frameNo)
{
    fn_1_B108();
}

/* The main-phase callback samples input, updates routes, and advances player movement each
 * frame. */
void fn_1_B0E0(void)
{
    fn_1_11BAC();
    fn_1_1155C();
    fn_1_B108();
}

/* Start, fade-in, main, finish, and pre-winner updates call this to advance all players' movement
 * and collision states. */
void fn_1_B108(void) {
    Point3d velocity;
    Point3d otherVelocity;
    Point3d position;
    Point3d otherPosition;
    s16 collisionFlags[4][2] = { 0 };
    Point3d firstCopy;
    Point3d secondCopy;
    Point3d boardPosition;
    Point3d firstHitPosition;
    Point3d secondHitPosition;
    Point3d thirdHitPosition;
    Point3d fourthHitPosition;
    s16 collisionKind;
    f32 scalarValue;
    f32 otherRadius;
    s16 playerIndexOrHitType;
    s16 index;
    Point3d *secondCandidate;
    Point3d *firstCandidate;
    Point3d *boardCandidate;
    Point3d *firstHit;
    Point3d *secondHit;
    Point3d *thirdHit;
    Point3d *fourthHit;

    for (index = 0; index < 4; index++) {
        fn_1_C368(index);
    }
    for (index = 0; index < 2; index++) {
        for (collisionKind = 0; collisionKind < 4; collisionKind++) {
            if (index == lbl_1_bss_590[collisionKind].groupNo) {
                position = lbl_1_bss_590[collisionKind].position;
                velocity = lbl_1_bss_590[collisionKind].velocity;
                /* Use the current player's collision radius for this overlap check. */
                scalarValue = lbl_1_bss_590[collisionKind].collisionRadius;
                if (lbl_1_bss_590[collisionKind].movementState == 3) {
                    position.y += lbl_1_bss_590[collisionKind].characterHeight - 150.0f;
                }
                for (playerIndexOrHitType = 0; playerIndexOrHitType < 4; playerIndexOrHitType++) {
                    if (index == lbl_1_bss_590[playerIndexOrHitType].groupNo &&
                        collisionKind != playerIndexOrHitType) {
                        otherPosition = lbl_1_bss_590[playerIndexOrHitType].position;
                        otherVelocity = lbl_1_bss_590[playerIndexOrHitType].velocity;
                        otherRadius = lbl_1_bss_590[playerIndexOrHitType].collisionRadius;
                        if (lbl_1_bss_590[playerIndexOrHitType].movementState == 3) {
                            otherPosition.y +=
                                lbl_1_bss_590[playerIndexOrHitType].characterHeight - 150.0f;
                        }
                        secondCopy = otherPosition;
                        secondCandidate = &secondCopy;
                        firstCopy = position;
                        firstCandidate = &firstCopy;
                        if (fn_1_540(firstCandidate, scalarValue, 150.0f, &velocity,
                                     secondCandidate, otherRadius, 150.0f,
                                     &otherVelocity) != 0) {
                            lbl_1_bss_590[collisionKind].velocity = velocity;
                            lbl_1_bss_590[playerIndexOrHitType].velocity = otherVelocity;
                            collisionFlags[collisionKind][0] = 1;
                            collisionFlags[playerIndexOrHitType][0] = 1;
                        }
                    }
                }
            }
        }
    }
    index = 0;
    while (index < 4) {
        velocity = lbl_1_bss_590[index].velocity;
        boardPosition = lbl_1_bss_590[index].position;
        boardCandidate = &boardPosition;
        /* Now stores the board hit type: -1 none, 0 ordinary, 1 item, or 2 special. */
        playerIndexOrHitType =
            fn_1_8400(lbl_1_bss_590[index].groupNo, index, boardCandidate, &velocity,
                      lbl_1_bss_590[index].collisionRadius, lbl_1_bss_590[index].characterHeight,
                      &collisionKind, &otherVelocity);
        if (playerIndexOrHitType == 0) {
            lbl_1_bss_590[index].velocity = velocity;
            lbl_1_bss_590[index].jumpVelocity.x = lbl_1_bss_590[index].jumpVelocity.y =
                lbl_1_bss_590[index].jumpVelocity.z = 0.0f;
            if (lbl_1_bss_7DC == 0) {
                fn_1_1403C(lbl_1_bss_590[index].groupNo, index, collisionKind);
                firstHitPosition = otherVelocity;
                firstHit = &firstHitPosition;
                fn_1_9EC8(lbl_1_bss_590[index].groupNo, firstHit);
            } else if ((lbl_1_bss_590[index].movementState == 5) ||
                       (lbl_1_bss_590[index].movementState == 6) ||
                       (lbl_1_bss_590[index].movementState == 7)) {
                fn_1_1403C(lbl_1_bss_590[index].groupNo, index, collisionKind);
                secondHitPosition = otherVelocity;
                secondHit = &secondHitPosition;
                fn_1_9EC8(lbl_1_bss_590[index].groupNo, secondHit);
            } else if ((lbl_1_bss_590[index].movementState != 8) &&
                       (lbl_1_bss_590[index].movementState != 9) &&
                       (lbl_1_bss_590[index].movementState != M641_MOVEMENT_STATE_HIT_REACTION) &&
                       (lbl_1_bss_590[index].movementState !=
                        M641_MOVEMENT_STATE_COLLISION_LAUNCH) &&
                       (lbl_1_bss_590[index].movementState !=
                        M641_MOVEMENT_STATE_COLLISION_AIRBORNE) &&
                       (lbl_1_bss_590[index].movementState !=
                        M641_MOVEMENT_STATE_COLLISION_LANDING)) {
                lbl_1_bss_590[index].movementState = M641_MOVEMENT_STATE_COLLISION_LAUNCH;
                omVibrate(index, 20, 7, 3);
            }
        } else if (playerIndexOrHitType == 1) {
            lbl_1_bss_590[index].velocity = velocity;
            lbl_1_bss_590[index].jumpVelocity.x = lbl_1_bss_590[index].jumpVelocity.y =
                lbl_1_bss_590[index].jumpVelocity.z = 0.0f;
            if (lbl_1_bss_7DC == 0) {
                lbl_1_bss_590[index].movementState = 8;
                omVibrate(index, 20, 7, 3);
                /* Map the player's board position between the x limits to the hit-sound pan. */
                scalarValue = fn_1_1DC4() - fn_1_1DB4();
                scalarValue = (lbl_1_bss_590[index].position.x - fn_1_1DB4()) / scalarValue;
                scalarValue = 32.0f + 64.0f * scalarValue;
            M641DebugSound(
                CharFXPlayPan(lbl_1_bss_590[index].characterNo, CHARVOICEID(9), (s16)scalarValue));
            } else if ((lbl_1_bss_590[index].movementState != 8) &&
                       (lbl_1_bss_590[index].movementState != 9) &&
                       (lbl_1_bss_590[index].movementState != M641_MOVEMENT_STATE_HIT_REACTION) &&
                       (lbl_1_bss_590[index].movementState !=
                        M641_MOVEMENT_STATE_COLLISION_LAUNCH) &&
                       (lbl_1_bss_590[index].movementState !=
                        M641_MOVEMENT_STATE_COLLISION_AIRBORNE) &&
                       (lbl_1_bss_590[index].movementState !=
                        M641_MOVEMENT_STATE_COLLISION_LANDING)) {
                lbl_1_bss_590[index].collisionRecoveryState = 24;
                lbl_1_bss_590[index].movementState = M641_MOVEMENT_STATE_COLLISION_LAUNCH;
                omVibrate(index, 20, 7, 3);
                /* Map the player's board position between the x limits to the hit-sound pan. */
                scalarValue = fn_1_1DC4() - fn_1_1DB4();
                scalarValue = (lbl_1_bss_590[index].position.x - fn_1_1DB4()) / scalarValue;
                scalarValue = 32.0f + 64.0f * scalarValue;
                M641DebugSound(CharFXPlayPan(lbl_1_bss_590[index].characterNo, CHARVOICEID(9),
                                             (s16) scalarValue));
            }
        } else if (playerIndexOrHitType == 2) {
            lbl_1_bss_590[index].velocity = velocity;
            lbl_1_bss_590[index].jumpVelocity.x = lbl_1_bss_590[index].jumpVelocity.y =
                lbl_1_bss_590[index].jumpVelocity.z = 0.0f;
            if (lbl_1_bss_7DC == 0) {
                fn_1_1403C(lbl_1_bss_590[index].groupNo, index, collisionKind);
                thirdHitPosition = otherVelocity;
                thirdHit = &thirdHitPosition;
                fn_1_9EC8(lbl_1_bss_590[index].groupNo, thirdHit);
            } else if ((lbl_1_bss_590[index].movementState == 5) ||
                       (lbl_1_bss_590[index].movementState == 6) ||
                       (lbl_1_bss_590[index].movementState == 7)) {
                fn_1_1403C(lbl_1_bss_590[index].groupNo, index, collisionKind);
                fourthHitPosition = otherVelocity;
                fourthHit = &fourthHitPosition;
                fn_1_9EC8(lbl_1_bss_590[index].groupNo, fourthHit);
            } else if ((lbl_1_bss_590[index].movementState != 8) &&
                       (lbl_1_bss_590[index].movementState != 9) &&
                       (lbl_1_bss_590[index].movementState != M641_MOVEMENT_STATE_HIT_REACTION) &&
                       (lbl_1_bss_590[index].movementState !=
                        M641_MOVEMENT_STATE_COLLISION_LAUNCH) &&
                       (lbl_1_bss_590[index].movementState !=
                        M641_MOVEMENT_STATE_COLLISION_AIRBORNE) &&
                       (lbl_1_bss_590[index].movementState !=
                        M641_MOVEMENT_STATE_COLLISION_LANDING)) {
                lbl_1_bss_590[index].movementState = M641_MOVEMENT_STATE_COLLISION_LAUNCH;
                omVibrate(index, 20, 7, 3);
            }
        }
        index += 1;
    }
    index = 0;
    while (index < 4) {
        /* Use the floor-height limit for the character height check. */
        scalarValue = fn_1_1D80();
        PSVECAdd(&lbl_1_bss_590[index].position, &lbl_1_bss_590[index].velocity, &velocity);
        if ((velocity.y + lbl_1_bss_590[index].characterHeight) > scalarValue) {
            if (lbl_1_bss_590[index].velocity.y > 0.0f) {
                lbl_1_bss_590[index].velocity.y = 0.0f;
            }
            if (lbl_1_bss_590[index].jumpVelocity.y > 0.0f) {
                lbl_1_bss_590[index].jumpVelocity.y = lbl_1_bss_590[index].jumpVelocity.z = 0.0f;
            }
        }
        /* Compare against the left board x limit. */
        scalarValue = fn_1_1DB4();
        PSVECAdd(&lbl_1_bss_590[index].position, &lbl_1_bss_590[index].velocity, &velocity);
        if (velocity.x < scalarValue) {
            if (lbl_1_bss_590[index].velocity.x < 0.0f) {
                lbl_1_bss_590[index].velocity.x = 0.0f;
            }
            collisionFlags[index][1] = 1;
        }
        /* Compare against the right board x limit. */
        scalarValue = fn_1_1DC4();
        PSVECAdd(&lbl_1_bss_590[index].position, &lbl_1_bss_590[index].velocity, &velocity);
        if (velocity.x > scalarValue) {
            if (lbl_1_bss_590[index].velocity.x > 0.0f) {
                lbl_1_bss_590[index].velocity.x = 0.0f;
            }
            collisionFlags[index][1] = 1;
        }
        index += 1;
    }
    index = 0;
    while (index < 4) {
        if ((collisionFlags[index][0] != 0) && (collisionFlags[index][1] != 0) &&
            (lbl_1_data_424[index] <= 0)) {
            lbl_1_data_424[index] = 6;
        } else if (lbl_1_data_424[index] > 0) {
            lbl_1_data_424[index]--;
        }
        collisionKind = 0;
        while (collisionKind < 4) {
            if ((lbl_1_bss_590[index].groupNo == lbl_1_bss_590[collisionKind].groupNo) &&
                (index != collisionKind) && (lbl_1_data_424[index] > 0) &&
                ((lbl_1_bss_590[index].movementState == 5) ||
                 (lbl_1_bss_590[index].movementState == 6) ||
                 (lbl_1_bss_590[index].movementState == 7) ||
                 (lbl_1_bss_590[index].movementState == 2) ||
                 (lbl_1_bss_590[index].movementState == 3) ||
                 (lbl_1_bss_590[index].movementState == 4))) {
                if (lbl_1_bss_590[index].position.x > 0.0f) {
                    lbl_1_bss_590[collisionKind].velocity.x = -16.0f;
                } else {
                    lbl_1_bss_590[collisionKind].velocity.x = 16.0f;
                }
            }
            collisionKind += 1;
        }
        index += 1;
    }
    fn_1_10278();
}

void fn_1_FE64(s16);

void fn_1_FF1C(s16);

/* Fade-in, main, finish, pre-winner, and start updates use this on motion changes to set the
 * animation, attributes, and voice cues. */
static inline void M641MotionChange(s16 playerIndex, s16 motionIndex) {
    s16 motionId;

    if (playerIndex < 0 || playerIndex >= 4 || motionIndex < 0 || motionIndex >= 14) {
        return;
    }
    if (motionIndex == lbl_1_bss_590[playerIndex].currentMotion) {
        return;
    }
    motionId = lbl_1_data_344[motionIndex].motionId;
    if (motionId != -1) {
        if (lbl_1_data_344[motionIndex].blendTime <= 1.0f) {
            CharMotionSet(lbl_1_bss_590[playerIndex].characterNo,
                          lbl_1_bss_590[playerIndex].motionIds[motionId]);
            Hu3DModelAttrReset(lbl_1_bss_590[playerIndex].modelId,
                               Hu3DModelMotionAttrGet(lbl_1_bss_590[playerIndex].modelId) |
                                   HU3D_MOTATTR);
            CharModelAttrSet(lbl_1_bss_590[playerIndex].characterNo,
                             lbl_1_data_344[motionIndex].attributes);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_590[playerIndex].modelId, HU3D_MOTATTR_PAUSE);
            CharMotionShiftSet(lbl_1_bss_590[playerIndex].characterNo,
                               lbl_1_bss_590[playerIndex].motionIds[motionId], 0.0f,
                               lbl_1_data_344[motionIndex].blendTime,
                               lbl_1_data_344[motionIndex].attributes);
        }
        CharModelVoiceFlagSet(lbl_1_bss_590[playerIndex].characterNo,
                              lbl_1_data_344[motionIndex].voiceEnabled);
    }
    lbl_1_bss_590[playerIndex].currentMotion = motionIndex;
}

/* The fade-in and main hooks call this per player to advance input, movement, and collision
 * state. */
void fn_1_C368(s16 playerIndex) {
    Point3d velocity;
    f32 motionMax;
    s16 threshold;
    s16 stickSquare;

    switch (lbl_1_bss_590[playerIndex].movementState) {
        case 1:
            fn_1_FE64(playerIndex);
            fn_1_FF1C(playerIndex);
            velocity = lbl_1_bss_590[playerIndex].velocity;
            lbl_1_bss_590[playerIndex].velocity.x = lbl_1_bss_590[playerIndex].velocity.y =
                lbl_1_bss_590[playerIndex].velocity.z = 0.0f;
            if ((velocity.x < 0.0f) && (lbl_1_bss_590[playerIndex].rotation.y <= -90.0f)) {
                lbl_1_bss_590[playerIndex].velocity = velocity;
            }
            if ((velocity.x > 0.0f) && (lbl_1_bss_590[playerIndex].rotation.y >= 90.0f)) {
                lbl_1_bss_590[playerIndex].velocity = velocity;
            }
            if ((lbl_1_bss_590[playerIndex].currentMotion != M641_MOTION_INDEX_WIN) &&
                (lbl_1_bss_590[playerIndex].currentMotion != M641_MOTION_INDEX_LOSS)) {
                if (lbl_1_bss_590[playerIndex].stickX != 0) {
                    threshold = 28;
                    threshold = threshold * threshold;
                    stickSquare =
                        lbl_1_bss_590[playerIndex].stickX * lbl_1_bss_590[playerIndex].stickX;
                    if ((stickSquare < threshold) &&
                        (lbl_1_bss_590[playerIndex].movementTimer == 0)) {
                        M641MotionChange(playerIndex, 2);
                        lbl_1_bss_590[playerIndex].movementTimer = 8;
                    }
                    if ((stickSquare >= threshold) &&
                        (lbl_1_bss_590[playerIndex].movementTimer == 0)) {
                        M641MotionChange(playerIndex, 3);
                        lbl_1_bss_590[playerIndex].movementTimer = 8;
                    }
                } else if (lbl_1_bss_590[playerIndex].movementTimer == 0) {
                    M641MotionChange(playerIndex, 1);
                    lbl_1_bss_590[playerIndex].movementTimer = 8;
                }
                lbl_1_bss_590[playerIndex].movementTimer -= 1;
                if (lbl_1_bss_590[playerIndex].movementTimer <= 0) {
                    lbl_1_bss_590[playerIndex].movementTimer = 0;
                }
            }
            if ((lbl_1_bss_590[playerIndex].buttonsDown & 0x100) != 0) {
                lbl_1_bss_590[playerIndex].movementState = 2;
                velocity.y = velocity.z = 0.0f;
                velocity.x = (f32) lbl_1_bss_590[playerIndex].stickX / 56.0f;
                velocity.x *= 8.0f;
                lbl_1_bss_590[playerIndex].jumpVelocity = velocity;
            }
            lbl_1_bss_590[playerIndex].jumpVelocity.y -= 2.0f;
            if ((lbl_1_bss_590[playerIndex].position.y + lbl_1_bss_590[playerIndex].velocity.y) >
                lbl_1_bss_20) {
                lbl_1_bss_590[playerIndex].velocity.y = lbl_1_bss_590[playerIndex].jumpVelocity.y;
            }
            if ((lbl_1_bss_590[playerIndex].position.y + lbl_1_bss_590[playerIndex].velocity.y) <
                lbl_1_bss_20) {
                lbl_1_bss_590[playerIndex].velocity.y =
                    lbl_1_bss_20 - lbl_1_bss_590[playerIndex].position.y;
                lbl_1_bss_590[playerIndex].jumpVelocity.y = 0.0f;
                return;
            }
            break;
        case 2:
            if (lbl_1_bss_590[playerIndex].currentMotion != 4) {
                M641MotionChange(playerIndex, 4);
                lbl_1_bss_590[playerIndex].jumpVelocity.y =
                    (350.0f - lbl_1_bss_590[playerIndex].characterHeight) / 10.0f;
                lbl_1_bss_590[playerIndex].movementState = 3;
            } else {
                lbl_1_bss_590[playerIndex].movementState = 3;
                lbl_1_bss_590[playerIndex].jumpVelocity.y =
                    (350.0f - lbl_1_bss_590[playerIndex].characterHeight) / 10.0f;
            }
            fn_1_FE64(playerIndex);
            fn_1_FF1C(playerIndex);
            lbl_1_bss_590[playerIndex].velocity.x = lbl_1_bss_590[playerIndex].velocity.y =
                lbl_1_bss_590[playerIndex].velocity.z = 0.0f;
            return;
        case 3:
            fn_1_FE64(playerIndex);
            fn_1_FF1C(playerIndex);
            if (lbl_1_bss_590[playerIndex].currentMotion == 4) {
                if (CharMotionShiftIDGet(lbl_1_bss_590[playerIndex].characterNo) < 0) {
                    if ((Hu3DData[lbl_1_bss_590[playerIndex].modelId].motWork.speed +
                         CharMotionTimeGet(lbl_1_bss_590[playerIndex].characterNo)) >=
                        (motionMax =
                             CharMotionMaxTimeGet(lbl_1_bss_590[playerIndex].characterNo))) {
                        CharModelAttrSet(lbl_1_bss_590[playerIndex].characterNo,
                                         HU3D_MOTATTR_PAUSE);
                    }
            }
                }
                if ((lbl_1_bss_590[playerIndex].jumpVelocity.y > 0.0f) &&
                    ((lbl_1_bss_590[playerIndex].position.y - lbl_1_bss_20) < 200.0f) &&
                    ((lbl_1_bss_590[playerIndex].buttons & 0x100) != 0)) {
                    lbl_1_bss_590[playerIndex].jumpVelocity.y =
                        (350.0f - lbl_1_bss_590[playerIndex].characterHeight) / 10.0f;
                }
            lbl_1_bss_590[playerIndex].jumpVelocity.x = lbl_1_bss_590[playerIndex].velocity.x;
            lbl_1_bss_590[playerIndex].velocity.y = lbl_1_bss_590[playerIndex].jumpVelocity.y;
            lbl_1_bss_590[playerIndex].jumpVelocity.y -= 2.0f;
            if (((lbl_1_bss_590[playerIndex].position.y + lbl_1_bss_590[playerIndex].velocity.y) <=
                 lbl_1_bss_20) &&
                (lbl_1_bss_590[playerIndex].position.y <= lbl_1_bss_20)) {
                lbl_1_bss_590[playerIndex].movementState = 4;
                lbl_1_bss_590[playerIndex].jumpVelocity.x =
                    lbl_1_bss_590[playerIndex].jumpVelocity.y =
                        lbl_1_bss_590[playerIndex].jumpVelocity.z = 0.0f;
                lbl_1_bss_590[playerIndex].velocity.y =
                    lbl_1_bss_20 - lbl_1_bss_590[playerIndex].position.y;
            } else if ((((lbl_1_bss_590[playerIndex].buttonsDown & 0x100) != 0) ||
                        (((lbl_1_bss_590[playerIndex].buttonsDown & 0x40) != 0) &&
                         (lbl_1_bss_590[playerIndex].jumpVelocity.y < 0.0f)) ||
                        (((lbl_1_bss_590[playerIndex].buttonsDown & 0x40) != 0) &&
                         ((lbl_1_bss_590[playerIndex].buttons & 0x100) == 0))) &&
                       (lbl_1_bss_7DC == 1)) {
                lbl_1_bss_590[playerIndex].movementState = 5;
                lbl_1_bss_590[playerIndex].jumpVelocity.x =
                    lbl_1_bss_590[playerIndex].jumpVelocity.y =
                        lbl_1_bss_590[playerIndex].jumpVelocity.z = 0.0f;
                lbl_1_bss_590[playerIndex].velocity = lbl_1_bss_590[playerIndex].jumpVelocity;
            }
            lbl_1_bss_590[playerIndex].heldButtonFlags = lbl_1_bss_590[playerIndex].jumpInputFlags =
                0;
            return;
        case 4:
            if (lbl_1_bss_590[playerIndex].currentMotion == 4) {
                M641MotionChange(playerIndex, 5);
                lbl_1_bss_590[playerIndex].heldButtonFlags =
                    lbl_1_bss_590[playerIndex].jumpInputFlags = 0;
                return;
            }
            if (lbl_1_bss_590[playerIndex].currentMotion == 5) {
                if (CharMotionShiftIDGet(lbl_1_bss_590[playerIndex].characterNo) < 0) {
                    motionMax = CharMotionMaxTimeGet(lbl_1_bss_590[playerIndex].characterNo);
                    if ((Hu3DData[lbl_1_bss_590[playerIndex].modelId].motWork.speed +
                         CharMotionTimeGet(lbl_1_bss_590[playerIndex].characterNo)) >= motionMax) {
                        M641MotionChange(playerIndex, 1);
                        lbl_1_bss_590[playerIndex].movementState = 1;
                        lbl_1_bss_590[playerIndex].movementTimer = 0;
                    }
                }
                if ((CharMotionShiftIDGet(lbl_1_bss_590[playerIndex].characterNo) < 0) &&
                    ((Hu3DData[lbl_1_bss_590[playerIndex].modelId].motWork.speed +
                      CharMotionTimeGet(lbl_1_bss_590[playerIndex].characterNo)) >= 5.0) &&
                    (((lbl_1_bss_590[playerIndex].buttonsDown & 0x100) != 0) ||
                     ((lbl_1_bss_590[playerIndex].jumpInputFlags & 0x100) != 0))) {
                    lbl_1_bss_590[playerIndex].movementState = 2;
                    velocity.y = velocity.z = 0.0f;
                    velocity.x = (f32) lbl_1_bss_590[playerIndex].stickX / 56.0f;
                    velocity.x *= 8.0f;
                    lbl_1_bss_590[playerIndex].jumpVelocity = velocity;
                    return;
                }
            }
            break;
        case 5:
            lbl_1_bss_590[playerIndex].jumpVelocity.x = lbl_1_bss_590[playerIndex].jumpVelocity.y =
                lbl_1_bss_590[playerIndex].jumpVelocity.z = 0.0f;
            lbl_1_bss_590[playerIndex].velocity = lbl_1_bss_590[playerIndex].jumpVelocity;
            if (lbl_1_bss_590[playerIndex].currentMotion != 6) {
                M641MotionChange(playerIndex, 6);
                break;
            } else if (CharMotionShiftIDGet(lbl_1_bss_590[playerIndex].characterNo) < 0) {
                motionMax = CharMotionMaxTimeGet(lbl_1_bss_590[playerIndex].characterNo);
                if (Hu3DData[lbl_1_bss_590[playerIndex].modelId].motWork.speed +
                        CharMotionTimeGet(lbl_1_bss_590[playerIndex].characterNo) >=
                    motionMax) {
                    if (lbl_1_bss_590[playerIndex].jumpDescentDelayFrames <= 0) {
                        Hu3DModelAttrSet(lbl_1_bss_590[playerIndex].modelId, HU3D_MOTATTR_PAUSE);
                        lbl_1_bss_590[playerIndex].jumpDescentDelayFrames = 6;
                        return;
                    }
                    lbl_1_bss_590[playerIndex].jumpDescentDelayFrames -= 1;
                    if (lbl_1_bss_590[playerIndex].jumpDescentDelayFrames <= 0) {
                        lbl_1_bss_590[playerIndex].jumpDescentDelayFrames = 0;
                        lbl_1_bss_590[playerIndex].jumpVelocity.y -= 7.0f;
                        lbl_1_bss_590[playerIndex].velocity =
                            lbl_1_bss_590[playerIndex].jumpVelocity;
                        lbl_1_bss_590[playerIndex].movementState = 6;
                        return;
                    }
                }
                }
            break;
        case 6:
            lbl_1_bss_590[playerIndex].jumpVelocity.y -= 2.0f;
            lbl_1_bss_590[playerIndex].velocity = lbl_1_bss_590[playerIndex].jumpVelocity;
            if (((lbl_1_bss_590[playerIndex].position.y + lbl_1_bss_590[playerIndex].velocity.y) <=
                 lbl_1_bss_20) &&
                (lbl_1_bss_590[playerIndex].position.y <= lbl_1_bss_20)) {
                M641MotionChange(playerIndex, 7);
                lbl_1_bss_590[playerIndex].movementState = 7;
                lbl_1_bss_590[playerIndex].jumpVelocity.x =
                    lbl_1_bss_590[playerIndex].jumpVelocity.y =
                        lbl_1_bss_590[playerIndex].jumpVelocity.z = 0.0f;
                lbl_1_bss_590[playerIndex].velocity.y =
                    lbl_1_bss_20 - lbl_1_bss_590[playerIndex].position.y;
                return;
            }
            break;
        case 7:
            if (lbl_1_bss_590[playerIndex].currentMotion == 7) {
                if (CharMotionShiftIDGet(lbl_1_bss_590[playerIndex].characterNo) < 0) {
                    motionMax = CharMotionMaxTimeGet(lbl_1_bss_590[playerIndex].characterNo);
                    if (((Hu3DData[lbl_1_bss_590[playerIndex].modelId].motWork.speed +
                          CharMotionTimeGet(lbl_1_bss_590[playerIndex].characterNo)) >=
                         motionMax)) {
                        M641MotionChange(playerIndex, 8);
                        break;
                    }
                }
            } else if (CharMotionShiftIDGet(lbl_1_bss_590[playerIndex].characterNo) < 0) {
                motionMax = CharMotionMaxTimeGet(lbl_1_bss_590[playerIndex].characterNo);
                if ((Hu3DData[lbl_1_bss_590[playerIndex].modelId].motWork.speed +
                     CharMotionTimeGet(lbl_1_bss_590[playerIndex].characterNo)) >= motionMax) {
                    M641MotionChange(playerIndex, 0);
                    lbl_1_bss_590[playerIndex].movementState = 1;
                    lbl_1_bss_590[playerIndex].movementTimer = 0;
                    return;
                }
            }
            break;
        case 8:
            if (lbl_1_bss_590[playerIndex].currentMotion != 10) {
                M641MotionChange(playerIndex, 10);
                lbl_1_bss_590[playerIndex].movementTimer = 0;
                lbl_1_bss_590[playerIndex].movementState = 9;
                return;
            }
            lbl_1_bss_590[playerIndex].movementTimer = 0;
            lbl_1_bss_590[playerIndex].movementState = 9;
            return;
        case 9:
            lbl_1_bss_590[playerIndex].movementTimer += 1;
            if (lbl_1_bss_590[playerIndex].movementTimer >= 90) {
                M641MotionChange(playerIndex, 11);
                lbl_1_bss_590[playerIndex].movementState = 10;
                lbl_1_bss_590[playerIndex].movementTimer = 0;
            }
            lbl_1_bss_590[playerIndex].jumpVelocity.y -= 2.0f;
            if ((lbl_1_bss_590[playerIndex].position.y + lbl_1_bss_590[playerIndex].velocity.y) >
                lbl_1_bss_20) {
                lbl_1_bss_590[playerIndex].velocity.y = lbl_1_bss_590[playerIndex].jumpVelocity.y;
            }
            if ((lbl_1_bss_590[playerIndex].position.y + lbl_1_bss_590[playerIndex].velocity.y) <
                lbl_1_bss_20) {
                lbl_1_bss_590[playerIndex].velocity.y =
                    lbl_1_bss_20 - lbl_1_bss_590[playerIndex].position.y;
                lbl_1_bss_590[playerIndex].jumpVelocity.y = 0.0f;
                return;
            }
            break;
        case 10:
            if (lbl_1_bss_590[playerIndex].currentMotion == 11) {
                if (CharMotionShiftIDGet(lbl_1_bss_590[playerIndex].characterNo) < 0) {
                    motionMax = CharMotionMaxTimeGet(lbl_1_bss_590[playerIndex].characterNo);
                    if ((Hu3DData[lbl_1_bss_590[playerIndex].modelId].motWork.speed +
                         CharMotionTimeGet(lbl_1_bss_590[playerIndex].characterNo)) >= motionMax) {
                        M641MotionChange(playerIndex, 1);
                        lbl_1_bss_590[playerIndex].collisionRecoveryState = 23;
                    }
                }
            } else if ((lbl_1_bss_590[playerIndex].currentMotion == 1) &&
                       (CharMotionShiftIDGet(lbl_1_bss_590[playerIndex].characterNo) < 0)) {
                lbl_1_bss_590[playerIndex].movementState = 1;
                lbl_1_bss_590[playerIndex].movementTimer = 0;
                lbl_1_bss_590[playerIndex].collisionRecoveryState = 23;
            }
            lbl_1_bss_590[playerIndex].jumpVelocity.y -= 2.0f;
            if ((lbl_1_bss_590[playerIndex].position.y + lbl_1_bss_590[playerIndex].velocity.y) >
                lbl_1_bss_20) {
                lbl_1_bss_590[playerIndex].velocity.y = lbl_1_bss_590[playerIndex].jumpVelocity.y;
            }
            if ((lbl_1_bss_590[playerIndex].position.y + lbl_1_bss_590[playerIndex].velocity.y) <
                lbl_1_bss_20) {
                lbl_1_bss_590[playerIndex].velocity.y =
                    lbl_1_bss_20 - lbl_1_bss_590[playerIndex].position.y;
                lbl_1_bss_590[playerIndex].jumpVelocity.y = 0.0f;
                return;
            }
            break;
        case 11:
            if ((lbl_1_bss_590[playerIndex].currentMotion != 9)) {
M641MotionChange(playerIndex, 9);
}
lbl_1_bss_590[playerIndex].velocity.x = lbl_1_bss_590[playerIndex].velocity.y =
    lbl_1_bss_590[playerIndex].velocity.z = 0.0f;
lbl_1_bss_590[playerIndex].jumpVelocity.x = lbl_1_bss_590[playerIndex].jumpVelocity.z = 0.0f;
lbl_1_bss_590[playerIndex].jumpVelocity.y = 15.0f;
                lbl_1_bss_590[playerIndex].movementState = M641_MOVEMENT_STATE_COLLISION_AIRBORNE;
return;
        case 12:
            lbl_1_bss_590[playerIndex].velocity = lbl_1_bss_590[playerIndex].jumpVelocity;
            lbl_1_bss_590[playerIndex].jumpVelocity.y -= 1.0f;
            if ((lbl_1_bss_590[playerIndex].position.y + lbl_1_bss_590[playerIndex].velocity.y) <=
                lbl_1_bss_20) {
                lbl_1_bss_590[playerIndex].movementState = M641_MOVEMENT_STATE_COLLISION_LANDING;
                lbl_1_bss_590[playerIndex].jumpVelocity.x =
                    lbl_1_bss_590[playerIndex].jumpVelocity.y =
                        lbl_1_bss_590[playerIndex].jumpVelocity.z = 0.0f;
                lbl_1_bss_590[playerIndex].velocity.y =
                    lbl_1_bss_20 - lbl_1_bss_590[playerIndex].position.y;
                return;
            }
            break;
        case 13:
            if (lbl_1_bss_590[playerIndex].collisionRecoveryState == 24) {
                lbl_1_bss_590[playerIndex].collisionRecoveryState = 23;
                lbl_1_bss_590[playerIndex].movementState = 8;
                return;
            }
            M641MotionChange(playerIndex, 1);
            lbl_1_bss_590[playerIndex].movementState = 1;
            lbl_1_bss_590[playerIndex].movementTimer = 0;
            lbl_1_bss_590[playerIndex].collisionRecoveryState = 23;
            break;
        }
}

/* Fade-in, finish, main, pre-winner, and start updates convert horizontal stick input into the
 * model's per-frame movement vector here. */
void fn_1_FE64(s16 playerIndex) {
    Point3d movementVelocity;

    movementVelocity.y = movementVelocity.z = 0.0f;
    movementVelocity.x = lbl_1_bss_590[playerIndex].stickX / 56.0f;
    movementVelocity.x *= 8.0f;
    lbl_1_bss_590[playerIndex].velocity = movementVelocity;
}

extern Point3d lbl_1_data_180[14];

/* Fade-in, finish, main, pre-winner, and start updates tilt the character toward its movement
 * direction here. */
void fn_1_FF1C(s16 playerIndex) {
    Point3d velocity;
    Point3d characterRotation;

    velocity = lbl_1_bss_590[playerIndex].velocity;
    characterRotation = lbl_1_bss_590[playerIndex].rotation;

    if (((velocity.x < 0.0f) &&
         (lbl_1_bss_590[playerIndex].tiltState == 0)) ||
        (lbl_1_bss_590[playerIndex].tiltState == -1)) {
        characterRotation.y -= 18.0f;
        if (characterRotation.y < -90.0f) {
            characterRotation.y = -90.0f;
            lbl_1_bss_590[playerIndex].tiltState = 0;
        } else {
            lbl_1_bss_590[playerIndex].tiltState = -1;
        }
    }
    if (((velocity.x > 0.0f) &&
         (lbl_1_bss_590[playerIndex].tiltState == 0)) ||
        (lbl_1_bss_590[playerIndex].tiltState == 1)) {
        characterRotation.y += 18.0f;
        if (characterRotation.y > 90.0f) {
            characterRotation.y = 90.0f;
            lbl_1_bss_590[playerIndex].tiltState = 0;
        } else {
            lbl_1_bss_590[playerIndex].tiltState = 1;
        }
    }
    if (((0.0f == velocity.x) &&
         (lbl_1_bss_590[playerIndex].tiltState == 0)) ||
        (lbl_1_bss_590[playerIndex].tiltState == 2)) {
        if (characterRotation.y < -1.0f) {
            characterRotation.y += 18.0f;
            lbl_1_bss_590[playerIndex].tiltState = 2;
        } else if (1.0f < characterRotation.y) {
            characterRotation.y -= 18.0f;
            lbl_1_bss_590[playerIndex].tiltState = 2;
        } else {
            characterRotation.y = 0.0f;
            lbl_1_bss_590[playerIndex].tiltState = 0;
        }
    }

    lbl_1_bss_590[playerIndex].rotation = characterRotation;
}

extern Point3d lbl_1_bss_560[4];

/* Fade-in, fade-out, finish, main, pre-winner, start, and winner hooks apply player movement and
 * rotation here, then clear transient input after latching it. */
void fn_1_10278(void) {
    s16 playerIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        lbl_1_bss_590[playerIndex].position.x += lbl_1_bss_590[playerIndex].velocity.x;
        lbl_1_bss_590[playerIndex].position.y += lbl_1_bss_590[playerIndex].velocity.y;
        lbl_1_bss_590[playerIndex].position.z += lbl_1_bss_590[playerIndex].velocity.z;
        Hu3DModelPosSetV(lbl_1_bss_590[playerIndex].modelId,
                         &lbl_1_bss_590[playerIndex].position);
        Hu3DModelRotSetV(lbl_1_bss_590[playerIndex].modelId,
                         &lbl_1_bss_590[playerIndex].rotation);
        lbl_1_bss_590[playerIndex].velocity.x = lbl_1_bss_590[playerIndex].velocity.y =
            lbl_1_bss_590[playerIndex].velocity.z = 0.0f;
        lbl_1_bss_590[playerIndex].stickX = lbl_1_bss_590[playerIndex].stickY = 0;
        lbl_1_bss_590[playerIndex].heldButtonFlags |= lbl_1_bss_590[playerIndex].buttons;
        lbl_1_bss_590[playerIndex].jumpInputFlags |= lbl_1_bss_590[playerIndex].buttonsDown;
        lbl_1_bss_590[playerIndex].buttons = lbl_1_bss_590[playerIndex].buttonsDown = 0;
        lbl_1_bss_560[playerIndex] = lbl_1_bss_590[playerIndex].position;
    }
}

/* The finish hook calls this to advance all players' movement and collision states. */
void fn_1_10530(void)
{
    fn_1_B108();
}

extern s16 fn_1_141A4(s16 playerIndex);

extern s16 fn_1_9B30(s16 boardIndex);

extern void fn_1_9CD0(s16 boardIndex, s16 spaceIndex, Point3d *position, s16 *state);

s16 lbl_1_data_464[4] = {0, 0, 0, 0};

s16 lbl_1_data_46C[4] = {-1, -1, -1, -1};

s16 lbl_1_data_474[6] = {-1, -1, -1, -1, 0, 0};

/* The pre-winner hook sets the shared result motion, gives 10 bonus coins to the winning group
 * outside practice mode, and records the four winners. */
void fn_1_10550(s16 groupNo) {
    s16 winnerIds[4];
    s16 playerIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        winnerIds[playerIndex] = -1;
        if (playerIndex >= 0 && playerIndex < 4 &&
            lbl_1_bss_590[playerIndex].currentMotion != 1) {
            s16 motionId = lbl_1_data_344[1].motionId;
            if (motionId != -1) {
                if (lbl_1_data_344[1].blendTime <= 1.0f) {
                    CharMotionSet(lbl_1_bss_590[playerIndex].characterNo,
                                  lbl_1_bss_590[playerIndex].motionIds[motionId]);
                    Hu3DModelAttrReset(
                        lbl_1_bss_590[playerIndex].modelId,
                        Hu3DModelMotionAttrGet(
                            lbl_1_bss_590[playerIndex].modelId) |
                            HU3D_MOTATTR);
                    CharModelAttrSet(lbl_1_bss_590[playerIndex].characterNo,
                                     lbl_1_data_344[1].attributes);
                } else {
                    Hu3DModelAttrSet(lbl_1_bss_590[playerIndex].modelId,
                                     HU3D_MOTATTR_PAUSE);
                    CharMotionShiftSet(lbl_1_bss_590[playerIndex].characterNo,
                                       lbl_1_bss_590[playerIndex].motionIds[motionId],
                                       0.0f, lbl_1_data_344[1].blendTime,
                                       lbl_1_data_344[1].attributes);
                }
                CharModelVoiceFlagSet(lbl_1_bss_590[playerIndex].characterNo,
                                      lbl_1_data_344[1].voiceEnabled);
            }
            lbl_1_bss_590[playerIndex].currentMotion = 1;
        }
        if (groupNo == lbl_1_bss_590[playerIndex].groupNo) {
            s32 playerSlot = lbl_1_bss_590[playerIndex].playerSlot;
    if (!_CheckFlag(FLAG_MG_PRACTICE)) {
                GwPlayer[playerSlot].mgCoinBonus = 10;
            }
            winnerIds[playerIndex] = lbl_1_bss_590[playerIndex].characterNo;
        }
    }
    MgSeqWinnerSet(winnerIds[0], winnerIds[1], winnerIds[2], winnerIds[3]);
}

/* The pre-winner hook advances result states here; it returns nonzero after all four players
 * finish. */
s32 fn_1_10844(void) {
    u8 finishedCount = 0;
    s16 playerIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        switch (lbl_1_bss_590[playerIndex].resultSequenceState) {
        case 14:
            lbl_1_bss_590[playerIndex].collisionRecoveryState =
                M641_COLLISION_RECOVERY_STATE_RESULT_SEQUENCE;
            if (lbl_1_bss_590[playerIndex].movementState == 2 ||
                lbl_1_bss_590[playerIndex].movementState == 3) {
                lbl_1_bss_590[playerIndex].jumpVelocity.x =
                    lbl_1_bss_590[playerIndex].jumpVelocity.y =
                    lbl_1_bss_590[playerIndex].jumpVelocity.z = 0.0f;
            }
            if (lbl_1_bss_590[playerIndex].movementState == 8 ||
                lbl_1_bss_590[playerIndex].movementState == 9) {
                lbl_1_bss_590[playerIndex].movementTimer = 1000;
            }
            if (lbl_1_bss_590[playerIndex].movementState == M641_MOVEMENT_STATE_COLLISION_LAUNCH ||
                lbl_1_bss_590[playerIndex].movementState ==
                    M641_MOVEMENT_STATE_COLLISION_AIRBORNE) {
                lbl_1_bss_590[playerIndex].jumpVelocity.x =
                    lbl_1_bss_590[playerIndex].jumpVelocity.y =
                    lbl_1_bss_590[playerIndex].jumpVelocity.z = 0.0f;
            }
            if (lbl_1_bss_590[playerIndex].movementState == 1) {
                lbl_1_bss_590[playerIndex].resultSequenceState =
                    M641_RESULT_SEQUENCE_STATE_POSITION_WITH_TEAMMATE;
            }
            break;
        case M641_RESULT_SEQUENCE_STATE_POSITION_WITH_TEAMMATE: {
            s16 teammateIndex = playerIndex;
            s16 index;
            f32 targetOffsetX;

            for (index = 0; index < 4; index++) {
                if (playerIndex != index &&
                    lbl_1_bss_590[playerIndex].groupNo == lbl_1_bss_590[index].groupNo) {
                    teammateIndex = index;
                }
            }
            if (lbl_1_bss_590[playerIndex].position.x <
                lbl_1_bss_590[teammateIndex].position.x) {
                index = 0;
            } else {
                index = 1;
            }
            targetOffsetX = lbl_1_data_33C[index] - lbl_1_bss_590[playerIndex].position.x;
            if (targetOffsetX < -5.0f) {
                lbl_1_bss_590[playerIndex].stickX = -M641_STICK_LIMIT;
            } else if (targetOffsetX > 5.0f) {
                lbl_1_bss_590[playerIndex].stickX = M641_STICK_LIMIT;
            } else {
                lbl_1_bss_590[playerIndex].resultSequenceState =
                    M641_RESULT_SEQUENCE_STATE_AFTER_POSITIONING;
                lbl_1_data_464[playerIndex] = 0;
            }
            break;
        }
        case M641_RESULT_SEQUENCE_STATE_AFTER_POSITIONING:
            lbl_1_data_464[playerIndex]++;
            if (lbl_1_data_464[playerIndex] > 10) {
                lbl_1_bss_590[playerIndex].resultSequenceState =
                    M641_RESULT_SEQUENCE_STATE_FINISHED;
            }
            break;
        case M641_RESULT_SEQUENCE_STATE_FINISHED:
            lbl_1_bss_590[playerIndex].resultSequenceState = M641_RESULT_SEQUENCE_STATE_FINISHED;
            finishedCount++;
            break;
        default:
            lbl_1_bss_590[playerIndex].resultSequenceState =
                M641_RESULT_SEQUENCE_STATE_WAIT_FOR_MOVEMENT;
            break;
        }
    }
    fn_1_B108();
    if (finishedCount >= 4) {
        return 1;
    }
    return 0;
}

/* Fade-out and winner callbacks call this to select each player's win or loss motion and advance
 * its animation. */
void fn_1_10D10(void) {
    s16 playerIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_590[playerIndex].currentMotion != 12 &&
            lbl_1_bss_590[playerIndex].currentMotion != 13) {
            if ((s16)lbl_1_bss_7DE == lbl_1_bss_590[playerIndex].groupNo) {
                fn_1_9F7C(playerIndex, 12);
            } else {
                fn_1_9F7C(playerIndex, 13);
            }
        } else if ((lbl_1_bss_590[playerIndex].currentMotion == 12 ||
                    lbl_1_bss_590[playerIndex].currentMotion == 13) &&
                   CharMotionShiftIDGet(lbl_1_bss_590[playerIndex].characterNo) <
                       0) {
            if (CharMotionTimeGet(lbl_1_bss_590[playerIndex].characterNo) +
                    Hu3DData[lbl_1_bss_590[playerIndex].modelId].motWork.speed >=
                CharMotionMaxTimeGet(lbl_1_bss_590[playerIndex].characterNo)) {
                CharModelAttrSet(lbl_1_bss_590[playerIndex].characterNo, HU3D_MOTATTR_PAUSE);
            }
        }
    }

    fn_1_10278();
}

M641CharacterRecord lbl_1_bss_590[4];

Point3d lbl_1_bss_560[4];

extern s16 fn_1_141A4(s16 playerIndex);

void fn_1_10550(s16 groupNo);

extern void fn_1_B108(void);

s32 fn_1_10844(void);

/* Advances character motion transitions and applies each player's frame movement. */
void fn_1_10D10(void);

void fn_1_11C98(s16 playerIndex);

void fn_1_12CA4(s16 playerIndex);

/* The main callback calls this once per update to sample active players' sticks and buttons. */
void fn_1_1155C(void) {
    s16 playerIndex;
    s16 xDeadzone;
    s16 yDeadzone;

    xDeadzone = 6;
    yDeadzone = 6;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_590[playerIndex].inputSamplingGate == 0) {
            lbl_1_bss_590[playerIndex].stickX = lbl_1_bss_590[playerIndex].stickY = 0;
            lbl_1_bss_590[playerIndex].buttons = lbl_1_bss_590[playerIndex].buttonsDown = 0;
            lbl_1_bss_590[playerIndex].stickX = HuPadStkX[lbl_1_bss_590[playerIndex].padNo];
            if (-xDeadzone < lbl_1_bss_590[playerIndex].stickX &&
                lbl_1_bss_590[playerIndex].stickX < xDeadzone) {
                lbl_1_bss_590[playerIndex].stickX = 0;
            }
            lbl_1_bss_590[playerIndex].stickX = (s32)(lbl_1_bss_590[playerIndex].stickX / 5.6f);
            lbl_1_bss_590[playerIndex].stickX *= 5;
            if (lbl_1_bss_590[playerIndex].stickX > M641_STICK_LIMIT) {
                lbl_1_bss_590[playerIndex].stickX = M641_STICK_LIMIT;
            }
            if (lbl_1_bss_590[playerIndex].stickX < -M641_STICK_LIMIT) {
                lbl_1_bss_590[playerIndex].stickX = -M641_STICK_LIMIT;
            }
            lbl_1_bss_590[playerIndex].stickY = HuPadStkY[lbl_1_bss_590[playerIndex].padNo];
            if (-yDeadzone < lbl_1_bss_590[playerIndex].stickY &&
                lbl_1_bss_590[playerIndex].stickY < yDeadzone) {
                lbl_1_bss_590[playerIndex].stickY = 0;
            }
            lbl_1_bss_590[playerIndex].stickY = (s32)(lbl_1_bss_590[playerIndex].stickY / 5.6f);
            lbl_1_bss_590[playerIndex].stickY *= 5;
            if (lbl_1_bss_590[playerIndex].stickY > M641_STICK_LIMIT) {
                lbl_1_bss_590[playerIndex].stickY = M641_STICK_LIMIT;
            }
            if (lbl_1_bss_590[playerIndex].stickY < -M641_STICK_LIMIT) {
                lbl_1_bss_590[playerIndex].stickY = -M641_STICK_LIMIT;
            }
            lbl_1_bss_590[playerIndex].buttons = HuPadBtn[lbl_1_bss_590[playerIndex].padNo];
            lbl_1_bss_590[playerIndex].buttonsDown =
                HuPadBtnDown[lbl_1_bss_590[playerIndex].padNo];
            if ((lbl_1_bss_590[playerIndex].buttonsDown & 0x2000) != 0) {
                lbl_1_bss_590[playerIndex].buttonsDown |= 0x20;
            }
            if ((lbl_1_bss_590[playerIndex].buttonsDown & 0x4000) != 0) {
                lbl_1_bss_590[playerIndex].buttonsDown |= 0x40;
            }
        }
    }
}

/* The init hook and player setup call this to clear per-round route and movement counters. */
void fn_1_11AD8(void) {
    s16 playerIndex;

    playerIndex = 0;
    while (playerIndex < 4) {
        lbl_1_bss_590[playerIndex].routeUpdateCount = 0;
        lbl_1_bss_590[playerIndex].targetActionFrames = 0;
        lbl_1_bss_590[playerIndex].targetSpaceIndex = -1;
        lbl_1_bss_590[playerIndex].autonomousMovementState = 0;
        lbl_1_bss_590[playerIndex].autonomousMovementFrames = 0;
        lbl_1_bss_590[playerIndex].directionChangeTimer = 0;
        playerIndex += 1;
    }
}

/* The main callback calls this to clear per-frame movement input and update every player's
 * route. */
void fn_1_11BAC(void) {
    s16 playerIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        lbl_1_bss_590[playerIndex].stickX = lbl_1_bss_590[playerIndex].stickY = 0;
        lbl_1_bss_590[playerIndex].buttons = lbl_1_bss_590[playerIndex].buttonsDown = 0;
        if (lbl_1_bss_7DC == 0) {
            fn_1_11C98(playerIndex);
        } else {
            fn_1_12CA4(playerIndex);
        }
        lbl_1_bss_590[playerIndex].routeUpdateCount++;
    }
}

/* The main hook calls this for each player in day mode to choose nearby-space targets and automatic
 * movement. */
void fn_1_11C98(s16 playerIndex) {
    s16 routeMode[4];
    s16 spaceState[8];
    Point3d spacePosition[7];
    f32 routeMetric;
    f32 routeComparisonValue;
    s16 nearestSpace;
    s16 markedSpace;
    s16 scanIndex;
    s16 routeDecisionValue;
    s16 routeProgressValue;
    switch (lbl_1_bss_590[playerIndex].computerDifficulty) {
    case 0:
        if (lbl_1_data_46C[playerIndex] < 0) {
            lbl_1_data_46C[playerIndex] = frandmod(2) + 2;
        }
        routeMode[playerIndex] = 0;
        break;
    case 1:
        if (lbl_1_data_46C[playerIndex] < 0) {
            lbl_1_data_46C[playerIndex] = frandmod(4) + 6;
        }
        routeMode[playerIndex] = 0;
        break;
    case 2:
        if (lbl_1_data_46C[playerIndex] < 0) {
            lbl_1_data_46C[playerIndex] = frandmod(2) + 11;
        }
        routeMode[playerIndex] = 1;
        break;
    case 3:
        if (lbl_1_data_46C[playerIndex] < 0) {
            lbl_1_data_46C[playerIndex] = frandmod(2) + 16;
        }
        routeMode[playerIndex] = 1;
        break;
    }

    routeMetric = 6000.0f;
    nearestSpace = -1;
    markedSpace = -1;
    for (scanIndex = 0; scanIndex < 7; scanIndex++) {
        fn_1_9CD0(lbl_1_bss_590[playerIndex].groupNo,
                  scanIndex, &spacePosition[scanIndex], &spaceState[scanIndex]);
        /* Compare the space and player x positions when scanning for a nearby space. */
        routeComparisonValue = spacePosition[scanIndex].x -
                         lbl_1_bss_590[playerIndex].position.x;
        if (routeComparisonValue < 0.0f) {
            routeComparisonValue *= -1.0f;
        }
        if ((routeMetric > routeComparisonValue) && (spaceState[scanIndex] == 0)) {
            nearestSpace = scanIndex;
            routeMetric = routeComparisonValue;
        }
        if ((spaceState[scanIndex] == 2) && (routeMode[playerIndex] != 0)) {
            markedSpace = scanIndex;
        }
    }

    lbl_1_bss_590[playerIndex].targetSpaceIndex = -1;
    routeMetric = routeComparisonValue = 0.0f;
    for (scanIndex = 0; scanIndex < 4; scanIndex++) {
        if ((scanIndex != playerIndex) && (markedSpace >= 0) &&
            (lbl_1_bss_590[scanIndex].groupNo ==
             lbl_1_bss_590[playerIndex].groupNo)) {
            /* Compare absolute x offsets from the marked space to both players. */
            routeMetric = spacePosition[markedSpace].x -
                            lbl_1_bss_590[playerIndex].position.x;
            routeComparisonValue = spacePosition[markedSpace].x -
                              lbl_1_bss_590[scanIndex].position.x;
            if (routeMetric < 0.0f) {
                routeMetric *= -1.0f;
            }
            if (routeComparisonValue < 0.0f) {
                routeComparisonValue *= -1.0f;
            }
        }
    }
    if (routeMetric < routeComparisonValue) {
        lbl_1_bss_590[playerIndex].targetSpaceIndex = markedSpace;
    } else {
        for (scanIndex = 0; scanIndex < 4; scanIndex++) {
            if ((scanIndex != playerIndex) && (nearestSpace >= 0) &&
                (lbl_1_bss_590[scanIndex].groupNo ==
                 lbl_1_bss_590[playerIndex].groupNo)) {
                /* Compare the players' x positions when selecting a target space. */
                routeMetric = lbl_1_bss_590[scanIndex].position.x -
                                  lbl_1_bss_590[playerIndex].position.x;
                routeComparisonValue = spacePosition[nearestSpace].x -
                                  lbl_1_bss_590[scanIndex].position.x;
                if (routeMetric > 0.0f) {
                    if (routeComparisonValue > 0.0f) {
                        lbl_1_bss_590[playerIndex].targetSpaceIndex = -1;
                    } else {
                        lbl_1_bss_590[playerIndex].targetSpaceIndex = nearestSpace;
                    }
                } else if (routeComparisonValue > 0.0f) {
                    lbl_1_bss_590[playerIndex].targetSpaceIndex = nearestSpace;
                } else {
                    lbl_1_bss_590[playerIndex].targetSpaceIndex = -1;
                }
            }
        }
    }

    /* Scale the fixed 1800-frame divisor using the stored route value. */
    routeDecisionValue = 1800;
    if (lbl_1_data_46C[playerIndex] > 0) {
        routeDecisionValue /= lbl_1_data_46C[playerIndex];
    }
    /* This value is elapsed frames before the route timing comparison. */
    routeProgressValue = 1800 - (s16)MgSeqTimerValueGet();
    /* Scale elapsed frames by the route decision divisor. */
    routeMetric = (f32)routeProgressValue / (f32)routeDecisionValue;
    /* This return value is the player entry score. */
    routeComparisonValue = (f32)fn_1_141A4(playerIndex);
    /* This return value counts scheduled and queued ordinary actions. */
    routeDecisionValue = fn_1_9B30(lbl_1_bss_590[playerIndex].groupNo);
    /* This value changes from elapsed frames to the score difference. */
    routeProgressValue = lbl_1_data_46C[playerIndex] - (s32)routeComparisonValue;
    if ((routeComparisonValue <= routeMetric) || (routeDecisionValue < routeProgressValue)) {
        if ((lbl_1_bss_590[playerIndex].targetSpaceIndex >= 0) &&
            (lbl_1_bss_590[playerIndex].targetActionFrames == 0)) {
            /* Use the target-space x offset to set movement direction and proximity. */
            routeMetric = spacePosition[lbl_1_bss_590[playerIndex].targetSpaceIndex].x -
                           lbl_1_bss_590[playerIndex].position.x;
            if (routeMetric < 0.0f) {
                lbl_1_bss_590[playerIndex].stickX = -M641_STICK_LIMIT;
            }
            if (routeMetric > 0.0f) {
                lbl_1_bss_590[playerIndex].stickX = M641_STICK_LIMIT;
            }
            if (routeMetric < 0.0f) {
                routeMetric *= -1.0f;
            }
    if (routeMetric < 45.0f) {
                lbl_1_bss_590[playerIndex].stickX = 0;
                lbl_1_bss_590[playerIndex].targetActionFrames = 1;
            }
        }
        if ((lbl_1_bss_590[playerIndex].targetSpaceIndex >= 0) &&
            (lbl_1_bss_590[playerIndex].targetActionFrames > 0)) {
            lbl_1_bss_590[playerIndex].buttons |= 0x100;
            if ((lbl_1_bss_590[playerIndex].targetActionFrames == 1) &&
                (lbl_1_bss_590[playerIndex].movementState != 2) &&
                (lbl_1_bss_590[playerIndex].movementState != 3)) {
                lbl_1_bss_590[playerIndex].buttonsDown |= 0x100;
            }
            lbl_1_bss_590[playerIndex].targetActionFrames++;
            if (lbl_1_bss_590[playerIndex].targetActionFrames > 50) {
                lbl_1_bss_590[playerIndex].targetActionFrames = 0;
            }
        } else {
            lbl_1_bss_590[playerIndex].targetActionFrames = 0;
        }
    } else {
        if (lbl_1_bss_590[playerIndex].autonomousMovementState == 0) {
            /* Use this random roll to choose autonomous movement state. */
            routeDecisionValue = frandmod(100);
            if (routeDecisionValue < 33) {
                lbl_1_bss_590[playerIndex].autonomousMovementState = 1;
            } else if (routeDecisionValue < 66) {
                lbl_1_bss_590[playerIndex].autonomousMovementState = 2;
            } else if (routeDecisionValue < 100) {
                lbl_1_bss_590[playerIndex].autonomousMovementState = 3;
            }
            lbl_1_bss_590[playerIndex].autonomousMovementFrames = 0;
        }
        for (scanIndex = 0; scanIndex < 7; scanIndex++) {
            if ((spaceState[scanIndex] == 1) &&
                (lbl_1_bss_590[playerIndex].position.x -
                 lbl_1_bss_590[playerIndex].collisionRadius <
                 spacePosition[scanIndex].x) &&
                (spacePosition[scanIndex].x <
                 lbl_1_bss_590[playerIndex].position.x +
                 lbl_1_bss_590[playerIndex].collisionRadius)) {
                lbl_1_bss_590[playerIndex].autonomousMovementState = 3;
                lbl_1_bss_590[playerIndex].autonomousMovementFrames = 0;
            }
        }
        if (lbl_1_bss_590[playerIndex].autonomousMovementState == 1) {
            lbl_1_bss_590[playerIndex].stickX = M641_STICK_LIMIT;
            (lbl_1_bss_590[playerIndex].directionChangeTimer)--;
            if (lbl_1_bss_590[playerIndex].directionChangeTimer <= 0) {
                lbl_1_bss_590[playerIndex].autonomousMovementState = 2;
                lbl_1_bss_590[playerIndex].directionChangeTimer = frandmod(15) + 45;
            }
            if (lbl_1_bss_590[playerIndex].autonomousMovementFrames >= 60) {
                lbl_1_bss_590[playerIndex].autonomousMovementState = 0;
            }
        } else if (lbl_1_bss_590[playerIndex].autonomousMovementState == 2) {
            lbl_1_bss_590[playerIndex].stickX = -M641_STICK_LIMIT;
            (lbl_1_bss_590[playerIndex].directionChangeTimer)--;
            if (lbl_1_bss_590[playerIndex].directionChangeTimer <= 0) {
                lbl_1_bss_590[playerIndex].autonomousMovementState = 1;
                lbl_1_bss_590[playerIndex].directionChangeTimer = frandmod(15) + 45;
            }
            if (lbl_1_bss_590[playerIndex].autonomousMovementFrames >= 60) {
                lbl_1_bss_590[playerIndex].autonomousMovementState = 0;
            }
        }
        if (lbl_1_bss_590[playerIndex].autonomousMovementState == 3) {
            if ((lbl_1_bss_590[playerIndex].movementState != 2) &&
                (lbl_1_bss_590[playerIndex].movementState != 3) &&
                (lbl_1_bss_590[playerIndex].movementState != 4) &&
                (lbl_1_bss_590[playerIndex].autonomousMovementFrames == 0)) {
                lbl_1_bss_590[playerIndex].buttonsDown |= 0x100;
            }
            if (lbl_1_bss_590[playerIndex].autonomousMovementFrames < 3) {
                lbl_1_bss_590[playerIndex].buttons |= 0x100;
            }
            for (scanIndex = 0; scanIndex < 7; scanIndex++) {
                if ((spaceState[scanIndex] == 1) &&
                    (lbl_1_bss_590[playerIndex].position.x -
                     lbl_1_bss_590[playerIndex].collisionRadius <
                     spacePosition[scanIndex].x) &&
                    (spacePosition[scanIndex].x <
                     lbl_1_bss_590[playerIndex].position.x +
                     lbl_1_bss_590[playerIndex].collisionRadius)) {
                    lbl_1_bss_590[playerIndex].buttons |= 0x100;
                }
            }
            if (lbl_1_bss_590[playerIndex].autonomousMovementFrames >= 50) {
                lbl_1_bss_590[playerIndex].autonomousMovementState = 0;
            }
        }
        lbl_1_bss_590[playerIndex].autonomousMovementFrames++;
    }
}

/* The main hook calls this for each player in night mode to choose nearby-space targets and
 * automatic movement. */
void fn_1_12CA4(s16 playerIndex) {
    s16 routeMode[4];
    s16 spaceState[8];
    Point3d spacePosition[7];
    f32 routeMetric;
    f32 routeComparisonValue;
    s16 nearestSpace;
    s16 markedSpace;
    s16 scanIndex;
    s16 routeDecisionValue;
    s16 routeProgressValue;

    extern s16 fn_1_14188(s16 boardIndex);

    switch (lbl_1_bss_590[playerIndex].computerDifficulty) {
    case 0:
        if (lbl_1_data_474[playerIndex] < 0) {
            lbl_1_data_474[playerIndex] = frandmod(2) + 1;
        }
        routeMode[playerIndex] = 0;
        break;
    case 1:
        if (lbl_1_data_474[playerIndex] < 0) {
            lbl_1_data_474[playerIndex] = frandmod(2) + 3;
        }
        routeMode[playerIndex] = 0;
        break;
    case 2:
        if (lbl_1_data_474[playerIndex] < 0) {
            lbl_1_data_474[playerIndex] = frandmod(3) + 7;
        }
        routeMode[playerIndex] = 1;
        break;
    case 3:
        if (lbl_1_data_474[playerIndex] < 0) {
            lbl_1_data_474[playerIndex] = frandmod(3) + 15;
        }
        routeMode[playerIndex] = 1;
        break;
    }
    routeMetric = 6000.0f;
    nearestSpace = -1;
    markedSpace = -1;
    for (scanIndex = 0; scanIndex < 7; scanIndex++) {
        fn_1_9CD0(lbl_1_bss_590[playerIndex].groupNo,
                  scanIndex, &spacePosition[scanIndex], &spaceState[scanIndex]);
        /* Compare the space and player x positions when scanning for a nearby space. */
        routeComparisonValue = spacePosition[scanIndex].x -
                         lbl_1_bss_590[playerIndex].position.x;
        if (routeComparisonValue < 0.0f) {
            routeComparisonValue *= -1.0f;
        }
        if ((routeMetric > routeComparisonValue) && (spaceState[scanIndex] == 0)) {
            nearestSpace = scanIndex;
            routeMetric = routeComparisonValue;
        }
        if ((spaceState[scanIndex] == 2) && (routeMode[playerIndex] != 0)) {
            markedSpace = scanIndex;
        }
    }
    lbl_1_bss_590[playerIndex].targetSpaceIndex = -1;
    routeMetric = routeComparisonValue = 0.0f;
    for (scanIndex = 0; scanIndex < 4; scanIndex++) {
        if ((scanIndex != playerIndex) && (markedSpace >= 0) &&
            (lbl_1_bss_590[scanIndex].groupNo ==
             lbl_1_bss_590[playerIndex].groupNo)) {
            /* Compare absolute x offsets from the marked space to both players. */
            routeMetric = spacePosition[markedSpace].x -
                             lbl_1_bss_590[playerIndex].position.x;
            routeComparisonValue = spacePosition[markedSpace].x -
                              lbl_1_bss_590[scanIndex].position.x;
            if (routeMetric < 0.0f) {
                routeMetric *= -1.0f;
            }
            if (routeComparisonValue < 0.0f) {
                routeComparisonValue *= -1.0f;
            }
        }
    }
    if (routeMetric < routeComparisonValue) {
        lbl_1_bss_590[playerIndex].targetSpaceIndex = markedSpace;
    } else {
        for (scanIndex = 0; scanIndex < 4; scanIndex++) {
            if ((scanIndex != playerIndex) && (nearestSpace >= 0) &&
                (lbl_1_bss_590[scanIndex].groupNo ==
                 lbl_1_bss_590[playerIndex].groupNo)) {
                /* Compare the players' x positions when selecting a target space. */
                routeMetric = lbl_1_bss_590[scanIndex].position.x -
                                  lbl_1_bss_590[playerIndex].position.x;
                routeComparisonValue = spacePosition[nearestSpace].x -
                                  lbl_1_bss_590[scanIndex].position.x;
                if (routeMetric > 0.0f) {
                    if (routeComparisonValue > 0.0f) {
                        lbl_1_bss_590[playerIndex].targetSpaceIndex = -1;
                    } else {
                        lbl_1_bss_590[playerIndex].targetSpaceIndex = nearestSpace;
                    }
                } else if (routeComparisonValue > 0.0f) {
                    lbl_1_bss_590[playerIndex].targetSpaceIndex = nearestSpace;
                } else {
                    lbl_1_bss_590[playerIndex].targetSpaceIndex = -1;
                }
            }
        }
    }

    /* Scale the fixed 1800-frame divisor using the stored route value. */
    routeDecisionValue = 1800;
    if (lbl_1_data_474[playerIndex] > 0) {
        routeDecisionValue /= lbl_1_data_474[playerIndex];
    }
    /* This value is elapsed frames before the route timing comparison. */
    routeProgressValue = 1800 - (s16)MgSeqTimerValueGet();
    /* Scale elapsed frames by the route decision divisor. */
    routeMetric = (f32)routeProgressValue / (f32)routeDecisionValue;
    /* This return value is the player entry score. */
    routeComparisonValue = (f32)fn_1_14188(lbl_1_bss_590[playerIndex].groupNo);
    /* This return value counts scheduled and queued ordinary actions. */
    routeDecisionValue = fn_1_9B30(lbl_1_bss_590[playerIndex].groupNo);
    /* This value changes from elapsed frames to the score difference. */
    routeProgressValue = lbl_1_data_474[playerIndex] - (s32)routeComparisonValue;
    if ((routeComparisonValue <= routeMetric) || (routeDecisionValue < routeProgressValue)) {
        if ((lbl_1_bss_590[playerIndex].targetSpaceIndex >= 0) &&
            (lbl_1_bss_590[playerIndex].targetActionFrames == 0)) {
            /* Use the target-space x offset to set movement direction and proximity. */
            routeMetric = spacePosition[lbl_1_bss_590[playerIndex].targetSpaceIndex].x -
                             lbl_1_bss_590[playerIndex].position.x;
            if (routeMetric < 0.0f) {
                lbl_1_bss_590[playerIndex].stickX = -M641_STICK_LIMIT;
            }
            if (routeMetric > 0.0f) {
                lbl_1_bss_590[playerIndex].stickX = M641_STICK_LIMIT;
            }
            if (routeMetric < 0.0f) {
                routeMetric *= -1.0f;
            }
    if (routeMetric < 300.0f) {
                lbl_1_bss_590[playerIndex].targetActionFrames = 1;
            }
        }
        if ((lbl_1_bss_590[playerIndex].targetSpaceIndex >= 0) &&
            (lbl_1_bss_590[playerIndex].targetActionFrames > 0)) {
            routeMetric = spacePosition[lbl_1_bss_590[playerIndex].targetSpaceIndex].x -
                             lbl_1_bss_590[playerIndex].position.x;
            if (routeMetric < 0.0f) {
                lbl_1_bss_590[playerIndex].stickX = -M641_STICK_LIMIT;
            }
            if (routeMetric > 0.0f) {
                lbl_1_bss_590[playerIndex].stickX = M641_STICK_LIMIT;
            }
            lbl_1_bss_590[playerIndex].buttons |= 0x100;
            if (lbl_1_bss_590[playerIndex].targetActionFrames == 1) {
                lbl_1_bss_590[playerIndex].buttonsDown |= 0x100;
            }
            routeMetric = spacePosition[lbl_1_bss_590[playerIndex].targetSpaceIndex].x -
                             lbl_1_bss_590[playerIndex].position.x;
            if (routeMetric < 0.0f) {
                routeMetric *= -1.0f;
            }
    if ((routeMetric < 50.0f) &&
                ((lbl_1_bss_590[playerIndex].buttonsDown & 0x100) == 0) &&
                (lbl_1_bss_590[playerIndex].targetActionFrames >= 5)) {
                lbl_1_bss_590[playerIndex].targetActionFrames = -1;
                lbl_1_bss_590[playerIndex].buttonsDown |= 0x100;
            }
            (lbl_1_bss_590[playerIndex].targetActionFrames)++;
            if (lbl_1_bss_590[playerIndex].targetActionFrames > 50) {
                lbl_1_bss_590[playerIndex].targetActionFrames = 0;
            }
        } else {
            lbl_1_bss_590[playerIndex].targetActionFrames = 0;
        }
    } else {
        if (lbl_1_bss_590[playerIndex].autonomousMovementState == 0) {
            /* Use this random roll to choose autonomous movement state. */
            routeDecisionValue = frandmod(100);
            if (routeDecisionValue < 25) {
                lbl_1_bss_590[playerIndex].autonomousMovementState = 1;
            } else if (routeDecisionValue < 50) {
                lbl_1_bss_590[playerIndex].autonomousMovementState = 2;
            } else if (routeDecisionValue < 75) {
                lbl_1_bss_590[playerIndex].autonomousMovementState = 3;
            } else if (routeDecisionValue < 100) {
                lbl_1_bss_590[playerIndex].autonomousMovementState = 4;
            }
            lbl_1_bss_590[playerIndex].autonomousMovementFrames = 0;
        }
        if (lbl_1_bss_590[playerIndex].autonomousMovementState == 1) {
            lbl_1_bss_590[playerIndex].stickX = M641_STICK_LIMIT;
            (lbl_1_bss_590[playerIndex].directionChangeTimer)--;
            if (lbl_1_bss_590[playerIndex].directionChangeTimer <= 0) {
                lbl_1_bss_590[playerIndex].autonomousMovementState = 2;
                lbl_1_bss_590[playerIndex].directionChangeTimer = frandmod(15) + 45;
            }
            if (lbl_1_bss_590[playerIndex].autonomousMovementFrames >= 60) {
                lbl_1_bss_590[playerIndex].autonomousMovementState = 0;
            }
        } else if (lbl_1_bss_590[playerIndex].autonomousMovementState == 2) {
            lbl_1_bss_590[playerIndex].stickX = -M641_STICK_LIMIT;
            (lbl_1_bss_590[playerIndex].directionChangeTimer)--;
            if (lbl_1_bss_590[playerIndex].directionChangeTimer <= 0) {
                lbl_1_bss_590[playerIndex].autonomousMovementState = 1;
                lbl_1_bss_590[playerIndex].directionChangeTimer = frandmod(15) + 45;
            }
            if (lbl_1_bss_590[playerIndex].autonomousMovementFrames >= 60) {
                lbl_1_bss_590[playerIndex].autonomousMovementState = 0;
            }
        }
        if (lbl_1_bss_590[playerIndex].autonomousMovementState == 3) {
            if (lbl_1_bss_590[playerIndex].autonomousMovementFrames == 0) {
                lbl_1_bss_590[playerIndex].buttonsDown |= 0x100;
            }
            if (lbl_1_bss_590[playerIndex].autonomousMovementFrames < 3) {
                lbl_1_bss_590[playerIndex].buttons |= 0x100;
            }
            if (lbl_1_bss_590[playerIndex].autonomousMovementFrames >= 30) {
                lbl_1_bss_590[playerIndex].autonomousMovementState = 0;
            }
        }
        if (lbl_1_bss_590[playerIndex].autonomousMovementState == 4) {
            if (lbl_1_bss_590[playerIndex].autonomousMovementFrames == 0) {
                lbl_1_bss_590[playerIndex].buttonsDown |= 0x100;
            }
            if (lbl_1_bss_590[playerIndex].autonomousMovementFrames < 3) {
                lbl_1_bss_590[playerIndex].buttons |= 0x100;
            }
            if (lbl_1_bss_590[playerIndex].autonomousMovementFrames == 15) {
                lbl_1_bss_590[playerIndex].buttonsDown |= 0x100;
            }
            if (lbl_1_bss_590[playerIndex].autonomousMovementFrames >= 60) {
                lbl_1_bss_590[playerIndex].autonomousMovementState = 0;
            }
        }
        (lbl_1_bss_590[playerIndex].autonomousMovementFrames)++;
    }
}
