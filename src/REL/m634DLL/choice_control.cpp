extern "C" {
#include "dolphin/types.h"
}

extern "C" {
#include "math.h"
}

extern "C" {
#include "dolphin/mtx.h"
}

#include "REL/m634DLL/choice_work.h"

#include "REL/m634DLL/camera_control.h"

#include "REL/m634DLL/player_part.h"

#include "REL/framework/stick_state.h"

extern "C" {
#include "game/gamework.h"
}

extern "C" {
#include "game/pad.h"
}

extern "C" {
#include "humath.h"
}

#include "REL/framework/random.h"

#include "REL/m634DLL/scene.h"

#include "REL/framework/vector3.h"

#include "REL/framework/minigame_core.h"

#include "REL/framework/singleton.h"

extern "C" {
#include "game/hu3d.h"
}

#include "REL/m634DLL/choice_motion.h"

#include "REL/m634DLL/scene_points.h"

// Interpolates between yaw angles along the shorter arc.
f32 fn_1_65E4(f32 start, f32 end, f32 t)
{
    f32 delta;
    f32 result;

    delta = fmod(end - start, 360.0);
    if (delta < 0.0f) {
        delta += 360.0f;
    }
    if (delta > 180.0f) {
        delta -= 360.0f;
    }
    result = fmod(start + t * delta, 360.0);
    if (result < 0.0f) {
        result += 360.0f;
    }
    return result;
}

// Returns the yaw of a direction vector in degrees.
f32 fn_1_66E8(HuVecF *direction)
{
    return (180.0 * atan2(direction->x, direction->z)) / M_PI;
}

// Clears the input state and reads the next input.
void ActPlayerPart::updateInput()
{
    input.x = 0.0f;
    input.y = 0.0f;
    input.pressedButtons = 0;
    input.magnitude = 0.0f;
    readInput(&input);
}

// Selects the controller assigned to the player.
PlayerPadPart::PlayerPadPart(ActPlayer *player)
    : ActPlayerPart(player)
{
    s32 playerNo = player->playerIndex;
    padNo = lbl_80265728[playerNo].padNo;
}

// Reads controller stick values and buttons into the input state.
void PlayerPadPart::readInput(StickState *state)
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

// Releases the object and its member-owned resources.
inline ActPlayerPart::~ActPlayerPart() { }

// Initializes computer choice input and its starting callback.
PlayerChoicePart::PlayerChoicePart(ActPlayer *player, s32 level)
    : ActPlayerPart(player)
    , padControl(2, 0.0f)
{
    Callback initialCallback;
    difficulty = level;
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 0.0f;
    selectedIndex = 0;
    unk54 = 0;
    updateCount = 0;
    buttonDelay = 0;
    initialCallback = &PlayerChoicePart::fn_1_6CB0;
    callback = initialCallback;
}

// Releases the object and its member-owned resources.
PlayerChoicePart::~PlayerChoicePart() { }

struct Scene;

void fn_1_72F0(PlayerChoicePart *);

// Restarts computer choice selection.
void PlayerChoicePart::reset()
{
    setCallback(&PlayerChoicePart::fn_1_6F08);
}

// Does nothing.
void fn_1_6C20() { }

// Runs the choice callback and applies the resulting pad control.
void PlayerChoicePart::readInput(StickState *output)
{
    ActPlayer *context;
    MGPLAYER *player;
    MGPLAYER *actionPlayer;

    if (callback) {
        (this->*callback)(output);
    }
    context = owner;
    player = context->player;
    actionPlayer = player;
    padControl.applyToPlayer(actionPlayer);
    updateCount++;
}

// Does nothing.
void PlayerChoicePart::fn_1_6CB0(StickState *output) { }

