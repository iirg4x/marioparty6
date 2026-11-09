#ifndef M681_FRAME_TIME_H
#define M681_FRAME_TIME_H

template<class Time>
Time secondsToFrames(Time seconds)
{
    return Time(60) * seconds;
}

template<class Time>
Time minutesToFrames(Time minutes)
{
    return secondsToFrames(minutes * Time(60));
}

template<class Value>
Value perFrame(Value value)
{
    return Value(1.0 / 60.0) * value;
}
#endif
