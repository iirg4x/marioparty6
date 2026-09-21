#define _MATH_H
#include "REL/m602Dll.h"

u32 lbl_1_data_3D0[12] = {
    9633792,
    9306174,
    9633840,
    9306182,
    9633845,
    9306184,
    9633817,
    9633838,
    9633814,
    9306140,
    9633830,
    9633832,
};

s32 lbl_1_data_400 = -1;

char lbl_1_data_404[16] = { 109, 54, 48, 50, 95, 102, 105, 101, 108, 100, 45, 100, 97, 105, 49, 0 };

char lbl_1_data_414[16] = { 109, 54, 48, 50, 95, 102, 105, 101, 108, 100, 45, 100, 97, 105, 50, 0 };

char lbl_1_data_424[16] = { 109, 54, 48, 50, 95, 102, 105, 101, 108, 100, 45, 100, 97, 105, 51, 0 };

char lbl_1_data_434[16] = { 109, 54, 48, 50, 95, 102, 105, 101, 108, 100, 45, 100, 97, 105, 52, 0 };

char lbl_1_data_444[10] = { 109, 54, 48, 50, 102, 117, 100, 97, 66, 0 };

char lbl_1_data_44E[14] = { 102, 45, 105, 116, 101, 109, 104, 111, 111, 107, 45, 114, 0, 0 };

struct _struct_lbl_1_bss_318_0x58 lbl_1_bss_318[4];

u8 lbl_1_bss_314;

s32 lbl_1_bss_310;

s16 lbl_1_bss_30C;

s16 lbl_1_bss_30A;

s16 lbl_1_bss_308;

Point3d lbl_1_bss_2FC;

s16 lbl_1_bss_2F4[4];

s16 lbl_1_bss_2F2;

s16 lbl_1_bss_2F0;

M602Bss250Record lbl_1_bss_250[4];

s16 lbl_1_bss_24E;

s16 lbl_1_bss_24C;

s16 lbl_1_bss_24A;

/* Three created and consumed model IDs. */
u8 lbl_1_bss_248;

void fn_1_4CD0(s16 unused)
{

}

void fn_1_4CD4(void)
{
    s16 var_r31;

    var_r31 = 0;
    while (var_r31 < 24) {
        Hu3DModelAttrReset(lbl_1_bss_202[var_r31], 1U);
        if ((var_r31 == 4) || (var_r31 == 16)) {
            Hu3DModelAttrSet(lbl_1_bss_202[var_r31], 1U);
        }
        var_r31 += 1;
    }
}

void fn_1_4D68(void)
{
    s16 var_r31;

    var_r31 = 0;
    while (var_r31 < 24) {
        if (((var_r31 >= 0) && (var_r31 <= 1)) || ((var_r31 >= 8) && (var_r31 <= 9)) || ((var_r31 >= 12) && (var_r31 <= 13)) || ((var_r31 >= 20) && (var_r31 <= 21))) {
            Hu3DModelAttrReset(lbl_1_bss_202[var_r31], 1U);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_202[var_r31], 1U);
        }
        var_r31 += 1;
    }
}

void fn_1_4E48(void)
{
    s16 var_r31;

    var_r31 = 0;
    while (var_r31 < 24) {
        if ((var_r31 >= 0) && (var_r31 <= 11) && (var_r31 != 4)) {
            Hu3DModelAttrReset(lbl_1_bss_202[var_r31], 1U);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_202[var_r31], 1U);
        }
        var_r31 += 1;
    }
}

void fn_1_4EEC(void)
{
    s16 var_r31;

    var_r31 = 0;
    while (var_r31 < 24) {
        if ((var_r31 >= 12) && (var_r31 <= 23) && (var_r31 != 16)) {
            Hu3DModelAttrReset(lbl_1_bss_202[var_r31], 1U);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_202[var_r31], 1U);
        }
        var_r31 += 1;
    }
}

s16 fn_1_4F90(void)
{
    s16 var_r30;
    s16 var_r31;

    var_r30 = 0;
    var_r31 = 0;
    while (var_r31 < 4) {
        var_r30 += fn_1_4FE8(var_r31);
        var_r31 += 1;
    }
    return var_r30;
}

s16 fn_1_4FE8(s16 arg0)
{
    if ((lbl_1_bss_314 != 0) && (lbl_1_bss_318[arg0].unk24 == 2)) {
        return 1;
    }
    return 0;
}

s16 fn_1_502C(s16 arg0)
{
    return lbl_1_bss_318[arg0].unk52;
}

s16 fn_1_5048(void)
{
    s16 var_r30;
    s16 var_r31;

    for (var_r31 = 0; var_r31 < 4; var_r31++) {
        if ((lbl_1_bss_314 != 0) && (lbl_1_bss_318[var_r31].unk24 == 2)) {
            var_r30 = 1;
        } else {
            var_r30 = 0;
        }
        if (var_r30 != 0) { return var_r31; }
    }
    return -1;
}

s16 fn_1_50D4(s16 arg0)
{
    return lbl_1_bss_318[arg0].unk54;
}

void fn_1_50F0(s16 arg0)
{
    lbl_1_bss_318[arg0].unk54 += 1;
    if (lbl_1_bss_318[arg0].unk54 > 2) {
        lbl_1_bss_318[arg0].unk54 = 2;
    }
}

void fn_1_5150(s16 arg0)
{
    lbl_1_bss_318[arg0].unk54 = 0;
}

s16 fn_1_5170(void)
{
    s16 var_r31;

    for (var_r31 = 0; var_r31 < 4; var_r31++) {
        if (fn_1_50D4(var_r31) >= 2) { return 1; }
    }
    return 0;
}

s16 fn_1_51D4(void)
{
    s16 var_r31;

    for (var_r31 = 0; var_r31 < 4; var_r31++) {
        if (fn_1_50D4(var_r31) >= 2) { return var_r31; }
    }
    return -1;
}

s16 fn_1_5238(s16 arg0)
{
    return lbl_1_bss_318[arg0].unk2;
}

