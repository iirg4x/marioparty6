#include "REL/m654DLL/time_conversion.h"

long fn_1_B80(long value)
{
    return secondsToFrames(value * 60);
}

template<class Time> Time secondsToFrames(Time value)
{
    return value * 60;
}

template long secondsToFrames(long);
