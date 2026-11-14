#ifndef REL_M681DLL_PLAYER_OBJECT_H
#define REL_M681DLL_PLAYER_OBJECT_H

#include <memory>
#include "dolphin/types.h"
#include "REL/m681DLL/texture_node.h"
extern "C" {
#include "game/charman.h"
}
#include "REL/framework/chara_motion.h"
#include "REL/framework/frame_interpolator.h"
#include "REL/framework/model.h"
#include "REL/m681DLL/model_resource.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_packet.h"
#include "REL/framework/p_chara.h"

struct ModePair { s32 values[2]; };
struct ModeBase;

struct PlayerObject : GameObject, PlayerConfigSelection, EventListener {
    PlayerObject(s32 player, const Vec *initialState);
    virtual ~PlayerObject();
    virtual void update();
    virtual void receive(const EventPacket *event);
    typedef void (PlayerObject::*Callback)();
    std::auto_ptr<ModeBase> input;
    std::auto_ptr<ModelResource> pipeModel;
    ModePair modes;
    Vec state;
    Vec rotation;
    FrameInterpolator<float> firstMotionTime;
    FrameInterpolator<float> secondMotionTime;
    f32 maximumMotionTime;
    std::auto_ptr<OwnedModel> firstItem;
    std::auto_ptr<OwnedModel> secondItem;
    u8 unknown8C[4]; /* No code names this field. */
    u32 elapsed;
    u32 fallFrames;
    f32 inputFrames;
    f32 inputDelay;
    // Seconds an eliminated player waits below the stage; each later elimination waits longer.
    f32 landingDelay;
    // Frames spent below the stage, counted against landingDelay.
    f32 landingFrames;
    f32 animationTime;
    f32 unkAC; /* The game's code never reads this field. */
    f32 unkB0; /* The game's code never reads this field. */
    u8 unknownB4[4]; /* No code names this field. */
    s32 unkB8; /* The game's code never reads this field. */
    s32 count;
    u8 active;
    // Set when the player's two choices differ from the answer, which eliminates the player.
    u8 choseWrong;
    char *firstItemHook;
    char *secondItemHook;
    Callback callback;
    void fn_1_5E90();
    void fn_1_5EF4();
    void fn_1_61B8();
    void fn_1_643C();
    void fn_1_6450();
    void fn_1_66F0();
    void fn_1_6824();
    void fn_1_6A50();
    void fn_1_7410();
    void fn_1_74E8();
    void fn_1_7608();
    void fn_1_77E4();
    void fn_1_5E8C();
    void showChoices();
};

void setCharacterMotionOverlay(MotionOwner *character, s16 modelId, unsigned int dataNum);
#endif
