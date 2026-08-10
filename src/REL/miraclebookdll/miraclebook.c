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
    f32 unk_00;
    f32 unk_04;
    f32 unk_08;
    f32 unk_0C;
    f32 unk_10;
    f32 unk_14;
    GXColor color;
    f32 unk_1C;
    f32 unk_20;
    f32 unk_24;
} UNK_MIRACLEBOOK_HOOK;

typedef struct MiracleBookVecTable_s {
    HuVecF entries[5];
} MIRACLEBOOK_VEC_TABLE;

enum {
    MIRACLEBOOK_SPRITE_PRIORITY = 256,
    MIRACLEBOOK_SPECIAL_SPRITE_PRIORITY = 512,
};

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

extern HuVecF lbl_1_rodata_10;
extern HuVecF lbl_1_rodata_1C;
extern GXColor lbl_1_rodata_28[2];
extern f32 lbl_1_rodata_30;
extern f64 lbl_1_rodata_38;
extern f64 lbl_1_rodata_40;
extern f32 lbl_1_rodata_48;
extern f32 lbl_1_rodata_4C;
extern f32 lbl_1_rodata_50;
extern f32 lbl_1_rodata_54;
extern f32 lbl_1_rodata_58;
extern f32 lbl_1_rodata_5C;
extern f32 lbl_1_rodata_60;
extern f32 lbl_1_rodata_64;
extern f32 lbl_1_rodata_68;
extern f32 lbl_1_rodata_6C;
extern f32 lbl_1_rodata_70;
extern f32 lbl_1_rodata_74;
extern f32 lbl_1_rodata_78;
extern f32 lbl_1_rodata_7C;
extern f32 lbl_1_rodata_80;
extern f32 lbl_1_rodata_84;
extern f32 lbl_1_rodata_88;
extern f32 lbl_1_rodata_8C;
extern f32 lbl_1_rodata_90;

#define MIRACLEBOOK_EARLY_RODATA_EXTERNS() \
    extern HuVecF lbl_1_rodata_10; \
    extern HuVecF lbl_1_rodata_1C; \
    extern GXColor lbl_1_rodata_28[2]; \
    extern f32 lbl_1_rodata_30; \
    extern f64 lbl_1_rodata_38; \
    extern f64 lbl_1_rodata_40; \
    extern f32 lbl_1_rodata_48; \
    extern f32 lbl_1_rodata_4C; \
    extern f32 lbl_1_rodata_50; \
    extern f32 lbl_1_rodata_54; \
    extern f32 lbl_1_rodata_58; \
    extern f32 lbl_1_rodata_5C; \
    extern f32 lbl_1_rodata_60; \
    extern f32 lbl_1_rodata_64; \
    extern f32 lbl_1_rodata_68; \
    extern f32 lbl_1_rodata_6C; \
    extern f32 lbl_1_rodata_70; \
    extern f32 lbl_1_rodata_74; \
    extern f32 lbl_1_rodata_78; \
    extern f32 lbl_1_rodata_7C; \
    extern f32 lbl_1_rodata_80; \
    extern f32 lbl_1_rodata_84; \
    extern f32 lbl_1_rodata_88; \
    extern f32 lbl_1_rodata_8C; \
    extern f32 lbl_1_rodata_90;
extern const HuVecF lbl_1_rodata_A0;
extern const HuVecF lbl_1_rodata_AC;
extern const f32 lbl_1_rodata_B8;
extern const f32 lbl_1_rodata_BC;
extern const f32 lbl_1_rodata_C0;
extern const f32 lbl_1_rodata_C4;
extern const f32 lbl_1_rodata_C8;
extern const f32 lbl_1_rodata_CC;
extern const f32 lbl_1_rodata_D0;
extern const f32 lbl_1_rodata_D4;
extern const f32 lbl_1_rodata_D8;
extern const f32 lbl_1_rodata_DC;
extern const f32 lbl_1_rodata_158;
extern const f32 lbl_1_rodata_15C;
extern const f32 lbl_1_rodata_160;
extern const f32 lbl_1_rodata_164;
extern const f32 lbl_1_rodata_168;
extern const f32 lbl_1_rodata_16C;
extern const f32 lbl_1_rodata_170;
extern const f32 lbl_1_rodata_174;
extern const f32 lbl_1_rodata_178;
extern const f32 lbl_1_rodata_17C;
extern const f32 lbl_1_rodata_180;
extern const f32 lbl_1_rodata_184;
extern const f32 lbl_1_rodata_188;
extern const f32 lbl_1_rodata_18C;
extern const f32 lbl_1_rodata_190;
extern const f32 lbl_1_rodata_194;
extern const f32 lbl_1_rodata_198;
extern const f32 lbl_1_rodata_19C;
extern const f32 lbl_1_rodata_1A0;
extern const f32 lbl_1_rodata_1A4;
extern const f32 lbl_1_rodata_1A8;
extern const f32 lbl_1_rodata_1AC;
extern const f32 lbl_1_rodata_1B0;
extern const f64 lbl_1_rodata_F0;
extern const f64 lbl_1_rodata_F8;
extern const f32 lbl_1_rodata_100;
extern const f32 lbl_1_rodata_104;
extern const f64 lbl_1_rodata_108;
extern const f32 lbl_1_rodata_110;
extern const f64 lbl_1_rodata_118;
extern const f32 lbl_1_rodata_120;
extern const f64 lbl_1_rodata_128;
extern const f64 lbl_1_rodata_130;
extern const f64 lbl_1_rodata_138;
extern const f64 lbl_1_rodata_140;
extern const f64 lbl_1_rodata_148;
extern const f64 lbl_1_rodata_150;
extern const f64 lbl_1_rodata_E0;
extern const f64 lbl_1_rodata_E8;
extern const f64 lbl_1_rodata_98;
extern const f64 lbl_1_rodata_1B8;
extern const f64 lbl_1_rodata_1C0;
extern const f64 lbl_1_rodata_1C8;
extern const f64 lbl_1_rodata_1D0;
extern const f32 lbl_1_rodata_1B4;
extern const f32 lbl_1_rodata_1D8;
extern const f32 lbl_1_rodata_1DC;
extern const f32 lbl_1_rodata_1E0;
extern const f32 lbl_1_rodata_1E4;
extern const f32 lbl_1_rodata_1E8;
extern const f32 lbl_1_rodata_1EC;
extern const f32 lbl_1_rodata_1F0;
extern const f32 lbl_1_rodata_1F4;
extern const f32 lbl_1_rodata_1F8;
extern const f32 lbl_1_rodata_1FC;
extern const HuVecF lbl_1_rodata_200;
extern const f32 lbl_1_rodata_20C;
extern const f32 lbl_1_rodata_210;
extern const f64 lbl_1_rodata_218;
extern const f64 lbl_1_rodata_220;
extern const f32 lbl_1_rodata_228;
extern const f64 lbl_1_rodata_230;
extern const f64 lbl_1_rodata_238;
extern const f64 lbl_1_rodata_240;
extern const f32 lbl_1_rodata_248;
extern const f32 lbl_1_rodata_24C;
extern const f32 lbl_1_rodata_250;
extern const f32 lbl_1_rodata_254;
extern const f32 lbl_1_rodata_258;
extern const f32 lbl_1_rodata_25C;
extern const f32 lbl_1_rodata_260;
extern const f32 lbl_1_rodata_264;
extern const f32 lbl_1_rodata_268;
extern const f32 lbl_1_rodata_26C;
extern const f32 lbl_1_rodata_270;
extern const f32 lbl_1_rodata_274;
extern const f32 lbl_1_rodata_278;
extern const f32 lbl_1_rodata_27C;
extern const GXColor lbl_1_rodata_280;
extern const MIRACLEBOOK_VEC_TABLE lbl_1_rodata_284;

extern s32 lbl_1_data_0[];
extern s32 lbl_1_data_50[][7];
extern s16 lbl_1_data_280[][7];
extern HU3D_PARMAN_PARAM lbl_1_data_5C8;
extern u32 lbl_1_data_750[];
extern char lbl_1_data_758[];
extern char lbl_1_data_778[];

extern s32 lbl_1_bss_0;
extern OMOBJMAN *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_8;
extern void *lbl_1_bss_C;
extern ANIMDATA *lbl_1_bss_10;
extern ANIMDATA *lbl_1_bss_14;
extern UNK_MIRACLEBOOK_HOOK lbl_1_bss_18[2];
extern s16 lbl_1_bss_68[6];
extern OMOBJMAN *lbl_1_bss_74;
extern HuVecF lbl_1_bss_78;
extern f32 lbl_1_bss_84;
extern f32 lbl_1_bss_88;
extern s32 lbl_1_bss_8C;
extern s16 lbl_1_bss_90[4];
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

void fn_1_A0(void);
void fn_1_1CC(void);
void fn_1_2AC(OMOBJMAN *objMan);
s32 fn_1_2FCC(s32 dir);
void fn_1_3154(s32 dir);
s32 fn_1_394(void);
void fn_1_434(void);
void fn_1_4CC(void);
void fn_1_A30(void);
void fn_1_3418(void);
void fn_1_34C4(void);
void fn_1_3734(s32 arg0);
void fn_1_3E40(void);
s32 fn_1_427C(void);
void fn_1_479C(void);
void fn_1_4E94(void);
void fn_1_4EFC(void);
void fn_1_4FD4(void);
void fn_1_5904(void);
void fn_1_5ED0(s32 arg0);
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

void _epilog(void)
{
    const VoidFunc *dtors = _dtors;

    while (*dtors) {
        (*dtors)();
        dtors++;
    }
}

