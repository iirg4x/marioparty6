/* Initializes Snow Whirled's course, riders, camera, score panels, and sequence. */
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

/* Called by _prolog before MgSeqCreate starts play; builds the course and all rider objects. */
void fn_1_43C8(void)
{
    Point3d playerModelOffset;
    Point3d lightTarget;
    Point3d lightPosition;
    Point3d dayLightPosition;
    Point3d dayLightDirection;
    Point3d unusedLightVector;
    Point3d nightLightPosition;
    Point3d nightLightDirection;
    Point3d unusedNightLightVector;
    GXColor dayLightColor;
    GXColor nightLightColor;
    f32 motionTime;
    int characterNumber;
    HU3D_LLIGHTID localLightId;
    s16 dayLightId;
    s16 npcModelId;
    s16 courseNpcModelId;
    s16 linkedModelId;
    s16 nightLightId;
    s16 npcMotionIndex;
    s16 isNightCourse;
    s32 index;

    Hu3DParManInit();
    memset(&lbl_1_bss_3E80, 0, sizeof(M608SceneConsumedContext));
    lbl_1_bss_3E80.activeCameraMotion = -1;
    lbl_1_bss_3E80.shadowCenter.x = 0.0f;
    lbl_1_bss_3E80.shadowCenter.y = 10000.0f;
    lbl_1_bss_3E80.shadowCenter.z = 0.0f;
    lbl_1_bss_3E80.shadowUpDirection.x = 0.0f;
    lbl_1_bss_3E80.shadowUpDirection.y = 0.0f;
    lbl_1_bss_3E80.shadowUpDirection.z = 1.0f;
    lbl_1_bss_3E80.shadowTarget.x = 0.0f;
    lbl_1_bss_3E80.shadowTarget.y = -100.0f;
    lbl_1_bss_3E80.shadowTarget.z = 0.0f;
    lbl_1_bss_3E80.recordBeaten = 0;
    lbl_1_bss_3E80.coursePathSampleCount = 0;
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
    lbl_1_bss_3E80.cameraObject =
        omAddObjEx(lbl_1_bss_3E80.objectManager, 16, 0U, 0U, -1, fn_1_2540);
    index = 0;
    while (index < 10) {
        lbl_1_bss_3E80.cameraMotions[index] = Hu3DMotionCreate(
            HuDataSelHeapReadNum(lbl_1_data_4F0[index], HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.cameraModels[index] =
            Hu3DModelCameraCreate(lbl_1_bss_3E80.cameraMotions[index], 1U);
        Hu3DCameraMotionOff(lbl_1_bss_3E80.cameraModels[index]);
        index += 1;
    }
    Hu3DShadowCreate(30.0f, 1.0f, 13000.0f);
    Hu3DShadowPosSet(&lbl_1_bss_3E80.shadowCenter, &lbl_1_bss_3E80.shadowUpDirection,
                     &lbl_1_bss_3E80.shadowTarget);
    Hu3DShadowColSet(50U, 50U, 50U);
    Hu3DShadowSizeSet(192U);
    Hu3DShadowTPLvlSet(0.7f);
    /* The later 240-pixel setting replaces the 192-pixel shadow size above. */
    Hu3DShadowColSet(50U, 50U, 50U);
    Hu3DShadowSizeSet(240U);
    index = 0;
    while (index < 4) {
        lbl_1_bss_3E80.playerModelsA[index] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_518[index], HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.playerModelsB[index] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_528[index], HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.playerModelsC[index] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_538[index], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelLayerSet(lbl_1_bss_3E80.playerModelsA[index], 1);
        Hu3DModelLayerSet(lbl_1_bss_3E80.playerModelsB[index], 1);
        Hu3DModelLayerSet(lbl_1_bss_3E80.playerModelsC[index], 1);
        playerModelOffset.x = 0.0f;
        playerModelOffset.y = -2.0f;
        playerModelOffset.z = 0.0f;
        Hu3DModelPosSetV(lbl_1_bss_3E80.playerModelsB[index], &playerModelOffset);
        Hu3DModelPosSetV(lbl_1_bss_3E80.playerModelsC[index], &playerModelOffset);
        index += 1;
    }
    index = 0;
    while (index < 4) {
        Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsB[index], lbl_1_data_684[index],
                           &lbl_1_bss_3E80.playerBoardHookPositions[index]);
        Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsB[index], lbl_1_data_624[index],
                           &lbl_1_bss_3E80.playerAttachmentPositions[index]);
        Hu3DModelObjPosGet(lbl_1_bss_3E80.playerModelsC[index], lbl_1_data_64C[index],
                           &lbl_1_bss_3E80.playerAttachmentPositions[index]);
        index += 1;
    }
    lbl_1_bss_3E80.mainModel =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m608, 44), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_3E80.mainMotion =
        Hu3DJointMotion(lbl_1_bss_3E80.mainModel,
                        HuDataSelHeapReadNum(DATANUM(DATA_m608, 45), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_3E80.mainModel, 2);
    Hu3DModelShadowSet(lbl_1_bss_3E80.mainModel);
    lbl_1_bss_3E80.timingModels[0] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m608, 58), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_3E80.timingModels[1] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m608, 59), HU_MEMNUM_OVL, HEAP_MODEL));
    /* The night flag selects a separate background, course set, and light setup. */
    isNightCourse = GwMgNightF;
    if (isNightCourse == 0) {
        lbl_1_bss_3E80.sceneModels[0] =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m608, 0), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.sceneModels[1] =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m608, 1), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_3E80.sceneModels[0], HU3D_MOTATTR_LOOP);
        lbl_1_bss_3E80.courseModels[8] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 64), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.courseModels[4] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 70), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.courseMotions[0] = Hu3DJointMotion(
            lbl_1_bss_3E80.courseModels[4],
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 71), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.courseMotions[1] = Hu3DJointMotion(
            lbl_1_bss_3E80.courseModels[4],
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 72), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_3E80.courseModels[4], HU3D_MOTATTR_LOOP);
        lbl_1_bss_3E80.courseModels[6] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 83), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.courseMotions[4] = Hu3DJointMotion(
            lbl_1_bss_3E80.courseModels[6],
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 84), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.courseMotions[5] = Hu3DJointMotion(
            lbl_1_bss_3E80.courseModels[6],
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 85), HU_MEMNUM_OVL, HEAP_MODEL));
        fn_1_5FDC(5, 4);
        lbl_1_bss_3E80.courseModels[5] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 77), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.courseMotions[2] = Hu3DJointMotion(
            lbl_1_bss_3E80.courseModels[5],
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 78), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.courseMotions[3] = Hu3DJointMotion(
            lbl_1_bss_3E80.courseModels[5],
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 79), HU_MEMNUM_OVL, HEAP_MODEL));
        fn_1_5FDC(4, 2);
        Hu3DModelAttrSet(lbl_1_bss_3E80.courseModels[5], HU3D_MOTATTR_LOOP);
        lbl_1_bss_3E80.courseModels[9] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 66), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.courseModels[10] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 68), HU_MEMNUM_OVL, HEAP_MODEL));
        dayLightPosition = lbl_1_rodata_88;
        dayLightDirection = lbl_1_rodata_94;
        unusedLightVector = lbl_1_rodata_A0; /* Retained preset; this vector is not used below. */
        dayLightColor = lbl_1_rodata_AC;
        dayLightId = Hu3DGLightCreateV(&dayLightPosition, &dayLightDirection, &dayLightColor);
        Hu3DGLightInfinitytSet(dayLightId);
        Hu3DGLightStaticSet(dayLightId, 1);
        lightTarget.x = 0.0f;
        lightTarget.y = 0.0f;
        lightTarget.z = 0.0f;
        lightPosition.x = 1.0f;
        lightPosition.y = 7825.0f;
        lightPosition.z = 9582.0f;
        Hu3DGLightPosAimSetV(dayLightId, &lightPosition, &lightTarget);
        Hu3DModelShadowMapObjSet(lbl_1_bss_3E80.sceneModels[0], lbl_1_data_295);
        Hu3DModelShadowMapObjSet(lbl_1_bss_3E80.sceneModels[0], lbl_1_data_2A0);
    } else {
        lbl_1_bss_3E80.sceneModels[0] =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m608, 3), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.sceneModels[2] =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m608, 5), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.sceneModels[3] =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m608, 2), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.sceneModels[4] =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m608, 6), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelClusterAttrSet(lbl_1_bss_3E80.sceneModels[0], 0, HU3D_CLUSTER_ATTR_LOOP);
        Hu3DModelAttrSet(lbl_1_bss_3E80.sceneModels[2], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(lbl_1_bss_3E80.sceneModels[3], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(lbl_1_bss_3E80.sceneModels[4], HU3D_MOTATTR_LOOP);
        lbl_1_bss_3E80.courseModels[8] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 65), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.courseModels[4] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 73), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.courseMotions[0] = Hu3DJointMotion(
            lbl_1_bss_3E80.courseModels[4],
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 74), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.courseMotions[1] = Hu3DJointMotion(
            lbl_1_bss_3E80.courseModels[4],
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 75), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_3E80.courseModels[4], HU3D_MOTATTR_LOOP);
        lbl_1_bss_3E80.courseModels[6] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 83), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.courseMotions[4] = Hu3DJointMotion(
            lbl_1_bss_3E80.courseModels[6],
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 84), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.courseMotions[5] = Hu3DJointMotion(
            lbl_1_bss_3E80.courseModels[6],
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 85), HU_MEMNUM_OVL, HEAP_MODEL));
        fn_1_5FDC(5, 4);
        lbl_1_bss_3E80.courseModels[5] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 80), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.courseMotions[2] = Hu3DJointMotion(
            lbl_1_bss_3E80.courseModels[5],
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 81), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.courseMotions[3] = Hu3DJointMotion(
            lbl_1_bss_3E80.courseModels[5],
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 82), HU_MEMNUM_OVL, HEAP_MODEL));
        fn_1_5FDC(4, 2);
        Hu3DModelAttrSet(lbl_1_bss_3E80.courseModels[5], HU3D_MOTATTR_LOOP);
        lbl_1_bss_3E80.courseModels[9] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 67), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.courseModels[10] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 69), HU_MEMNUM_OVL, HEAP_MODEL));
        nightLightPosition = lbl_1_rodata_B0;
        nightLightDirection = lbl_1_rodata_BC;
        unusedNightLightVector = lbl_1_rodata_C8; /* Retained preset; this vector is not used
                                                   * below. */
        nightLightColor = lbl_1_rodata_D4;
        nightLightId =
            Hu3DGLightCreateV(&nightLightPosition, &nightLightDirection, &nightLightColor);
        Hu3DGLightPointSet(nightLightId, 1000.0f, 1.0f, GX_DA_OFF);
        Hu3DGLightStaticSet(nightLightId, 1);
    }
    index = 0;
    while (index < 4) {
        Hu3DModelShadowSet(lbl_1_bss_3E80.playerModelsA[index]);
        Hu3DModelShadowSet(lbl_1_bss_3E80.playerModelsB[index]);
        Hu3DModelShadowSet(lbl_1_bss_3E80.playerModelsC[index]);
        index += 1;
    }
    lbl_1_bss_3E80.courseModels[1] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m608, 60), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_3E80.courseModels[2] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m608, 61), HU_MEMNUM_OVL, HEAP_MODEL));
    fn_1_5F90(0);
    fn_1_5F90(1);
    fn_1_5F90(7);
    fn_1_5F90(8);
    fn_1_5F90(9);
    fn_1_5F90(5);
    lbl_1_bss_3E80.featuredRiderIndex = (s32) (frand() & 0x3);
    lbl_1_bss_3E80.courseModels[3] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m608, 76), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_3E80.courseModels[3], HU3D_MOTATTR_LOOP);
    lbl_1_bss_3E80.courseModels[7] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m608, 90), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_3E80.courseMotions[6] =
        Hu3DJointMotion(lbl_1_bss_3E80.courseModels[7],
                        HuDataSelHeapReadNum(DATANUM(DATA_m608, 91), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_3E80.courseMotions[7] =
        Hu3DJointMotion(lbl_1_bss_3E80.courseModels[7],
                        HuDataSelHeapReadNum(DATANUM(DATA_m608, 92), HU_MEMNUM_OVL, HEAP_MODEL));
    {
        /* The ten named sockets carry spectators; two use motion 6 and the other eight use motion
         * 7. */
        char *hookNames[10] = {
            "NPC3", "NPC4", "NPC5", "NPC6", "NPC7", "NPC8", "NPC9", "NPC10", "NPC11", "NPC12"
        };
        BOOL useMotion7[10] = { TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE };
    index = 0;
    while (index < 10) {
        linkedModelId = Hu3DModelLink(lbl_1_bss_3E80.courseModels[7]);
        npcMotionIndex = useMotion7[index] ? 7 : 6;
        Hu3DModelHookSet(lbl_1_bss_3E80.courseModels[3], hookNames[index], linkedModelId);
        Hu3DMotionSet(linkedModelId, lbl_1_bss_3E80.courseMotions[npcMotionIndex]);
        motionTime = Hu3DMotionMaxTimeGet(linkedModelId);
        Hu3DMotionTimeSet(linkedModelId, motionTime * frandf());
        Hu3DModelAttrSet(linkedModelId, HU3D_MOTATTR_LOOP);
        localLightId =
            Hu3DLLightCreate(linkedModelId, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 128U, 128U, 128U);
        Hu3DLLightInfinitytSet(linkedModelId, localLightId);
        index += 1;
    }
    }
    npcModelId = lbl_1_bss_3E80.courseModels[7];
    Hu3DModelHookSet(lbl_1_bss_3E80.courseModels[5], lbl_1_data_2B0, npcModelId);
    Hu3DMotionSet(npcModelId, lbl_1_bss_3E80.courseMotions[6]);
    Hu3DModelAttrSet(npcModelId, HU3D_MOTATTR_LOOP);
    lbl_1_bss_3E80.courseModels[11] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m608, 93), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_3E80.courseMotions[8] =
        Hu3DJointMotion(lbl_1_bss_3E80.courseModels[11],
                        HuDataSelHeapReadNum(DATANUM(DATA_m608, 94), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_3E80.courseModels[12] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m608, 86), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_3E80.courseModels[13] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m608, 87), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_3E80.courseModels[14] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m608, 88), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_3E80.courseModels[15] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m608, 89), HU_MEMNUM_OVL, HEAP_MODEL));
    courseNpcModelId = lbl_1_bss_3E80.courseModels[11];
    Hu3DModelHookSet(lbl_1_bss_3E80.courseModels[6], lbl_1_data_2B5, courseNpcModelId);
    Hu3DModelHookSet(courseNpcModelId, lbl_1_data_2BA, lbl_1_bss_3E80.courseModels[15]);
    Hu3DModelHookSet(courseNpcModelId, lbl_1_data_2C5, lbl_1_bss_3E80.courseModels[15]);
    Hu3DModelHookSet(courseNpcModelId, lbl_1_data_2D0, lbl_1_bss_3E80.courseModels[12]);
    Hu3DModelAttrSet(lbl_1_bss_3E80.courseModels[11], HU3D_MOTATTR_LOOP);
    index = 0;
    while (index < 4) {
        characterNumber = lbl_1_bss_3E80.characterNos[index] = GwPlayerConf[index].charNo;
        lbl_1_bss_3E80.padNos[index] = GwPlayerConf[index].padNo;
        lbl_1_bss_3E80.characterModels[index] = CharModelMotListCreate(
            (s16) characterNumber, 4, lbl_1_data_548, lbl_1_bss_3E80.characterMotions[index]);
        lbl_1_bss_3E80.playerObjects[index] =
            omAddObjEx(lbl_1_bss_3E80.objectManager, 8704, 1U, 0U, -1, NULL);
        *(lbl_1_bss_3E80.playerObjects[index])->mdlId = lbl_1_bss_3E80.characterModels[index];
        Hu3DModelShadowSet(lbl_1_bss_3E80.characterModels[index]);
        CharMotionDataClose((s16) characterNumber);
        OSReport(lbl_1_data_2DB, index, characterNumber, lbl_1_bss_3E80.padNos[index]);
        index += 1;
    }
    index = 0;
    while (index < 4) {
        lbl_1_bss_3E80.secondaryModels[index] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m608, 63), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelShadowSet(lbl_1_bss_3E80.secondaryModels[index]);
        Hu3DModelLayerSet(lbl_1_bss_3E80.secondaryModels[index], 3);
        index += 1;
    }
    lbl_1_bss_3E80.courseModels[0] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m608, 62), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_3E80.courseModels[0], 3);
    index = 0;
    while (index < 27) {
        lbl_1_bss_3E80.movingCourseModels[index] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_694[index], HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3E80.movingCourseMotions[index] =
            Hu3DJointMotion(lbl_1_bss_3E80.movingCourseModels[index],
                            HuDataSelHeapReadNum(lbl_1_data_694[index], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_3E80.movingCourseModels[index], HU3D_ATTR_DISPOFF);
        lbl_1_bss_3E80.movingCourseObjects[index] =
            omAddObjEx(lbl_1_bss_3E80.objectManager, 32730, 1U, 1U, -1, fn_1_388C);
        (lbl_1_bss_3E80.movingCourseObjects[index])->work[0] = index;
        index += 1;
    }
    index = 0;
    while (index < 4) {
        localLightId = Hu3DLLightCreate(lbl_1_bss_3E80.characterModels[index], 0.0f, 0.0f, 0.0f,
                                        0.0f, -1.0f, -1.0f, 180U, 180U, 180U);
        Hu3DLLightInfinitytSet(lbl_1_bss_3E80.characterModels[index], localLightId);
        index += 1;
    }
    fn_1_6150();
    fn_1_6534(4, (s16) GWRecordGet(GW_RECORD_M608));
    lbl_1_bss_3E80.record = GWRecordGet(GW_RECORD_M608);
    fn_1_69F4();
    fn_1_6148();
    lbl_1_bss_3E80.courseMusicStreamHandle = -1;
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
