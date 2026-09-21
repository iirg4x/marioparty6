#define _MATH_H
#include "REL/m602Dll.h"

M602Light lbl_1_data_278 = { 0, { 0.0f, 1500.0f, 1000.0f }, { 0.0f, -1500.0f, -1000.0f }, { 255, 255, 255, 255 } };

HU3D_PARMAN_PARAM lbl_1_data_298 = {
    20,
    0,
    2.0f,
    180.0f,
    90.0f,
    { 0.0f, 0.001f, 0.0f },
    0.1f,
    0.0f,
    16.0f,
    1.0f,
    2,
    {
        { 255, 255, 255, 255 },
        { 255, 255, 255, 255 },
        { 0, 0, 0, 0 },
        { 0, 0, 0, 0 },
    },
    {
        { 255, 255, 255, 128 },
        { 255, 255, 255, 128 },
        { 0, 0, 0, 0 },
        { 0, 0, 0, 0 },
    },
};

s32 lbl_1_data_2E8[2] = { 3997719, 3997723 };

s32 lbl_1_data_2F0[4] = { 3997720, 3997722, 3997724, 3997726 };

char lbl_1_data_300[4] = { 102, 45, 49, 0 };

char lbl_1_data_304[4] = { 102, 45, 50, 0 };

char lbl_1_data_308[4] = { 102, 45, 51, 0 };

char lbl_1_data_30C[4] = { 102, 45, 52, 0 };

char lbl_1_data_310[4] = { 102, 45, 53, 0 };

char lbl_1_data_314[4] = { 102, 45, 54, 0 };

char lbl_1_data_318[4] = { 102, 45, 55, 0 };

char lbl_1_data_31C[4] = { 102, 45, 56, 0 };

char lbl_1_data_320[4] = { 102, 45, 57, 0 };

char lbl_1_data_324[5] = { 102, 45, 49, 48, 0 };

char lbl_1_data_329[5] = { 102, 45, 49, 49, 0 };

char lbl_1_data_32E[5] = { 102, 45, 49, 50, 0 };

char lbl_1_data_333[5] = { 102, 45, 49, 51, 0 };

char lbl_1_data_338[5] = { 102, 45, 49, 52, 0 };

char lbl_1_data_33D[5] = { 102, 45, 49, 53, 0 };

char lbl_1_data_342[5] = { 102, 45, 49, 54, 0 };

char lbl_1_data_347[5] = { 102, 45, 49, 55, 0 };

char lbl_1_data_34C[5] = { 102, 45, 49, 56, 0 };

char lbl_1_data_351[5] = { 102, 45, 49, 57, 0 };

char lbl_1_data_356[5] = { 102, 45, 50, 48, 0 };

char lbl_1_data_35B[5] = { 102, 45, 50, 49, 0 };

char lbl_1_data_360[5] = { 102, 45, 50, 50, 0 };

char lbl_1_data_365[5] = { 102, 45, 50, 51, 0 };

char lbl_1_data_36A[6] = { 102, 45, 50, 52, 0, 0 };

char *lbl_1_data_370[24] = {
    lbl_1_data_300,
    lbl_1_data_304,
    lbl_1_data_308,
    lbl_1_data_30C,
    lbl_1_data_310,
    lbl_1_data_314,
    lbl_1_data_318,
    lbl_1_data_31C,
    lbl_1_data_320,
    lbl_1_data_324,
    lbl_1_data_329,
    lbl_1_data_32E,
    lbl_1_data_333,
    lbl_1_data_338,
    lbl_1_data_33D,
    lbl_1_data_342,
    lbl_1_data_347,
    lbl_1_data_34C,
    lbl_1_data_351,
    lbl_1_data_356,
    lbl_1_data_35B,
    lbl_1_data_360,
    lbl_1_data_365,
    lbl_1_data_36A,
};

s16 lbl_1_bss_23C[3];

s16 lbl_1_bss_236[3];

s16 lbl_1_bss_234;

s16 lbl_1_bss_232;

s16 lbl_1_bss_202[24];

s16 lbl_1_bss_1A2[24][2];

s16 lbl_1_bss_1A0;

