#ifndef M639DLL_LIGHTING_TYPES_H
#define M639DLL_LIGHTING_TYPES_H
/* A pair of model node names: the left and the right light. */
struct AnimationResourcePair { char *resources[2]; };
template <class T> inline T squared(const T &value) { return value * value; }
#endif