void fn_1_A0(void)
{
    HuVecF lightPos = lbl_1_rodata_10;
    HuVecF lightDir = lbl_1_rodata_1C;
    GXColor lightColor = lbl_1_rodata_28[0];
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

void fn_1_2AC(OMOBJMAN *objMan)
{
    lbl_1_bss_74 = objMan;
    fn_1_34C4();
    HuWinInit(1);
    fn_1_4CC();
    fn_1_479C();
    fn_1_69A8();
    fn_1_7560();
    HuPrcChildCreate(fn_1_60A4, 100, 8192, 0, HuPrcCurrentGet());
    if (MiracleBookEvtNo != 0) {
        lbl_1_bss_224 = 1;
    }
    lbl_1_bss_AC = -1;
    lbl_1_bss_A4[0] = lbl_1_bss_A4[1] = -1;
    lbl_1_bss_A0 = 0;
    lbl_1_bss_98[0] = lbl_1_bss_98[1] = 0;
}

s32 fn_1_2FCC(s32 dir)
{
    s32 result = 0;
    s32 i;

    if (dir > 0) {
        if (lbl_1_bss_1E4 + 1 == lbl_1_bss_1D4) {
            if (lbl_1_bss_1E8[lbl_1_bss_1E4 + 1] != 255) {
                result = 1;
            }
        } else {
            for (i = lbl_1_bss_1E4 + 1; i <= lbl_1_bss_1D4; i++) {
                if (lbl_1_bss_1E8[i] != 255) {
                    result = i - lbl_1_bss_1E4;
                    break;
                }
            }
        }
    } else if (dir < 0) {
        if (lbl_1_bss_1E4 - 1 == lbl_1_bss_1D8) {
            if (lbl_1_bss_1E8[lbl_1_bss_1E4 - 1] != 255) {
                result = -1;
            }
        } else {
            for (i = lbl_1_bss_1E4 - 1; i >= lbl_1_bss_1D8; i--) {
                if (lbl_1_bss_1E8[i] != 255) {
                    result = i - lbl_1_bss_1E4;
                    break;
                }
            }
        }
    }
    return result;
}

void fn_1_3154(s32 dir)
{
    s32 bankFlag;
    s32 bank;
    s32 page;
    s32 total = 0;
    s32 sumIndex;
    s32 itemIndex;

    if (dir == 0) {
        bankFlag = 0;
    } else {
        bankFlag = 1;
    }
    bank = bankFlag;
    page = lbl_1_bss_1E4 + dir;
    if (page > 0) {
        for (sumIndex = 0; sumIndex < page; sumIndex++) {
            total += lbl_1_data_0[sumIndex];
        }
    }
    for (itemIndex = 0; itemIndex < 7; itemIndex++) {
        if (itemIndex >= lbl_1_data_0[page]) {
            lbl_1_bss_1FC[bank][itemIndex] = -1;
        } else {
            lbl_1_bss_1FC[bank][itemIndex] = Hu3DModelCreate(HuDataSelHeapReadNum(
                DATA_miraclebook + total + itemIndex + 8, 268435456, 2));
            Hu3DModelCameraSet(lbl_1_bss_1FC[bank][itemIndex], 1);
            Hu3DModelPosSet(lbl_1_bss_1FC[bank][itemIndex], lbl_1_rodata_30,
                lbl_1_rodata_6C, lbl_1_rodata_30);
            Hu3DModelRotSet(lbl_1_bss_1FC[bank][itemIndex], lbl_1_rodata_30,
                lbl_1_rodata_30, lbl_1_rodata_84);
            if (lbl_1_data_50[page][itemIndex] == 1) {
                Hu3DModelAttrSet(lbl_1_bss_1FC[bank][itemIndex], HU3D_MOTATTR_PAUSE);
            } else if (lbl_1_data_50[page][itemIndex] == 2) {
                Hu3DModelAttrSet(lbl_1_bss_1FC[bank][itemIndex],
                    HU3D_MOTATTR_SHAPE_PAUSE);
            }
            Hu3DModelLayerSet(lbl_1_bss_1FC[bank][itemIndex],
                lbl_1_data_280[page][itemIndex]);
            Hu3DModelAttrSet(lbl_1_bss_1FC[bank][itemIndex], HU3D_ATTR_DISPOFF);
        }
    }
}

s32 fn_1_394(void)
{
    s32 result = 0;

    if (lbl_1_bss_AC == -1) {
        lbl_1_bss_AC = HuAudBGMPlay(94);
    }
    fn_1_A30();
    fn_1_4E94();
    if (fn_1_7354() == 0) {
        result = 1;
    }
    if (lbl_1_bss_17C != 0) {
        result = 1;
    }
    if (result == 1) {
        HuAudSStreamFadeOut(lbl_1_bss_AC, 100);
    }
    return result;
}

void fn_1_434(void)
{
    s32 ovl;

    if (lbl_1_bss_134 > 0) {
        omOvlHisChg(0, 108, 0, 0);
        if (lbl_1_bss_134 == 1) {
            ovl = 110;
        } else {
            ovl = 2;
        }
        omOvlCallEx(ovl, 1, 0, 0);
        MiracleBookEvtNo = lbl_1_bss_134;
    }
}

void fn_1_4CC(void)
{
    s32 i;
    s32 flag;
    s32 value;
    s32 valueFlag;

    lbl_1_bss_21C = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_miraclebook, 0), 268435456, 2));
    Hu3DModelCameraSet(lbl_1_bss_21C, 1);
    Hu3DModelPosSet(lbl_1_bss_21C, lbl_1_rodata_30,
        (f32)(lbl_1_rodata_38 * sin(lbl_1_rodata_40)), lbl_1_rodata_48);
    Hu3DModelRotSet(lbl_1_bss_21C, lbl_1_rodata_4C,
        lbl_1_rodata_30, lbl_1_rodata_30);
    Hu3DModelLayerSet(lbl_1_bss_21C, 0);
    Hu3DModelAttrSet(lbl_1_bss_21C, HU3D_MOTATTR_LOOP);
    Hu3DMotionSpeedSet(lbl_1_bss_21C, lbl_1_rodata_50);
    lbl_1_bss_21A = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_miraclebook, 1), 268435456, 2));
    Hu3DModelCameraSet(lbl_1_bss_21A, 1);
    Hu3DModelPosSet(lbl_1_bss_21A, lbl_1_rodata_54,
        lbl_1_rodata_58, lbl_1_rodata_5C);
    Hu3DModelRotSet(lbl_1_bss_21A, lbl_1_rodata_60,
        lbl_1_rodata_64, lbl_1_rodata_68);
    Hu3DModelLayerSet(lbl_1_bss_21A, 0);
    Hu3DModelAttrSet(lbl_1_bss_21A, HU3D_MOTATTR_PAUSE);
    Hu3DModelClusterAttrSet(lbl_1_bss_21A, 0, HU3D_CLUSTER_ATTR_PAUSE);
    lbl_1_bss_218 = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_miraclebook, 2), 268435456, 2));
    Hu3DModelCameraSet(lbl_1_bss_218, 1);
    Hu3DModelPosSet(lbl_1_bss_218, lbl_1_rodata_54,
        lbl_1_rodata_58, lbl_1_rodata_5C);
    Hu3DModelRotSet(lbl_1_bss_218, lbl_1_rodata_60,
        lbl_1_rodata_64, lbl_1_rodata_68);
    Hu3DModelLayerSet(lbl_1_bss_218, 5);
    Hu3DModelAttrSet(lbl_1_bss_218, HU3D_MOTATTR_PAUSE);
    for (i = 0; i < 2; i++) {
        lbl_1_bss_1C0[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATA_miraclebook + i + 6, 268435456, 2));
        Hu3DModelCameraSet(lbl_1_bss_1C0[i], 1);
        Hu3DModelPosSet(lbl_1_bss_1C0[i], lbl_1_rodata_30,
            lbl_1_rodata_6C, lbl_1_rodata_30);
        Hu3DModelRotSet(lbl_1_bss_1C0[i], lbl_1_rodata_30,
            lbl_1_rodata_30, lbl_1_rodata_30);
        Hu3DModelLayerSet(lbl_1_bss_1C0[i], 0);
        Hu3DModelAttrSet(lbl_1_bss_1C0[i], HU3D_ATTR_DISPOFF);
    }
    for (i = 0; i < 7; i++) {
        lbl_1_bss_1FC[0][i] = -1;
        lbl_1_bss_1FC[1][i] = -1;
    }
    lbl_1_bss_1D8 = 0;
    lbl_1_bss_1D4 = 19;
    lbl_1_bss_224 = 0;
    lbl_1_bss_220 = -1;
    lbl_1_bss_1E0 = 0;
    if (GWMiracleBookFlagGet(0)) {
        flag = 1;
    } else {
        flag = 0;
    }
    lbl_1_bss_1E8[0] = flag;
    lbl_1_bss_130 = lbl_1_bss_1E8[0];
    for (i = 1; i < 20; i++) {
        if (!GWBankFlagGet(i + 10)) {
            value = 255;
        } else {
            if (GWMiracleBookFlagGet(i)) {
                valueFlag = 1;
            } else {
                valueFlag = 0;
            }
            value = valueFlag;
        }
        lbl_1_bss_1E8[i] = value;
        if (lbl_1_bss_1E8[i] == 1) {
            lbl_1_bss_130++;
        }
    }
}

__declspec(section ".rodata") HuVecF lbl_1_rodata_10 = {0.0f, 1000.0f, 5000.0f};
__declspec(section ".rodata") HuVecF lbl_1_rodata_1C = {0.0f, -1.0f, -1.0f};
__declspec(section ".rodata") GXColor lbl_1_rodata_28[2] = {
    {255, 255, 255, 255},
    {0, 0, 0, 0},
};
__declspec(section ".rodata") f32 lbl_1_rodata_30 = 0.0f;
__declspec(section ".rodata") f64 lbl_1_rodata_38 = 2000.0;
__declspec(section ".rodata") f64 lbl_1_rodata_40 = -0.4363323129985824;
__declspec(section ".rodata") f32 lbl_1_rodata_48 = -2000.0f;
__declspec(section ".rodata") f32 lbl_1_rodata_4C = -25.0f;
__declspec(section ".rodata") f32 lbl_1_rodata_50 = 0.04f;
__declspec(section ".rodata") f32 lbl_1_rodata_54 = 100.0f;
__declspec(section ".rodata") f32 lbl_1_rodata_58 = -229.0f;
__declspec(section ".rodata") f32 lbl_1_rodata_5C = -500.0f;
__declspec(section ".rodata") f32 lbl_1_rodata_60 = 125.0f;
__declspec(section ".rodata") f32 lbl_1_rodata_64 = -130.0f;
__declspec(section ".rodata") f32 lbl_1_rodata_68 = -45.0f;
__declspec(section ".rodata") f32 lbl_1_rodata_6C = -65.0f;
__declspec(section ".rodata") f32 lbl_1_rodata_70 = 86.0f;
__declspec(section ".rodata") f32 lbl_1_rodata_74 = 5.0f;
__declspec(section ".rodata") f32 lbl_1_rodata_78 = -54.0f;
__declspec(section ".rodata") f32 lbl_1_rodata_7C = 60.0f;
__declspec(section ".rodata") f32 lbl_1_rodata_80 = 139.0f;
__declspec(section ".rodata") f32 lbl_1_rodata_84 = -90.0f;
__declspec(section ".rodata") f32 lbl_1_rodata_88 = 149.0f;
__declspec(section ".rodata") f32 lbl_1_rodata_8C = 209.0f;
__declspec(section ".rodata") f32 lbl_1_rodata_90 = 90.0f;

void fn_1_3418(void)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    s32 i;

    for (i = 0; i < 7; i++) {
        if (lbl_1_bss_1FC[0][i] != -1) {
            Hu3DModelKill(lbl_1_bss_1FC[0][i]);
        }
        lbl_1_bss_1FC[0][i] = lbl_1_bss_1FC[1][i];
        lbl_1_bss_1FC[1][i] = -1;
    }
}

void fn_1_34C4(void)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    OMOBJ *obj;
    HuVecF center;
    HuVecF rot;
    HuVecF *centerP;
    HuVecF *rotP;

    obj = omAddObjEx(lbl_1_bss_74, 32730, 0, 0, -1, omOutViewMulti);
    obj->work[0] = 2;
    Hu3DCameraCreate(1);
    Hu3DCameraViewportSet(1, lbl_1_rodata_30, lbl_1_rodata_30,
        lbl_1_rodata_B8, lbl_1_rodata_BC, lbl_1_rodata_30, lbl_1_rodata_C0);
    Hu3DCameraPerspectiveSet(1, lbl_1_rodata_C4, lbl_1_rodata_C8,
        lbl_1_rodata_CC, lbl_1_rodata_D0);
    Hu3DCameraCreate(2);
    Hu3DCameraViewportSet(2, lbl_1_rodata_30, lbl_1_rodata_30,
        lbl_1_rodata_B8, lbl_1_rodata_BC, lbl_1_rodata_30, lbl_1_rodata_C0);
    Hu3DCameraPerspectiveSet(2, lbl_1_rodata_C4, lbl_1_rodata_C8,
        lbl_1_rodata_CC, lbl_1_rodata_D0);
    center = lbl_1_rodata_A0;
    centerP = &center;
    lbl_1_bss_1A4.center = *centerP;
    rot = lbl_1_rodata_AC;
    rotP = &rot;
    lbl_1_bss_1A4.rot = *rotP;
    lbl_1_bss_1A4.zoom = lbl_1_rodata_D4;
    omCameraViewSetMulti(1, &lbl_1_bss_1A4);
    omCameraViewSetMulti(2, &lbl_1_bss_1A4);
    lbl_1_bss_1A0 = 0;
    lbl_1_bss_19C = lbl_1_rodata_30;
}

