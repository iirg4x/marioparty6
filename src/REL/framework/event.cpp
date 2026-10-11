/* Owns minigame event channels and delivers immediate or scheduled packets to their receivers. */
static const char rcsid[] = "$Id: event.cpp,v 1.22 2004/09/09 12:24:38 saf Exp $";

extern "C" {
#include "REL/framework/assertion.h"
#include <stdio.h>
}
#include "REL/framework/tree_node.h"
void *operator new(unsigned long);
void operator delete(void *);

/* Virtual receiver interface called when a packet is dispatched on a registered channel. */
struct EventCallback {
    virtual ~EventCallback() {}
    virtual void receive(const struct EventPacket *) = 0;
};

/* Argument and result types shared by two-argument event operations. */
template<class First, class Second, class Result> struct EventBinaryFunction {
    typedef First first_argument_type;
    typedef Second second_argument_type;
    typedef Result result_type;
};
/* Argument and result types for a receiver operation with its packet already bound. */
template<class Argument, class Result> struct EventUnaryFunction {
    typedef Argument argument_type;
    typedef Result result_type;
};
/* Adapts a forwarding function for iteration over a channel's receiver addresses. */
template <class Receiver, class Packet>
struct EventForwardFunction : EventBinaryFunction<Receiver, Packet, int> {
    int (*callbackFunction)(Receiver, Packet); /* Called with the receiver and bound packet. */
    EventForwardFunction(int (*callback)(Receiver, Packet)) : callbackFunction(callback) {}
    int operator()(Receiver receiver, Packet packet) const {
        return callbackFunction(receiver, packet);
    }
};
typedef EventForwardFunction<unsigned long, const EventPacket *> EventForwardOperation;

struct EventChannel;
struct EventNode;

/* Tree links reserve the low address bit for a node tag. */
#define EVENT_PARENT_TAG_MASK 0x1UL

/* Tagged parent link used while walking the ordered channel tree. */
typedef TreeParent EventNodeParent;
/* Shared base of channel-map iterators. */
struct EventIteratorBase {};

