/* Camera, model and shadow helpers used during the rolling course sequence. */
#include <math.h>
#include "REL/m608dll.h"

/* Called by the intro and run callbacks in sequence.c to follow the current course hook. */
void fn_1_5DA4(Point3d position)
{
    lbl_1_bss_3E80.shadowCenter.x = position.x;
    lbl_1_bss_3E80.shadowCenter.y = (f32) (3500.0f + position.y);
    lbl_1_bss_3E80.shadowCenter.z = position.z;
    lbl_1_bss_3E80.shadowTarget.x = position.x;
    lbl_1_bss_3E80.shadowTarget.y = -100.0f;
    lbl_1_bss_3E80.shadowTarget.z = position.z;
    Hu3DShadowPosSet(&lbl_1_bss_3E80.shadowCenter, &lbl_1_bss_3E80.shadowUpDirection,
                     &lbl_1_bss_3E80.shadowTarget);
}

/* Called by the intro and rider-state callbacks in sequence.c to switch camera motions. */
void fn_1_5E6C(s32 cameraIndex)
{
    s16 cameraMotionId;
    s16 cameraModelId;

    cameraMotionId = lbl_1_bss_3E80.cameraMotions[cameraIndex];
    cameraModelId = lbl_1_bss_3E80.cameraModels[cameraIndex];
    fn_800284C8(cameraModelId, 1U);
    lbl_1_bss_3E80.cameraTime = 0.0f;
    lbl_1_bss_3E80.cameraMaxTime = fn_8002D530(cameraModelId);
    if (lbl_1_bss_3E80.activeCameraMotion != -1) {
        fn_8002858C(lbl_1_bss_3E80.activeCameraMotion);
    }
    lbl_1_bss_3E80.activeCameraMotion = cameraModelId;
    fn_80028478(lbl_1_bss_3E80.activeCameraMotion, 1U);
    OSReport(
        "start camera motion ... idx:%d time:%f\012", cameraIndex,
        /* m608 preset float one bits. */
        lbl_1_bss_3E80.cameraMaxTime);
/* m608 preset float half bits. */
/* m608 preset header bits. */
/* m608 preset float 100 bits. */
}

/* These words are stored preset data; their meanings are not established by this module. */
/* m608 preset word zero. */
/* m608 preset float negative 0 3 bits. */
/* m608 preset float 180 bits. */
/* m608 initial camera height. */
/* m608 preset shared data bits. */
/* m608 preset word all bits. */
/* m608 preset upper half bits. */
/* m608 preset lower half bits. */
M608Unknown80 lbl_1_data_320 = { { 0x07080000, 0x3F000000 },
                                 1000.0f,
                                 { 0x43340000, 0,
                                   0xBE99999AU, 0,
                                   0x3F800000, 0x3F800000,
                                   0x42C80000, 0x3F800000,
                                   0x0002FFFF, 0xFFFFFFFFU,
                                   0xFFFF0000U, 0,
                                   0x0000FFFF, 0xFFFFFFFFU,
                                   0xFFFF0000U, 0,
                                   0 } };

M608Unknown80 lbl_1_data_370 = { { 0x07080000, 0x3F800000 },
                                 1000.0f,
                                 { 0x43340000, 0,
                                   0xBE99999AU, 0,
                                   0x3F800000, 0x3F800000,
                                   0x42C80000, 0x3F800000,
                                   0x0002FFFF, 0xFFFFFFFFU,
                                   0xFFFF0000U, 0,
                                   0x0000FFFF, 0xFFFFFFFFU,
                                   /* m608 preset alternate header bits. */
                                   0xFFFF0000U, 0,
                                   0 } };

/* m608 preset float negative 0 03 bits. */
M608Unknown80 lbl_1_data_3C0 = { { 0x02580000, 0x3F800000 },
                                 1000.0f,
                                 /* m608 preset float 30 bits. */
                                 { 0x43340000, 0,
                                   0xBCF5C28FU, 0,
                                   0x3F800000, 0x3F800000,
                                   0x41F00000, 0x3F800000,
                                   0x0002FFFF, 0xFFFFFFFFU,
                                   0xFFFF0000U, 0,
                                   0x0000FFFF, 0xFFFFFFFFU,
                                   0xFFFF0000U, 0,
                                   0 } };

/* Polled by the intro and rider-state callbacks in sequence.c to detect camera-motion end. */
BOOL fn_1_5F64(void)
{
    return fn_8002D75C(lbl_1_bss_3E80.activeCameraMotion);
}

/* Called during rider setup in sequence.c to hide the indexed course prop. */
void fn_1_5F90(s16 coursePropIndex)
{
    s16 modelId;

    modelId = (&lbl_1_bss_3E80.courseModels[1])[coursePropIndex];
    fn_800269E8(modelId, (1 << 0));
}

/* Called during intro and rider setup in sequence.c to start and reveal a course prop. */
void fn_1_5FDC(s16 coursePropIndex, s16 courseMotionIndex)
{
    s16 coursePropModelId;

    coursePropModelId = (&lbl_1_bss_3E80.courseModels[1])[coursePropIndex];
    fn_8002C95C(coursePropModelId, lbl_1_bss_3E80.courseMotions[courseMotionIndex]);
    fn_80026A44(coursePropModelId, (1 << 0));
    fn_800269E8(coursePropModelId, ((1 << 30) | (1 << 0)));
}

/* Called during rider setup in sequence.c to restart and reveal the indexed prop. */
void fn_1_605C(s16 coursePropIndex)
{
    s16 coursePropModelId;

    coursePropModelId = (&lbl_1_bss_3E80.courseModels[1])[coursePropIndex];
    fn_8002D414(coursePropModelId, 0.0f);
    fn_80026A44(coursePropModelId, (1 << 0));
    fn_800269E8(coursePropModelId, ((1 << 30) | (1 << 0)));
}

/* Replaces the three preset height fields with random values for camera setup. */
void fn_1_60CC(void)
{
    lbl_1_data_320.randomizedHeight = (f32) (5000.0f * frandf());
    lbl_1_data_370.randomizedHeight = (f32) (5000.0f * frandf());
    lbl_1_data_3C0.randomizedHeight = (f32) (2000.0f * frandf());
}

/* Called at the end of setup; this original hook intentionally has no work. */
void fn_1_6148(void)
{

}

/* Called after the intro camera finishes; this original hook intentionally has no work. */
void fn_1_614C(void)
{

}
