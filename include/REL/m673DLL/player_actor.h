// Declares the player actor and its state callbacks.

#ifndef REL_M673DLL_PLAYER_ACTOR_H
#define REL_M673DLL_PLAYER_ACTOR_H
#include "dolphin/types.h"
#include "REL/framework/game_object2.h"
#include "REL/framework/event_hm.h"
#include "REL/m673DLL/collision_body.h"
#include "REL/framework/octtree_man.h"
#include <vector>
#include <memory>
#include "REL/m673DLL/controllers.h"
#include "REL/m673DLL/follow_effect.h"

extern "C" {
#include "game/mg/actman.h"
}

// Creates a computer or pad controller for the selected player.
PlayerController *fn_1_EE2C(PlayerActor *player, s32 playerIndex);

// Discards the player-number check during play.
static inline void DebugPlayerNumber(const s32 &value)
{
}

// Discards the height check during play.
static inline void DebugHeight(f32 value)
{
}

// Owns a player model, its controller, and its state callbacks.
class PlayerActor : public GameObject {
public:
    typedef void (PlayerActor::*State)();

    s16 modelId; // Engine ID of the player model.
    s16 character; // Character selected for this player.
    s32 playerIndex; // Slot in the minigame player list.
    u8 ready; // Signals completion of the current phase.
    MGPLAYER *player; // Engine player configuration.
    // Returns the player configuration used by the controller.
    MGPLAYER *getPlayer()
    {
        return player;
    }
    std::auto_ptr<PlayerController> controller; // Input controller for this player.
    CollisionBody component; // Collision bounds and position.
    f32 rotationY; // Player rotation about the vertical axis.
    u8 active; // Player remains in play.
    // Returns whether the player is still in play.
    u8 isActive()
    {
        return active;
    }
    std::auto_ptr<FollowEffect> followEffect; // Quad drawn at the player effect position.
    Position3 effectPosition; // Position of the attached quad.
    u16 jumpFrames; // Frames in the current jump phase.
    s32 collisionFrames; // Count incremented by the collision hook.
    f32 positionParameters[5]; // Values copied from the player's configuration row.
    EventHandlerManager channel0Events; // Receives the player actor's channel 0 events.
    EventHandlerManager channel1Events; // Receives the player actor's channel 1 events.
    EventHandlerManager channel2Events; // Receives the player actor's channel 2 events.
    State state; // Current frame update.
    // Sets the next state callback.
    void setState(State method)
    {
        state = method;
    }

    s32 currentMotion; // Index of the current motion in the loaded motion list.
    std::vector<short> motionRecords; // Loaded engine motion IDs.

    // Copies the player's collision position.
    void getPosition(Vec &result)
    {
        result.x = component.position.value.x;
        result.y = component.position.value.y;
        result.z = component.position.value.z;
    }

    // Returns the player index.
    s32 getPlayerNumber()
    {
        DebugPlayerNumber(playerIndex);
        return playerIndex;
    }

    // Returns the collision count.
    s32 getCollisionFrames()
    {
        return collisionFrames;
    }

    // Sets the collision count.
    void setCollisionFrames(s32 value)
    {
        collisionFrames = value;
    }

    // Creates the player model and controller and registers the player events.
    PlayerActor(s32 playerIndex, const Position3 *position);
    // Releases the underlying player actor.
    virtual ~PlayerActor();
    // Updates the controller and attached effect position before running the state callback.
    virtual void update();

    // Updates the player position and chooses the next action state.
    void fn_1_FDC0();
    // Loads the player's motion resources.
    void fn_1_106A0();
    // Increments the player's collision count.
    void fn_1_10DC0();
    // Shifts the player's character motion when the requested motion changes.
    void fn_1_107B8(u32 motionId, u8 loop);
    // Appends a loaded character motion to the player's motion list.
    void fn_1_10744(u32 dataNum);
    // Raises the player while updating its attached effect.
    void fn_1_FFE0();
    // Lowers the player, then hides its effect and enters the idle state.
    void fn_1_10164();
    // Returns the player's rotation toward zero and resumes the idle state.
    void fn_1_104A0();
    // Idle callback installed when the ending motion is selected.
    void fn_1_1069C();
    // Leaves the player unchanged in this state.
    void fn_1_FD68();
    // Returns to the looping motion after the current motion finishes.
    void fn_1_FD6C();
    // Resets the player actor and selects the motion-completion state.
    void fn_1_1087C(const EventPacket &);
    // Starts the player action at the current collision position.
    void fn_1_108E4(const EventPacket &);
    // Disables collision and starts the player's return rotation if the player remains in play.
    void fn_1_1095C(const EventPacket &);
    // Records an active player as a winner and selects the ending motion.
    void fn_1_10A70(const EventPacket &);
    // Shows the player's attached effect.
    void fn_1_10BE0(const EventPacket &);
    // Hides the player's attached effect.
    void fn_1_10C14(const EventPacket &);
    // Calls fn_8006A030 for the player and requests motion 2.
    void fn_1_10C48(const EventPacket &);
};

struct ColNarrowParam_s;

// Handles player collisions and increments the selected player's collision count.
int fn_1_10D0C(ColNarrowParam_s *otherContact, ColNarrowParam_s *contact);

#endif
