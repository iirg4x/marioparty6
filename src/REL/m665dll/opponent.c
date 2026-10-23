/* Sets up Verbal Assault opponents, movement, and collision effects. */
#include "REL/m665dll/effects.h"
#include "REL/m665dll/player.h"
#include "REL/m665dll/stage.h"
#include "dolphin/types.h"

#include "math.h"
#include "REL/m665dll/sequence.h"

#include "dolphin/mtx.h"

#include "dolphin/gx.h"

#include "game/data.h"

#include "game/hu3d.h"

#include "game/object.h"

#include "game/memory.h"

#include "game/gamemes.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/mg/score.h"

#include "game/sprite.h"

#include "game/flag.h"

#include "game/gamework.h"

#include "game/main.h"

#include "game/audio.h"

#include "game/charman.h"

#include "game/hsfex.h"

#include "game/mg/seqman.h"

#include "game/mg/timer.h"

#include "game/wipe.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

/* Shared memory tag for Verbal Assault asset allocations. */
#define M665_MEMORY_TAG (1 << 28)

s32 lbl_1_data_338[2] = {DATANUM(DATA_m665, 7), DATANUM(DATA_m665, 10)};

f32 lbl_1_data_340[3] = {0.0f, 400.0f, 200.0f};

f32 lbl_1_data_34C[3] = {-200.0f, -150.0f, -200.0f};

f32 lbl_1_data_358[3] = {100.0f, 288.0f, 476.0f};

s16 lbl_1_data_364[4] = {30, 40, 45, 55};

char lbl_1_data_36C[] = "itemhook";

char lbl_1_data_375[] = "itemhook_L";

char lbl_1_data_380[] = "itemhook_R";

char lbl_1_data_38B[] = "attacker's beam is not empty\n";

char lbl_1_data_3A9[] = "itemhook_gun1";

void fn_1_170C(s16 playerNo, s16 mode, f32 frame);

void fn_1_12948(void);

void fn_1_C838(void);

extern s32 frandmod(s32 modulus);

/* Creates opponent vehicles and hooks their models during the sequence init hook. */
void fn_1_C40C(void) {
    s32 playerIndex;
    s16 playerNo;
    M665PlayerWork *playerWork;
    s16 modelID;
    s32 motionIndex;
    s16 motionID;

    playerIndex = 0;
    while (playerIndex < 3) {
        playerNo = lbl_1_bss_250.otherPlayers[playerIndex];
        playerWork = &lbl_1_bss_10[playerNo];
        modelID = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m665, 6), M665_MEMORY_TAG, HEAP_MODEL));
        Hu3DModelPosSet(modelID,
                        (-600.0f) + 600.0f * (f32) playerIndex,
                        0.0f,
                        2500.0f + lbl_1_data_340[playerIndex]);
        for (motionIndex = 0; motionIndex < 2; motionIndex++) {
            motionID = Hu3DJointMotion(
                modelID,
                HuDataSelHeapReadNum(lbl_1_data_338[motionIndex], M665_MEMORY_TAG,
                                     HEAP_MODEL));
            playerWork->models.other.motions[motionIndex] = motionID;
        }
        Hu3DMotionSet(modelID, playerWork->models.other.motions[0]);
        Hu3DModelAttrSet(modelID, HU3D_MOTATTR_LOOP);
        Hu3DModelShadowSet(modelID);
        Hu3DMotionTimeSet(modelID, (f32) frandmod(40));
        lbl_1_bss_10[playerNo].vehicleModel = modelID;
        Hu3DModelHookSet(playerWork->vehicleModel, lbl_1_data_36C,
                         playerWork->characterModel);
        Hu3DModelRotSet(playerWork->characterModel, 0.0f, 180.0f,
                        0.0f);
        fn_1_170C(playerNo, 1, 0.0f);
        Hu3DMotionSpeedSet(playerWork->vehicleModel, 2.0f);
        playerWork->motionSpeed = 2.0f;
        modelID = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m665, 11), M665_MEMORY_TAG, HEAP_MODEL));
        Hu3DModelHookSet(playerWork->vehicleModel, lbl_1_data_375, modelID);
        playerWork->models.other.effects[0].model = modelID;
        modelID = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m665, 11), M665_MEMORY_TAG, HEAP_MODEL));
        Hu3DModelHookSet(playerWork->vehicleModel, lbl_1_data_380, modelID);
        playerWork->models.other.effects[1].model = modelID;
        playerIndex++;
    }
    fn_1_12948();
    fn_1_C838();
}

typedef s16 HUSPR_GROUPID;

void MgScoreBoxDispSet(HUSPR_GROUPID groupID, BOOL display);

void MgScoreBoxTPLvlSet(HUSPR_GROUPID groupID, float level);

void HuSprAttrReset(HUSPR_GROUPID groupID, s16 memberNo, s32 attr);

void HuSprAttrSet(HUSPR_GROUPID groupID, s16 memberNo, s32 attr);

/* Restores opponent score boxes and sprite attributes during the main hook. */
void fn_1_C6B8(void) {
    s32 playerIndex;
    s32 memberIndex;
    s16 playerNo;
    M665PlayerWork *playerWork;

    for (playerIndex = 0; playerIndex < 3; playerIndex++) {
        playerNo = lbl_1_bss_250.otherPlayers[playerIndex];
        playerWork = &lbl_1_bss_10[playerNo];
        MgScoreBoxDispSet(playerWork->effect.other.scoreGroup, 1);
        MgScoreBoxTPLvlSet(playerWork->effect.other.scoreGroup, 1.0f);
        for (memberIndex = 0; memberIndex < 3; memberIndex++) {
            HuSprAttrReset(playerWork->effect.other.spriteGroup, memberIndex + 1, 4);
        }
    }
}

/* Hides opponent score boxes and updates their sprite attributes in the pre-winner hook. */
void fn_1_C778(void) {
    s32 playerIndex;
    s32 memberIndex;
    s16 playerNo;
    M665PlayerWork *playerWork;

    for (playerIndex = 0; playerIndex < 3; playerIndex++) {
        playerNo = lbl_1_bss_250.otherPlayers[playerIndex];
        playerWork = &lbl_1_bss_10[playerNo];
        MgScoreBoxDispSet(playerWork->effect.other.scoreGroup, 0);
        MgScoreBoxTPLvlSet(playerWork->effect.other.scoreGroup, 0.0f);
        for (memberIndex = 0; memberIndex < 3; memberIndex++) {
            HuSprAttrSet(playerWork->effect.other.spriteGroup, memberIndex + 1, 4);
        }
    }
}

static inline void DebugValue(s32 condition) { }

/* Creates opponent score boxes and icon groups during the sequence init hook. */
void fn_1_C838(void) {
    s32 playerIndex;
    s16 playerNo;
    M665PlayerWork *playerWork;
    HUSPR_GROUPID scoreBox;
    s16 spriteGroup;
    s32 member;

    playerIndex = 0;
    while (playerIndex < 3) {
        playerNo = lbl_1_bss_250.otherPlayers[playerIndex];
        playerWork = &lbl_1_bss_10[playerNo];
        scoreBox = MgScoreBoxCreateChar(152, 48, playerWork->character);
        MgScoreBoxTPLvlSet(scoreBox, 0.0f);
        MgScoreBoxPosSet(scoreBox, lbl_1_data_358[playerIndex],
                         420.0f);
        MgScoreBoxDispSet(scoreBox, 0);
        playerWork->effect.other.scoreGroup = scoreBox;
        lbl_1_bss_EA8.animation = HuSprAnimRead(
            HuDataSelHeapReadNum(DATANUM(DATA_m665, 36), M665_MEMORY_TAG, HEAP_MODEL));
        spriteGroup = HuSprGrpCreate(4);
        HuSprGrpPosSet(spriteGroup, lbl_1_data_358[playerIndex],
                       420.0f);
        member = 0;
        while (member < 3) {
            HuSprGrpMemberSet(spriteGroup, (s16) (member + 1),
                              HuSprCreate(lbl_1_bss_EA8.animation, 10, 0));
            HuSprPosSet(spriteGroup, (s16) (member + 1),
                        (f32) ((member << 5) - 16), 0.0f);
            HuSprAttrSet(spriteGroup, (s16) (member + 1), 4);
            HuSprScaleSet(spriteGroup, (s16) (member + 1), 0.75f,
                          0.75f);
            member++;
        }
        playerWork->effect.other.spriteGroup = spriteGroup;
        playerIndex++;
    }
}

