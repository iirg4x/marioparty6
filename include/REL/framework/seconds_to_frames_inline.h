// The frame-time conversion as an inline template.
#ifndef REL_FRAMEWORK_SECONDS_TO_FRAMES_INLINE_H
#define REL_FRAMEWORK_SECONDS_TO_FRAMES_INLINE_H

// Converts seconds to frames at sixty frames per second.
template <class Time> inline Time secondsToFrames(Time seconds)
{
    return 60 * seconds;
}

#endif
