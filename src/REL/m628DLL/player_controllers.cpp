#include "math.h"

#include "dolphin/mtx.h"

extern "C"
{
#include "dolphin/types.h"
}

#include "REL/m628DLL/player_actor.h"

extern "C"
{
#include "game/pad.h"
}

extern "C"
{
#include "game/gamework.h"
}

#include "REL/m628DLL/player_input_controller.h"

#include "REL/framework/random.h"

#include "REL/m628DLL/board_positions.h"

#include "REL/framework/pad_control.h"

#include "REL/framework/singleton.h"

#include "REL/framework/vector3.h"

#include "REL/m628DLL/computer_lever_controller.h"
#include "REL/m628DLL/computer_player_controller.h"
#include "REL/m628DLL/player_input_binding.h"
#include "REL/m628DLL/player_controller.h"

// Finds the intersection of a movement segment with a board boundary.
s32 fn_1_4EF0(Vec *start, Vec *end, Vec *firstCorner, Vec *secondCorner, Vec *intersection)
{
    Vec movement;
    Vec boundary;
    Vec relativeStart;
    Vec relativeCorner;
    f32 absoluteMovementDenominator;
    f32 absoluteBoundaryDenominator;

    if (start->x >= end->x)
    {
        if (((start->x < firstCorner->x) && (start->x < secondCorner->x)) ||
            ((end->x > firstCorner->x) && (end->x > secondCorner->x)))
        {
            return -1;
        }
    }
    else if (((end->x < firstCorner->x) && (end->x < secondCorner->x)) ||
             ((start->x > firstCorner->x) && (start->x > secondCorner->x)))
    {
        return -1;
    }
    if (start->z >= end->z)
    {
        if (((start->z < firstCorner->z) && (start->z < secondCorner->z)) ||
            ((end->z > firstCorner->z) && (end->z > secondCorner->z)))
        {
            return -1;
        }
    }
    else if (((end->z < firstCorner->z) && (end->z < secondCorner->z)) ||
             ((start->z > firstCorner->z) && (start->z > secondCorner->z)))
    {
        return -1;
    }

    PSVECSubtract(end, start, &movement);
    PSVECSubtract(secondCorner, firstCorner, &boundary);
    PSVECSubtract(start, firstCorner, &relativeStart);
    PSVECSubtract(firstCorner, start, &relativeCorner);
    f32 movementDenominator = boundary.x * movement.z - boundary.z * movement.x;
    f32 boundaryDenominator = movement.x * boundary.z - movement.z * boundary.x;
    absoluteMovementDenominator = __fabsf(movementDenominator);
    if (absoluteMovementDenominator < 1.0e-7)
    {
        return 0;
    }
    absoluteBoundaryDenominator = __fabsf(boundaryDenominator);
    if (absoluteBoundaryDenominator < 1.0e-7)
    {
        return 0;
    }
    f64 movementRatio = (f64)(boundary.x * relativeCorner.z - boundary.z * relativeCorner.x) /
                        (f64)movementDenominator;
    f64 boundaryRatio = (f64)(movement.x * relativeStart.z - movement.z * relativeStart.x) /
                        (f64)boundaryDenominator;
    if (movementRatio < 0.0 || movementRatio > 1.0 || boundaryRatio < 0.0 || boundaryRatio > 1.0)
    {
        return -2;
    }
    intersection->x = (f32)((f64)firstCorner->x + (f64)boundary.x * boundaryRatio);
    intersection->z = (f32)((f64)firstCorner->z + (f64)boundary.z * boundaryRatio);
    return 1;
}

// Clears the input state and invokes the controller.
void ActorController::fn_1_525C()
{
    state.x = 0.0f;
    state.y = 0.0f;
    state.pressedButtons = 0;
    state.magnitude = 0.0f;
    invoke(&state);
}

// Clears the input state and invokes the player controller.
void fn_1_52D4(PlayerInputController *object)
{
    object->state.x = 0.0f;
    object->state.y = 0.0f;
    object->state.pressedButtons = 0;
    object->state.magnitude = 0.0f;
    object->invoke(&object->state);
}

// Binds the pad controller to its player actor.
LeverPadController::LeverPadController(PlayerActor *actor) : ActorController(actor) {}

// Releases the actor input controller.
ActorController::~ActorController() {}

