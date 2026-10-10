/* Defines Odd Card Out lighting, particles, and effect resources. */
#define _MATH_H
#include "REL/m602Dll.h"

#define MSM_SE_M602_CORRECT_CHOICE_EFFECT 1574
#define MSM_SE_M602_WINNER_PARTICLES 1577

M602Light lbl_1_data_278 = {
    0, { 0.0f, 1500.0f, 1000.0f }, { 0.0f, -1500.0f, -1000.0f }, { 255, 255, 255, 255 }
};

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

s32 lbl_1_data_2E8[2] = { DATANUM(DATA_m602, 23), DATANUM(DATA_m602, 27) };

s32 lbl_1_data_2F0[4] = {
    DATANUM(DATA_m602, 24), DATANUM(DATA_m602, 26), DATANUM(DATA_m602, 28), DATANUM(DATA_m602, 30)
};

char lbl_1_data_300[4] = "f-1";

char lbl_1_data_304[4] = "f-2";

char lbl_1_data_308[4] = "f-3";

char lbl_1_data_30C[4] = "f-4";

char lbl_1_data_310[4] = "f-5";

char lbl_1_data_314[4] = "f-6";

char lbl_1_data_318[4] = "f-7";

char lbl_1_data_31C[4] = "f-8";

char lbl_1_data_320[4] = "f-9";

char lbl_1_data_324[5] = "f-10";

char lbl_1_data_329[5] = "f-11";

char lbl_1_data_32E[5] = "f-12";

char lbl_1_data_333[5] = "f-13";

char lbl_1_data_338[5] = "f-14";

char lbl_1_data_33D[5] = "f-15";

char lbl_1_data_342[5] = "f-16";

char lbl_1_data_347[5] = "f-17";

char lbl_1_data_34C[5] = "f-18";

char lbl_1_data_351[5] = "f-19";

char lbl_1_data_356[5] = "f-20";

char lbl_1_data_35B[5] = "f-21";

char lbl_1_data_360[5] = "f-22";

char lbl_1_data_365[5] = "f-23";

char lbl_1_data_36A[6] = "f-24";

char *lbl_1_data_370[24] = {
    lbl_1_data_300, lbl_1_data_304, lbl_1_data_308, lbl_1_data_30C,
    lbl_1_data_310, lbl_1_data_314, lbl_1_data_318, lbl_1_data_31C,
    lbl_1_data_320, lbl_1_data_324, lbl_1_data_329, lbl_1_data_32E,
    lbl_1_data_333, lbl_1_data_338, lbl_1_data_33D, lbl_1_data_342,
    lbl_1_data_347, lbl_1_data_34C, lbl_1_data_351, lbl_1_data_356,
    lbl_1_data_35B, lbl_1_data_360, lbl_1_data_365, lbl_1_data_36A,
};

/* Stage scenery models rendered by the main camera. */
s16 lbl_1_bss_23C[3];

/* Three animated models shown during the correct-choice effect. */
s16 lbl_1_bss_236[3];

/* Frames remaining in the 147-frame correct-choice effect; zero hides its models. */
s16 lbl_1_bss_234;

/* Particle manager used by the final winner-stage effect. */
s16 lbl_1_bss_232;

/* Models attached at the stage's f-1 through f-24 hooks. */
s16 lbl_1_bss_202[24];

/* Two motion IDs for each attached stage model. */
s16 lbl_1_bss_1A2[24][2];

/* Parent model containing the 24 named attachment hooks. */
s16 lbl_1_bss_1A0;

/* Called by intro setup to create fixed stage lighting, scenery, attached models, and result
 * effects. */
