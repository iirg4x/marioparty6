// Maintains the request-object list and sends requests to its members.
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

// Updates each object in the request list in list order.
void updateRequestObjects()
{
    RequestObject *object = requestObjects.front();
    Node *end = requestObjects.end_node();
    for (; object != end; object = object->next()) {
        object->update();
    }
}

// Deletes request objects from the back until the list is empty.
void deleteRequestObjects()
{
    while (!requestObjects.empty()) {
        delete requestObjects.back();
    }
}

// Returns the most recently stored request.
long currentRequest()
{
    return lastRequest;
}

