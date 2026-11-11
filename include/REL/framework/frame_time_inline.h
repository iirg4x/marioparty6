// The earlier version of the framework's frame time header, which m603 and m648 were built
// with: the conversion is an inline template, and the phase durations are declared ahead of its
// body. A static initialiser therefore calls the long and double instances (they are compiled
// after it, at the end of the file), while a conversion in a function is inlined: neither module
// has a float instance, and both compile secondsToFrames(0.2f) in line. The later version is
// frame_time.h (non-inline templates). This header's name, and how the original divided these
// lines between its headers, are not known.
#ifndef REL_FRAMEWORK_FRAME_TIME_INLINE_H
#define REL_FRAMEWORK_FRAME_TIME_INLINE_H

#include "REL/framework/phase_duration_statics.h"

template <class Time> inline Time secondsToFrames(Time seconds)
{
    return 60 * seconds;
}

#endif
