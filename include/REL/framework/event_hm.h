/* Defines channel-event callbacks and the owning map used to dispatch them by type. */
#ifndef FRAMEWORK_EVENT_HM_H
#define FRAMEWORK_EVENT_HM_H

/* Assigning a parent pointer preserves this low-bit node tag. */
#define HANDLER_PARENT_TAG_MASK 0x1UL

void *operator new(unsigned long);
void operator delete(void *);
inline void *operator new(unsigned long, void *storage) { return storage; }

/* A message carrying an event type and additional data to its registered callbacks. */
struct EventMessage {
    unsigned long context; // Additional message data; this dispatcher does not read it.
    int type; // Event type used to select callbacks.
};

/* Virtual interface for receiving a minigame event. */
struct EventCallback {
    virtual ~EventCallback();
    virtual void receive(EventMessage *);
};

/* A callback interface that can register with an event channel. */
struct EventListener : EventCallback {
    EventListener();
    virtual ~EventListener();
    void registerChannel(int);
};

/* One event-type key and callback pointer stored in the dispatch map. */
struct HandlerValue {
    int eventType; // Ordered key; several entries may share it.
    EventCallback *callback; // Deleted by the manager before its tree is destroyed.
    HandlerValue(const int& eventType, EventCallback *const& handler)
        : eventType(eventType), callback(handler) {}
    HandlerValue(const HandlerValue &other) : eventType(other.eventType), callback(other.callback)
    {
    }
};

struct HandlerNode;
/* Parent link with a low-bit tag retained when insertion sets the pointer. */
struct HandlerParent {
    unsigned long parentBits; // Parent pointer together with HANDLER_PARENT_TAG_MASK.
    /* insertNode assigns the parent without overwriting the existing node tag. */
    HandlerParent& operator=(HandlerNode *node) {
        parentBits = (unsigned long)node | (parentBits & HANDLER_PARENT_TAG_MASK);
        return *this;
    }
};
/* The shared left-link layout also provides the tree's end anchor. */
struct HandlerAnchor {
    void *left; // Left child on nodes, or the tree root on the end anchor; null when absent.
    HandlerAnchor() : left(0) {}
};
/* A tree node containing one callback registration and its tree links. */
struct HandlerNode : HandlerAnchor {
    HandlerNode *right; // Right child, or null when absent.
    HandlerParent parent; // Tagged link to the parent, including the end anchor at the root.
    HandlerValue entry; // Event type and the registered callback.
};

/* Orders callback entries by their signed event-type keys. */
struct HandlerCompare {
    bool operator()(const int& left, const int& right) const { return bool(left < right); }
};
/* Destroys stored registration values; the manager deletes callback objects separately. */
struct HandlerAllocator {
    HandlerAllocator() {}
    HandlerAllocator(const HandlerAllocator&) {}
    void destroy(HandlerValue *entry) { entry->~HandlerValue(); }
};
/* Allocates node storage for the callback map and releases it on tree destruction. */
struct HandlerNodeAllocator {
    HandlerNodeAllocator(const HandlerAllocator&) {}
    /* insertNode requests storage here; the optional allocation hint is ignored. */
    HandlerNode *allocate(unsigned long nodeCount, const void *allocationHint = 0) {
        HandlerNode *allocatedNode = (HandlerNode*)::operator new(nodeCount * sizeof(HandlerNode));
        if (allocatedNode == 0) {
            // A null storage result reports the allocation error and aborts.
            fprintf(stderr, "Memory allocation failure");
            abort();
        }
        return allocatedNode;
    }
    void deallocate(HandlerNode *node, unsigned long) { ::operator delete(node); }
};
/* Pairs the entry allocator with the number of callback registrations in the tree. */
struct HandlerCount : HandlerAllocator {
    unsigned long entryCount; // Number of inserted nodes; zero for an empty tree.
    HandlerCount(const HandlerAllocator& allocator) : HandlerAllocator(allocator), entryCount(0) {}
    HandlerAllocator& first() { return *this; }
    unsigned long& second() { return entryCount; }
    const unsigned long& second() const { return entryCount; }
    void construct(HandlerValue *storage, const HandlerValue& entry) {
        new(storage) HandlerValue(entry);
    }
};
/* Pairs node allocation with the anchor that stores the root and end iterator. */
struct HandlerRoot : HandlerNodeAllocator {
    HandlerAnchor anchor; // Its left link stores the root; its address is the end iterator.
    HandlerRoot(const HandlerAllocator& allocator) : HandlerNodeAllocator(allocator) {}
    HandlerNodeAllocator& first() { return *this; }
    HandlerAnchor& second() { return anchor; }
};
/* Adapts event-type comparison for the callback map's searches and insertions. */
struct HandlerValueCompare {
    HandlerCompare compare; // Strict ascending comparison of event types.
    HandlerValueCompare() {}
    HandlerValueCompare(const HandlerCompare& other) : compare(other) {}
    bool operator()(const int& left, const int& right) const { return compare(left, right); }
};
/* Pairs the comparison rule with the first node cached for ordered iteration. */
struct HandlerBegin : HandlerValueCompare {
    HandlerNode *firstNode; // Lowest-key node, or the end anchor when the tree is empty.
    HandlerBegin(const HandlerValueCompare& compare) : HandlerValueCompare(compare) {}
    HandlerValueCompare& first() { return *this; }
    HandlerNode*& second() { return firstNode; }
};
/* Shared base of callback-map iterators. */
struct HandlerIteratorBase {};
/* Refers to one callback entry during dispatch or destruction. */
struct HandlerIterator : HandlerIteratorBase {
    HandlerNode *currentNode; // Current entry, or the tree's end anchor after the last entry.
    HandlerIterator(HandlerNode *node) : currentNode(node) {}
    HandlerValue& operator*() const { return currentNode->entry; }
    HandlerIterator& operator++();
    bool operator!=(const HandlerIterator& other) const { return currentNode != other.currentNode; }
};

