/* Declares m699Dll's shared state; each file defines its own after its code. */
#ifndef M699DLL_H
#define M699DLL_H
#include "dolphin/types.h"
#include "game/object.h"

/* Startup and sequence state (the startup file). */
extern OMOBJMAN *lbl_1_bss_188;
extern s32 lbl_1_bss_184;
extern s32 lbl_1_bss_180;
extern s32 lbl_1_bss_17C;
extern s32 lbl_1_bss_178;
extern s32 lbl_1_bss_16C;
extern s16 lbl_1_bss_15C[4];
extern s32 lbl_1_bss_140;

/* Engine object helpers: the asset slot tables and the sound, camera, fog, light and
 * root objects. */
extern s32 lbl_1_bss_190[80];
extern s32 lbl_1_bss_2D0[80];
extern OMOBJ *lbl_1_bss_410;
extern OMOBJ *lbl_1_bss_414;
extern OMOBJ *lbl_1_bss_418;
extern OMOBJ *lbl_1_bss_41C;
extern OMOBJ *lbl_1_bss_420;
void fn_1_12B8(OMOBJMAN *manager);
void fn_1_14E0(void);
void fn_1_24C4(s32 camera, Vec *direction, Vec *target, float fov, float radius);
void fn_1_2A48(HuVecF *direction);
Vec *fn_1_2A98(void);
float fn_1_4EC8(float start, float end, float factor);
float fn_1_4FE4(float start, float end);
float fn_1_5098(HuVecF *from, HuVecF *to);
float fn_1_5134(Vec *first, Vec *second);
float fn_1_5230(Vec *first, Vec *second);
void fn_1_1210(s32 index, s32 value14C, s32 value144);

#endif
