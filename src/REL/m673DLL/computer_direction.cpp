// Reads player input and chooses movement for computer-controlled players.

#include "humath.h"
#include "REL/framework/event_manager.h"
#include "REL/m673DLL/position_table.h"
#include "REL/m673DLL/moving_actor.h"
#include "REL/framework/frame_time.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
extern "C" {
#include "math.h"
#include "game/gamework.h"
#include "game/pad.h"
}
#include "REL/framework/random.h"
#include "REL/m673DLL/vector_check.h"
#include "REL/m673DLL/movement_vector.h"
#include "REL/m673DLL/controllers.h"
#include "REL/m673DLL/player_actor.h"

// Clears the stick values and updates the controller.
void PlayerController::fn_1_BDC()
{
    values.x = 0.0f;
    values.y = 0.0f;
    values.magnitude = 0.0f;
    update(&values);
}

// Selects the pad assigned to the player.
PadController::PadController(PlayerActor *player) : PlayerController(player)
{
    s32 playerIndex;

    playerIndex = player->playerIndex;
    padNumber = lbl_80265728[playerIndex].padNo;
}

// Releases the pad input state.
PlayerController::~PlayerController()
{
}

// Reads the configured pad's stick values and buttons.
void PadController::update(StickState *stickState)
{
    s32 padIndex = padNumber;
    f32 x = lbl_8024C7D8[padIndex];
    f32 y = lbl_8024C7E8[padIndex];
    f64 magnitude = sqrt(x * x + y * y);

    stickState->x = x;
    stickState->y = y;
    stickState->magnitude = (f32)magnitude;
    stickState->pressedButtons = lbl_802C0008[padIndex];
    stickState->heldButtons = lbl_802C0010[padIndex];
}

// Loads the difficulty settings and selects the initial steering callback.
ComController::ComController(PlayerActor *player, s32 computerDifficulty)
    : PlayerController(player)
{
    difficulty = computerDifficulty;
    steeringParameters[0] = fn_1_2450(computerDifficulty);
    steeringParameters[1] = fn_1_2478(computerDifficulty);
    steeringParameters[2] = fn_1_24A0(computerDifficulty);
    steeringParameters[3] = fn_1_24C8(computerDifficulty);
    speed = 0.0f;
    state = 0;
    needsDestination = 1;
    direction.x = 0.0f;
    direction.y = 0.0f;
    direction.z = 0.0f;
    destination.x = 0.0f;
    destination.y = 0.0f;
    destination.z = 0.0f;
    setUpdate(&ComController::updateDirection);
}

// Releases the base controller and its pad state.
ComController::~ComController()
{
}

// Stores the scene position table and the player position.
struct PositionSnapshot {
    ScenePositionTable *unk00; // The game's code never reads this field.
    Vec position;
};

// Saves the player position and passes input to the steering callback.
void ComController::update(StickState *stickState)
{
    PositionSnapshot snapshot;
    PlayerActor *actor;

    snapshot.unk00 = &ScenePositionTable::getChecked();
    actor = player;
    snapshot.position.x = actor->component.position.value.x;
    snapshot.position.y = actor->component.position.value.y;
    snapshot.position.z = actor->component.position.value.z;
    input.applyToPlayer(player->getPlayer());
    if (callback) {
        (this->*callback)(stickState);
    }
}

// Computes the desired direction and applies camera-relative stick input.
void ComController::updateDirection(StickState *stickState)
{
    MovementVector desiredDirection;
    f32 scale;
    f32 magnitude;

    desiredDirection.x = 0.0f;
    desiredDirection.y = 0.0f;
    desiredDirection.z = 0.0f;
    desiredDirection = fn_1_1414();
    desiredDirection.y = 0.0f;
    if (fn_1_B58(&desiredDirection) == 0) {
        PSVECNormalize(&desiredDirection, &this->direction);
        this->speed = 560.0f / 60.0f;
    } else {
        if (fn_1_B58(&this->direction) == 0) {
            PSVECNormalize(&this->direction, &this->direction);
            this->speed += -this->speed / (60.0f * this->steeringParameters[3]);
            magnitude = PSVECMag(&this->direction);
            if (magnitude < 140.0f / 60.0f) {
                this->direction.x = 0.0f;
                this->direction.y = 0.0f;
                this->direction.z = 0.0f;
                this->state = 0;
            }
        }
    }
    scale = this->speed;
    PSVECScale(&this->direction, &this->direction, scale);
    this->input.updateMovement(&this->direction, 0.15f);
}

