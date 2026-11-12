// The players' charge: each value rises with the events the players send. The asserts keep the
// original file name and lines (charger.cpp, lines 36, 42, 50 and 60); the text of the lines between
// them is unknown. NDEBUG is undefined because the build defines it for every object while the game
// kept these asserts.
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
#include <cmath>
#include "REL/framework/event_manager.h"
#include "REL/m639DLL/mode_game.h"

// These constants keep translation-unit-local initialized storage.
// Game durations include the closing interval.
#include "REL/m639DLL/game_constants.h"
// Gravity uses world units per frame squared.
// Frame and world-unit conversions are defined by the framework.

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
#include "REL/m639DLL/pair_bounds.h"


float ModeGame::fn_1_1CC(int id)
{
    assert((id >= 0) && (id < 4));
    return Min(1.0f, values[id] / 800.0f);

}
float ModeGame::fn_1_2B0(int team)
{
    assert((team >= 0) && (team < 2));
    float *first = &values[team * 2];
    float *second = &values[team * 2 + 1];
    return Min(1.0f, (*first + *second) / 800.0f);

}
bool ModeGame::fn_1_3B0(int team)
{
    assert((team >= 0) && (team < 2));
    return fn_1_2B0(team) >= 1.0f;
}
// Adds the charge an event carries to the player's value.
void ModeGame::receive(const EventPacket *packet)
{
    if (packet->type != 1 || packet->context == 0) {
        return;
    }
    int ch = packet->channel - 1;
    assert((ch >= 0) && (ch < 4));
    values[ch] += *(float *)packet->context;
}

void ModeGame::update() {}