/* Refers to a channel entry, or to its tree's end anchor after the final entry. */
struct EventIterator : EventIteratorBase {
    TreeNodeBase *node; /* Current channel node or end anchor. */
    EventIterator(EventNode *currentNode);
    bool operator==(const EventIterator& other) const { return node == other.node; }
    bool operator!=(const EventIterator& other) const { return node != other.node; }
    EventChannel& operator*() const;
    EventIterator& operator++() { advanceTreeNode(node); return *this; }
};
/* Carries paired tree values, including an insertion position and whether it was new. */
template<class First, class Second> struct EventPair {
    First first; /* First component selected by the caller's pair type. */
    Second second; /* Second component selected by the caller's pair type. */
    EventPair(const First& first, const Second& second) : first(first), second(second) {}
    template<class OtherFirst, class OtherSecond>
    EventPair(const EventPair<OtherFirst, OtherSecond>& other)
        : first(other.first), second(other.second) {}
};
struct EventMutableChannel;
/* Orders channel registrations by their signed channel number. */
struct EventCompare : EventBinaryFunction<int, int, bool> {
    bool operator()(const int& left, const int& right) const { return bool(left < right); }
};
/* Destroys tree values and releases nodes without deleting the receiver objects. */
struct EventAllocator {
    EventAllocator() {}
    EventAllocator(const EventAllocator&) {}
    template<class Value> void destroy(Value *value) { value->~Value(); }
    template<class Node> void deallocate(Node *node, unsigned long) { ::operator delete(node); }
};
/* Associates the shared event allocator with one tree value or node type. */
template<class Value> struct EventTypedAllocator : EventAllocator {
    EventTypedAllocator(const EventAllocator& allocator) : EventAllocator(allocator) {}
};
/* Keeps the tree's entry count beside its value allocator. */
template<class Allocator> struct EventCountStorage : Allocator {
    unsigned long entryCount; /* Number of stored entries; zero for an empty tree. */
    template<class OtherAllocator> EventCountStorage(const OtherAllocator& allocator)
        : Allocator(allocator), entryCount(0) {}
    Allocator& first() { return *this; }
    const unsigned long& second() const { return entryCount; }
};
typedef EventCountStorage<EventAllocator> EventCount;
/* Shared left-link layout for tree nodes and the root/end anchor. */
typedef TreeAnchor EventAnchor;
/* Keeps the root/end anchor beside the node allocator. */
template<class Allocator> struct EventRootStorage : Allocator {
    EventAnchor anchor; /* Stores the root in its left link and supplies the end iterator. */
    template <class OtherAllocator>
    EventRootStorage(const OtherAllocator &allocator) : Allocator(allocator)
    {
    }
    Allocator& first() { return *this; }
    EventAnchor& second() { return anchor; }
    const EventAnchor& second() const { return anchor; }
};
typedef EventRootStorage<EventAllocator> EventRoot;
/* Adapts channel-number comparison for the map's searches and insertions. */
struct EventValueCompare {
    EventCompare compare; /* Strict ascending comparison of signed channel numbers. */
    EventValueCompare(const EventCompare& other) : compare(other) {}
    bool operator()(const int& left, const int& right) const { return compare(left, right); }
};
/* Caches the first channel node beside the map's comparison rule. */
struct EventBegin : EventValueCompare {
    EventAnchor *firstNode; /* Lowest-key node, or the end anchor for an empty tree. */
    EventBegin(const EventCompare& compare) : EventValueCompare(compare) {}
    EventAnchor*& second() { return firstNode; }
    EventValueCompare& comparator() { return *this; }
};
/* Owns the ordered map from channel numbers to receiver sets. */
struct EventTree {
    /* Value allocator and channel count. */
    EventCountStorage<EventTypedAllocator<EventChannel> > count;
    EventRootStorage<EventTypedAllocator<EventNode> > root; /* Node allocator and root/end
                                                             * anchor. */
    EventBegin first; /* Channel comparison and cached first node. */
    EventAnchor *end_node() { return &root.second(); }
    EventAnchor *&begin_node() { return first.second(); }
    EventNode *&root_node() { return (EventNode*&)root.second().left; }
    EventAllocator& value_allocator() { return count.first(); }
    EventAllocator& node_allocator() { return root.first(); }
    template<class Node> void erase(Node *);
    ~EventTree() { if (root_node()) erase(root_node()); }
    EventIterator begin() { return (EventNode*)begin_node(); }
    EventIterator end() { return (EventNode*)&root.second(); }
    EventIterator find(const int&);
    template<class Value> EventPair<EventIterator, bool> insert(const Value&);
    EventNode *insertNode(EventNode *, bool, bool, const EventChannel&);
    template<class Allocator> EventTree(const EventCompare&, const Allocator&);
};
/* Channel map created with the default event comparison and allocator. */
struct EventMap : EventTree {
    ~EventMap() {}
    EventMap() : EventTree(EventCompare(), EventAllocator()) {}
    EventIterator end() { return EventTree::end(); }
    EventPair<EventIterator, bool> insert(const EventChannel& entry) {
        return EventTree::insert(entry);
    }
};