// Moves toward the selected choice and updates the input direction.
void PlayerChoicePart::fn_1_6CB4(StickState *output)
{
    Scene &manager = MinigameSingleton<Scene>::getChecked();
    Vector3 movement;
    ActPlayer *actor = owner;
    Vec currentPosition = actor->position;
    PSVECSubtract(&position, &currentPosition, &movement);
    movement.y = 0.0f;
    float distance = movement.length();
    float step = 9.333333f;
    step *= 100.0f * (distance / 2.0f);
    if (step > 9.333333333333334)
        step = 9.333333f;
    if (step < 0.16666666666666666)
        step = 0.16666667f;
    output->magnitude = step / 9.333333333333334;
    PSVECNormalize(&movement, &movement);
    PSVECScale(&movement, &movement, step);
    padControl.updateMovement(&movement, 0.2f);
    u8 occupied = manager.fn_1_ED80(selectedIndex);
    if (occupied == 1) {
        setCallback(&PlayerChoicePart::fn_1_6F08);
        return;
    }
    if (!(distance > 40.0f)) {
        setCallback(&PlayerChoicePart::fn_1_70D8);
    }
}

// Chooses an available target and starts moving toward it.
void PlayerChoicePart::fn_1_6F08(StickState *output)
{
    s32 availableChoices[4];
    Scene &manager = MinigameSingleton<Scene>::getChecked();
    s32 totalChoices = manager.fn_1_ED70();
    s32 playerNo = owner->getPlayerMode();
    unk54 = 0;
    s32 availableCount = 0;
    for (s32 choiceIndex = 0; choiceIndex < totalChoices; choiceIndex++) {
        if (manager.fn_1_ED80(choiceIndex) != 1) {
            availableChoices[availableCount] = choiceIndex;
            availableCount++;
        }
    }
    selectedIndex = availableChoices[(((((s32)RandomUnsigned()) % availableCount) < 0)
            ? -(((s32)RandomUnsigned()) % availableCount)
            : (((s32)RandomUnsigned()) % availableCount))];
    manager.fn_1_EFF0(playerNo, selectedIndex);
    setPosition(fn_1_EF18(&manager, selectedIndex));
    Vec currentPosition = owner->getPosition();
    setCallback(&PlayerChoicePart::fn_1_6CB4);
}

const s32 buttonIntervals[4] = { 40, 25, 20, 18 };

// Moves toward the target home position.
void PlayerChoicePart::fn_1_70D8(StickState *output)
{
    Scene &manager = MinigameSingleton<Scene>::getChecked();
    Vec currentPosition = owner->getPosition();
    Vec targetPosition = fn_1_EF18(&manager, selectedIndex);
    Vector3 movement;
    PSVECSubtract(&targetPosition, &currentPosition, &movement);
    movement.y = 0.0f;
    if (movement.length() > 80.0f) {
        setCallback(&PlayerChoicePart::fn_1_6CB4);
    }
    fn_1_72F0(this);
    u32 interval = buttonDelay + buttonIntervals[difficulty];
    if (updateCount % interval == 0) {
        padControl.pressButtons(0x100);
        output->pressedButtons |= 0x100;
        buttonDelay = ((s32)RandomUnsigned()) & 3;
    }
    u8 occupied = manager.fn_1_ED80(selectedIndex);
    if (occupied != 0) {
        setCallback(&PlayerChoicePart::fn_1_6F08);
    }
}

extern "C" {
struct CameraPointDirectory;
}


