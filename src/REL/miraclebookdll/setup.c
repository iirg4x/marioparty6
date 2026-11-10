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

#include "REL/miraclebookdll/book.h"

typedef void (*VoidFunc)(void);

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

void ObjectSetup(void);

void fn_1_1CC(void);

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
    ObjectSetup();
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
void ObjectSetup(void)
{
    HuVecF lightPos = {0.0f, 1000.0f, 5000.0f};
    HuVecF lightDir = {0.0f, -1.0f, -1.0f};
    GXColor lightColor = {255, 255, 255, 255};
    HU3D_LIGHTID light;

    lbl_1_bss_4 = fn_8003A8BC(200, 8192);
    fn_80069F1C(lbl_1_bss_4);
    light = fn_800286D0(&lightPos, &lightDir, &lightColor);
    Hu3DGLightStaticSet(light, TRUE);
    Hu3DGLightInfinitytSet(light);
    fn_1_2AC(lbl_1_bss_4);
    fn_8000DAC0(fn_1_1CC, 256, 12288, 0, fn_8000DD9C());
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
            fn_80026648();
            fn_80028D70();
            fn_8003A5E4(1, TRUE);
            break;
    }
    HuPrcVSleep();
}