/* Owns the ordered nodes that group callback registrations by event type. */
struct HandlerTree {
    HandlerCount entryStorage; // Entry allocator and number of nodes.
    HandlerRoot nodeStorage; // Node allocator and root/end anchor.
    HandlerBegin ordering; // Event-type ordering and cached first node.
    HandlerTree(const HandlerValueCompare&, const HandlerAllocator&);
    ~HandlerTree();
    HandlerNode*& root() { return (HandlerNode*&)nodeStorage.second().left; }
    HandlerNode *endNode() { return (HandlerNode*)&nodeStorage.second(); }
    HandlerNode*& beginNode() { return ordering.second(); }
    HandlerIterator begin() { return beginNode(); }
    HandlerIterator end() { return (HandlerNode*)&nodeStorage.second(); }
    HandlerIterator lowerBound(const int&);
    HandlerIterator upperBound(const int&);
    void erase(HandlerNode *);
    HandlerNodeAllocator& nodeAllocator() { return nodeStorage.first(); }
    HandlerAllocator& valueAllocator() { return entryStorage.first(); }
    unsigned long size() const { return entryStorage.second(); }
    static unsigned long limit() { return (unsigned long)-1; }
    unsigned long maxSize() const { return limit(); }
    template<class Value>
    HandlerNode *insertNode(HandlerNode *, bool, bool, const Value&);
    HandlerIterator insert(const HandlerValue&);
};
inline HandlerTree::~HandlerTree() { if (root()) erase(root()); }
/* Creates the callback tree with its default event-type comparison and allocator. */
struct HandlerMapBase : HandlerTree {
    HandlerMapBase() : HandlerTree(HandlerValueCompare(), HandlerAllocator()) {}
    ~HandlerMapBase() {}
};
/* Callback map owned by an event-handler manager. */
struct HandlerMap : HandlerMapBase {
    HandlerMap() {}
    ~HandlerMap();
};
inline HandlerMap::~HandlerMap() {}
/* Deletes the map and its nodes when the owning manager is destroyed. */
struct HandlerMapOwner {
    HandlerMap *map; // Owned map; the manager deletes its callbacks separately.
    HandlerMapOwner(HandlerMap *handlerMap) : map(handlerMap) {}
    ~HandlerMapOwner() { delete map; }
    HandlerMap *get() const { return map; }
    HandlerMap *operator->() const { return get(); }
    HandlerMap& operator*() const { return *map; }
};

/* Owns callbacks grouped by event type and receives events from one channel. */
struct EventHandlerManager : EventListener {
    HandlerMapOwner handlers; // Owned callback registrations, ordered by event type.
    EventHandlerManager(int channel);
    virtual ~EventHandlerManager();
    void addHandler(EventCallback *, int);
    virtual void receive(EventMessage *);
};

void rebalanceHandlerTree(HandlerNode *, HandlerNode *);

#endif