void fn_1_411C(void)
{
    s16 var_r30;
    s16 var_r31;
    ANIMDATA *particleAnim;

    lbl_1_data_278.id = Hu3DGLightCreateV(&lbl_1_data_278.pos, &lbl_1_data_278.dir, &lbl_1_data_278.color);
    Hu3DGLightStaticSet(lbl_1_data_278.id, 1);
    Hu3DGLightInfinitytSet(lbl_1_data_278.id);
    lbl_1_bss_23C[0] = Hu3DModelCreate(HuDataSelHeapReadNum(3997697, 268435456, HEAP_MODEL));
    lbl_1_bss_23C[1] = Hu3DModelCreate(HuDataSelHeapReadNum(3997698, 268435456, HEAP_MODEL));
    lbl_1_bss_23C[2] = Hu3DModelCreate(HuDataSelHeapReadNum(3997699, 268435456, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_23C[0], 8U);
    Hu3DModelCameraSet(lbl_1_bss_23C[1], 8U);
    Hu3DModelCameraSet(lbl_1_bss_23C[2], 8U);
    Hu3DModelLayerSet(lbl_1_bss_23C[0], 1);
    Hu3DModelLayerSet(lbl_1_bss_23C[1], 2);
    Hu3DModelLayerSet(lbl_1_bss_23C[2], 0);
    Hu3DModelShadowMapSet(lbl_1_bss_23C[2]);
    var_r31 = 0;
    while (var_r31 < 3) {
        lbl_1_bss_236[var_r31] = Hu3DModelCreate(HuDataSelHeapReadNum(3997709, 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_236[var_r31], 8U);
        Hu3DModelLayerSet(lbl_1_bss_236[var_r31], 2);
        Hu3DModelAttrSet(lbl_1_bss_236[var_r31], 1073741825U);
        Hu3DModelAttrSet(lbl_1_bss_236[var_r31], 1U);
        Hu3DMotionSpeedSet(lbl_1_bss_236[var_r31], 1.2f);
        var_r31 += 1;
    }
    Hu3DModelPosSet(lbl_1_bss_236[0], -400.0f, 0.0f, 0.0f);
    Hu3DModelPosSet(lbl_1_bss_236[1], 0.0f, 0.0f, 0.0f);
    Hu3DModelPosSet(lbl_1_bss_236[2], 400.0f, 0.0f, 0.0f);
    lbl_1_bss_234 = 0;
    particleAnim = HuSprAnimRead(HuDataSelHeapReadNum(3997710, 268435456, HEAP_MODEL));
    lbl_1_bss_232 = Hu3DParManCreate(particleAnim, 40, &lbl_1_data_298);
    Hu3DParManAttrSet(lbl_1_bss_232, 19);
    Hu3DParticleBlendModeSet(Hu3DParManModelIDGet(lbl_1_bss_232), 1U);
    Hu3DParManRotSet(lbl_1_bss_232, 0.0f, 0.0f, 0.0f);
    Hu3DModelCameraSet(Hu3DParManModelIDGet(lbl_1_bss_232), 4U);
    Hu3DModelLayerSet(Hu3DParManModelIDGet(lbl_1_bss_232), 6);
    Hu3DParManPosSet(lbl_1_bss_232, 0.0f, 120.0f, 1.0f);
    lbl_1_bss_1A0 = Hu3DModelCreate(HuDataSelHeapReadNum(3997708, 268435456, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_1A0, 8U);
    Hu3DModelLayerSet(lbl_1_bss_1A0, 1);
    Hu3DModelShadowSet(lbl_1_bss_1A0);
    for (var_r31 = 0; var_r31 < 24; var_r31++) {
        if (((var_r31 >= 0) && (var_r31 <= 3)) || ((var_r31 >= 4) && (var_r31 <= 7)) || ((var_r31 >= 12) && (var_r31 <= 15)) || ((var_r31 >= 16) && (var_r31 <= 19))) {
            lbl_1_bss_202[var_r31] = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_2E8[0], 268435456, HEAP_MODEL));
            var_r30 = 0;
            while (var_r30 < 2) {
                lbl_1_bss_1A2[var_r31][var_r30] = Hu3DJointMotion(lbl_1_bss_202[var_r31], HuDataSelHeapReadNum(lbl_1_data_2F0[var_r30], 268435456, HEAP_MODEL));
                var_r30 += 1;
            }
            Hu3DModelShadowSet(lbl_1_bss_202[var_r31]);
        } else {
            lbl_1_bss_202[var_r31] = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_2E8[1], 268435456, HEAP_MODEL));
            var_r30 = 0;
            while (var_r30 < 2) {
                lbl_1_bss_1A2[var_r31][var_r30] = Hu3DJointMotion(lbl_1_bss_202[var_r31], HuDataSelHeapReadNum((&lbl_1_data_2F0[var_r30])[2], 268435456, HEAP_MODEL));
                var_r30 += 1;
            }
        }
        Hu3DModelCameraSet(lbl_1_bss_202[var_r31], 8U);
        Hu3DModelLayerSet(lbl_1_bss_202[var_r31], 1);
        Hu3DMotionSet(lbl_1_bss_202[var_r31], lbl_1_bss_1A2[var_r31][0]);
        Hu3DModelAttrSet(lbl_1_bss_202[var_r31], 1073741825U);
        if ((var_r31 == 4) || (var_r31 == 16)) {
            Hu3DModelAttrSet(lbl_1_bss_202[var_r31], 1U);
        }
        Hu3DModelHookSet(lbl_1_bss_1A0, lbl_1_data_370[var_r31], lbl_1_bss_202[var_r31]);
    }
    fn_1_48B4();
    fn_1_4A98();
}