/* Applies opponent input, separates nearby vehicles, and avoids fire positions for fn_1_112C0. */
f32 fn_1_CA20(Point3d *position, s16 playerNo, s16 buttonX, s16 stickY) {
    Point3d otherPosition;
    Point3d positionDelta;
    Point3d mainPosition;
    f32 verticalDelta;
    f32 distance;
    f32 distanceSquared;
    f32 frameTime;
    f32 depthSpeed;
    s32 index;
    M665PlayerWork *playerWork;
    M665PlayerWork *mainPlayerWork;
    M665PlayerWork *otherPlayerWork;

    depthSpeed = position->z;
    position->x += (f32) buttonX / 6.0f;
    position->z -= (f32) stickY / 6.0f;
    playerWork = &lbl_1_bss_10[playerNo];
    mainPlayerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    if (position->x < -840.0f) {
        position->x = -840.0f;
    }
    if (position->x > 840.0f) {
        position->x = 840.0f;
    }
    if (position->z > -50.0f) {
        position->z = -50.0f;
    }
    if (position->z < -1700.0f) {
        position->z = -1700.0f;
    }
    Hu3DModelPosGet(mainPlayerWork->vehicleModel, &mainPosition);
    if (mainPlayerWork->action == 2) {
        for (index = 0; index < 2; index++) {
            if (position->x < 50.0f + lbl_1_bss_2E8.firePosition[index][0] &&
                position->x > lbl_1_bss_2E8.firePosition[index][0] - 50.0f &&
                position->z < lbl_1_bss_2E8.firePosition[index][2]) {
                position->z += 5.0f;
            }
        }
    } else if (mainPlayerWork->action > 2 && mainPlayerWork->action <= 7) {
        for (index = 0; index < 2; index++) {
            if (position->x < 50.0f + lbl_1_bss_2E8.firePosition[index][0] &&
                position->x > lbl_1_bss_2E8.firePosition[index][0] - 50.0f &&
                position->z < lbl_1_bss_2E8.firePosition[index][2]) {
                if (position->x < lbl_1_bss_2E8.firePosition[index][0]) {
                    position->x -= 5.0f;
                } else {
                    position->x += 5.0f;
                }
            }
        }
    }
    if (mainPlayerWork->action >= 14 && mainPlayerWork->action <= 16 &&
        position->x < 250.0f + mainPosition.x &&
        position->x > mainPosition.x - 250.0f &&
        position->z < 600.0f + mainPosition.z) {
        position->z += 5.0f;
    }
    if (mainPlayerWork->action >= 9) {
        DebugValue(mainPlayerWork->action <= 12);
    }
    for (index = 0; index < 3; index++) {
        if (lbl_1_bss_250.otherPlayers[index] !=
            playerNo) {
            otherPlayerWork = &lbl_1_bss_10[lbl_1_bss_250.otherPlayers[index]];
            if (otherPlayerWork->status.other.state != 5) {
                Hu3DModelPosGet(otherPlayerWork->vehicleModel, &otherPosition);
                distanceSquared = PSVECSquareDistance(&otherPosition, position);
                distance = sqrtf(distanceSquared);
                if (distance < 250.0f) {
                    positionDelta.x = position->x - otherPosition.x;
                    positionDelta.y = 0.0f;
                    positionDelta.z = position->z - otherPosition.z;
                    positionDelta.x /= distance;
                    positionDelta.z /= distance;
                    distance = 250.0f - distance;
                    position->x += (positionDelta.x * distance) / 2.0f;
                    position->z += (positionDelta.z * distance) / 2.0f;
                    otherPosition.x -= (positionDelta.x * distance) / 2.0f;
                    otherPosition.z -= (positionDelta.z * distance) / 2.0f;
                    Hu3DModelPosSetV(otherPlayerWork->vehicleModel, &otherPosition);
                }
            }
        }
    }
    depthSpeed = (depthSpeed - position->z) / 4.0f;
    if (lbl_1_bss_250.scrollSpeed == 15.0f) {
        verticalDelta = (lbl_1_bss_250.scrollSpeed + depthSpeed) /
                lbl_1_bss_250.scrollSpeed;
    } else {
        frameTime = Hu3DMotionTimeGet(playerWork->vehicleModel);
        Hu3DModelAttrReset(playerWork->vehicleModel, HU3D_MOTATTR_LOOP);
        if (frameTime <= 15.0f) {
            Hu3DMotionStartEndSet(playerWork->vehicleModel, 0.0f, 15.0f);
        } else if (frameTime <= 45.0f) {
            Hu3DMotionStartEndSet(playerWork->vehicleModel, 15.0f, 45.0f);
        } else if (frameTime >= 59.0f) {
            Hu3DMotionTimeSet(playerWork->vehicleModel, 0.0f);
        }
        if (frameTime == 15.0f || frameTime == 45.0f) {
            position->z += lbl_1_bss_250.scrollSpeed;
        }
        verticalDelta = 1.0f;
    }
    return verticalDelta;
}

typedef struct ThresholdTable {
    s16 chancePercentages[4];
} ThresholdTable;

/* Rolls difficulty-based movement overrides for the three other players. */
void fn_1_D25C(void)
{
    ThresholdTable thresholds = {{20, 40, 60, 95}};
    s32 playerIndex;
    M665PlayerWork *playerWork;
    for (playerIndex = 0; playerIndex < 3; playerIndex++) {
        playerWork = &lbl_1_bss_10[lbl_1_bss_250.otherPlayers[playerIndex]];
        if ((playerWork->difficulty != -1) &&
            (frandmod(100) < thresholds.chancePercentages[playerWork->difficulty])) {
            playerWork->movementOverride = 1;
            playerWork->movementAngle = (-300.0f);
        }
    }
}

static const ThresholdTable thresholdsSource = {{5, 20, 60, 90}};

/* Tests a random roll against the difficulty threshold selected by objectData. */
s32 fn_1_D32C(s16 *objectData)
{
    s16 difficultyIndex = objectData[70];
    ThresholdTable thresholds = thresholdsSource;

    if (frandmod(100) < thresholds.chancePercentages[difficultyIndex]) {
        return 1;
    }
    return 0;
}

void fn_1_D3B4(s16 playerNo, s16 *buttonX, s16 *stickY) {
    *buttonX = 0;
    *stickY = 0;
}

f32 PSVECSquareDistance(const HuVecF *a, const HuVecF *b);

void PSVECNormalize(const HuVecF *source, HuVecF *destination);

/* Adds random steering drift when fn_1_10F18 invokes this selected movement callback. */
void fn_1_D3C8(s16 playerNo, s16 *buttonX, s16 *stickY) {
    s16 randomValue;
    M665PlayerWork *playerWork;

    playerWork = &lbl_1_bss_10[playerNo];
    randomValue = frandmod(3);
    if (randomValue == 0) {
        playerWork->movementAngle += 1.0f;
    } else if (randomValue == 1) {
        playerWork->movementAngle -= 1.0f;
    }
    *buttonX =
        (s16) (((f64) lbl_1_data_364[playerWork->difficulty] *
                (0.8 +
                 (f64) ((f32) frandmod(20) / 100.0f))) *
               sin((3.141592653589793 *
                    (f64) playerWork->movementAngle) /
                   180.0));
    *stickY =
        (s16) (((f64) lbl_1_data_364[playerWork->difficulty] *
                (0.8 +
                 (f64) ((f32) frandmod(20) / 100.0f))) *
               cos((3.141592653589793 *
                    (f64) playerWork->movementAngle) /
                   180.0));
}

/* Computes horizontal and depth input from the player's position. */
void fn_1_D620(s16 playerNo, s16 *horizontalInput, s16 *depthInput) {
    M665PlayerWork *playerWork;
    HuVecF position;

    playerWork = &lbl_1_bss_10[playerNo];
    Hu3DModelPosGet(playerWork->vehicleModel, &position);
    if (position.z > (-1400.0f)) {
        *depthInput = (s16) -((f64) lbl_1_data_364[playerWork->difficulty] *
                                  (0.8 +
                                   (f64) ((f32) frandmod(20) / 100.0f)));
    } else if (position.x > 0.0f) {
        *horizontalInput = (s16) (0.7 *
                                 ((f64) lbl_1_data_364[playerWork->difficulty] *
                                  (0.8 +
                                   (f64) ((f32) frandmod(20) / 100.0f))));
        *depthInput = (s16) (0.7 *
                                 ((f64) lbl_1_data_364[playerWork->difficulty] *
                                  (0.8 +
                                   (f64) ((f32) frandmod(20) / 100.0f))));
    } else {
        *horizontalInput = (s16) (0.7 *
                                 -((f64) lbl_1_data_364[playerWork->difficulty] *
                                   (0.8 +
                                    (f64) ((f32) frandmod(20) / 100.0f))));
        *depthInput = (s16) (0.7 *
                                 ((f64) lbl_1_data_364[playerWork->difficulty] *
                                  (0.8 +
                                   (f64) ((f32) frandmod(20) / 100.0f))));
    }
}

/* Avoids active missiles when fn_1_F7BC selects this movement strategy. */
void fn_1_DA0C(s16 playerNo, s16 *buttonX, s16 *stickY) {
    HuVecF objectPosition;
    HuVecF playerPosition;
    HuVecF positionDelta;
    f32 absoluteHorizontalDistance;
    f32 turnRate;
    f32 targetAngle;
    f32 angle;
    M665MissileWork *objectWork;
    M665PlayerWork *playerWork;
    s32 objectIndex;

    playerWork = &lbl_1_bss_10[playerNo];
    Hu3DModelPosGet(playerWork->vehicleModel, &playerPosition);
    if (playerPosition.z < (-500.0f)) {
        *buttonX = 0;
        *stickY = (s16) -((f64) lbl_1_data_364[playerWork->difficulty] *
                                 (0.8 +
                                  (f64) ((f32) frandmod(20) /
                                         100.0f)));
        return;
    } else {
        for (objectIndex = 0; objectIndex < 6; objectIndex++) {
            objectWork = &lbl_1_bss_2E8.missiles[objectIndex];
            if (objectWork->state == 2) {
                Hu3DModelPosGet(objectWork->modelID, &objectPosition);
                absoluteHorizontalDistance =
                    fabs((f64) (playerPosition.x - objectPosition.x));
                positionDelta.x = playerPosition.x - objectPosition.x;
                positionDelta.y = 0.0f;
                positionDelta.z = playerPosition.z - objectPosition.z;
                targetAngle = (f32) (180.0 *
                                     (atan2((f64) positionDelta.z,
                                            (f64) positionDelta.x) /
                                      3.141592653589793));
                angle = fabs((f64) (targetAngle - objectWork->angle));
                turnRate = (f32) ((f64) absoluteHorizontalDistance *
                                  sin((3.141592653589793 * (f64) angle) /
                                      180.0));
                if (turnRate > 145.0f) {
                    continue;
                }
                if (targetAngle > objectWork->angle) {
                    angle = objectWork->angle - 90.0f;
                } else {
                    angle = 90.0f + objectWork->angle;
                }
                *buttonX = (s16) (sin((3.141592653589793 *
                                              (f64) angle) /
                                             180.0) *
                                          ((f64) lbl_1_data_364[
                                               playerWork->difficulty] *
                                           (0.8 +
                                            (f64) ((f32) frandmod(20) /
                                                   100.0f))));
                *stickY = (s16) (cos((3.141592653589793 *
                                              (f64) angle) /
                                             180.0) *
                                          -((f64) lbl_1_data_364[
                                                 playerWork->difficulty] *
                                             (0.8 +
                                              (f64) ((f32) frandmod(20) /
                                                     100.0f))));
                return;
            }
        }
    }
}

