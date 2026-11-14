#ifndef REL_M681DLL_GLOBAL_STATE_H
#define REL_M681DLL_GLOBAL_STATE_H

#include "dolphin/types.h"

typedef struct StandSoundState {
    u8 flag;
    u8 stateBytes[7]; // Trailing bytes of unknown purpose.
} StandSoundState;

#ifdef __cplusplus
extern "C" {
#endif
extern StandSoundState lbl_1_bss_C0;
#ifdef __cplusplus
}
#endif

#endif
