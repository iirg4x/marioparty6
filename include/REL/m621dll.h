#ifndef _REL_M621DLL_H
#define _REL_M621DLL_H

#include "game/main.h"
#include "game/object.h"
#include "game/mg/seqman.h"

extern OMOBJMAN *lbl_1_bss_0;
extern MGSEQ_PARAM lbl_1_data_0;
extern s32 lbl_1_data_28;

void *fn_1_A0(s32 priority, u32 size, OMOBJ_FUNC callback);
void fn_1_140(OMOBJ *obj, OMOBJ_FUNC callback);
void fn_1_148(void);
void fn_1_2BC(void);
void fn_1_2E0(s16 mode, s16 frameNo);
void fn_1_358(s16 mode, s16 frameNo);
void fn_1_35C(s16 mode, s16 frameNo);
void fn_1_3B8(s16 mode, s16 frameNo);
void fn_1_3BC(s16 mode, s16 frameNo);
void fn_1_410(s16 mode, s16 frameNo);
void fn_1_414(s16 mode, s16 frameNo);
void fn_1_418(s16 mode, s16 frameNo);
void fn_1_41C(s16 mode, s16 frameNo);
s32 fn_1_420(HuVecF *pos, s16 soundId);
s32 fn_1_594(s16 pan, s16 soundId);
void fn_1_5C8(s32 sound);
void fn_1_5F0(OMOBJ *obj);

#endif