/* Computes movement offsets from active work records when fn_1_F7BC selects this strategy. */
void fn_1_DEAC(s16 playerNo, s16 *buttonX, s16 *stickY) {
    HuVecF currentModelPosition;
    HuVecF currentPosition;
    HuVecF targetPosition;
    HuVecF direction;
    HuVecF modelRotation;
    M665PlayerWork *playerWork;
    M665KillerWork *killerWork;
    struct ButtonChanceTable {
        u16 chanceThresholds[4];
    };
    static const struct ButtonChanceTable lbl_1_rodata_4DC = {{500, 500, 500, 600}};
    struct ButtonChanceTable buttonChances;
    s32 killerIndex;
    f32 angleDifference;
    f32 distanceSquared;
    f32 distance;
    f32 targetAngle;
    f32 turnRate;

    playerWork = &lbl_1_bss_10[playerNo];
    buttonChances = lbl_1_rodata_4DC;
    for (killerIndex = 0; killerIndex < 3; killerIndex++) {
        killerWork = &lbl_1_bss_2E8.killers[killerIndex];
        if (killerWork->targetPlayer != -1 && killerWork->state != 0) {
            Hu3DModelPosGet(playerWork->vehicleModel, &currentPosition);
            if (killerWork->targetPlayer != playerNo) {
                Hu3DModelPosGet(
                    lbl_1_bss_10[killerWork->targetPlayer].vehicleModel,
                    &targetPosition);
                direction.x = currentPosition.x - targetPosition.x;
                direction.y = 0.0f;
                direction.z = currentPosition.z - targetPosition.z;
                PSVECNormalize(&direction, &direction);
                *buttonX = (s16) ((f64) direction.x *
                    ((f64) lbl_1_data_364[playerWork->difficulty] *
                     (0.8 +
                      (f64) ((f32) frandmod(20) / 100.0f))));
                *stickY = (s16) (-direction.z *
                    ((f64) lbl_1_data_364[playerWork->difficulty] *
                     (0.8 +
                      (f64) ((f32) frandmod(20) / 100.0f))));
                continue;
            }
            Hu3DModelPosGet(killerWork->modelID, &currentModelPosition);
            Hu3DModelRotGet(killerWork->modelID, &modelRotation);
            distanceSquared = PSVECSquareDistance(&currentModelPosition,
                                                  &currentPosition);
            distance = sqrtf(distanceSquared);
            direction.x = currentPosition.x - currentModelPosition.x;
            direction.y = 0.0f;
            /* The Z component uses the player's X coordinate against the killer's Z coordinate. */
            direction.z = currentPosition.x - currentModelPosition.z;
            targetAngle = (f32) (180.0 *
                (atan2((f64) direction.z, (f64) direction.x) /
                 3.141592653589793));
            angleDifference = targetAngle - killerWork->angle;
            while (angleDifference < 0.0f) {
                angleDifference += 360.0f;
            }
            while (angleDifference >= 360.0f) {
                angleDifference -= 360.0f;
            }
            if (distance > 675.0f) {
                turnRate = 45.0f;
            } else if (distance > 475.0f) {
                turnRate = 70.0f;
            } else if (distance > 275.0f) {
                turnRate = 90.0f;
            } else {
                turnRate = 110.0f;
            }
            if (angleDifference <= 180.0f) {
                targetAngle = killerWork->angle + turnRate;
            } else {
                targetAngle = killerWork->angle - turnRate;
            }
            *buttonX = (s16) (sin((3.141592653589793 *
                (f64) targetAngle) / 180.0) *
                ((f64) lbl_1_data_364[playerWork->difficulty] *
                 (0.8 +
                  (f64) ((f32) frandmod(20) / 100.0f))));
            *stickY = (s16) (cos((3.141592653589793 *
                (f64) targetAngle) / 180.0) *
                -((f64) lbl_1_data_364[playerWork->difficulty] *
                  (0.8 +
                   (f64) ((f32) frandmod(20) / 100.0f))));
            return;
        }
    }
}

/* Chooses a difficulty-scaled angle offset from the player's horizontal side. */
void fn_1_E57C(s16 playerNo, s16 *angle) {
    HuVecF position;
    M665PlayerWork *playerWork;

    playerWork = &lbl_1_bss_10[playerNo];
    Hu3DModelPosGet(playerWork->vehicleModel, &position);
    if (position.x > 0.0f) {
        *angle = (s16) ((f64) lbl_1_data_364[playerWork->difficulty] *
                        (0.8 +
                         (f64) ((f32) frandmod(20) / 100.0f)));
    } else {
        *angle = (s16) -((f64) lbl_1_data_364[playerWork->difficulty] *
                         (0.8 +
                          (f64) ((f32) frandmod(20) / 100.0f)));
    }
}

/* Uses nearby model positions to set movement inputs when called by fn_1_F7BC. */
void fn_1_E728(s16 playerNo, s16 *buttonX, s16 *stickY) {
    HuVecF objectPosition;
    HuVecF playerPosition;
    HuVecF direction;
    HuVecF nextPosition;
    f32 objectDistanceSquared;
    f32 playerDistanceSquared;
    f32 gapDistance;
    f32 angle;
    M665PlayerWork *playerWork;
    s32 objectIndex;
    s32 neighborIndex;
    s32 forwardIndex;
    s32 backwardIndex;
    M665Hazard *hazardWork;

    playerWork = &lbl_1_bss_10[playerNo];
    Hu3DModelPosGet(playerWork->vehicleModel, &playerPosition);
    if (playerPosition.z <= -400.0f) {
        *stickY = (s16) -((f64) lbl_1_data_364[playerWork->difficulty] *
            (0.8 + (f64) ((f32) frandmod(20) / 100.0f)));
        return;
    }

    for (objectIndex = 0; objectIndex < 7; objectIndex++) {
        hazardWork = &lbl_1_bss_2E8.hazards[objectIndex];
        if (hazardWork->state <= 3) {
            Hu3DModelPosGet(hazardWork->model, &objectPosition);
        } else {
            Hu3DModelPosGet(hazardWork->firstAnimation, &objectPosition);
        }
        objectPosition.y = 0.0f;
        objectDistanceSquared = PSVECSquareDistance(&objectPosition,
                                                     &playerPosition);
        if (sqrtf(objectDistanceSquared) < 210.0f) {
            if (objectPosition.z < playerPosition.z) {
                *stickY = (s16) -((f64) lbl_1_data_364[playerWork->difficulty] *
                    (0.8 + (f64) ((f32) frandmod(20) / 100.0f)));
            } else {
                *stickY = (s16) ((f64) lbl_1_data_364[playerWork->difficulty] *
                    (0.8 + (f64) ((f32) frandmod(20) / 100.0f)));
            }
        }

        if (playerPosition.x - 125.0f < objectPosition.x + 85.0f &&
            playerPosition.x + 125.0f > objectPosition.x - 85.0f &&
            playerPosition.z > objectPosition.z) {
            for (neighborIndex = 0; neighborIndex < 7; neighborIndex++) {
                forwardIndex = objectIndex + neighborIndex;
                backwardIndex = objectIndex - neighborIndex;
                if (forwardIndex <= 6 && backwardIndex >= 0) {
        gapDistance =
                        lbl_1_bss_280[forwardIndex].gapDistance >
                        lbl_1_bss_280[backwardIndex].gapDistance
                            ? lbl_1_bss_280[forwardIndex].gapDistance
                            : lbl_1_bss_280[backwardIndex].gapDistance;
        if (gapDistance > 450.0f) {
                        playerWork->selectedStage =
                            lbl_1_bss_280[forwardIndex].gapDistance >
                            lbl_1_bss_280[backwardIndex].gapDistance
                                ? forwardIndex : backwardIndex;
                        break;
                    }
                } else if (forwardIndex <= 6) {
                    if (lbl_1_bss_280[forwardIndex].gapDistance > 450.0f) {
                        playerWork->selectedStage = forwardIndex;
                        break;
                    }
                } else if (backwardIndex >= 0 &&
                           lbl_1_bss_280[backwardIndex].gapDistance > 450.0f) {
                    playerWork->selectedStage = backwardIndex;
                    break;
                }
            }
        } else {
            continue;
        }
        break;
    }

    if (playerWork->selectedStage != -1) {
        Hu3DModelPosGet(
            lbl_1_bss_2E8.hazards[playerWork->selectedStage].firstAnimation,
            &direction);
        Hu3DModelPosGet(
            lbl_1_bss_2E8.hazards[playerWork->selectedStage + 1].firstAnimation,
            &nextPosition);
        if (lbl_1_bss_280[playerWork->selectedStage].depthDistance >= 0.0f) {
            if (playerPosition.z + 125.0f >= nextPosition.z - 85.0f) {
                if (nextPosition.x - 85.0f < playerPosition.x + 125.0f) {
                    *buttonX = (s16) -((f64) lbl_1_data_364[playerWork->difficulty] *
                        (0.8 + (f64) ((f32) frandmod(20) / 100.0f)));
                    return;
                }
                if (playerPosition.x < direction.x) {
                    *buttonX = (s16) ((f64) lbl_1_data_364[playerWork->difficulty] *
                        (0.8 + (f64) ((f32) frandmod(20) / 100.0f)));
                }
            } else if (playerPosition.x - 125.0f < direction.x + 85.0f) {
                *buttonX = (s16) ((f64) lbl_1_data_364[playerWork->difficulty] *
                    (0.8 + (f64) ((f32) frandmod(20) / 100.0f)));
            }
        } else if (playerPosition.z + 125.0f > direction.z - 85.0f) {
            if (direction.x + 85.0f > playerPosition.x - 125.0f) {
                *buttonX = (s16) ((f64) lbl_1_data_364[playerWork->difficulty] *
                    (0.8 + (f64) ((f32) frandmod(20) / 100.0f)));
                return;
            }
            if (playerPosition.x > nextPosition.x) {
                *buttonX = (s16) -((f64) lbl_1_data_364[playerWork->difficulty] *
                    (0.8 + (f64) ((f32) frandmod(20) / 100.0f)));
            }
        } else if (playerPosition.x + 125.0f > nextPosition.x - 85.0f) {
            *buttonX = (s16) -((f64) lbl_1_data_364[playerWork->difficulty] *
                (0.8 + (f64) ((f32) frandmod(20) / 100.0f)));
        }
    } else {
        for (objectIndex = 0; objectIndex < 3; objectIndex++) {
            if (lbl_1_bss_250.otherPlayers[objectIndex] !=
                playerNo) {
                Hu3DModelPosGet(lbl_1_bss_10[lbl_1_bss_250.otherPlayers[objectIndex]].vehicleModel,
                    &direction);
                playerDistanceSquared = PSVECSquareDistance(&playerPosition,
                                                             &direction);
                if (sqrtf(playerDistanceSquared) < 500.0f) {
                    direction.x = direction.x - playerPosition.x;
                    direction.y = 0.0f;
                    direction.z = direction.z - playerPosition.z;
                    PSVECNormalize(&direction, &direction);
                    angle = (f32) (180.0 * (atan2((f64) direction.z,
                                                  (f64) direction.x) /
                                               3.141592653589793));
                    *buttonX = (s16) (
                        (f64) lbl_1_data_364[playerWork->difficulty] *
                        (0.8 + (f64) ((f32) frandmod(20) / 100.0f)) *
                        sin((3.141592653589793 * (f64) angle) / 180.0));
                    *stickY = (s16) (-(
                        (f64) lbl_1_data_364[playerWork->difficulty] *
                        (0.8 + (f64) ((f32) frandmod(20) / 100.0f))) *
                        cos((3.141592653589793 * (f64) angle) / 180.0));
                }
            }
        }
    }
}

