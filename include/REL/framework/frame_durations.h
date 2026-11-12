// Three additional durations, expressed in whole frames.
#ifndef REL_FRAMEWORK_FRAME_DURATIONS_H
#define REL_FRAMEWORK_FRAME_DURATIONS_H

#include "REL/framework/frame_time.h"

// These three initialized values are never read.
static int twoSeconds = secondsToFrames(2L);
static int threeAndHalfSecondsA = (int)secondsToFrames(3.5);
static int threeAndHalfSecondsB = (int)secondsToFrames(3.5);

#endif
