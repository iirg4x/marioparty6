/* Camera, model and shadow helpers used during the rolling course sequence. */
#define _MATH_H
#include "dolphin/math.h"
#include "REL/m608dll.h"

char lbl_1_data_2F8[40] = "start camera motion ... idx:%d time:%f\012";

/* Opaque camera preset words are kept as named constants; only the height is interpreted here. */
#define M608_INITIAL_CAMERA_HEIGHT 1000.0f
#define M608_PRESET_HEADER_BITS 0x07080000
#define M608_PRESET_ALTERNATE_HEADER_BITS 0x02580000
#define M608_PRESET_FLOAT_HALF_BITS 0x3F000000
#define M608_PRESET_FLOAT_ONE_BITS 0x3F800000
#define M608_PRESET_FLOAT_180_BITS 0x43340000
#define M608_PRESET_FLOAT_NEGATIVE_0_3_BITS 0xBE99999AU
#define M608_PRESET_FLOAT_NEGATIVE_0_03_BITS 0xBCF5C28FU
#define M608_PRESET_FLOAT_100_BITS 0x42C80000
#define M608_PRESET_FLOAT_30_BITS 0x41F00000
#define M608_PRESET_SHARED_DATA_BITS 0x0002FFFF
#define M608_PRESET_WORD_ALL_BITS 0xFFFFFFFFU
#define M608_PRESET_UPPER_HALF_BITS 0xFFFF0000U
#define M608_PRESET_LOWER_HALF_BITS 0x0000FFFF
#define M608_PRESET_WORD_ZERO 0

/* These words are stored preset data; their meanings are not established by this module. */
M608Unknown80 lbl_1_data_320 = { { M608_PRESET_HEADER_BITS, M608_PRESET_FLOAT_HALF_BITS },
                                 M608_INITIAL_CAMERA_HEIGHT,
                                 { M608_PRESET_FLOAT_180_BITS, M608_PRESET_WORD_ZERO,
                                   M608_PRESET_FLOAT_NEGATIVE_0_3_BITS, M608_PRESET_WORD_ZERO,
                                   M608_PRESET_FLOAT_ONE_BITS, M608_PRESET_FLOAT_ONE_BITS,
                                   M608_PRESET_FLOAT_100_BITS, M608_PRESET_FLOAT_ONE_BITS,
                                   M608_PRESET_SHARED_DATA_BITS, M608_PRESET_WORD_ALL_BITS,
                                   M608_PRESET_UPPER_HALF_BITS, M608_PRESET_WORD_ZERO,
                                   M608_PRESET_LOWER_HALF_BITS, M608_PRESET_WORD_ALL_BITS,
                                   M608_PRESET_UPPER_HALF_BITS, M608_PRESET_WORD_ZERO,
                                   M608_PRESET_WORD_ZERO } };

M608Unknown80 lbl_1_data_370 = { { M608_PRESET_HEADER_BITS, M608_PRESET_FLOAT_ONE_BITS },
                                 M608_INITIAL_CAMERA_HEIGHT,
                                 { M608_PRESET_FLOAT_180_BITS, M608_PRESET_WORD_ZERO,
                                   M608_PRESET_FLOAT_NEGATIVE_0_3_BITS, M608_PRESET_WORD_ZERO,
                                   M608_PRESET_FLOAT_ONE_BITS, M608_PRESET_FLOAT_ONE_BITS,
                                   M608_PRESET_FLOAT_100_BITS, M608_PRESET_FLOAT_ONE_BITS,
                                   M608_PRESET_SHARED_DATA_BITS, M608_PRESET_WORD_ALL_BITS,
                                   M608_PRESET_UPPER_HALF_BITS, M608_PRESET_WORD_ZERO,
                                   M608_PRESET_LOWER_HALF_BITS, M608_PRESET_WORD_ALL_BITS,
                                   M608_PRESET_UPPER_HALF_BITS, M608_PRESET_WORD_ZERO,
                                   M608_PRESET_WORD_ZERO } };

