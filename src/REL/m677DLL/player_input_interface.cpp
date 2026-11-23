// Reads the human controller or chooses the computer player's move.

#include "REL/m677DLL/player_input_interface.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/random.h"
#include "REL/m677DLL/player_work.h"

// Destroys the action through the base interface.
inline PlayerInputInterface::~PlayerInputInterface()
{
}

// Releases the input-driven action.
HumanPlayerInput::~HumanPlayerInput()
{
}

// Reads directional input and confirms the rope selection on a button press.
void HumanPlayerInput::updateInput()
{
    unsigned int buttons = owner->buttonsRepeat() | owner->dpadRepeat();
    int direction = ((buttons & 1) ? -1 : 0) + ((buttons & 2) ? 1 : 0);
    if (direction) {
        fn_1_1D0B4(owner, direction);
    }
    if (buttons & 0x100) {
        fn_1_1D144(owner);
    }
}

// Associates the automatic selection action with the character.
ComputerPlayerInput::ComputerPlayerInput(PlayerWork *player)
    : PlayerInputInterface(player), difficulty(player->difficulty())
{
}

// Releases the automatic selection action.
ComputerPlayerInput::~ComputerPlayerInput()
{
}

// Leaves the automatic selection unchanged when the action starts.
void ComputerPlayerInput::beginInput()
{
}

// Tests for movement with a 1/60 chance per frame. If the CPU does not move, it tests
// for confirmation with a difficulty-dependent probability.
void ComputerPlayerInput::updateInput()
{
    if (RandomFloat() < 1.0 / (double) secondsToFrames<long>(1)) {
        fn_1_1D0B4(owner, ((s32) RandomUnsigned() & 1) ? -1 : 1);
    } else {
        if (RandomFloat() < 1.0 / (double) secondsToFrames<long>(6 - difficulty)) {
            fn_1_1D144(owner);
        }
    }
}

// Provides the default action start behavior.
void PlayerInputInterface::beginInput()
{
}