// Copies pad axes and buttons into the actor input state.
void LeverPadController::invoke(StickState *state)
{
    s32 padIndex = actor->padNo();
    f32 x = lbl_8024C7D8[padIndex];
    f32 y = lbl_8024C808[padIndex];
    f64 magnitude = sqrt((f64)(x * x + y * y));

    state->x = x;
    state->y = y;
    state->magnitude = (f32)magnitude;
    state->pressedButtons = lbl_802C0008[padIndex];
    state->heldButtons = lbl_802C0010[padIndex];
}

// Binds the player input controller to the configured pad.
PlayerInputBinding::PlayerInputBinding(ActPlayer *owner) : PlayerInputController(owner)
{
    s32 playerIndex = owner->playerNo;
    padNo = lbl_80265728[playerIndex].padNo;
}

// Captures one controller's stick position, magnitude, and button state.
void PlayerInputBinding::invoke(StickState *state)
{
    s32 padIndex = padNo;
    f32 x = lbl_8024C7D8[padIndex];
    f32 y = lbl_8024C7E8[padIndex];
    f64 magnitude = sqrt((f64)(x * x + y * y));

    state->x = x;
    state->y = y;
    state->magnitude = (f32)magnitude;
    state->pressedButtons = lbl_802C0008[padIndex];
    state->heldButtons = lbl_802C0010[padIndex];
}

extern "C"
{
extern u32 lbl_1_data_224[4];
}

Vec fn_1_62D8(ComputerPlayerController *);

u8 fn_1_5FAC(ComputerPlayerController *);

// Returns the player mode used to exclude a corner choice.
static inline s32 ownerMode(ComputerPlayerController *object)
{
    ActPlayer *target = object->owner;
    return target->mode;
}

// Returns the requested board corner position.
static inline Vec *positionTableEntryGet(BoardPositions *table, s32 index)
{
    return &table->cornerPositions[index];
}

// Returns the requested board corner score.
static inline s32 positionTableScoreGet(BoardPositions *table, s32 index)
{
    return table->cornerStates[index];
}

// Initializes the computer player target and movement state.
ComputerPlayerController::ComputerPlayerController(ActPlayer *target, s32 difficulty)
    : PlayerInputController(target)
{
    BoardPositions *table;

    this->difficulty = difficulty;
    this->position.x = 0.0f;
    this->position.y = 0.0f;
    this->position.z = 0.0f;
    setCallback(&ComputerPlayerController::fn_1_5D6C);
    table = &BoardPositions::getChecked();
    this->selectedIndex = fn_1_128C4(table, this->owner->positionValue());
    this->unk54 = this->selectedIndex;
    this->state = 0;
    this->unk5C = 0.0f;
}

// Releases the computer player controller.
ComputerPlayerController::~ComputerPlayerController() {}

// Chooses a board corner other than the excluded corner.
void ComputerPlayerController::invoke()
{
    BoardPositions *table;
    Vec *cornerPosition;
    static const s32 excludedCorners[4] = {3, 3, 2, 1};
    s32 ownerModeValue;
    s32 choice;
    s32 attempts;

    table = &BoardPositions::getChecked();
    ownerModeValue = ownerMode(this);
    choice = 0;
    attempts = 0;
    while (attempts < 10)
    {
        choice = RandomUnsigned() & 3;
        if (choice != excludedCorners[ownerModeValue])
        {
            break;
        }
        ++attempts;
    }
    cornerPosition = positionTableEntryGet(table, choice);
    this->position.x = cornerPosition->x;
    this->position.y = cornerPosition->y;
    this->position.z = cornerPosition->z;
}

// Updates the board corner and supplies input while the player is spawned.
void ComputerPlayerController::invoke(StickState *argument)
{
    BoardPositions *table;
    ActPlayer *positionOwner;
    ActPlayer *spawnOwner;
    ActPlayer *inputOwner;

    table = &BoardPositions::getChecked();
    positionOwner = this->owner;
    Vec playerPosition = positionOwner->positionValue();
    this->selectedIndex = fn_1_128C4(table, playerPosition);
    spawnOwner = this->owner;
    if (spawnOwner->spawned != 0)
    {
        inputOwner = this->owner;

        this->inputState.applyToPlayer(inputOwner->getPlayer());
        if (this->callback != 0)
        {
            (this->*this->callback)(argument);
        }
        this->state++;
        this->unk5C += (1.0 / 60.0);
    }
}