struct EventListenerNode;
/* Orders a channel's receivers by their stored unsigned addresses. */
struct EventListenerCompare : EventBinaryFunction<unsigned long, unsigned long, bool> {
    bool operator()(const unsigned long &left, const unsigned long &right) const
    {
        return bool(left < right);
    }
};
/* Caches the first receiver beside the registration tree's address comparison. */
struct EventListenerBegin : EventListenerCompare {
    EventListenerNode *firstNode; /* Lowest-address receiver node, or the end anchor when empty. */
    EventListenerBegin(const EventListenerCompare& compare) : EventListenerCompare(compare) {}
    EventListenerNode *&second() { return firstNode; }
    EventListenerCompare& comparator() { return *this; }
};
/* Refers to a receiver registration or the registration tree's end anchor. */
struct EventListenerIterator {
    TreeNodeBase *node; /* Current receiver node or end anchor. */
    EventListenerIterator(EventListenerNode *currentNode);
    bool operator==(const EventListenerIterator& other) const { return node == other.node; }
};
/* Visits registered receiver addresses without modifying the stored values. */
struct EventListenerConstIterator : EventListenerIterator {
    EventListenerConstIterator(const EventListenerIterator& other) : EventListenerIterator(other) {}
    bool operator!=(const EventListenerConstIterator& other) const { return node != other.node; }
    const unsigned long& operator*() const;
    EventListenerConstIterator& operator++();
};
/* Owns one channel's ordered receiver registrations, without owning the receivers. */
struct EventChannelTree {
    /* Value allocator and receiver count. */
    EventCountStorage<EventTypedAllocator<unsigned long> > count;
    /* Node allocator and root/end anchor. */
    EventRootStorage<EventTypedAllocator<EventListenerNode> > root;
    EventListenerBegin first; /* Receiver comparison and cached first node. */
    EventAnchor *end_node() { return &root.second(); }
    EventListenerNode *&root_node() { return (EventListenerNode*&)end_node()->left; }
    EventListenerNode *root_node() const { return (EventListenerNode*)root.second().left; }
    EventListenerNode *&begin_node() { return first.second(); }
    EventListenerNode *copyNode(EventListenerNode *);
    EventAllocator& value_allocator() { return count.first(); }
    EventAllocator& node_allocator() { return root.first(); }
    template<class Node> void erase(Node *);
    bool erase(const unsigned long&);
    EventListenerIterator find(const unsigned long&);
    EventListenerIterator end() { return (EventListenerNode*)&root.second(); }
    template<class Value> EventPair<EventListenerIterator, bool> insert(const Value&);
    EventListenerNode *insertNode(EventListenerNode *, bool, bool, const unsigned long&);
    unsigned long size() const { return count.entryCount; }
    ~EventChannelTree() { if (root_node()) erase(root_node()); }
    template<class OtherTree> EventChannelTree(const OtherTree&);
    template<class Allocator> EventChannelTree(const EventListenerCompare&, const Allocator&);
};
/* Receiver set that rejects duplicate addresses and exposes read-only iteration. */
struct EventChannelSet : EventChannelTree {
    EventChannelSet() : EventChannelTree(EventListenerCompare(), EventAllocator()) {}
    EventChannelSet(const EventChannelSet& other) : EventChannelTree(other) {}
    ~EventChannelSet() {}
    EventPair<EventListenerConstIterator, bool> insert(const unsigned long& listener) {
        EventPair<EventListenerIterator, bool> result = EventChannelTree::insert(listener);
        return EventPair<EventListenerConstIterator, bool>(result.first, result.second);
    }
    bool empty() const { return count.second() == 0; }
    EventListenerIterator tree_begin() { return first.second(); }
    EventListenerIterator tree_end() { return (EventListenerNode*)&root.second(); }
    EventListenerConstIterator begin() { return tree_begin(); }
    EventListenerConstIterator end() { return tree_end(); }
};
/* Temporary channel entry copied into the map on its first registration. */
struct EventMutableChannel {
    int key; /* Signed channel number used as the map key. */
    EventChannelSet listeners; /* Receiver registrations copied into the new channel. */
    EventMutableChannel(int channel, const EventChannelSet& registrations)
        : key(channel), listeners(registrations) {}
};
/* Stored channel number and the receiver addresses registered for it. */
struct EventChannel {
    const int key; /* Immutable signed channel number used by map searches. */
    EventChannelSet listeners; /* Unique registered receiver addresses. */
    EventChannel(const EventMutableChannel& other) : key(other.key), listeners(other.listeners) {}
};
/* Channel-map node containing a registration set and its ordered tree links. */
struct EventNode : TreeNodeBase {
    EventChannel value; /* Channel key and receiver registrations stored in this node. */
};
inline EventIterator::EventIterator(EventNode *currentNode) : node(currentNode) {}
inline EventChannel &EventIterator::operator*() const
{
    return static_cast<EventNode *>(node)->value;
}
inline EventMutableChannel makeEventChannel(int channel, const EventChannelSet& registrations)
{
    return EventMutableChannel(channel, registrations);
}

