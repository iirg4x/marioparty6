// Conversions between seconds and frames at the game's 60 frames per second.
#ifndef REL_FRAMEWORK_FRAME_TIME_H
#define REL_FRAMEWORK_FRAME_TIME_H

template <class Time> Time secondsToFrames(Time seconds)
{
    return 60 * seconds;
}

template <class Time> Time minutesToFrames(Time minutes)
{
    return secondsToFrames(minutes * 60);
}

template <class Time> Time framesToSeconds(Time frames)
{
    return (1.0 / 60.0) * frames;
}

#endif
