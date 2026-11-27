/* Shared declarations for the m674Dll main code. */
#ifndef REL_M674DLL_MAIN_H
#define REL_M674DLL_MAIN_H

#include "dolphin/types.h"

/* Setup and shared game state. */
s32 fn_1_E18(s32 resourceIndex);
void fn_1_EE4(s32 slotIndex, s32 scoreIndex, s32 characterNo);
s32 fn_1_E20(void);
u32 fn_1_E68(u32 scoreIndex);
void fn_1_F1C(unsigned int playerIndex);
int fn_1_F3C(void);

#endif
