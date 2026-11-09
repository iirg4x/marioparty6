#ifndef REL_M652DLL_TIME_CONVERSION_H
#define REL_M652DLL_TIME_CONVERSION_H
template<class T> T secondsToFrames(T value);
template<class T> int fn_1_2178(T value);
template<class T> inline T framesToSeconds(T value) { return (1.0 / 60.0) * value; }
#endif
