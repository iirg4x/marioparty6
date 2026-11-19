#include "REL/m605DLL/timed_model_object.h"
#include "REL/framework/p_info.h"
#include "REL/framework/frame_durations.h"
#include "REL/m605DLL/module_globals.h"
#include "REL/m605DLL/falling_rock.h"
extern "C" {
#include "math.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
}
#include "REL/m605DLL/player_states.h"
#include "REL/m605DLL/player_character.h"

extern "C" {
#include "game/audio.h"
#include "game/data.h"
#include "datadir_enum.h"
}
#include "REL/m605DLL/animation_owner.h"
#include "REL/framework/chara_motion.h"
#include "REL/framework/tri_colli.h"
#include "REL/framework/registered_value.h"

const char *lbl_1_data_328[] = {
    "STATE_IDLE",
    "STATE_KYORO",
    "STATE_BIKKURI",
    "STATE_JUMP",
    "STATE_FALL",
    "STATE_RUN",
    "STATE_HIT",
    "STATE_DIE",
    "STATE_STUMBLE",
    "STATE_STANDUP",
    "STATE_WALK",
    "STATE_VICTORY",
};

// Returns the state name selected by the callback table index.
const char *fn_1_5BF8(OwnerCallback *callback) {
    return lbl_1_data_328[callback->state];
}

// Starts the idle character motion.
void IdleState::enter(PlayerCharacter *player)
{
    fn_1_8B20(player, DATANUM(0x930000, 0), 1);
}

// Stops character velocity while either ground flag is set.
void IdleState::advance(PlayerCharacter *player)
{
    if ((player->rayHit | player->contactFacesUp) != 0) {
        player->velocity.x = 0.0f;
        player->velocity.y = 0.0f;
        player->velocity.z = 0.0f;
    }
}

// Starts the alternate idle character motion.
void AlternateIdleState::enter(PlayerCharacter *player)
{
    fn_1_8B20(player, DATANUM(0x930000, 23), 1);
}

// Starts the reverse-facing idle motion and sets its angle.
void ReverseIdleState::enter(PlayerCharacter *player)
{
    fn_1_8B20(player, DATANUM(0x930000, 24), 0);
    player->angle =
        player->targetAngle = 180.0f;
}

// Starts the jump character motion.
void JumpState::enter(PlayerCharacter *player)
{
    fn_1_8B20(player, DATANUM(0x930000, 3), 0);
}

// Switches from jumping to falling once vertical velocity is nonpositive.
void JumpState::advance(PlayerCharacter *player)
{
    if (player->velocity.y <= 0.0f) {
        fn_1_7A68(player, new FallState);
        return;
    }
}

// Initializes the falling state countdown.
void FallState::enter(PlayerCharacter *) {
    framesRemaining = 5;
}

// Finishes the landing countdown and selects walking or running.
void FallState::advance(PlayerCharacter *player)
{
    if (player->rayHit | player->contactFacesUp) {
        fn_1_8B20(player, DATANUM(0x930000, 4), 0);
        if ((framesRemaining -= 1) <= 0) {
            if (player->body.center.z > 48500.0f) {
                fn_1_7A68(player, new WalkState);
                return;
            }

            fn_1_7A68(player, new RunState);
        }
    }
}

// Starts the running motion and records its start time.
void RunState::enter(PlayerCharacter *player)
{
    fn_1_8B20(player, DATANUM(0x930000, 2), 1);
    lastActiveTime = lbl_1_bss_3C;
}

// Updates running movement and switches state for obstacles or lost ground.
void RunState::advance(PlayerCharacter *player)
{
    TriangleCollision * collision;
    Vector3 intersection;
    Vector3 start;
    Vector3 end;
    bool pathBlocked;
    f32 slopeProgress;
    f32 limitedVelocity;
    if (player->body.center.z > 48500.0f)
    {
        fn_1_7A68(player, new WalkState);
        return;
    }
    if ((player->rayHit | player->contactFacesUp) != 0)
    {
        if ((u8)player->jumpRequested != 0 && player->body.center.z < 46000.0f)
        {
            player->velocity.y = 0.0f;
            player->acceleration.y += (160.0 / 6.0);
            fn_1_7A68(player, new JumpState);
            return;
        }
        static_cast<Vec &>(start) = player->body.center;
        start.y += 20.0f;
        end = start;
        end.z += 80.0f;
        collision = player->GetCollisionObject();
        pathBlocked = false;
        if (collision->intersectSegment(&intersection, &start, &end))
        {
            pathBlocked = collision->getTriangleMetadata() == 0;
        }
        if (!pathBlocked)
        {
            pathBlocked = fn_1_59C4(&player->body);
        }
        if (pathBlocked)
        {
            fn_1_7A68(player, new StumbleState);
            return;
        }
        lastActiveTime = lbl_1_bss_3C;
    }
    else
    {
        f32 inactiveTime = lbl_1_bss_3C - lastActiveTime;
        if ((inactiveTime > 0.12345f) &&
            (((slopeProgress = (player->body.center.z - 35700.0f) / 1200.0f) < -0.4f) ||
             (slopeProgress > 1.4f)))
        {
            fn_1_7A68(player, new JumpState);
            return;
        }
    }
    player->velocity.z = 15.0f;
    Vec movement = player->movementInput;
    if (player->body.center.z > 46000.0f)
    {
        movement.x = 0.0f;
        movement.z = 1.0f;
    }
    PSVECScale(&movement, &movement, 5.0f);
    PSVECAdd(&movement, &player->velocity, &player->velocity);
    f32 velocity = player->velocity.x;
    if (velocity < -5.0f)
    {
        limitedVelocity = -5.0f;
    }
    else
    {
        if (velocity > 5.0f)
        {
            limitedVelocity = 5.0f;
        }
        else
        {
            limitedVelocity = velocity;
        }
    }
    player->velocity.x = limitedVelocity;
    if (player->movementInput.x == 0.0f)
    {
        player->velocity.x *= 0.8f;
    }
}

