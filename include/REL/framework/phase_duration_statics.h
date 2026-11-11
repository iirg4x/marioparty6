// The introduction, start and finish durations of a minigame, in frames, with the conversion that
// sets them only declared. Each file that includes this keeps its own copy, set by its static
// initialiser. A file that does not also see the conversion's body (frame_time.h or
// frame_time_inline.h) compiles no instance of it, and so keeps no 60.0 for one in its .rodata
// (m648 animation_owners.cpp, m671 text_0B3A8.cpp).
#ifndef REL_FRAMEWORK_PHASE_DURATION_STATICS_H
#define REL_FRAMEWORK_PHASE_DURATION_STATICS_H

template <class Time> Time secondsToFrames(Time seconds);

static long introDelay = secondsToFrames(2L);
static long startDelay = secondsToFrames(3.5);
static long finishDelay = secondsToFrames(3.5);

#endif