void fn_1_411C(void)
{
    s16 motionIndex;
    s16 stageModelIndex;
    ANIMDATA *particleAnim;

    lbl_1_data_278.id =
        Hu3DGLightCreateV(&lbl_1_data_278.pos, &lbl_1_data_278.dir, &lbl_1_data_278.color);
    Hu3DGLightStaticSet(lbl_1_data_278.id, 1);
    Hu3DGLightInfinitytSet(lbl_1_data_278.id);
    lbl_1_bss_23C[0] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m602, 1), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_23C[1] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m602, 2), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_23C[2] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m602, 3), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_23C[0], 8U);
    Hu3DModelCameraSet(lbl_1_bss_23C[1], 8U);
    Hu3DModelCameraSet(lbl_1_bss_23C[2], 8U);
    Hu3DModelLayerSet(lbl_1_bss_23C[0], 1);
    Hu3DModelLayerSet(lbl_1_bss_23C[1], 2);
    Hu3DModelLayerSet(lbl_1_bss_23C[2], 0);
    Hu3DModelShadowMapSet(lbl_1_bss_23C[2]);
    stageModelIndex = 0;
    while (stageModelIndex < 3) {
        lbl_1_bss_236[stageModelIndex] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m602, 13), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_236[stageModelIndex], 8U);
        Hu3DModelLayerSet(lbl_1_bss_236[stageModelIndex], 2);
        Hu3DModelAttrSet(lbl_1_bss_236[stageModelIndex], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(lbl_1_bss_236[stageModelIndex], HU3D_ATTR_DISPOFF);
        Hu3DMotionSpeedSet(lbl_1_bss_236[stageModelIndex], 1.2f);
        stageModelIndex += 1;
    }
    Hu3DModelPosSet(lbl_1_bss_236[0], -400.0f, 0.0f, 0.0f);
    Hu3DModelPosSet(lbl_1_bss_236[1], 0.0f, 0.0f, 0.0f);
    Hu3DModelPosSet(lbl_1_bss_236[2], 400.0f, 0.0f, 0.0f);
    lbl_1_bss_234 = 0;
    particleAnim =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_m602, 14), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_232 = Hu3DParManCreate(particleAnim, 40, &lbl_1_data_298);
    /* Start with emission stopped; the winner sequence clears the time-up flag. */
    Hu3DParManAttrSet(lbl_1_bss_232, HU3D_PARMAN_ATTR_TIMEUP | HU3D_PARMAN_ATTR_RANDSPEED90 |
                                         HU3D_PARMAN_ATTR_RANDSCALE90);
    Hu3DParticleBlendModeSet(Hu3DParManModelIDGet(lbl_1_bss_232), HU3D_PARTICLE_BLEND_ADDCOL);
    Hu3DParManRotSet(lbl_1_bss_232, 0.0f, 0.0f, 0.0f);
    Hu3DModelCameraSet(Hu3DParManModelIDGet(lbl_1_bss_232), 4U);
    Hu3DModelLayerSet(Hu3DParManModelIDGet(lbl_1_bss_232), 6);
    Hu3DParManPosSet(lbl_1_bss_232, 0.0f, 120.0f, 1.0f);
    lbl_1_bss_1A0 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m602, 12), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_1A0, 8U);
    Hu3DModelLayerSet(lbl_1_bss_1A0, 1);
    Hu3DModelShadowSet(lbl_1_bss_1A0);
    for (stageModelIndex = 0; stageModelIndex < 24; stageModelIndex++) {
        /* Four groups use the first model family; the remaining hooks use the second. */
        if (((stageModelIndex >= 0) && (stageModelIndex <= 3)) ||
            ((stageModelIndex >= 4) && (stageModelIndex <= 7)) ||
            ((stageModelIndex >= 12) && (stageModelIndex <= 15)) ||
            ((stageModelIndex >= 16) && (stageModelIndex <= 19))) {
            lbl_1_bss_202[stageModelIndex] =
                Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_2E8[0], HU_MEMNUM_OVL, HEAP_MODEL));
            motionIndex = 0;
            while (motionIndex < 2) {
                lbl_1_bss_1A2[stageModelIndex][motionIndex] = Hu3DJointMotion(
                    lbl_1_bss_202[stageModelIndex],
                    HuDataSelHeapReadNum(lbl_1_data_2F0[motionIndex], HU_MEMNUM_OVL, HEAP_MODEL));
                motionIndex += 1;
            }
            Hu3DModelShadowSet(lbl_1_bss_202[stageModelIndex]);
        } else {
            lbl_1_bss_202[stageModelIndex] =
                Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_2E8[1], HU_MEMNUM_OVL, HEAP_MODEL));
            motionIndex = 0;
            while (motionIndex < 2) {
                lbl_1_bss_1A2[stageModelIndex][motionIndex] =
                    Hu3DJointMotion(lbl_1_bss_202[stageModelIndex],
                                    HuDataSelHeapReadNum((&lbl_1_data_2F0[motionIndex])[2],
                                                         HU_MEMNUM_OVL, HEAP_MODEL));
                motionIndex += 1;
            }
        }
        Hu3DModelCameraSet(lbl_1_bss_202[stageModelIndex], 8U);
        Hu3DModelLayerSet(lbl_1_bss_202[stageModelIndex], 1);
        Hu3DMotionSet(lbl_1_bss_202[stageModelIndex], lbl_1_bss_1A2[stageModelIndex][0]);
        Hu3DModelAttrSet(lbl_1_bss_202[stageModelIndex], HU3D_MOTATTR_LOOP);
        if ((stageModelIndex == 4) || (stageModelIndex == 16)) {
            Hu3DModelAttrSet(lbl_1_bss_202[stageModelIndex], HU3D_ATTR_DISPOFF);
        }
        Hu3DModelHookSet(lbl_1_bss_1A0, lbl_1_data_370[stageModelIndex],
                         lbl_1_bss_202[stageModelIndex]);
    }
    fn_1_48B4();
    fn_1_4A98();
}