void fn_1_5254(void)
{
    ANIMDATA *temp_r29;
    s32 var_r31;
    s32 var_r30;

    temp_r29 = HuSprAnimRead(HuDataSelHeapReadNum(3997716, 268435456, HEAP_MODEL));
    for (var_r31 = 0; var_r31 < 4; var_r31++) {
        lbl_1_bss_318[var_r31].unk0 = var_r31;
        lbl_1_bss_318[var_r31].unk2 = GwPlayerConf[var_r31].charNo;
        lbl_1_bss_318[var_r31].unk4 = GwPlayerConf[var_r31].padNo;
        lbl_1_bss_318[var_r31].unk6 = GwPlayerConf[var_r31].grpNo;
        if (GwPlayerConf[var_r31].type != 0) {
            lbl_1_bss_318[var_r31].unk8 = GwPlayerConf[var_r31].comDif;
        } else {
            lbl_1_bss_318[var_r31].unk8 = -1;
        }
        lbl_1_bss_318[var_r31].unkA = CharModelCreate(lbl_1_bss_318[var_r31].unk2, 4);
        var_r30 = 0;
        while (var_r30 < 12) {
            lbl_1_bss_318[var_r31].motion[var_r30] = CharMotionCreate(lbl_1_bss_318[var_r31].unk2, lbl_1_data_3D0[var_r30]);
            var_r30 += 1;
        }
        CharMotionDataClose(lbl_1_bss_318[var_r31].unk2);
        lbl_1_bss_318[var_r31].unk24 = 3;
        CharMotionSet(lbl_1_bss_318[var_r31].unk2, lbl_1_bss_318[var_r31].motion[lbl_1_bss_318[var_r31].unk24]);
        CharModelAttrSet(lbl_1_bss_318[var_r31].unk2, 1073741825U);
        Hu3DModelCameraSet(lbl_1_bss_318[var_r31].unkA, 8U);
        Hu3DModelLayerSet(lbl_1_bss_318[var_r31].unkA, 1);
        lbl_1_bss_318[var_r31].pos.x = lbl_1_bss_318[var_r31].pos.y = lbl_1_bss_318[var_r31].pos.z = 0.0f;
        lbl_1_bss_318[var_r31].rot.x = lbl_1_bss_318[var_r31].rot.y = lbl_1_bss_318[var_r31].rot.z = 0.0f;
        lbl_1_bss_318[var_r31].scale.x = lbl_1_bss_318[var_r31].scale.y = lbl_1_bss_318[var_r31].scale.z = 1.0f;
        if (var_r31 == 0) {
            Hu3DModelObjPosGet(lbl_1_bss_23C[0], lbl_1_data_404, &lbl_1_bss_318[var_r31].pos);
            lbl_1_bss_318[var_r31].rot.y = 14.0f;
        }
        if (var_r31 == 1) {
            Hu3DModelObjPosGet(lbl_1_bss_23C[0], lbl_1_data_414, &lbl_1_bss_318[var_r31].pos);
            lbl_1_bss_318[var_r31].rot.y = 5.0f;
        }
        if (var_r31 == 2) {
            Hu3DModelObjPosGet(lbl_1_bss_23C[0], lbl_1_data_424, &lbl_1_bss_318[var_r31].pos);
            lbl_1_bss_318[var_r31].rot.y = -5.0f;
        }
        if (var_r31 == 3) {
            Hu3DModelObjPosGet(lbl_1_bss_23C[0], lbl_1_data_434, &lbl_1_bss_318[var_r31].pos);
            lbl_1_bss_318[var_r31].rot.y = -14.0f;
        }
        Hu3DModelPosSetV(lbl_1_bss_318[var_r31].unkA, &lbl_1_bss_318[var_r31].pos);
        Hu3DModelRotSetV(lbl_1_bss_318[var_r31].unkA, &lbl_1_bss_318[var_r31].rot);
        Hu3DModelScaleSetV(lbl_1_bss_318[var_r31].unkA, &lbl_1_bss_318[var_r31].scale);
        Hu3DModelShadowSet(lbl_1_bss_318[var_r31].unkA);
        lbl_1_bss_318[var_r31].unk4C = Hu3DLLightCreate(lbl_1_bss_318[var_r31].unkA, 0.0f, -5000.0f, 0.0f, 0.0f, 1.0f, 0.0f, 255U, 255U, 128U);
        Hu3DLLightPosSet(lbl_1_bss_318[var_r31].unkA, lbl_1_bss_318[var_r31].unk4C, 0.0f, -5000.0f, 0.0f, 0.0f, -1.0f, 0.0f);
        lbl_1_bss_318[var_r31].unk4E = Hu3DModelCreate(HuDataSelHeapReadNum(3997701, 268435456, HEAP_MODEL));
        lbl_1_bss_318[var_r31].unk50 = Hu3DAnimCreate(temp_r29, lbl_1_bss_318[var_r31].unk4E, lbl_1_data_444);
        var_r30 = (u8) frand() % 3;
        if (var_r30 == 0) {
            lbl_1_bss_318[var_r31].unk52 = 1;
        } else if (var_r30 == 1) {
            lbl_1_bss_318[var_r31].unk52 = 2;
        } else {
            lbl_1_bss_318[var_r31].unk52 = 0;
        }
        Hu3DAnimBankSet(lbl_1_bss_318[var_r31].unk50, (u16) lbl_1_bss_318[var_r31].unk52);
        Hu3DModelCameraSet(lbl_1_bss_318[var_r31].unk4E, 8U);
        Hu3DModelLayerSet(lbl_1_bss_318[var_r31].unk4E, 1);
        Hu3DModelHookSet(lbl_1_bss_318[var_r31].unkA, lbl_1_data_44E, lbl_1_bss_318[var_r31].unk4E);
        fn_1_5150(var_r31);
        lbl_1_bss_318[var_r31].unk56 = 0;
        lbl_1_bss_2F4[var_r31] = Hu3DModelCreate(HuDataSelHeapReadNum(3997706, 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_2F4[var_r31], 8U);
        Hu3DModelLayerSet(lbl_1_bss_2F4[var_r31], 2);
        Hu3DModelAttrSet(lbl_1_bss_2F4[var_r31], 1U);
        Hu3DModelPosSetV(lbl_1_bss_2F4[var_r31], &lbl_1_bss_318[var_r31].pos);
    }
    lbl_1_bss_314 = 0;
    lbl_1_bss_310 = 0;
    lbl_1_bss_30C = -1;
    CharEffectLayerSet(2);
    lbl_1_bss_30A = Hu3DModelCreate(HuDataSelHeapReadNum(3997717, 268435456, HEAP_MODEL));
    lbl_1_bss_308 = Hu3DJointMotion(lbl_1_bss_30A, HuDataSelHeapReadNum(3997718, 268435456, HEAP_MODEL));
    Hu3DMotionSet(lbl_1_bss_30A, lbl_1_bss_308);
    Hu3DModelCameraSet(lbl_1_bss_30A, 8U);
    Hu3DModelLayerSet(lbl_1_bss_30A, 3);
    Hu3DModelAttrSet(lbl_1_bss_30A, 1U);
    Hu3DModelAttrSet(lbl_1_bss_30A, 1073741825U);
    lbl_1_bss_2FC.x = lbl_1_bss_2FC.y = lbl_1_bss_2FC.z = 0.0f;
    Hu3DModelPosSetV(lbl_1_bss_30A, &lbl_1_bss_2FC);
    fn_1_96EC();
}

static const s16 lbl_1_rodata_138[6] = { 60, 105, 150, 195, 240, 0 };

s8 fn_1_5C7C(s16 arg0)
{
    f32 temp_f31;
    s16 var_r31;

    var_r31 = 0;
    while (var_r31 < 4) {
        if (arg0 == lbl_1_rodata_138[var_r31]) {
            lbl_1_bss_318[var_r31].unk24 = 4;
            CharMotionShiftSet(lbl_1_bss_318[var_r31].unk2, lbl_1_bss_318[var_r31].motion[lbl_1_bss_318[var_r31].unk24], 0.0f, 8.0f, 1073741825U);
        }
        if ((lbl_1_bss_318[var_r31].unk24 == 4) && (Hu3DMotionShiftIDGet(lbl_1_bss_318[var_r31].unkA) == -1)) {
            temp_f31 = CharMotionMaxTimeGet(lbl_1_bss_318[var_r31].unk2);
            if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[var_r31].unk2)) >= temp_f31) {
                lbl_1_bss_318[var_r31].unk24 = 3;
                CharMotionSet(lbl_1_bss_318[var_r31].unk2, lbl_1_bss_318[var_r31].motion[lbl_1_bss_318[var_r31].unk24]);
                lbl_1_bss_248 += 1;
            }
        }
        var_r31 += 1;
    }
    var_r31 = 0;
    while (var_r31 < 4) {
        if ((lbl_1_rodata_138[var_r31] <= arg0) && (arg0 < lbl_1_rodata_138[var_r31 + 1])) {
            Hu3DModelAttrReset(lbl_1_bss_2F4[var_r31], 1U);
            Hu3DLLightPosSet(lbl_1_bss_318[var_r31].unkA, lbl_1_bss_318[var_r31].unk4C, 0.0f, -5000.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_2F4[var_r31], 1U);
            Hu3DLLightPosSet(lbl_1_bss_318[var_r31].unkA, lbl_1_bss_318[var_r31].unk4C, 0.0f, -5000.0f, 0.0f, 0.0f, -1.0f, 0.0f);
        }
        var_r31 += 1;
    }
    if (lbl_1_bss_248 >= 4U) {
        return 1;
    }
    return 0;
}

