#ifndef M653_NATIVE_RESOURCE_H
#define M653_NATIVE_RESOURCE_H
#include <vector>
#include "dolphin/mtx.h"
#include "REL/framework/model.h"
#include "REL/framework/event_callback_inline.h"
#include "REL/framework/singleton.h"
extern "C" {
#include "game/sprite.h"
}
#include "REL/m653DLL/resource_handles.h"
struct ResourceChild {
    std::auto_ptr<ResourceVirtual> handler;
    unsigned char payload[0x18];
    std::auto_ptr<OwnedModel> resourceHandle;
    std::auto_ptr<void> allocation;
};
struct Car;
class ExhaustSmoke;
struct ResourceOwner : MinigameSingleton<ResourceOwner>, EventListener {
    std::auto_ptr<ResourceChild> child;
    std::vector<ANIMDATA *> animations;
    std::vector<OwnedModel *> cleanup;
    ResourceOwner();
    virtual ~ResourceOwner();
    virtual void receive(const EventPacket *);
    Car *fn_1_7118(s32 resourceIndex);
    void fn_1_7238(s32 resourceIndex);
    ExhaustSmoke *fn_1_7370();
    void fn_1_73DC();
    s32 fn_1_73E0();
    void fn_1_73E8();
};

ANIMDATA *fn_1_6920(unsigned long resource);
void fn_1_736C();
extern "C" void fn_1_74AC(std::vector<OwnedModel *> *models);

#endif