// Moves the computer player toward its selected target.
void ComputerPlayerController::fn_1_5D6C(StickState *)
{
    Vec offset;
    Vec currentPosition = ownerPositionValue();
    PSVECSubtract(&position, &currentPosition, &offset);
    f32 offsetMagnitude = PSVECMag(&offset);
    f32 distance = offsetMagnitude;
    BoardPositions *table = &BoardPositions::getChecked();
    s32 cornerRank = table->cornerRank(selectedIndex);
    Vec tablePosition = table->firstTrackedPosition();
    Vec flatOffset;
    PSVECSubtract(&tablePosition, &currentPosition, &flatOffset);
    flatOffset.y = 0.0f;
    f32 flatMagnitude = PSVECMag(&flatOffset);
    f32 headingDistance = flatMagnitude;
    if (cornerRank >= 2 && distance < 200.0f)
    {
        position = fn_1_62D8(this);
    }
    if (distance < 100.0f)
    {
        offset.x = 0.0f;
        offset.y = 0.0f;
        offset.z = 0.0f;
    }
    else
    {
        PSVECNormalize(&offset, &offset);
        PSVECScale(&offset, &offset, ((200.0f * 2.8f) / 60.0));
    }
    inputState.updateMovement(&offset, 0.4f);
    if (fn_1_5FAC(this))
    {
        inputState.pressA();
    }
}

// Returns the player rotation used for target selection.
static inline f32 targetRotation(ComputerPlayerController *object)
{
    ActPlayer *target = object->owner;
    return target->rotation;
}

// Returns whether the target player was hit.
static inline u8 targetHit(ComputerPlayerController *object)
{
    ActPlayer *target = object->owner;
    return target->hit;
}

// Chooses a computer player steering action from the board state.
u8 fn_1_5FAC(ComputerPlayerController *object)
{
    s32 playerMode = object->owner->modeValue();
    BoardPositions *table;
    f32 scale;
    f64 firstBoundZ;
    f64 secondBoundZ;
    if ((u32)(object->state % lbl_1_data_224[object->difficulty]) == 0U)
    {
        return 0U;
    }
    if ((s32)((object->state == 0) % (playerMode * 5)) != 0)
    {
        return 0U;
    }
    table = &BoardPositions::getChecked();
    scale = (((200.0f * 2.8f) / 60.0) * 20.0);
    f32 rotation = targetRotation(object);
    Vector3 targetPosition = object->ownerPositionValue();
    Vector3 endpoint;
    Vector3 direction;
    Vector3 intersection;
    Vector3 bounds1;
    Vector3 bounds2;

    targetPosition.y = 0.0f;
    Vector3 tablePosition = table->firstTrackedPosition();
    PSVECSubtract(&object->position, &targetPosition, &direction);
    direction.y = 0.0f;
    PSVECNormalize(&direction, &direction);
    PSVECScale(&direction, &direction, scale);
    PSVECAdd(&direction, &targetPosition, &endpoint);
    bounds1.x = (f64)tablePosition.x;
    bounds1.y = 0.0f;
    bounds1.z = -600.0f;
    bounds2.x = (f64)tablePosition.x;
    bounds2.y = 0.0f;
    bounds2.z = 600.0f;
    if (fn_1_4EF0(&targetPosition, &endpoint, &bounds1, &bounds2, &intersection) == 1)
    {
        return 1U;
    }
    firstBoundZ = tablePosition.z;
    bounds1.x = -600.0f;
    bounds1.y = 0.0f;
    bounds1.z = firstBoundZ;
    secondBoundZ = tablePosition.z;
    bounds2.x = 600.0f;
    bounds2.y = 0.0f;
    bounds2.z = secondBoundZ;
    if (fn_1_4EF0(&targetPosition, &endpoint, &bounds1, &bounds2, &intersection) == 1)
    {
        return 1U;
    }
    if (targetHit(object) == 1)
    {
        return 1U;
    }
    return 0U;
}

// Selects the lowest scoring board corner.
Vec fn_1_62D8(ComputerPlayerController *owner)
{
    BoardPositions *positionTable;
    s32 bestScore;
    s32 score;
    s32 bestIndex;
    static const s32 choices[4] = {3, 2, 1, 0};
    s32 index;

    positionTable = &BoardPositions::getChecked();
    bestScore = 99;
    bestIndex = 0;
    index = 0;
    for (; index < 4; index++)
    {
        score = positionTableScoreGet(positionTable, index);
        if (index != choices[owner->selectedIndex] && bestScore > score)
        {
            bestScore = score;
            bestIndex = index;
        }
    }
    owner->unk54 = bestIndex;
    return *positionTableEntryGet(positionTable, bestIndex);
}

