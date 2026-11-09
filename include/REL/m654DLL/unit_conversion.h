#ifndef M654_UNIT_CONVERSION_H
#define M654_UNIT_CONVERSION_H
// Same-game framework conversions: frames at 60 per second to seconds, centimeters to world units.
template<class Time> inline Time framesToSeconds(Time value) { return (1.0 / 60.0) * value; }
static inline float cmToWorld(float centimeters) { return centimeters; }
#endif