// Turns the player toward its selected first-piece position.
void fn_1_72F0(PlayerChoicePart *work)
{
    Scene *scene;
    f32 angleDifference;
    f32 blendStartAngle;
    f32 resultAngle;
    f32 desiredAngleCopy;
    f32 adjustedAngle;
    f32 cameraAngle;
    f32 angleOffset;
    f32 desiredAngle;
    ActPlayer *actor;
    ActPlayer *angleCamera;

    /* MSL assert(): the singleton must exist */
    scene = &MinigameSingleton<Scene>::getChecked();
    actor = work->owner;
    Vec cameraPosition = actor->getPosition();
    Vec targetPosition = fn_1_EE50(scene, work->selectedIndex);
    Vec direction;
    PSVECSubtract(&targetPosition, &cameraPosition, &direction);
    direction.y = 0.0f;

    angleCamera = work->owner;
    cameraAngle = angleCamera->angle;
    angleOffset = cameraAngle;
    blendStartAngle = angleOffset;
    desiredAngle = (f32)(180.0 * atan2((double)direction.x, (double)direction.z) / M_PI);
    desiredAngleCopy = desiredAngle;
    angleDifference = (f32)fmod(desiredAngleCopy - blendStartAngle, 360.0);
    if (angleDifference < 0.0f)
        angleDifference += 360.0f;
    if (angleDifference > 180.0f)
        angleDifference -= 360.0f;

    resultAngle = (f32)fmod(blendStartAngle + (0.4f * angleDifference), 360.0);
    if (resultAngle < 0.0f)
        resultAngle += 360.0f;
    adjustedAngle = resultAngle;
    blendStartAngle = adjustedAngle;
    fn_1_22FC(work->owner, blendStartAngle);
}

// Initializes computer motion input and its selection callback.
PlayerMotionPart::PlayerMotionPart(ActPlayer *player, s32 level)
    : ActPlayerPart(player)
    , padControl(2, 0.0f)
{
    Callback values;
    selectionStrategy = level;
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 0.0f;
    selectedIndex = 0;
    unk5C = 0;
    transitionCount = 0;
    values = &PlayerMotionPart::fn_1_779C;
    callback = values;
}


// Releases the object and its member-owned resources.
PlayerMotionPart::~PlayerMotionPart() { }
// Restarts computer motion selection.
void PlayerMotionPart::reset()
{
    Callback value = &PlayerMotionPart::fn_1_7DC0;
    callback = value;
}

// Does nothing.
void fn_1_7718(void) { }

// Runs the motion callback and applies the resulting pad control.
void PlayerMotionPart::readInput(StickState *output)
{
    ActPlayer *state;
    MGPLAYER *player;
    MGPLAYER *actionPlayer;
    if (callback) {
        (this->*callback)(output);
    }
    state = owner;
    player = state->player;
    actionPlayer = player;
    padControl.applyToPlayer(actionPlayer);
}

// Does nothing.
void PlayerMotionPart::fn_1_779C(StickState *output) { }

struct CameraPointDirectory;

struct PointWords;

extern void fn_1_849C(PlayerMotionPart *work);







static const f32 stepScale = 100.0f;

// Moves along the selected scene path and updates the control state.
void PlayerMotionPart::fn_1_77A0(StickState *output)
{
    Vector3 movement;

    Scene &manager = MinigameSingleton<Scene>::getChecked();
    s32 selections;
    f32 distance;
    f32 step;
    u8 occupied;
    Vector3 currentPosition = getOwner()->position;
    PSVECSubtract(&position, &currentPosition, &movement);
    movement.y = 0.0f;
    distance = movement.length();
    step = 9.333333f;
    step *= stepScale * (distance / 2.0f);
    if (step > 9.333333333333334) {
        step = 9.333333f;
    }
    if (step < 0.16666666666666666) {
        step = 0.16666667f;
    }
    output->magnitude = step / 9.333333333333334;
    PSVECNormalize(&movement, &movement);
    PSVECScale(&movement, &movement, step);
    padControl.updateMovement(&movement, 0.2f);
    selections = owner->getPlayerMode();
    if (owner->getProgress() >= selections * 10) {
        owner->resetProgress();
        setCallback(&PlayerMotionPart::fn_1_7A68);
    }
    occupied = manager.fn_1_EDE8(selectedIndex);
    if (occupied == 1) {
        setCallback(&PlayerMotionPart::fn_1_7DC0);
        return;
    }
    if (!(distance > 40.0f)) {
        setCallback(&PlayerMotionPart::fn_1_82C8);
    }
}

