#define _MATH_H
#include "dolphin/math.h"
#include "REL/m608dll.h"

char lbl_1_data_2F8[40] = "start camera motion ... idx:%d time:%f\012";

/* Raw words preserve unconsumed retail storage; only field08 has a typed consumer. */
M608Unknown80 lbl_1_data_320 = {
    { 117964800, 1056964608 }, 1000.0f,
    { 1127481344, 0, 3197737370U, 0, 1065353216, 1065353216, 1120403456, 1065353216,
      196607, 4294967295U, 4294901760U, 0, 65535, 4294967295U, 4294901760U, 0, 0 }
};

M608Unknown80 lbl_1_data_370 = {
    { 117964800, 1065353216 }, 1000.0f,
    { 1127481344, 0, 3197737370U, 0, 1065353216, 1065353216, 1120403456, 1065353216,
      196607, 4294967295U, 4294901760U, 0, 65535, 4294967295U, 4294901760U, 0, 0 }
};

M608Unknown80 lbl_1_data_3C0 = {
    { 39321600, 1065353216 }, 1000.0f,
    { 1127481344, 0, 3170222735U, 0, 1065353216, 1065353216, 1106247680, 1065353216,
      196607, 4294967295U, 4294901760U, 0, 65535, 4294967295U, 4294901760U, 0, 0 }
};

void fn_1_5DA4(Point3d position)
{
    lbl_1_bss_3E80.vector_428.x = position.x;
    lbl_1_bss_3E80.vector_428.y = (f32) (3500.0f + position.y);
    lbl_1_bss_3E80.vector_428.z = position.z;
    lbl_1_bss_3E80.vector_434.x = position.x;
    lbl_1_bss_3E80.vector_434.y = -100.0f;
    lbl_1_bss_3E80.vector_434.z = position.z;
    Hu3DShadowPosSet((Point3d *) &lbl_1_bss_3E80.vector_428.x, (Point3d *) &lbl_1_bss_3E80.vector_440, (Point3d *) &lbl_1_bss_3E80.vector_434.x);
}

void fn_1_5E6C(s32 arg0)
{
    s16 sp8;
    s16 temp_r31;

    sp8 = lbl_1_bss_3E80.cameraMotions[arg0];
    temp_r31 = lbl_1_bss_3E80.cameraModels[arg0];
    Hu3DCameraMotionStart(temp_r31, 1U);
    lbl_1_bss_3E80.cameraTime = 0.0f;
    lbl_1_bss_3E80.cameraMaxTime = Hu3DMotionMaxTimeGet(temp_r31);
    if (lbl_1_bss_3E80.activeCameraMotion != -1) {
        Hu3DCameraMotionOff(lbl_1_bss_3E80.activeCameraMotion);
    }
    lbl_1_bss_3E80.activeCameraMotion = temp_r31;
    Hu3DCameraMotionOn(lbl_1_bss_3E80.activeCameraMotion, 1U);
    OSReport(lbl_1_data_2F8, arg0, lbl_1_bss_3E80.cameraMaxTime);
}

BOOL fn_1_5F64(void)
{
    return Hu3DMotionEndCheck(lbl_1_bss_3E80.activeCameraMotion);
}

void fn_1_5F90(s16 modelIndex)
{
    s16 modelId;

    modelId = (&lbl_1_bss_3E80.models_184_199[1])[modelIndex];
    Hu3DModelAttrSet(modelId, 1U);
}

void fn_1_5FDC(s16 modelIndex, s16 motionIndex)
{
    s16 temp_r31;

    temp_r31 = (&lbl_1_bss_3E80.models_184_199[1])[modelIndex];
    Hu3DMotionSet(temp_r31, lbl_1_bss_3E80.motions_200_208[motionIndex]);
    Hu3DModelAttrReset(temp_r31, 1U);
    Hu3DModelAttrSet(temp_r31, 1073741825U);
}

void fn_1_605C(s16 arg0)
{
    s16 temp_r31;

    temp_r31 = (&lbl_1_bss_3E80.models_184_199[1])[arg0];
    Hu3DMotionTimeSet(temp_r31, 0.0f);
    Hu3DModelAttrReset(temp_r31, 1U);
    Hu3DModelAttrSet(temp_r31, 1073741825U);
}

void fn_1_60CC(void)
{
    lbl_1_data_320.field08 = (f32) (5000.0f * frandf());
    lbl_1_data_370.field08 = (f32) (5000.0f * frandf());
    lbl_1_data_3C0.field08 = (f32) (2000.0f * frandf());
}

void fn_1_6148(void)
{

}

void fn_1_614C(void)
{

}
