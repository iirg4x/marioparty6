#ifndef FRAMEWORK_EVENT_PACKET_H
#define FRAMEWORK_EVENT_PACKET_H
struct EventPacket { int channel; int type; void *context; };
typedef const EventPacket EventMessage;
struct EventCallback { virtual ~EventCallback(); virtual void receive(const EventPacket *) = 0; };
struct EventListener : EventCallback { EventListener(); virtual ~EventListener(); void registerChannel(int); };
#endif