void fn_1_6078(void)
{
    s16 var_r31;

    var_r31 = 0;
    while (var_r31 < 4) {
        lbl_1_bss_318[var_r31].unk24 = 3;
        CharMotionShiftSet(lbl_1_bss_318[var_r31].unk2, lbl_1_bss_318[var_r31].motion[lbl_1_bss_318[var_r31].unk24], 0.0f, 10.0f, 1073741825U);
        Hu3DModelAttrSet(lbl_1_bss_2F4[var_r31], 1U);
        Hu3DLLightPosSet(lbl_1_bss_318[var_r31].unkA, lbl_1_bss_318[var_r31].unk4C, 0.0f, -5000.0f, 0.0f, 0.0f, -1.0f, 0.0f);
        var_r31 += 1;
    }
}

void fn_1_61E0(void)
{
    Point3d sp8;
    s32 var_r31;

    var_r31 = 0;
    while (var_r31 < 4) {
        if ((lbl_1_bss_318[var_r31].unk24 != 3) && (lbl_1_bss_318[var_r31].unk24 != 8)) {
            lbl_1_bss_318[var_r31].unk24 = 3;
            CharMotionShiftSet(lbl_1_bss_318[var_r31].unk2, lbl_1_bss_318[var_r31].motion[lbl_1_bss_318[var_r31].unk24], 0.0f, 10.0f, 1073741825U);
        }
        if ((lbl_1_bss_318[var_r31].unk52 == 3) && (lbl_1_bss_318[var_r31].unk24 != 8)) {
            if (lbl_1_bss_24A == 0) {
                Hu3DModelObjPosGet(lbl_1_bss_318[var_r31].unkA, lbl_1_data_44E, &sp8);
                sp8.z += 70.0f;
                CharEffectSmokeCreate(8, &sp8);
                lbl_1_bss_24A += 1;
            } else if (lbl_1_bss_24A == 7) {
                lbl_1_bss_318[var_r31].unk52 = 0;
                Hu3DAnimBankSet(lbl_1_bss_318[var_r31].unk50, (u16) lbl_1_bss_318[var_r31].unk52);
                Hu3DModelScaleSet(lbl_1_bss_318[var_r31].unk4E, 1.0f, 1.0f, 1.0f);
                lbl_1_bss_24A = 0;
            } else {
                lbl_1_bss_24A += 1;
            }
        }
        var_r31 += 1;
    }
    lbl_1_bss_314 = 0;
    lbl_1_bss_310 = 0;
    lbl_1_bss_30C = -1;
}

