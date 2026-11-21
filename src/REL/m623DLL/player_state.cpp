// Player states, character models and collision updates.

extern "C" {
#include "dolphin/types.h"
#include "game/audio.h"
#include "msm_se.h"
#include "dolphin/mtx.h"
}

#include "REL/framework/frame_durations.h"
#include "REL/m623DLL/score_owner.h"
#include "REL/m623DLL/scene_collision.h"
#include "REL/m623DLL/item_effects_owner.h"
#include "REL/m623DLL/scene_storage.h"
#include "REL/framework/sound.h"
#include "REL/framework/list_object.h"
#include "REL/m623DLL/circle_entry.h"
#include "datadir_enum.h"
#include "REL/framework/chara_motion.h"
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "REL/framework/category_sphere.h"
#include "REL/m623DLL/player_state.h"
#include "REL/framework/misc_diff_deg.h"
#include "REL/framework/random.h"
#include "REL/framework/frame_time.h"
#include "REL/m623DLL/scene_camera_api.h"
#include "REL/framework/vector_math.h"
#include <new>
#include "REL/m623DLL/player_trail_particle.h"

// Sets the starting X and Z coordinates and samples their terrain height.
void fn_1_8BD4(Vec *position)
{
    position->x = -4859.523f;
    position->z = 4735.523f;
    position->y = fn_1_1ACC(position->x, position->z);
}

// Sets the current motion time to a fraction of its maximum.
void fn_1_8C2C(PlayerConfigSelection *character, f32 scale)
{
    f32 maximumTime = getMotionOwnerMaximumTime(&character->charNo);
    f32 baseline = 0.0f;
    scale = scale * (maximumTime - baseline);
    setMotionOwnerTime(&static_cast<MotionOwner &>(*character), scale);
    character->setAttribute(0x40000002);
}

// Centers the indexed value using the supplied spacing.
template <class Value> Value fn_1_9E94(s32 index, s32 count, Value spacing)
{
    --count;
    return spacing * index - spacing * count / 2.0f;
}

// Creates the character, moving body and registered interaction volumes.
PlayerState::PlayerState(s32 playerIndex, unsigned char mode, s32 slot)
    : GameObject(1), player(playerIndex), stateCountdown(0), body(new PlayerBody(slot == 0)),
      character(new PlayerConfigSelection(playerIndex, 3)), bodySphere(0), upperSphere(0),
      shakeAmplitude(0.0f), circle(0)
{
    static const unsigned int motionList[] = { DATANUM(0x930000, 113), DATANUM(0x930000, 59) };
    Vec position;
    bool secondary = 1 <= slot;
    fn_1_8BD4(&position);
    viewIndex = (secondary ? 1 : 0);
    createCharacterMotions(&static_cast<MotionOwner &>(*character), motionList, 2);
    if (secondary) {
        character->setCamera(2);
        bodySphere.setPriority(1);
        upperSphere.setPriority(1);
        // Spaces the three team members across Z.
        position.z += fn_1_9E94(slot - 1, 3, 250.0f);
        character->resetShadow();
        circle = fn_1_D940(1);
        circle->setScale(90.0f);
        circle->setAlpha(0.62f);
    } else {
        character->setCamera(1);
        bodySphere.setPriority(0);
        upperSphere.setPriority(0);
        positionModel.reset(new OwnedModel(DATANUM(0x520000, 28)));
        positionModel->setRotationY(90.0f);
        positionModel->setCamera(1);
        positionModel->setLayer(5);
        positionModel->setShadow();
        character->resetShadow();
    }
    targetRotation = rotation = 90.0f;
    character->setRotationY(rotation);
    character->setLayer(5);
    if (secondary) {
        selectCharacterMotion(&static_cast<MotionOwner &>(*character), DATANUM(0x930000, 59));
        f32 radius = 225.0f;
        bodySphere.radius = radius;
    } else {
        selectCharacterMotion(&static_cast<MotionOwner &>(*character), DATANUM(0x930000, 113));
        f32 radius = 270.0f;
        bodySphere.radius = radius;
    }
    motionTimeFraction = 0.5f;
    fn_1_8C2C(character, 0.5f);
    fn_1_CE5C(body, &position);
    eliminated = 0;
    // Both interaction spheres use the body sphere radius.
    upperSphere.setRadius(bodySphere.getRadius());
    upperSphere.clearFlag6();
    soundEffect = -1;
}

