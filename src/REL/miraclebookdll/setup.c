// Miracle Book overlay startup and shared state declarations.
#include "dolphin.h"

#include "game/audio.h"

#include "game/data.h"

#include "game/esprite.h"

#include "game/gamework.h"

#include "game/hu3d.h"

#include "game/hsfex.h"

#include "game/mgdata.h"

#include "game/memory.h"

#include "game/object.h"

#include "game/pad.h"

#include "game/process.h"

#include "game/window.h"

#include "game/wipe.h"

typedef void (*VoidFunc)(void);

typedef struct UnkMiracleBookHook_s {
    f32 textureMatrix00;
    f32 textureMatrix01;
    f32 textureMatrix02;
    f32 textureMatrix10;
    f32 textureMatrix11;
    f32 textureMatrix12;
    GXColor color;
    f32 textureOffsetY;
    f32 textureOffsetSpeed;
    f32 effectFramesRemaining;
} UNK_MIRACLEBOOK_HOOK;

typedef struct MiracleBookVecTable_s {
    HuVecF entries[5];
} MIRACLEBOOK_VEC_TABLE;

extern const VoidFunc _ctors[];

extern const VoidFunc _dtors[];

extern s32 lbl_1_data_0[20];

extern s32 lbl_1_data_50[20][7];

extern s16 lbl_1_data_280[][7];

extern HU3D_PARMAN_PARAM lbl_1_data_5C8;

extern u32 lbl_1_data_750[];

extern char lbl_1_data_758[];

extern char lbl_1_data_778[];

extern OMOBJ *lbl_1_bss_8;

extern void *lbl_1_bss_C;

extern ANIMDATA *lbl_1_bss_10;

extern ANIMDATA *lbl_1_bss_14;

extern UNK_MIRACLEBOOK_HOOK lbl_1_bss_18[2];

extern s16 lbl_1_bss_68[5];

extern OMOBJMAN *lbl_1_bss_74;

extern HuVecF lbl_1_bss_78;

extern f32 lbl_1_bss_84;

extern f32 lbl_1_bss_88;

extern s32 lbl_1_bss_8C;

extern s16 lbl_1_bss_90[3];

extern s32 lbl_1_bss_98[2];

extern s32 lbl_1_bss_A0;

extern s32 lbl_1_bss_A4[2];

extern s32 lbl_1_bss_AC;

extern s32 lbl_1_bss_B0;

extern s32 lbl_1_bss_B4[22];

extern s32 lbl_1_bss_10C;

extern s32 lbl_1_bss_110;

extern s32 lbl_1_bss_114;

extern f32 lbl_1_bss_118;

extern f32 lbl_1_bss_11C;

extern f32 lbl_1_bss_120[4];

extern s32 lbl_1_bss_130;

extern s32 lbl_1_bss_134;

extern s32 lbl_1_bss_138;

extern s32 lbl_1_bss_13C;

extern s32 lbl_1_bss_140;

extern s16 lbl_1_bss_144[20];

extern s16 lbl_1_bss_16C[2][2];

extern s16 lbl_1_bss_174[2];

extern s16 lbl_1_bss_178;

extern s16 lbl_1_bss_17A;

extern s32 lbl_1_bss_17C;

extern s32 lbl_1_bss_180;

extern f32 lbl_1_bss_184[6];

extern f32 lbl_1_bss_19C;

extern s32 lbl_1_bss_1A0;

extern OM_CAMERA_VIEW lbl_1_bss_1A4;

extern s16 lbl_1_bss_1C0[2];

extern s32 lbl_1_bss_1D4;

extern s32 lbl_1_bss_1D8;

extern s32 lbl_1_bss_1C4;

extern f32 lbl_1_bss_1C8[3];

extern s32 lbl_1_bss_1DC;

extern s32 lbl_1_bss_1E0;

extern s32 lbl_1_bss_1E4;

extern u8 lbl_1_bss_1E8[];

extern s16 lbl_1_bss_1FC[2][7];