void fn_1_3734(s32 arg0)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()

    f64 zeroForward;
    f64 zeroReverse;

    if (lbl_1_bss_1A0 == 0) {
        lbl_1_bss_1A0 = arg0;
        lbl_1_bss_224 = 8;
        return;
    }

    if (lbl_1_rodata_30 == lbl_1_bss_19C) {
        if (lbl_1_bss_1A0 == 1) {
            lbl_1_bss_1A4.zoom = lbl_1_rodata_D8;
            omCameraViewMoveSimpleMulti(3, &lbl_1_bss_1A4, 50);
        } else if (lbl_1_bss_1A0 == -1) {
            lbl_1_bss_1A4.zoom = lbl_1_rodata_D4;
            omCameraViewMoveSimpleMulti(3, &lbl_1_bss_1A4, 50);
        }
    }

    lbl_1_bss_19C += lbl_1_rodata_DC;
    if (lbl_1_bss_19C > lbl_1_rodata_90) {
        lbl_1_bss_19C = lbl_1_rodata_90;
    }

    if (lbl_1_bss_1A0 == 1) {
        lbl_1_bss_184[1] = (f32) ((lbl_1_rodata_70 + lbl_1_bss_11C) *
            sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8));
        sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8);
        zeroForward = lbl_1_rodata_F0;
        lbl_1_bss_184[2] = (f32)zeroForward;
        lbl_1_bss_184[3] = (f32) (lbl_1_rodata_F8 *
            sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8));
        lbl_1_bss_184[5] = (f32) (lbl_1_bss_118 *
            sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8));

        Hu3DModelPosSet(lbl_1_bss_21A, lbl_1_rodata_30,
            lbl_1_rodata_100 + lbl_1_bss_11C - lbl_1_bss_184[1],
            -lbl_1_bss_184[2]);
        Hu3DModelRotSet(lbl_1_bss_21A,
            lbl_1_rodata_104 - lbl_1_bss_184[3], lbl_1_rodata_30,
            lbl_1_bss_118 - lbl_1_bss_184[5]);
        Hu3DModelPosSet(lbl_1_bss_218, lbl_1_rodata_30,
            lbl_1_rodata_100 + lbl_1_bss_11C - lbl_1_bss_184[1],
            -lbl_1_bss_184[2]);
        Hu3DModelRotSet(lbl_1_bss_218,
            lbl_1_rodata_104 - lbl_1_bss_184[3], lbl_1_rodata_30,
            lbl_1_bss_118 - lbl_1_bss_184[5]);

        if (lbl_1_rodata_90 == lbl_1_bss_19C) {
            lbl_1_bss_224 = 2;
            lbl_1_bss_19C = lbl_1_rodata_30;
        }
    } else if (lbl_1_bss_1A0 == -1) {
        lbl_1_bss_184[1] = (f32) (lbl_1_rodata_108 *
            sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8));
        sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8);
        zeroReverse = lbl_1_rodata_F0;
        lbl_1_bss_184[2] = (f32)zeroReverse;
        lbl_1_bss_184[3] = (f32) (lbl_1_rodata_F8 *
            sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8));

        Hu3DModelPosSet(lbl_1_bss_21A, lbl_1_rodata_30,
            lbl_1_rodata_6C + lbl_1_bss_184[1], lbl_1_bss_184[2]);
        Hu3DModelRotSet(lbl_1_bss_21A, lbl_1_bss_184[3],
            lbl_1_rodata_30, lbl_1_rodata_30);
        Hu3DModelPosSet(lbl_1_bss_218, lbl_1_rodata_30,
            lbl_1_rodata_6C + lbl_1_bss_184[1], lbl_1_bss_184[2]);
        Hu3DModelRotSet(lbl_1_bss_218, lbl_1_bss_184[3],
            lbl_1_rodata_30, lbl_1_rodata_30);

        if (lbl_1_rodata_90 == lbl_1_bss_19C) {
            lbl_1_bss_224 = 2;
            lbl_1_bss_1A0 = 0;
            lbl_1_bss_19C = lbl_1_rodata_30;
            fn_1_71E0(0);
            lbl_1_bss_11C = lbl_1_rodata_30;
            lbl_1_bss_120[2] = lbl_1_bss_120[3] = lbl_1_rodata_30;
        }
    }
}

void fn_1_3E40(void)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    lbl_1_bss_19C += lbl_1_rodata_110;
    if (lbl_1_bss_19C > lbl_1_rodata_90) {
        lbl_1_bss_19C = lbl_1_rodata_90;
    }

    lbl_1_bss_184[0] = (f32) (lbl_1_rodata_118 *
        sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8));
    lbl_1_bss_184[1] = (f32) ((lbl_1_rodata_120 + lbl_1_bss_11C) *
        sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8));
    lbl_1_bss_184[2] = (f32) (lbl_1_rodata_128 *
        sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8));
    lbl_1_bss_184[3] = (f32) (lbl_1_rodata_130 *
        sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8));
    lbl_1_bss_184[4] = (f32) (lbl_1_rodata_138 *
        sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8));
    lbl_1_bss_184[5] = (f32) (lbl_1_rodata_140 *
        sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8));

    Hu3DModelPosSet(lbl_1_bss_21A,
        lbl_1_rodata_54 - lbl_1_bss_184[0],
        lbl_1_rodata_100 + (lbl_1_rodata_120 + lbl_1_bss_11C) - lbl_1_bss_184[1],
        lbl_1_rodata_5C - lbl_1_bss_184[2]);
    Hu3DModelRotSet(lbl_1_bss_21A,
        lbl_1_rodata_60 - lbl_1_bss_184[3],
        lbl_1_rodata_64 - lbl_1_bss_184[4],
        lbl_1_rodata_68 - lbl_1_bss_184[5]);
    Hu3DModelPosSet(lbl_1_bss_218,
        lbl_1_rodata_54 - lbl_1_bss_184[0],
        lbl_1_rodata_100 + (lbl_1_rodata_120 + lbl_1_bss_11C) - lbl_1_bss_184[1],
        lbl_1_rodata_5C - lbl_1_bss_184[2]);
    Hu3DModelRotSet(lbl_1_bss_218,
        lbl_1_rodata_60 - lbl_1_bss_184[3],
        lbl_1_rodata_64 - lbl_1_bss_184[4],
        lbl_1_rodata_68 - lbl_1_bss_184[5]);
}

s32 fn_1_427C(void)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    s32 result = 0;

    lbl_1_bss_19C += lbl_1_rodata_110;
    if (lbl_1_bss_19C > lbl_1_rodata_90) {
        lbl_1_bss_19C = lbl_1_rodata_90;
    }

    lbl_1_bss_184[1] = (f32) (lbl_1_bss_11C *
        (lbl_1_rodata_148 - sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8)));
    lbl_1_bss_184[2] = (f32) (lbl_1_bss_118 *
        (lbl_1_rodata_148 - sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8)));
    lbl_1_bss_184[5] = (f32) (lbl_1_rodata_118 *
        sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8));

    Hu3DModelPosSet(lbl_1_bss_21A, lbl_1_rodata_30,
        lbl_1_rodata_100 + lbl_1_bss_184[1], lbl_1_rodata_30);
    Hu3DModelRotSet(lbl_1_bss_21A, lbl_1_rodata_104,
        lbl_1_rodata_30, lbl_1_bss_184[2]);
    Hu3DModelPosSet(lbl_1_bss_218, lbl_1_rodata_30,
        lbl_1_rodata_100 + lbl_1_bss_184[1], lbl_1_rodata_30);
    Hu3DModelRotSet(lbl_1_bss_218, lbl_1_rodata_104,
        lbl_1_rodata_30, lbl_1_bss_184[2]);

    lbl_1_bss_184[0] = (f32) (lbl_1_rodata_150 *
        sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8));
    if (lbl_1_bss_134 == 1) {
        Hu3DModelPosSet(lbl_1_bss_90[0], lbl_1_rodata_30,
            lbl_1_rodata_70, lbl_1_rodata_74 + lbl_1_bss_184[5]);
        Hu3DModelPosSet(lbl_1_bss_90[2], lbl_1_rodata_30,
            lbl_1_rodata_70, lbl_1_rodata_74 + lbl_1_bss_184[5]);
    } else if (lbl_1_bss_134 == 2) {
        Hu3DModelPosSet(lbl_1_bss_90[1], lbl_1_rodata_30,
            lbl_1_rodata_78, lbl_1_rodata_7C + lbl_1_bss_184[5]);
        Hu3DModelPosSet(lbl_1_bss_90[2], lbl_1_rodata_30,
            lbl_1_rodata_78, lbl_1_rodata_7C + lbl_1_bss_184[5]);
    }
    Hu3DModelRotSet(lbl_1_bss_90[0], lbl_1_rodata_158 + lbl_1_bss_184[0],
        lbl_1_rodata_30, lbl_1_rodata_30);
    Hu3DModelRotSet(lbl_1_bss_90[1], lbl_1_rodata_158 + lbl_1_bss_184[0],
        lbl_1_rodata_30, lbl_1_rodata_30);
    Hu3DModelRotSet(lbl_1_bss_90[2], lbl_1_rodata_158 + lbl_1_bss_184[0],
        lbl_1_rodata_30, lbl_1_rodata_30);

    if (lbl_1_rodata_90 == lbl_1_bss_19C) {
        lbl_1_bss_19C = lbl_1_rodata_30;
        result = 1;
    }
    return result;
}

