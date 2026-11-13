#ifndef REL_M681DLL_GAME_CONTROLLER_H
#define REL_M681DLL_GAME_CONTROLLER_H

#include "REL/framework/singleton.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_packet.h"
#include "REL/m681DLL/monitor_interface.h"

struct M681GameState : MinigameSingleton<M681GameState>, GameObject, EventListener {
    typedef void (M681GameState::*Callback)();
    f32 interval;
    u32 frameCount;
    InitializationValues textureChoices;
    s32 playersRemaining;
    s32 current;
    s32 winnerPlayer;
    s32 answeredPlayers;
    s32 unknown3C; // Initialized to zero and never read.
    f32 additionalLandingDelay;
    f32 unknown44; // Initialized to zero and never read.
    f32 unknown48; // Initialized to zero and never read.
    s32 roundCount;
    Callback callback;
    // Reports whether a player has been selected for the camera.
    bool hasWinner() const { return winnerPlayer >= 0; }
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