extern s16 lbl_1_bss_218;

extern s16 lbl_1_bss_21A;

extern s16 lbl_1_bss_21C;

extern s32 lbl_1_bss_220;

extern s32 lbl_1_bss_224;

extern u32 lbl_1_data_6F4[];

extern u32 lbl_1_data_748[];

extern s32 lbl_1_data_398[];

extern f32 lbl_1_data_3E8[];

void fn_1_A0(void);

void fn_1_1CC(void);

void fn_1_2AC(OMOBJMAN *objMan);

s32 fn_1_2FCC(s32 dir);

void fn_1_3154(s32 dir);

void fn_1_3734(s32 cameraMoveDirection);

s32 fn_1_394(void);

void fn_1_434(void);

void fn_1_4CC(void);

void fn_1_A30(void);

void fn_1_3418(void);

void fn_1_34C4(void);

void fn_1_3E40(void);

s32 fn_1_427C(void);

void fn_1_479C(void);

void fn_1_4E94(void);

void fn_1_4EFC(void);

void fn_1_4FD4(void);

void fn_1_5904(void);

void fn_1_5ED0(s32 displaySprites);

void fn_1_60A4(void);

void fn_1_69A8(void);

void fn_1_69F8(void);

void fn_1_6B8C(void);

void fn_1_7158(void);

void fn_1_71E0(s32 messNo);

void fn_1_7324(void);

s32 fn_1_7354(void);

void fn_1_7560(void);

void fn_1_7998(void);

void fn_1_8A0C(s16 layerNo);

void fn_1_8A78(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);

void fn_1_8F10(OMOBJ *obj);

void fn_1_9130(s16 hookNo, HuVecF *pos);

void fn_1_92B4(OMOBJ *obj);

void fn_1_95B8(void);

void fn_1_9718(void);

OMOBJMAN *lbl_1_bss_4;

s32 lbl_1_bss_0;

// Runs on overlay load, invoking registered constructors before initializing the scene.
s32 _prolog(void)
{
    const VoidFunc *ctors = _ctors;

    while (*ctors) {
        (*ctors)();
        ctors++;
    }
    fn_1_A0();
    return 0;
}

// Runs on overlay unload to invoke registered destructors.
void _epilog(void)
{
    const VoidFunc *dtors = _dtors;

    while (*dtors) {
        (*dtors)();
        dtors++;
    }
}

// Called by the overlay prolog to create the scene manager, lighting and entry process.
void fn_1_A0(void)
{
    HuVecF lightPos = {0.0f, 1000.0f, 5000.0f};
    HuVecF lightDir = {0.0f, -1.0f, -1.0f};
    GXColor lightColor = {255, 255, 255, 255};
    HU3D_LIGHTID light;

    lbl_1_bss_4 = omInitObjMan(200, 8192);
    omGameSysInit(lbl_1_bss_4);
    light = Hu3DGLightCreateV(&lightPos, &lightDir, &lightColor);
    Hu3DGLightStaticSet(light, TRUE);
    Hu3DGLightInfinitytSet(light);
    fn_1_2AC(lbl_1_bss_4);
    HuPrcChildCreate(fn_1_1CC, 256, 12288, 0, HuPrcCurrentGet());
    lbl_1_bss_0 = -1;
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
}

// Runs as the overlay child process, advancing entry and exit wipes once per scheduler pass.
void fn_1_1CC(void)
{
    switch (lbl_1_bss_0) {
        case -1:
        case 0:
            if (!WipeCheck()) {
                lbl_1_bss_0 += 2;
            }
            break;
        case 1:
            if (fn_1_394() || omSysExitReq) {
                WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
                lbl_1_bss_0 = 0;
            }
            break;
        case 2:
            fn_1_434();
            HuAudAllStop();
            Hu3DModelAllKill();
            Hu3DLightAllKill();
            omOvlReturnEx(1, 1);
            break;
    }
    HuPrcVSleep();
}
