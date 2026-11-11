// The players' charge: each value rises with the events the players send. The asserts keep the
// original file name and lines (charger.cpp, lines 36, 42, 50 and 60); the text of the lines between
// them is unknown. NDEBUG is undefined because the build defines it for every object while the game
// kept these asserts.
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
#include <cmath>
#include "REL/m639DLL/event_manager.h"
#include "REL/m639DLL/mode_game.h"

extern "C" {
unsigned int lbl_1_bss_0 = minutesToFrames(5L);
unsigned int lbl_1_bss_4 = secondsToFrames(30L);
float lbl_1_bss_8 = (1.0 / 60.0) * ((1.0 / 60.0) * cmToWorld(980.665f));
}

/* Initializes the shared listener and its four initial motion values. */
ModeGame::ModeGame() : GameObject(3)
{
    for (int index = 0; index < 4; ++index) {
        registerChannel(index + 1);
        values[index] = 0.0f;
    }
}

ModeGame::~ModeGame() {}

// A player's charge as a fraction of a full charge (800), at most 1.



float ModeGame::fn_1_1CC(int id)
{
    assert((id >= 0) && (id < 4));
    float ratio = values[id] / 800.0f;
    return (1.0f < ratio) ? 1.0f : ratio;
}
float ModeGame::fn_1_2B0(int team)
{
    assert((team >= 0) && (team < 2));
    float *first = &values[team * 2];
    float *second = &values[team * 2 + 1];
    float average = (*first + *second) / 800.0f;
    return (1.0f < average) ? 1.0f : average;
}
bool ModeGame::fn_1_3B0(int team)
{
    assert((team >= 0) && (team < 2));
    float *second;
    ((team >= 0 && team < 2)) ? (void)0 : __msl_assertion_failed("(team >= 0) && (team < 2)", "charger.cpp", 0x2A);

    float *first = &values[team * 2];
    second = &values[team * 2 + 1];
    float average = (*first + *second) / 800.0f;
    float selectedRatio;
    if (1.0f < average) {
        selectedRatio = 1.0f;
    } else {
        selectedRatio = average;
    }
    float ratio = selectedRatio;
    float result = ratio;
    float adjustedResult = result;
    float comparisonValue = adjustedResult;
    return comparisonValue >= 1.0f;
}

// Applies the selected value when the state is active and its index is valid.
void ModeGame::receive(const EventPacket *packet)
{
    if (packet->type != 1 || packet->context == 0) {
        return;
    }
    int ch = packet->channel - 1;
    ((ch >= 0) && (ch < 4)) ? (void)0 :
        __msl_assertion_failed("(ch >= 0) && (ch < 4)", "charger.cpp", 0x3C);
    values[ch] += *(float *)packet->context;
}

void ModeGame::update() {}