/* Packet routed by channel; its remaining words are passed unchanged to each receiver. */
struct EventPacket {
    int channel; /* Signed channel number selecting the registered receiver set. */
    unsigned long value; /* Message value supplied by the sender. */
    unsigned long context; /* Additional sender data; zero in the shorter sendEvent overload. */
};
/* Queued packet copy with its requested delivery time on the owner's event clock. */
struct EventEntry : EventPacket {
    float scheduled_time; /* Delivery time in seconds, compared with the last processed tick. */
    EventEntry(const EventPacket& packet, float time) : EventPacket(packet), scheduled_time(time) {}
    EventEntry(const EventEntry& other)
        : EventPacket(other), scheduled_time(other.scheduled_time) {}
    /* Queue assignments copy the packet words and scheduled time together. */
    EventEntry& operator=(const EventEntry& other) {
        channel=other.channel;
        value=other.value;
        context=other.context;
        scheduled_time=other.scheduled_time;
        return *this;
    }
};
/* Allocator base used by the queue's event-entry sequence. */
template<class T> struct EventSequenceAllocator {};
/* Contiguous storage used by the scheduled-event queue. */
struct EventSequence : EventSequenceAllocator<EventEntry> {
    unsigned long reserved, count; /* Storage counts initialized to zero for an empty queue. */
    EventEntry *storage; /* Entry buffer; null before the queue stores any packets. */
    EventEntry *&storage_pointer() { return storage; }
    EventEntry *begin() { return storage_pointer(); }
    EventSequence() : reserved(0), count(0), storage(0) {}
    ~EventSequence();
};
/* Sequence owner that releases its entry storage through the base destructor. */
struct EventSequenceOwner : EventSequence { EventSequenceOwner() {} ~EventSequenceOwner() {} };
/* Sequence wrapper passed into scheduled-queue construction. */
struct EventSequenceOuter : EventSequenceOwner { EventSequenceOuter() {} ~EventSequenceOuter() {} };
/* Comparison object passed to the scheduled-event queue constructor. */
struct EventQueueCompare { EventQueueCompare() {} };
/* Holds copied packets until process_event_queue reaches their scheduled time. */
struct EventQueue {
    EventSequenceOuter values; /* Owned packet storage; the first entry is the next to dispatch. */
    EventQueueCompare compare; /* Queue comparison object retained at construction. */
    EventQueue(const EventQueueCompare&, const EventSequenceOuter&);
    ~EventQueue() {}
    bool empty() const { return values.count == 0; }
    const EventEntry& top() { return *values.begin(); }
    void push(const EventEntry&);
};
struct EventOwner;
/* Owner published at construction and cleared when that owner is destroyed. */
EventOwner *active_event_owner;
/* Owns channel registrations and the queue/clock used by the minigame event interface. */
struct EventOwner : EventMap {
    EventQueue queue; /* Pending copies of delayed packets. */
    int tick_count; /* Number of event-queue updates since construction. */
    float current_time; /* Last processed event time in seconds; initially zero. */
    EventOwner();
    ~EventOwner();
    void registerListener(int, unsigned long);
    void removeListener(unsigned long);
};
inline float event_time_from_tick(float);
void dispatch_event(EventOwner *, const EventPacket&);
void remove_queued_event(EventQueue *);

inline EventOwner *current_event_owner() { return active_event_owner; }

/* EventManager construction creates the active owner with an empty map, queue, and clock. */
EventOwner::EventOwner()
    : EventMap(), queue(EventQueueCompare(), EventSequenceOuter())
{
    active_event_owner = this;
    tick_count = 0;
    current_time = 0.0f;
}

/* Deleting the event manager releases its scheduled entries and channel map. */
EventOwner::~EventOwner()
{
    active_event_owner = 0;
}

/* Add a listener to its channel, creating an empty channel on the first registration. */
void EventOwner::registerListener(int channel, unsigned long listener)
{
    EventIterator position = find(channel);
    if (position == end()) {
        position = insert(makeEventChannel(channel, EventChannelSet())).first;
    }
    (*position).listeners.insert(listener);
}

/* Remove a listener from every channel when its event receiver is released. */
void EventOwner::removeListener(unsigned long listener)
{
    EventIterator position = begin();
    EventIterator finish = end();
    for (; position != finish; ++position) {
        EventChannelSet& registrations = (*position).listeners;
        registrations.erase(listener);
    }
}

