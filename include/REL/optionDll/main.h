#ifndef OPTIONDLL_MAIN_H
#define OPTIONDLL_MAIN_H
#include "dolphin/types.h"
#include "game/object.h"
struct OptionSelectionData;
void ObjectSetup(void);
void fn_1_804(void);
void fn_1_FDC(void);
void fn_1_1044(void);
void fn_1_1084(void);
void fn_1_3AE0(s16 modelId, s16 animNo1, s16 bank1, s16 animNo2, s16 bank2);
s16 fn_1_3318(s16 modelId, struct OptionSelectionData *selection, u32 *messages);
void fn_1_18B8(s16 cameraNo);
void fn_1_18D8(void);
void fn_1_19B8(s32 value);
void fn_1_19E0(void);
void fn_1_3BD8(OMOBJ *obj);
s16 fn_1_1DF0(s16 currentOption);
#endif
