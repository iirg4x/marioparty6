#ifndef REL_M634DLL_EVENT_CALLS_H
#define REL_M634DLL_EVENT_CALLS_H
void sendEvent(int channel, unsigned long value, void *context, float delay);
void sendEvent(int channel, unsigned long value, float delay);
void process_event_queue();
#endif
