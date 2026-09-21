#define _MATH_H
#include "REL/m602Dll.h"

s32 lbl_1_data_E8[13] = {
    3997711,
    3997712,
    3997713,
    3997714,
    3997715,
    0,
    0,
    0,
    1148846080,
    0,
    0,
    -998637568,
    -1,
};

s32 lbl_1_data_11C[2] = { 3997702, 3997703 };

M602CameraParams lbl_1_data_124[3] = {
    {
        1,
        20.0f,
        10.0f,
        5000.0f,
        0.6666667f,
        0.0f,
        0.0f,
        192.0f,
        288.0f,
        0.0f,
        1.0f,
        { 0.0f, 0.0f, 1800.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
    },
    {
        2,
        20.0f,
        10.0f,
        5000.0f,
        0.6666667f,
        192.0f,
        0.0f,
        192.0f,
        288.0f,
        0.0f,
        1.0f,
        { 0.0f, 0.0f, 1800.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
    },
    {
        4,
        20.0f,
        10.0f,
        5000.0f,
        0.6666667f,
        384.0f,
        0.0f,
        192.0f,
        288.0f,
        0.0f,
        1.0f,
        { 0.0f, 0.0f, 1800.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
    },
};

M602CameraParams lbl_1_data_214 = {
    4,
    20.0f,
    10.0f,
    5000.0f,
    0.9791667f,
    0.0f,
    0.0f,
    576.0f,
    576.0f,
    0.0f,
    1.0f,
    { 0.0f, -19.0f, 1200.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, -19.0f, 0.0f },
};

char lbl_1_data_264[9] = { 54, 48, 50, 95, 99, 97, 114, 100, 0 };

char lbl_1_data_26D[11] = { 116, 101, 115, 116, 0, 0, 0, 0, 0, 0, 0 };

/* const */
static const Point3d lbl_1_rodata_60 = { 0.0f, 0.0f, -160.0f };

static const Point3d lbl_1_rodata_6C = { 1.1f, 1.1f, 1.1f };

static const Point3d lbl_1_rodata_78 = { 0.0f, 0.0f, 200.0f };

static const Point3d lbl_1_rodata_84 = { 0.0f, 0.0f, -140.0f };

static const Point3d lbl_1_rodata_90 = { 0.7f, 0.8f, 0.05f };

static const Point3d lbl_1_rodata_9C = { 0.9f, 1.15f, 0.05f };

M602BssA0Record lbl_1_bss_A0[3];

ANIMDATA *lbl_1_bss_8C[5];

struct _struct_lbl_1_bss_3C_0x8 lbl_1_bss_3C[10];

s16 lbl_1_bss_3A;

s16 lbl_1_bss_38;

s16 lbl_1_bss_34[2];

s16 lbl_1_bss_32;

s16 lbl_1_bss_30;

s16 lbl_1_bss_2E;

s16 lbl_1_bss_2C;

s16 lbl_1_bss_2A;

u8 lbl_1_bss_28;

ANIMDATA *lbl_1_bss_24;

s16 lbl_1_bss_20;

void fn_1_1770(void)
{

}

void fn_1_1774(void)
{

}

void fn_1_1778(s16 unused)
{

}

void fn_1_177C(void)
{

}

void fn_1_1780(void)
{

}

void fn_1_1784(void)
{

}

void fn_1_1788(void)
{

}

s16 fn_1_178C(void)
{
    return lbl_1_bss_20;
}

void fn_1_179C(void)
{
    s32 var_r30;
    s32 var_r31;
    u16 var_r29;
    s16 layer;
    u32 bufferSize;

    /* The loop index is 0..2; each path assigns the camera mask before use. */
    var_r31 = 0;
    while (var_r31 < 3) {
        Hu3DCameraCreate(lbl_1_data_124[var_r31].cameraBit);
        Hu3DCameraViewportSet(lbl_1_data_124[var_r31].cameraBit, lbl_1_data_124[var_r31].viewportX, lbl_1_data_124[var_r31].viewportY, lbl_1_data_124[var_r31].viewportW, lbl_1_data_124[var_r31].viewportH, lbl_1_data_124[var_r31].minZ, lbl_1_data_124[var_r31].maxZ);
        Hu3DCameraPerspectiveSet(lbl_1_data_124[var_r31].cameraBit, lbl_1_data_124[var_r31].fov, lbl_1_data_124[var_r31].nearPlane, lbl_1_data_124[var_r31].farPlane, lbl_1_data_124[var_r31].aspect);
        Hu3DCameraScissorSet(lbl_1_data_124[var_r31].cameraBit, (u32) lbl_1_data_124[var_r31].viewportX, (u32) lbl_1_data_124[var_r31].viewportY, (u32) lbl_1_data_124[var_r31].viewportW, (u32) lbl_1_data_124[var_r31].viewportH);
        Hu3DCameraPosSetV(lbl_1_data_124[var_r31].cameraBit, &lbl_1_data_124[var_r31].pos, &lbl_1_data_124[var_r31].up, &lbl_1_data_124[var_r31].target);
        var_r31 += 1;
    }
    var_r31 = 0;
    while (var_r31 < 5) {
        lbl_1_bss_8C[var_r31] = HuSprAnimRead(HuDataSelHeapReadNum(lbl_1_data_E8[var_r31], 268435456, HEAP_MODEL));
        var_r31 += 1;
    }
    for (var_r31 = 0; var_r31 < 3; var_r31++) {
        lbl_1_bss_A0[var_r31].unknown014 = 0;
        lbl_1_bss_A0[var_r31].unknown018 = 0;
        lbl_1_bss_A0[var_r31].unknown040 = 0;
        lbl_1_bss_A0[var_r31].unknown044 = 0.0f;
        lbl_1_bss_A0[var_r31].unknown048 = -1;
        lbl_1_bss_A0[var_r31].unknown04C = -1;
        var_r30 = 0;
        while (var_r30 < 5) {
            lbl_1_bss_A0[var_r31].model[var_r30] = Hu3DModelCreate(HuDataSelHeapReadNum(3997696, 268435456, HEAP_MODEL));
            if (var_r31 == 0) {
                var_r29 = 1;
            }
            if (var_r31 == 1) {
                var_r29 = 2;
            }
            if (var_r31 == 2) {
                var_r29 = 4;
            }
            layer = 5;
            Hu3DModelCameraSet(lbl_1_bss_A0[var_r31].model[var_r30], var_r29);
            Hu3DModelLayerSet(lbl_1_bss_A0[var_r31].model[var_r30], layer);
            lbl_1_bss_A0[var_r31].pos.x = lbl_1_bss_A0[var_r31].pos.y = lbl_1_bss_A0[var_r31].pos.z = 0.0f;
            lbl_1_bss_A0[var_r31].rot.x = lbl_1_bss_A0[var_r31].rot.z = 0.0f;
            lbl_1_bss_A0[var_r31].rot.y = 180.0f;
            lbl_1_bss_A0[var_r31].scale.x = lbl_1_bss_A0[var_r31].scale.y = lbl_1_bss_A0[var_r31].scale.z = 1.0f;
            Hu3DModelPosSetV(lbl_1_bss_A0[var_r31].model[var_r30], &lbl_1_bss_A0[var_r31].pos);
            Hu3DModelRotSetV(lbl_1_bss_A0[var_r31].model[var_r30], &lbl_1_bss_A0[var_r31].rot);
            Hu3DModelScaleSetV(lbl_1_bss_A0[var_r31].model[var_r30], &lbl_1_bss_A0[var_r31].scale);
            lbl_1_bss_A0[var_r31].anim[var_r30] = Hu3DAnimCreate(lbl_1_bss_8C[var_r30], lbl_1_bss_A0[var_r31].model[var_r30], lbl_1_data_264);
            Hu3DAnimBankSet(lbl_1_bss_A0[var_r31].anim[var_r30], (u16) lbl_1_bss_A0[var_r31].unknown018);
            if (var_r30 != lbl_1_bss_A0[var_r31].unknown014) {
                Hu3DModelAttrSet(lbl_1_bss_A0[var_r31].model[var_r30], 1U);
            }
            var_r30 += 1;
        }
    }
    Hu3DCameraLayerHookSet(8, 0, fn_1_2250);
    lbl_1_bss_3A = Hu3DModelCreate(HuDataSelHeapReadNum(3997700, 268435456, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_3A, 8U);
    Hu3DModelLayerSet(lbl_1_bss_3A, 0);
    lbl_1_bss_24 = HuSprAnimMake(576, 288, 0);
    lbl_1_bss_24->bmp->palData = NULL;
    lbl_1_bss_24->bmp->palNum = 0;
    bufferSize = GXGetTexBufferSize(576U, 288U, 6U, 0U, 0U);
    lbl_1_bss_24->bmp->data = HuMemDirectMallocNum(HEAP_MODEL, bufferSize, 268435456U);
    lbl_1_bss_38 = Hu3DAnimCreate(lbl_1_bss_24, lbl_1_bss_3A, lbl_1_data_26D);
    lbl_1_bss_32 = 0;
    var_r31 = 0;
    while (var_r31 < 2) {
        lbl_1_bss_34[var_r31] = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_11C[var_r31], 268435456, HEAP_MODEL));
        Hu3DModelAmbSet(lbl_1_bss_34[var_r31], 1.0f, 1.0f, 1.0f);
        if (var_r31 != 0) {
            Hu3DModelAttrSet(lbl_1_bss_34[var_r31], 1U);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_34[var_r31], 1073741825U);
        }
        Hu3DModelPosSet(lbl_1_bss_34[var_r31], lbl_1_rodata_60.x, lbl_1_rodata_60.y, lbl_1_rodata_60.z);
        Hu3DModelScaleSet(lbl_1_bss_34[var_r31], lbl_1_rodata_6C.x, lbl_1_rodata_6C.y, lbl_1_rodata_6C.z);
        Hu3DModelCameraSet(lbl_1_bss_34[var_r31], 7U);
        Hu3DModelLayerSet(lbl_1_bss_34[var_r31], 4);
        var_r31 += 1;
    }
    lbl_1_bss_30 = Hu3DModelCreate(HuDataSelHeapReadNum(3997704, 268435456, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_30, 1U);
    Hu3DModelAttrSet(lbl_1_bss_30, 1073741825U);
    Hu3DModelPosSet(lbl_1_bss_30, lbl_1_rodata_78.x, lbl_1_rodata_78.y, lbl_1_rodata_78.z);
    Hu3DModelScaleSet(lbl_1_bss_30, lbl_1_rodata_90.x, lbl_1_rodata_90.y, lbl_1_rodata_90.z);
    Hu3DModelCameraSet(lbl_1_bss_30, 7U);
    Hu3DModelLayerSet(lbl_1_bss_30, 6);
    Hu3DMotionSpeedSet(lbl_1_bss_30, 4.0f);
    fn_1_39D0();
    lbl_1_bss_20 = 0;
    Hu3DBGColorSet(0U, 178U, 235U);
    lbl_1_bss_2E = Hu3DModelCreate(HuDataSelHeapReadNum(3997705, 268435456, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_2E, 1U);
    Hu3DModelAttrSet(lbl_1_bss_2E, 1073741825U);
    Hu3DModelCameraSet(lbl_1_bss_2E, 4U);
    Hu3DModelLayerSet(lbl_1_bss_2E, 5);
}

void fn_1_2250(s16 layerNo)
{
    Hu3DFbCopyExec(0, 0, 576, 288, GX_TF_RGBA8, 0, lbl_1_bss_24->bmp->data);
    Hu3DZClear();
}

void fn_1_22A0(void)
{
    HuSprAnimKill(lbl_1_bss_24);
    lbl_1_bss_24 = NULL;
}

void fn_1_22DC(void)
{
    s16 var_r29;
    s32 var_r30;
    s32 var_r31;
    u8 var_r28;

    /* The random value is consumed only when at least one countdown is > 0,
     * the same predicate which initializes it in the preceding do/while. */
    var_r29 = 0;
    if ((lbl_1_bss_A0->unknown050 > 0) || (lbl_1_bss_A0[1].unknown050 > 0) || (lbl_1_bss_A0[2].unknown050 > 0)) {
        do {
            var_r28 = (u8) frand() % 5;
        } while (var_r28 == lbl_1_bss_28);
    }
    for (var_r31 = 0; var_r31 < 3; var_r31++) {
        if ((lbl_1_bss_A0[var_r31].unknown048 >= 0) && (lbl_1_bss_A0[var_r31].unknown04C >= 0) && (lbl_1_bss_A0[var_r31].unknown050 > 0)) {
            lbl_1_bss_A0[var_r31].unknown050 -= 1;
            if (lbl_1_bss_A0[var_r31].unknown050 <= 0) {
                fn_1_3FD0((s16) var_r31, (s16) lbl_1_bss_A0[var_r31].unknown048, (s16) lbl_1_bss_A0[var_r31].unknown04C);
                lbl_1_bss_A0[var_r31].unknown048 = -1;
                lbl_1_bss_A0[var_r31].unknown04C = -1;
                lbl_1_bss_A0[var_r31].unknown050 = 0;
            } else if ((s32) (fn_1_2E3C((s16) var_r31) & 4) != 0) {
                fn_1_3FD0((s16) var_r31, (s16) (u8) var_r28, 0);
            } else {
                lbl_1_bss_28 = (u8) lbl_1_bss_A0[var_r31].unknown014;
            }
        }
        if (lbl_1_bss_A0[var_r31].unknown040 > 0) {
            lbl_1_bss_A0[var_r31].rot.y += lbl_1_bss_A0[var_r31].unknown044;
            if (lbl_1_bss_A0[var_r31].rot.y >= 360.0f) {
                lbl_1_bss_A0[var_r31].rot.y -= 360.0f;
            }
            if (lbl_1_bss_A0[var_r31].rot.y <= -360.0f) {
                lbl_1_bss_A0[var_r31].rot.y += 360.0f;
            }
            lbl_1_bss_A0[var_r31].unknown040 -= 1;
            if (lbl_1_bss_A0[var_r31].unknown040 <= 0) {
                lbl_1_bss_A0[var_r31].unknown040 = 0;
                lbl_1_bss_A0[var_r31].unknown044 = 0.0f;
                if (((-90.0f <= lbl_1_bss_A0[var_r31].rot.y) && (lbl_1_bss_A0[var_r31].rot.y <= 90.0f)) || (lbl_1_bss_A0[var_r31].rot.y <= -270.0f) || (270.0f <= lbl_1_bss_A0[var_r31].rot.y)) {
                    lbl_1_bss_A0[var_r31].rot.y = 0.0f;
                } else {
                    lbl_1_bss_A0[var_r31].rot.y = 180.0f;
                }
            }
            var_r30 = 0;
            while (var_r30 < 5) {
                Hu3DModelRotSetV(lbl_1_bss_A0[var_r31].model[var_r30], &lbl_1_bss_A0[var_r31].rot);
                var_r30 += 1;
            }
            var_r29 += 1;
        }
    }
    if (var_r29 < 3) {
        Hu3DModelPosSet(lbl_1_bss_30, lbl_1_rodata_84.x, lbl_1_rodata_84.y, lbl_1_rodata_84.z);
        Hu3DModelScaleSet(lbl_1_bss_30, lbl_1_rodata_9C.x, lbl_1_rodata_9C.y, lbl_1_rodata_9C.z);
        Hu3DModelAttrReset(lbl_1_bss_30, 1U);
        Hu3DMotionSpeedSet(lbl_1_bss_30, 2.0f);
    } else {
        Hu3DModelPosSet(lbl_1_bss_30, lbl_1_rodata_78.x, lbl_1_rodata_78.y, lbl_1_rodata_78.z);
        Hu3DModelScaleSet(lbl_1_bss_30, lbl_1_rodata_90.x, lbl_1_rodata_90.y, lbl_1_rodata_90.z);
        Hu3DModelAttrReset(lbl_1_bss_30, 1U);
        Hu3DMotionSpeedSet(lbl_1_bss_30, 4.0f);
    }
    if (lbl_1_bss_32 != 0) {
        Hu3DModelAttrSet(lbl_1_bss_34[0], 1U);
        Hu3DModelAttrReset(lbl_1_bss_34[1], 1U);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_34[0], 1U);
        Hu3DModelAttrSet(lbl_1_bss_34[1], 1U);
    }
    lbl_1_bss_32 -= 1;
    if (lbl_1_bss_32 < 0) {
        lbl_1_bss_32 = 0;
    }
}

void fn_1_2998(void)
{
    s16 angle;
    s16 count;

    angle = frand() % 60 + 90;
    count = angle / 10;
    count % 2 ? count-- : 0;
    if (lbl_1_bss_20 == 0) {
        count += 1;
    }
    lbl_1_bss_20 += 1;
    if (lbl_1_bss_20 > 10) {
        lbl_1_bss_20 = 10;
    }
    fn_1_2BB0(angle, count);
    count = 0.5f + (360.0f / lbl_1_bss_A0[0].unknown044) * ((count - 1) / 2.0f);
    fn_1_2CBC(lbl_1_bss_20, count);
}

s16 fn_1_2B28(void)
{
    s16 temp_r3;

    temp_r3 = fn_1_2D5C();
    if (temp_r3 != 3) {
        fn_1_2C00(temp_r3, 90, 2);
        if (temp_r3 == 0) { return 0; }
        if (temp_r3 == 1) { return 1; }
        if (temp_r3 == 2) { return 2; }
    }
    return 1;
}

void fn_1_2BB0(s16 arg0, s16 arg1)
{
    s32 var_r31;

    var_r31 = 0;
    while (var_r31 < 3) {
        fn_1_2C00((s16) var_r31, arg0, arg1);
        var_r31 += 1;
    }
}

void fn_1_2C00(s16 arg0, s16 arg1, s16 arg2)
{
    if ((arg0 < 0) || (arg0 > 2)) {
        return;
    }
    lbl_1_bss_A0[arg0].unknown040 = arg1;
    lbl_1_bss_A0[arg0].unknown044 = (180.0f * (f32) arg2) / (f32) arg1;
}

void fn_1_2CBC(s16 arg0, s32 arg1)
{
    s32 var_r31;

    arg0 -= 1;
    var_r31 = 0;
    while (var_r31 < 3) {
        lbl_1_bss_A0[var_r31].unknown048 = (s32) lbl_1_bss_3C[arg0].unk0;
        lbl_1_bss_A0[var_r31].unknown04C = (s32) lbl_1_bss_3C[arg0].choices[var_r31];
        lbl_1_bss_A0[var_r31].unknown050 = arg1;
        var_r31 += 1;
    }
}

s16 fn_1_2D5C(void)
{
    if ((lbl_1_bss_A0->unknown014 == lbl_1_bss_A0[1].unknown014) && (lbl_1_bss_A0->unknown018 == lbl_1_bss_A0[1].unknown018)) {
        return 2;
    }
    if ((lbl_1_bss_A0->unknown014 == lbl_1_bss_A0[2].unknown014) && (lbl_1_bss_A0->unknown018 == lbl_1_bss_A0[2].unknown018)) {
        return 1;
    }
    if ((lbl_1_bss_A0[1].unknown014 == lbl_1_bss_A0[2].unknown014) && (lbl_1_bss_A0[1].unknown018 == lbl_1_bss_A0[2].unknown018)) {
        return 0;
    }
    return 3;
}

s16 fn_1_2E3C(s16 arg0)
{
    if (lbl_1_bss_A0[arg0].unknown040 == 0) {
        if (((-90.0f < lbl_1_bss_A0[arg0].rot.y) && (lbl_1_bss_A0[arg0].rot.y < 90.0f)) || (lbl_1_bss_A0[arg0].rot.y < -270.0f) || (270.0f < lbl_1_bss_A0[arg0].rot.y)) {
            return 2;
        }
        return 1;
    }
    if (((-90.0f < lbl_1_bss_A0[arg0].rot.y) && (lbl_1_bss_A0[arg0].rot.y < 90.0f)) || (lbl_1_bss_A0[arg0].rot.y < -270.0f) || (270.0f < lbl_1_bss_A0[arg0].rot.y)) {
        return 8;
    }
    return 4;
}

void fn_1_2FDC(s16 arg0)
{
    if (lbl_1_bss_32 <= 0) {
        lbl_1_bss_32 = arg0;
    }
}

s32 fn_1_3000(s32 arg0)
{
    s16 var_r29;
    s16 var_r30;
    s16 var_r31;

    var_r29 = 0;
    for (var_r31 = 0; var_r31 < 3; var_r31++) {
        if (arg0 != 0) {
            lbl_1_bss_2A = 0;
            lbl_1_bss_A0[var_r31].unknown040 = 45;
            lbl_1_bss_A0[var_r31].unknown044 = (f32) (270 / lbl_1_bss_A0[var_r31].unknown040);
        }
        if (lbl_1_bss_A0[var_r31].unknown040 > 0) {
            lbl_1_bss_A0[var_r31].rot.y += lbl_1_bss_A0[var_r31].unknown044;
            if (lbl_1_bss_A0[var_r31].rot.y >= 360.0f) {
                lbl_1_bss_A0[var_r31].rot.y -= 360.0f;
            }
            if (lbl_1_bss_A0[var_r31].rot.y <= -360.0f) {
                lbl_1_bss_A0[var_r31].rot.y += 360.0f;
            }
            lbl_1_bss_A0[var_r31].unknown040 -= 1;
            if (lbl_1_bss_A0[var_r31].unknown040 <= 0) {
                lbl_1_bss_A0[var_r31].unknown040 = 0;
                lbl_1_bss_A0[var_r31].unknown044 = 0.0f;
                if (((0.0f <= lbl_1_bss_A0[var_r31].rot.y) && (lbl_1_bss_A0[var_r31].rot.y <= 180.0f)) || ((-360.0f <= lbl_1_bss_A0[var_r31].rot.y) && (lbl_1_bss_A0[var_r31].rot.y <= -180.0f))) {
                    lbl_1_bss_A0[var_r31].rot.y = 90.0f;
                } else {
                    lbl_1_bss_A0[var_r31].rot.y = 270.0f;
                }
                var_r30 = 0;
                while (var_r30 < 5) {
                    Hu3DModelAttrSet(lbl_1_bss_A0[var_r31].model[var_r30], 1U);
                    var_r30 += 1;
                }
                var_r29 += 1;
            }
            var_r30 = 0;
            while (var_r30 < 5) {
                Hu3DModelRotSetV(lbl_1_bss_A0[var_r31].model[var_r30], &lbl_1_bss_A0[var_r31].rot);
                var_r30 += 1;
            }
        }
    }
    Hu3DModelAttrSet(lbl_1_bss_30, 1U);
    if (var_r29 >= 3) {
        lbl_1_bss_2A = 1;
    }
    if (lbl_1_bss_2A > 0) {
        if (lbl_1_bss_2A > 5) {
            return 1;
        }
        lbl_1_bss_2A += 1;
    }
    return 0;
}

s16 fn_1_34A4(s16 arg0)
{
    f32 temp_f31;
    s16 var_r30;
    s16 var_r31;

    if (arg0 == 0) {
        fn_1_3000(1);
        lbl_1_bss_2C = 0;
    } else {
        if (lbl_1_bss_2C < 0) {
            if (lbl_1_bss_2C == -1) {
                var_r31 = 0;
                while (var_r31 < 2) {
                    Hu3DModelAttrSet(lbl_1_bss_34[var_r31], 1U);
                    var_r31 += 1;
                }
                Hu3DCameraViewportSet(lbl_1_data_214.cameraBit, lbl_1_data_214.viewportX, lbl_1_data_214.viewportY, lbl_1_data_214.viewportW, lbl_1_data_214.viewportH, lbl_1_data_214.minZ, lbl_1_data_214.maxZ);
                Hu3DCameraPerspectiveSet(lbl_1_data_214.cameraBit, lbl_1_data_214.fov, lbl_1_data_214.nearPlane, lbl_1_data_214.farPlane, lbl_1_data_214.aspect);
                Hu3DCameraScissorSet(lbl_1_data_214.cameraBit, (u32) lbl_1_data_214.viewportX, (u32) lbl_1_data_214.viewportY, (u32) lbl_1_data_214.viewportW, (u32) lbl_1_data_214.viewportH);
                Hu3DCameraPosSetV(lbl_1_data_214.cameraBit, &lbl_1_data_214.pos, &lbl_1_data_214.up, &lbl_1_data_214.target);
                Hu3DModelAttrReset(lbl_1_bss_2E, 1U);
                Hu3DMotionTimeSet(lbl_1_bss_2E, 0.0f);
                Hu3DModelAttrSet(lbl_1_bss_2E, 1073741825U);
                HuAudFXPlay(1576);
                fn_1_4C84();
                lbl_1_bss_2C -= 1;
            }
            if (lbl_1_bss_2C < -1) {
                temp_f31 = Hu3DMotionMaxTimeGet(lbl_1_bss_2E);
                if ((1.0f + Hu3DMotionTimeGet(lbl_1_bss_2E)) >= temp_f31) {
                    lbl_1_bss_2C = -512;
                    Hu3DModelAttrSet(lbl_1_bss_2E, 1073741826U);
                } else {
                    lbl_1_bss_2C = -20;
                }
            }
        }
        if (lbl_1_bss_2C > 0) {
            var_r30 = 0;
            var_r31 = 0;
            while (var_r31 < 2) {
                if ((Hu3DMotionTimeGet(lbl_1_bss_34[var_r31]) <= 50.0f) || (280.0f <= Hu3DMotionTimeGet(lbl_1_bss_34[var_r31]))) {
                    Hu3DModelAttrSet(lbl_1_bss_34[var_r31], 1073741826U);
                } else {
                    var_r30 += 1;
                }
                var_r31 += 1;
            }
            if (var_r30 == 0) {
                lbl_1_bss_2C = -1;
            }
        }
        if (lbl_1_bss_2C == 0) {
            var_r30 = fn_1_3000(0);
            if (var_r30 != 0) { lbl_1_bss_2C += 1; }
        }
    }
    fn_1_3908();
    if (lbl_1_bss_2C == -512) { return 1; }
    if (lbl_1_bss_2C == -1) { return -1; }
    return 0;
}

void fn_1_3908(void)
{
    s16 var_r31;

    var_r31 = 0;
    while (var_r31 < 2) {
        if ((Hu3DMotionTimeGet(lbl_1_bss_34[var_r31]) <= 50.0f) || (280.0f <= Hu3DMotionTimeGet(lbl_1_bss_34[var_r31]))) {
            Hu3DModelAttrSet(lbl_1_bss_34[var_r31], 1073741826U);
        }
        var_r31 += 1;
    }
}

void fn_1_39D0(void)
{
    s16 previous;
    s16 work;
    s16 selected;
    s16 other;
    s16 i;

    for (i = 0; i < 10; i++) {
        if (i == 0) {
            lbl_1_bss_3C[i].unk0 = (u8)frand() % 5;
        } else {
            work = i - 1;
            do {
                lbl_1_bss_3C[i].unk0 = (u8)frand() % 5;
            } while (lbl_1_bss_3C[i].unk0 == lbl_1_bss_3C[work].unk0);
        }
        lbl_1_bss_3C[i].choices[0] = 0;
        lbl_1_bss_3C[i].choices[1] = (u8)frand() % 4 + 1;
        lbl_1_bss_3C[i].choices[2] = lbl_1_bss_3C[i].choices[(u8)frand() % 2];
        other = (u8)frand() % 10;
        if (other < 5) {
            other = lbl_1_bss_3C[i].choices[0];
            lbl_1_bss_3C[i].choices[0] = lbl_1_bss_3C[i].choices[1];
            lbl_1_bss_3C[i].choices[1] = other;
        }
        other = (u8)frand() % 10;
        if (other < 5) {
            other = lbl_1_bss_3C[i].choices[1];
            lbl_1_bss_3C[i].choices[1] = lbl_1_bss_3C[i].choices[2];
            lbl_1_bss_3C[i].choices[2] = other;
        }
        other = (u8)frand() % 10;
        if (other < 5) {
            other = lbl_1_bss_3C[i].choices[0];
            lbl_1_bss_3C[i].choices[0] = lbl_1_bss_3C[i].choices[2];
            lbl_1_bss_3C[i].choices[2] = other;
        }
    }
    selected = previous = -1;
    for (i = 0; i < 10; i++) {
        if (lbl_1_bss_3C[i].choices[0] == lbl_1_bss_3C[i].choices[1]) {
            selected = 2;
        } else if (lbl_1_bss_3C[i].choices[0] == lbl_1_bss_3C[i].choices[2]) {
            selected = 1;
        } else {
            selected = 0;
        }
        if (previous >= 0 && previous == selected) {
            other = (u8)frand() % 100;
            if (other >= 4) {
                if (other < 52) {
                    other = selected + 1;
                    if (other > 2) { other = 0; }
                    if (other < 0) { other = 2; }
                    work = lbl_1_bss_3C[i].choices[selected];
                    lbl_1_bss_3C[i].choices[selected] = lbl_1_bss_3C[i].choices[other];
                    lbl_1_bss_3C[i].choices[other] = work;
                } else {
                    other = selected - 1;
                    if (other > 2) { other = 0; }
                    if (other < 0) { other = 2; }
                    work = lbl_1_bss_3C[i].choices[selected];
                    lbl_1_bss_3C[i].choices[selected] = lbl_1_bss_3C[i].choices[other];
                    lbl_1_bss_3C[i].choices[other] = work;
                }
            }
        }
        previous = selected;
        selected = -1;
    }
}

void fn_1_3FD0(s16 arg0, s16 arg1, s16 arg2)
{
    s16 var_r31;

    lbl_1_bss_A0[arg0].unknown014 = (s32) arg1;
    lbl_1_bss_A0[arg0].unknown018 = (s32) arg2;
    var_r31 = 0;
    while (var_r31 < 5) {
        if (var_r31 != arg1) {
            Hu3DModelAttrSet(lbl_1_bss_A0[arg0].model[var_r31], 1U);
        }
        if (var_r31 == arg1) {
            Hu3DModelAttrReset(lbl_1_bss_A0[arg0].model[var_r31], 1U);
        }
        Hu3DAnimBankSet((&lbl_1_bss_A0[arg0].model[var_r31])[5], (u16) lbl_1_bss_A0[arg0].unknown018);
        var_r31 += 1;
    }
}
