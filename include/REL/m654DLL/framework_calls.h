#ifndef M654_FRAMEWORK_CALLS_H
#define M654_FRAMEWORK_CALLS_H
unsigned long RandomUnsigned();
void sendEvent(int channel, unsigned long value, float delay);
void sendEvent(int channel, unsigned long value, void *context, float delay);
#endif