/* Intro, player, and winner callbacks blend the attached stage models to their second looping
 * motion. */
void fn_1_48B4(void)
{
    s16 attachmentIndex;
    f32 motionTime;

    attachmentIndex = 0;
    while (attachmentIndex < 24) {
        /* Offset each loop so the attached models do not animate in unison. */
        motionTime = (u8)frand() % 20;
        Hu3DMotionShiftSet(lbl_1_bss_202[attachmentIndex], lbl_1_bss_1A2[attachmentIndex][1],
                           motionTime, 8.0f, HU3D_MOTATTR_LOOP);
        attachmentIndex += 1;
    }
}

/* Intro and player callbacks blend the attached stage models back to their first looping motion. */
void fn_1_4998(void)
{
    s16 attachmentIndex;
    f32 motionTime;

    attachmentIndex = 0;
    while (attachmentIndex < 24) {
        /* Offset each loop so the attached models do not animate in unison. */
        motionTime = (u8)frand() % 60;
        Hu3DMotionShiftSet(lbl_1_bss_202[attachmentIndex], lbl_1_bss_1A2[attachmentIndex][0],
                           motionTime, 8.0f, HU3D_MOTATTR_LOOP);
        attachmentIndex += 1;
    }
}

/* A correct card choice starts the 147-frame stage effect and returns its duration to the round
 * callback. */
s16 fn_1_4A80(void)
{
    lbl_1_bss_234 = 147;
    return 147;
}

/* Intro setup and the round callback clear the correct-choice effect countdown. */
void fn_1_4A98(void)
{
    lbl_1_bss_234 = 0;
}

/* Sequence callbacks update the correct-choice effect each frame, restarting or hiding all three
 * models. */
void fn_1_4AAC(void)
{
    s16 effectModelIndex;

    if ((lbl_1_bss_234 != 0) && (lbl_1_bss_234 == 147)) {
        HuAudFXPlay(MSM_SE_M602_CORRECT_CHOICE_EFFECT);
        effectModelIndex = 0;
        while (effectModelIndex < 3) {
            Hu3DModelAttrReset(lbl_1_bss_236[effectModelIndex], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_236[effectModelIndex], HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(lbl_1_bss_236[effectModelIndex], 1.2f);
            Hu3DMotionTimeSet(lbl_1_bss_236[effectModelIndex], 0.0f);
            effectModelIndex += 1;
        }
    }
    lbl_1_bss_234 -= 1;
    if (lbl_1_bss_234 <= 0) {
        lbl_1_bss_234 = 0;
    }
    if (lbl_1_bss_234 == 0) {
        effectModelIndex = 0;
        while (effectModelIndex < 3) {
            Hu3DModelAttrSet(lbl_1_bss_236[effectModelIndex], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_236[effectModelIndex], HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(lbl_1_bss_236[effectModelIndex], 1.2f);
            effectModelIndex += 1;
        }
    }
}

/* The winner-stage sequence starts 480 frames of particle emission and plays its accompanying
 * sound. */
void fn_1_4C84(void)
{
    Hu3DParManTimeLimitSet(lbl_1_bss_232, 480);
    Hu3DParManAttrReset(lbl_1_bss_232, HU3D_PARMAN_ATTR_TIMEUP);
    HuAudFXPlay(MSM_SE_M602_WINNER_PARTICLES);
}
