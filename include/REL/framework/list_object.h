#ifndef REL_FRAMEWORK_LIST_OBJECT_H
#define REL_FRAMEWORK_LIST_OBJECT_H

// Objects join one request list during construction. A request sets its bit in every object;
// each object clears the bit after handling it.
#include "REL/framework/list.h"

struct RequestObject : ListNode<RequestObject> {
    RequestObject();
    virtual ~RequestObject() {}
    virtual void flagEnabled(int bit) = 0;
    // Provides the default empty update for request objects.
    virtual void update() {}
    int flags; // One bit for each pending request.
    // Returns the next link as a request object.
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
