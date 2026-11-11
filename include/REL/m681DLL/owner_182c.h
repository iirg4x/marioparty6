#ifndef REL_M681DLL_OWNER_182C_H
#define REL_M681DLL_OWNER_182C_H

#include "REL/framework/singleton.h"
#include "REL/framework/game_object.h"
#include "REL/m681DLL/event_callback.h"
#include "REL/m681DLL/monitor_interface.h"

struct M681GameState : MinigameSingleton<M681GameState>, GameObject, EventListener {
    typedef void (M681GameState::*Callback)();
    f32 interval;
    u32 frameCount;
    InitializationValues textureChoices;
    s32 minimum;
    s32 current;
    s32 negativeState;
    s32 state38;
    s32 state3C;
    f32 state40;
    f32 state44;
    f32 state48;
    s32 state4C;
    Callback callback;
    bool poseActive() const { return negativeState >= 0; }
    M681GameState();
    virtual ~M681GameState();
    virtual void update();
    virtual void receive(const EventPacket *);
    void fn_1_1B04();
    void fn_1_1B08();
    void fn_1_1B0C();
    void fn_1_1B98();
    void fn_1_1CA8();
    void fn_1_1CAC();
    void fn_1_1CB0();
    void fn_1_1CB4();
    void fn_1_1FB0();
    void fn_1_2138();
};

#endif