void fn_1_647C(void)
{
    f32 temp_f31;
    s16 var_r30;
    s16 var_r31;

    fn_1_6C5C();
    fn_1_9D10();
    var_r30 = 0;
    var_r31 = 0;
    while (var_r31 < 4) {
        if ((lbl_1_bss_314 == 0) && (lbl_1_bss_318[var_r31].unk52 != 3) && (((s32) (lbl_1_bss_318[var_r31].unk56 & 512) != 0) || ((s32) (lbl_1_bss_318[var_r31].unk56 & 256) != 0) || ((s32) (lbl_1_bss_318[var_r31].unk56 & 32) != 0))) {
            var_r30 += 1;
        }
        var_r31 += 1;
    }
    if (var_r30 > 1) {
        var_r30 = frandmod(var_r30);
        var_r31 = 0;
        while (var_r31 < 4) {
            if ((lbl_1_bss_314 == 0) && (lbl_1_bss_318[var_r31].unk52 != 3) && (((s32) (lbl_1_bss_318[var_r31].unk56 & 512) != 0) || ((s32) (lbl_1_bss_318[var_r31].unk56 & 256) != 0) || ((s32) (lbl_1_bss_318[var_r31].unk56 & 32) != 0))) {
                if (var_r30 != 0) {
                    lbl_1_bss_318[var_r31].unk56 = 0;
                }
                var_r30 -= 1;
            }
            var_r31 += 1;
        }
    }
    var_r31 = 0;
    while (var_r31 < 4) {
        if ((lbl_1_bss_314 == 0) && (lbl_1_bss_318[var_r31].unk52 != 3)) {
            if ((s32) (lbl_1_bss_318[var_r31].unk56 & 512) != 0) {
                lbl_1_bss_318[var_r31].unk24 = 1;
                CharMotionSet(lbl_1_bss_318[var_r31].unk2, lbl_1_bss_318[var_r31].motion[lbl_1_bss_318[var_r31].unk24]);
                lbl_1_bss_314 = 1;
                lbl_1_bss_318[var_r31].unk52 = 0;
                Hu3DAnimBankSet(lbl_1_bss_318[var_r31].unk50, (u16) lbl_1_bss_318[var_r31].unk52);
                omVibrate(var_r31, 20, 7, 3);
                if (var_r31 == 0) {
                    HuAudFXPanning(HuAudFXPlay(1570), 32);
                } else if (var_r31 == 1) {
                    HuAudFXPanning(HuAudFXPlay(1570), 53);
                } else if (var_r31 == 2) {
                    HuAudFXPanning(HuAudFXPlay(1570), 75);
                } else if (var_r31 == 3) {
                    HuAudFXPanning(HuAudFXPlay(1570), 96);
                }
            }
            if ((s32) (lbl_1_bss_318[var_r31].unk56 & 256) != 0) {
                lbl_1_bss_318[var_r31].unk24 = 1;
                CharMotionSet(lbl_1_bss_318[var_r31].unk2, lbl_1_bss_318[var_r31].motion[lbl_1_bss_318[var_r31].unk24]);
                lbl_1_bss_314 = 1;
                lbl_1_bss_318[var_r31].unk52 = 1;
                Hu3DAnimBankSet(lbl_1_bss_318[var_r31].unk50, (u16) lbl_1_bss_318[var_r31].unk52);
                omVibrate(var_r31, 20, 7, 3);
                if (var_r31 == 0) {
                    HuAudFXPanning(HuAudFXPlay(1570), 32);
                } else if (var_r31 == 1) {
                    HuAudFXPanning(HuAudFXPlay(1570), 53);
                } else if (var_r31 == 2) {
                    HuAudFXPanning(HuAudFXPlay(1570), 75);
                } else if (var_r31 == 3) {
                    HuAudFXPanning(HuAudFXPlay(1570), 96);
                }
            }
            if ((s32) (lbl_1_bss_318[var_r31].unk56 & 32) != 0) {
                lbl_1_bss_318[var_r31].unk24 = 1;
                CharMotionSet(lbl_1_bss_318[var_r31].unk2, lbl_1_bss_318[var_r31].motion[lbl_1_bss_318[var_r31].unk24]);
                lbl_1_bss_314 = 1;
                lbl_1_bss_318[var_r31].unk52 = 2;
                Hu3DAnimBankSet(lbl_1_bss_318[var_r31].unk50, (u16) lbl_1_bss_318[var_r31].unk52);
                omVibrate(var_r31, 20, 7, 3);
                if (var_r31 == 0) {
                    HuAudFXPanning(HuAudFXPlay(1570), 32);
                } else if (var_r31 == 1) {
                    HuAudFXPanning(HuAudFXPlay(1570), 53);
                } else if (var_r31 == 2) {
                    HuAudFXPanning(HuAudFXPlay(1570), 75);
                } else if (var_r31 == 3) {
                    HuAudFXPanning(HuAudFXPlay(1570), 96);
                }
            }
        }
        if ((lbl_1_bss_314 != 0) && (lbl_1_bss_318[var_r31].unk24 == 1)) {
            temp_f31 = CharMotionMaxTimeGet(lbl_1_bss_318[var_r31].unk2);
            if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[var_r31].unk2)) >= temp_f31) {
                lbl_1_bss_318[var_r31].unk24 = 2;
                CharMotionSet(lbl_1_bss_318[var_r31].unk2, lbl_1_bss_318[var_r31].motion[lbl_1_bss_318[var_r31].unk24]);
            }
        }
        var_r31 += 1;
    }
}

void fn_1_6C5C(void)
{
    s16 var_r31;

    var_r31 = 0;
    while (var_r31 < 4) {
        lbl_1_bss_318[var_r31].unk56 = 0;
        if ((s32) (HuPadBtnDown[lbl_1_bss_318[var_r31].unk4] & 512) != 0) {
            lbl_1_bss_318[var_r31].unk56 |= 512;
        }
        if ((s32) (HuPadBtnDown[lbl_1_bss_318[var_r31].unk4] & 256) != 0) {
            lbl_1_bss_318[var_r31].unk56 |= 256;
        }
        if ((s32) (HuPadBtnDown[lbl_1_bss_318[var_r31].unk4] & 1024) != 0) {
            lbl_1_bss_318[var_r31].unk56 |= 32;
        }
        var_r31 += 1;
    }
}

void fn_1_6DB8(void)
{
    f32 temp_f31;
    s16 var_r31;

    var_r31 = 0;
    while (var_r31 < 4) {
        if ((lbl_1_bss_314 != 0) && (lbl_1_bss_318[var_r31].unk24 == 1)) {
            temp_f31 = CharMotionMaxTimeGet(lbl_1_bss_318[var_r31].unk2);
            if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[var_r31].unk2)) >= temp_f31) {
                lbl_1_bss_318[var_r31].unk24 = 2;
                CharMotionSet(lbl_1_bss_318[var_r31].unk2, lbl_1_bss_318[var_r31].motion[lbl_1_bss_318[var_r31].unk24]);
            }
        }
        var_r31 += 1;
    }
}

void fn_1_6F00(void)
{
    lbl_1_bss_24C = 0;
}

