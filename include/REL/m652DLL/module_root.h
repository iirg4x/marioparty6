#ifndef REL_M652DLL_MODULE_ROOT_H
#define REL_M652DLL_MODULE_ROOT_H

#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"
#include "REL/framework/event_packet.h"

template <class Resource> class OwnedResource {
public:
    Resource *value;
    OwnedResource() {}
    OwnedResource(Resource *initial) : value(initial) {}
    ~OwnedResource() { delete value; }
    operator Resource *() const { return value; }
    Resource *operator->() const { return value; }
    void reset(Resource *replacement)
    {
        if (value != replacement) {
            delete value;
            value = replacement;
        }
    }
};

class ResultView;

class ModuleRoot : public MinigameCore, public ThreadEntry,
                            public EventListener {
public:
    OwnedResource<ResultView> result;
    ModuleRoot();
    virtual ~ModuleRoot();
    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    virtual void receive(const EventPacket *event);
    virtual void run(void *);
};

#endif
