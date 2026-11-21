#ifndef REL_M623DLL_PLAYER_STATE_H
#define REL_M623DLL_PLAYER_STATE_H
#include "dolphin/types.h"
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "REL/framework/owned_pointer.h"
#include "REL/framework/category_sphere.h"
#include "REL/m623DLL/circle_entry.h"
#include "REL/framework/frame_interpolator.h"
#include "REL/framework/p_info.h"

#include "REL/framework/p_chara.h"

#include "REL/m623DLL/player_body.h"

struct MotionController;

// One player's character, moving body and interaction state.
struct PlayerState : GameObject {
    // Controller, character and group information for this player.
    PlayerInfo player;
    // The zero-based view used by the player and the item records.
    int viewIndex;
    // Set after the player is removed from play.
    unsigned char eliminated;
    // Frames until the current player state can advance.
    int stateCountdown;
    // The moving body used to resolve terrain contact.
    OwnedPointer<PlayerBody> body;
    // The character model and its motions.
    OwnedPointer<PlayerConfigSelection> character;
    // Model positioned under the lone player.
    std::auto_ptr<OwnedModel> positionModel;
    // The main player contact sphere.
    RegisteredValue bodySphere;
    // The second contact sphere, above the body.
    RegisteredValue upperSphere;
    // Interpolates the amount of character shake.
    FrameInterpolator<f32> shakeAmplitude;
    // Tracks targets and analog input for computer steering.
    OwnedPointer<MotionController> motionController;
    // The circle registered for this player in the team view.
    CircleEntry *circle;
    // Current character facing angle.
    f32 rotation;
    // Desired character facing angle.
    f32 targetRotation;
    // Handle of the active sound effect, or -1 when idle.
    int soundEffect;
    // Fraction of the current motion used by the character model.
    f32 motionTimeFraction;

    PlayerState(s32 playerIndex, unsigned char mode, s32 slot);
    virtual ~PlayerState();
    // Advances the state countdown and reports its transition to zero.
    int advanceStateCountdown()
    {
        int expired = 0;
        if (stateCountdown > 0) {
            if (--stateCountdown == 0) {
                expired = 1;
            }
        }
        return expired;
    }

    virtual void update();
};

// Checks contact between pairs of players on the three-player team.
void fn_1_9D60(void);

// Applies horizontal and vertical input to the moving body.
void fn_1_9A5C(PlayerState *playerState, f32 horizontal, f32 vertical);

// Updates the computer player input.
void fn_1_AE70(PlayerState *playerState);

#endif