void fn_1_DA0C(s16 playerNo, s16 *buttonX, s16 *stickY);

void fn_1_DEAC(s16 playerNo, s16 *buttonX, s16 *stickY);

void fn_1_E728(s16 playerNo, s16 *buttonX, s16 *stickY);

/* Chooses a difficulty-scaled angle offset from the player's horizontal side. */
void fn_1_F610(s16 playerNo, s16 *angle) {
    HuVecF position;
    M665PlayerWork *playerWork;

    playerWork = &lbl_1_bss_10[playerNo];
    Hu3DModelPosGet(playerWork->vehicleModel, &position);
    if (position.x > 0.0f) {
        *angle = (s16) ((f64) lbl_1_data_364[playerWork->difficulty] *
                        (0.8 +
                         (f64) ((f32) frandmod(20) / 100.0f)));
    } else {
        *angle = (s16) -((f64) lbl_1_data_364[playerWork->difficulty] *
                         (0.8 +
                          (f64) ((f32) frandmod(20) / 100.0f)));
    }
}

/* Sets horizontal input from the player's side while fn_1_F7BC selects movement. */
static inline void horizontalInputFromX(s16 playerNo, s16 *horizontalInput) {
    HuVecF position;
    M665PlayerWork *playerWork;

    playerWork = &lbl_1_bss_10[playerNo];
    Hu3DModelPosGet(playerWork->vehicleModel, &position);
    if (position.x > 0.0f) {
        *horizontalInput = (s16) ((f64) lbl_1_data_364[playerWork->difficulty] *
                        (0.8 +
                         (f64) ((f32) frandmod(20) / 100.0f)));
    } else {
        *horizontalInput = (s16) -((f64) lbl_1_data_364[playerWork->difficulty] *
                         (0.8 +
                          (f64) ((f32) frandmod(20) / 100.0f)));
    }
}

/* Chooses movement outputs from the main player's action and active effects for fn_1_10F18. */
void fn_1_F7BC(s16 playerNo, s16 *buttonX, s16 *stickY) {
    s32 index;
    HuVecF windPosition;
    M665PlayerWork *mainPlayerWork;
    s16 mainPlayerMode;
    M665PlayerWork *playerWork;
    s16 hasNonzeroKillerState;
    s16 hasRollingState3;
    s16 hasNonzeroHazardState;

    mainPlayerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    mainPlayerMode = mainPlayerWork->action;
    hasNonzeroKillerState = 0;
    for (index = 0; index < 3; index++) {
        if (lbl_1_bss_2E8.killers[index].state != 0) {
            hasNonzeroKillerState = 1;
            break;
        }
    }
    hasRollingState3 = 0;
    for (index = 0; index < 10; index++) {
        if (lbl_1_bss_2E8.rolling[index].state == 3) {
            hasRollingState3 = 1;
            break;
        }
    }
    hasNonzeroHazardState = 0;
    for (index = 0; index < 7; index++) {
        if (lbl_1_bss_2E8.hazards[index].state != 0) {
            hasNonzeroHazardState = 1;
            break;
        }
    }
    if (mainPlayerMode >= 1 && mainPlayerMode <= 7) {
        playerWork = &lbl_1_bss_10[playerNo];
        Hu3DModelPosGet(playerWork->vehicleModel, &windPosition);
        if (windPosition.z > -1400.0f) {
            *stickY = (s16)-((f64)lbl_1_data_364[playerWork->difficulty] *
                            (0.8 + (f64)((f32)frandmod(20) / 100.0f)));
            return;
        }
        if (windPosition.x > 0.0f) {
            *buttonX = (s16)(0.7 * ((f64)lbl_1_data_364[playerWork->difficulty] *
                                  (0.8 + (f64)((f32)frandmod(20) / 100.0f))));
            *stickY = (s16)(0.7 * ((f64)lbl_1_data_364[playerWork->difficulty] *
                                  (0.8 + (f64)((f32)frandmod(20) / 100.0f))));
            return;
        }
        *buttonX = (s16)(0.7 * -((f64)lbl_1_data_364[playerWork->difficulty] *
                               (0.8 + (f64)((f32)frandmod(20) / 100.0f))));
        *stickY = (s16)(0.7 * ((f64)lbl_1_data_364[playerWork->difficulty] *
                              (0.8 + (f64)((f32)frandmod(20) / 100.0f))));
        return;
    }
    else if (mainPlayerMode >= 8 && mainPlayerMode <= 12) {
    horizontalInputFromX(playerNo, buttonX);
    }
    else if (mainPlayerMode >= 13 && mainPlayerMode <= 16) {
        fn_1_DA0C(playerNo, buttonX, stickY);
        return;
    }
    else if ((mainPlayerMode >= 17 && mainPlayerMode <= 22) || hasNonzeroHazardState != 0) {
        fn_1_E728(playerNo, buttonX, stickY);
        return;
    }
    else if ((mainPlayerMode >= 23 && mainPlayerMode <= 27) || hasNonzeroKillerState != 0) {
        fn_1_DEAC(playerNo, buttonX, stickY);
        return;
    }
    else if ((mainPlayerMode >= 28 && mainPlayerMode <= 32) || hasRollingState3 != 0) {
    horizontalInputFromX(playerNo, buttonX);
    }
}

/* Sets side-dependent movement outputs when selected by fn_1_10F18. */
void fn_1_1005C(s16 playerNo, s16 *buttonX, s16 *stickY) {
    M665PlayerWork *playerWork;
    HuVecF position;

    playerWork = &lbl_1_bss_10[playerNo];
    Hu3DModelPosGet(playerWork->vehicleModel, &position);
    if (position.x > 0.0f) {
        *buttonX = (s16) (0.7 * ((f64) lbl_1_data_364[playerWork->difficulty] *
                                    (0.8 + (f64) ((f32) frandmod(20) / 100.0f))));
        *stickY = (s16) (0.7 * -((f64) lbl_1_data_364[playerWork->difficulty] *
                                    (0.8 + (f64) ((f32) frandmod(20) / 100.0f))));
    } else {
        *buttonX = (s16) (0.7 * -((f64) lbl_1_data_364[playerWork->difficulty] *
                                    (0.8 + (f64) ((f32) frandmod(20) / 100.0f))));
        *stickY = (s16) (0.7 * -((f64) lbl_1_data_364[playerWork->difficulty] *
                                    (0.8 + (f64) ((f32) frandmod(20) / 100.0f))));
    }
}

void fn_1_D3B4(s16 playerNo, s16 *buttonX, s16 *stickY);

void fn_1_D3C8(s16 playerNo, s16 *buttonX, s16 *stickY);

void fn_1_10394(s16 playerNo, s16 *buttonX, s16 *stickY);

void fn_1_1005C(s16 playerNo, s16 *buttonX, s16 *stickY);

void fn_1_109AC(s16 playerNo, s16 *horizontalSpeed, s16 *verticalSpeed);