void fn_1_479C(void)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    s32 i;
    s32 j;
    f32 bankPos;
    f32 itemPos;
    f32 posY;
    f32 posYWork;
    f32 posYForCall;

    lbl_1_bss_17A = espEntry(DATANUM(DATA_miraclebook, 83), 0, 0);
    espDrawNoSet(lbl_1_bss_17A, 0);
    espPosSet(lbl_1_bss_17A, lbl_1_rodata_15C, lbl_1_rodata_160);
    espDispOff(lbl_1_bss_17A);
    espPriSet(lbl_1_bss_17A, MIRACLEBOOK_SPECIAL_SPRITE_PRIORITY);
    espScaleSet(lbl_1_bss_17A, lbl_1_rodata_7C, lbl_1_rodata_164);
    espTPLvlSet(lbl_1_bss_17A, lbl_1_rodata_168);

    lbl_1_bss_178 = espEntry(DATANUM(DATA_miraclebook, 84), 0, 0);
    espDrawNoSet(lbl_1_bss_178, 0);
    espPosSet(lbl_1_bss_178, lbl_1_rodata_16C, lbl_1_rodata_170);
    espScaleSet(lbl_1_bss_178, lbl_1_rodata_174, lbl_1_rodata_C0);
    espTPLvlSet(lbl_1_bss_178, lbl_1_rodata_178);
    espDispOff(lbl_1_bss_178);
    espPriSet(lbl_1_bss_178, MIRACLEBOOK_SPRITE_PRIORITY);

    for (i = 0; i < 2; i++) {
        lbl_1_bss_174[i] = espEntry(DATANUM(DATA_miraclebook, 81), 0, 0);
        espDrawNoSet(lbl_1_bss_174[i], 0);
        espBankSet(lbl_1_bss_174[i], i);
        if (i == 0) {
            bankPos = lbl_1_rodata_17C;
        } else {
            bankPos = lbl_1_rodata_180;
        }
        espPosSet(lbl_1_bss_174[i],
            lbl_1_rodata_C8 + (lbl_1_rodata_C8 + bankPos),
            lbl_1_rodata_184);
        espDispOff(lbl_1_bss_174[i]);
        espPriSet(lbl_1_bss_174[i], MIRACLEBOOK_SPRITE_PRIORITY);

        for (j = 0; j < 2; j++) {
            lbl_1_bss_16C[i][j] =
                espEntry(DATANUM(DATA_miraclebook, 82), 0, 0);
            espDrawNoSet(lbl_1_bss_16C[i][j], 0);
            espBankSet(lbl_1_bss_16C[i][j], j + i * 2);
            if (i == 0) {
                itemPos = lbl_1_rodata_188;
            } else {
                itemPos = lbl_1_rodata_18C;
            }
            espPosSet(lbl_1_bss_16C[i][j],
                lbl_1_rodata_190 + (lbl_1_rodata_C8 + itemPos),
                lbl_1_rodata_194);
            espDispOff(lbl_1_bss_16C[i][j]);
            espPriSet(lbl_1_bss_16C[i][j], MIRACLEBOOK_SPRITE_PRIORITY);
            if (j == 1) {
                espTPLvlSet(lbl_1_bss_16C[i][j], lbl_1_rodata_30);
            }
        }
    }

    for (i = 0; i < 20; i++) {
        lbl_1_bss_144[i] = espEntry(DATANUM(DATA_miraclebook, 80), 0, 0);
        espDrawNoSet(lbl_1_bss_144[i], 0);
        if (i < 5) {
            posY = lbl_1_rodata_198 + lbl_1_rodata_19C * (f32)i;
        } else {
            if (i < 15) {
                posYWork = lbl_1_rodata_1A0 +
                    lbl_1_rodata_19C * (f32)(i - 5);
            } else {
                posYWork = lbl_1_rodata_1A0 +
                    lbl_1_rodata_19C * (f32)(i - 15);
            }
            posY = posYWork;
        }
        posYForCall = posY;
        espPosSet(lbl_1_bss_144[i], lbl_1_rodata_1A4,
            posYForCall - lbl_1_rodata_C8);
        espDispOff(lbl_1_bss_144[i]);
        espPriSet(lbl_1_bss_144[i], MIRACLEBOOK_SPRITE_PRIORITY);
    }

    lbl_1_bss_180 = 0;
    lbl_1_bss_140 = 0;
    lbl_1_bss_13C = 0;
    lbl_1_bss_138 = 0;
    lbl_1_bss_134 = 0;
    lbl_1_bss_120[0] = lbl_1_bss_120[1] = lbl_1_bss_120[2] =
        lbl_1_bss_120[3] = lbl_1_rodata_30;
    lbl_1_bss_11C = lbl_1_rodata_30;
    lbl_1_bss_17C = 0;
}

void fn_1_4E94(void)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    if (lbl_1_bss_224 == 2) {
        if (lbl_1_bss_1A0 == 0) {
            fn_1_5ED0(1);
            fn_1_4FD4();
            fn_1_5904();
            fn_1_6B8C();
        }
    } else {
        fn_1_5ED0(0);
        fn_1_7158();
    }
}

void fn_1_4EFC(void)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    s32 i;
    s32 j;

    espKill(lbl_1_bss_17A);
    espKill(lbl_1_bss_178);
    for (i = 0; i < 2; i++) {
        espKill(lbl_1_bss_174[i]);
        for (j = 0; j < 2; j++) {
            espKill(lbl_1_bss_16C[i][j]);
        }
    }
    for (i = 0; i < 20; i++) {
        espKill(lbl_1_bss_144[i]);
    }
}

