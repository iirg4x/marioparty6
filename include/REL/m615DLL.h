#ifndef M615DLL_H
#define M615DLL_H

#include "game/object.h"
#include "humath.h"

void fn_1_26C(void);
void *fn_1_2F0(s32 priority, u32 bytes, void (*update)(OMOBJ *));
void fn_1_390(OMOBJ *object, OMOBJ_FUNC callback);
f32 fn_1_398(f32 farg0);
float fn_1_454(const HuVecF *vec);
void ObjectSetup(void);
void fn_1_9620(OMOBJ *obj);

void fn_1_444(HuVecF *out, f32 x, f32 y, f32 z);
f32 fn_1_574(const HuVecF *v);
f32 fn_1_5A4(const HuVecF *a, const HuVecF *b);
void fn_1_5D4(HuVecF *dst, const HuVecF *a, const HuVecF *b);
void fn_1_60C(HuVecF *dst, const HuVecF *a, const HuVecF *b);
void fn_1_644(HuVecF *dst, const HuVecF *a, const HuVecF *b);
f32 fn_1_67C(HuVecF *dst, const HuVecF *src);
void fn_1_85C(HuVecF *dst, const HuVecF *src, f32 scale);

extern OMOBJMAN *lbl_1_bss_4;
typedef struct M615EffectWork {
    OMOBJ *modelObject;
    u8 unknown_04[4]; /* No code names this field. */
    s16 particles[4];
    HuVecF position;
    f32 rotation;
    f32 scale[4];
    s32 curveIndex;
    HuVecF direction;
    HuVecF velocity;
    HuVecF updatedDirection;
} M615EffectWork;

#endif