extern "C"
{
u32 lbl_1_data_224[4] = {5, 10, 40, 80};
}

// Initializes computer control for the lever actor.
ComputerLeverController::ComputerLeverController(PlayerActor *actor, s32 difficulty)
    : ActorController(actor)
{
    elapsed = 0.0f;
    this->difficulty = difficulty;
    speed = 1.0f;
    positionChoice = 0;
    requestCount = 0;
    duration = 0.0f;
    stickInput.x = 0.0f;
    stickInput.y = 0.0f;
    stickInput.z = 0.0f;
    setCallback(&ComputerLeverController::fn_1_6564);
}

// Releases the computer lever controller.
ComputerLeverController::~ComputerLeverController() {}

// Runs the selected computer input callback.
void ComputerLeverController::invoke(StickState *value)
{
    if (callback)
    {
        (this->*callback)(value);
    }
}

void fn_1_6758(ComputerLeverController *object);

// Copies the chosen movement vector into the input state.
void ComputerLeverController::fn_1_6564(StickState *output)
{
    output->x = this->stickInput.x;
    output->y = this->stickInput.y;
    output->magnitude = this->stickInput.z;
    fn_1_6758(this);
    this->setCallback(&ComputerLeverController::fn_1_65F0);
}

// Receives the three components of the controller position result.
struct PositionResult
{
    Vec value;
};

void fn_1_68FC(PositionResult *position, ComputerLeverController *request);

// Returns the actor displacement vector.
static inline Vec *actorPositionOf(PlayerActor *actor)
{
    return (Vec *)&actor->getDisplacement();
}

// Steers the actor toward the selected target.
void ComputerLeverController::fn_1_65F0(StickState *velocity)
{
    PositionResult targetPosition;
    Vec difference;
    PositionResult selectedPosition;
    const Vec *actorPosition;
    PlayerActor *controlledActor = this->actor;
    actorPosition = actorPositionOf(controlledActor);

    fn_1_68FC(&selectedPosition, this);
    targetPosition = selectedPosition;
    PSVECSubtract(&targetPosition.value, actorPosition, &difference);
    PSVECNormalize(&difference, &difference);
    velocity->magnitude = this->speed;
    velocity->x = difference.x * this->speed;
    velocity->y = -difference.z * this->speed;
    this->setStickInput(velocity->x, velocity->y, velocity->magnitude);
    this->elapsed += (1.0 / 60.0);
    if (!(this->elapsed < this->duration))
    {
        this->setCallback(&ComputerLeverController::fn_1_6564);
    }
}

extern "C"
{
s32 lbl_1_data_258[4] = {2, 3, 4, 5};
}

extern "C"
{
const f32 lbl_1_rodata_140[4] = {1.0f, 3.0f, 4.0f, 5.0f};
}

// Chooses the next computer movement duration and corner.
void fn_1_6758(ComputerLeverController *request)
{
    BoardPositions *table = &BoardPositions::getChecked();
    if (request->requestCount % lbl_1_data_258[request->difficulty] == 0 &&
        request->difficulty != 3)
    {
        for (s32 choice = 2; choice >= 0; --choice)
        {
            if (table->isLocked(choice + 1) != 1)
            {
                request->positionChoice = choice + 1;
                break;
            }
        }
    }
    else
    {
        for (s32 choice = 0; choice < 3; ++choice)
        {
            if (table->isLocked(choice + 1) != 1)
            {
                request->positionChoice = choice + 1;
                break;
            }
        }
    }
    request->elapsed = 0.0f;
    request->duration = RandomFloat(1.0f, lbl_1_rodata_140[request->difficulty]);
    if (request->difficulty < 2)
    {
        request->speed = RandomFloat(0.5f, 1.0f);
    }
    else
    {
        request->speed = 1.0f;
    }
    request->requestCount++;
}

// Returns the computer controller target position.
void fn_1_68FC(PositionResult *position, ComputerLeverController *request)
{
    PositionResult selectedPosition;
    BoardPositions *table;
    s32 positionChoice;

    selectedPosition.value.x = 0.0f;
    selectedPosition.value.y = 0.0f;
    selectedPosition.value.z = 0.0f;
    table = &BoardPositions::getChecked();
    positionChoice = request->positionChoice;
    if (table->isLocked(positionChoice))
    {
        request->setCallback(&ComputerLeverController::fn_1_6564);
    }
    else
    {
        const Vec *selected = table->getPosition(request->positionChoice);
        selectedPosition.value = *selected;
    }
    *position = selectedPosition;
}
