#include "REL/m652DLL/time_conversion.h"
#include "dolphin/types.h"
extern "C" {


s32 lbl_1_bss_58 = secondsToFrames<long>(2);
s32 lbl_1_bss_5C = (s32)secondsToFrames(3.5);
s32 lbl_1_bss_60 = (s32)secondsToFrames(3.5);
}

template<class T> T secondsToFrames(T value) { return 60 * value; }
