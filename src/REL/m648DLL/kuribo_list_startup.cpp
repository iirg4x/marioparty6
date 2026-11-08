#include "REL/m648DLL/time_conversion.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/framework/linknode.h"
struct KuriboListSentinel : Node { ~KuriboListSentinel() {} };
s32 lbl_1_bss_C8 = secondsToFrames(2L);
s32 lbl_1_bss_CC = (s32)secondsToFrames(3.5);
s32 lbl_1_bss_D0 = (s32)secondsToFrames(3.5);
Vec lbl_1_bss_D4[0x21];
KuriboListSentinel lbl_1_bss_26C;

#include "REL/m648DLL/time_conversion_body.h"
