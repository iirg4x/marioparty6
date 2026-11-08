#ifndef REL_M622DLL_FRAME_TIME_H
#define REL_M622DLL_FRAME_TIME_H

template <class Time>
Time secondsToFrames(Time seconds)
{
    return 60 * seconds;
}

#endif