/* Steers toward the central front boundary, but farther outward in X when already behind it. */
void fn_1_10394(s16 playerNo, s16 *buttonX, s16 *stickY) {
    HuVecF playerPosition;
    HuVecF velocityDirection;
    M665PlayerWork *playerWork;
    playerWork = &lbl_1_bss_10[playerNo];
    Hu3DModelPosGet(playerWork->vehicleModel, &playerPosition);
    if (playerPosition.x < (-300.0f)) {
        if (playerPosition.z > (-600.0f)) {
            velocityDirection.x = (-300.0f) - playerPosition.x;
            velocityDirection.y = 0.0f;
            velocityDirection.z = (-600.0f) - playerPosition.z;
            PSVECNormalize(&velocityDirection, &velocityDirection);
            *buttonX = (s16) ((f64) velocityDirection.x *
                ((f64) lbl_1_data_364[playerWork->difficulty] *
                 (0.8 +
                  (f64) ((f32) frandmod(20) / 100.0f))));
            *stickY = (s16) ((f64) velocityDirection.z *
                -((f64) lbl_1_data_364[playerWork->difficulty] *
                  (0.8 +
                   (f64) ((f32) frandmod(20) / 100.0f))));
            return;
        }
        *buttonX = (s16) -((f64) lbl_1_data_364[playerWork->difficulty] *
            (0.8 +
             (f64) ((f32) frandmod(20) / 100.0f)));
        *stickY = 0;
        return;
    }
    if (playerPosition.x > 300.0f) {
        if (playerPosition.z > (-600.0f)) {
            velocityDirection.x = 300.0f - playerPosition.x;
            velocityDirection.y = 0.0f;
            velocityDirection.z = (-600.0f) - playerPosition.z;
            PSVECNormalize(&velocityDirection, &velocityDirection);
            *buttonX = (s16) ((f64) velocityDirection.x *
                ((f64) lbl_1_data_364[playerWork->difficulty] *
                 (0.8 +
                  (f64) ((f32) frandmod(20) / 100.0f))));
            *stickY = (s16) ((f64) velocityDirection.z *
                -((f64) lbl_1_data_364[playerWork->difficulty] *
                  (0.8 +
                   (f64) ((f32) frandmod(20) / 100.0f))));
            return;
        }
        *buttonX = (s16) ((f64) lbl_1_data_364[playerWork->difficulty] *
            (0.8 +
             (f64) ((f32) frandmod(20) / 100.0f)));
        *stickY = 0;
        return;
    }
    if (playerPosition.z > (-600.0f)) {
        *buttonX = 0;
        *stickY = (s16) ((f64) lbl_1_data_364[playerWork->difficulty] *
            (0.8 +
             (f64) ((f32) frandmod(20) / 100.0f)));
        return;
    }
    *buttonX = *stickY = 0;
}

/* Keeps opponents near the main vehicle during fn_1_10F18's movement update. */
void fn_1_109AC(s16 playerNo, s16 *horizontalSpeed, s16 *verticalSpeed) {
    HuVecF playerPosition;
    HuVecF leaderPosition;
    M665PlayerWork *playerWork;

    playerWork = &lbl_1_bss_10[playerNo];
    Hu3DModelPosGet(
        lbl_1_bss_10[lbl_1_bss_250.mainPlayer].vehicleModel,
        &leaderPosition);
    Hu3DModelPosGet(playerWork->vehicleModel, &playerPosition);
    if (playerPosition.x < leaderPosition.x - 350.0f) {
        *horizontalSpeed = (s16) (lbl_1_data_364[playerWork->difficulty] *
            (0.8 + (f64) ((f32) frandmod(20) / 100.0f)));
    } else if (playerPosition.x > 350.0f + leaderPosition.x) {
        *horizontalSpeed = (s16) -(lbl_1_data_364[playerWork->difficulty] *
            (0.8 + (f64) ((f32) frandmod(20) / 100.0f)));
    } else {
        *horizontalSpeed = 0;
    }
    if (playerPosition.z > (-500.0f)) {
        *verticalSpeed = (s16) (
            sin((3.141592653589793 * (f64) frandmod(90)) /
                180.0) *
            ((f64) lbl_1_data_364[playerWork->difficulty] *
             (0.8 + (f64) ((f32) frandmod(20) / 100.0f))) / 2);
    }
}

/* Chooses a difficulty-weighted movement callback when fn_1_10F18's timer expires. */
void fn_1_10CE8(s16 playerNo) {
    M665MovementCallback movementFunctions[5] = {
        fn_1_D3B4,
        fn_1_D3C8,
        fn_1_10394,
        fn_1_1005C,
        fn_1_109AC,
    };
    M665PlayerWork *playerWork;
    s16 randomValue;
    s32 selectionIndex;
    s16 movementIndex;

    playerWork = &lbl_1_bss_10[playerNo];
    {
        s16 delayLimits[4] = { 180, 150, 120, 60 };
        s16 selectionLimits[4][5] = {
            { 20, 50, 70, 80, 100 },
            { 20, 50, 65, 80, 100 },
            { 10, 20, 20, 25, 100 },
            { 5, 10, 10, 15, 100 },
        };

        randomValue = (s16)frandmod(100);
        for (selectionIndex = 0; selectionIndex < 5; selectionIndex++) {
            if (randomValue <=
                selectionLimits[playerWork->difficulty][selectionIndex]) {
                break;
            }
        }
        if (selectionIndex < 5) {
            movementIndex = selectionIndex;
        } else {
            movementIndex = 0;
        }
        playerWork->movement = movementFunctions[movementIndex];
        playerWork->movementIndex = movementIndex;
        playerWork->movementFrames =
            delayLimits[playerWork->difficulty] +
            frandmod(delayLimits[playerWork->difficulty]);
        if (movementIndex == 1) {
            playerWork->movementAngle = (f32)frandmod(360);
        }
    }
}

u32 MgSeqModeGet(void);

void fn_1_F7BC(s16 playerNo, s16 *buttonX, s16 *stickY);

f32 fn_1_CA20(Point3d *position, s16 playerNo, s16 buttonX, s16 stickY);

extern s32 fn_1_4FA4(HuVecF *position);

extern s32 fn_1_50D4();

extern void fn_1_13A44(s16 playerNo);

extern void fn_1_11FD0(M665PlayerWork *work);

/* Updates one AI opponent's movement outputs from its object-manager callback. */
void fn_1_10F18(s16 playerNo, s16 *buttonX, s16 *stickY) {
    M665PlayerWork *playerWork;
    s16 sequenceMode;
    s32 workIndex;
    s16 hasNonzeroKillerState;
    s16 hasRollingState3;
    s16 hasNonzeroHazardState;

    playerWork = &lbl_1_bss_10[playerNo];
    sequenceMode = MgSeqModeGet();
    if (sequenceMode != 5) {
        *buttonX = *stickY = 0;
        return;
    }
    if (playerWork->movement == NULL ||
        (playerWork->movementFrames)-- <= 0) {
        fn_1_10CE8(playerNo);
    }

    *buttonX = *stickY = 0;
    hasNonzeroKillerState = 0;
    workIndex = 0;
    while (workIndex < 3) {
        if (lbl_1_bss_2E8.killers[workIndex].state !=
            0) {
            hasNonzeroKillerState = 1;
            break;
        }
        workIndex++;
    }
    hasRollingState3 = 0;
    workIndex = 0;
    while (workIndex < 10) {
        if (lbl_1_bss_2E8.rolling[workIndex].state ==
            3) {
            hasRollingState3 = 1;
            break;
        }
        workIndex++;
    }
    hasNonzeroHazardState = 0;
    workIndex = 0;
    while (workIndex < 7) {
        if (lbl_1_bss_2E8.hazards[workIndex].state !=
            0) {
            hasNonzeroHazardState = 1;
            break;
        }
        workIndex++;
    }
    if (playerWork->movementOverride != 0 || hasNonzeroKillerState != 0 ||
        hasRollingState3 != 0 || hasNonzeroHazardState != 0) {
        fn_1_F7BC(playerNo, buttonX, stickY);
        return;
    }
    playerWork->selectedStage = -1;
    (playerWork->movement)(playerNo, buttonX, stickY);
}

