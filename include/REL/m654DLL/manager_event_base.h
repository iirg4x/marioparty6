#ifndef M654DLL_MANAGER_EVENT_BASE_H
#define M654DLL_MANAGER_EVENT_BASE_H
// A scene manager on the list every frame walks: it hears phase events and advances each frame.
#include "REL/m654DLL/framework/linknode.h"

extern "C" Node lbl_1_bss_10C;

struct FlagNodeLink : Node {
    ~FlagNodeLink() {}
};

struct ManagerEventBase : FlagNodeLink {
    virtual ~ManagerEventBase();
    virtual void handleEvent(int event) = 0;
    virtual void advanceFrame();
    int eventMask;
    ManagerEventBase() : eventMask(0) { insert4(&lbl_1_bss_10C); }
};

inline ManagerEventBase::~ManagerEventBase() {}

#endif
