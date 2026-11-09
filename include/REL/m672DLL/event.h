#ifndef M672_EVENT_H
#define M672_EVENT_H

void sendEvent(int channel, unsigned long value, float delay);
void sendEvent(int channel, unsigned long value, void *context, float delay);

#endif
