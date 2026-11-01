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

s32 lbl_1_data_2E8[2] = { DATANUM(0x3d0000, 23), DATANUM(0x3d0000, 27) };

s32 lbl_1_data_2F0[4] = {
    DATANUM(0x3d0000, 24), DATANUM(0x3d0000, 26), DATANUM(0x3d0000, 28), DATANUM(0x3d0000, 30)
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
        fn_800286D0(&lbl_1_data_278.pos, &lbl_1_data_278.dir, &lbl_1_data_278.color);
    Hu3DGLightStaticSet(lbl_1_data_278.id, 1);
    Hu3DGLightInfinitytSet(lbl_1_data_278.id);
    lbl_1_bss_23C[0] =
        fn_80025A04(fn_80008DD0(DATANUM(0x3d0000, 1), 0x10000000, 2));
    lbl_1_bss_23C[1] =
        fn_80025A04(fn_80008DD0(DATANUM(0x3d0000, 2), 0x10000000, 2));
    lbl_1_bss_23C[2] =
        fn_80025A04(fn_80008DD0(DATANUM(0x3d0000, 3), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_23C[0], 8U);
    fn_80026B64(lbl_1_bss_23C[1], 8U);
    fn_80026B64(lbl_1_bss_23C[2], 8U);
    fn_80026B8C(lbl_1_bss_23C[0], 1);
    fn_80026B8C(lbl_1_bss_23C[1], 2);
    fn_80026B8C(lbl_1_bss_23C[2], 0);
    fn_80026FFC(lbl_1_bss_23C[2]);
    stageModelIndex = 0;
    while (stageModelIndex < 3) {
        lbl_1_bss_236[stageModelIndex] = fn_80025A04(
            fn_80008DD0(DATANUM(0x3d0000, 13), 0x10000000, 2));
        fn_80026B64(lbl_1_bss_236[stageModelIndex], 8U);
        fn_80026B8C(lbl_1_bss_236[stageModelIndex], 2);
        fn_800269E8(lbl_1_bss_236[stageModelIndex], ((1 << 30) | (1 << 0)));
        fn_800269E8(lbl_1_bss_236[stageModelIndex], (1 << 0));
        fn_8002D86C(lbl_1_bss_236[stageModelIndex], 1.2f);
        stageModelIndex += 1;
    }
    fn_80026760(lbl_1_bss_236[0], -400.0f, 0.0f, 0.0f);
    fn_80026760(lbl_1_bss_236[1], 0.0f, 0.0f, 0.0f);
    fn_80026760(lbl_1_bss_236[2], 400.0f, 0.0f, 0.0f);
    lbl_1_bss_234 = 0;
    particleAnim =
        HuSprAnimRead(fn_80008DD0(DATANUM(0x3d0000, 14), 0x10000000, 2));
    lbl_1_bss_232 = fn_800334F4(particleAnim, 40, &lbl_1_data_298);
    /* Start with emission stopped; the winner sequence clears the time-up flag. */
    Hu3DParManAttrSet(lbl_1_bss_232, (1 << 0) | (1 << 1) |
                                         (1 << 4));
    fn_8003269C(Hu3DParManModelIDGet(lbl_1_bss_232), 1);
    Hu3DParManRotSet(lbl_1_bss_232, 0.0f, 0.0f, 0.0f);
    fn_80026B64(Hu3DParManModelIDGet(lbl_1_bss_232), 4U);
    fn_80026B8C(Hu3DParManModelIDGet(lbl_1_bss_232), 6);
    Hu3DParManPosSet(lbl_1_bss_232, 0.0f, 120.0f, 1.0f);
    lbl_1_bss_1A0 =
        fn_80025A04(fn_80008DD0(DATANUM(0x3d0000, 12), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_1A0, 8U);
    fn_80026B8C(lbl_1_bss_1A0, 1);
    fn_80026E44(lbl_1_bss_1A0);
    for (stageModelIndex = 0; stageModelIndex < 24; stageModelIndex++) {
        /* Four groups use the first model family; the remaining hooks use the second. */
        if (((stageModelIndex >= 0) && (stageModelIndex <= 3)) ||
            ((stageModelIndex >= 4) && (stageModelIndex <= 7)) ||
            ((stageModelIndex >= 12) && (stageModelIndex <= 15)) ||
            ((stageModelIndex >= 16) && (stageModelIndex <= 19))) {
            lbl_1_bss_202[stageModelIndex] =
                fn_80025A04(fn_80008DD0(lbl_1_data_2E8[0], 0x10000000, 2));
            motionIndex = 0;
            while (motionIndex < 2) {
                lbl_1_bss_1A2[stageModelIndex][motionIndex] = fn_80030458(
                    lbl_1_bss_202[stageModelIndex],
                    fn_80008DD0(lbl_1_data_2F0[motionIndex], 0x10000000, 2));
                motionIndex += 1;
            }
            fn_80026E44(lbl_1_bss_202[stageModelIndex]);
        } else {
            lbl_1_bss_202[stageModelIndex] =
                fn_80025A04(fn_80008DD0(lbl_1_data_2E8[1], 0x10000000, 2));
            motionIndex = 0;
            while (motionIndex < 2) {
                lbl_1_bss_1A2[stageModelIndex][motionIndex] =
                    fn_80030458(lbl_1_bss_202[stageModelIndex],
                                    fn_80008DD0((&lbl_1_data_2F0[motionIndex])[2],
                                                         0x10000000, 2));
                motionIndex += 1;
            }
        }
        fn_80026B64(lbl_1_bss_202[stageModelIndex], 8U);
        fn_80026B8C(lbl_1_bss_202[stageModelIndex], 1);
        fn_8002C95C(lbl_1_bss_202[stageModelIndex], lbl_1_bss_1A2[stageModelIndex][0]);
        fn_800269E8(lbl_1_bss_202[stageModelIndex], ((1 << 30) | (1 << 0)));
        if ((stageModelIndex == 4) || (stageModelIndex == 16)) {
            fn_800269E8(lbl_1_bss_202[stageModelIndex], (1 << 0));
        }
        fn_8002741C(lbl_1_bss_1A0, lbl_1_data_370[stageModelIndex],
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
        fn_8002CCC0(lbl_1_bss_202[attachmentIndex], lbl_1_bss_1A2[attachmentIndex][1],
                           motionTime, 8.0f, ((1 << 30) | (1 << 0)));
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
        fn_8002CCC0(lbl_1_bss_202[attachmentIndex], lbl_1_bss_1A2[attachmentIndex][0],
                           motionTime, 8.0f, ((1 << 30) | (1 << 0)));
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
            fn_80026A44(lbl_1_bss_236[effectModelIndex], (1 << 0));
            fn_800269E8(lbl_1_bss_236[effectModelIndex], ((1 << 30) | (1 << 0)));
            fn_8002D86C(lbl_1_bss_236[effectModelIndex], 1.2f);
            fn_8002D414(lbl_1_bss_236[effectModelIndex], 0.0f);
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
            fn_800269E8(lbl_1_bss_236[effectModelIndex], (1 << 0));
            fn_800269E8(lbl_1_bss_236[effectModelIndex], ((1 << 30) | (1 << 0)));
            fn_8002D86C(lbl_1_bss_236[effectModelIndex], 1.2f);
            effectModelIndex += 1;
        }
    }
}

/* The winner-stage sequence starts 480 frames of particle emission and plays its accompanying
 * sound. */
void fn_1_4C84(void)
{
    Hu3DParManTimeLimitSet(lbl_1_bss_232, 480);
    Hu3DParManAttrReset(lbl_1_bss_232, (1 << 0));
    HuAudFXPlay(MSM_SE_M602_WINNER_PARTICLES);
}