s16 fn_1_6F14(void)
{
    Point3d sp8;
    f32 heightScale;
    s16 player;
    s16 count;

    player = 0;
    while (player < 4) {
        if ((lbl_1_bss_314 != 0) && (lbl_1_bss_318[player].unk24 == 1)) {
            if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[player].unk2)) >= CharMotionMaxTimeGet(lbl_1_bss_318[player].unk2)) {
                lbl_1_bss_318[player].unk24 = 2;
                CharMotionSet(lbl_1_bss_318[player].unk2, lbl_1_bss_318[player].motion[lbl_1_bss_318[player].unk24]);
            }
        }
        player += 1;
    }
    if (lbl_1_bss_24C < 0) {
        return -1;
    }
    if (lbl_1_bss_30C >= 0) {
        if (lbl_1_bss_310 != 0) {
            {
                switch (lbl_1_bss_24C) {            /* switch 1 */
                case 0:                             /* switch 1 */
                    lbl_1_bss_2F2 += 1;
                    player = lbl_1_bss_30C;
                    if (lbl_1_bss_2F2 == 2) {
                        Hu3DModelAttrReset(lbl_1_bss_2F4[player], 1U);
                        Hu3DLLightPosSet(lbl_1_bss_318[player].unkA, lbl_1_bss_318[player].unk4C, 0.0f, -5000.0f, 0.0f, 0.0f, 1.0f, 0.0f);
                        if (player == 0) {
                            HuAudFXPanning(HuAudFXPlay(1573), 32);
                        } else if (player == 1) {
                            HuAudFXPanning(HuAudFXPlay(1573), 53);
                        } else if (player == 2) {
                            HuAudFXPanning(HuAudFXPlay(1573), 75);
                        } else {
                            HuAudFXPanning(HuAudFXPlay(1573), 96);
                        }
                    }
                    if (lbl_1_bss_2F2 >= 30) {
                        lbl_1_bss_24C += 1;
                        lbl_1_bss_2F2 = 0;
                    }
                    break;
                case 1:                             /* switch 1 */
                    player = 0;
                    while (player < 4) {
                        if (lbl_1_bss_30C == player) {
                            lbl_1_bss_318[player].unk24 = 4;
                            CharMotionShiftSet(lbl_1_bss_318[player].unk2, lbl_1_bss_318[player].motion[lbl_1_bss_318[player].unk24], 0.0f, 10.0f, 1073741825U);
                        } else if (lbl_1_bss_318[player].unk24 != 8) {
                            lbl_1_bss_318[player].unk24 = 5;
                            CharMotionShiftSet(lbl_1_bss_318[player].unk2, lbl_1_bss_318[player].motion[lbl_1_bss_318[player].unk24], 0.0f, 10.0f, 1073741825U);
                        }
                        player += 1;
                    }
                    fn_1_48B4();
                    lbl_1_bss_24C += 1;
                    break;
                case 2:                             /* switch 1 */
                    count = 0;
                    player = 0;
                    while (player < 4) {
                        if ((lbl_1_bss_318[player].unk24 == 3) || (lbl_1_bss_318[player].unk24 == 8)) {
                            count += 1;
                        }
                        if ((lbl_1_bss_318[player].unk24 != 3) && (lbl_1_bss_318[player].unk24 != 8) && (Hu3DMotionShiftIDGet(lbl_1_bss_318[player].unkA) == -1)) {
                            if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[player].unk2)) >= CharMotionMaxTimeGet(lbl_1_bss_318[player].unk2)) {
                                lbl_1_bss_318[player].unk24 = 3;
                                CharMotionSet(lbl_1_bss_318[player].unk2, lbl_1_bss_318[player].motion[lbl_1_bss_318[player].unk24]);
                            }
                        }
                        player += 1;
                    }
                    if (count >= 4) {
                        lbl_1_bss_24C += 1;
                        lbl_1_bss_2F2 = 0;
                    }
                    break;
                case 3:                             /* switch 1 */
                    lbl_1_bss_2F2 += 1;
                    if (lbl_1_bss_2F2 >= 30) {
                        fn_1_4998();
                        lbl_1_bss_24C += 1;
                        lbl_1_bss_2F2 = 0;
                    }
                    break;
                case 4:                             /* switch 1 */
                    count = 0;
                    player = 0;
                    while (player < 4) {
                        if ((lbl_1_bss_318[lbl_1_bss_30C].unk54 < 2) && (lbl_1_bss_318[player].unk24 == 8)) {
                            lbl_1_bss_318[player].unk24 = 9;
                            CharMotionShiftSet(lbl_1_bss_318[player].unk2, lbl_1_bss_318[player].motion[lbl_1_bss_318[player].unk24], 0.0f, 4.0f, 1073741825U);
                            count += 1;
                        }
                        player += 1;
                    }
                    if (count == 0) {
                        lbl_1_bss_24C = 6;
                    } else {
                        lbl_1_bss_24C += 1;
                    }
                    break;
                case 5:                             /* switch 1 */
                    count = 0;
                    player = 0;
                    while (player < 4) {
                        if (lbl_1_bss_318[player].unk24 == 9) {
                            if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[player].unk2)) >= CharMotionMaxTimeGet(lbl_1_bss_318[player].unk2)) {
                                lbl_1_bss_318[player].unk24 = 3;
                                CharMotionSet(lbl_1_bss_318[player].unk2, lbl_1_bss_318[player].motion[lbl_1_bss_318[player].unk24]);
                                count += 1;
                            }
                        }
                        player += 1;
                    }
                    if (count != 0) {
                        lbl_1_bss_24C += 1;
                        lbl_1_bss_2F2 = 0;
                    }
                    break;
                case 6:                             /* switch 1 */
                    lbl_1_bss_2F2 += 1;
                    if (lbl_1_bss_2F2 >= 30) {
                        player = lbl_1_bss_30C;
                        Hu3DModelAttrSet(lbl_1_bss_2F4[player], 1U);
                        Hu3DLLightPosSet(lbl_1_bss_318[player].unkA, lbl_1_bss_318[player].unk4C, 0.0f, -5000.0f, 0.0f, 0.0f, -1.0f, 0.0f);
                        lbl_1_bss_24C = -1;
                        lbl_1_bss_2F2 = 0;
                    }
                    break;
                }
            }
        } else {
            switch (lbl_1_bss_24C) {                /* switch 3 */
            case 0:                                 /* switch 3 */
                count = 0;
                player = 0;
                while (player < 4) {
                    if (lbl_1_bss_318[player].unk24 == 8) {
                        lbl_1_bss_318[player].unk24 = 9;
                        CharMotionSet(lbl_1_bss_318[player].unk2, lbl_1_bss_318[player].motion[lbl_1_bss_318[player].unk24]);
                        count += 1;
                    }
                    player += 1;
                }
                if (count == 0) {
                    lbl_1_bss_24C = 2;
                } else {
                    lbl_1_bss_24C += 1;
                }
                break;
            case 1:                                 /* switch 3 */
                count = 0;
                player = 0;
                while (player < 4) {
                    if (lbl_1_bss_318[player].unk24 == 9) {
                        if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[player].unk2)) >= CharMotionMaxTimeGet(lbl_1_bss_318[player].unk2)) {
                            lbl_1_bss_318[player].unk24 = 3;
                            CharMotionSet(lbl_1_bss_318[player].unk2, lbl_1_bss_318[player].motion[lbl_1_bss_318[player].unk24]);
                            count += 1;
                        }
                    }
                    player += 1;
                }
                if (count != 0) {
                    lbl_1_bss_24C += 1;
                }
                break;
            case 2:                                 /* switch 3 */
                player = lbl_1_bss_30C;
                lbl_1_bss_318[player].unk24 = 3;
                CharMotionShiftSet(lbl_1_bss_318[player].unk2, lbl_1_bss_318[player].motion[lbl_1_bss_318[player].unk24], 0.0f, 15.0f, 1073741825U);
                lbl_1_bss_24C += 1;
                lbl_1_bss_2F2 = 0;
                break;
            case 3:                                 /* switch 3 */
                lbl_1_bss_2F2 += 1;
                if (lbl_1_bss_2F2 >= 15) {
                    Hu3DModelObjPosGet(lbl_1_bss_318[lbl_1_bss_30C].unkA, lbl_1_data_44E, &sp8);
                    sp8.z += 70.0f;
                    CharEffectSmokeCreate(8, &sp8);
                    lbl_1_bss_24C += 1;
                    lbl_1_bss_2F2 = 0;
                }
                break;
            case 4:                                 /* switch 3 */
                lbl_1_bss_2F2 += 1;
                if (lbl_1_bss_2F2 >= 15) {
                    player = lbl_1_bss_30C;
                    Hu3DModelScaleSet(lbl_1_bss_318[player].unk4E, 0.0f, 0.0f, 0.0f);
                    lbl_1_bss_318[player].unk52 = 3;
                }
                if (lbl_1_bss_2F2 >= 30) {
                    lbl_1_bss_2F2 = 0;
                    lbl_1_bss_24C += 1;
                }
                break;
            case 5:                                 /* switch 3 */
                lbl_1_bss_2F2 += 1;
                if (lbl_1_bss_2F2 >= 30) {
                    lbl_1_bss_24C += 1;
                    lbl_1_bss_2F2 = 0;
                }
                break;
            case 6:                                 /* switch 3 */
                player = lbl_1_bss_30C;
                lbl_1_bss_318[player].unk24 = 6;
                CharMotionShiftSet(lbl_1_bss_318[player].unk2, lbl_1_bss_318[player].motion[lbl_1_bss_318[player].unk24], 0.0f, 4.0f, 1073741825U);
                lbl_1_bss_24C += 1;
                break;
            case 7:                                 /* switch 3 */
                count = 0;
                player = 0;
                while (player < 4) {
                    if (player == lbl_1_bss_30C) {
                        if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[player].unk2)) >= CharMotionMaxTimeGet(lbl_1_bss_318[player].unk2)) {
                            CharModelAttrSet(lbl_1_bss_318[player].unk2, 1073741826U);
                            lbl_1_bss_2FC = lbl_1_bss_318[player].pos;
                            lbl_1_bss_2FC.y += 1000.0f;
                            Hu3DModelPosSetV(lbl_1_bss_30A, &lbl_1_bss_2FC);
                            Hu3DModelAttrReset(lbl_1_bss_30A, 1U);
                            count += 1;
                            lbl_1_data_400 = HuAudFXPlay(1575);
                            if (player == 0) {
                                HuAudFXPanning(lbl_1_data_400, 32);
                            } else if (player == 1) {
                                HuAudFXPanning(lbl_1_data_400, 53);
                            } else if (player == 2) {
                                HuAudFXPanning(lbl_1_data_400, 75);
                            } else {
                                HuAudFXPanning(lbl_1_data_400, 96);
                            }
                        }
                    }
                    player += 1;
                }
                if (count != 0) {
                    lbl_1_bss_24C += 1;
                }
                break;
            case 8:                                 /* switch 3 */
                lbl_1_bss_2F2 += 1;
                if (lbl_1_bss_2F2 >= 30) {
                    lbl_1_bss_24C += 1;
                    lbl_1_bss_2F2 = 0;
                    lbl_1_bss_24E = 1;
                }
                break;
            case 9:                                 /* switch 3 */
                player = lbl_1_bss_30C;
                heightScale = 70.0f + CharModelHeightGet(player);
                lbl_1_bss_2FC.y -= 50.0f;
                Hu3DModelPosSetV(lbl_1_bss_30A, &lbl_1_bss_2FC);
                if (lbl_1_bss_2FC.y <= heightScale) {
                    lbl_1_bss_318[player].unk24 = 7;
                    CharMotionSet(lbl_1_bss_318[player].unk2, lbl_1_bss_318[player].motion[lbl_1_bss_318[player].unk24]);
                    CharMotionStartEndSet(lbl_1_bss_318[player].unk2, 0.0f, 11.0f);
                    CharModelAttrReset(lbl_1_bss_318[player].unk2, 1073741826U);
                    fn_1_2FDC(130);
                    lbl_1_bss_24C += 1;
                    if (lbl_1_bss_24E != 0) {
                        if (lbl_1_data_400 >= 0) {
                            HuAudFXStop(lbl_1_data_400);
                        }
                        lbl_1_data_400 = -1;
                        if (player == 0) {
                            HuAudFXPanning(HuAudFXPlay(1579), 32);
                            HuAudFXPlayPan(939, 32);
                        } else if (player == 1) {
                            HuAudFXPanning(HuAudFXPlay(1579), 53);
                            HuAudFXPlayPan(939, 53);
                        } else if (player == 2) {
                            HuAudFXPanning(HuAudFXPlay(1579), 75);
                            HuAudFXPlayPan(939, 75);
                        } else {
                            HuAudFXPanning(HuAudFXPlay(1579), 96);
                            HuAudFXPlayPan(939, 96);
                        }
                    }
                    lbl_1_bss_24E = 0;
                }
                break;
            case 10:                                /* switch 3 */
                player = lbl_1_bss_30C;
                CharModelAttrSet(lbl_1_bss_318[player].unk2, 1073741826U);
                heightScale = 50.0f;
                lbl_1_bss_2FC.y -= heightScale;
                if (lbl_1_bss_2FC.y < 70.0f) {
                    lbl_1_bss_2FC.y = 70.0f;
                }
                Hu3DModelPosSetV(lbl_1_bss_30A, &lbl_1_bss_2FC);
                heightScale = (lbl_1_bss_2FC.y - 70.0f) / CharModelHeightGet(player);
                if (heightScale < 0.05f) {
                    heightScale = 0.05f;
                }
                if (heightScale > 1.0f) {
                    heightScale = 1.0f;
                }
                Hu3DModelScaleSet(lbl_1_bss_318[player].unkA, 1.0f, heightScale, 1.0f);
                if (lbl_1_bss_2FC.y <= 70.0f) {
                    lbl_1_bss_318[player].unk24 = 8;
                    CharMotionSet(lbl_1_bss_318[player].unk2, lbl_1_bss_318[player].motion[lbl_1_bss_318[player].unk24]);
                    Hu3DModelScaleSet(lbl_1_bss_318[player].unkA, 1.0f, 0.05f, 1.0f);
                    Hu3DMotionTimeSet(lbl_1_bss_30A, 0.0f);
                    sp8 = lbl_1_bss_2FC;
                    sp8.z += 100.0f;
                    CharEffectSmokeCreate(8, &sp8);
                    lbl_1_bss_24C += 1;
                    lbl_1_bss_2F2 = 0;
                    omVibrate(player, 20, 20, 0);
                    fn_1_1418(30);
                }
                break;
            case 11:                                /* switch 3 */
                lbl_1_bss_2F2 += 1;
                if (lbl_1_bss_2F2 >= 120) {
                    lbl_1_bss_24C += 1;
                    lbl_1_bss_2F2 = 0;
                }
                break;
            case 12:                                /* switch 3 */
                player = lbl_1_bss_30C;
                lbl_1_bss_2FC.y += 20.0f;
                Hu3DModelPosSetV(lbl_1_bss_30A, &lbl_1_bss_2FC);
                if (lbl_1_bss_2FC.y > 2200.0f) {
                    Hu3DModelAttrSet(lbl_1_bss_30A, 1U);
                    lbl_1_bss_24C = -1;
                    lbl_1_bss_2F2 = 0;
                }
                heightScale = (lbl_1_bss_2FC.y - 570.0f) / CharModelHeightGet(player);
                if (heightScale < 0.05f) {
                    heightScale = 0.05f;
                }
                if (heightScale >= 1.0f) {
                    if (lbl_1_bss_2F2 < 3) {
                        heightScale = 1.0f + (0.06f * (f32) lbl_1_bss_2F2);
                    } else if (lbl_1_bss_2F2 < 9) {
                        heightScale = 1.2f - (0.06f * (f32) (lbl_1_bss_2F2 - 3));
                    } else if (lbl_1_bss_2F2 < 15) {
                        heightScale = 0.8f + (0.05f * (f32) (lbl_1_bss_2F2 - 9));
                    } else if (lbl_1_bss_2F2 < 21) {
                        heightScale = 1.1f - (0.016f * (f32) (lbl_1_bss_2F2 - 15));
                    } else {
                        heightScale = 1.0f;
                        CharModelAttrReset(lbl_1_bss_318[player].unk2, 1073741826U);
                    }
                    lbl_1_bss_2F2 += 1;
                }
                Hu3DModelScaleSet(lbl_1_bss_318[player].unkA, 1.0f, heightScale, 1.0f);
                break;
            }
        }
    } else {
        switch (lbl_1_bss_24C) {              /* switch 6; irregular */
        case 0:                                     /* switch 6 */
            count = 0;
            player = 0;
            while (player < 4) {
                if (lbl_1_bss_318[player].unk24 == 8) {
                    lbl_1_bss_318[player].unk24 = 9;
                    CharMotionSet(lbl_1_bss_318[player].unk2, lbl_1_bss_318[player].motion[lbl_1_bss_318[player].unk24]);
                    count += 1;
                }
                player += 1;
            }
            if (count == 0) {
                lbl_1_bss_24C = -1;
            } else {
                lbl_1_bss_24C += 1;
            }
            break;
        case 1:                                     /* switch 6 */
            count = 0;
            player = 0;
            while (player < 4) {
                if (lbl_1_bss_318[player].unk24 == 9) {
                    if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[player].unk2)) >= CharMotionMaxTimeGet(lbl_1_bss_318[player].unk2)) {
                        lbl_1_bss_318[player].unk24 = 3;
                        CharMotionSet(lbl_1_bss_318[player].unk2, lbl_1_bss_318[player].motion[lbl_1_bss_318[player].unk24]);
                        count += 1;
                    }
                }
                player += 1;
            }
            if (count != 0) {
                lbl_1_bss_24C += 1;
                lbl_1_bss_2F2 = 0;
            }
            break;
        case 2:                                     /* switch 6 */
            lbl_1_bss_2F2 += 1;
            if (lbl_1_bss_2F2 >= 30) {
                lbl_1_bss_24C = -1;
                lbl_1_bss_2F2 = 0;
            }
            break;
        }
    }
    return 0;
}

