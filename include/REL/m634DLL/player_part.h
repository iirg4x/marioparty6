#ifndef REL_M634DLL_PLAYER_PART_H
#define REL_M634DLL_PLAYER_PART_H
#include "REL/m634DLL/stick_state.h"
struct ActPlayer;
struct ActPlayerPart {
    // Stores the input part owner.
    explicit ActPlayerPart(ActPlayer *player)
        : owner(player)
    {
    }
    virtual ~ActPlayerPart();
    virtual void readInput(StickState *output) = 0;
    virtual void reset();
    StickState input;

    u8 unk18[4]; /* No code names this field. */

    ActPlayer *owner;
    void updateInput();
};
#endif
