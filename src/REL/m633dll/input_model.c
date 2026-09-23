#include "REL/m633dll.h"

void fn_1_128C(void)
{
    s32 i;
    f32 timerX;
    f32 timerY;

    lbl_1_bss_0.unk000 = MgActorObjectSetup();
    lbl_1_bss_0.unk194 = 0.0f;
    lbl_1_bss_0.unk108 = 0;
    lbl_1_bss_0.unk10C = 3;
    lbl_1_bss_0.unk0E0 = 3;
    lbl_1_bss_0.unk0F4 = 0;
    lbl_1_bss_0.unkA34 = 0.0f;

    i = 0;
    while (i < 4) {
        memset(&lbl_1_bss_0.unkA40[i], 0, sizeof(M633AI));
        lbl_1_bss_0.unkA40[i].unk00 = -1;
        lbl_1_bss_0.unkA40[i].unk04 = -1;
        i += 1;
    }

    i = 0;
    while (i < 50) {
        lbl_1_bss_0.unk19C[i].unk00 = i;
        i += 1;
    }

    CRot.x = -35.0f;
    CRot.y = 0.0f;
    CRot.z = 0.0f;
    Center.x = 0.0f;
    Center.y = 700.0f;
    Center.z = 400.0f;
    CZoom = 1000.0f;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 20.0f, 8000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);

    timerX = 528.0f;
    timerY = 64.0f;
    lbl_1_bss_0.unk02C = MgTimerCreate(0);
    MgTimerPosSet(lbl_1_bss_0.unk02C, timerX, timerY);
    lbl_1_bss_0.unk030 = omAddObjEx(lbl_1_bss_0.unk000, 8192, 1U, 5U, -1, fn_1_1598);
    MgSeqCreate(&lbl_1_data_0);
}