// Removes this player's record from the circle lists.
PlayerState::~PlayerState()
{
    fn_1_DA2C(circle);
}

void fn_1_9530(PlayerState *playerState);

void fn_1_9AC8(PlayerState *playerState);

// Advances the countdown and updates this player's eliminated state.
void PlayerState::update()
{
    if (currentRequest() == 2 && this->eliminated == 0) {
        if (player.isComputer() != 0) {
            fn_1_AE70(this);
        } else {
            fn_1_9A5C(this, player.stickX(), player.stickY());
        }
    }
    fn_1_9530(this);
    fn_1_9AC8(this);
    advanceStateCountdown();
}

// Clamps the supplied value to the unit interval.
static inline f32 clampVolume(f32 volume)
{
    if (volume < 0.0f) {
        return 0.0f;
    } else if (volume > 1.0f) {
        return 1.0f;
    } else {
        return volume;
    }
}

// Updates the character, interaction volumes and sound from the moving body.
void fn_1_9530(PlayerState *playerState)
{
    PlayerBody *body = playerState->body.get();
    Vector3 *bodyPosition;

    f32 stickInput;
    f32 motionTimeFraction;
    f32 shake;
    f32 rotation;

    if (body == 0) {
        return;
    }

    stickInput = body->getStickInput();
    motionTimeFraction = 0.5f + (-stickInput / 2.0f);
    playerState->motionTimeFraction +=
        0.2f * (motionTimeFraction - playerState->motionTimeFraction);
    motionTimeFraction = playerState->motionTimeFraction;
    fn_1_8C2C(playerState->character.get(), motionTimeFraction);

    bodyPosition = &body->position;
    Vector3 position = *bodyPosition;
    if (playerState->viewIndex == 0) {
        lbl_1_bss_44 = position.y;
        lbl_1_bss_48 = position.z;
    }
    playerState->bodySphere = position;
    playerState->upperSphere = position;
    // Raises the upper interaction sphere above the body.
    playerState->upperSphere.center.y += 600.0f;
    shake = playerState->shakeAmplitude.update();
    if (shake > 0.0f) {
        position.x += RandomFloat(-shake, shake);
        position.z += RandomFloat(-shake, shake);
    }

    if (currentRequest() == 2) {
        Vec *positionDelta = body->getPositionDelta();
        f32 speed = vectorMagnitude(positionDelta);
        rotation = (f32) (180.0 * (atan2((f64) positionDelta->x, (f64) positionDelta->z) / M_PI));
        if (positionDelta->x == 0.0f && positionDelta->z == 0.0f) {
            rotation = 90.0f;
        }
        playerState->targetRotation = rotation;
        playerState->rotation +=
            limitedDegreeDifference(playerState->targetRotation, playerState->rotation, 2.0f);
        if (playerState->soundEffect == -1) {
            if (playerState->viewIndex == 0) {
                playerState->soundEffect = HuAudFXPlayPan(0x6FB, 0x50);
            } else {
                playerState->soundEffect = HuAudFXPlayPan(0x6FC, 0x30);
            }
            HuAudFXVolSet(playerState->soundEffect, 0x60);
        } else {
            // Maps movement speed to the clamped pitch range.
            speed = (speed - 13.0f) / 53.0f;
            speed = clampVolume(speed);
            soundSetPitch(playerState->soundEffect, (s16) (-8192.0f + 8192.0f * speed));
        }
    }
    if (currentRequest() == 3 && playerState->soundEffect != -1) {
        HuAudFXFadeOut(playerState->soundEffect, 100);
        playerState->soundEffect = -1;
    }

    rotation = playerState->rotation;
    if (playerState->viewIndex == 0) {
        // Offsets the solo marker and character to opposite sides of the body height.
        position.y -= (0.6f * 100.0f);
        playerState->positionModel.get()->setPosition(&position);
        playerState->positionModel.get()->setRotationY(rotation);
        position.y += (1.2f * 100.0f);
    } else {
        // Offsets the team circle along X and lowers its character model.
        Vec shifted = position;
        shifted.x = (f32) ((f64) shifted.x - 50.0);
        playerState->circle->fn_1_D534(&shifted);
        position.y -= 100.0f;
    }
    new PlayerTrailParticles(&position, playerState->viewIndex);
    playerState->character.get()->setPosition(&position);
    playerState->character.get()->setRotationY(rotation);
}