/* Updates a player's input, movement, and hit reactions during each object update. */
void fn_1_112C0(s16 playerNo) {
    HuVecF position;
    HuVecF modelPosition;
    HuVecF rotation;
    M665PlayerWork *playerWork;
    s16 actionInput;
    ThresholdTable buttonChancesA;
    ThresholdTable buttonChancesB;
    s16 buttonX;
    s16 stickY;
    s16 chanceIndexA;
    s16 chanceIndexB;
    f32 motionSpeed;
    f32 positionX;
    f32 absolutePositionValue;
    f64 absolutePositionX;

    playerWork = &lbl_1_bss_10[playerNo];
    if (lbl_1_bss_250.finished == 0 && MgSeqModeGet() >= 5U) {
        Hu3DModelPosGet(playerWork->vehicleModel, &position);
        if (playerWork->difficulty == -1) {
            buttonX = HuPadStkX[playerWork->controller];
            stickY = HuPadStkY[playerWork->controller];
        } else {
            fn_1_10F18(playerNo, &buttonX, &stickY);
        }
        if (MgSeqModeGet() != 5U) {
            buttonX = stickY = 0;
        }
        switch (playerWork->status.other.state) {
        case 1:
            motionSpeed = fn_1_CA20(&position, playerNo, buttonX, stickY);
            Hu3DMotionSpeedSet(playerWork->vehicleModel, motionSpeed);
            playerWork->motionSpeed = motionSpeed;
            if ((s16) fn_1_4FA4(&position) != 0) {
                fn_1_11FD0(playerWork);
            }
            if (fn_1_50D4(playerNo) != 0) {
                fn_1_11FD0(playerWork);
            }
            if (playerWork->action == 0) {
                if (playerWork->difficulty == -1) {
                    actionInput = HuPadBtn[playerWork->controller] & 0x100;
                } else {
                    chanceIndexA = playerWork->difficulty;
                    buttonChancesA = thresholdsSource;
                    actionInput =
                        frandmod(100) < buttonChancesA.chancePercentages[chanceIndexA] ? 1 : 0;
                }
                if (actionInput != 0 && MgSeqModeGet() == 5U) {
                    fn_1_13A44(playerNo);
                }
            } else {
                if (playerWork->action <= 10 &&
                    (HuPadBtnDown[playerWork->controller] & 0x100) != 0 &&
                    MgSeqModeGet() == 5U) {
                    fn_1_13A44(playerNo);
                }
                playerWork->action -= 1;
            }
            break;
        case 2:
            if ((playerWork->models.other.recoveryFrames)-- <= 0) {
                playerWork->models.other.recoveryFrames = 120;
                playerWork->status.other.state = 3;
            } else if ((playerWork->models.other.recoveryFrames & 1) != 0) {
                Hu3DModelAttrSet(playerWork->vehicleModel, HU3D_ATTR_DISPOFF);
            } else {
                Hu3DModelAttrReset(playerWork->vehicleModel, HU3D_ATTR_DISPOFF);
            }
            break;
        case 3:
            if ((playerWork->models.other.recoveryFrames)-- <= 0) {
                playerWork->status.other.state = 1;
                Hu3DModelAttrReset(playerWork->vehicleModel, HU3D_ATTR_DISPOFF);
            } else {
                if ((playerWork->models.other.recoveryFrames & 1) != 0) {
                    Hu3DModelAttrSet(playerWork->vehicleModel, HU3D_ATTR_DISPOFF);
                } else {
                    Hu3DModelAttrReset(playerWork->vehicleModel, HU3D_ATTR_DISPOFF);
                }
                motionSpeed = fn_1_CA20(&position, playerNo, buttonX, stickY);
                Hu3DMotionSpeedSet(playerWork->vehicleModel, motionSpeed);
                playerWork->motionSpeed = motionSpeed;
                if (playerWork->action == 0) {
                    if (playerWork->difficulty == -1) {
                        actionInput = HuPadBtn[playerWork->controller] & 0x100;
                    } else {
                        chanceIndexB = playerWork->difficulty;
                        buttonChancesB = thresholdsSource;
                        actionInput =
                            frandmod(100) < buttonChancesB.chancePercentages[chanceIndexB] ? 1 : 0;
                    }
                    if (actionInput != 0 && MgSeqModeGet() == 5U) {
                        fn_1_13A44(playerNo);
                    }
                } else {
                    if (playerWork->action <= 10 &&
                        (HuPadBtnDown[playerWork->controller] & 0x100) != 0 &&
                        MgSeqModeGet() == 5U) {
                        fn_1_13A44(playerNo);
                    }
                    playerWork->action -= 1;
                }
            }
            break;
        case 5:
            playerWork->models.other.recoveryFrames += 1;
            if (playerWork->models.other.recoveryFrames % 8 == 4) {
                Hu3DModelAttrSet(playerWork->vehicleModel, HU3D_ATTR_DISPOFF);
                Hu3DModelAttrSet(playerWork->characterModel, HU3D_ATTR_DISPOFF);
            } else {
                Hu3DModelAttrReset(playerWork->vehicleModel, HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(playerWork->characterModel, HU3D_ATTR_DISPOFF);
            }
            Hu3DModelRotGet(playerWork->vehicleModel, &rotation);
            if (position.x > 0.0f) {
                position.x += 20.0f;
                rotation.y += 0.5f;
            } else {
                position.x -= 20.0f;
                rotation.y -= 0.5f;
            }
            rotation.x += 1.0f;
            Hu3DModelRotSetV(playerWork->vehicleModel, &rotation);
            position.z += 20.0f;
            position.y += playerWork->speed;
            if (playerWork->speed > 0.0f) {
                playerWork->speed -= 0.5f;
            }
            positionX = position.x;
            absolutePositionValue = __fabsf(positionX);
            absolutePositionX = absolutePositionValue;
            if (absolutePositionX > 1500.0) {
                playerWork->status.other.state = 6;
            }
            Hu3DModelObjPosGet(playerWork->vehicleModel, &lbl_1_data_36C[0], &modelPosition);
            Hu3DModelPosSetV(playerWork->characterModel, &modelPosition);
            break;
        }
        Hu3DModelPosSetV(playerWork->vehicleModel, &position);
    }
}

typedef struct {
    char *hookNames[2];
} HookNames;

/* Advances detached vehicle models for an opponent during its object update. */
void fn_1_11920(s16 playerNo) {
    static const HookNames lbl_1_rodata_570 = {{lbl_1_data_375, lbl_1_data_380}};
    HuVecF position;
    HuVecF rotation;
    s32 hookIndex;
    HookNames hookNames;
    M665PlayerWork *playerWork;
    M665DetachedModel *effect;

    playerWork = &lbl_1_bss_10[playerNo];
    hookNames = lbl_1_rodata_570;
    hookIndex = 0;
    while (hookIndex < 2) {
        effect = &playerWork->models.other.effects[hookIndex];
        if (effect->state != 0 && effect->state != 3) {
            switch (effect->state) {
            case 1:
                Hu3DModelObjPosGet(playerWork->vehicleModel, hookNames.hookNames[hookIndex],
                                   &position);
                Hu3DModelHookObjReset(playerWork->vehicleModel, hookNames.hookNames[hookIndex]);
                Hu3DModelPosSetV(effect->model, &position);
                Hu3DModelShadowSet(effect->model);
                effect->velocityX =
                    (-2.0f) + 4.0f * (f32) hookIndex;
                effect->velocityY = 10.0f;
                effect->velocityZ = 25.0f;
                effect->state = 2;
                break;
            case 2:
                effect->blinkFrames += 1;
                if ((effect->blinkFrames & 1) != 0) {
                    Hu3DModelAttrSet(effect->model, HU3D_ATTR_DISPOFF);
                } else {
                    Hu3DModelAttrReset(effect->model, HU3D_ATTR_DISPOFF);
                }
                Hu3DModelPosGet(effect->model, &position);
                Hu3DModelRotGet(effect->model, &rotation);
                position.x += effect->velocityX;
                position.y += effect->velocityY;
                position.z += effect->velocityZ;
                rotation.x += 1.0f;
                effect->velocityY -= 1.5f;
                if (position.y <= 0.0f) {
                    effect->velocityX *= 0.5f;
                    effect->velocityY *= (-0.5f);
                    effect->velocityZ *= 0.5f;
                    if (effect->velocityY < 2.0f) {
                        effect->velocityY = 0.0f;
                        position.y = 0.0f;
                        effect->state = 3;
                        Hu3DModelAttrSet(effect->model, HU3D_ATTR_DISPOFF);
                    }
                }
                Hu3DModelPosSetV(effect->model, &position);
                Hu3DModelRotSetV(effect->model, &rotation);
                break;
            }
        }
        hookIndex++;
    }
}

typedef struct SoundIds {
    s32 soundID;
    s32 second; /* Second sound ID, selected by index from this three-entry table. */
    s32 third; /* Third sound ID, selected by index from this three-entry table. */
} SoundIds;

const SoundIds lbl_1_rodata_590 = {0x860, 0x861, 0x862};

void fn_1_112C0(s16 playerNo);

void fn_1_11920(s16 playerNo);

void fn_1_13024(void);

/* Plays motion-timed sounds and updates opponent and hit effects on each call. */
void fn_1_11C04(void) {
    HuVecF position;
    SoundIds soundIds;
    s16 playerNo;
    M665PlayerWork *playerWork;
    s32 playerIndex;
    f32 motionTime;

    soundIds = lbl_1_rodata_590;
    for (playerIndex = 0; playerIndex < 3; playerIndex++) {
        playerNo = lbl_1_bss_250.otherPlayers[playerIndex];
        playerWork = &lbl_1_bss_10[playerNo];
        Hu3DModelPosGet(playerWork->vehicleModel, &position);
        if ((position.z < 100.0f) &&
            (MgSeqModeGet() <= 5U)) {
            motionTime = Hu3DMotionTimeGet(playerWork->vehicleModel);
            if (((motionTime > 14.0f) &&
                 (motionTime <= (14.0f +
                                 playerWork->motionSpeed))) ||
                ((motionTime > 44.0f) &&
                 (motionTime <= (44.0f +
                                 playerWork->motionSpeed)))) {
                HuAudFXPlayPan((&soundIds.soundID)[playerWork->groupIndex],
                               (s16) ((s32) (position.x / 40.0f) + 64));
            }
        }
        fn_1_112C0(playerNo);
        fn_1_11920(playerNo);
    }
    fn_1_13024();
}

void fn_1_11D98(s16 playerIndex, s16 mode, f32 frame)
{
    fn_1_170C(lbl_1_bss_250.otherPlayers[playerIndex], mode, frame);
}

/* Marks all opponents active from the sequence main hook. */
void fn_1_11DE4(void)
{
    s32 player;

    for (player = 0; player < 3; player++) {
        s16 playerNo = lbl_1_bss_250.otherPlayers[player];
        M665PlayerWork *playerWork = &lbl_1_bss_10[playerNo];

        playerWork->status.other.state = 1;
    }
}

void fn_1_11E4C(void) {
}

void omVibrate(s16 player, s16 duration, s16 intensity, s16 decay);

/* Moves opponents to their fade-in positions and reports when all have arrived. */
s32 fn_1_11E50(void) {
    HuVecF position;
    M665PlayerWork *playerWork;
    s32 playerIndex;
    s16 playerNo;

    playerIndex = 0;
    while (playerIndex < 3) {
        playerNo = lbl_1_bss_250.otherPlayers[playerIndex];
        playerWork = &lbl_1_bss_10[playerNo];
        Hu3DModelPosGet(playerWork->vehicleModel, &position);
        position.z -= 16.666666f;
        if (position.z <= lbl_1_data_34C[playerIndex]) {
            position.z = lbl_1_data_34C[playerIndex];
            Hu3DMotionSpeedSet(playerWork->vehicleModel, 1.0f);
            playerWork->motionSpeed = 1.0f;
        }
        Hu3DModelPosSetV(playerWork->vehicleModel, &position);
        playerWork->position[0] = position.x;
        playerWork->position[1] = position.y;
        playerWork->position[2] = position.z;
        playerIndex++;
    }
    playerIndex = 0;
    while (playerIndex < 3) {
        playerNo = lbl_1_bss_250.otherPlayers[playerIndex];
        playerWork = &lbl_1_bss_10[playerNo];
        if (playerWork->position[2] > lbl_1_data_34C[playerIndex]) {
            return 0;
        }
        playerIndex++;
    }
    return 1;
}

const SoundIds lbl_1_rodata_5AC = {0x86A, 0x86B, 0x86C};

/* Handles an opponent collision after fn_1_112C0 detects it. */
void fn_1_11FD0(M665PlayerWork *playerWork) {
    HuVecF modelPosition;
    SoundIds soundIDs;
    HuVecF objectPosition;
    s16 sequenceMode;
    s32 memberIndex;

    sequenceMode = (s16) MgSeqModeGet();
    soundIDs = lbl_1_rodata_5AC;
    if (playerWork->status.other.state == 1) {
        if (playerWork->models.other.damageCount < 2) {
            Hu3DModelPosGet(playerWork->vehicleModel, &modelPosition);
            HuAudFXPlayPan((&soundIDs.soundID)[playerWork->groupIndex],
                           (s16) ((s32) (modelPosition.x / 40.0f) + 64));
            playerWork->status.other.state = 2;
            playerWork->models.other.recoveryFrames = 60;
            if (sequenceMode == 5) {
                omVibrate(playerWork->playerIndex, 20, 7, 3);
                playerWork->models.other.effects[playerWork->models.other.damageCount].state = 1;
                HuSprAttrSet(playerWork->effect.other.spriteGroup,
                             3 - playerWork->models.other.damageCount, 4);
                playerWork->models.other.damageCount++;
            }
        } else if (sequenceMode == 5) {
            s16 playerNumber;

            Hu3DModelObjPosGet(playerWork->vehicleModel, lbl_1_data_36C, &objectPosition);
            Hu3DModelHookObjReset(playerWork->vehicleModel, lbl_1_data_36C);
            Hu3DModelPosSetV(playerWork->characterModel, &objectPosition);
            playerWork->status.other.state = 5;
            playerNumber = playerWork->groupIndex;
            fn_1_170C((&lbl_1_bss_250.otherPlayers[0])[playerNumber], 6, 8.0f);
            omVibrate(playerWork->playerIndex, 60, 20, 0);
            Hu3DMotionSpeedSet(playerWork->vehicleModel, 0.0f);
            playerWork->motionSpeed = 0.0f;
            HuSprAttrSet(playerWork->effect.other.spriteGroup, 1, 4);
            playerWork->effect.other.damaged = 1;
            playerWork->effect.other.damageFrames = 420;
            playerWork->speed = 40.0f;
            MgScoreBoxDispSet(playerWork->effect.other.scoreGroup, 0);
            MgScoreBoxTPLvlSet(playerWork->effect.other.scoreGroup, 0.0f);
            memberIndex = 0;
            while (memberIndex < 3) {
                HuSprAttrSet(playerWork->effect.other.spriteGroup, memberIndex + 1, 4);
                memberIndex++;
            }
        }
    }
}

/* Resets opponent models and sets their coin bonus during the pre-winner hook. */
void fn_1_12228(void) {
    s32 playerIndex;
    M665PlayerWork *playerWork;
    s16 modelID;
    s16 motionID;
    s32 playerNo;

    playerIndex = 0;
    while (playerIndex < 3) {
        playerWork = &lbl_1_bss_10[lbl_1_bss_250.otherPlayers[playerIndex]];
        Hu3DModelHookObjReset(playerWork->vehicleModel, lbl_1_data_36C);
        Hu3DModelAttrSet(playerWork->vehicleModel, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(playerWork->characterModel, HU3D_ATTR_DISPOFF);
        Hu3DModelRotSet(playerWork->characterModel, 0.0f, 0.0f,
                        0.0f);
        Hu3DModelPosSet(playerWork->characterModel, 240.0f * (f32) playerIndex,
                        0.0f, 200.0f);
        modelID = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m665, 6), M665_MEMORY_TAG, HEAP_MODEL));
        motionID = Hu3DJointMotion(
            modelID,
            HuDataSelHeapReadNum(DATANUM(DATA_m665, 10), M665_MEMORY_TAG, HEAP_MODEL));
        Hu3DMotionSet(modelID, motionID);
        Hu3DModelPosSet(modelID, 300.0f * (f32) playerIndex,
                        0.0f, (-300.0f));
        Hu3DModelRotSet(modelID, 0.0f, 140.0f,
                        0.0f);
        Hu3DModelShadowSet(modelID);
        Hu3DModelShadowSet(playerWork->characterModel);
        fn_1_170C(
            lbl_1_bss_250.otherPlayers[(s16) playerIndex], 0,
            0.0f);
        playerNo = lbl_1_bss_250.otherPlayers[playerIndex];
        if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
            GwPlayer[playerNo].mgCoinBonus = 10;
        }
        playerIndex++;
    }
}