void fn_1_4FD4(void)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    s32 i;
    s32 start;
    s32 end;
    s32 limit;
    s32 upLimit;
    s32 downLimit;
    f32 posY;

    if (lbl_1_bss_180 == 0) {
        Hu3DModelAttrReset(lbl_1_bss_90[0], HU3D_ATTR_DISPOFF);
        if (lbl_1_bss_138 == -1 || lbl_1_bss_138 == 5) {
            espDispOff(lbl_1_bss_178);
        }
        lbl_1_bss_1E4 = 0;
        if (HuPadDStkRep[0] & PAD_BUTTON_UP) {
            HuAudFXPlay(1155);
            lbl_1_bss_138--;
            if (lbl_1_bss_138 < -1) {
                lbl_1_bss_138 = 4;
            }
            if (lbl_1_bss_138 == -1) {
                espDispOff(lbl_1_bss_178);
                Hu3DModelPosSet(lbl_1_bss_90[2], lbl_1_rodata_30,
                    lbl_1_rodata_70, lbl_1_rodata_74);
                Hu3DModelAttrReset(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
                lbl_1_bss_134 = -1;
            } else {
                espDispOn(lbl_1_bss_178);
                Hu3DModelAttrSet(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
                lbl_1_bss_134 = 0;
            }
        } else if (HuPadDStkRep[0] & PAD_BUTTON_DOWN) {
            HuAudFXPlay(1155);
            lbl_1_bss_138++;
            if (lbl_1_bss_138 > 5) {
                lbl_1_bss_138 = 0;
            }
            if (lbl_1_bss_138 == 5) {
                espDispOff(lbl_1_bss_178);
                Hu3DModelPosSet(lbl_1_bss_90[2], lbl_1_rodata_30,
                    lbl_1_rodata_70, lbl_1_rodata_74);
                Hu3DModelAttrReset(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
                lbl_1_bss_134 = -1;
            } else {
                espDispOn(lbl_1_bss_178);
                Hu3DModelAttrSet(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
                lbl_1_bss_134 = 0;
            }
        }
        posY = lbl_1_rodata_1A8 + lbl_1_rodata_19C *
            (f32)lbl_1_bss_138 - lbl_1_rodata_C0;
        start = 0;
        end = 5;
        lbl_1_bss_1E4 += lbl_1_bss_138;
    } else if (lbl_1_bss_180 == 1) {
        lbl_1_bss_1E4 = 5;
        if (HuPadDStkRep[0] & PAD_BUTTON_UP) {
            HuAudFXPlay(1155);
            lbl_1_bss_138--;
            if (lbl_1_bss_138 < 0) {
                lbl_1_bss_138 = 9;
            }
        } else if (HuPadDStkRep[0] & PAD_BUTTON_DOWN) {
            HuAudFXPlay(1155);
            lbl_1_bss_138++;
            if (lbl_1_bss_138 > 9) {
                lbl_1_bss_138 = 0;
            }
        }
        posY = lbl_1_rodata_1AC + lbl_1_rodata_19C *
            (f32)lbl_1_bss_138 - lbl_1_rodata_C0;
        start = 5;
        end = 15;
        lbl_1_bss_1E4 += lbl_1_bss_138;
        Hu3DModelAttrSet(lbl_1_bss_90[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_90[1], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
        lbl_1_bss_134 = 0;
    } else {
        Hu3DModelAttrReset(lbl_1_bss_90[1], HU3D_ATTR_DISPOFF);
        if (lbl_1_bss_138 == -1 || lbl_1_bss_138 == 5) {
            espDispOff(lbl_1_bss_178);
        }
        lbl_1_bss_1E4 = 15;
        if (HuPadDStkRep[0] & PAD_BUTTON_UP) {
            HuAudFXPlay(1155);
            if (lbl_1_bss_130 == 20) {
                upLimit = -1;
            } else {
                upLimit = 0;
            }
            limit = upLimit;
            lbl_1_bss_138--;
            if (lbl_1_bss_138 < limit) {
                lbl_1_bss_138 = 4;
            }
            if (lbl_1_bss_138 == -1) {
                espDispOff(lbl_1_bss_178);
                Hu3DModelPosSet(lbl_1_bss_90[2], lbl_1_rodata_30,
                    lbl_1_rodata_78, lbl_1_rodata_7C);
                Hu3DModelAttrReset(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
                lbl_1_bss_134 = -2;
            } else {
                espDispOn(lbl_1_bss_178);
                Hu3DModelAttrSet(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
                lbl_1_bss_134 = 0;
            }
        } else if (HuPadDStkRep[0] & PAD_BUTTON_DOWN) {
            HuAudFXPlay(1155);
            if (lbl_1_bss_130 == 20) {
                downLimit = 5;
            } else {
                downLimit = 4;
            }
            limit = downLimit;
            lbl_1_bss_138++;
            if (lbl_1_bss_138 > limit) {
                lbl_1_bss_138 = 0;
            }
            if (lbl_1_bss_138 == 5) {
                espDispOff(lbl_1_bss_178);
                Hu3DModelPosSet(lbl_1_bss_90[2], lbl_1_rodata_30,
                    lbl_1_rodata_78, lbl_1_rodata_7C);
                Hu3DModelAttrReset(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
                lbl_1_bss_134 = -2;
            } else {
                espDispOn(lbl_1_bss_178);
                Hu3DModelAttrSet(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
                lbl_1_bss_134 = 0;
            }
        }
        posY = lbl_1_rodata_1AC + lbl_1_rodata_19C *
            (f32)lbl_1_bss_138 - lbl_1_rodata_C0;
        start = 15;
        end = 20;
        lbl_1_bss_1E4 += lbl_1_bss_138;
    }

    espPosSet(lbl_1_bss_178, lbl_1_rodata_16C,
        posY - lbl_1_rodata_C8);
    for (i = 0; i < 20; i++) {
        if (i >= start && i < end && lbl_1_bss_1E8[i] == 0) {
            espDispOn(lbl_1_bss_144[i]);
        } else {
            espDispOff(lbl_1_bss_144[i]);
        }
    }
}

void fn_1_5904(void)
{
    s32 timerMax = 30;
    f32 posY;

    if (lbl_1_bss_13C == 0) {
        if ((HuPadBtnDown[0] & PAD_TRIGGER_R) && lbl_1_bss_180 != 2) {
            lbl_1_bss_140 = timerMax;
            lbl_1_bss_13C = 1;
        } else if ((HuPadBtnDown[0] & PAD_TRIGGER_L) && lbl_1_bss_180 != 0) {
            lbl_1_bss_140 = timerMax;
            lbl_1_bss_13C = -1;
        }
        if (lbl_1_bss_13C == 1) {
            espTPLvlSet(lbl_1_bss_16C[1][0], lbl_1_rodata_30);
            espTPLvlSet(lbl_1_bss_16C[1][1], lbl_1_rodata_C0);
        } else if (lbl_1_bss_13C == -1) {
            espTPLvlSet(lbl_1_bss_16C[0][0], lbl_1_rodata_30);
            espTPLvlSet(lbl_1_bss_16C[0][1], lbl_1_rodata_C0);
        }
        if (lbl_1_bss_13C != 0) {
            HuAudFXPlay(1162);
        }
    } else {
        if (lbl_1_bss_140 == timerMax / 2) {
            if (lbl_1_bss_13C == 1) {
                espTPLvlSet(lbl_1_bss_16C[1][0], lbl_1_rodata_C0);
                espTPLvlSet(lbl_1_bss_16C[1][1], lbl_1_rodata_30);
            } else if (lbl_1_bss_13C == -1) {
                espTPLvlSet(lbl_1_bss_16C[0][0], lbl_1_rodata_C0);
                espTPLvlSet(lbl_1_bss_16C[0][1], lbl_1_rodata_30);
            }
        }
        if (--lbl_1_bss_140 == 0) {
            if (lbl_1_bss_13C == 1) {
                lbl_1_bss_180++;
                if (lbl_1_bss_180 > 2) {
                    lbl_1_bss_180 = 2;
                }
            } else if (lbl_1_bss_13C == -1) {
                lbl_1_bss_180--;
                if (lbl_1_bss_180 < 0) {
                    lbl_1_bss_180 = 0;
                }
            }
            lbl_1_bss_13C = 0;
            if (lbl_1_bss_180 == 0) {
                posY = lbl_1_rodata_198;
            } else {
                posY = lbl_1_rodata_1A0;
            }
            espPosSet(lbl_1_bss_178, lbl_1_rodata_16C, posY - lbl_1_rodata_C8);
            lbl_1_bss_138 = 0;
        }
    }
    if (lbl_1_bss_180 == 0) {
        espColorSet(lbl_1_bss_174[0], 128, 128, 128);
        espColorSet(lbl_1_bss_174[1], 255, 255, 255);
        espColorSet(lbl_1_bss_16C[0][0], 128, 128, 128);
        espColorSet(lbl_1_bss_16C[0][1], 128, 128, 128);
        espColorSet(lbl_1_bss_16C[1][0], 255, 255, 255);
        espColorSet(lbl_1_bss_16C[1][1], 255, 255, 255);
    } else if (lbl_1_bss_180 == 1) {
        espColorSet(lbl_1_bss_174[0], 255, 255, 255);
        espColorSet(lbl_1_bss_174[1], 255, 255, 255);
        espColorSet(lbl_1_bss_16C[0][0], 255, 255, 255);
        espColorSet(lbl_1_bss_16C[0][1], 255, 255, 255);
        espColorSet(lbl_1_bss_16C[1][0], 255, 255, 255);
        espColorSet(lbl_1_bss_16C[1][1], 255, 255, 255);
    } else {
        espColorSet(lbl_1_bss_174[0], 255, 255, 255);
        espColorSet(lbl_1_bss_174[1], 128, 128, 128);
        espColorSet(lbl_1_bss_16C[0][0], 255, 255, 255);
        espColorSet(lbl_1_bss_16C[0][1], 255, 255, 255);
        espColorSet(lbl_1_bss_16C[1][0], 128, 128, 128);
        espColorSet(lbl_1_bss_16C[1][1], 128, 128, 128);
    }
}

void fn_1_5ED0(s32 dispF)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    s32 i;
    s32 j;

    if (dispF != 0) {
        espDispOn(lbl_1_bss_17A);
        espDispOn(lbl_1_bss_178);
        for (i = 0; i < 2; i++) {
            espDispOn(lbl_1_bss_174[i]);
            for (j = 0; j < 2; j++) {
                espDispOn(lbl_1_bss_16C[i][j]);
            }
        }
        for (i = 0; i < 20; i++) {
            espDispOn(lbl_1_bss_144[i]);
        }
    } else {
        espDispOff(lbl_1_bss_17A);
        espDispOff(lbl_1_bss_178);
        for (i = 0; i < 2; i++) {
            espDispOff(lbl_1_bss_174[i]);
            for (j = 0; j < 2; j++) {
                espDispOff(lbl_1_bss_16C[i][j]);
            }
        }
        for (i = 0; i < 20; i++) {
            espDispOff(lbl_1_bss_144[i]);
        }
        if (lbl_1_bss_224 != 9) {
            Hu3DModelAttrSet(lbl_1_bss_90[0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_90[1], HU3D_ATTR_DISPOFF);
        }
    }
}

void fn_1_60A4(void)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    s32 i;

    if (lbl_1_bss_224 == 0) {
        lbl_1_bss_120[2] += lbl_1_rodata_1B0;
        if (lbl_1_bss_120[2] > lbl_1_rodata_1B4) {
            lbl_1_bss_120[2] -= lbl_1_rodata_1B4;
        }
        lbl_1_bss_11C = (f32) (lbl_1_rodata_1B8 *
            sin(lbl_1_rodata_E0 * lbl_1_bss_120[2] / lbl_1_rodata_E8));
        Hu3DModelPosSet(lbl_1_bss_21A, lbl_1_rodata_54,
            lbl_1_rodata_100 + (lbl_1_rodata_120 + lbl_1_bss_11C),
            lbl_1_rodata_5C);
        Hu3DModelPosSet(lbl_1_bss_218, lbl_1_rodata_54,
            lbl_1_rodata_100 + (lbl_1_rodata_120 + lbl_1_bss_11C),
            lbl_1_rodata_5C);
    }

    if (lbl_1_bss_224 == 2 && lbl_1_bss_1A0 == 0) {
        lbl_1_bss_120[0] += lbl_1_rodata_74;
        if (lbl_1_bss_120[0] > lbl_1_rodata_1B4) {
            lbl_1_bss_120[0] -= lbl_1_rodata_1B4;
        }
        for (i = 0; i < 20; i++) {
            espZRotSet(lbl_1_bss_144[i], (f32) (lbl_1_rodata_1B8 *
                sin(lbl_1_rodata_E0 * lbl_1_bss_120[0] / lbl_1_rodata_E8)));
        }

        if (lbl_1_bss_134 == 0) {
            lbl_1_bss_120[1] += lbl_1_rodata_1B0;
            if (lbl_1_bss_120[1] > lbl_1_rodata_1B4) {
                lbl_1_bss_120[1] -= lbl_1_rodata_1B4;
            }
            if (lbl_1_bss_180 == 2 && lbl_1_bss_130 < 20) {
                lbl_1_bss_120[1] = lbl_1_rodata_30;
            }
            Hu3DModelRotSet(lbl_1_bss_90[0], lbl_1_rodata_158,
                lbl_1_rodata_30, (f32) (lbl_1_rodata_1C0 *
                    sin(lbl_1_rodata_E0 * lbl_1_bss_120[1] / lbl_1_rodata_E8)));
            Hu3DModelRotSet(lbl_1_bss_90[1], lbl_1_rodata_158,
                lbl_1_rodata_30, (f32) (lbl_1_rodata_1C0 *
                    sin(lbl_1_rodata_E0 * lbl_1_bss_120[1] / lbl_1_rodata_E8)));
            Hu3DModelRotSet(lbl_1_bss_90[2], lbl_1_rodata_158,
                lbl_1_rodata_30, (f32) (lbl_1_rodata_1C0 *
                    sin(lbl_1_rodata_E0 * lbl_1_bss_120[1] / lbl_1_rodata_E8)));
            lbl_1_bss_84 = lbl_1_rodata_C0;
            Hu3DModelScaleSet(lbl_1_bss_90[0], lbl_1_bss_84,
                lbl_1_bss_84, lbl_1_rodata_C0);
            Hu3DModelScaleSet(lbl_1_bss_90[1], lbl_1_bss_84,
                lbl_1_bss_84, lbl_1_rodata_C0);
            Hu3DModelScaleSet(lbl_1_bss_90[2], lbl_1_bss_84,
                lbl_1_bss_84, lbl_1_rodata_C0);
        } else {
            lbl_1_bss_120[1] = lbl_1_rodata_30;
            Hu3DModelRotSet(lbl_1_bss_90[0], lbl_1_rodata_158,
                lbl_1_rodata_30, lbl_1_rodata_30);
            Hu3DModelRotSet(lbl_1_bss_90[1], lbl_1_rodata_158,
                lbl_1_rodata_30, lbl_1_rodata_30);
            Hu3DModelRotSet(lbl_1_bss_90[2], lbl_1_rodata_158,
                lbl_1_rodata_30, lbl_1_rodata_30);
            lbl_1_bss_84 = lbl_1_rodata_D0;
            Hu3DModelScaleSet(lbl_1_bss_90[0], lbl_1_bss_84,
                lbl_1_bss_84, lbl_1_rodata_C0);
            Hu3DModelScaleSet(lbl_1_bss_90[1], lbl_1_bss_84,
                lbl_1_bss_84, lbl_1_rodata_C0);
            Hu3DModelScaleSet(lbl_1_bss_90[2], lbl_1_bss_84,
                lbl_1_bss_84, lbl_1_rodata_C0);
        }

        lbl_1_bss_120[2] += lbl_1_rodata_1B0;
        if (lbl_1_bss_120[2] > lbl_1_rodata_1B4) {
            lbl_1_bss_120[2] -= lbl_1_rodata_1B4;
        }
        lbl_1_bss_11C = (f32) (lbl_1_rodata_1C8 *
            sin(lbl_1_rodata_E0 * lbl_1_bss_120[2] / lbl_1_rodata_E8));
        Hu3DModelPosSet(lbl_1_bss_21A, lbl_1_rodata_30,
            lbl_1_rodata_100 + lbl_1_bss_11C, lbl_1_rodata_30);
        Hu3DModelPosSet(lbl_1_bss_218, lbl_1_rodata_30,
            lbl_1_rodata_100 + lbl_1_bss_11C, lbl_1_rodata_30);

        lbl_1_bss_120[3] += lbl_1_rodata_C0;
        if (lbl_1_bss_120[3] > lbl_1_rodata_1B4) {
            lbl_1_bss_120[3] -= lbl_1_rodata_1B4;
        }
        lbl_1_bss_118 = (f32) (lbl_1_rodata_1D0 *
            sin(lbl_1_rodata_E0 * lbl_1_bss_120[3] / lbl_1_rodata_E8));
        Hu3DModelRotSet(lbl_1_bss_21A, lbl_1_rodata_104,
            lbl_1_rodata_30, lbl_1_bss_118);
        Hu3DModelRotSet(lbl_1_bss_218, lbl_1_rodata_104,
            lbl_1_rodata_30, lbl_1_bss_118);
    }
    HuPrcVSleep();
}

void fn_1_69A8(void)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    s32 i;

    for (i = 0; i < 20; i++) {
        lbl_1_bss_B4[i] = -1;
    }
    lbl_1_bss_B0 = 0;
}

void fn_1_69F8(void)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    HuVec2f size;
    f32 posY;
    HUWIN *win;

    HuWinMesMaxSizeGet(1, &size, 524311);
    size.x = lbl_1_rodata_1D8;
    posY = lbl_1_rodata_1B4;
    lbl_1_bss_114 = HuWinExCreateFrame(lbl_1_rodata_1DC, posY, (s16)size.x,
        (s16)size.y, -1, 0);
    HuWinAttrSet((s16)lbl_1_bss_114, HUWIN_ATTR_ALIGN_CENTER);
    HuWinExOpen((s16)lbl_1_bss_114);
    HuWinPriSet((s16)lbl_1_bss_114, 0);
    HuWinMesSpeedSet((s16)lbl_1_bss_114, 1);
    HuWinMesSet((s16)lbl_1_bss_114, 524311);
    win = &winData[lbl_1_bss_114];
    win->padMask = 1;
    win->disablePlayer = 14;
    HuWinMesWait((s16)lbl_1_bss_114);
    HuWinExClose((s16)lbl_1_bss_114);
    HuWinExKill((s16)lbl_1_bss_114);
}

void fn_1_6B8C(void)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    HuVec2f size;
    f32 posX;
    f32 posY;
    s32 start;
    s32 end;
    s32 i;
    u32 messNo;

    fn_1_7158();
    if (lbl_1_bss_180 == 0) {
        start = 0;
        end = 5;
        for (i = start; i < end; i++) {
            messNo = (lbl_1_bss_1E8[i] == 255) ?
                lbl_1_data_6F4[0] : lbl_1_data_6F4[i + 1];
            HuWinMesMaxSizeGet(1, &size, messNo);
            posX = lbl_1_rodata_1E0;
            posY = lbl_1_rodata_1E4 +
                lbl_1_rodata_19C * (f32)(i - start) -
                lbl_1_rodata_1E8 - lbl_1_rodata_C8;
            lbl_1_bss_B4[i] = HuWinCreate(posX, posY,
                (s16)size.x, (s16)size.y, 0);
            HuWinPriSet((s16)lbl_1_bss_B4[i], 0);
            HuWinBGTPLvlSet((s16)lbl_1_bss_B4[i], lbl_1_rodata_30);
            HuWinMesSpeedSet((s16)lbl_1_bss_B4[i], 0);
            HuWinMesSet((s16)lbl_1_bss_B4[i], messNo);
        }
        messNo = lbl_1_data_748[0];
        HuWinMesMaxSizeGet(1, &size, messNo);
        posX = lbl_1_rodata_1EC;
        posY = lbl_1_rodata_1F0;
        lbl_1_bss_B4[20] = HuWinCreate(posX, posY,
            (s16)size.x, (s16)size.y, 0);
        HuWinPriSet((s16)lbl_1_bss_B4[20], 0);
        HuWinBGTPLvlSet((s16)lbl_1_bss_B4[20], lbl_1_rodata_30);
        HuWinMesSpeedSet((s16)lbl_1_bss_B4[20], 0);
        HuWinMesSet((s16)lbl_1_bss_B4[20], messNo);
    } else {
        if (lbl_1_bss_180 == 1) {
            start = 5;
            end = 15;
        } else {
            start = 15;
            end = 20;
        }
        for (i = start; i < end; i++) {
            messNo = (lbl_1_bss_1E8[i] == 255) ?
                lbl_1_data_6F4[0] : lbl_1_data_6F4[i + 1];
            HuWinMesMaxSizeGet(1, &size, messNo);
            posX = lbl_1_rodata_1E0;
            posY = lbl_1_rodata_1F4 +
                lbl_1_rodata_19C * (f32)(i - start) -
                lbl_1_rodata_1E8 - lbl_1_rodata_C8;
            lbl_1_bss_B4[i] = HuWinCreate(posX, posY,
                (s16)size.x, (s16)size.y, 0);
            HuWinPriSet((s16)lbl_1_bss_B4[i], 0);
            HuWinBGTPLvlSet((s16)lbl_1_bss_B4[i], lbl_1_rodata_30);
            HuWinMesSpeedSet((s16)lbl_1_bss_B4[i], 0);
            HuWinMesSet((s16)lbl_1_bss_B4[i], messNo);
        }
        if (lbl_1_bss_180 == 2) {
            messNo = lbl_1_data_748[1];
            HuWinMesMaxSizeGet(1, &size, messNo);
            posX = lbl_1_rodata_1EC;
            posY = lbl_1_rodata_1F8;
            lbl_1_bss_B4[21] = HuWinCreate(posX, posY,
                (s16)size.x, (s16)size.y, 0);
            HuWinPriSet((s16)lbl_1_bss_B4[21], 0);
            HuWinBGTPLvlSet((s16)lbl_1_bss_B4[21], lbl_1_rodata_30);
            HuWinMesSpeedSet((s16)lbl_1_bss_B4[21], 0);
            HuWinMesSet((s16)lbl_1_bss_B4[21], messNo);
        }
    }
}

void fn_1_7158(void)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    s32 i;

    for (i = 0; i < 22; i++) {
        if (lbl_1_bss_B4[i] != -1) {
            HuWinKill((s16)lbl_1_bss_B4[i]);
            lbl_1_bss_B4[i] = -1;
        }
    }
}

void fn_1_71E0(s32 messNo)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    HuVec2f size;
    f32 posX;
    f32 posY;

    HuWinMesMaxSizeGet(1, &size, lbl_1_data_750[messNo]);
    posX = lbl_1_rodata_1DC;
    posY = lbl_1_rodata_1FC - size.y;
    lbl_1_bss_110 = HuWinCreate(posX, posY, (s16)size.x, (s16)size.y, 0);
    HuWinPriSet((s16)lbl_1_bss_110, 0);
    HuWinBGTPLvlSet((s16)lbl_1_bss_110, lbl_1_rodata_30);
    HuWinMesSpeedSet((s16)lbl_1_bss_110, 0);
    HuWinMesSet((s16)lbl_1_bss_110, lbl_1_data_750[messNo]);
}

s32 fn_1_7354(void)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    HuVec2f size;
    f32 posX;
    f32 posY;
    HUWIN *win;
    s32 result = -1;

    if (((lbl_1_bss_224 == 2 && lbl_1_bss_13C == 0) ||
            (lbl_1_bss_224 == 4 && lbl_1_bss_1DC == 0)) &&
        (HuPadBtnDown[0] & PAD_BUTTON_X)) {
        HuAudFXPlay(3);
        HuWinMesMaxSizeGet(1, &size, 524312);
        posX = lbl_1_rodata_1DC;
        posY = lbl_1_rodata_1EC - size.y;
        lbl_1_bss_10C = HuWinCreate(posX, posY, (s16)size.x, (s16)size.y, 0);
        HuWinAttrSet((s16)lbl_1_bss_10C, HUWIN_ATTR_ALIGN_CENTER);
        HuWinPriSet((s16)lbl_1_bss_10C, 0);
        HuWinMesSpeedSet((s16)lbl_1_bss_10C, 0);
        HuWinMesSet((s16)lbl_1_bss_10C, 524312);
        win = &winData[lbl_1_bss_10C];
        win->padMask = 1;
        win->disablePlayer = 14;
        result = (s16)HuWinChoiceGet((s16)lbl_1_bss_10C, -1);
        HuWinKill((s16)lbl_1_bss_10C);
        lbl_1_bss_B0 = 1;
    }
    return result;
}

void fn_1_7560(void)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    ANIMDATA *anim;
    s16 i;

    omAddObjEx(lbl_1_bss_74, 4096, 16, 16, -1, fn_1_92B4);
    Hu3DParManInit();
    lbl_1_bss_90[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_miraclebook, 3), 268435456, 2));
    Hu3DModelCameraSet(lbl_1_bss_90[0], 1);
    Hu3DModelPosSet(lbl_1_bss_90[0], lbl_1_rodata_30,
        lbl_1_rodata_70, lbl_1_rodata_74);
    Hu3DModelRotSet(lbl_1_bss_90[0], lbl_1_rodata_158,
        lbl_1_rodata_30, lbl_1_rodata_30);
    Hu3DModelLayerSet(lbl_1_bss_90[0], 5);
    Hu3DModelAttrSet(lbl_1_bss_90[0], HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(lbl_1_bss_90[0], HU3D_ATTR_DISPOFF);
    lbl_1_bss_90[1] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_miraclebook, 4), 268435456, 2));
    Hu3DModelCameraSet(lbl_1_bss_90[1], 1);
    Hu3DModelPosSet(lbl_1_bss_90[1], lbl_1_rodata_30,
        lbl_1_rodata_78, lbl_1_rodata_7C);
    Hu3DModelRotSet(lbl_1_bss_90[1], lbl_1_rodata_158,
        lbl_1_rodata_30, lbl_1_rodata_30);
    Hu3DModelLayerSet(lbl_1_bss_90[1], 5);
    Hu3DModelAttrSet(lbl_1_bss_90[1], HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(lbl_1_bss_90[1], HU3D_ATTR_DISPOFF);
    lbl_1_bss_90[2] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_miraclebook, 5), 268435456, 2));
    Hu3DModelCameraSet(lbl_1_bss_90[2], 1);
    Hu3DModelPosSet(lbl_1_bss_90[2], lbl_1_rodata_30,
        lbl_1_rodata_70, lbl_1_rodata_74);
    Hu3DModelRotSet(lbl_1_bss_90[2], lbl_1_rodata_158,
        lbl_1_rodata_30, lbl_1_rodata_30);
    Hu3DModelLayerSet(lbl_1_bss_90[2], 5);
    Hu3DModelAttrSet(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
    lbl_1_bss_8C = 0;
    anim = HuSprAnimRead(HuDataSelHeapReadNum(
        DATANUM(DATA_effect, 0), 268435456, 2));
    for (i = 0; i < 5; i++) {
        lbl_1_bss_68[i] = Hu3DParManCreate(anim, 100, &lbl_1_data_5C8);
        Hu3DParManAttrSet(lbl_1_bss_68[i], 100);
        Hu3DParticleBlendModeSet(Hu3DParManModelIDGet(lbl_1_bss_68[i]), 1);
        Hu3DParManRotSet(lbl_1_bss_68[i], lbl_1_rodata_90,
            lbl_1_rodata_30, lbl_1_rodata_30);
        Hu3DModelCameraSet(Hu3DParManModelIDGet(lbl_1_bss_68[i]), 2);
        Hu3DModelLayerSet(Hu3DParManModelIDGet(lbl_1_bss_68[i]), 4);
        Hu3DParManAttrSet(lbl_1_bss_68[i], HU3D_PARMAN_ATTR_TIMEUP);
    }
}

