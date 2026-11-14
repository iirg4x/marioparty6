#include "math.h"
#include "REL/framework/minigame_core.h"
#include "REL/m681DLL/game_controller.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/event.h"
#include "REL/framework/random.h"
#include "REL/m681DLL/game_constants.h"

// Initializes four-player round state and registers for game events.
M681GameState::M681GameState() : GameObject(2)
{
    Callback initialCallback;
    playersRemaining = 4;
    current = 4;
    winnerPlayer = -1;
    answeredPlayers = 0;
    unknown3C = 0;
    additionalLandingDelay = 0.0f;
    unknown44 = 0.0f;
    unknown48 = 0.0f;
    roundCount = 0;
    registerChannel(0);
    initialCallback = &M681GameState::fn_1_1B04;
    callback = initialCallback;
    textureChoices.values[0] = 0;
    textureChoices.values[1] = 0;
    textureChoices.values[2] = 0;
    textureChoices.values[3] = 0;
    interval = 2.4f;
    frameCount = 0;
}

// Releases the game-state bases.
M681GameState::~M681GameState() {}

// Runs the round callback and broadcasts changes in the remaining player count.
void M681GameState::update()
{
    (this->*callback)();
    if (current > playersRemaining) {
        current = playersRemaining;
        sendEvent(1, 0, 0.0f);
    }
    fn_1_2138();
}

// Performs no round update.
void M681GameState::fn_1_1B04() {}
// Performs no round update.
void M681GameState::fn_1_1B08() {}

// Sends the input event after the configured frame interval.
void M681GameState::fn_1_1B0C()
{
    frameCount++;
    if ((f32)frameCount >= secondsToFrames(interval)) {
        sendEvent(0, 7, 0.0f);
        frameCount = 0;
    }
}

// Returns the larger value.
static inline f32 maximumValue(f32 a, f32 b)
{
    return a > b ? a : b;
}

// Sends the answer event and reduces the next interval, down to 0.2.
void M681GameState::fn_1_1B98()
{
    frameCount++;
    if ((f32)frameCount > secondsToFrames(interval)) {
        interval = maximumValue(0.2f, interval - 0.1f);
        frameCount = 0;
        sendEvent(0, 8, 0.0f);
    }
}

// Performs no round update.
void M681GameState::fn_1_1CA8() {}

// Performs no round update.
void M681GameState::fn_1_1CAC() {}
// Performs no round update.
void M681GameState::fn_1_1CB0() {}
// Performs no round update.
void M681GameState::fn_1_1CB4() {}

// Selects the round callback and refreshes the monitor choices for game events.
void M681GameState::receive(const EventPacket *selection)
{
    switch (selection->type) {
    case 1: {
        fn_1_1FB0();
        fn_1_381C(&MinigameSingleton<Monitor>::getChecked(), &textureChoices, 2.4f);
        M681GameState::Callback callback =
            &M681GameState::fn_1_1B08;
        this->callback = callback;
        break;
    }
    case 5: {
        fn_1_1FB0();
        fn_1_381C(&MinigameSingleton<Monitor>::getChecked(), &textureChoices, interval);
        ++roundCount;
        M681GameState::Callback callback =
            &M681GameState::fn_1_1B0C;
        this->callback = callback;
        break;
    }
    case 7: {
        M681GameState::Callback callback =
            &M681GameState::fn_1_1B98;
        this->callback = callback;
        break;
    }
    case 8: {
        M681GameState::Callback callback =
            &M681GameState::fn_1_1CA8;
        this->callback = callback;
        break;
    }
    case 9: {
        M681GameState::Callback callback =
            &M681GameState::fn_1_1CAC;
        this->callback = callback;
        break;
    }
    case 10: {
        M681GameState::Callback callback =
            &M681GameState::fn_1_1CB0;
        this->callback = callback;
        break;
    }
    case 11: {
        M681GameState::Callback callback =
            &M681GameState::fn_1_1CB4;
        this->callback = callback;
        break;
    }
    default: {
        M681GameState::Callback callback =
            &M681GameState::fn_1_1B04;
        this->callback = callback;
        break;
    }
    }
}

extern "C" {
const s32 lbl_1_rodata_150[8] = { 4, 1, 3, 0, 4, 2, 3, 0 };
}

// Chooses two target values and two alternative values at random.
void M681GameState::fn_1_1FB0()
{
    textureChoices.values[0] = lbl_1_rodata_150[(s32)RandomUnsigned() % 3];
    textureChoices.values[1] = *(&lbl_1_rodata_150[(s32)RandomUnsigned() % 3] + 4);
    textureChoices.values[2] = lbl_1_rodata_150[(s32)RandomUnsigned() % 4];
    textureChoices.values[3] = *(&lbl_1_rodata_150[(s32)RandomUnsigned() % 4] + 4);

    if (textureChoices.values[1] == 3 && textureChoices.values[0] == 3) {
        if ((s32)RandomUnsigned() % 2 != 0) {
            textureChoices.values[0] = lbl_1_rodata_150[(s32)RandomUnsigned() % 2];
            return;
        }
        textureChoices.values[1] = *(&lbl_1_rodata_150[(s32)RandomUnsigned() % 2] + 4);
    }
}

// Performs no operation.
void M681GameState::fn_1_2138()
{
}