/* Restores opponent presentation and configures props in the pre-winner hook. */
void fn_1_1247C(void) {
    s32 playerIndex;
    M665PlayerWork *playerWork;
    s16 modelID;

    playerIndex = 0;
    while (playerIndex < 3) {
        playerWork = &lbl_1_bss_10[lbl_1_bss_250.otherPlayers[playerIndex]];
        Hu3DModelAttrSet(playerWork->vehicleModel, HU3D_ATTR_DISPOFF);
        Hu3DModelRotSet(playerWork->characterModel, 0.0f, 0.0f,
                        0.0f);
        Hu3DModelPosSet(playerWork->characterModel,
                        240.0f * (f32) playerIndex,
                        0.0f, (-300.0f));
        Hu3DModelAttrReset(playerWork->characterModel, HU3D_ATTR_DISPOFF);
        Hu3DModelShadowSet(playerWork->characterModel);
        fn_1_170C(lbl_1_bss_250.otherPlayers[(s16) playerIndex], 0,
                  0.0f);
        /* This selects motion ID 1 directly, rather than a stored vehicle motion ID. */
        Hu3DMotionSet(playerWork->vehicleModel, 1);
        Hu3DModelAttrSet(playerWork->vehicleModel, HU3D_MOTATTR_LOOP);
        playerWork->motionSpeed = 0.0f;
        playerIndex++;
    }
    modelID = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m665, 9), M665_MEMORY_TAG, HEAP_MODEL));
    Hu3DModelPosSet(modelID, 300.0f, 0.0f,
                    (-800.0f));
    Hu3DModelShadowSet(modelID);
    modelID = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m665, 34), M665_MEMORY_TAG, HEAP_MODEL));
    Hu3DModelPosSet(modelID, 250.0f, 150.0f,
                    (-800.0f));
    Hu3DModelAttrSet(modelID, HU3D_MOTATTR_LOOP);
    modelID = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m665, 34), M665_MEMORY_TAG, HEAP_MODEL));
    Hu3DModelPosSet(modelID, 400.0f, 140.0f,
                    (-750.0f));
    Hu3DModelScaleSet(modelID, 0.5f, 0.5f,
                      0.5f);
    Hu3DModelAttrSet(modelID, HU3D_MOTATTR_LOOP);
    Hu3DMotionSpeedSet(modelID, 1.5f);
}

/* Repositions opponent models during the pre-winner hook. */
void fn_1_12720(void) {
    M665PlayerWork *playerWork;
    s16 modelID;
    s16 motionID;
    s32 playerIndex;

    playerIndex = 0;
    while (playerIndex < 3) {
        playerWork = &lbl_1_bss_10[lbl_1_bss_250.otherPlayers[playerIndex]];
        Hu3DModelHookObjReset(playerWork->vehicleModel, lbl_1_data_36C);
        Hu3DModelAttrSet(playerWork->vehicleModel, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(playerWork->characterModel, HU3D_ATTR_DISPOFF);
        modelID = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m665, 6), M665_MEMORY_TAG, HEAP_MODEL));
        motionID = Hu3DJointMotion(
            modelID,
            HuDataSelHeapReadNum(DATANUM(DATA_m665, 10), M665_MEMORY_TAG, HEAP_MODEL));
        Hu3DMotionSet(modelID, motionID);
        Hu3DModelRotSet(modelID, 0.0f, 140.0f,
                        0.0f);
        Hu3DModelPosSet(modelID, 300.0f * (f32) playerIndex,
                        0.0f, (-600.0f));
        /* The motion selected above is immediately replaced with motion ID 1. */
        Hu3DMotionSet(modelID, 1);
        Hu3DModelRotSet(playerWork->characterModel, 0.0f, 0.0f,
                        0.0f);
        Hu3DModelPosSet(playerWork->characterModel, 200.0f * (f32) playerIndex,
                        0.0f, (-200.0f));
        Hu3DModelShadowSet(playerWork->characterModel);
        fn_1_170C(
            lbl_1_bss_250.otherPlayers[(s16) playerIndex], 0,
            0.0f);
        playerIndex++;
    }
}

/* Initializes hit and impact models during the sequence init hook. */
void fn_1_12948(void) {
    s16 baseModel;
    s16 modelID;
    s32 index;

    memset(lbl_1_bss_CA8, 0, 512);
    for (index = 0; index < 64; index++) {
        if (index == 0) {
            baseModel = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_m665, 23), M665_MEMORY_TAG, HEAP_MODEL));
            modelID = baseModel;
        } else {
            modelID = Hu3DModelLink(baseModel);
        }
        Hu3DModelAttrSet(modelID, HU3D_ATTR_DISPOFF);
        Hu3DModelScaleSet(modelID, 2.0f, 2.0f,
                          2.0f);
        lbl_1_bss_CA8[index].model = modelID;
    }
    for (index = 0; index < 64; index++) {
        if (index == 0) {
            baseModel = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_m665, 33), M665_MEMORY_TAG, HEAP_MODEL));
            modelID = baseModel;
        } else {
            modelID = Hu3DModelLink(baseModel);
        }
        Hu3DModelScaleSet(modelID, 2.0f, 2.0f,
                          2.0f);
        Hu3DModelAttrSet(modelID, HU3D_ATTR_DISPOFF);
        Hu3DMotionSpeedSet(modelID, 0.0f);
        lbl_1_bss_CA8[index].impactModel = modelID;
    }
}

/* Tests whether a point falls within an active opponent vehicle's bounds. */
s32 fn_1_12AD4(HuVecF *point) {
    HuVecF playerPosition;
    s32 playerIndex;
    M665PlayerWork *playerWork;

    for (playerIndex = 0; playerIndex < 3; playerIndex++) {
        playerWork = &lbl_1_bss_10[lbl_1_bss_250.otherPlayers[playerIndex]];
        if (playerWork->status.other.state == 1) {
            Hu3DModelPosGet(playerWork->vehicleModel, &playerPosition);
            if ((point->x <= 120.0f + playerPosition.x) &&
                (point->x >= playerPosition.x - 120.0f) &&
                (point->z <= 150.0f + playerPosition.z) &&
                (point->z >= playerPosition.z - 150.0f)) {
                return 1;
            }
        }
    }
    return 0;
}

