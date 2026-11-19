#include "REL/framework/world_units.h"
#include "REL/framework/p_info.h"
#include "REL/m605DLL/timed_model_object.h"
#include <math.h>
#include "REL/m605DLL/module_globals.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/random.h"
extern "C" {
#include "dolphin/types.h"
}

#include "dolphin/mtx.h"

#include "REL/m605DLL/motion_controller.h"
#include "REL/m605DLL/player_character.h"
#include "REL/framework/tri_colli.h"

#include "game/hu3d.h"

// Initializes movement timing and scale from the player difficulty.
MotionController *fn_1_A7D8(MotionController *result, PlayerCharacter *context)
{
    s32 shortWait;
    s32 mediumWait;
    s32 longWait;
    s32 longDuration;
    PlayerInfo *playerForScale;
    PlayerInfo *playerForSelection;

    result->duration = 0;
    result->sampledDuration = 0;
    result->secondaryDuration = 0;
    result->wait = 0;
    playerForScale = context->playerConfig.pointer;
    result->randomScale = (1.0f +
        (f32)(playerForScale)->difficulty()) / 4.0f;
    result->waitExpired = 0;
    result->duration = 10;
    playerForSelection = context->playerConfig.pointer;
    switch ((playerForSelection)->difficulty()) {
    case 0:
        shortWait = secondsToFrames(23L);
        result->wait = shortWait;
        break;
    case 1:
        mediumWait = secondsToFrames(38L);
        result->wait = mediumWait;
        break;
    case 2:
        longWait = secondsToFrames(50L);
        result->wait = longWait;
        longDuration = secondsToFrames(10L);
        result->duration = longDuration;
        break;
    }
    return result;
}

// Chooses a new lateral offset when the direction timer expires.
void fn_1_A90C(MotionController *controller, PlayerCharacter *player)
{
    u8 expired;
    s32 newDuration;

    player->movementInput.z = 0.8f;
    expired = 0;
    if (controller->duration > 0) {
        if (--controller->duration == 0) {
            expired = 1;
        }
    }
    if (expired != 0) {
        controller->direction = RandomFloat(-1.0f, 1.0f);
        newDuration = (s32)secondsToFrames(RandomFloat(0.3f, 1.5f));
        controller->duration = newDuration;
        return;
    }
    player->movementInput.x = controller->direction;
    controller->direction *= 0.95f;
}

// Selects obstacle avoidance or course-centering movement.
void fn_1_AA08(MotionController *controller, PlayerCharacter *owner) {
    owner->movementInput.z = 1.0f;
    if (fn_1_AF80(controller, owner) == 0) {
        fn_1_AD80(controller, owner);
    }
}

// Updates difficulty-dependent steering, waits and direction changes.
void fn_1_AA70(MotionController *controller, PlayerCharacter *owner)
{
    u8 waitExpired;
    f32 distance;
    s32 nextDuration;
    if (owner->GetPlayerConfiguration()->difficulty() == 3)
    {
        owner->movementInput.z = 1.0f;
        if (fn_1_AF80(controller, owner) == 0)
        {
            fn_1_AD80(controller, owner);
        }
        return;
    }
    distance = owner->body.center.z - lbl_1_bss_34;
    s32 newDuration;
    waitExpired = 0;
    if (controller->wait > 0)
    {
        if (--controller->wait == 0)
        {
            waitExpired = 1;
        }
    }
    if (waitExpired != 0)
    {
        controller->waitExpired = 1;
    }
    if (controller->waitExpired != 0)
    {
        u8 expired;
        s32 newDuration;
        owner->movementInput.z = 0.8f;
        expired = 0;
        if (controller->duration > 0)
        {
            if (--controller->duration == 0)
            {
                expired = 1;
            }
        }
        if (expired != 0)
        {
            controller->direction = RandomFloat(-1.0f, 1.0f);
            newDuration = (s32)secondsToFrames(RandomFloat(0.3f, 1.5f));
            controller->duration = newDuration;
        }
        else
        {
            owner->movementInput.x = controller->direction;
            controller->direction *= 0.95f;
        }
        return;
    }
    if (controller->secondaryDuration <= 0)
    {
        if (distance < 800.0f)
        {
            nextDuration = (s32)(controller->randomScale * (f32) secondsToFrames(5L));
            controller->secondaryDuration = nextDuration;
        }
        owner->movementInput.z = 0.5f * controller->randomScale;
    }
    else
    {
        owner->movementInput.z = 1.0f;
    }
    if ((controller->sampledDuration > 0) || (fn_1_AF80(controller, owner) == 0))
    {
        u8 expired;
        expired = 0;
        if (controller->duration > 0)
        {
            if (--controller->duration == 0)
            {
                expired = 1;
            }
        }
        if (expired != 0)
        {
            controller->direction = RandomFloat(-1.0f, 1.0f);
            newDuration = (s32)secondsToFrames(RandomFloat(2.0f, 3.0f));
            controller->duration = newDuration;
        }
        else
        {
            owner->movementInput.x = controller->direction;
            controller->direction *= 0.95f;
        }
        fn_1_AD80(controller, owner);
    }
}