void fn_1_8D90(s16 arg0, s32 arg1)
{
    if (arg0 >= 0) {
        if (arg1 != 0) {
            lbl_1_bss_318[arg0].unk54 += 1;
            if (lbl_1_bss_318[arg0].unk54 > 2) {
                lbl_1_bss_318[arg0].unk54 = 2;
            }
            lbl_1_bss_310 = 1;
            lbl_1_bss_30C = arg0;
            return;
        }
        lbl_1_bss_310 = 0;
        lbl_1_bss_30C = arg0;
        return;
    }
    lbl_1_bss_310 = 0;
    lbl_1_bss_30C = -1;
}

s16 fn_1_8E64(s16 arg0)
{
    Point3d sp8;
    s16 var_r31;

    if (arg0 == 0) {
        var_r31 = 0;
        while (var_r31 < 4) {
            if (lbl_1_bss_318[var_r31].unk24 != 8) {
                lbl_1_bss_318[var_r31].unk24 = 3;
                CharMotionShiftSet(lbl_1_bss_318[var_r31].unk2, lbl_1_bss_318[var_r31].motion[lbl_1_bss_318[var_r31].unk24], 0.0f, 4.0f, 1073741825U);
            }
            var_r31 += 1;
        }
    } else if (arg0 == 5) {
        var_r31 = 0;
        while (var_r31 < 4) {
            Hu3DModelScaleGet(lbl_1_bss_318[var_r31].unk4E, &sp8);
            if (sp8.y > 0.5) {
                Hu3DModelObjPosGet(lbl_1_bss_318[var_r31].unkA, lbl_1_data_44E, &sp8);
                sp8.z += 70.0f;
                CharEffectSmokeCreate(8, &sp8);
            }
            var_r31 += 1;
        }
    } else if (arg0 == 12) {
        var_r31 = 0;
        while (var_r31 < 4) {
            Hu3DModelScaleSet(lbl_1_bss_318[var_r31].unk4E, 0.0f, 0.0f, 0.0f);
            lbl_1_bss_318[var_r31].unk52 = 3;
            var_r31 += 1;
        }
    } else if (arg0 == 42) {
        return 1;
    }
    return 0;
}