/* Tracks the active event interface and rejects construction while another one exists. */
template<class Owner> struct EventSingleton {
    static EventSingleton *self; /* Active interface instance, or null before creation/after
                                  * deletion. */
    /* Interface construction reports and aborts if the singleton slot is already occupied. */
    EventSingleton() {
        !self ? (void)0 : __msl_assertion_failed("self == 0", "singleton.h", 50);
        self = this;
    }
    ~EventSingleton() { self = 0; }
};
template<class Owner> EventSingleton<Owner> *EventSingleton<Owner>::self;
/* Minigame-facing lifetime owner of the shared event map, queue, and clock. */
struct EventManager : EventSingleton<EventManager> {
    EventManager();
    ~EventManager();
};

/* Construct the shared event owner when the minigame event interface starts. */
EventManager::EventManager()
{
    new EventOwner();
}

/* Release pending events before clearing the active minigame event interface. */
EventManager::~EventManager()
{
    EventOwner *owner = current_event_owner();
    delete owner;
}

/* On each event update, advance the clock and dispatch then remove due entries. */
void process_event_queue()
{
    EventOwner *owner=current_event_owner();
    if (owner) {
        float currentTime=event_time_from_tick(++owner->tick_count);
        owner->current_time=currentTime;
        EventQueue &queue=owner->queue;
        while (!queue.empty()) {
            const EventEntry &entry=queue.top();
            if (entry.scheduled_time > currentTime) break;
            /* Receivers run before the queued entry is removed. */
            dispatch_event(owner,entry);
            remove_queued_event(&queue);
        }
    }
}

/* Tagged parent link retained when a registration node is connected to a new parent. */
typedef TreeParent EventListenerParent;
/* Listener-set node containing one receiver address and its ordered tree links. */
struct EventListenerNode : TreeNodeBase {
    unsigned long receiver; /* Registered EventCallback address; not owned by the node. */
};
inline EventListenerIterator::EventListenerIterator(EventListenerNode *currentNode)
    : node(currentNode)
{
}
inline const unsigned long &EventListenerConstIterator::operator*() const
{
    return static_cast<EventListenerNode *>(node)->receiver;
}
/* Advance the receiver iterator used by dispatch_event and return it for continued traversal. */
inline EventListenerConstIterator& EventListenerConstIterator::operator++()
{
    /* Channel and receiver nodes share the same left, right and tagged-parent links. */
    advanceTreeNode(node);
    return *this;
}
int forwardEvent(unsigned long, const EventPacket *);

/* Binds the same packet pointer to the forwarding operation used for each receiver. */
template <class Operation>
struct EventPacketBinder : EventUnaryFunction<typename Operation::first_argument_type, int> {
    Operation operation; /* Forwarding operation called for each registered receiver. */
    typename Operation::second_argument_type packet; /* Packet passed to every operation call. */
    EventPacketBinder(const Operation& operation, typename Operation::second_argument_type packet)
        : operation(operation) { this->packet = packet; }
    int operator()(typename Operation::first_argument_type const &receiver) const
    {
        return operation(receiver, packet);
    }
};
typedef EventPacketBinder<EventForwardOperation> EventBoundPacket;
/* dispatch_event wraps the virtual-call adapter for receiver iteration. */
inline EventForwardOperation eventForwardFunction(int (*callback)(unsigned long,
                                                                  const EventPacket *))
{
    return EventForwardOperation(callback);
}
/* dispatch_event binds its packet before visiting a channel's registered receivers. */
inline EventBoundPacket bindEventPacket(const EventForwardOperation& operation,
    const EventPacket *packet)
{
    return EventBoundPacket(operation, packet);
}
/* dispatch_event invokes the bound operation once per receiver, ignoring its return value. */
inline EventBoundPacket visitEventReceivers(EventListenerConstIterator first,
    EventListenerConstIterator last, EventBoundPacket callback)
{
    for (; first != last; ++first) callback(*first);
    return callback;
}

/* Deliver a packet to every receiver currently registered for its channel. */
void dispatch_event(EventOwner *owner, const EventPacket& packet)
{
    EventIterator position = owner->find(packet.channel);
    if (position != owner->end()) {
        EventChannelSet& receivers = (*position).listeners;
        if (!receivers.empty()) {
            visitEventReceivers(receivers.begin(), receivers.end(),
                bindEventPacket(eventForwardFunction(forwardEvent), &packet));
            return;
        }
    }
    /* Drop the packet with a warning when its channel is absent or has no receivers. */
    printf("CEventBroadcaster::WARNING\nlistener not found in ch.%d\n", packet.channel);
}

