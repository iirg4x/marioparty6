#ifndef REL_ENDINGDLL_ENDING_H
#define REL_ENDINGDLL_ENDING_H

#include "game/object.h"
#include "game/window.h"

void fn_1_98(HUWINID window, u32 message, s16 character);
void fn_1_25C(OMOBJ *object);
void fn_1_1C0C(s16 index, s16 motion, float time);
void fn_1_DD14(void);
void fn_1_1160(OMOBJ *object);
void fn_1_3B4C(OMOBJ *object);
void fn_1_3D5C(OMOBJ *object);
void fn_1_36E0(OMOBJ *object);
void fn_1_23D8(OMOBJ *object);
void fn_1_2AEC(OMOBJ *object);
void fn_1_702C(void);
void fn_1_8F80(OMOBJ *object);
void fn_1_A964(void);
void fn_1_B568(void);
void fn_1_BE30(void);
void fn_1_C7DC(void);
void fn_1_D434(void);
void fn_1_7B4(void);
void fn_1_45C0(OMOBJ *object);
void fn_1_4BC8(OMOBJ *object);
float fn_1_DDF8(float start, float end, float time, float duration);
void fn_1_DF60(HuVecF *dest, HuVecF *target, float time);
void fn_1_E0EC(HUSPR_GROUPID groupId, s32 attr);
void fn_1_E16C(HUSPR_GROUPID groupId, s32 attr);
void fn_1_E1EC(s16 display, HuVecF *pos);
void fn_1_E270(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_EAB8(s16 display, HuVecF *pos);
void fn_1_EB54(s16 time);
void fn_1_EBCC(HU3D_MODEL *model, HU3D_PARTICLE *work, Mtx mtx);
void fn_1_F068(s16 index, s16 display, HuVecF *pos);
void fn_1_F11C(s16 index, s16 count);
void fn_1_F1B8(s16 display, HuVecF *pos);
void fn_1_F23C(s16 count);
void fn_1_F2CC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_FDD4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_10628(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_10DEC(s16 modelNo, HuVecF *pos, s16 display);
void fn_1_111B0(s16 display);
void fn_1_11208(s16 index, HuVecF *pos, s16 mode);
void fn_1_112E0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_11714(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_11F34(void);
void fn_1_12838(void);

#endif
