#ifndef FRAMEWORK_EVENT_PACKET_H
#define FRAMEWORK_EVENT_PACKET_H

// One event: sendEvent delivers it to the listeners registered on its channel.
struct EventPacket {
    // The channel whose listeners receive the event.
    int channel;
    // The event's value; an EventHandlerManager calls the callbacks registered for it.
    int type;
    // Data passed along with the event; zero when the sender gives none.
    void *context;
};

// The packet as receivers declare it.
typedef const EventPacket EventMessage;

// Anything that can receive an event packet.
struct EventCallback {
    virtual ~EventCallback();
    // Called with each packet delivered to this receiver.
    virtual void receive(const EventPacket *packet) = 0;
};

// A receiver that registers itself on event channels and leaves them all when it is deleted.
struct EventListener : EventCallback {
    // Registers on no channel; registerChannel adds one.
    EventListener();
    // Removes this receiver from every channel it registered on.
    virtual ~EventListener();
    // Adds this receiver to a channel's listeners; the event manager must already exist.
    void registerChannel(int);
};

#endif
