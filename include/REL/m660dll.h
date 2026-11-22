#ifndef M660DLL_H
#define M660DLL_H

#include "REL/m660dll/scene.h"
#include "game/mg/timer.h"
#include "game/mg/actman.h"

void fn_1_3D0(void);
void fn_1_71C(void);
void fn_1_EB8(void);
void fn_1_24C4(void);
void fn_1_2B34(void);
s16 fn_1_2E80(f32 *value, f32 stepCount, f32 start, f32 end);
s16 fn_1_3AA4(s16 modelIndex, s16 lightId);
f32 fn_1_3C54(f32 range);
void fn_1_3CCC(void);
void fn_1_4004(Vec *origin);
void fn_1_42E0(M660Model *origin, s16 effectIndex);
void fn_1_4B38(M660Model *origin);
void fn_1_4F64(Vec *origin, s16 effectIndex);
void fn_1_55DC(s16 objectIndex, s16 variant);
void fn_1_601C(M660Player *player, s16 spatialMode);
void fn_1_6678(void);
void fn_1_66C8(s16 streamSlot, s16 fadeSpeed);
void fn_1_67D4(s16 slot, s16 mode, Vec *position);
void fn_1_6F38(void);
void fn_1_7008(void);
void fn_1_915C(void);
s32 fn_1_A3E4(void);
s32 fn_1_A9B0(void);
s32 fn_1_A9F0(void);
int fn_1_D29C(void);
s32 fn_1_D2E8(void);
s32 fn_1_D328(void);
s32 fn_1_D57C(void);
s32 fn_1_D5A4(void);
void fn_1_E6F4(void);
void fn_1_11F50(void);
s16 fn_1_12030(M660Player *player, s16 advanceState);
void fn_1_142F8(s16 group);
void fn_1_15484(s16 selection);

MGTIMER *fn_1_2FA0(int type,int duration,int end,float x,float y);
void fn_1_3014(MGTIMER *timer);
void fn_1_3068(MGTIMER *timer);
s32 fn_1_6F58(void);
int fn_1_90A8(COL_NARROW_PARAM *first,
              COL_NARROW_PARAM *second);
void fn_1_9128(MGACTOR *actor, int parameter);

s32 fn_1_3040(MGTIMER *timer);

extern s16 lbl_1_bss_2680[4];

#endif