void fn_1_7324(void)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    HuWinKill((s16)lbl_1_bss_110);
}

void fn_1_7998(void)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    HuVecF pos;
    f32 amplitude;

    switch (lbl_1_bss_8C) {
        case 0:
            fn_1_5ED0(0);
            fn_1_7158();
            HuWinKill((s16) lbl_1_bss_110);
            if (fn_1_427C() == 0) {
                return;
            }
            lbl_1_bss_8C++;
            lbl_1_bss_78.x = lbl_1_bss_78.y = lbl_1_bss_78.z =
                lbl_1_rodata_30;
            lbl_1_bss_88 = lbl_1_rodata_C0;
            HuAudFXPlay(1163);
            break;

        case 1:
            lbl_1_bss_78.x += lbl_1_rodata_1B0;
            if (lbl_1_bss_78.x > lbl_1_rodata_20C) {
                lbl_1_bss_78.x = lbl_1_rodata_20C;
            }
            lbl_1_bss_78.y += lbl_1_rodata_C0;
            if (lbl_1_bss_78.y > lbl_1_rodata_90) {
                lbl_1_bss_78.y = lbl_1_rodata_90;
            }
            if (lbl_1_bss_78.x >= lbl_1_rodata_90) {
                lbl_1_bss_78.z += lbl_1_rodata_110;
                if (lbl_1_bss_78.z > lbl_1_rodata_90) {
                    lbl_1_bss_78.z = lbl_1_rodata_90;
                }
            }

            if (lbl_1_bss_134 == 1) {
                amplitude = lbl_1_rodata_210;
                Hu3DModelPosSet(lbl_1_bss_90[0], lbl_1_rodata_30,
                    (f32) (lbl_1_rodata_108 - amplitude *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.y / lbl_1_rodata_E8)),
                    (f32) (lbl_1_rodata_218 + lbl_1_rodata_118 *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.x / lbl_1_rodata_E8)));
                Hu3DModelRotSet(lbl_1_bss_90[0], lbl_1_rodata_30,
                    lbl_1_rodata_30,
                    (f32) (lbl_1_rodata_220 *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.y / lbl_1_rodata_E8) +
                        lbl_1_rodata_220 * (lbl_1_rodata_148 -
                            sin(lbl_1_rodata_E0 *
                                (lbl_1_rodata_90 - lbl_1_bss_78.z) /
                                lbl_1_rodata_E8))));

                Hu3DModelPosSet(lbl_1_bss_90[2], lbl_1_rodata_30,
                    (f32) (lbl_1_rodata_108 - amplitude *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.y / lbl_1_rodata_E8)),
                    (f32) (lbl_1_rodata_218 + lbl_1_rodata_118 *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.x / lbl_1_rodata_E8)));
                Hu3DModelRotSet(lbl_1_bss_90[2], lbl_1_rodata_30,
                    lbl_1_rodata_30,
                    (f32) (lbl_1_rodata_220 *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.y / lbl_1_rodata_E8) +
                        lbl_1_rodata_220 * (lbl_1_rodata_148 -
                            sin(lbl_1_rodata_E0 *
                                (lbl_1_rodata_90 - lbl_1_bss_78.z) /
                                lbl_1_rodata_E8))));
            } else if (lbl_1_bss_134 == 2) {
                amplitude = lbl_1_rodata_228;
                Hu3DModelPosSet(lbl_1_bss_90[1], lbl_1_rodata_30,
                    (f32) (lbl_1_rodata_230 + amplitude *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.y / lbl_1_rodata_E8)),
                    (f32) (lbl_1_rodata_238 + lbl_1_rodata_240 *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.x / lbl_1_rodata_E8)));
                Hu3DModelRotSet(lbl_1_bss_90[1], lbl_1_rodata_30,
                    lbl_1_rodata_30,
                    (f32) (lbl_1_rodata_220 *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.y / lbl_1_rodata_E8) +
                        lbl_1_rodata_220 * (lbl_1_rodata_148 -
                            sin(lbl_1_rodata_E0 *
                                (lbl_1_rodata_90 - lbl_1_bss_78.z) /
                                lbl_1_rodata_E8))));

                Hu3DModelPosSet(lbl_1_bss_90[2], lbl_1_rodata_30,
                    (f32) (lbl_1_rodata_230 + amplitude *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.y / lbl_1_rodata_E8)),
                    (f32) (lbl_1_rodata_238 + lbl_1_rodata_240 *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.x / lbl_1_rodata_E8)));
                Hu3DModelRotSet(lbl_1_bss_90[2], lbl_1_rodata_30,
                    lbl_1_rodata_30,
                    (f32) (lbl_1_rodata_220 *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.y / lbl_1_rodata_E8) +
                        lbl_1_rodata_220 * (lbl_1_rodata_148 -
                            sin(lbl_1_rodata_E0 *
                                (lbl_1_rodata_90 - lbl_1_bss_78.z) /
                                lbl_1_rodata_E8))));
            }

            if (lbl_1_bss_78.x >= lbl_1_rodata_248) {
                lbl_1_bss_88 -= lbl_1_rodata_24C;
                if (lbl_1_bss_88 < lbl_1_rodata_30) {
                    lbl_1_bss_88 = lbl_1_rodata_30;
                }
                if (lbl_1_bss_134 == 1) {
                    Hu3DModelTPLvlSet(lbl_1_bss_90[0], lbl_1_bss_88);
                } else if (lbl_1_bss_134 == 2) {
                    Hu3DModelTPLvlSet(lbl_1_bss_90[1], lbl_1_bss_88);
                }
                Hu3DModelTPLvlSet(lbl_1_bss_90[2], lbl_1_bss_88);
            }
            fn_1_9718();
            if (lbl_1_bss_78.x > lbl_1_rodata_250) {
                Hu3DParManAttrSet(lbl_1_bss_68[0], HU3D_PARMAN_ATTR_TIMEUP);
                Hu3DParManAttrSet(lbl_1_bss_68[1], HU3D_PARMAN_ATTR_TIMEUP);
                Hu3DParManAttrSet(lbl_1_bss_68[2], HU3D_PARMAN_ATTR_TIMEUP);
                Hu3DParManAttrSet(lbl_1_bss_68[3], HU3D_PARMAN_ATTR_TIMEUP);
                Hu3DParManAttrSet(lbl_1_bss_68[4], HU3D_PARMAN_ATTR_TIMEUP);
            }
            if (lbl_1_rodata_250 == lbl_1_bss_78.x) {
                pos = lbl_1_rodata_200;
                fn_1_9130(0, &pos);
                HuAudFXPlay(1164);
            }
            if (lbl_1_rodata_90 == lbl_1_bss_78.x) {
                lbl_1_bss_1A4.zoom = lbl_1_rodata_54;
                omCameraViewMoveMulti(3, &lbl_1_bss_1A4, 90, 1);
            }
            break;
    }
    if (lbl_1_rodata_30 == lbl_1_bss_88) {
        lbl_1_bss_17C = 1;
    }
}

