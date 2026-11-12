#ifndef M653_NATIVE_RESOURCE_H
#define M653_NATIVE_RESOURCE_H
#include <vector>
#include "dolphin/mtx.h"
#include "REL/framework/model.h"
#include "REL/framework/event_callback_inline.h"
#include "REL/framework/singleton.h"
typedef signed short s16;
extern "C" {
#include "game/sprite.h"
}
#include "REL/m653DLL/resource_handles.h"
struct ResourceChild : ResourcePointer<ResourceVirtual> {
    unsigned char payload[0x18];
    ResourcePointer<OwnedReleasedModel> resourceHandle;
    ResourceAllocation allocation;
};
struct Car;
class ExhaustSmoke;
struct ResourceOwner : MinigameSingleton<ResourceOwner>, EventListener {
    ResourcePointer<ResourceChild> child;
    std::vector<ANIMDATA *> animations;
    std::vector<OwnedReleasedModel *> cleanup;
    ResourceOwner();
    virtual ~ResourceOwner();
    virtual void receive(const EventPacket *);
    Car *fn_1_7118(s32 resourceIndex);
    void fn_1_7238(s32 resourceIndex);
    ExhaustSmoke *fn_1_7370();
};

#endif
