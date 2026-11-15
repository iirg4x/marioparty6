// The game manager: the shared sprites and stage models, and the objects it owns.
#ifndef REL_M634DLL_GAME_MANAGER_H
#define REL_M634DLL_GAME_MANAGER_H

#include <vector>
#include <memory>
#include "REL/m634DLL/motion_effect.h"
#include "REL/framework/singleton.h"
#include "REL/framework/event_packet.h"
#include "REL/framework/model.h"

struct GameManager : MinigameSingleton<GameManager>, EventListener {
    std::vector<ANIMDATA *> animations;
    std::vector<OwnedModel *> models;
    std::auto_ptr<MotionEffect> secondaryResource;
    std::auto_ptr<MotionEffect> primaryResource;

    GameManager();
    virtual ~GameManager();
    virtual void receive(const EventPacket *);
    void fn_1_C7F0();
    void fn_1_CA30(const EventPacket *event);
    void fn_1_CCA8(const EventPacket *event);
};

extern "C" {
void fn_1_C124(GameManager *);
void fn_1_C534(GameManager *);
}
void fn_1_C708(GameManager *);

#endif
