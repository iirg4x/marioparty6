#ifndef REL_M628DLL_ACT_PLAYER_H
#define REL_M628DLL_ACT_PLAYER_H
#include "REL/framework/game_object.h"
#include "REL/framework/event_hm.h"
#include "REL/m628DLL/range_record.h"
#include "REL/m628DLL/player_input_controller.h"
extern "C"
{
#include "game/mg/actman.h"
}
#include <vector>
#include <memory>
class ActPlayer;
typedef void (ActPlayer::*StateCallback)();
// Updates a player character, collision record and round state.
class ActPlayer : public GameObject
{
  public:
    ActorRangeRecord record; // Player collision shape and position.
    Vector3 position;
    Vec velocity;
    f32 motionTime;
    s32 unk54; /* The game's code never reads this field. */
    s32 playerNo;
    std::vector<GXColor> colors; // Character material colors restored after a hit flash.
    MGPLAYER *player;
    std::auto_ptr<PlayerInputController> inputController; // Owns the character input controller.
    bool playerFlag;                            // Records whether the player uses mode 0.
    s32 mode; // Selects the player group and corner exclusion.
    u8 locked;         // Set when the player has locked in.
    u8 finishComplete; // Set after the result animation completes.
    f32 unk7C[3];      /* The game's code never reads this field. */
    s32 state;
    f32 elapsed;
    s32 frame;
    s16 modelId;
    s16 characterModelId;
    f32 rotation;
    u8 spawned;
    f32 unkA0;    /* The game's code never reads this field. */
    s8 unkA4;     /* The game's code never reads this field. */
    s32 unkA8;    /* The game's code never reads this field. */
    u8 hit; // Set when a collision triggers the hit response.
    EventHandlerManager channelOneCallbacks;  // Receives events on channel 1.
    EventHandlerManager channelZeroCallbacks; // Receives events on channel 0.
    StateCallback callback;                   // Runs the current object state.
    // Selects the current update callback.
    void setCallback(StateCallback value)
    {
        callback = value;
    }
    u32 motionId; // Currently selected motion ID.
    enum
    {
        NUMBEROF_MOTION = 9
    };
    std::vector<s16> m_motion_id; // Motion IDs loaded for this player character.

    // Returns whether the player has locked in.
    const u8 isLocked() const
    {
        return locked;
    }
    // Returns the player position.
    Vec positionValue() const
    {
        return position;
    }
    // Returns the player mode.
    s32 modeValue() const
    {
        return mode;
    }
    // Returns the player character data.
    void *getPlayerData()
    {
        return player;
    }
    // Returns typed player character data.
    MGPLAYER *getPlayer()
    {
        return (MGPLAYER *)getPlayerData();
    }
    ActPlayer(s32 playerNo, s32 mode);
    virtual ~ActPlayer();
    virtual void update();
    void fn_1_F40(u32 dataNum);
    void fn_1_FB4();
    void fn_1_124C(u32 motionId, u8 refresh, u8 mode, f32 startTime);
    void fn_1_1308();
    void fn_1_13B8();
    void fn_1_14A0(f32 red, f32 green, f32 blue);
    void fn_1_1628();
    void fn_1_16D0();
    u8 fn_1_1708();
    void fn_1_1970();
    void fn_1_1C8C();
    void fn_1_1C90();
    void fn_1_1E34();
    void fn_1_1E40();
    void fn_1_2298();
    void fn_1_2400();
    void fn_1_26C4();
    void fn_1_27FC();
    void fn_1_2AD8();
    void fn_1_2D40();
    void fn_1_2E28(u8 active);
    void fn_1_2F4C(const EventPacket &event);
    void fn_1_3030(const EventPacket &event);
    void fn_1_30D0(const EventPacket &event);
    void fn_1_3254(const EventPacket &event);
    void fn_1_338C(const EventPacket &event);
    void fn_1_15C0();
    void fn_1_15F4();
};
#endif
