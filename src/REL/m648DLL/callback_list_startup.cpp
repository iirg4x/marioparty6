#include "REL/m648DLL/time_conversion.h"
#include "dolphin/types.h"
#include "REL/framework/linknode.h"
struct CallbackListSentinel : Node { ~CallbackListSentinel() {} };
s32 lbl_1_bss_A8 = secondsToFrames(2L);
s32 lbl_1_bss_AC = (s32)secondsToFrames(3.5);
s32 lbl_1_bss_B0 = (s32)secondsToFrames(3.5);
CallbackListSentinel lbl_1_bss_C0;

#include "REL/m648DLL/time_conversion_body.h"
