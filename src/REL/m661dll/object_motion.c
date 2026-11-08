/* Stage piece movement (wandering, moving away from nearby players, leaving through
 * the side exits), the spotlight update, actor contact hooks, and boundary and overlap
 * resolution for players and pieces. */
#include "dolphin/types.h"

#include "dolphin/gx.h"

#include "REL/m661dll/motion.h"
#include "REL/m661dll/light.h"

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

#include <math.h>
#include "humath.h"
#include "REL/m661dll/particle.h"
#include "REL/m661dll/sound.h"

/* These intervals are not interpreted by the functions in this file. */
typedef struct MotionSettings {
    s16 resetValue;
    u8 beforeMotionId[8];
    s16 motionId;
    u8 beforeMovementSpeed[8];
    f32 movementSpeed;
    u32 beforeStartFrame;
    f32 startFrame;
    u8 remainingFields[0x30];
} MotionSettings;

typedef struct MotionManagerState {
    u8 beforeMode[0x7C];
    s16 mode;
} MotionManagerState;

extern MotionObject lbl_1_bss_1E3C[];

extern MotionControl lbl_1_bss_10[];

extern MotionManagerState lbl_1_bss_196C;

s32 fn_1_2EA4(f32 *value, f32 step, f32 start, f32 end);

void fn_1_5A7C(void)
{
    s16 workIndex;
    s16 objectIndex;
    s16 inputIndex;
    s16 inputCount;
    f32 angleA;
    f32 angleB;
    f32 originalAngleA;
    f32 originalAngleB;
    f32 adjustedAngleA;
    f32 adjustedAngleB;
    f32 scale;
    f32 wrappedTurnAmount;
    f32 directTurnAmount;
    f32 turnAmount;
    f32 ambient;
    MotionObject *object;
    MotionControl *control;
    workIndex = 0x23;
    objectIndex = 0;

    while (workIndex < 0x71) {
        object = &lbl_1_bss_1E3C[workIndex];
        control = &lbl_1_bss_10[objectIndex];
        if (control->active != 0) {
            switch (control->mode) {
            case 0:
                if (lbl_1_bss_196C.mode == 0) {
                    inputIndex = 0;
                    inputCount = 0;
                    while (inputIndex < 2) {
                        if (control->inputDirection[inputIndex] == 1) {
                            inputCount++;
                        }
                        inputIndex++;
                    }
                    if (inputCount != 0) {
                        control->waitFrames = 0x1E;
                        control->movementSpeed = 0.0f;
                        if (control->inputLimit[0] == -1 && control->inputLimit[1] == -1) {
                            control->radius = 10.0f +
                                fn_1_3D04(30.0f) / 10.0f;
                            control->animationState = 1;
                        }
                        if (control->controlA != 0 && control->pieceContact[0] != 0 &&
                            control->pieceContact[1] != 0) {
                            control->radius = 5.0f;
                            control->randomizeAngle = 1;
                        } else {
                            if (control->randomizeAngle != 0) {
                                control->radius = 10.0f +
                                    fn_1_3D04(30.0f) / 10.0f;
                            }
                            control->randomizeAngle = 0;
                        }
                    }
                    if (inputCount == 2) {
                        angleA = (180.0*(atan2(((f64)control->input[0].x), ((f64)control->input[0].z)) / M_PI));
                        originalAngleA = angleA;
                        angleB = (180.0*(atan2(((f64)control->input[1].x), ((f64)control->input[1].z)) / M_PI));
                        originalAngleB = angleB;
                        scale = 1.0f;
                        if ((angleA > 90.0f && angleB < -90.0f) ||
                            (angleA < -90.0f && angleB > 90.0f)) {
                            if (angleA < 0.0f) {
                                adjustedAngleA = 180.0f + angleA;
                            } else {
                                adjustedAngleA = 180.0f - angleA;
                            }
                            angleA = adjustedAngleA;
                            if (angleB < 0.0f) {
                                adjustedAngleB = 180.0f + angleB;
                            } else {
                                adjustedAngleB = 180.0f - angleB;
                            }
                            angleB = adjustedAngleB;
                            if (angleA + angleB > 0.0f) {
                                wrappedTurnAmount = angleA + angleB;
                            } else {
                                wrappedTurnAmount = -1.0f * (angleA + angleB);
                            }
                            turnAmount = wrappedTurnAmount;
                            scale = -1.0f;
                        } else {
                            if ((angleA - angleB) > 0.0f) {
                                directTurnAmount = angleA - angleB;
                            } else {
                                directTurnAmount = -1.0f * (angleA - angleB);
                            }
                            turnAmount = directTurnAmount;
                        }
                        if (turnAmount > 180.0f) {
                            turnAmount = 360.0f - turnAmount;
                            scale = -1.0f;
                        }
                        turnAmount /= 2.0f * scale;
                        turnAmount = originalAngleA < originalAngleB ?
                            originalAngleA + turnAmount : originalAngleB + turnAmount;
                        object->x += control->radius *
                            sin(M_PI*turnAmount/180.0);
                        object->z += control->radius *
                            cos(M_PI*turnAmount/180.0);
                        object->angle = turnAmount;
                    } else if (inputCount == 1) {
                        for (inputIndex = 0; inputIndex < 2; inputIndex++) {
                            if (control->inputDirection[inputIndex] == 1) {
                                control->inputMagnitude[inputIndex] =
                                    sqrt(control->inputMagnitude[inputIndex]);
                                control->inputMagnitude[inputIndex] =
                                    control->inputMagnitude[inputIndex] == 0.0f ?
                                    1.0f : control->inputMagnitude[inputIndex];
                                scale = control->radius / control->inputMagnitude[inputIndex];
                                control->input[inputIndex].x *= scale;
                                control->input[inputIndex].z *= scale;
                                object->x += control->input[inputIndex].x;
                                object->z += control->input[inputIndex].z;
                                object->angle = (180.0*(atan2(((f64)control->input[inputIndex].x), ((f64)control->input[inputIndex].z)) / M_PI));
                            }
                        }
                    } else {
                        if (control->waitFrames-- < 0) {
                            control->waitFrames = frandmod(0x32) + 0x32;
                            control->movementSpeed = (control->waitFrames % 2 == 0) ?
                                fn_1_3D04(50.0f) : 0.0f;
                            control->movementSpeed /= 10.0f;
                            control->angle = 180.0f + fn_1_3D04(360.0f);
                        }
                        if (control->movementSpeed > 0.0f) {
                            object->x += control->movementSpeed *
                                sin(M_PI*control->angle/180.0);
                            object->z += control->movementSpeed *
                                cos(M_PI*control->angle/180.0);
                            object->angle = control->angle;
                            control->animationState = 1;
                        } else {
                            control->animationState = 0;
                        }
                    }
                }
                break;
            case 1:
                if (control->directionMode == 0) {
                    fn_1_2EA4(&object->angle, 30.0f,
                              control->endSpeed, -90.0f);
                    control->angle = object->angle;
                    if (object->x < -1000.0f) {
                        fn_1_5020(2, 5, 0);
                        control->mode++;
                    }
                } else if (control->directionMode != 0) {
                    fn_1_2EA4(&object->angle, 30.0f,
                              control->endSpeed, 90.0f);
                    control->angle = object->angle;
                    if (object->x > 1000.0f) {
                        fn_1_5020(2, 5, 0);
                        control->mode++;
                    }
                }
                control->movementSpeed = 12.0f;
                object->x += control->movementSpeed *
                    sin(M_PI*control->angle/180.0);
                object->z += control->movementSpeed *
                    cos(M_PI*control->angle/180.0);
                control->animationState = 1;
                ambient = object->x > 0.0f ? object->x :
                    -1.0f * object->x;
                ambient = (ambient - 775.0f) * 0.0006f;
                ambient = 0.7f - ambient;
                fn_800273EC(object->modelId, ambient, ambient, ambient);
                break;
            case 2:
                if (control->waitFrames-- < 0) {
                    control->waitFrames = frandmod(0x32) + 0x32;
                    control->movementSpeed = (control->waitFrames % 2 == 0) ?
                        fn_1_3D04(50.0f) : 0.0f;
                    control->movementSpeed /= 10.0f;
                    control->angle = 180.0f + fn_1_3D04(360.0f);
                }
                if (control->movementSpeed > 0.0f) {
                    object->x += control->movementSpeed *
                        sin(M_PI*control->angle/180.0);
                    object->z += control->movementSpeed *
                        cos(M_PI*control->angle/180.0);
                    object->angle = control->angle;
                    control->animationState = 1;
                } else {
                    control->animationState = 0;
                }
                break;
            }
        }
        if (control->animationState != control->previousAnimationState) {
                fn_8002CCC0(object->modelId,
                    control->motionIds[control->animationState],
                    0.0f, 5.0f, ((1 << 30) | (1 << 0)));
            }
        control->previousAnimationState = control->animationState;
        workIndex++;
        objectIndex++;
    }
}