void fn_1_1598(OMOBJ *obj)
{
    Point3d sp48;
    Point3d sp3C;
    Point3d sp30;
    Point3d sp24;
    Point3d sp18;
    Point3d spC;
    MGACTOR_PARAM actorParam;
    s16 sp8;
    f32 temp_f1;
    f32 var_f30;
    s32 var_r29;
    s32 var_r31;
    HU3D_LIGHTID lightId;
    HU3D_LLIGHTID localLightId;

    lbl_1_bss_0.unk03E = Hu3DModelCreate(HuDataSelHeapReadNum(6029315, 268435456, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_0.unk03E, 1U);
    var_r31 = 0;
    while (var_r31 < 4) {
        lbl_1_bss_0.unk010[var_r31] = Hu3DMotionCreate(HuDataSelHeapReadNum(lbl_1_data_1B8[var_r31], 268435456, HEAP_MODEL));
        lbl_1_bss_0.unk018[var_r31] = Hu3DModelCameraCreate(lbl_1_bss_0.unk010[var_r31], 1U);
        Hu3DCameraMotionOff(lbl_1_bss_0.unk018[var_r31]);
        var_r31 += 1;
    }
    lbl_1_bss_0.unk020 = -1;
    lightId = Hu3DGLightCreate(0.0f, 300.0f, 0.0f, 0.0f, 1.0f, 0.0f, 255U, 255U, 255U);
    Hu3DGLightPointSet(lightId, 1600.0f, 1.0f, GX_DA_OFF);
    Hu3DGLightStaticSet(0, 1);
    sp48.x = 0.0f;
    sp48.y = 10000.0f;
    sp48.z = 0.0f;
    sp3C.x = 0.0f;
    sp3C.y = 1.0f;
    sp3C.z = 0.0f;
    sp30.x = 0.0f;
    sp30.y = 0.0f;
    sp30.z = -0.1f;
    Hu3DShadowCreate(8.5f, 5000.0f, 13000.0f);
    Hu3DShadowPosSet(&sp48, &sp3C, &sp30);
    lbl_1_bss_0.unkA38[0] = Hu3DModelCreate(HuDataSelHeapReadNum(4259860, 268435456, HEAP_MODEL));
    var_r31 = 1;
    while (var_r31 < 4) {
        lbl_1_bss_0.unkA38[var_r31] = Hu3DModelLink(lbl_1_bss_0.unkA38[0]);
        var_r31 += 1;
    }
    var_r31 = 0;
    while (var_r31 < 4) {
        Hu3DModelAttrSet(lbl_1_bss_0.unkA38[var_r31], 1U);
        Hu3DModelAttrSet(lbl_1_bss_0.unkA38[var_r31], 1073741825U);
        Hu3DModelLayerSet(lbl_1_bss_0.unkA38[var_r31], 5);
        var_r31 += 1;
    }
    lbl_1_bss_0.unk110 = Hu3DModelCreate(HuDataSelHeapReadNum(6029328, 268435456, HEAP_MODEL));
    lbl_1_bss_0.unk112 = Hu3DModelCreate(HuDataSelHeapReadNum(6029329, 268435456, HEAP_MODEL));
    lbl_1_bss_0.unk114 = Hu3DJointMotion(lbl_1_bss_0.unk110, HuDataSelHeapReadNum(6029330, 268435456, HEAP_MODEL));
    lbl_1_bss_0.unk116[0] = Hu3DJointMotion(lbl_1_bss_0.unk110, HuDataSelHeapReadNum(6029331, 268435456, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_0.unk110, 5);
    Hu3DModelLayerSet(lbl_1_bss_0.unk112, 4);
    Hu3DModelShadowMapObjSet(lbl_1_bss_0.unk110, lbl_1_data_104);
    Hu3DMotionSet(lbl_1_bss_0.unk110, lbl_1_bss_0.unk114);
    Hu3DReflectNoSet(2);
    lbl_1_bss_0.unk116[1] = Hu3DModelCreate(HuDataSelHeapReadNum(6029332, 268435456, HEAP_MODEL));
    lbl_1_bss_0.unk11A = Hu3DModelCreate(HuDataSelHeapReadNum(6029333, 268435456, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_0.unk116[1], 6);
    Hu3DModelLayerSet(lbl_1_bss_0.unk11A, 6);
    var_r31 = 0;
    while (var_r31 < 50) {
        if (var_r31 == 0) {
            lbl_1_bss_0.unk11C[var_r31] = Hu3DModelCreate(HuDataSelHeapReadNum(6029334, 268435456, HEAP_MODEL));
        } else {
            lbl_1_bss_0.unk11C[var_r31] = Hu3DModelLink(lbl_1_bss_0.unk11C[0]);
        }
        Hu3DModelLayerSet(lbl_1_bss_0.unk11C[var_r31], 6);
        Hu3DModelAttrSet(lbl_1_bss_0.unk11C[var_r31], 1U);
        var_r31 += 1;
    }
    var_r31 = 0;
    while (var_r31 < 2) {
        lbl_1_bss_0.unk180[var_r31] = Hu3DModelCreate(HuDataSelHeapReadNum(6029335, 268435456, HEAP_MODEL));
        Hu3DModelLayerSet(lbl_1_bss_0.unk180[var_r31], 5);
        var_r31 += 1;
    }
    var_r31 = 0;
    while (var_r31 < 4) {
        lbl_1_bss_0.unk184[var_r31] = Hu3DJointMotion(lbl_1_bss_0.unk180[0], HuDataSelHeapReadNum(lbl_1_data_1D0[var_r31], 268435456, HEAP_MODEL));
        lbl_1_bss_0.unk18C[var_r31] = Hu3DJointMotion(lbl_1_bss_0.unk180[1], HuDataSelHeapReadNum(lbl_1_data_1D0[var_r31], 268435456, HEAP_MODEL));
        var_r31 += 1;
    }
    Hu3DModelHookSet(lbl_1_bss_0.unk110, lbl_1_data_11F, lbl_1_bss_0.unk180[0]);
    Hu3DModelObjPosGet(lbl_1_bss_0.unk110, lbl_1_data_11F, &sp24);
    sp18 = sp24;
    PSVECNormalize(&sp18, &sp18);
    temp_f1 = PSVECMag(&sp24);
    sp24.x += sp18.x * -(2.0f * temp_f1);
    sp24.z += sp18.z * -(2.0f * temp_f1);
    Hu3DModelPosSetV(lbl_1_bss_0.unk180[1], &sp24);
    *obj->mdlId = Hu3DModelCreate(HuDataSelHeapReadNum(6029312, 268435456, HEAP_MODEL));
    sp8 = Hu3DModelCreate(HuDataSelHeapReadNum(6029314, 268435456, HEAP_MODEL));
    Hu3DModelLayerSet(*obj->mdlId, 2);
    obj->mtnId[0] = Hu3DJointMotion(*obj->mdlId, HuDataSelHeapReadNum(6029316, 268435456, HEAP_MODEL));
    obj->mtnId[1] = Hu3DJointMotion(*obj->mdlId, HuDataSelHeapReadNum(6029317, 268435456, HEAP_MODEL));
    obj->mtnId[2] = Hu3DJointMotion(*obj->mdlId, HuDataSelHeapReadNum(6029318, 268435456, HEAP_MODEL));
    obj->mtnId[3] = Hu3DJointMotion(*obj->mdlId, HuDataSelHeapReadNum(6029319, 268435456, HEAP_MODEL));
    Hu3DMotionSet(*obj->mdlId, obj->mtnId[0]);
    Hu3DModelAttrSet(*obj->mdlId, 1073741825U);
    Hu3DModelAttrReset(*obj->mdlId, 1U);
    Hu3DModelAttrSet(sp8, 1U);
    Hu3DModelShadowMapObjSet(*obj->mdlId, lbl_1_data_136);
    lbl_1_bss_0.unk03C = Hu3DModelCreate(HuDataSelHeapReadNum(6029313, 268435456, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_0.unk03C, 2);
    lbl_1_bss_0.unk034[0] = Hu3DModelCreate(HuDataSelHeapReadNum(6029320, 268435456, HEAP_MODEL));
    lbl_1_bss_0.unk034[1] = Hu3DModelCreate(HuDataSelHeapReadNum(6029321, 268435456, HEAP_MODEL));
    lbl_1_bss_0.unk034[2] = Hu3DModelCreate(HuDataSelHeapReadNum(6029322, 268435456, HEAP_MODEL));
    lbl_1_bss_0.unk034[3] = Hu3DModelCreate(HuDataSelHeapReadNum(6029323, 268435456, HEAP_MODEL));
    var_r31 = 0;
    while (var_r31 < 4) {
        Hu3DModelAttrSet(lbl_1_bss_0.unk034[var_r31], 1U);
        Hu3DModelLayerSet(lbl_1_bss_0.unk034[var_r31], 2);
        var_r31 += 1;
    }
    Hu3DModelAttrReset(lbl_1_bss_0.unk034[0], 1U);
    MgActorColMapInit(&sp8, 1, 25);
    actorParam.height = 150.0f;
    actorParam.radius = 40.0f;
    actorParam.param = 0;
    actorParam.type = 0;
    actorParam.attr = 0;
    actorParam.narrowHook = 0;
    actorParam.correctHook = 0;
    var_r29 = 1;
    var_r31 = 0;
    while (var_r31 < 4) {
        lbl_1_bss_0.unk050[var_r31] = (s32) GwPlayerConf[var_r31].charNo;
        lbl_1_bss_0.unk060[var_r31] = (s32) GwPlayerConf[var_r31].padNo;
        if (GwPlayerConf[var_r31].grpNo == 0) {
            lbl_1_bss_0.unk070[var_r31] = 0;
        } else {
            lbl_1_bss_0.unk070[var_r31] = 1;
        }
        if ((s32) lbl_1_bss_0.unk050[var_r31] == 3) {
            lbl_1_bss_0.unk040[var_r31] = MgPlayerCreate((s16) var_r31, &actorParam, 2, 1U, -15U, lbl_1_data_220);
        } else {
            lbl_1_bss_0.unk040[var_r31] = MgPlayerCreate((s16) var_r31, &actorParam, 2, 1U, -15U, lbl_1_data_1E0);
        }
        MgPlayerVibrateCreate(lbl_1_bss_0.unk040[var_r31]);
        actorParam.param += 1;
        localLightId = Hu3DLLightCreate((s16) lbl_1_bss_0.unk040[var_r31]->actor->mdlId, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 160U, 160U, 160U);
        Hu3DLLightInfinitytSet((s16) lbl_1_bss_0.unk040[var_r31]->actor->mdlId, localLightId);
        if ((s32) lbl_1_bss_0.unk070[var_r31] == 0) {
            Hu3DModelHookSet(lbl_1_bss_0.unk110, lbl_1_data_14F, (s16) lbl_1_bss_0.unk040[var_r31]->actor->mdlId);
            MgPlayerDespawn(lbl_1_bss_0.unk040[var_r31]);
            lbl_1_bss_0.unk0E4[var_r31] = omAddObjEx(lbl_1_bss_0.unk000, 32730, 0U, 0U, -1, fn_1_26B4);
            CharMotionShiftSet((s16) lbl_1_bss_0.unk050[var_r31], lbl_1_bss_0.unk040[var_r31]->omObj->mtnId[13], 0.0f, 0.0f, 1073741825U);
        } else {
            Hu3DModelObjPosGet(lbl_1_bss_0.unk03E, lbl_1_data_F4[var_r29], &spC);
            lbl_1_bss_0.unk040[var_r31]->actor->pos = spC;
            lbl_1_bss_0.unk0E4[var_r31] = omAddObjEx(lbl_1_bss_0.unk000, 32730, 0U, 0U, -1, fn_1_34AC);
            MgPlayerDespawn(lbl_1_bss_0.unk040[var_r31]);
            lbl_1_bss_0.unkA40[var_r31].unk0C = (f32) (180.0 * (atan2((f64) spC.z, (f64) spC.x) / 3.141592653589793));
            if (frandmod(100) < 50U) {
                var_f30 = -1.0f;
            } else {
                var_f30 = 1.0f;
            }
            lbl_1_bss_0.unkA40[var_r31].unk10 = var_f30;
            var_r29 += 1;
        }
        lbl_1_bss_0.unk0E4[var_r31]->work[0] = var_r31;
        lbl_1_bss_0.unk0E4[var_r31]->work[1] = 0;
        Hu3DModelShadowSet((s16) lbl_1_bss_0.unk040[var_r31]->actor->mdlId);
        var_r31 += 1;
    }
    CharEffectLayerSet(5);
    lbl_1_bss_0.unkAE4 = -1;
    lbl_1_bss_0.unkAE0 = -1;
    fn_1_70A8(3);
    obj->objFunc = NULL;
}
