#ifndef REL_M648_TIME_CONVERSION_BODY_H
#define REL_M648_TIME_CONVERSION_BODY_H
template <class Time> Time secondsToFrames(Time seconds)
{
    return 60 * seconds;
}
#endif