void fn_1_90B4(void)
{
    s16 var_r28;
    s16 var_r31;

    var_r28 = fn_1_51D4();
    if (var_r28 >= 0) {
        var_r31 = 0;
        while (var_r31 < 4) {
            if (var_r31 == var_r28) {
                lbl_1_bss_318[var_r31].unk24 = 10;
                CharMotionShiftSet(lbl_1_bss_318[var_r31].unk2, lbl_1_bss_318[var_r31].motion[lbl_1_bss_318[var_r31].unk24], 0.0f, 10.0f, 1073741825U);
            } else if (lbl_1_bss_318[var_r31].unk24 != 8) {
                lbl_1_bss_318[var_r31].unk24 = 11;
                CharMotionShiftSet(lbl_1_bss_318[var_r31].unk2, lbl_1_bss_318[var_r31].motion[lbl_1_bss_318[var_r31].unk24], 0.0f, 10.0f, 1073741825U);
            }
            var_r31 += 1;
        }
        return;
    }
    var_r31 = 0;
    while (var_r31 < 4) {
        if (lbl_1_bss_318[var_r31].unk24 != 8) {
            lbl_1_bss_318[var_r31].unk24 = 11;
            CharMotionShiftSet(lbl_1_bss_318[var_r31].unk2, lbl_1_bss_318[var_r31].motion[lbl_1_bss_318[var_r31].unk24], 0.0f, 10.0f, 1073741825U);
        }
        var_r31 += 1;
    }
}

