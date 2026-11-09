#ifndef REL_M681DLL_GLOBAL_STATE_H
#define REL_M681DLL_GLOBAL_STATE_H

#include "dolphin/types.h"

#include "REL/m681DLL/owner_182c.h"

typedef struct M681C0State {
    u8 flag;
    u8 stateBytes[7];
} M681C0State;

#ifdef __cplusplus
extern "C" {
#endif
extern M681C0State lbl_1_bss_C0;
#ifdef __cplusplus
}
#endif

#endif
