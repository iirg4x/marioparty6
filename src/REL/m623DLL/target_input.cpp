// Target selection and stick input for computer players.

extern "C" {
#include "dolphin/mtx.h"
}

#include <new>
#include "REL/framework/random.h"
#include "REL/m623DLL/item_effects_owner.h"
#include "REL/m623DLL/motion_controller.h"
#include "REL/m623DLL/player_body.h"
#include "REL/m623DLL/player_state.h"

// Initializes computer-player inputs and random retargeting delays.
MotionController::MotionController()
{
    this->framesRemaining = RandomUnsigned() & 0x7F;
    this->randomVerticalInput = (float) (RandomFloat() - 0.2);
    this->randomHorizontalInput = (float) (RandomFloat() - 0.5);
    this->horizontalInput = this->unk04 = this->verticalInput = this->smoothedHorizontalInput =
        this->unk10 = this->smoothedVerticalInput = 0.0f;
}

// Completes destruction of the motion controller.
MotionController::~MotionController()
{
}

// Looks up a collectible and a spiny in the selected view.
void fn_1_A15C(MotionController *motion, int viewIndex, Vec *position)
{
    motion->horizontalInput = motion->verticalInput = 0.0f;
    motion->hasTarget = fn_1_76E0(viewIndex, position, &motion->targetPosition);
    motion->hasSpinyTarget = fn_1_7534(viewIndex, position, &motion->spinyPosition);
}

// The horizontal input magnitudes by difficulty.
const float lbl_1_rodata_464[4] = { 0.25f, 0.5f, 0.75f, 1.0f };

// Returns the absolute value of the supplied input.
static inline float motionAbs(float value)
{
    if (value < 0.0f) {
        return -value;
    }
    return value;
}

// Steers the computer player away from a nearby spiny.
void fn_1_A1D8(MotionController *motion, Vec *position, int difficulty)
{
    float deltaX = motion->spinyPosition.x - position->x;
    float deltaZ = motion->spinyPosition.z - position->z;

    // Reacts to a spiny within the forward and rear X limits.
    if (motion->hasSpinyTarget != 0 && deltaX < 10000.0f && deltaX > -200.0f) {
        // The lateral dodge input tapers to zero at the edge of the corridor.
        if (motionAbs(deltaZ) < 300.0f) {
            motion->horizontalInput =
                ((300.0f - motionAbs(deltaZ)) / 300.0f) *
                (deltaZ > 0.0f ? -lbl_1_rodata_464[difficulty] : lbl_1_rodata_464[difficulty]);
            switch (difficulty) {
            case 0:
            case 1:
                motion->verticalInput = 0.0f;
                break;
            case 2:
            case 3:
                if (deltaX > 0.0f) {
                    // On higher difficulties, backs away from a close spiny ahead.
                    motion->verticalInput =
                        deltaX < (float) ((difficulty + 2) * 150) ? -1.0f : 0.0f;
                }
                break;
            }
        }
    }
}

// Steers the computer player toward a coin or dollar.
void fn_1_A3EC(MotionController *motion, Vec *position, int difficulty)
{
    float deltaZ = motion->targetPosition.z - position->z;
    float deltaX = motion->targetPosition.x - position->x;
    if (motion->hasTarget != 0) {
        if (motionAbs(deltaZ) < 200.0f) {
            // A nearly aligned player can move forward to collect the item.
            if (motionAbs(deltaZ) > 50.0f) {
                motion->horizontalInput = deltaZ / 200.0f;
            } else if (difficulty > 1) {
                motion->verticalInput = 1.0f;
                return;
            }
        } else {
            if (deltaZ > 200.0f) {
                motion->horizontalInput = 1.0f;
            }
            if (deltaZ < -200.0f) {
                motion->horizontalInput = -1.0f;
            }
        }
        // Difficulty governs whether to advance toward or back away from the item.
        switch (difficulty) {
        case 0:
            motion->verticalInput = 0.0f;
            break;
        case 1:
        case 2:
            motion->verticalInput = deltaX > 1000.0f ? 1.0f : 0.0f;
            break;
        case 3:
            if (deltaX > 1000.0f) {
                motion->verticalInput = 1.0f;
                break;
            }
            if (deltaX > 0.0f && deltaX < 500.0f) {
                motion->verticalInput = -1.0f;
            }
            break;
        }
    }
}

// Horizontal input scales by difficulty.
const float lbl_1_rodata_4A4[4] = { 0.3f, 0.5f, 0.7f, 0.8f };

// Computes, smooths and applies the computer player's stick input.
void fn_1_A618(MotionController *motion, PlayerState *playerState)
{
    PlayerBody *body;
    Vec *sourcePosition;
    int difficulty;
    int viewIndex;
    body = playerState->body.get();
    if (body != 0) {
        difficulty = playerState->player.difficulty();
        viewIndex = playerState->player.groupNo();
        sourcePosition = &body->position;
        Vec position = *sourcePosition;
        if (viewIndex > 1) {
            viewIndex = 1;
        }
        fn_1_A15C(motion, viewIndex, &position);
        motion->horizontalInput = motion->randomHorizontalInput;
        if (--motion->framesRemaining > 0) {
            fn_1_A3EC(motion, &position, difficulty);
        } else {
            motion->verticalInput = motion->randomVerticalInput;
            if (((s32) RandomUnsigned() & 0xFF) < ((difficulty + 1) * 10)) {
                motion->framesRemaining = (RandomUnsigned() & 0x7F) + 30;
                fn_1_A3EC(motion, &position, difficulty);
                motion->randomVerticalInput = RandomFloat() - 0.2f;
                motion->randomHorizontalInput = RandomFloat() - 0.5f;
            }
        }
        fn_1_A1D8(motion, &position, difficulty);
        motion->horizontalInput = motion->horizontalInput * (lbl_1_rodata_4A4[difficulty]);
        // Moves each axis a fifth of the way toward the new input.
        motion->smoothedHorizontalInput =
            (float) ((double) motion->smoothedHorizontalInput +
                     (0.2 * (double) (motion->horizontalInput - motion->smoothedHorizontalInput)));
        motion->smoothedVerticalInput =
            (float) ((double) motion->smoothedVerticalInput +
                     (0.2 * (double) (motion->verticalInput - motion->smoothedVerticalInput)));
        fn_1_9A5C(playerState, motion->smoothedHorizontalInput, motion->smoothedVerticalInput);
    }
}

// Creates the computer motion controller when needed, then applies its input.
void fn_1_AE70(PlayerState *playerState)
{
    if (playerState->motionController.get() == 0) {
        playerState->motionController = new MotionController;
    }
    fn_1_A618(playerState->motionController.get(), playerState);
}
