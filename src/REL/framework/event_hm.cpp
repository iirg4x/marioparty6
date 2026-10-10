/* Dispatches channel events to the minigame callbacks registered for each event type. */
extern "C" {
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/stdio.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/abort_exit.h"
}
#include "REL/framework/event_hm.h"

static const char rcsid[] = "$Id: event_hm.cpp,v 1.21 2004/09/09 12:24:38 saf Exp $";

/* At construction, create the owned callback map and register the requested channel. */
EventHandlerManager::EventHandlerManager(int channel) : handlers(new HandlerMap)
{
    registerChannel(channel);
}

/* On destruction, delete every callback before the map owner releases the tree nodes. */
EventHandlerManager::~EventHandlerManager()
{
    HandlerMap& handlerMap = *handlers;
    HandlerIterator currentHandler = handlerMap.begin();
    HandlerIterator endHandler = handlerMap.end();
    while (currentHandler != endHandler) {
        delete (*currentHandler).callback;
        ++currentHandler;
    }
}

/* When a caller registers a callback, retain it under its type, including duplicate types. */
void EventHandlerManager::addHandler(EventCallback *handler, int eventType)
{
    HandlerValue entry(eventType, handler);
    HandlerMap& handlerMap = *handlers;
    handlerMap.insert(entry);
}

/* On receipt of a channel event, invoke every callback registered for its type. */
void EventHandlerManager::receive(EventMessage *message)
{
    HandlerMap& handlerMap = *handlers;
    HandlerIterator currentHandler = handlerMap.lowerBound(message->type);
    HandlerIterator pastTypeHandlers = handlerMap.upperBound(message->type);
    while (currentHandler != pastTypeHandlers) {
        (*currentHandler).callback->receive(message);
        ++currentHandler;
    }
}

/* receive uses this to find the first callback at or after the requested event type. */
HandlerIterator HandlerTree::lowerBound(const int& eventType)
{
    HandlerNode *currentNode = root();
    HandlerNode *bound = (HandlerNode*)&nodeStorage.second();
    // The anchor is the end iterator when no callback meets the bound.
    while (currentNode != 0) {
        if (!ordering.first()(currentNode->entry.eventType, eventType)) {
            bound = currentNode;
            currentNode = (HandlerNode*)currentNode->left;
        } else {
            currentNode = currentNode->right;
        }
    }
    return bound;
}

/* receive uses this to mark the end of callbacks with the requested event type. */
HandlerIterator HandlerTree::upperBound(const int& eventType)
{
    HandlerNode *currentNode = root();
    HandlerNode *bound = (HandlerNode*)&nodeStorage.second();
    while (currentNode != 0) {
        if (ordering.first()(eventType, currentNode->entry.eventType)) {
            bound = currentNode;
            currentNode = (HandlerNode*)currentNode->left;
        } else {
            currentNode = currentNode->right;
        }
    }
    return bound;
}

/* The tree destructor releases a subtree here; callback objects were deleted by the manager. */
void HandlerTree::erase(HandlerNode *node)
{
    if (node->left) erase((HandlerNode*)node->left);
    if (node->right) erase(node->right);
    valueAllocator().destroy(&node->entry);
    nodeAllocator().deallocate(node, 1);
}

/* addHandler inserts here, placing another callback after entries with the same event type. */
HandlerIterator HandlerTree::insert(const HandlerValue& entry)
{
    HandlerNode *insertionParent = (HandlerNode*)&nodeStorage.second();
    HandlerNode *searchNode = root();
    bool insertLeft = true;
    bool insertFirst = true;
    while (searchNode != 0) {
        insertionParent = searchNode;
        if (ordering.first()(entry.eventType, searchNode->entry.eventType)) {
            searchNode = (HandlerNode*)searchNode->left;
            insertLeft = true;
        } else {
            searchNode = searchNode->right;
            insertLeft = false;
            insertFirst = false;
        }
    }
    return insertNode(insertionParent, insertLeft, insertFirst, entry);
}

/* Map construction sets the first iterator to the empty tree's end anchor. */
HandlerTree::HandlerTree(const HandlerValueCompare& compare, const HandlerAllocator& allocator)
    : entryStorage(allocator), nodeStorage(allocator), ordering(compare)
{
    beginNode() = (HandlerNode*)&nodeStorage.second();
}

/* insert calls this after choosing a parent, to allocate, link and rebalance the new entry. */
template <class Value>
HandlerNode *HandlerTree::insertNode(HandlerNode *insertionParent, bool insertLeft,
                                     bool insertFirst, const Value &entry)
{
    if (size() > maxSize() - 1) {
        fprintf(stderr, "tree::insert length error\n");
        abort();
    }
    HandlerNode *newNode = nodeAllocator().allocate(1);
    entryStorage.construct(&newNode->entry, entry);
    newNode->left = newNode->right = 0;
    newNode->parent = insertionParent;
    if (insertLeft) insertionParent->left = newNode;
    else insertionParent->right = newNode;
    ++entryStorage.second();
    rebalanceHandlerTree(newNode, root());
    // A path containing only left branches makes this the new first entry.
    if (insertFirst) beginNode() = newNode;
    return newNode;
}
