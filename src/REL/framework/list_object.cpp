// The list of request objects and the requests sent to all of them. Retail has no $Id string for this file.
#include "REL/framework/list_object.h"

ListRoot<RequestObject> requestObjects;
unsigned long lastRequest; // The last request posted.

// Sets the request's bit in every object and lets each one act on it.
void postRequest(int request)
{
    RequestObject *object = requestObjects.front();
    Node *end = requestObjects.end_node();
    lastRequest = request;
    for (; object != end; object = object->next()) {
        object->flags |= 1U << request;
        object->flagEnabled(request);
    }
}

// True once no object still has the last request's bit.
bool requestsCleared()
{
    RequestObject *object = requestObjects.front();
    Node *end = requestObjects.end_node();
    for (; object != end; object = object->next()) {
        unsigned long request = lastRequest;
        if ((object->flags & (1 << request)) != 0) {
            return false;
        }
    }
    return true;
}

void updateRequestObjects()
{
    RequestObject *object = requestObjects.front();
    Node *end = requestObjects.end_node();
    for (; object != end; object = object->next()) {
        object->update();
    }
}

void deleteRequestObjects()
{
    while (!requestObjects.empty()) {
        delete requestObjects.back();
    }
}

long currentRequest()
{
    return lastRequest;
}

