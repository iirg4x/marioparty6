#include "REL/m648DLL/time_conversion.h"
#include "dolphin/types.h"
#include "REL/m648DLL/player_root.h"

extern "C" {
s32 lbl_1_bss_5E0 = secondsToFrames(2L);
s32 lbl_1_bss_5E4 = (s32)secondsToFrames(3.5);
s32 lbl_1_bss_5E8 = (s32)secondsToFrames(3.5);
Node *lbl_1_bss_5EC;
PlayerListRoot lbl_1_bss_5FC;
}

#include "REL/m648DLL/time_conversion_body.h"
