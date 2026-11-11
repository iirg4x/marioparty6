/* Owns minigame event channels and delivers immediate or scheduled packets to their receivers. */
#include <map>
#include <set>
#include <queue>
#include <functional>
#include <algorithm>
#include <cstdio>

extern "C" {
#include "REL/framework/assertion.h"
}

#include "REL/framework/event_callback_inline.h"
#include "REL/framework/singleton.h"
#include "REL/framework/frame_time.h"

static const char rcsid[] = "$Id: event.cpp,v 1.22 2004/09/09 12:24:38 saf Exp $";

/* Queued packet copy with its requested delivery time on the broadcaster's event clock. */
struct EventEntry : EventPacket {
    float scheduled_time; /* Delivery time in seconds, compared with the last processed tick. */
    EventEntry(const EventPacket& packet, float time) : EventPacket(packet), scheduled_time(time) {}
    EventEntry(const EventEntry& other)
        : EventPacket(other), scheduled_time(other.scheduled_time) {}
    /* Queue assignments copy the packet words and scheduled time together. */
    EventEntry& operator=(const EventEntry& other) {
        channel=other.channel;
        type=other.type;
        context=other.context;
        scheduled_time=other.scheduled_time;
        return *this;
    }
};
/* A later delivery time orders first, so the heap keeps the earliest entry at its top. */
inline bool operator<(const EventEntry& left, const EventEntry& right)
{
    return left.scheduled_time > right.scheduled_time;
}
/* Holds copied packets until process_event_queue reaches their scheduled time. */
typedef std::priority_queue<EventEntry> EventQueue;

/* Registered receiver addresses of one channel, without ownership of the receivers. */
typedef std::set<unsigned long> EventReceivers;

struct CEventBroadcaster;
/* Owner published at construction and cleared when that owner is destroyed. */
CEventBroadcaster *active_event_owner;
/* Owns channel registrations and the queue/clock used by the minigame event interface. */
struct CEventBroadcaster : std::map<int, EventReceivers> {
    EventQueue queue; /* Pending copies of delayed packets. */
    int tick_count; /* Number of event-queue updates since construction. */
    float current_time; /* Last processed event time in seconds; initially zero. */
    CEventBroadcaster();
    ~CEventBroadcaster();
    void registerListener(int, unsigned long);
    void removeListener(unsigned long);
};
void dispatch_event(CEventBroadcaster *, const EventPacket&);

inline CEventBroadcaster *current_event_owner() { return active_event_owner; }

/* EventManager construction creates the active owner with an empty map, queue, and clock. */
CEventBroadcaster::CEventBroadcaster()
    : queue()
{
    active_event_owner = this;
    tick_count = 0;
    current_time = 0.0f;
}

/* Deleting the event manager releases its scheduled entries and channel map. */
CEventBroadcaster::~CEventBroadcaster()
{
    active_event_owner = 0;
}

/* Add a listener to its channel, creating an empty channel on the first registration. */
void CEventBroadcaster::registerListener(int channel, unsigned long listener)
{
    iterator position = find(channel);
    if (position == end()) {
        position = insert(std::make_pair(channel, EventReceivers())).first;
    }
    (*position).second.insert(listener);
}

/* Remove a listener from every channel when its event receiver is released. */
void CEventBroadcaster::removeListener(unsigned long listener)
{
    iterator position = begin();
    iterator finish = end();
    for (; position != finish; ++position) {
        (*position).second.erase(listener);
    }
}

/* Minigame-facing lifetime owner of the shared event map, queue, and clock. */
struct EventManager : MinigameSingleton<EventManager> {
    EventManager();
    ~EventManager();
};

/* Construct the shared event owner when the minigame event interface starts. */
EventManager::EventManager()
{
    new CEventBroadcaster();
}

/* Release pending events before clearing the active minigame event interface. */
EventManager::~EventManager()
{
    CEventBroadcaster *owner = current_event_owner();
    delete owner;
}

/* On each event update, advance the clock and dispatch then remove due entries. */
void process_event_queue()
{
    CEventBroadcaster *owner=current_event_owner();
    if (owner) {
        float currentTime=framesToSeconds<float>(++owner->tick_count);
        owner->current_time=currentTime;
        EventQueue &queue=owner->queue;
        while (!queue.empty()) {
            const EventEntry &entry=queue.top();
            if (entry.scheduled_time > currentTime) break;
            /* Receivers run before the queued entry is removed. */
            dispatch_event(owner,entry);
            queue.pop();
        }
    }
}

int forwardEvent(unsigned long, const EventPacket&);

/* Deliver a packet to every receiver currently registered for its channel. */
void dispatch_event(CEventBroadcaster *owner, const EventPacket& packet)
{
    CEventBroadcaster::iterator position = owner->find(packet.channel);
    if (position != owner->end()) {
        EventReceivers& receivers = (*position).second;
        if (!receivers.empty()) {
            std::for_each(receivers.begin(), receivers.end(),
                std::bind2nd(std::ptr_fun(forwardEvent), packet));
            return;
        }
    }
    /* Drop the packet with a warning when its channel is absent or has no receivers. */
    std::printf("CEventBroadcaster::WARNING\nlistener not found in ch.%d\n", packet.channel);
}

/* Adapt a receiver's virtual event callback for the channel iteration operation. */
int forwardEvent(unsigned long receiver, const EventPacket& packet)
{
    ((EventCallback*)receiver)->receive(&packet);
    return 0;
}

/* Queue a packet copy for a positive delay, or dispatch immediately otherwise.
 * With no active event owner, the packet is ignored. */
void sendEvent(const EventPacket& packet, float delay)
{
    CEventBroadcaster *owner = current_event_owner();
    if (owner) {
        if (delay > 0.0f) {
            owner->queue.push(EventEntry(packet, owner->current_time + delay));
        } else {
            dispatch_event(owner, packet);
        }
    }
}

/* Receiver interface that registers its address on channels and unregisters on destruction. */
EventListener::EventListener() {}

/* Remove this receiver from every channel before its callback interface is destroyed. */
EventListener::~EventListener()
{
    CEventBroadcaster *owner = current_event_owner();
    if (owner) owner->removeListener((unsigned long)this);
}

/* Register this receiver on a channel after the shared event owner has been created. */
void EventListener::registerChannel(int channel)
{
    CEventBroadcaster *owner = current_event_owner();
    owner != 0 ? (void)0 : __msl_assertion_failed("p != NULL", "event.cpp", 234);
    owner->registerListener(channel, (unsigned long)this);
}

/* Construct an event with a channel, value and context, then apply the requested delay. */
void sendEvent(int channel, unsigned long value, void *context, float delay)
{
    EventPacket packet = {};
    packet.channel = channel;
    packet.type = value;
    packet.context = context;
    sendEvent(packet, delay);
}

/* Construct an event with zero context, then apply the requested delay. */
void sendEvent(int channel, unsigned long value, float delay)
{
    EventPacket packet = {};
    packet.channel = channel;
    packet.type = value;
    sendEvent(packet, delay);
}