void fn_1_9378(void)
{
    f32 temp_f31;
    s16 var_r31;

    var_r31 = 0;
    while (var_r31 < 4) {
        if ((lbl_1_bss_318[var_r31].unk24 != 0) && (lbl_1_bss_318[var_r31].unk24 != 8)) {
            temp_f31 = CharMotionMaxTimeGet(lbl_1_bss_318[var_r31].unk2);
            if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[var_r31].unk2)) >= temp_f31) {
                CharModelAttrSet(lbl_1_bss_318[var_r31].unk2, 1073741826U);
            }
        }
        var_r31 += 1;
    }
}

void fn_1_9480(s32 arg0)
{
    s16 var_r28;
    s16 var_r31;
    s32 var_r30;

    var_r28 = fn_1_51D4();
    if (arg0 != 0) {
        lbl_1_bss_2F0 = 0;
    }
    var_r30 = lbl_1_bss_2F0 / 10;
    if (var_r30 != 0) {
        var_r30 %= 2;
    } else {
        var_r30 = 0;
    }
    var_r31 = 0;
    while (var_r31 < 4) {
        if (var_r31 == var_r28) {
            if (var_r30 != 0) {
                Hu3DModelAttrReset(lbl_1_bss_2F4[var_r31], 1U);
                Hu3DLLightPosSet(lbl_1_bss_318[var_r31].unkA, lbl_1_bss_318[var_r31].unk4C, 0.0f, -5000.0f, 0.0f, 0.0f, 1.0f, 0.0f);
            } else {
                Hu3DModelAttrSet(lbl_1_bss_2F4[var_r31], 1U);
                Hu3DLLightPosSet(lbl_1_bss_318[var_r31].unkA, lbl_1_bss_318[var_r31].unk4C, 0.0f, -5000.0f, 0.0f, 0.0f, -1.0f, 0.0f);
            }
        }
        var_r31 += 1;
    }
    lbl_1_bss_2F0 += 1;
}

void fn_1_96EC(void)
{
    s16 temp_r28;
    s16 var_r30;
    s16 var_r29;
    s16 var_r31;

    for (var_r30 = 0; var_r30 < 4; var_r30++) {
        var_r29 = 0;
        while (var_r29 < 10) {
            lbl_1_bss_250[var_r30].unknown000[var_r29] = 0;
            lbl_1_bss_250[var_r30].unknown014[var_r29] = 0;
            if (lbl_1_bss_3C[var_r29].choices[0] == lbl_1_bss_3C[var_r29].choices[1]) {
                var_r31 = 2;
            } else if (lbl_1_bss_3C[var_r29].choices[0] == lbl_1_bss_3C[var_r29].choices[2]) {
                var_r31 = 1;
            } else {
                var_r31 = 0;
            }
            temp_r28 = (u8)frand() % 100;
            if (lbl_1_bss_318[var_r30].unk8 == 0) {
                if (temp_r28 < 20) {
                    lbl_1_bss_250[var_r30].unknown000[var_r29] = var_r31;
                } else if (temp_r28 < 60) {
                    var_r31 += 1;
                    if (var_r31 > 2) {
                        var_r31 = 0;
                    }
                    lbl_1_bss_250[var_r30].unknown000[var_r29] = var_r31;
                } else {
                    var_r31 -= 1;
                    if (var_r31 < 0) {
                        var_r31 = 2;
                    }
                    lbl_1_bss_250[var_r30].unknown000[var_r29] = var_r31;
                }
                lbl_1_bss_250[var_r30].unknown014[var_r29] = (s16) (60 - ((u8)frand() % 60));
            }
            if (lbl_1_bss_318[var_r30].unk8 == 1) {
                if (temp_r28 < 40) {
                    lbl_1_bss_250[var_r30].unknown000[var_r29] = var_r31;
                } else if (temp_r28 < 70) {
                    var_r31 += 1;
                    if (var_r31 > 2) {
                        var_r31 = 0;
                    }
                    lbl_1_bss_250[var_r30].unknown000[var_r29] = var_r31;
                } else {
                    var_r31 -= 1;
                    if (var_r31 < 0) {
                        var_r31 = 2;
                    }
                    lbl_1_bss_250[var_r30].unknown000[var_r29] = var_r31;
                }
                lbl_1_bss_250[var_r30].unknown014[var_r29] = (s16) (120 - ((u8)frand() % 60));
            }
            if (lbl_1_bss_318[var_r30].unk8 == 2) {
                if (temp_r28 < 60) {
                    lbl_1_bss_250[var_r30].unknown000[var_r29] = var_r31;
                } else if (temp_r28 < 80) {
                    var_r31 += 1;
                    if (var_r31 > 2) {
                        var_r31 = 0;
                    }
                    lbl_1_bss_250[var_r30].unknown000[var_r29] = var_r31;
                } else {
                    var_r31 -= 1;
                    if (var_r31 < 0) {
                        var_r31 = 2;
                    }
                    lbl_1_bss_250[var_r30].unknown000[var_r29] = var_r31;
                }
                lbl_1_bss_250[var_r30].unknown014[var_r29] = (s16) (180 - ((u8)frand() % 60));
            }
            if (lbl_1_bss_318[var_r30].unk8 == 3) {
                if (temp_r28 < 85) {
                    lbl_1_bss_250[var_r30].unknown000[var_r29] = var_r31;
                } else if (temp_r28 < 92) {
                    var_r31 += 1;
                    if (var_r31 > 2) {
                        var_r31 = 0;
                    }
                    lbl_1_bss_250[var_r30].unknown000[var_r29] = var_r31;
                } else {
                    var_r31 -= 1;
                    if (var_r31 < 0) {
                        var_r31 = 2;
                    }
                    lbl_1_bss_250[var_r30].unknown000[var_r29] = var_r31;
                }
                lbl_1_bss_250[var_r30].unknown014[var_r29] = (s16) (240 - ((u8)frand() % 60));
            }
            var_r29 += 1;
        }
    }
}

void fn_1_9D10(void)
{
    s16 temp_r30;
    s16 var_r31;
    s32 temp_r29;

    temp_r29 = fn_1_9EEC();
    temp_r30 = fn_1_178C() - 1;
    if (temp_r29 > 0) {
        var_r31 = 0;
        while (var_r31 < 4) {
            if (lbl_1_bss_318[var_r31].unk8 >= 0) {
                lbl_1_bss_318[var_r31].unk56 = 0;
                if (temp_r29 == lbl_1_bss_250[var_r31].unknown014[temp_r30]) {
                    if (lbl_1_bss_250[var_r31].unknown000[temp_r30] == 0) {
                        lbl_1_bss_318[var_r31].unk56 |= 512;
                    } else if (lbl_1_bss_250[var_r31].unknown000[temp_r30] == 1) {
                        lbl_1_bss_318[var_r31].unk56 |= 256;
                    } else if (lbl_1_bss_250[var_r31].unknown000[temp_r30] == 2) {
                        lbl_1_bss_318[var_r31].unk56 |= 32;
                    }
                }
            }
            var_r31 += 1;
        }
    }
}