// Moves toward the selected home position and waits for arrival.
void PlayerMotionPart::fn_1_7A68(StickState *output)
{
    Scene &manager = MinigameSingleton<Scene>::getChecked();
    f32 distance = 0.0f;
    s32 motion;
    s32 selectionCount = owner->getPlayerMode();
    Vector3 sourcePosition = owner->getPosition();
    Vector3 movement;

    f32 step;

    motion = fn_1_F0A8(&manager, selectedMotion, &selectedMotionTime);
    Vector3 targetPosition = fn_1_F044(&manager, motion);
    PSVECSubtract(&targetPosition, &sourcePosition, &movement);
    movement.y = 0.0f;
    distance = movement.length();
    if (distance < 50.0f) {
        ++selectedMotionTime;
    }
    if (owner->getProgress() >= selectionCount * 10) {
        fn_1_F13C(&manager, &selectedMotion, &selectedMotionTime, motion);
        owner->resetProgress();
        ++transitionCount;
    }
    if (transitionCount > 3) {
        setCallback(&PlayerMotionPart::fn_1_7DC0);
        return;
    }
    step = 9.333333f;
    step *= 100.0f * distance;
    if (step > 9.333333333333334) {
        step = 9.333333f;
    }
    if (step < 0.016666666666666666) {
        step = 0.016666668f;
    }
    output->magnitude = step / 9.333333333333334;
    PSVECNormalize(&movement, &movement);
    PSVECScale(&movement, &movement, step);
    padControl.updateMovement(&movement, 0.2f);
    PSVECSubtract(&position, &sourcePosition, &movement);
    movement.y = 0.0f;
    distance = movement.length();
    if (!(distance > 140.0f)) {
        setCallback(&PlayerMotionPart::fn_1_77A0);
    }
}

// Selects a scene motion using the configured choice strategy.
void PlayerMotionPart::fn_1_7DC0(StickState *output)
{
    Scene &manager = MinigameSingleton<Scene>::getChecked();
    s32 selectedCount = manager.fn_1_ED78();
    s32 currentIndex;
    s32 wrappedChoice;
    currentIndex = owner->getPlayerMode();
    s32 choiceCount;
    s32 choices[12];
    s32 excludedChoices[2];
    s32 excludedCount;
    s32 index;
    unk5C = 0;
    choiceCount = manager.fn_1_ED78();
    excludedCount = 0;
    index = 0;
    while (index < 4) {
        if ((currentIndex != index) && (index != 0)) {
            s32 excludedChoice = manager.fn_1_EFE0(index);
            excludedChoices[excludedCount] = excludedChoice;
            ++excludedCount;
        }
        ++index;
    }
    Vector3 position = owner->getPosition();
    s32 optionCount = 0;
    fn_1_F4F8(&manager, &position, choices, &optionCount);
    if (optionCount != 0) {
        switch (selectionStrategy) {
        case 0: {

            selectedIndex = choices[((((s32)RandomUnsigned()) % optionCount) < 0)
                    ? -(((s32)RandomUnsigned()) % optionCount)
                    : (((s32)RandomUnsigned()) % optionCount)];
            break;
        }
        case 1:
            if (optionCount >= 3) {

                selectedIndex
                    = choices[((((s32)RandomUnsigned()) % 3) < 0) ? -(((s32)RandomUnsigned()) % 3)
                                                                  : (((s32)RandomUnsigned()) % 3)];
            } else {
                selectedIndex = choices[0];
            }
            break;
        case 2: {
            s32 seed = ((s32)RandomUnsigned()) & 1;
            for (s32 loopIndex = 0; loopIndex < optionCount; ++loopIndex) {
                s32 choice;

                choice = choices[(((loopIndex + seed) % optionCount) < 0)
                        ? -((loopIndex + seed) % optionCount)
                        : ((loopIndex + seed) % optionCount)];
                if ((optionCount < 3)
                    || ((excludedChoices[0] != choice) && (excludedChoices[1] != choice))) {
                    selectedIndex = choice;
                    break;
                }
            }
            break;
        }
        case 3: {
            s32 seed = ((s32)RandomUnsigned()) & 1;
            for (s32 loopIndex = 0; loopIndex < optionCount; ++loopIndex) {
                s32 choice;
                wrappedChoice = loopIndex;
                if (optionCount >= 9) {
                    wrappedChoice = (((seed + loopIndex) % optionCount) < 0)
                        ? -((seed + loopIndex) % optionCount)
                        : ((seed + loopIndex) % optionCount);
                }
                choice = choices[wrappedChoice];
                if ((optionCount < 3)
                    || ((excludedChoices[0] != choice) && (excludedChoices[1] != choice))) {
                    selectedIndex = choice;
                    break;
                }
            }
            break;
        }
        }
        manager.fn_1_EFF0(currentIndex, selectedIndex);
        setPosition(fn_1_EF7C(&manager, selectedIndex));
        s32 result
            = fn_1_F23C(&manager, &position, &selectedMotion, &selectedMotionTime, selectedIndex);
        if ((selectionStrategy == 3) && (optionCount >= 3)) {
            s32 value = fn_1_F0A8(&manager, selectedMotion, &selectedMotionTime);
            if (fn_1_F72C(&manager, selectedMotion, selectedMotionTime, result) == 0) {
                fn_1_F13C(&manager, &selectedMotion, &selectedMotionTime, (s32)value);
            }
        }
        transitionCount = 0;
        setCallback(&PlayerMotionPart::fn_1_7A68);
    }
}

