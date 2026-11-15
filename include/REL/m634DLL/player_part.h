#ifndef REL_M634DLL_PLAYER_PART_H
#define REL_M634DLL_PLAYER_PART_H
#include "REL/m634DLL/stick_state.h"
struct ActPlayer;
// The common input object: constructors store its owner at 0x1C.
struct ActPlayerPart {
    explicit ActPlayerPart(ActPlayer *player) : owner(player) {}
    virtual ~ActPlayerPart();
    virtual void readInput(StickState *output) = 0;
    virtual void reset();
    StickState input;

    u8 unknown18[4]; /* No code names this field. */

    ActPlayer *owner;
    void updateInput();
};
#endif