// Applies input to the moving body while the player countdown is inactive.
void fn_1_9A5C(PlayerState *playerState, f32 horizontal, f32 vertical)
{
    PlayerBody *body = playerState->body.get();
    if (body != 0 && playerState->stateCountdown <= 0) {
        fn_1_D000(body, horizontal, vertical);
    }
}

void fn_1_9CE8(PlayerState *playerState);

// Tests this player's position against effects and handles the resulting score or hit.
void fn_1_9AC8(PlayerState *playerState)
{
    bool solo;
    int result;

    int countdownFrames;
    PlayerBody *body;
    Vec *bodyPosition;

    if (playerState->eliminated != 0) {
        return;
    }
    body = playerState->body.get();
    if (body == 0) {
        return;
    }
    solo = playerState->viewIndex == 0;

    bodyPosition = &body->position;
    Vec position = *bodyPosition;
    Vec itemPosition;
    // The solo player has twice the item contact radius of a team member.
    result = fn_1_7210(playerState->viewIndex, &position, solo ? 200.0f : 100.0f, &itemPosition);
    switch (result) {
    case 0:
        break;
    case 1:
        fn_1_8A10(playerState->viewIndex, 1);
        HuAudFXPlayPan(MSM_SE_CMN_COIN_GET, playerState->viewIndex != 0 ? 0x50 : 0x28);
        break;
    case 2:
        fn_1_8A10(playerState->viewIndex, 5);
        HuAudFXPlayPan(MSM_SE_CMN_COIN_GET_LARGE, playerState->viewIndex != 0 ? 0x50 : 0x28);
        break;
    case 3:
        playerState->player.rumble(2, (int) secondsToFrames(1.0f));
        if (playerState->viewIndex != 0) {
            HuAudFXPlayPan(0x6FA, 0x50);
            HuAudFXPlayPan(0x6FE, 0x50);
            fn_1_9CE8(playerState);
            break;
        }
        HuAudFXPlayPan(0x6F9, 0x30);
        HuAudFXPlayPan(0x6FD, 0x30);
        countdownFrames = (int) secondsToFrames(1.0f);
        playerState->stateCountdown = countdownFrames;
        playerState->shakeAmplitude.set(40.0f, 0.0f, (int) secondsToFrames(1.0f));
        break;
    }
}

// Launches the eliminated player upward, clears contact and starts the team-view blend.
// Fades the player's sound effect if it is active.
void fn_1_9CE8(PlayerState *playerState)
{
    playerState->eliminated = 1;
    fn_1_D4FC(playerState->body.get());
    playerState->bodySphere.clearFlag7();
    playerState->upperSphere.clearFlag7();
    fn_1_C0C8();
    if (playerState->soundEffect != -1) {
        HuAudFXFadeOut(playerState->soundEffect, 100);
        playerState->soundEffect = -1;
    }
}

// Resolves contacts among the three players in the opposing group.
void fn_1_9D60(void)
{
    for (int pass = 0; pass < 8; ++pass) {
        for (int firstIndex = 0; firstIndex < 2; ++firstIndex) {
            PlayerState *first = lbl_1_bss_2C[firstIndex];
            for (int secondIndex = firstIndex + 1; secondIndex < 3; ++secondIndex) {
                PlayerState *second = lbl_1_bss_2C[secondIndex];
                if (first->eliminated == 0 && second->eliminated == 0) {
                    fn_1_D370(first->body.get(), second->body.get());
                }
            }
        }
    }
}