/* Adapt a receiver's virtual event callback for the channel iteration operation. */
int forwardEvent(unsigned long receiver, const EventPacket *packet)
{
    ((EventCallback*)receiver)->receive(packet);
    return 0;
}

/* Queue a packet copy for a positive delay, or dispatch immediately otherwise.
 * With no active event owner, the packet is ignored. */
void sendEvent(const EventPacket& packet, float delay)
{
    EventOwner *owner = current_event_owner();
    if (owner) {
        if (delay > 0.0f) {
            owner->queue.push(EventEntry(packet, owner->current_time + delay));
        } else {
            dispatch_event(owner, packet);
        }
    }
}

/* Receiver interface that registers its address on channels and unregisters on destruction. */
struct EventListener : EventCallback {
    EventListener();
    virtual ~EventListener();
    void registerChannel(int);
};
EventListener::EventListener() {}

/* Remove this receiver from every channel before its callback interface is destroyed. */
EventListener::~EventListener()
{
    EventOwner *owner = current_event_owner();
    if (owner) owner->removeListener((unsigned long)this);
}

/* Register this receiver on a channel after the shared event owner has been created. */
void EventListener::registerChannel(int channel)
{
    EventOwner *owner = current_event_owner();
    owner != 0 ? (void)0 : __msl_assertion_failed("p != NULL", "event.cpp", 234);
    owner->registerListener(channel, (unsigned long)this);
}

/* Construct an event with a channel, value and context, then apply the requested delay. */
void sendEvent(int channel, unsigned long value, unsigned long context, float delay)
{
    EventPacket packet = {};
    packet.channel = channel;
    packet.value = value;
    packet.context = context;
    sendEvent(packet, delay);
}

/* Construct an event with zero context, then apply the requested delay. */
void sendEvent(int channel, unsigned long value, float delay)
{
    EventPacket packet = {};
    packet.channel = channel;
    packet.value = value;
    sendEvent(packet, delay);
}

void eraseEventListenerPosition(EventChannelTree *, EventListenerIterator);

/* Remove the matching receiver from a channel's ordered registration tree. */
bool EventChannelTree::erase(const unsigned long& listener)
{
    EventListenerIterator position = find(listener);
    if (position == end()) return false;
    eraseEventListenerPosition(this, position);
    return true;
}

/* Convert the event owner's frame count to seconds at the minigame's 60 Hz update rate. */
inline float event_time_from_tick(float frames)
{
    return (1.0 / 60.0) * frames;
}

/* Registration and packet dispatch find the requested channel, or the map's end iterator. */
EventIterator EventTree::find(const int& channel)
{
    EventNode *node = (EventNode*)root_node();
    EventNode *candidate = (EventNode*)&root.second();
    while (node != 0) {
        if (!first.comparator()(node->value.key, channel)) {
            candidate = node;
            node = (EventNode*)node->left;
        } else {
            node = static_cast<EventNode *>(node->right);
        }
    }
    if (candidate == (EventNode *) &root.second() ||
        first.comparator()(channel, candidate->value.key))
        return end();
    return candidate;
}

/* Shared ordered-tree iteration finds the first entry when advancing into a right subtree. */
inline TreeNodeBase *firstEventDescendant(TreeNodeBase *node)
{
    while (node->left != 0) node = (TreeNodeBase*)node->left;
    return node;
}

/* Ordered event and selected-leaf iterators call this to find their next entry or end anchor. */
void advanceTreeNode(TreeNodeBase *&node)
{
    if (node->right != 0) {
        node = firstEventDescendant(node->right);
    } else {
        /* Climb past ancestors reached from a right child, then visit the parent. */
        while (node != node->parent->left) node = node->parent;
        node = node->parent;
    }
}

/* Channel destruction releases its registration nodes without destroying their receivers. */
template<class Node> inline void EventChannelTree::erase(Node *node)
{
    if (node->left) erase((EventListenerNode*)node->left);
    if (node->right) erase(static_cast<EventListenerNode *>(node->right));
    value_allocator().destroy(&node->receiver);
    node_allocator().deallocate(node, 1);
}

