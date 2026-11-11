#ifndef M654_FRAMEWORK_CALLS_H
#define M654_FRAMEWORK_CALLS_H
void RandomSeed(unsigned long seed);
unsigned long RandomUnsigned();
long RandomInteger(long range);
void sendEvent(int channel, unsigned long value, float delay);
void sendEvent(int channel, unsigned long value, void *context, float delay);
#endif