void fn_1_8A0C(s16 layerNo)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    if (lbl_1_bss_C) {
        GXSetTexCopySrc(0, 0, 640, 480);
        GXSetTexCopyDst(640, 480, GX_TF_RGB565, GX_FALSE);
        GXCopyTex(lbl_1_bss_C, GX_FALSE);
    }
}

void fn_1_8A78(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    HU3D_CAMERA *camera = &Hu3DCamera[Hu3DCameraNo];
    UNK_MIRACLEBOOK_HOOK *hook;
    Mtx tmpMtx;
    Mtx scaleMtx;
    Mtx texMtx;
    Mtx lightMtx;
    Mtx invMtx;
    GXTexObj texObj;
    f32 transS;

    hook = drawObj->model->hookData;
    hook->unk_1C = hook->unk_1C + hook->unk_20;

    GXInitTexObj(&texObj, lbl_1_bss_C, 640, 480, GX_TF_RGB565, 0, 0, 0);
    GXInitTexObjLOD(&texObj, 1, 1, lbl_1_rodata_30, lbl_1_rodata_30,
        lbl_1_rodata_30, 0, 0, 0);
    GXLoadTexObj(&texObj, 1);
    HuSprTexLoad(lbl_1_bss_10, 0, 2, 1, 1, 1);
    HuSprTexLoad(lbl_1_bss_14, 0, 3, 1, 1, 1);
    GXSetNumTexGens(3);
    GXSetNumTevStages(2);

    transS = ((f32)camera->scissorX +
        (f32)camera->scissorW * lbl_1_rodata_254) / lbl_1_rodata_B8;
    C_MTXLightPerspective(lightMtx, camera->fov, lbl_1_rodata_D0,
        lbl_1_rodata_254, lbl_1_rodata_258, transS, lbl_1_rodata_254);
    PSMTXInverse(Hu3DCameraMtx, invMtx);
    PSMTXConcat(invMtx, drawObj->matrix, tmpMtx);
    PSMTXConcat(lightMtx, Hu3DCameraMtx, texMtx);
    PSMTXConcat(texMtx, tmpMtx, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX1, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS,
        GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);

    PSMTXTrans(tmpMtx, lbl_1_rodata_30, hook->unk_1C, lbl_1_rodata_30);
    PSMTXScale(scaleMtx, lbl_1_rodata_25C, lbl_1_rodata_25C,
        lbl_1_rodata_C0);
    PSMTXConcat(scaleMtx, tmpMtx, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0,
        GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0,
        GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);

    GXSetTevColor(1, hook->color);
    GXSetTevOrder(0, 0, 1, 0);
    GXSetTevColorIn(0, 15, 15, 15, 8);
    GXSetTevColorOp(0, 0, 0, 0, 1, 0);
    GXSetTevAlphaIn(0, 7, 7, 7, 6);
    GXSetTevAlphaOp(0, 0, 0, 0, 1, 0);
    GXSetTevOrder(1, 1, 0, 0);
    GXSetTevColorIn(1, 15, 8, 2, 0);
    GXSetTevColorOp(1, 0, 0, 0, 1, 0);
    GXSetTevAlphaIn(1, 7, 7, 7, 6);
    GXSetTevAlphaOp(1, 0, 0, 0, 1, 0);
    GXSetNumIndStages(1);
    GXSetIndTexOrder(0, 1, 2);
    GXSetIndTexCoordScale(0, 0, 0);
    GXSetTevIndWarp(0, 0, 1, 0, 1);
    GXSetIndTexMtx(1, (f32 (*)[3])hook, -1);
}

void fn_1_8F10(OMOBJ *obj)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    UNK_MIRACLEBOOK_HOOK *hook;
    s16 i = 0;
    f32 colorTime;

    hook = lbl_1_bss_18;
    for (; i < 2; i++, hook++) {
        if (hook->unk_24 <= lbl_1_rodata_30) {
            continue;
        } else {
            hook->unk_00 = lbl_1_rodata_30;
            hook->unk_04 = lbl_1_rodata_30;
            hook->unk_08 = lbl_1_rodata_260 * hook->unk_24 / lbl_1_rodata_264;
            hook->unk_0C = lbl_1_rodata_30;
            hook->unk_10 = lbl_1_rodata_30;
            hook->unk_14 = lbl_1_rodata_260 * hook->unk_24 / lbl_1_rodata_264;
            hook->unk_20 = lbl_1_rodata_268 * hook->unk_24 / lbl_1_rodata_264;
            colorTime = (hook->unk_24 - lbl_1_rodata_26C) / lbl_1_rodata_270;
            if (colorTime >= lbl_1_rodata_30) {
                hook->color.r = 0;
                hook->color.g = (u8)(lbl_1_rodata_274 * colorTime);
                hook->color.b = (u8)(lbl_1_rodata_278 * colorTime);
            }
            hook->unk_24 -= lbl_1_rodata_C0;
            if (hook->unk_24 <= lbl_1_rodata_30) {
                Hu3DLayerHookReset(10);
                Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
                obj->objFunc = NULL;
            }
        }
    }
}

void fn_1_9130(s16 hookNo, HuVecF *pos)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    OMOBJ *obj = lbl_1_bss_8;

    if (!obj) {
        return;
    }
    if (hookNo >= 2 || hookNo < 0) {
        return;
    }
    lbl_1_bss_18[hookNo].unk_1C = lbl_1_rodata_30;
    lbl_1_bss_18[hookNo].unk_24 = lbl_1_rodata_264;
    Hu3DModelAttrReset(obj->mdlId[hookNo], HU3D_ATTR_DISPOFF);
    Hu3DModelPosSet(obj->mdlId[hookNo], lbl_1_rodata_30,
        lbl_1_rodata_100, lbl_1_rodata_27C);
    Hu3DModelRotSet(obj->mdlId[hookNo], lbl_1_rodata_104,
        lbl_1_rodata_30, lbl_1_rodata_30);
    Hu3DModelScaleSet(obj->mdlId[hookNo], lbl_1_rodata_C0,
        lbl_1_rodata_C0, lbl_1_rodata_C0);
    Hu3DLayerHookSet(10, fn_1_8A0C);
    obj->objFunc = fn_1_8F10;
}

void fn_1_92B4(OMOBJ *obj)
{
    HU3D_MODEL *model = NULL;
    GXColor color;
    GXColor *colorP;
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    lbl_1_bss_10 = HuSprAnimRead(HuDataSelHeapReadNum(
        DATANUM(DATA_miraclebook, 86), 268435456, 2));
    lbl_1_bss_14 = HuSprAnimRead(HuDataSelHeapReadNum(
        DATANUM(DATA_miraclebook, 86), 268435456, 2));
    lbl_1_bss_C = HuMemDirectMallocNum(2,
        GXGetTexBufferSize(640, 480, GX_TF_RGB565, GX_FALSE, 0), 268435456);
    for (i = 0; i < 2; i++) {
        obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_miraclebook, 85), 268435456, 2));
        Hu3DModelCameraSet(obj->mdlId[i], 2);
        Hu3DModelLayerSet(obj->mdlId[i], 3);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        Hu3DModelMatHookSet(obj->mdlId[i], fn_1_8A78);
        lbl_1_bss_18[i].unk_00 = lbl_1_bss_18[i].unk_04 =
            lbl_1_bss_18[i].unk_08 = lbl_1_bss_18[i].unk_0C =
            lbl_1_bss_18[i].unk_10 = lbl_1_bss_18[i].unk_14 =
                lbl_1_rodata_30;
        color = lbl_1_rodata_280;
        colorP = &color;
        lbl_1_bss_18[i].color = *colorP;
        lbl_1_bss_18[i].unk_1C = lbl_1_rodata_30;
        lbl_1_bss_18[i].unk_20 = lbl_1_rodata_30;
        model = &Hu3DData[obj->mdlId[i]];
        model->hookData = &lbl_1_bss_18[i];
    }
    obj->objFunc = NULL;
    lbl_1_bss_8 = obj;
}

