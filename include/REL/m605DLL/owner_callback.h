#ifndef REL_M605DLL_OWNER_CALLBACK_H
#define REL_M605DLL_OWNER_CALLBACK_H

#include "dolphin/types.h"

struct PlayerCharacter;

// Defines the indices in the player state name table.
enum {
    STATE_IDLE,
    STATE_KYORO,
    STATE_BIKKURI,
    STATE_JUMP,
    STATE_FALL,
    STATE_RUN,
    STATE_HIT,
    STATE_DIE,
    STATE_STUMBLE,
    STATE_STANDUP,
    STATE_WALK,
    STATE_VICTORY,
};
// Provides callbacks for a character state and stores its state-table index.
class OwnerCallback {
public:
    virtual ~OwnerCallback() {}
    // Provides the default callback when a state is entered.
    virtual void enter(PlayerCharacter *player) {}
    // Provides the default callback for a state update.
    virtual void advance(PlayerCharacter *player) {}
    // Provides the default callback when a state is left.
    virtual void leave(PlayerCharacter *player) {}
    // Advances this state for the current frame.
    void updatePlayer(PlayerCharacter *player) { advance(player); }
    u32 state;
    // Returns the state table index.
    s32 getState() const { return state; }
    // Stores the selected state-table index.
    explicit OwnerCallback(s32 state) : state(state) {}
};

#endif
