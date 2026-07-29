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

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

extern const HuVecF lbl_1_rodata_10;
extern const HuVecF lbl_1_rodata_1C;
extern const GXColor lbl_1_rodata_28;
extern const f32 lbl_1_rodata_30;
extern const f64 lbl_1_rodata_38;
extern const f64 lbl_1_rodata_40;
extern const f32 lbl_1_rodata_48;
extern const f32 lbl_1_rodata_4C;
extern const f32 lbl_1_rodata_50;
extern const f32 lbl_1_rodata_54;
extern const f32 lbl_1_rodata_58;
extern const f32 lbl_1_rodata_5C;
extern const f32 lbl_1_rodata_60;
extern const f32 lbl_1_rodata_64;
extern const f32 lbl_1_rodata_68;
extern const f32 lbl_1_rodata_6C;
extern const f32 lbl_1_rodata_70;
extern const f32 lbl_1_rodata_74;
extern const f32 lbl_1_rodata_78;
extern const f32 lbl_1_rodata_7C;
extern const f32 lbl_1_rodata_84;
extern const f32 lbl_1_rodata_90;
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
extern const f32 lbl_1_rodata_158;
extern const f32 lbl_1_rodata_16C;
extern const f32 lbl_1_rodata_198;
extern const f32 lbl_1_rodata_1A0;
extern const f32 lbl_1_rodata_1B0;
extern const f64 lbl_1_rodata_F0;
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
extern const f64 lbl_1_rodata_1B8;
extern const f64 lbl_1_rodata_1C0;
extern const f64 lbl_1_rodata_1C8;
extern const f64 lbl_1_rodata_1D0;
extern const f32 lbl_1_rodata_1B4;
extern const f32 lbl_1_rodata_1D8;
extern const f32 lbl_1_rodata_1DC;
extern const f32 lbl_1_rodata_1EC;
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
extern const f32 lbl_1_rodata_258;
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

s32 fn_1_2FCC(s32 dir)
{
    s32 result = 0;
    s32 i;

    if (dir > 0) {
        if (lbl_1_bss_1E4 + 1 == lbl_1_bss_1D4) {
            if (lbl_1_bss_1E8[lbl_1_bss_1E4 + 1] != 0xFF) {
                result = 1;
            }
        } else {
            for (i = lbl_1_bss_1E4 + 1; i <= lbl_1_bss_1D4; i++) {
                if (lbl_1_bss_1E8[i] != 0xFF) {
                    result = i - lbl_1_bss_1E4;
                    break;
                }
            }
        }
    } else if (dir < 0) {
        if (lbl_1_bss_1E4 - 1 == lbl_1_bss_1D8) {
            if (lbl_1_bss_1E8[lbl_1_bss_1E4 - 1] != 0xFF) {
                result = -1;
            }
        } else {
            for (i = lbl_1_bss_1E4 - 1; i >= lbl_1_bss_1D8; i--) {
                if (lbl_1_bss_1E8[i] != 0xFF) {
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
                DATA_miraclebook + total + itemIndex + 8, 0x10000000, 2));
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

void fn_1_3418(void)
{
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
    OMOBJ *obj;
    HuVecF center;
    HuVecF rot;
    HuVecF *centerP;
    HuVecF *rotP;

    obj = omAddObjEx(lbl_1_bss_74, 0x7FDA, 0, 0, -1, omOutViewMulti);
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