void fn_1_67DC(void)
{
    s16 index;
    LightConfig *light;

    for (index = 0; index < 3; index++) {
        if (index != 0) {
            break;
        }
        light = &lbl_1_bss_1870[index];
        fn_80028F14(light->modelId, light->lightId, &light->position, &light->direction);
        switch (light->type) {
        case 0:
            break;
        case 1:
            fn_80028C1C(light->modelId, light->lightId, light->referenceDistance,
                               light->referenceBrightness, GX_DA_STEEP);
            break;
        case 2:
            fn_80028AA8(light->modelId, light->lightId, light->cutoff, GX_SP_COS);
            break;
        }
    }
}

typedef struct PlayerState {
    u8 beforeRecordIndex[4];
    s32 recordIndex;
} PlayerState;

typedef struct RequestState {
    u8 beforeRecordIndex[4];
    s32 recordIndex;
    s32 mode;
} RequestState;

typedef struct PlayerBoardState {
    u8 beforeX[0x34];
    f32 x;
    u8 betweenCoordinates[0x4];
    f32 z;
    u8 beforeValue[0x1A];
    s16 value;
    u8 beforePairedIndex[0x2];
    s16 pairedIndex;
    u8 beforeUpdateCount[0x2];
    s16 updateCount;
    s16 piecesPastEdge;
    s16 boundaryAdjustmentFrames;
    u8 remainingFields[0x54];
} PlayerBoardState;

