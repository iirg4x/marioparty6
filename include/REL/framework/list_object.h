#ifndef REL_FRAMEWORK_LIST_OBJECT_H
#define REL_FRAMEWORK_LIST_OBJECT_H

// Objects that take requests: each one goes on one list when it is built, a request sets its bit in every object, and
// each object clears the bit when it has handled it. The names are descriptive: retail has no symbol or string for
// them. The class and file names are m648's, the member names those most modules gave; the file name sorts where
// retail links the object, between linknode.cpp and minigame_core.cpp.
#include "REL/framework/list.h"

struct RequestObject : ListNode<RequestObject> {
    RequestObject();
    virtual ~RequestObject() {}
    virtual void flagEnabled(int bit) = 0;
    virtual void update() {}
    int flags; // One bit for each pending request.
    RequestObject *next() { return static_cast<RequestObject *>(link0); }
};

extern ListRoot<RequestObject> requestObjects;

// Each object goes on the list when it is built.
inline RequestObject::RequestObject() : flags(0) { insert4(&requestObjects); }

void postRequest(int request);
bool requestsCleared();
void updateRequestObjects();
void deleteRequestObjects();
long currentRequest();

#endif