/* Owner destruction releases descendants before each channel's receiver set and node allocation. */
template<class Node> void EventTree::erase(Node *node)
{
    if (node->left) erase((EventNode*)node->left);
    if (node->right) erase(static_cast<EventNode *>(node->right));
    value_allocator().destroy(&node->value);
    node_allocator().deallocate(node, 1);
}

/* First registration inserts a channel and returns its iterator with true; duplicate keys
 * return the existing channel's iterator with false. */
template<class Value> EventPair<EventIterator, bool> EventTree::insert(const Value& entry)
{
    EventNode *predecessor = 0;
    EventNode *parent = (EventNode*)&root.second();
    EventNode *node = root_node();
    bool insertLeft = true;
    bool becomesFirst = true;
    while (node != 0) {
        parent = node;
        if (first.comparator()(entry.key, node->value.key)) {
            node = (EventNode*)node->left;
            insertLeft = true;
        } else {
            predecessor = parent;
            node = static_cast<EventNode *>(node->right);
            insertLeft = false;
            becomesFirst = false;
        }
    }
    /* The predecessor detects an existing key after the search reaches an empty link. */
    if (predecessor == 0 || first.comparator()(predecessor->value.key, entry.key)) {
        node = insertNode(parent, insertLeft, becomesFirst, entry);
        EventIterator position(node);
        return EventPair<EventIterator, bool>(position, true);
    }
    EventIterator position(predecessor);
    return EventPair<EventIterator, bool>(position, false);
}

/* Start with an empty channel tree whose begin iterator points at its end anchor. */
template <class Allocator>
EventTree::EventTree(const EventCompare &compare, const Allocator &allocator)
    : count(allocator), root(allocator), first(compare)
{
    begin_node() = end_node();
}

/* Listener registration inserts an address once and reports whether the set gained an entry. */
template <class Value>
EventPair<EventListenerIterator, bool> EventChannelTree::insert(const Value &listener)
{
    EventListenerNode *predecessor = 0;
    EventListenerNode *parent = (EventListenerNode*)&root.second();
    EventListenerNode *node = root_node();
    bool insertLeft = true;
    bool becomesFirst = true;
    while (node != 0) {
        parent = node;
        if (first.comparator()(listener, node->receiver)) {
            node = (EventListenerNode*)node->left;
            insertLeft = true;
        } else {
            predecessor = parent;
            node = static_cast<EventListenerNode *>(node->right);
            insertLeft = false;
            becomesFirst = false;
        }
    }
    /* The predecessor detects a repeated address after the search reaches an empty link. */
    if (predecessor == 0 || first.comparator()(predecessor->receiver, listener)) {
        node = insertNode(parent, insertLeft, becomesFirst, listener);
        EventListenerIterator position(node);
        return EventPair<EventListenerIterator, bool>(position, true);
    }
    EventListenerIterator position(predecessor);
    return EventPair<EventListenerIterator, bool>(position, false);
}

/* Registration-tree copying finds its first receiver by following the copied left links. */
inline EventListenerNode *firstEventListenerDescendant(EventListenerNode *node)
{
    while (node->left != 0) node = (EventListenerNode*)node->left;
    return node;
}

/* Channel creation copies its registration tree and reconnects the root to the new end anchor. */
template<class OtherTree> EventChannelTree::EventChannelTree(const OtherTree& other)
    : count(other.count), root(other.root), first(other.first)
{
    if (other.root_node() != 0) {
        root_node() = copyNode(other.root_node());
        /* Traversal from the copied root must finish at this tree's own end anchor. */
        root_node()->parent = end_node();
        begin_node() = firstEventListenerDescendant(root_node());
    } else {
        begin_node() = (EventListenerNode*)end_node();
    }
}

/* Start with an empty listener tree whose first position is its end anchor. */
template<class Allocator> EventChannelTree::EventChannelTree(const EventListenerCompare& compare,
    const Allocator& allocator)
    : count(allocator), root(allocator), first(compare)
{
    begin_node() = (EventListenerNode*)end_node();
}