void fn_1_95B8(void)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    OMOBJ *obj = lbl_1_bss_8;
    HU3D_MODEL *model = NULL;
    s16 i;

    if (obj) {
        Hu3DLayerHookReset(10);
        for (i = 0; i < 2; i++) {
            model = &Hu3DData[obj->mdlId[i]];
            model->hookData = NULL;
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_74, obj);
    }
    obj = NULL;
    if (lbl_1_bss_10) {
        HuSprAnimKill(lbl_1_bss_10);
    }
    lbl_1_bss_10 = NULL;
    if (lbl_1_bss_14) {
        HuSprAnimKill(lbl_1_bss_14);
    }
    lbl_1_bss_14 = NULL;
    if (lbl_1_bss_C) {
        HuMemDirectFree(lbl_1_bss_C);
    }
    lbl_1_bss_C = NULL;
}

void fn_1_9718(void)
{
    MIRACLEBOOK_EARLY_RODATA_EXTERNS()
    Mtx objMtx;
    Mtx transMtx;
    MIRACLEBOOK_VEC_TABLE offsets = lbl_1_rodata_284;
    HuVecF pos;
    HuVecF base;
    s32 i;

    for (i = 0; i < 5; i++) {
        Hu3DParManAttrReset(lbl_1_bss_68[i], HU3D_PARMAN_ATTR_TIMEUP);
    }
    if (lbl_1_bss_134 == 1) {
        Hu3DModelObjMtxGet(lbl_1_bss_90[0], lbl_1_data_758, objMtx);
    } else if (lbl_1_bss_134 == 2) {
        Hu3DModelObjMtxGet(lbl_1_bss_90[1], lbl_1_data_778, objMtx);
    }
    Hu3DMtxTransGet(objMtx, &base);
    if (base.z > lbl_1_rodata_30) {
        for (i = 0; i < 5; i++) {
            PSMTXTrans(transMtx, offsets.entries[i].x, offsets.entries[i].y,
                offsets.entries[i].z);
            PSMTXConcat(objMtx, transMtx, transMtx);
            Hu3DMtxTransGet(transMtx, &pos);
            Hu3DParManPosSet(lbl_1_bss_68[i], pos.x, pos.y, pos.z);
            PSVECSubtract(&pos, &base, &pos);
            if (HuMag2Point3D(pos.x, pos.y, pos.z) <= lbl_1_rodata_F0) {
                pos.x = pos.y = pos.z = lbl_1_rodata_30;
            } else {
                PSVECNormalize(&pos, &pos);
            }
            Hu3DParManVecSet(lbl_1_bss_68[i], pos.x, lbl_1_rodata_258, pos.z);
        }
    } else {
        for (i = 0; i < 5; i++) {
            Hu3DParManAttrSet(lbl_1_bss_68[i], HU3D_PARMAN_ATTR_TIMEUP);
        }
    }
}

__declspec(section ".rodata") const HuVecF lbl_1_rodata_A0 = {0.0f, 0.0f, 0.0f};
__declspec(section ".rodata") const HuVecF lbl_1_rodata_AC = {-25.0f, 0.0f, 0.0f};
__declspec(section ".rodata") const f32 lbl_1_rodata_B8 = 640.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_BC = 480.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_C0 = 1.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_C4 = 35.259f;
__declspec(section ".rodata") const f32 lbl_1_rodata_C8 = 20.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_CC = 30000.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_D0 = 1.2f;
__declspec(section ".rodata") const f32 lbl_1_rodata_D4 = 701.8f;
__declspec(section ".rodata") const f32 lbl_1_rodata_D8 = 745.9f;
__declspec(section ".rodata") const f32 lbl_1_rodata_DC = 1.8f;
__declspec(section ".rodata") const f64 lbl_1_rodata_E0 = 3.141592653589793;
__declspec(section ".rodata") const f64 lbl_1_rodata_E8 = 180.0;
__declspec(section ".rodata") const f64 lbl_1_rodata_F0 = 0.0;
__declspec(section ".rodata") const f64 lbl_1_rodata_F8 = 55.0;
__declspec(section ".rodata") const f32 lbl_1_rodata_100 = 21.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_104 = 55.0f;
__declspec(section ".rodata") const f64 lbl_1_rodata_108 = 86.0;
__declspec(section ".rodata") const f32 lbl_1_rodata_110 = 1.5f;
__declspec(section ".rodata") const f64 lbl_1_rodata_118 = 100.0;
__declspec(section ".rodata") const f32 lbl_1_rodata_120 = -250.0f;
__declspec(section ".rodata") const f64 lbl_1_rodata_128 = -500.0;
__declspec(section ".rodata") const f64 lbl_1_rodata_130 = 70.0;
__declspec(section ".rodata") const f64 lbl_1_rodata_138 = -130.0;
__declspec(section ".rodata") const f64 lbl_1_rodata_140 = -45.0;
__declspec(section ".rodata") const f64 lbl_1_rodata_148 = 1.0;
__declspec(section ".rodata") const f64 lbl_1_rodata_150 = 35.0;
__declspec(section ".rodata") const f32 lbl_1_rodata_158 = -35.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_15C = 288.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_160 = 220.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_164 = 37.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_168 = 0.2f;
__declspec(section ".rodata") const f32 lbl_1_rodata_16C = 306.2f;
__declspec(section ".rodata") const f32 lbl_1_rodata_170 = 246.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_174 = 12.4f;
__declspec(section ".rodata") const f32 lbl_1_rodata_178 = 0.4f;
__declspec(section ".rodata") const f32 lbl_1_rodata_17C = 74.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_180 = 422.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_184 = 95.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_188 = 30.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_18C = 462.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_190 = 22.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_194 = 92.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_198 = 266.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_19C = 24.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_1A0 = 146.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_1A4 = 118.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_1A8 = 267.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_1AC = 147.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_1B0 = 2.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_1B4 = 360.0f;
__declspec(section ".rodata") const f64 lbl_1_rodata_1B8 = 10.0;
__declspec(section ".rodata") const f64 lbl_1_rodata_1C0 = 3.0;
__declspec(section ".rodata") const f64 lbl_1_rodata_1C8 = 2.0;
__declspec(section ".rodata") const f64 lbl_1_rodata_1D0 = 1.5;
__declspec(section ".rodata") const f32 lbl_1_rodata_1D8 = 544.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_1DC = -10000.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_1E0 = 140.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_1E4 = 254.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_1E8 = 8.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_1EC = 240.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_1F0 = 127.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_1F4 = 134.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_1F8 = 297.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_1FC = 440.0f;
__declspec(section ".rodata") const HuVecF lbl_1_rodata_200 = {0.0f, 0.0f, 0.0f};
__declspec(section ".rodata") const f32 lbl_1_rodata_20C = 180.1f;
__declspec(section ".rodata") const f32 lbl_1_rodata_210 = 35.0f;
__declspec(section ".rodata") const f64 lbl_1_rodata_218 = 105.0;
__declspec(section ".rodata") const f64 lbl_1_rodata_220 = 360.0;
__declspec(section ".rodata") const f32 lbl_1_rodata_228 = 105.0f;
__declspec(section ".rodata") const f64 lbl_1_rodata_230 = -54.0;
__declspec(section ".rodata") const f64 lbl_1_rodata_238 = 160.0;
__declspec(section ".rodata") const f64 lbl_1_rodata_240 = 40.0;
__declspec(section ".rodata") const f32 lbl_1_rodata_248 = 160.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_24C = 0.03f;
__declspec(section ".rodata") const f32 lbl_1_rodata_250 = 180.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_254 = 0.5f;
__declspec(section ".rodata") const f32 lbl_1_rodata_258 = -0.5f;
__declspec(section ".rodata") const f32 lbl_1_rodata_25C = 0.8f;
__declspec(section ".rodata") const f32 lbl_1_rodata_260 = 0.09f;
__declspec(section ".rodata") const f32 lbl_1_rodata_264 = 120.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_268 = -0.02f;
__declspec(section ".rodata") const f32 lbl_1_rodata_26C = 80.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_270 = 40.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_274 = 16.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_278 = 64.0f;
__declspec(section ".rodata") const f32 lbl_1_rodata_27C = 10.0f;
__declspec(section ".rodata") const GXColor lbl_1_rodata_280 = {0, 16, 64, 16};
__declspec(section ".rodata") const MIRACLEBOOK_VEC_TABLE lbl_1_rodata_284 = {{
    {0.0f, 0.0f, 0.0f},
    {40.0f, 10.0f, 0.0f},
    {-40.0f, 10.0f, 0.0f},
    {20.0f, -40.0f, 0.0f},
    {-20.0f, -40.0f, 0.0f},
}};

s32 lbl_1_bss_224;
s32 lbl_1_bss_220;
s16 lbl_1_bss_21C;
s16 lbl_1_bss_21A;
s16 lbl_1_bss_218;
s16 lbl_1_bss_1FC[2][7];
u8 lbl_1_bss_1E8[20];
s32 lbl_1_bss_1E4;
s32 lbl_1_bss_1E0;
s32 lbl_1_bss_1DC;
s32 lbl_1_bss_1D8;
s32 lbl_1_bss_1D4;
f32 lbl_1_bss_1C8[3];
s32 lbl_1_bss_1C4;
s16 lbl_1_bss_1C0[2];
OM_CAMERA_VIEW lbl_1_bss_1A4;
s32 lbl_1_bss_1A0;
f32 lbl_1_bss_19C;
f32 lbl_1_bss_184[6];
s32 lbl_1_bss_180;
s32 lbl_1_bss_17C;
s16 lbl_1_bss_17A;
s16 lbl_1_bss_178;
s16 lbl_1_bss_174[2];
s16 lbl_1_bss_16C[2][2];
s16 lbl_1_bss_144[20];
s32 lbl_1_bss_140;
s32 lbl_1_bss_13C;
s32 lbl_1_bss_138;
s32 lbl_1_bss_134;
s32 lbl_1_bss_130;
f32 lbl_1_bss_120[4];
f32 lbl_1_bss_11C;
f32 lbl_1_bss_118;
s32 lbl_1_bss_114;
s32 lbl_1_bss_110;
s32 lbl_1_bss_10C;
s32 lbl_1_bss_B4[22];
s32 lbl_1_bss_B0;
s32 lbl_1_bss_AC;
s32 lbl_1_bss_A4[2];
s32 lbl_1_bss_A0;
s32 lbl_1_bss_98[2];
s16 lbl_1_bss_90[4];
s32 lbl_1_bss_8C;
f32 lbl_1_bss_88;
f32 lbl_1_bss_84;
HuVecF lbl_1_bss_78;
OMOBJMAN *lbl_1_bss_74;
s16 lbl_1_bss_68[6];
UNK_MIRACLEBOOK_HOOK lbl_1_bss_18[2];
ANIMDATA *lbl_1_bss_14;
ANIMDATA *lbl_1_bss_10;
void *lbl_1_bss_C;
OMOBJ *lbl_1_bss_8;
OMOBJMAN *lbl_1_bss_4;
s32 lbl_1_bss_0;