void fn_1_48B4(void)
{
    s16 var_r31;
    f32 motionTime;

    var_r31 = 0;
    while (var_r31 < 24) {
        motionTime = (u8)frand() % 20;
        Hu3DMotionShiftSet(lbl_1_bss_202[var_r31], lbl_1_bss_1A2[var_r31][1], motionTime, 8.0f, 1073741825U);
        var_r31 += 1;
    }
}

void fn_1_4998(void)
{
    s16 var_r31;
    f32 motionTime;

    var_r31 = 0;
    while (var_r31 < 24) {
        motionTime = (u8)frand() % 60;
        Hu3DMotionShiftSet(lbl_1_bss_202[var_r31], lbl_1_bss_1A2[var_r31][0], motionTime, 8.0f, 1073741825U);
        var_r31 += 1;
    }
}

s16 fn_1_4A80(void)
{
    lbl_1_bss_234 = 147;
    return 147;
}

void fn_1_4A98(void)
{
    lbl_1_bss_234 = 0;
}

void fn_1_4AAC(void)
{
    s16 var_r31;

    if ((lbl_1_bss_234 != 0) && (lbl_1_bss_234 == 147)) {
        HuAudFXPlay(1574);
        var_r31 = 0;
        while (var_r31 < 3) {
            Hu3DModelAttrReset(lbl_1_bss_236[var_r31], 1U);
            Hu3DModelAttrSet(lbl_1_bss_236[var_r31], 1073741825U);
            Hu3DMotionSpeedSet(lbl_1_bss_236[var_r31], 1.2f);
            Hu3DMotionTimeSet(lbl_1_bss_236[var_r31], 0.0f);
            var_r31 += 1;
        }
    }
    lbl_1_bss_234 -= 1;
    if (lbl_1_bss_234 <= 0) {
        lbl_1_bss_234 = 0;
    }
    if (lbl_1_bss_234 == 0) {
        var_r31 = 0;
        while (var_r31 < 3) {
            Hu3DModelAttrSet(lbl_1_bss_236[var_r31], 1U);
            Hu3DModelAttrSet(lbl_1_bss_236[var_r31], 1073741825U);
            Hu3DMotionSpeedSet(lbl_1_bss_236[var_r31], 1.2f);
            var_r31 += 1;
        }
    }
}

void fn_1_4C84(void)
{
    Hu3DParManTimeLimitSet(lbl_1_bss_232, 480);
    Hu3DParManAttrReset(lbl_1_bss_232, 1);
    HuAudFXPlay(1577);
}
