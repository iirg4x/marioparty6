#ifndef M659_CONTEXT_H
#define M659_CONTEXT_H

/* The camera prefix preserves storage whose original field types are unknown. */
typedef struct M659Camera {
    unsigned char unobserved_00[8];
    s32 cameraMask;
    OMOBJ *target;
    HuVecF pos;
    HuVecF targetPos;
} M659Camera;

typedef struct M659Work {
    OMOBJMAN *objman;
    OMOBJ *cameraObj[2];
    HU3D_LIGHTID light;
    s32 unk_10;
    s32 unk_14;
    s32 sound;
    s16 winners[2];
    s16 winnerCount;
    s16 otherPlayer;
} M659Work;

typedef struct M659Viewport {
    float y, x, width, height;
} M659Viewport;

extern M659Work lbl_1_bss_4;
extern M659Work *lbl_1_data_0;
extern M659Viewport lbl_1_data_C[2];
extern const HuVecF lbl_1_rodata_10;
extern const GXColor lbl_1_rodata_6C;
extern const HuVecF lbl_1_rodata_70;
extern const HuVecF lbl_1_rodata_7C;
extern const HuVecF lbl_1_rodata_88;
extern const GXColor lbl_1_rodata_9C;
extern const HuVecF lbl_1_rodata_A0;
extern const HuVecF lbl_1_rodata_AC;
void fn_1_3D0(s16 index, OMOBJ *target);
void fn_1_408(OMOBJ *obj);
void fn_1_48C(OMOBJ *obj);
void fn_1_558(OMOBJ *obj);
void fn_1_4918(HuVecF a, HuVecF b, HuVecF *delta);
double fn_1_493C(HuVecF delta);
double fn_1_4ABC(HuVecF a, HuVecF b);
/* The imported MSL abs provider is unary; this is not the two-argument
 * signature inferred from stale volatile registers by the raw translator. */
int abs(int value);
#endif