// Chooses the computer-controlled movement direction using the scene position table.
Vec ComController::fn_1_1414()
{
    // Holds the state and distances used when comparing steering directions.
    struct DirectionContext {
        f32 scale;
        f32 lineDistance;
        s32 state;
    };
    DirectionContext context;
    MovementVector nearestPoint;
    MovementVector lineEnd;
    MovementVector difference;
    MovementVector heading;
    MovementVector sourcePosition;
    f32 angle;
    f32 edgeX;
    f32 safeDistance;
    s32 playerNumber;
    u8 outside;
    ScenePositionTable *positionTable = &ScenePositionTable::getChecked();
    Vec intersection = { 0.0f, 0.0f, 0.0f };
    Vec actorPosition;
    MovementVector result;

    safeDistance = 100.0 + 0.3 * this->difficulty;
    result.set(0.0, 0.0, 0.0);
    this->player->getPosition(actorPosition);
    actorPosition.y = 0.0f;
    PlayerActor &actor = *this->player;
    playerNumber = actor.getPlayerNumber();
    heading.set(0.0, 0.0, 0.0);
    difference.set(0.0, 0.0, 0.0);
    lineEnd.set(0.0, 0.0, 0.0);
    nearestPoint.set(0.0, 0.0, 0.0);
    sourcePosition = positionTable->getPosition();
    sourcePosition.y = 0.0f;
    HuSubVecF(&difference, &actorPosition, &sourcePosition);
    angle = positionTable->getAngle();
    heading.x = sin((M_PI * angle) / 180.0);
    heading.z = cos((M_PI * angle) / 180.0);
    PSVECNormalize(&heading, &heading);
    context.state = positionTable->getState();
    context.scale = 400.0f / 60.0f;
    Vec scaledHeading;
    PSVECScale(&heading, &scaledHeading, 400.0f / 60.0f);
    lineEnd = sourcePosition;
    lineEnd += (scaledHeading *= secondsToFrames(2L));
    context.lineDistance = fn_1_24E0(&actorPosition, &sourcePosition, &lineEnd, &nearestPoint);
    if (context.state == 4) {
        if ((this->difficulty == 2 || this->difficulty == 3) && difference.length() > 700.0f &&
            fabs(actorPosition.x) < 100.0f * 5.3f - this->steeringParameters[1] &&
            fabs(actorPosition.z) < 100.0f * 4.3f - this->steeringParameters[2]) {
            return result;
        }
        MovementVector projected;
        HuSubVecF(&projected, &nearestPoint, &sourcePosition);
        if (context.lineDistance > 0.0f && context.lineDistance < projected.length()) {
            HuSubVecF(&result, &actorPosition, &nearestPoint);
            if (!fn_1_B58(&result)) {
                PSVECNormalize(&result, &result);
            }
        } else {
            if (difference.length() < this->steeringParameters[0] && !fn_1_B58(&difference)) {
                HuSubVecF(&result, &actorPosition, &sourcePosition);
                if (!fn_1_B58(&result)) {
                    PSVECNormalize(&difference, &result);
                }
            }
        }
    } else {
        if (difference.length() < this->steeringParameters[0] && !fn_1_B58(&difference)) {
            this->needsDestination = 1;
            PSVECNormalize(&difference, &result);
        } else {
            if (context.lineDistance < safeDistance && context.lineDistance > 0.0) {
                if (context.state == 2) {
                    return result;
                }
                HuSubVecF(&result, &actorPosition, &nearestPoint);
                if (!fn_1_B58(&result)) {
                    PSVECNormalize(&result, &result);
                }
            }
        }
    }
    outside = 0;
    if (actorPosition.x > 100.0f * 5.3f - this->steeringParameters[1] ||
        actorPosition.x < -(100.0f * 5.3f - this->steeringParameters[1]) ||
        ((actorPosition.z > 100.0f * 4.3f - this->steeringParameters[2] ||
          actorPosition.z < -(100.0f * 4.3f - this->steeringParameters[2])) &&
         !fn_1_B58(&result))) {
        // Direction-selection weights by difficulty.
        static const s32 probabilities[4] = { 2, 4, 8, 10 };
        outside = 1;
        if (!this->state) {
            this->state = 0;
            if ((s32)RandomUnsigned() % 10 < probabilities[this->difficulty]) {
                this->state = 1;
            }
        }
    }
    if (outside) {
        if (difference.length() < 500.0f) {
            f32 distances[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            distances[0] = fabs(100.0f * 5.3f + actorPosition.x);
            distances[1] = fabs(actorPosition.x - 100.0f * 5.3f);
            distances[2] = fabs(100.0f * 4.3f + actorPosition.z);
            distances[3] = fabs(actorPosition.z - 100.0f * 4.3f);
            f32 original[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            original[0] = fabs(100.0f * 5.3f + actorPosition.x);
            original[1] = fabs(actorPosition.x - 100.0f * 5.3f);
            original[2] = fabs(100.0f * 4.3f + actorPosition.z);
            original[3] = fabs(actorPosition.z - 100.0f * 4.3f);
            fn_1_25F8(distances, 4);
            if (distances[0] == original[0] || distances[0] == original[1]) {
                if (distances[1] != original[2] && actorPosition.z > sourcePosition.z) {
                    result.x = 0.0f;
                    result.y = 0.0f;
                    result.z = -1.0f;
                } else {
                    if (distances[1] != original[3] && actorPosition.z < sourcePosition.z) {
                        result.x = 0.0f;
                        result.y = 0.0f;
                        result.z = 1.0f;
                    } else {
                        return result;
                    }
                }
            } else {
                if (distances[1] != original[0] && actorPosition.x > sourcePosition.x) {
                    result.x = 1.0f;
                    result.y = 0.0f;
                    result.z = 0.0f;
                } else {
                    if (distances[1] != original[1] && actorPosition.x < sourcePosition.x) {
                        result.x = -1.0f;
                        result.y = 0.0f;
                        result.z = 0.0f;
                    } else {
                        return result;
                    }
                }
            }
        } else {
            // Both projections ignore the return and use their zero-initialized output
            // if the projection leaves it untouched.
            if (fabs(actorPosition.x) > 100.0f * 5.3f - this->steeringParameters[1]) {
                actorPosition.x > 0.0 ? (edgeX = 100.0f * 5.3f - this->steeringParameters[1]) :
                    (edgeX = -(100.0f * 5.3f - this->steeringParameters[1]));
                MovementVector edge(edgeX, actorPosition.y, actorPosition.z);
                MovementVector intersection(0.0, 0.0, 0.0);
                fn_1_24E0(&actorPosition, &sourcePosition, &edge, &intersection);
                HuSubVecF(&result, &intersection, &actorPosition);
                PSVECNormalize(&result, &result);
            }
            if (fabs(actorPosition.z) > 100.0f * 4.3f - this->steeringParameters[2]) {
                f32 edgeZ;
                actorPosition.z > 0.0 ? (edgeZ = 100.0f * 4.3f - this->steeringParameters[2]) :
                    (edgeZ = -(100.0f * 4.3f - this->steeringParameters[2]));
                MovementVector edge(actorPosition.x, actorPosition.y, edgeZ);
                MovementVector intersection(0.0, 0.0, 0.0);
                fn_1_24E0(&actorPosition, &sourcePosition, &edge, &intersection);
                HuSubVecF(&result, &intersection, &actorPosition);
                PSVECNormalize(&result, &result);
            }
        }
    }
    if (fn_1_B58(&result)) {
        MovementVector step;
        if (this->needsDestination) {
            this->destination.x = 0.0f;
            this->destination.y = 0.0f;
            this->destination.z = 0.0f;
            do {
                this->destination.x = RandomFloat(100.0f * 5.3f - this->steeringParameters[1],
                                                  -(100.0f * 5.3f - this->steeringParameters[1]));
                this->destination.z = RandomFloat(100.0f * 4.3f - this->steeringParameters[2],
                                                  -(100.0f * 4.3f - this->steeringParameters[2]));
                HuSubVecF(&step, &this->destination, &actorPosition);
            } while (step.length() < 300.0f);
            this->needsDestination = 0;
        } else {
            HuSubVecF(&step, &this->destination, &actorPosition);
            if (step.length() > 50.0f) {
                PSVECNormalize(&step, &result);
            } else {
                this->needsDestination = 1;
            }
        }

        if (this->player->getCollisionFrames() > secondsToFrames(1.2f - 0.3 * this->difficulty) +
            this->player->getPlayerNumber() * 10) {
            this->player->setCollisionFrames(0);
            PSVECScale(&this->destination, &this->destination, -1.0f);
        }
    } else {
        this->needsDestination = 1;
    }
    return result;
}

// Distance at which the computer flees the moving actor, by difficulty.
const f32 lbl_1_rodata_E8[4] = { 2.0f, 2.5f, 2.5f, 3.0f };

// Returns the distance at which the computer flees the moving actor.
f32 fn_1_2450(s32 index)
{
    return 100.0f * lbl_1_rodata_E8[index];
}

// Steering margins from the side walls, by difficulty.
const f32 lbl_1_rodata_FC[4] = { -1.0f, -1.0f, 1.5f, 2.0f };

// Returns the steering margin from the side walls.
f32 fn_1_2478(s32 index)
{
    return 100.0f * lbl_1_rodata_FC[index];
}

// Steering margins from the end walls, by difficulty.
const f32 lbl_1_rodata_10C[4] = { -1.5f, -1.5f, 2.0f, 2.5f };

// Returns the steering margin from the end walls.
f32 fn_1_24A0(s32 index)
{
    return 100.0f * lbl_1_rodata_10C[index];
}

// Steering interval in seconds for each difficulty.
const f32 lbl_1_rodata_11C[4] = { 0.1f, 0.3f, 0.3f, 0.3f };

// Returns the steering interval in seconds for this difficulty.
f32 fn_1_24C8(s32 index)
{
    return lbl_1_rodata_11C[index];
}

// Stores the projected point and returns the distance to it. A projection parameter
// outside [0, 1] returns -1 without writing the output. Coincident endpoints leave an
// unguarded zero denominator.
f32 fn_1_24E0(const Vec *point, const Vec *lineStart, const Vec *lineEnd,
              Vec *nearestPoint)
{
    Vec lineDirection;
    Vec pointFromStart;
    Vec scaledDirection;
    Vec perpendicular;
    f32 directionLengthSquared;
    f32 projectionNumerator;
    f64 projection;

    HuSubVecF(&lineDirection, lineEnd, lineStart);
    HuSubVecF(&pointFromStart, point, lineStart);
    directionLengthSquared = PSVECDotProduct(&lineDirection, &lineDirection);
    projectionNumerator = PSVECDotProduct(&lineDirection, &pointFromStart);
    projection = projectionNumerator / directionLengthSquared;
    if ((projection < 0.0) || (projection > 1.0)) {
        return -1.0f;
    }

    PSVECScale(&lineDirection, &scaledDirection, (f32)projection);
    PSVECAdd(lineStart, &scaledDirection, nearestPoint);
    HuSubVecF(&perpendicular, &scaledDirection, &pointFromStart);
    return PSVECMag(&perpendicular);
}

// Sorts the supplied f32 values in ascending order.
void fn_1_25F8(f32 *values, s32 count)
{
    s32 i;
    s32 j;
    f32 value;

    if (count > 0) {
        for (i = 0; i < count - 1; i++) {
            for (j = i + 1; j < count; j++) {
                if (values[i] > values[j]) {
                    value = values[j];
                    values[j] = values[i];
                    values[i] = value;
                }
            }
        }
    }
}

// Provides the default controller hook with no input change.
void PlayerController::fn_1_2690()
{
}
