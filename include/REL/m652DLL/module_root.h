#ifndef REL_M652DLL_MODULE_ROOT_H
#define REL_M652DLL_MODULE_ROOT_H

#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"
#include "REL/framework/event_packet.h"

template <class Resource> class OwnedResource {
public:
    Resource *value;
    /* Leaves the owned pointer uninitialized. */
    OwnedResource() {}
    /* Takes ownership of the initial pointer. */
    OwnedResource(Resource *initial) : value(initial) {}
    /* Deletes the owned resource. */
    ~OwnedResource() { delete value; }
    /* Returns the owned pointer. */
    operator Resource *() const { return value; }
    /* Returns the owned resource for member access. */
    Resource *operator->() const { return value; }
    /* Deletes the previous resource when the replacement differs. */
    void reset(Resource *replacement)
    {
        if (value != replacement) {
            delete value;
            value = replacement;
        }
    }
};

class GameResults;

class ModuleRoot : public MinigameCore, public ThreadEntry,
                            public EventListener {
public:
    OwnedResource<GameResults> result;
    ModuleRoot();
    virtual ~ModuleRoot();
    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    virtual void receive(const EventPacket *event);
    virtual void run(void *);
};

#endif