// Steers toward the course center using its current width.
void fn_1_AD80(MotionController *controller, PlayerCharacter *player)
{
    f32 displacement;
    f32 clampedDisplacement;
    f32 scaledDisplacement;
    f32 alternateDisplacement;
    f32 alternateBlend;
    f32 alternateScaledDisplacement;
    f32 absoluteDisplacement;
    f32 alternateAbsoluteDisplacement;

    if ((player->body.center.z > 21500.0f) && (player->body.center.z < 26000.0f)) {
        displacement = player->body.center.x;
        if (displacement < 0.0f) {
            absoluteDisplacement = -displacement;
        } else {
            absoluteDisplacement = displacement;
        }
        if (absoluteDisplacement > 100.0f) {
            scaledDisplacement = -player->body.center.x / 100.0f;
            if (scaledDisplacement < -1.0f) {
                clampedDisplacement = -1.0f;
            } else if (scaledDisplacement > 1.0f) {
                clampedDisplacement = 1.0f;
            } else {
                clampedDisplacement = scaledDisplacement;
            }
            player->movementInput.x = clampedDisplacement;
        }
    } else {
        alternateDisplacement = player->body.center.x;
        if (alternateDisplacement < 0.0f) {
            alternateAbsoluteDisplacement = -alternateDisplacement;
        } else {
            alternateAbsoluteDisplacement = alternateDisplacement;
        }
        if (alternateAbsoluteDisplacement > 250.0f) {
            alternateScaledDisplacement = -player->body.center.x / 100.0f;
            if (alternateScaledDisplacement < -1.0f) {
                alternateBlend = -1.0f;
            } else if (alternateScaledDisplacement > 1.0f) {
                alternateBlend = 1.0f;
            } else {
                alternateBlend = alternateScaledDisplacement;
            }
            player->movementInput.x = alternateBlend;
        }
    }
}

// Checks objects and collision geometry ahead, then steers around an obstacle.
u8 fn_1_AF80(MotionController *controller, PlayerCharacter *owner)
{
    TriangleCollision *collisionObject;
    Vector3 hitPosition;
    Vector3 start;
    Vector3 end;
    f32 displacement;
    f32 randomChance;
    f32 absoluteDisplacement;
    f32 blend;
    f32 scaledDisplacement;
    s32 index;
    s32 sampledDuration;

    if (owner->body.center.z > 47010.0f) {
        return 0;
    }
    randomChance = controller->randomScale * RandomFloat(0.5f, 1.0f);
    if (randomChance < 0.1875f) {
        sampledDuration = (s32)secondsToFrames(RandomFloat(1.0f, 2.0f));
        controller->sampledDuration = sampledDuration;
    }

    collisionObject = owner->collisionObject.get();
    start.x = owner->body.center.x;
    start.y = owner->body.center.y;
    start.z = owner->body.center.z;
    start.y += 20.0f;
    end.x = start.x;
    end.y = start.y;
    end.z = start.z;
    end.z += 150.0f;
    for (index = -1; index < 2; index++) {
        start.x = end.x = owner->body.center.x + (f32)index * cmToWorld(40.0f);
        if (((u8)collisionObject->intersectSegment(&hitPosition, &start, &end)) != 0) {
            owner->jumpRequested = 1;
            owner->velocity.x = 0.0f;
            displacement = owner->body.center.x;
            if (displacement < 0.0f) {
                absoluteDisplacement = -displacement;
            } else {
                absoluteDisplacement = displacement;
            }
            if (absoluteDisplacement > 250.0f) {
                scaledDisplacement = -owner->body.center.x / 100.0f;
                if (scaledDisplacement < -1.0f) {
                    blend = -1.0f;
                } else if (scaledDisplacement > 1.0f) {
                    blend = 1.0f;
                } else {
                    blend = scaledDisplacement;
                }
                owner->movementInput.x = blend;
            }
            return 1;
        }
    }

    Sphere comparison(owner->body);
    comparison.center.z += 100.0f;
    if ((u8)fn_1_59C4(&comparison) != 0) {
        owner->jumpRequested = 1;
        owner->movementInput.x = 0.0f;
        return 1;
    }
    return 0;
}

// Initializes movement timing and scale from the player difficulty.
inline MotionController::MotionController(PlayerCharacter *owner)
{
    s32 longDuration;
    s32 longWait;
    s32 mediumWait;
    s32 shortWait;

    duration = 0;
    sampledDuration = 0;
    secondaryDuration = 0;
    wait = 0;
    randomScale = (1.0f + (f32)(owner->playerConfig.get())->difficulty()) / 4.0f;
    waitExpired = 0;
    duration = 10;
    switch ((owner->playerConfig.get())->difficulty()) {
    case 0:
        shortWait = secondsToFrames(23L);
        wait = shortWait;
        break;
    case 1:
        mediumWait = secondsToFrames(38L);
        wait = mediumWait;
        break;
    case 2:
        longWait = secondsToFrames(50L);
        wait = longWait;
        longDuration = secondsToFrames(10L);
        duration = longDuration;
        break;
    }
}

// Creates the movement controller on demand and updates its steering.
void fn_1_B2AC(PlayerCharacter *owner)
{
    if (owner->steering.get() == 0) {
        owner->steering.reset(new MotionController(owner));
    }
    fn_1_AA70(owner->steering.get(), owner);
}
