#ifndef REL_M655DLL_PLAYER_STATE_H
#define REL_M655DLL_PLAYER_STATE_H
#include "REL/framework/frame_interpolator.h"
#include "REL/framework/p_chara.h"
#include "REL/m655DLL/player_motion_interface.h"
#include "REL/framework/frame_interpolator.h"
#include "REL/framework/category_sphere.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_packet.h"
#include "REL/framework/event_manager.h"
#include "REL/framework/range.h"
#include <memory>
#include "REL/m655DLL/icon_renderer.h"
struct PlayerStartPosition { Vec value; };
struct PlayerFlags {
    u8 unknownFlag0 : 1;
    u8 atLimit : 1;
    u8 unknown : 6;
    PlayerFlags() : unknownFlag0(0), atLimit(0) {}
};

struct PlayerWork : PlayerConfigSelection, GameObject, EventListener {
    virtual ~PlayerWork();
    virtual void update();
    virtual void receive(const EventPacket *);
    typedef void (PlayerWork::*State)(void);
    State state;
    int mode;
    f32 scale;
    PlayerFlags flags;
    Sphere position;
    Vec offset;
    Range<float> firstRotation, rotation;
    FrameInterpolator<float> transition;
    s32 resourceNumber;
    std::auto_ptr<IconRenderer> resource;
    RegisteredValue firstRegisteredValue, secondRegisteredValue;
    std::auto_ptr<PlayerInputInterface> consumer;
    PlayerWork(s32 player,s32 mode,const PlayerStartPosition *position);
    IconRenderer *currentResource() { return resource.get(); }
    PlayerInputInterface *currentMotion() { return consumer.get(); }
    void fn_1_8D7C();
    void fn_1_9210();
    void fn_1_9344();
    void fn_1_9428();
    void fn_1_8C44();
    void fn_1_8B64(void);
    void fn_1_9554(void);
    void fn_1_9B40(void);
    void fn_1_9A80(void);
    void fn_1_988C(void);
};
#include "REL/m655DLL/player_entry.h"
#endif
