#define _MATH_H
#include "dolphin/math.h"
#include "REL/m608dll.h"

const Point3d lbl_1_rodata_88 = { 100.0f, 800.0f, -100.0f };

const Point3d lbl_1_rodata_94 = { 0.3f, -0.8f, 0.3f };

const Point3d lbl_1_rodata_A0 = {20.0f, 45.0f, 1000.0f};

const GXColor lbl_1_rodata_AC = {255, 255, 255, 255};

const Point3d lbl_1_rodata_B0 = { 6379.0f, 4448.0f, -3.2f };

const Point3d lbl_1_rodata_BC = { 0.3f, -0.8f, 0.3f };

const Point3d lbl_1_rodata_C8 = {20.0f, 45.0f, 1000.0f};

const GXColor lbl_1_rodata_D4 = {255, 255, 255, 255};

void fn_1_43C8(void)
{
    Point3d sp70;
    Point3d sp64;
    Point3d sp58;
    Point3d sp4C;
    Point3d sp40;
    Point3d sp34;
    Point3d sp28;
    Point3d sp1C;
    Point3d sp10;
    GXColor spC;
    GXColor sp8;
    f32 temp_f31;
    int temp_r6;
    HU3D_LLIGHTID localLightId;
    s16 temp_r3;
    s16 temp_r25;
    s16 temp_r29;
    s16 temp_r30;
    s16 temp_r3_2;
    s16 var_r23;
    s16 nightFlag;
    s32 var_r31;

    Hu3DParManInit();
    memset(&lbl_1_bss_3E80, 0, sizeof(M608SceneConsumedContext));
    lbl_1_bss_3E80.activeCameraMotion = -1;
    lbl_1_bss_3E80.vector_428.x = 0.0f;
    lbl_1_bss_3E80.vector_428.y = 10000.0f;
    lbl_1_bss_3E80.vector_428.z = 0.0f;
    lbl_1_bss_3E80.vector_440.x = 0.0f;
    lbl_1_bss_3E80.vector_440.y = 0.0f;
    lbl_1_bss_3E80.vector_440.z = 1.0f;
    lbl_1_bss_3E80.vector_434.x = 0.0f;
    lbl_1_bss_3E80.vector_434.y = -100.0f;
    lbl_1_bss_3E80.vector_434.z = 0.0f;
    lbl_1_bss_3E80.flag = 0;
    lbl_1_bss_3E80.count = 0;
    lbl_1_bss_3E80.objectManager = omInitObjMan(100, 8192);
    omGameSysInit(lbl_1_bss_3E80.objectManager);
    fn_1_6828();
    CRot.x = -20.0f;
    CRot.y = 0.0f;
    CRot.z = 0.0f;
    Center.x = 0.0f;
    Center.y = 100.0f;
    Center.z = 0.0f;
    CZoom = 400.0f;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 20.0f, 800.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    lbl_1_bss_3E80.cameraObject = omAddObjEx(lbl_1_bss_3E80.objectManager, 16, 0U, 0U, -1, fn_1_2540);
    var_r31 = 0;
    while (var_r31 < 10) {
        lbl_1_bss_3E80.cameraMotions[var_r31] = Hu3DMotionCreate(HuDataSelHeapReadNum(lbl_1_data_4F0[var_r31], 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.cameraModels[var_r31] = Hu3DModelCameraCreate(lbl_1_bss_3E80.cameraMotions[var_r31], 1U);
        Hu3DCameraMotionOff(lbl_1_bss_3E80.cameraModels[var_r31]);
        var_r31 += 1;
    }
    Hu3DShadowCreate(30.0f, 1.0f, 13000.0f);
    Hu3DShadowPosSet(&lbl_1_bss_3E80.vector_428, &lbl_1_bss_3E80.vector_440, &lbl_1_bss_3E80.vector_434);
    Hu3DShadowColSet(50U, 50U, 50U);
    Hu3DShadowSizeSet(192U);
    Hu3DShadowTPLvlSet(0.7f);
    Hu3DShadowColSet(50U, 50U, 50U);
    Hu3DShadowSizeSet(240U);
    var_r31 = 0;
    while (var_r31 < 4) {
        lbl_1_bss_3E80.playerModelsA[var_r31] = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_518[var_r31], 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.playerModelsB[var_r31] = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_528[var_r31], 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.playerModelsC[var_r31] = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_538[var_r31], 268435456, HEAP_MODEL));
        Hu3DModelLayerSet(lbl_1_bss_3E80.playerModelsA[var_r31], 1);
        Hu3DModelLayerSet(lbl_1_bss_3E80.playerModelsB[var_r31], 1);
        Hu3DModelLayerSet(lbl_1_bss_3E80.playerModelsC[var_r31], 1);
        sp70.x = 0.0f;
        sp70.y = -2.0f;
        sp70.z = 0.0f;
        Hu3DModelPosSetV(lbl_1_bss_3E80.playerModelsB[var_r31], &sp70);
        Hu3DModelPosSetV(lbl_1_bss_3E80.playerModelsC[var_r31], &sp70);
        var_r31 += 1;
    }
    var_r31 = 0;
    while (var_r31 < 4) {
        Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsB[var_r31], lbl_1_data_684[var_r31], &lbl_1_bss_3E80.positions_070[var_r31]);
        Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsB[var_r31], lbl_1_data_624[var_r31], &lbl_1_bss_3E80.positions_0D0[var_r31]);
        Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsC[var_r31], lbl_1_data_64C[var_r31], &lbl_1_bss_3E80.positions_0D0[var_r31]);
        var_r31 += 1;
    }
    lbl_1_bss_3E80.mainModel = Hu3DModelCreate(HuDataSelHeapReadNum(4390956, 268435456, HEAP_MODEL));
    lbl_1_bss_3E80.mainMotion = Hu3DJointMotion(lbl_1_bss_3E80.mainModel, HuDataSelHeapReadNum(4390957, 268435456, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_3E80.mainModel, 2);
    Hu3DModelShadowSet(lbl_1_bss_3E80.mainModel);
    lbl_1_bss_3E80.models_212_213[0] = Hu3DModelCreate(HuDataSelHeapReadNum(4390970, 268435456, HEAP_MODEL));
    lbl_1_bss_3E80.models_212_213[1] = Hu3DModelCreate(HuDataSelHeapReadNum(4390971, 268435456, HEAP_MODEL));
    nightFlag = GwMgNightF;
    if (nightFlag == 0) {
        lbl_1_bss_3E80.sceneModels[0] = Hu3DModelCreate(HuDataSelHeapReadNum(4390912, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.sceneModels[1] = Hu3DModelCreate(HuDataSelHeapReadNum(4390913, 268435456, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_3E80.sceneModels[0], 1073741825U);
        lbl_1_bss_3E80.models_184_199[8] = Hu3DModelCreate(HuDataSelHeapReadNum(4390976, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.models_184_199[4] = Hu3DModelCreate(HuDataSelHeapReadNum(4390982, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.motions_200_208[0] = Hu3DJointMotion(lbl_1_bss_3E80.models_184_199[4], HuDataSelHeapReadNum(4390983, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.motions_200_208[1] = Hu3DJointMotion(lbl_1_bss_3E80.models_184_199[4], HuDataSelHeapReadNum(4390984, 268435456, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_3E80.models_184_199[4], 1073741825U);
        lbl_1_bss_3E80.models_184_199[6] = Hu3DModelCreate(HuDataSelHeapReadNum(4390995, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.motions_200_208[4] = Hu3DJointMotion(lbl_1_bss_3E80.models_184_199[6], HuDataSelHeapReadNum(4390996, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.motions_200_208[5] = Hu3DJointMotion(lbl_1_bss_3E80.models_184_199[6], HuDataSelHeapReadNum(4390997, 268435456, HEAP_MODEL));
        fn_1_5FDC(5, 4);
        lbl_1_bss_3E80.models_184_199[5] = Hu3DModelCreate(HuDataSelHeapReadNum(4390989, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.motions_200_208[2] = Hu3DJointMotion(lbl_1_bss_3E80.models_184_199[5], HuDataSelHeapReadNum(4390990, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.motions_200_208[3] = Hu3DJointMotion(lbl_1_bss_3E80.models_184_199[5], HuDataSelHeapReadNum(4390991, 268435456, HEAP_MODEL));
        fn_1_5FDC(4, 2);
        Hu3DModelAttrSet(lbl_1_bss_3E80.models_184_199[5], 1073741825U);
        lbl_1_bss_3E80.models_184_199[9] = Hu3DModelCreate(HuDataSelHeapReadNum(4390978, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.models_184_199[10] = Hu3DModelCreate(HuDataSelHeapReadNum(4390980, 268435456, HEAP_MODEL));
        sp4C = lbl_1_rodata_88;
        sp40 = lbl_1_rodata_94;
        sp34 = lbl_1_rodata_A0;
        spC = lbl_1_rodata_AC;
        temp_r3 = Hu3DGLightCreateV(&sp4C, &sp40, &spC);
        Hu3DGLightInfinitytSet(temp_r3);
        Hu3DGLightStaticSet(temp_r3, 1);
        sp64.x = 0.0f;
        sp64.y = 0.0f;
        sp64.z = 0.0f;
        sp58.x = 1.0f;
        sp58.y = 7825.0f;
        sp58.z = 9582.0f;
        Hu3DGLightPosAimSetV(temp_r3, &sp58, &sp64);
        Hu3DModelShadowMapObjSet(lbl_1_bss_3E80.sceneModels[0], lbl_1_data_295);
        Hu3DModelShadowMapObjSet(lbl_1_bss_3E80.sceneModels[0], lbl_1_data_2A0);
    } else {
        lbl_1_bss_3E80.sceneModels[0] = Hu3DModelCreate(HuDataSelHeapReadNum(4390915, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.sceneModels[2] = Hu3DModelCreate(HuDataSelHeapReadNum(4390917, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.sceneModels[3] = Hu3DModelCreate(HuDataSelHeapReadNum(4390914, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.sceneModels[4] = Hu3DModelCreate(HuDataSelHeapReadNum(4390918, 268435456, HEAP_MODEL));
        Hu3DModelClusterAttrSet(lbl_1_bss_3E80.sceneModels[0], 0, -1073741823);
        Hu3DModelAttrSet(lbl_1_bss_3E80.sceneModels[2], 1073741825U);
        Hu3DModelAttrSet(lbl_1_bss_3E80.sceneModels[3], 1073741825U);
        Hu3DModelAttrSet(lbl_1_bss_3E80.sceneModels[4], 1073741825U);
        lbl_1_bss_3E80.models_184_199[8] = Hu3DModelCreate(HuDataSelHeapReadNum(4390977, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.models_184_199[4] = Hu3DModelCreate(HuDataSelHeapReadNum(4390985, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.motions_200_208[0] = Hu3DJointMotion(lbl_1_bss_3E80.models_184_199[4], HuDataSelHeapReadNum(4390986, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.motions_200_208[1] = Hu3DJointMotion(lbl_1_bss_3E80.models_184_199[4], HuDataSelHeapReadNum(4390987, 268435456, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_3E80.models_184_199[4], 1073741825U);
        lbl_1_bss_3E80.models_184_199[6] = Hu3DModelCreate(HuDataSelHeapReadNum(4390995, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.motions_200_208[4] = Hu3DJointMotion(lbl_1_bss_3E80.models_184_199[6], HuDataSelHeapReadNum(4390996, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.motions_200_208[5] = Hu3DJointMotion(lbl_1_bss_3E80.models_184_199[6], HuDataSelHeapReadNum(4390997, 268435456, HEAP_MODEL));
        fn_1_5FDC(5, 4);
        lbl_1_bss_3E80.models_184_199[5] = Hu3DModelCreate(HuDataSelHeapReadNum(4390992, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.motions_200_208[2] = Hu3DJointMotion(lbl_1_bss_3E80.models_184_199[5], HuDataSelHeapReadNum(4390993, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.motions_200_208[3] = Hu3DJointMotion(lbl_1_bss_3E80.models_184_199[5], HuDataSelHeapReadNum(4390994, 268435456, HEAP_MODEL));
        fn_1_5FDC(4, 2);
        Hu3DModelAttrSet(lbl_1_bss_3E80.models_184_199[5], 1073741825U);
        lbl_1_bss_3E80.models_184_199[9] = Hu3DModelCreate(HuDataSelHeapReadNum(4390979, 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.models_184_199[10] = Hu3DModelCreate(HuDataSelHeapReadNum(4390981, 268435456, HEAP_MODEL));
        sp28 = lbl_1_rodata_B0;
        sp1C = lbl_1_rodata_BC;
        sp10 = lbl_1_rodata_C8;
        sp8 = lbl_1_rodata_D4;
        temp_r3_2 = Hu3DGLightCreateV(&sp28, &sp1C, &sp8);
        Hu3DGLightPointSet(temp_r3_2, 1000.0f, 1.0f, GX_DA_OFF);
        Hu3DGLightStaticSet(temp_r3_2, 1);
    }
    var_r31 = 0;
    while (var_r31 < 4) {
        Hu3DModelShadowSet(lbl_1_bss_3E80.playerModelsA[var_r31]);
        Hu3DModelShadowSet(lbl_1_bss_3E80.playerModelsB[var_r31]);
        Hu3DModelShadowSet(lbl_1_bss_3E80.playerModelsC[var_r31]);
        var_r31 += 1;
    }
    lbl_1_bss_3E80.models_184_199[1] = Hu3DModelCreate(HuDataSelHeapReadNum(4390972, 268435456, HEAP_MODEL));
    lbl_1_bss_3E80.models_184_199[2] = Hu3DModelCreate(HuDataSelHeapReadNum(4390973, 268435456, HEAP_MODEL));
    fn_1_5F90(0);
    fn_1_5F90(1);
    fn_1_5F90(7);
    fn_1_5F90(8);
    fn_1_5F90(9);
    fn_1_5F90(5);
    lbl_1_bss_3E80.flag_44C = (s32) (frand() & 3);
    lbl_1_bss_3E80.models_184_199[3] = Hu3DModelCreate(HuDataSelHeapReadNum(4390988, 268435456, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_3E80.models_184_199[3], 1073741825U);
    lbl_1_bss_3E80.models_184_199[7] = Hu3DModelCreate(HuDataSelHeapReadNum(4391002, 268435456, HEAP_MODEL));
    lbl_1_bss_3E80.motions_200_208[6] = Hu3DJointMotion(lbl_1_bss_3E80.models_184_199[7], HuDataSelHeapReadNum(4391003, 268435456, HEAP_MODEL));
    lbl_1_bss_3E80.motions_200_208[7] = Hu3DJointMotion(lbl_1_bss_3E80.models_184_199[7], HuDataSelHeapReadNum(4391004, 268435456, HEAP_MODEL));
    {
        char *hookNames[10] = {
            "NPC3", "NPC4", "NPC5", "NPC6", "NPC7", "NPC8", "NPC9", "NPC10", "NPC11", "NPC12"
        };
        BOOL hookEnabled[10] = { TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE };
    var_r31 = 0;
    while (var_r31 < 10) {
        temp_r30 = Hu3DModelLink(lbl_1_bss_3E80.models_184_199[7]);
        var_r23 = hookEnabled[var_r31] ? 7 : 6;
        Hu3DModelHookSet(lbl_1_bss_3E80.models_184_199[3], hookNames[var_r31], temp_r30);
        Hu3DMotionSet(temp_r30, lbl_1_bss_3E80.motions_200_208[var_r23]);
        temp_f31 = Hu3DMotionMaxTimeGet(temp_r30);
        Hu3DMotionTimeSet(temp_r30, temp_f31 * frandf());
        Hu3DModelAttrSet(temp_r30, 1073741825U);
        localLightId = Hu3DLLightCreate(temp_r30, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 128U, 128U, 128U);
        Hu3DLLightInfinitytSet(temp_r30, localLightId);
        var_r31 += 1;
    }
    }
    temp_r25 = lbl_1_bss_3E80.models_184_199[7];
    Hu3DModelHookSet(lbl_1_bss_3E80.models_184_199[5], lbl_1_data_2B0, temp_r25);
    Hu3DMotionSet(temp_r25, lbl_1_bss_3E80.motions_200_208[6]);
    Hu3DModelAttrSet(temp_r25, 1073741825U);
    lbl_1_bss_3E80.models_184_199[11] = Hu3DModelCreate(HuDataSelHeapReadNum(4391005, 268435456, HEAP_MODEL));
    lbl_1_bss_3E80.motions_200_208[8] = Hu3DJointMotion(lbl_1_bss_3E80.models_184_199[11], HuDataSelHeapReadNum(4391006, 268435456, HEAP_MODEL));
    lbl_1_bss_3E80.models_184_199[12] = Hu3DModelCreate(HuDataSelHeapReadNum(4390998, 268435456, HEAP_MODEL));
    lbl_1_bss_3E80.models_184_199[13] = Hu3DModelCreate(HuDataSelHeapReadNum(4390999, 268435456, HEAP_MODEL));
    lbl_1_bss_3E80.models_184_199[14] = Hu3DModelCreate(HuDataSelHeapReadNum(4391000, 268435456, HEAP_MODEL));
    lbl_1_bss_3E80.models_184_199[15] = Hu3DModelCreate(HuDataSelHeapReadNum(4391001, 268435456, HEAP_MODEL));
    temp_r29 = lbl_1_bss_3E80.models_184_199[11];
    Hu3DModelHookSet(lbl_1_bss_3E80.models_184_199[6], lbl_1_data_2B5, temp_r29);
    Hu3DModelHookSet(temp_r29, lbl_1_data_2BA, lbl_1_bss_3E80.models_184_199[15]);
    Hu3DModelHookSet(temp_r29, lbl_1_data_2C5, lbl_1_bss_3E80.models_184_199[15]);
    Hu3DModelHookSet(temp_r29, lbl_1_data_2D0, lbl_1_bss_3E80.models_184_199[12]);
    Hu3DModelAttrSet(lbl_1_bss_3E80.models_184_199[11], 1073741825U);
    var_r31 = 0;
    while (var_r31 < 4) {
        temp_r6 = lbl_1_bss_3E80.characterNos[var_r31] = GwPlayerConf[var_r31].charNo;
        lbl_1_bss_3E80.padNos[var_r31] = GwPlayerConf[var_r31].padNo;
        lbl_1_bss_3E80.characterModels[var_r31] = CharModelMotListCreate((s16) temp_r6, 4, lbl_1_data_548, lbl_1_bss_3E80.characterMotions[var_r31]);
        lbl_1_bss_3E80.playerObjects[var_r31] = omAddObjEx(lbl_1_bss_3E80.objectManager, 8704, 1U, 0U, -1, NULL);
        *(lbl_1_bss_3E80.playerObjects[var_r31])->mdlId = lbl_1_bss_3E80.characterModels[var_r31];
        Hu3DModelShadowSet(lbl_1_bss_3E80.characterModels[var_r31]);
        CharMotionDataClose((s16) temp_r6);
        OSReport(lbl_1_data_2DB, var_r31, temp_r6, lbl_1_bss_3E80.padNos[var_r31]);
        var_r31 += 1;
    }
    var_r31 = 0;
    while (var_r31 < 4) {
        lbl_1_bss_3E80.secondaryModels[var_r31] = Hu3DModelCreate(HuDataSelHeapReadNum(4390975, 268435456, HEAP_MODEL));
        Hu3DModelShadowSet(lbl_1_bss_3E80.secondaryModels[var_r31]);
        Hu3DModelLayerSet(lbl_1_bss_3E80.secondaryModels[var_r31], 3);
        var_r31 += 1;
    }
    lbl_1_bss_3E80.models_184_199[0] = Hu3DModelCreate(HuDataSelHeapReadNum(4390974, 268435456, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_3E80.models_184_199[0], 3);
    var_r31 = 0;
    while (var_r31 < 27) {
        lbl_1_bss_3E80.models[var_r31] = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_694[var_r31], 268435456, HEAP_MODEL));
        lbl_1_bss_3E80.motions[var_r31] = Hu3DJointMotion(lbl_1_bss_3E80.models[var_r31], HuDataSelHeapReadNum(lbl_1_data_694[var_r31], 268435456, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_3E80.models[var_r31], 1U);
        lbl_1_bss_3E80.objects[var_r31] = omAddObjEx(lbl_1_bss_3E80.objectManager, 32730, 1U, 1U, -1, fn_1_388C);
        (lbl_1_bss_3E80.objects[var_r31])->work[0] = var_r31;
        var_r31 += 1;
    }
    var_r31 = 0;
    while (var_r31 < 4) {
        localLightId = Hu3DLLightCreate(lbl_1_bss_3E80.characterModels[var_r31], 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, -1.0f, 180U, 180U, 180U);
        Hu3DLLightInfinitytSet(lbl_1_bss_3E80.characterModels[var_r31], localLightId);
        var_r31 += 1;
    }
    fn_1_6150();
    fn_1_6534(4, (s16) GWRecordGet(GW_RECORD_M608));
    lbl_1_bss_3E80.record = GWRecordGet(GW_RECORD_M608);
    fn_1_69F4();
    fn_1_6148();
    lbl_1_bss_3E80.audioHandle_454 = -1;
    MgSeqCreate(&lbl_1_data_0);
}

char lbl_1_data_295[11] = "kurukuru14";

char lbl_1_data_2A0[16] = "kurukuru14-hsha";

char lbl_1_data_2B0[5] = "npc1";

char lbl_1_data_2B5[5] = "npc2";

char lbl_1_data_2BA[11] = "itemhook_L";

char lbl_1_data_2C5[11] = "itemhook_R";

char lbl_1_data_2D0[11] = "itemhook_C";

char lbl_1_data_2DB[29] = "*** %dP ... char %d  pad %d\012";