// Starts the stumble motion, vertical impulse and recovery timer.
void StumbleState::enter(PlayerCharacter *player)
{
    fn_1_8B20(player, DATANUM(0x930000, 17), 0);
    player->acceleration.y += (70.0 / 6.0);
    duration = (s32)secondsToFrames(0.5f);
}

// Enters recovery and creates a landing effect when the stumble timer ends.
void StumbleState::advance(PlayerCharacter *player)
{
    if (--duration <= 0 &&
        ((player->rayHit) |
         (player->contactFacesUp))) {

        fn_1_7A68(player, new StandUpState);
        player->getResource()->rumble(1, 24);
        fn_1_6F4(&player->body.center);
        return;
    }
}

// Starts the stand-up motion and stops horizontal movement.
void StandUpState::enter(PlayerCharacter *player)
{
    fn_1_8B20(player, DATANUM(0x930000, 19), 0);
    player->velocity.x = 0.0f;
    player->velocity.y = 0.0f;
    player->velocity.z = 0.0f;
    framesRemaining = (s32)secondsToFrames(0.5f);
}

// Returns to running when the stand-up countdown ends.
void StandUpState::advance(PlayerCharacter *player)
{
    if (--framesRemaining <= 0) {

        fn_1_7A68(player, new RunState);
    }
}

// Starts the hit motion, rumble and character sound.
void HitState::enter(PlayerCharacter *player)
{
    fn_1_8B20(player, DATANUM(0x930000, 18), 0);
    player->getResource()->rumble(2, 24);
    CharFXPlay((s16)player->getResource()->characterNo(), 0x24C);
}

// Checks hit motion completion on the ground and updates its projected sphere.
void HitState::advance(PlayerCharacter *player)
{
    Sphere *position;

    if ((player->rayHit) | (player->contactFacesUp)) {
        if (characterMotionEnded(&player->getResource()->charNo) != 0) {
            fn_1_7A68(player, new DeathState);
            return;
        }
    }

    position = &player->body;
    f32 restoreVelocity = position->radius;
    f32 velocity = restoreVelocity;
    position->center.y += velocity;
    fn_1_1E38(lbl_1_bss_30, position);
    position->center.y -= velocity;
}

// Initializes the death scale and starts controller rumble.
void DeathState::enter(PlayerCharacter *player)
{
    scale = 1.0f;
    player->getResource()->rumble(2, 24);
}

// Stops movement and reduces the player's scale when behind the rock.
void DeathState::advance(PlayerCharacter *player)
{
    player->velocity.x = 0.0f;
    player->velocity.y = 0.0f;
    player->velocity.z = 0.0f;
    player->acceleration.x = 0.0f;
    player->acceleration.y = 0.0f;
    player->acceleration.z = 0.0f;

    if (player->body.center.z < lbl_1_bss_34) {
        player->projectedPosition.clearFlag7();
    }

    f32 range = 200.0f;
    f32 normalizedHeight =
        (player->body.center.z - (lbl_1_bss_34 + range)) / range;
    f32 scale;
    if (normalizedHeight < 0.1f) {
        scale = 0.1f;
    } else if (normalizedHeight > 1.0f) {
        scale = 1.0f;
    } else {
        scale = normalizedHeight;
    }
    this->scale = scale;
    f64 storedScale = this->scale;

    Vec scaleVector;
    scaleVector.x = 1.0f;
    scaleVector.y = (f32)storedScale;
    scaleVector.z = 1.0f;
    player->getResource()->setScale(&scaleVector);
}

// Starts the walking character motion.
void WalkState::enter(PlayerCharacter *player)
{
    fn_1_8B20(player, DATANUM(0x930000, 1), 1);
}

// Moves toward the finish position and enters idle when it is reached.
void WalkState::advance(PlayerCharacter *player)
{
    f64 positionX;
    f64 positionY;
    Vec target;
    Vec difference;
    Vec *positionAlias;
    Vec *position;
    f32 distanceForScale;
    f32 distance;

    positionY = player->body.center.y;
    positionX = player->targetX;
    target.x = positionX;
    target.y = positionY;
    target.z = 49100.0f;
    PSVECSubtract(&target, &player->body.center, &difference);
    distanceForScale = PSVECMag(&difference);
    distance = distanceForScale;
    if (distance < (50.0f / 6.0f)) {
        positionAlias = &player->body.center;
        position = positionAlias;
        position->x = target.x;
        position->y = target.y;
        position->z = target.z;
        player->velocity.x = 0.0f;
        player->velocity.y = 0.0f;
        player->velocity.z = 0.0f;
        player->targetAngle = 0.0f;
        player->atFinish = 1;
        fn_1_7A68(player, new IdleState);
        return;
    }
    if (player->rayHit | player->contactFacesUp) {
        PSVECScale(&difference, &player->velocity,
                   (50.0f / 6.0f) / distance);
        player->velocity.y = 0.0f;
    }
}