// Moves toward the selected second-piece home position.
void PlayerMotionPart::fn_1_82C8(StickState *output)
{
    Scene &manager = MinigameSingleton<Scene>::getChecked();
    ActPlayer *actor = owner;

    f32 distance;
    u8 occupied;

    Vector3 sourceCopy = actor->getPosition();
    Vec targetCopy = fn_1_EF7C(&manager, selectedIndex);
    Vec movement;
    PSVECSubtract(&targetCopy, &sourceCopy, &movement);
    movement.y = 0.0f;
    distance = PSVECMag(&movement);
    if (distance > 80.0f) {
        setCallback(&PlayerMotionPart::fn_1_7A68);
    }
    fn_1_849C(this);
    padControl.pressButtons(0x100);
    output->pressedButtons |= 0x100;
    occupied = manager.fn_1_EDE8(selectedIndex);
    if (occupied != 0) {
        setCallback(&PlayerMotionPart::fn_1_7DC0);
    }
}

// Turns the player toward its selected second-piece position.
void fn_1_849C(PlayerMotionPart *work)
{

    Scene *scene;
    f32 angleDifference;
    f32 blendStartAngle;
    f32 resultAngle;
    f32 desiredAngleCopy;
    f32 adjustedAngle;
    f32 cameraAngle;
    f32 angleOffset;
    f32 desiredAngle;
    ActPlayer *actor;
    ActPlayer *angleCamera;

    /* MSL assert(): the singleton must exist */
    scene = &MinigameSingleton<Scene>::getChecked();
    actor = work->owner;

    Vector3 cameraPosition = actor->getPosition();
    Vector3 targetPosition = fn_1_EEB4(scene, work->selectedIndex);
    Vector3 direction;
    PSVECSubtract(&targetPosition, &cameraPosition, &direction);
    direction.y = 0.0f;

    angleCamera = work->owner;
    cameraAngle = angleCamera->angle;
    angleOffset = cameraAngle;
    blendStartAngle = angleOffset;
    desiredAngle = (f32)(180.0 * atan2((double)direction.x, (double)direction.z) / M_PI);
    desiredAngleCopy = desiredAngle;
    angleDifference = (f32)fmod(desiredAngleCopy - blendStartAngle, 360.0);
    if (angleDifference < 0.0f)
        angleDifference += 360.0f;
    if (angleDifference > 180.0f)
        angleDifference -= 360.0f;

    resultAngle = (f32)fmod(blendStartAngle + (0.4f * angleDifference), 360.0);
    if (resultAngle < 0.0f)
        resultAngle += 360.0f;
    adjustedAngle = resultAngle;
    blendStartAngle = adjustedAngle;
    fn_1_22FC(work->owner, blendStartAngle);
}