typedef struct OffsetTable {
    HuVecF positions[4];
} OffsetTable;

typedef struct FloatTable4 {
    f32 distanceThresholds[4];
} FloatTable4;

const OffsetTable lbl_1_rodata_5E0 = {{
    {250.0f, 0.0f, 130.0f}, {-250.0f, 0.0f, 130.0f},
    {400.0f, 0.0f, -300.0f}, {-400.0f, 0.0f, -300.0f},
}};

const FloatTable4 lbl_1_rodata_610 = {{180.0f, 180.0f, 150.0f, 150.0f}};

/* Tests whether a point is near one of four offsets from the main vehicle. */
s32 fn_1_12BF8(HuVecF *position) {
    OffsetTable offsets;
    HuVecF currentPosition;
    HuVecF candidatePosition;
    FloatTable4 thresholds;
    s32 index;
    M665PlayerWork *playerWork;

    playerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    offsets = lbl_1_rodata_5E0;
    thresholds = lbl_1_rodata_610;
    Hu3DModelPosGet(playerWork->vehicleModel, &currentPosition);
    index = 0;
    while (index < 4) {
        candidatePosition.x = currentPosition.x + offsets.positions[index].x;
        candidatePosition.y = position->y;
        candidatePosition.z = currentPosition.z + offsets.positions[index].z;
        if (sqrtf(PSVECSquareDistance(&candidatePosition, position)) <
            thresholds.distanceThresholds[index]) {
            return 1;
        }
        index++;
    }
    return 0;
}

/* Starts a hit's impact model when called by fn_1_13024. */
void fn_1_12E4C(s16 hitIndex) {
    Point3d modelPosition;

    lbl_1_bss_CA8[hitIndex].state = 2;
    Hu3DModelPosGet(lbl_1_bss_CA8[hitIndex].model, &modelPosition);
    Hu3DModelPosSetV(lbl_1_bss_CA8[hitIndex].impactModel, &modelPosition);
    Hu3DModelAttrReset(lbl_1_bss_CA8[hitIndex].impactModel, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_CA8[hitIndex].model, HU3D_ATTR_DISPOFF);
    Hu3DMotionTimeSet(lbl_1_bss_CA8[hitIndex].impactModel, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_CA8[hitIndex].impactModel, 1.2f);
    Hu3DModelScaleSet(lbl_1_bss_CA8[hitIndex].impactModel, 2.0f, 2.0f, 2.0f);
}

/* Hides a hit model when called by fn_1_13024. */
void fn_1_12FA0(s16 hitIndex) {
    Point3d modelPosition;

    lbl_1_bss_CA8[hitIndex].state = 2;
    Hu3DModelPosGet(lbl_1_bss_CA8[hitIndex].model, &modelPosition);
    Hu3DModelAttrSet(lbl_1_bss_CA8[hitIndex].model, HU3D_ATTR_DISPOFF);
}

void fn_1_C090(void);

s32 fn_1_B66C(HuVecF *position);

void OSReport(const char *format, ...);

/* Checks main-vehicle hit offsets during fn_1_13024's effect scan. */
static inline s32 fn_1_12BF8_for_13024(HuVecF *position) {
    OffsetTable offsets;
    HuVecF currentPosition;
    HuVecF candidatePosition;
    FloatTable4 thresholds;
    s32 index;
    M665PlayerWork *playerWork;

    playerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    offsets = lbl_1_rodata_5E0;
    thresholds = lbl_1_rodata_610;
    Hu3DModelPosGet(playerWork->vehicleModel, &currentPosition);
    index = 0;
    while (index < 4) {
        candidatePosition.x = currentPosition.x + offsets.positions[index].x;
        candidatePosition.y = position->y;
        candidatePosition.z = currentPosition.z + offsets.positions[index].z;
        if (sqrtf(PSVECSquareDistance(&candidatePosition, position)) <
            thresholds.distanceThresholds[index]) {
            return 1;
        }
        index++;
    }
    return 0;
}

/* Checks active opponent bounds during fn_1_13024's effect scan. */
static inline s32 fn_1_12AD4_for_13024(HuVecF *point) {
    HuVecF playerPosition;
    s32 playerIndex;
    M665PlayerWork *playerWork;

    for (playerIndex = 0; playerIndex < 3; playerIndex++) {
        playerWork = &lbl_1_bss_10[lbl_1_bss_250.otherPlayers[playerIndex]];
        if (playerWork->status.other.state == 1) {
            Hu3DModelPosGet(playerWork->vehicleModel, &playerPosition);
            if ((point->x <= 120.0f + playerPosition.x) &&
                (point->x >= playerPosition.x - 120.0f) &&
                (point->z <= 150.0f + playerPosition.z) &&
                (point->z >= playerPosition.z - 150.0f)) {
                return 1;
            }
        }
    }
    return 0;
}

/* Advances hit models and resolves their collisions in its object-manager callback. */
void fn_1_13024(void) {
    static const SoundIds lbl_1_rodata_624 = {0x867, 0x868, 0x869};

    HuVecF effectPosition;
    HuVecF mainPosition;
    HuVecF sequenceHitPosition;
    HuVecF otherPlayerHitPosition;
    HuVecF pointHitPosition;
    HuVecF offsetOrigin;
    HuVecF boxHitPosition;
    M665PlayerWork *mainPlayerWork;
    M665HitWork *record;
    s16 soundIndex;
    s32 recordIndex;
    f32 distance;
    f32 maxTime;

    mainPlayerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    {
    SoundIds soundIds = lbl_1_rodata_624;
    Hu3DModelPosGet(mainPlayerWork->vehicleModel, &mainPosition);
    for (recordIndex = 0; recordIndex < 64; recordIndex++) {
        record = &lbl_1_bss_CA8[recordIndex];
        switch (record->state) {
        case 1:
            Hu3DModelPosGet(record->model, &effectPosition);
            effectPosition.z -= 25.0f;
            Hu3DModelPosSetV(record->model, &effectPosition);
            if (effectPosition.z <= -4000.0f) {
                record->state = 0;
                Hu3DModelAttrSet(record->model, HU3D_ATTR_DISPOFF);
            }
            /* These tests remain independent after a hit changes the record state. */
            if (effectPosition.x <= mainPosition.x + 350.0f &&
                effectPosition.x >= mainPosition.x - 350.0f &&
                effectPosition.z <= mainPosition.z + 100.0f &&
                effectPosition.z >= mainPosition.z - 600.0f) {
                fn_1_12E4C((s16) recordIndex);
                fn_1_C090();
                soundIndex = record->soundPlayerIndex;
                soundIndex = lbl_1_bss_10[soundIndex].groupIndex;
                HuAudFXPlayPan((&soundIds.soundID)[soundIndex],
                    (s16) ((s32) (effectPosition.x / 40.0f) + 64));
            }
            if ((s16) fn_1_12BF8_for_13024(&effectPosition)) {
                fn_1_12E4C((s16) recordIndex);
                fn_1_C090();
                soundIndex = record->soundPlayerIndex;
                soundIndex = lbl_1_bss_10[soundIndex].groupIndex;
                HuAudFXPlayPan((&soundIds.soundID)[soundIndex],
                    (s16) ((s32) (effectPosition.x / 40.0f) + 64));
            }
            if ((s16) fn_1_12AD4_for_13024(&effectPosition)) {
                fn_1_12FA0((s16) recordIndex);
                record->state = 0;
            }
            if (fn_1_B66C(&effectPosition) != 0) {
                fn_1_12E4C((s16) recordIndex);
            }
            break;
        case 2:
            maxTime = Hu3DMotionMaxTimeGet(record->impactModel);
            if (Hu3DMotionTimeGet(record->impactModel) >= maxTime) {
                Hu3DModelAttrSet(record->impactModel, HU3D_ATTR_DISPOFF);
                record->state = 0;
            }
            if ((u8) (mainPlayerWork->status.main.flags.byte >> 7 & 1) != 0) {
                Hu3DModelPosGet(record->impactModel, &effectPosition);
                if ((u8) (mainPlayerWork->status.main.flags.byte >> 6 & 1) != 0) {
                    effectPosition.x += mainPlayerWork->speed;
                } else {
                    effectPosition.x -= mainPlayerWork->speed;
                }
                Hu3DModelPosSetV(record->impactModel, &effectPosition);
            }
            break;
        }
    }
    }
}

/* Spawns a gun-hook hit object when fn_1_112C0 accepts an opponent action. */
void fn_1_13A44(s16 playerNo) {
    static const SoundIds lbl_1_rodata_634 = {0x864, 0x865, 0x866};
    HuVecF position;
    M665PlayerWork *playerWork;
    SoundIds soundIds;
    s32 recordIndex;

    playerWork = &lbl_1_bss_10[playerNo];
    soundIds = lbl_1_rodata_634;
    if (MgSeqModeGet() == 5) {
        for (recordIndex = 0; recordIndex < 64; recordIndex++) {
            if (lbl_1_bss_CA8[recordIndex].state == 0) {
                break;
            }
        }
        if (recordIndex == 64) {
            OSReport(lbl_1_data_38B);
            return;
        }
        lbl_1_bss_CA8[recordIndex].state = 1;
        Hu3DModelObjPosGet(playerWork->vehicleModel, lbl_1_data_3A9, &position);
        position.y -= 25.0f;
        position.z -= 10.0f;
        Hu3DModelPosSetV(lbl_1_bss_CA8[recordIndex].model,
                         &position);
        Hu3DModelAttrReset(
            lbl_1_bss_CA8[recordIndex].model, HU3D_ATTR_DISPOFF);
        playerWork->action = 30;
        HuAudFXPlayPan((&soundIds.soundID)[playerWork->groupIndex],
                       (s16) ((s32) (position.x / 40.0f) + 64));
    }
}

M665AnimationStorage lbl_1_bss_EA8;
M665HitWork lbl_1_bss_CA8[64];
