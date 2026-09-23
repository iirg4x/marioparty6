#include "REL/m633dll.h"

void fn_1_73D8(void)
{
    Point3d sp78;
    Point3d sp6C;
    Point3d sp60;
    Point3d delta;
    Point3d sp48;
    Point3d sp3C;
    Point3d sp30;
    Vec dir;
    Point3d sp18;
    f32 temp_f25;
    f32 temp_f26;
    f32 temp_f31;

    lbl_1_bss_0.unk024 = Hu3DMotionTimeGet(lbl_1_bss_0.unk020);
    Hu3DCameraPosGet(1, &sp78, &sp60, &sp6C);
    Center = sp6C;
    delta.x = sp78.x - sp6C.x;
    delta.y = sp78.y - sp6C.y;
    delta.z = sp78.z - sp6C.z;
    CRot.x = (f32) HuAtan(-delta.y, sqrtf((delta.x * delta.x) + (delta.z * delta.z)));
    CRot.y = (f32) HuAtan(delta.x, delta.z);
    CRot.z = 0.0f;
    CZoom = sqrtf((delta.z * delta.z) + ((delta.x * delta.x) + (delta.y * delta.y)));
    temp_f26 = CRot.x;
    temp_f25 = CRot.y;
    temp_f31 = CRot.z;
    sp48.x = Center.x + (CZoom * (HuSin(temp_f25) * HuCos(temp_f26)));
    sp48.y = Center.y + (CZoom * -HuSin(temp_f26));
    sp48.z = Center.z + (CZoom * (HuCos(temp_f25) * HuCos(temp_f26)));
    sp3C.x = Center.x;
    sp3C.y = Center.y;
    sp3C.z = Center.z;
    dir.x = HuSin(temp_f25) * HuSin(temp_f26);
    dir.y = HuCos(temp_f26);
    dir.z = HuCos(temp_f25) * HuSin(temp_f26);
    PSVECSubtract(&sp48, &sp3C, &sp18);
    PSVECNormalize(&sp18, &sp18);
    sp30.x = dir.x * (sp18.x * sp18.x + (1.0f - sp18.x * sp18.x) * HuCos(temp_f31))
        + dir.y * (sp18.x * sp18.y * (1.0f - HuCos(temp_f31)) - sp18.z * HuSin(temp_f31))
        + dir.z * (sp18.x * sp18.z * (1.0f - HuCos(temp_f31)) + sp18.y * HuSin(temp_f31));
    sp30.y = dir.y * (sp18.y * sp18.y + (1.0f - sp18.y * sp18.y) * HuCos(temp_f31))
        + dir.x * (sp18.x * sp18.y * (1.0f - HuCos(temp_f31)) + sp18.z * HuSin(temp_f31))
        + dir.z * (sp18.y * sp18.z * (1.0f - HuCos(temp_f31)) - sp18.x * HuSin(temp_f31));
    sp30.z = dir.z * (sp18.z * sp18.z + (1.0f - sp18.z * sp18.z) * HuCos(temp_f31))
        + (dir.x * (sp18.x * sp18.z * (1.0 - HuCos(temp_f31)) - sp18.y * HuSin(temp_f31))
        + dir.y * (sp18.y * sp18.z * (1.0 - HuCos(temp_f31)) + sp18.x * HuSin(temp_f31)));
    PSVECNormalize(&sp30, &sp30);
    Hu3DCameraPosSet(1, sp48.x, sp48.y, sp48.z, sp30.x, sp30.y, sp30.z, sp3C.x, sp3C.y, sp3C.z);
}

void fn_1_7E70(s32 arg0)
{
    s16 sp8;
    s16 temp_r31;

    sp8 = lbl_1_bss_0.unk010[arg0];
    temp_r31 = lbl_1_bss_0.unk018[arg0];
    Hu3DCameraMotionStart(temp_r31, 1U);
    lbl_1_bss_0.unk024 = 0.0f;
    lbl_1_bss_0.unk028 = Hu3DMotionMaxTimeGet(temp_r31);
    if (lbl_1_bss_0.unk020 != -1) {
        Hu3DCameraMotionOff(lbl_1_bss_0.unk020);
    }
    lbl_1_bss_0.unk020 = temp_r31;
    Hu3DCameraMotionOn(temp_r31, 1U);
}

int fn_1_7F40(void)
{
    return Hu3DMotionEndCheck(lbl_1_bss_0.unk020);
}
