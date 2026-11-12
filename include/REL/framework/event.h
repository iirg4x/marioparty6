// Sending events to the channels' listeners (event.cpp).
#ifndef REL_FRAMEWORK_EVENT_H
#define REL_FRAMEWORK_EVENT_H

#include "REL/framework/event_packet.h"

// Delivers a packet to its channel's listeners after `delay` frames.
void sendEvent(const EventPacket &packet, float delay);
void sendEvent(int channel, unsigned long value, void *context, float delay);
void sendEvent(int channel, unsigned long value, float delay);

// Delivers the queued packets whose time has come.
void process_event_queue();

#endif
