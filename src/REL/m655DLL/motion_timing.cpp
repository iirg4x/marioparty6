#include "REL/framework/random.h"
#include "REL/framework/frame_time.h"
#include "REL/m655DLL/player_motion_interface.h"
#include "REL/m655DLL/player_state.h"
#include "dolphin/types.h"

inline PlayerInputInterface::PlayerInputInterface(PlayerWork *object)
{
    owner = object;
}

inline PlayerInputInterface::~PlayerInputInterface() {}

HumanPlayerInput::~HumanPlayerInput() {}

void HumanPlayerInput::invoke()
{
    unsigned int flags = owner->buttonsRepeat() | owner->dpadRepeat();
    int updateMode = ((flags & 1) ? -1 : 0) + ((flags & 2) ? 1 : 0);
    if (updateMode != 0) {
        fn_1_8B68(owner, updateMode);
    }
    if (flags & 0x100) {
        fn_1_8CA4(owner);
    }
}

ComputerPlayerInput::ComputerPlayerInput(PlayerWork *object)
    : PlayerInputInterface(object)
{
    selection = object->difficulty();
}

ComputerPlayerInput::~ComputerPlayerInput() {}

void ComputerPlayerInput::setPosition() {}

void ComputerPlayerInput::invoke(void)
{
    if (RandomFloat() < 1.0 / (double)secondsToFrames<long>(1)) {
        fn_1_8B68(owner,
                  ((s32)RandomUnsigned() & 1) ? -1 : 1);
    } else {
        if (RandomFloat() <
            1.0 / (double)secondsToFrames<long>(6 - selection)) {
            fn_1_8CA4(owner);
        }
    }
}

void PlayerInputInterface::setPosition() {}