extern PlayerBoardState lbl_1_bss_1C04[2];

s32 fn_1_68A8(PlayerState *first, RequestState *second)
{
    switch (second->mode) {
    case 0:
        lbl_1_bss_1C04[first->recordIndex].pairedIndex = (s16)second->recordIndex;
        lbl_1_bss_1C04[second->recordIndex].pairedIndex = (s16)first->recordIndex;
        lbl_1_bss_1C04[first->recordIndex].updateCount += 1;
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

typedef struct RecordOwner {
    u8 beforeRecordIndex[0x0C];
    s32 recordIndex;
    u8 beforeValue[0x64];
    u16 value;
} RecordOwner;

void fn_1_6928(RecordOwner *owner)
{
    s32 recordIndex = owner->recordIndex;
    lbl_1_bss_1C04[recordIndex].value = owner->value;
}

typedef struct BoardBoundary { f32 x0, x1, z0, z1; } BoardBoundary;

void fn_1_695C(void)
{
    Point3d edgeNormals[2] = {{1.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 1.0f}};
    BoardBoundary boardBoundaries[6];
    s16 modelIndex, motionIndex, otherIndex, edgeIndex, adjusted;
    PlayerBoardState *boardPlayer;
    MotionObject *piece;
    MotionControl *motion;
    f32 distance, scale, radius60, lengthSquared, amount, radius500, radius60Squared, radius300, radius400;
    Point3d delta, nearest, side;
    f32 playerRadius, projection, radius80, radius80Squared, radius200;
    s16 wasNearPlayer[2];

    boardBoundaries[0].x0 = -590.0f;
    boardBoundaries[0].x1 = -790.0f;
    boardBoundaries[0].z0 = -790.0f;
    boardBoundaries[0].z1 = -590.0f;
    boardBoundaries[1].x0 = 590.0f;
    boardBoundaries[1].x1 = 790.0f;
    boardBoundaries[1].z0 = -790.0f;
    boardBoundaries[1].z1 = -590.0f;
    boardBoundaries[2].x0 = -1160.0f;
    boardBoundaries[2].x1 = 860.0f;
    boardBoundaries[3].x0 = 1160.0f;
    boardBoundaries[3].x1 = 860.0f;
    radius500 = 500.0f;
    radius500 *= radius500;
    boardBoundaries[2].z0 = -800.0f;
    boardBoundaries[2].z1 = 0.0f;
    boardBoundaries[3].z0 = 800.0f;
    boardBoundaries[3].z1 = 0.0f;
    radius200 = 200.0f;
    radius200 *= radius200;
    boardBoundaries[4].x0 = -781.0f;
    boardBoundaries[4].x1 = -1000.0f;
    boardBoundaries[4].z0 = 200.0f;
    boardBoundaries[4].z1 = -200.0f;
    boardBoundaries[5].x0 = 1000.0f;
    boardBoundaries[5].x1 = 781.0f;
    boardBoundaries[5].z0 = 200.0f;
    boardBoundaries[5].z1 = -200.0f;
    radius80 = 80.0f;
    radius80Squared = radius80 * radius80;
    radius60 = 60.0f;
    radius60Squared = radius60 * radius60;
    radius300 = 300.0f;
    radius300 *= radius300;
    radius400 = 400.0f;
    radius400 *= radius400;
    for (modelIndex = 0; modelIndex < 2; modelIndex++) {
        boardPlayer = &lbl_1_bss_1C04[modelIndex];
        edgeIndex = 0;
        adjusted = 0;
        for (; edgeIndex < 2; edgeIndex++) {
            delta.x = boardBoundaries[edgeIndex].x0 - boardBoundaries[edgeIndex].x1;
            delta.z = boardBoundaries[edgeIndex].z0 - boardBoundaries[edgeIndex].z1;
            lengthSquared = delta.x * delta.x + delta.z * delta.z;
            projection = -1.0f * (delta.x * (boardBoundaries[edgeIndex].x0 - boardPlayer->x) + delta.z * (boardBoundaries[edgeIndex].z0 - boardPlayer->z));
            amount = lengthSquared ? projection / lengthSquared : 0.0f;
            nearest.x = delta.x * amount + boardBoundaries[edgeIndex].x0;
            nearest.z = delta.z * amount + boardBoundaries[edgeIndex].z0;
            delta.x = boardPlayer->x - nearest.x;
            delta.z = boardPlayer->z - nearest.z;
            side.x = delta.x != 0.0f ? (delta.x > 0.0f ? 1.0f : -1.0f) : edgeNormals[edgeIndex].x;
            side.z = delta.z != 0.0f ? (delta.z > 0.0f ? 1.0f : -1.0f) : edgeNormals[edgeIndex].z;
            if (side.x != edgeNormals[edgeIndex].x || side.z != edgeNormals[edgeIndex].z) {
                adjusted = 1;
                boardPlayer->x = nearest.x;
                boardPlayer->z = nearest.z;
            }
        }
        for (edgeIndex = 2; edgeIndex < 4; edgeIndex++) {
            delta.x = boardPlayer->x - boardBoundaries[edgeIndex].x0;
            delta.z = boardPlayer->z - boardBoundaries[edgeIndex].x1;
            distance = delta.x * delta.x + delta.z * delta.z;
            if (distance < radius500) {
                adjusted = 1;
                distance = (f32)sqrt((f64)distance);
                distance = distance == 0.0f ? 1.0f : distance;
                scale = 500.0f / distance;
                delta.x *= scale;
                delta.z *= scale;
                boardPlayer->x = delta.x + boardBoundaries[edgeIndex].x0;
                boardPlayer->z = delta.z + boardBoundaries[edgeIndex].x1;
            }
        }
        boardPlayer->boundaryAdjustmentFrames = adjusted ? boardPlayer->boundaryAdjustmentFrames + 1 : 0;
        if (boardPlayer->x > 750.0f) boardPlayer->x = 750.0f;
        if (boardPlayer->x < -750.0f) boardPlayer->x = -750.0f;
        if (boardPlayer->z > 750.0f) boardPlayer->z = 750.0f;
        if (boardPlayer->z < -750.0f) boardPlayer->z = -750.0f;
    }
    for (modelIndex = 0x23, motionIndex = 0; modelIndex < 0x71; modelIndex++, motionIndex++) {
        piece = &lbl_1_bss_1E3C[modelIndex];
        motion = &lbl_1_bss_10[motionIndex];
        motion->controlA = motion->pieceContact[0] = motion->pieceContact[1] = 0;
        if (motion->active && motion->mode == 0) {
            for (otherIndex = 0x23, edgeIndex = 0; otherIndex < 0x71; otherIndex++, edgeIndex++) {
                if (modelIndex == otherIndex || lbl_1_bss_10[edgeIndex].mode != 0 || !lbl_1_bss_10[edgeIndex].active) continue;
                delta.x = piece->x - lbl_1_bss_1E3C[otherIndex].x;
                delta.z = piece->z - lbl_1_bss_1E3C[otherIndex].z;
                distance = delta.x * delta.x + delta.z * delta.z;
                if (distance < radius60Squared) {
                    nearest.x = piece->x - delta.x / 2.0f;
                    nearest.z = piece->z - delta.z / 2.0f;
                    distance = (f32)sqrt((f64)distance);
                    distance = distance == 0.0f ? 1.0f : distance;
                    scale = radius60 / distance;
                    delta.x *= scale;
                    delta.z *= scale;
                    delta.x /= 2.0f;
                    delta.z /= 2.0f;
                    piece->x = nearest.x + delta.x;
                    piece->z = nearest.z + delta.z;
                    lbl_1_bss_1E3C[otherIndex].x = nearest.x - delta.x;
                    lbl_1_bss_1E3C[otherIndex].z = nearest.z - delta.z;
                }
            }
        }
    }
    for (modelIndex = 0x23, motionIndex = 0; modelIndex < 0x71; modelIndex++, motionIndex++) {
        piece = &lbl_1_bss_1E3C[modelIndex];
        motion = &lbl_1_bss_10[motionIndex];
        motion->controlA = motion->pieceContact[0] = motion->pieceContact[1] = 0;
        if (!motion->active) continue;
        if (motion->mode == 0) {
            wasNearPlayer[0] = wasNearPlayer[1] = 0;
            if (motion->inputDirection[0] == 1) wasNearPlayer[0] = 1;
            if (motion->inputDirection[1] == 1) wasNearPlayer[1] = 1;
            motion->inputLimit[0] = motion->inputDirection[0];
            motion->inputLimit[1] = motion->inputDirection[1];
            motion->inputDirection[1] = motion->inputDirection[0] = -1;
            for (otherIndex = 0; otherIndex < 2; otherIndex++) {
                boardPlayer = &lbl_1_bss_1C04[otherIndex];
                delta.x = piece->x - boardPlayer->x;
                delta.z = piece->z - boardPlayer->z;
                distance = delta.x * delta.x + delta.z * delta.z;
                playerRadius = wasNearPlayer[otherIndex] ? radius400 : radius300;
                if (distance < playerRadius) {
                    motion->inputDirection[otherIndex] = 1;
                    motion->input[otherIndex] = delta;
                    motion->inputMagnitude[otherIndex] = distance;
                    motion->pieceContact[otherIndex] = 1;
                    if (distance < radius80Squared) {
                        nearest.x = piece->x - delta.x / 2.0f;
                        nearest.z = piece->z - delta.z / 2.0f;
                        distance = (f32)sqrt((f64)distance);
                        distance = distance == 0.0f ? 1.0f : distance;
                        scale = radius80 / distance;
                        delta.x *= scale;
                        delta.z *= scale;
                        delta.x /= 2.0f;
                        delta.z /= 2.0f;
                        piece->x = nearest.x + delta.x;
                        piece->z = nearest.z + delta.z;
                        boardPlayer->x = nearest.x - delta.x;
                        boardPlayer->z = nearest.z - delta.z;
                    }
                }
            }
            for (edgeIndex = 0; edgeIndex < 2; edgeIndex++) {
                delta.x = boardBoundaries[edgeIndex].x0 -
                    boardBoundaries[edgeIndex].x1;
                delta.z = boardBoundaries[edgeIndex].z0 -
                    boardBoundaries[edgeIndex].z1;
                lengthSquared = delta.x * delta.x + delta.z * delta.z;
                projection = -1.0f * (delta.x * (boardBoundaries[edgeIndex].x0 - piece->x) + delta.z * (boardBoundaries[edgeIndex].z0 - piece->z));
                amount = lengthSquared ? projection / lengthSquared : 0.0f;
                nearest.x = delta.x * amount + boardBoundaries[edgeIndex].x0;
                nearest.z = delta.z * amount + boardBoundaries[edgeIndex].z0;
                delta.x = piece->x - nearest.x;
                delta.z = piece->z - nearest.z;
                side.x = delta.x != 0.0f ? (delta.x > 0.0f ? 1.0f : -1.0f) : edgeNormals[edgeIndex].x;
                side.z = delta.z != 0.0f ? (delta.z > 0.0f ? 1.0f : -1.0f) : edgeNormals[edgeIndex].z;
                if (side.x != edgeNormals[edgeIndex].x || side.z != edgeNormals[edgeIndex].z) {
                    piece->x = nearest.x;
                    piece->z = nearest.z;
                    motion->controlA = 1;
                }
            }
            for (edgeIndex = 2; edgeIndex < 4; edgeIndex++) {
                delta.x = piece->x - boardBoundaries[edgeIndex].x0;
                delta.z = piece->z - boardBoundaries[edgeIndex].x1;
                distance = delta.x * delta.x + delta.z * delta.z;
                if (distance < radius500) {
                    distance = (f32)sqrt((f64)distance);
                    distance = distance == 0.0f ? 1.0f : distance;
                    scale = 500.0f / distance;
                    delta.x *= scale;
                    delta.z *= scale;
                    piece->x = delta.x + boardBoundaries[edgeIndex].x0;
                    piece->z = delta.z + boardBoundaries[edgeIndex].x1;
                    motion->controlA = 1;
                }
            }
            if (motion->inputDirection[0] == 1 || motion->inputDirection[1] == 1) {
                if (piece->x > 775.0f && (piece->z > 200.0f || piece->z < -200.0f)) piece->x = 775.0f;
                if (piece->x < -775.0f && (piece->z > 200.0f || piece->z < -200.0f)) piece->x = -775.0f;
            } else {
                if (piece->x > 775.0f) { piece->x = 775.0f; motion->controlA = 1; }
                if (piece->x < -775.0f) { piece->x = -775.0f; motion->controlA = 1; }
            }
            if (piece->z > 775.0f) { piece->z = 775.0f; motion->controlA = 1; }
            if (piece->z < -775.0f) { piece->z = -775.0f; motion->controlA = 1; }
            if (motion->inputDirection[0] == 1 || motion->inputDirection[1] == 1) {
                if (piece->x < -790.0f) { lbl_1_bss_1C04[0].piecesPastEdge++; motion->mode = 1; motion->endSpeed = piece->angle; motion->directionMode = 0; }
                if (piece->x > 790.0f) { lbl_1_bss_1C04[1].piecesPastEdge++; motion->mode = 1; motion->endSpeed = piece->angle; motion->directionMode = 1; }
            }
        } else if (motion->mode != 0) {
            for (otherIndex = 0x23, edgeIndex = 0; otherIndex < 0x71; otherIndex++, edgeIndex++) {

                if (modelIndex == otherIndex || lbl_1_bss_10[edgeIndex].mode == 0 || !lbl_1_bss_10[edgeIndex].active) continue;
                delta.x = piece->x - lbl_1_bss_1E3C[otherIndex].x;
                delta.z = piece->z - lbl_1_bss_1E3C[otherIndex].z;
                distance = delta.x * delta.x + delta.z * delta.z;
                if (distance < radius60Squared) {
                    nearest.x = piece->x - delta.x / 2.0f;
                    nearest.z = piece->z - delta.z / 2.0f;
                    distance = (f32)sqrt((f64)distance);
                    distance = distance == 0.0f ? 1.0f : distance;
                    scale = radius60 / distance;
                    delta.x *= scale;
                    delta.z *= scale;
                    delta.x /= 2.0f;
                    delta.z /= 2.0f;
                    piece->x = nearest.x + delta.x;
                    piece->z = nearest.z + delta.z;
                    lbl_1_bss_1E3C[otherIndex].x = nearest.x - delta.x;
                    lbl_1_bss_1E3C[otherIndex].z = nearest.z - delta.z;
                }
            }
            if (motion->mode == 1) {
                if (piece->z > 200.0f) piece->z = 200.0f;
                if (piece->z < -200.0f) piece->z = -200.0f;
            }
            else if (motion->mode == 2) {
                if (motion->directionMode == 0) {
                    if (piece->x < -1270.0f) piece->x = -1270.0f;
                    if (piece->x > -1000.0f) piece->x = -1000.0f;
                } else {
                    if (piece->x > 1270.0f) piece->x = 1270.0f;
                    if (piece->x < 1000.0f) piece->x = 1000.0f;
                }
                if (piece->z > 210.0f) piece->z = 210.0f;
                if (piece->z < -220.0f) piece->z = -220.0f;
            }
        }
    }

    radius60 = 80.0f;
    radius60Squared = radius60 * radius60;
    for (modelIndex = 0x23, motionIndex = 0; modelIndex < 0x71; modelIndex++, motionIndex++) {
        piece = &lbl_1_bss_1E3C[modelIndex];
        motion = &lbl_1_bss_10[motionIndex];
        if (!motion->active || motion->mode != 0) continue;
        for (otherIndex = 0x23, edgeIndex = 0; otherIndex < 0x71; otherIndex++, edgeIndex++) {

            if (modelIndex == otherIndex || lbl_1_bss_10[edgeIndex].mode != 0 || !lbl_1_bss_10[edgeIndex].active) continue;
            delta.x = piece->x - lbl_1_bss_1E3C[otherIndex].x;
            delta.z = piece->z - lbl_1_bss_1E3C[otherIndex].z;
            distance = delta.x * delta.x + delta.z * delta.z;
            if (distance < radius60Squared) {
                if (motion->controlA != 0 || lbl_1_bss_10[edgeIndex].controlA != 0) motion->controlA = lbl_1_bss_10[edgeIndex].controlA = 1;
                if (motion->pieceContact[0] != 0 || lbl_1_bss_10[edgeIndex].pieceContact[0] != 0) motion->pieceContact[0] = lbl_1_bss_10[edgeIndex].pieceContact[0] = 1;
                if (motion->pieceContact[1] != 0 || lbl_1_bss_10[edgeIndex].pieceContact[1] != 0) motion->pieceContact[1] = lbl_1_bss_10[edgeIndex].pieceContact[1] = 1;
            }
        }
    }
}