M608Unknown80 lbl_1_data_3C0 = { { M608_PRESET_ALTERNATE_HEADER_BITS, M608_PRESET_FLOAT_ONE_BITS },
                                 M608_INITIAL_CAMERA_HEIGHT,
                                 { M608_PRESET_FLOAT_180_BITS, M608_PRESET_WORD_ZERO,
                                   M608_PRESET_FLOAT_NEGATIVE_0_03_BITS, M608_PRESET_WORD_ZERO,
                                   M608_PRESET_FLOAT_ONE_BITS, M608_PRESET_FLOAT_ONE_BITS,
                                   M608_PRESET_FLOAT_30_BITS, M608_PRESET_FLOAT_ONE_BITS,
                                   M608_PRESET_SHARED_DATA_BITS, M608_PRESET_WORD_ALL_BITS,
                                   M608_PRESET_UPPER_HALF_BITS, M608_PRESET_WORD_ZERO,
                                   M608_PRESET_LOWER_HALF_BITS, M608_PRESET_WORD_ALL_BITS,
                                   M608_PRESET_UPPER_HALF_BITS, M608_PRESET_WORD_ZERO,
                                   M608_PRESET_WORD_ZERO } };

/* Called by the intro and run callbacks in sequence.c to follow the current course hook. */
void fn_1_5DA4(Point3d position)
{
    lbl_1_bss_3E80.shadowCenter.x = position.x;
    lbl_1_bss_3E80.shadowCenter.y = (f32) (3500.0f + position.y);
    lbl_1_bss_3E80.shadowCenter.z = position.z;
    lbl_1_bss_3E80.shadowTarget.x = position.x;
    lbl_1_bss_3E80.shadowTarget.y = -100.0f;
    lbl_1_bss_3E80.shadowTarget.z = position.z;
    Hu3DShadowPosSet((Point3d *) &lbl_1_bss_3E80.shadowCenter.x,
                     (Point3d *) &lbl_1_bss_3E80.shadowUpDirection,
                     (Point3d *) &lbl_1_bss_3E80.shadowTarget.x);
}

/* Called by the intro and rider-state callbacks in sequence.c to switch camera motions. */
void fn_1_5E6C(s32 cameraIndex)
{
    s16 cameraMotionId;
    s16 cameraModelId;

    cameraMotionId = lbl_1_bss_3E80.cameraMotions[cameraIndex];
    cameraModelId = lbl_1_bss_3E80.cameraModels[cameraIndex];
    Hu3DCameraMotionStart(cameraModelId, 1U);
    lbl_1_bss_3E80.cameraTime = 0.0f;
    lbl_1_bss_3E80.cameraMaxTime = Hu3DMotionMaxTimeGet(cameraModelId);
    if (lbl_1_bss_3E80.activeCameraMotion != -1) {
        Hu3DCameraMotionOff(lbl_1_bss_3E80.activeCameraMotion);
    }
    lbl_1_bss_3E80.activeCameraMotion = cameraModelId;
    Hu3DCameraMotionOn(lbl_1_bss_3E80.activeCameraMotion, 1U);
    OSReport(lbl_1_data_2F8, cameraIndex, lbl_1_bss_3E80.cameraMaxTime);
}

/* Polled by the intro and rider-state callbacks in sequence.c to detect camera-motion end. */
BOOL fn_1_5F64(void)
{
    return Hu3DMotionEndCheck(lbl_1_bss_3E80.activeCameraMotion);
}

/* Called during rider setup in sequence.c to hide the indexed course prop. */
void fn_1_5F90(s16 coursePropIndex)
{
    s16 modelId;

    modelId = (&lbl_1_bss_3E80.courseModels[1])[coursePropIndex];
    Hu3DModelAttrSet(modelId, HU3D_ATTR_DISPOFF);
}

/* Called during intro and rider setup in sequence.c to start and reveal a course prop. */
void fn_1_5FDC(s16 coursePropIndex, s16 courseMotionIndex)
{
    s16 coursePropModelId;

    coursePropModelId = (&lbl_1_bss_3E80.courseModels[1])[coursePropIndex];
    Hu3DMotionSet(coursePropModelId, lbl_1_bss_3E80.courseMotions[courseMotionIndex]);
    Hu3DModelAttrReset(coursePropModelId, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(coursePropModelId, HU3D_MOTATTR_LOOP);
}

/* Called during rider setup in sequence.c to restart and reveal the indexed prop. */
void fn_1_605C(s16 coursePropIndex)
{
    s16 coursePropModelId;

    coursePropModelId = (&lbl_1_bss_3E80.courseModels[1])[coursePropIndex];
    Hu3DMotionTimeSet(coursePropModelId, 0.0f);
    Hu3DModelAttrReset(coursePropModelId, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(coursePropModelId, HU3D_MOTATTR_LOOP);
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
