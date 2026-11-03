/* Initializes Snow Whirled's course, riders, camera, score panels, and sequence. */
#define _MATH_H
#include <math.h>
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
    lbl_1_bss_3E80.objectManager = fn_8003A8BC(100, 8192);
    fn_80069F1C(lbl_1_bss_3E80.objectManager);
    fn_1_6828();
    lbl_80266A40.x = -20.0f;
    lbl_80266A40.y = 0.0f;
    lbl_80266A40.z = 0.0f;
    lbl_80266A4C.x = 0.0f;
    lbl_80266A4C.y = 100.0f;
    lbl_80266A4C.z = 0.0f;
    lbl_802C028C = 400.0f;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 20.0f, 800.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    lbl_1_bss_3E80.cameraObject =
        fn_8003AB38(lbl_1_bss_3E80.objectManager, 16, 0U, 0U, -1, fn_1_2540);
    index = 0;
    while (index < 10) {
        lbl_1_bss_3E80.cameraMotions[index] = fn_8002C4C0(
            fn_80008DD0(lbl_1_data_4F0[index], 0x10000000, 2));
        lbl_1_bss_3E80.cameraModels[index] =
            fn_800283F4(lbl_1_bss_3E80.cameraMotions[index], 1U);
        fn_8002858C(lbl_1_bss_3E80.cameraModels[index]);
        index += 1;
    }
    Hu3DShadowCreate(30.0f, 1.0f, 13000.0f);
    Hu3DShadowPosSet(&lbl_1_bss_3E80.shadowCenter, &lbl_1_bss_3E80.shadowUpDirection,
                     &lbl_1_bss_3E80.shadowTarget);
    fn_8002AAB4(50U, 50U, 50U);
    Hu3DShadowSizeSet(192U);
    Hu3DShadowTPLvlSet(0.7f);
    /* The later 240-pixel setting replaces the 192-pixel shadow size above. */
    fn_8002AAB4(50U, 50U, 50U);
    Hu3DShadowSizeSet(240U);
    index = 0;
    while (index < 4) {
        lbl_1_bss_3E80.playerModelsA[index] =
            fn_80025A04(fn_80008DD0(lbl_1_data_518[index], 0x10000000, 2));
        lbl_1_bss_3E80.playerModelsB[index] =
            fn_80025A04(fn_80008DD0(lbl_1_data_528[index], 0x10000000, 2));
        lbl_1_bss_3E80.playerModelsC[index] =
            fn_80025A04(fn_80008DD0(lbl_1_data_538[index], 0x10000000, 2));
        fn_80026B8C(lbl_1_bss_3E80.playerModelsA[index], 1);
        fn_80026B8C(lbl_1_bss_3E80.playerModelsB[index], 1);
        fn_80026B8C(lbl_1_bss_3E80.playerModelsC[index], 1);
        playerModelOffset.x = 0.0f;
        playerModelOffset.y = -2.0f;
        playerModelOffset.z = 0.0f;
        fn_80026790(lbl_1_bss_3E80.playerModelsB[index], &playerModelOffset);
        fn_80026790(lbl_1_bss_3E80.playerModelsC[index], &playerModelOffset);
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
        fn_80025A04(fn_80008DD0(DATANUM(0x430000, 44), 0x10000000, 2));
    lbl_1_bss_3E80.mainMotion =
        fn_80030458(lbl_1_bss_3E80.mainModel,
                        fn_80008DD0(DATANUM(0x430000, 45), 0x10000000, 2));
    fn_80026B8C(lbl_1_bss_3E80.mainModel, 2);
    fn_80026E44(lbl_1_bss_3E80.mainModel);
    lbl_1_bss_3E80.timingModels[0] =
        fn_80025A04(fn_80008DD0(DATANUM(0x430000, 58), 0x10000000, 2));
    lbl_1_bss_3E80.timingModels[1] =
        fn_80025A04(fn_80008DD0(DATANUM(0x430000, 59), 0x10000000, 2));
    /* The night flag selects a separate background, course set, and light setup. */
    isNightCourse = lbl_802C0278;
    if (isNightCourse == 0) {
        lbl_1_bss_3E80.sceneModels[0] =
            fn_80025A04(fn_80008DD0(DATANUM(0x430000, 0), 0x10000000, 2));
        lbl_1_bss_3E80.sceneModels[1] =
            fn_80025A04(fn_80008DD0(DATANUM(0x430000, 1), 0x10000000, 2));
        fn_800269E8(lbl_1_bss_3E80.sceneModels[0], ((1 << 30) | (1 << 0)));
        lbl_1_bss_3E80.courseModels[8] = fn_80025A04(
            fn_80008DD0(DATANUM(0x430000, 64), 0x10000000, 2));
        lbl_1_bss_3E80.courseModels[4] = fn_80025A04(
            fn_80008DD0(DATANUM(0x430000, 70), 0x10000000, 2));
        lbl_1_bss_3E80.courseMotions[0] = fn_80030458(
            lbl_1_bss_3E80.courseModels[4],
            fn_80008DD0(DATANUM(0x430000, 71), 0x10000000, 2));
        lbl_1_bss_3E80.courseMotions[1] = fn_80030458(
            lbl_1_bss_3E80.courseModels[4],
            fn_80008DD0(DATANUM(0x430000, 72), 0x10000000, 2));
        fn_800269E8(lbl_1_bss_3E80.courseModels[4], ((1 << 30) | (1 << 0)));
        lbl_1_bss_3E80.courseModels[6] = fn_80025A04(
            fn_80008DD0(DATANUM(0x430000, 83), 0x10000000, 2));
        lbl_1_bss_3E80.courseMotions[4] = fn_80030458(
            lbl_1_bss_3E80.courseModels[6],
            fn_80008DD0(DATANUM(0x430000, 84), 0x10000000, 2));
        lbl_1_bss_3E80.courseMotions[5] = fn_80030458(
            lbl_1_bss_3E80.courseModels[6],
            fn_80008DD0(DATANUM(0x430000, 85), 0x10000000, 2));
        fn_1_5FDC(5, 4);
        lbl_1_bss_3E80.courseModels[5] = fn_80025A04(
            fn_80008DD0(DATANUM(0x430000, 77), 0x10000000, 2));
        lbl_1_bss_3E80.courseMotions[2] = fn_80030458(
            lbl_1_bss_3E80.courseModels[5],
            fn_80008DD0(DATANUM(0x430000, 78), 0x10000000, 2));
        lbl_1_bss_3E80.courseMotions[3] = fn_80030458(
            lbl_1_bss_3E80.courseModels[5],
            fn_80008DD0(DATANUM(0x430000, 79), 0x10000000, 2));
        fn_1_5FDC(4, 2);
        fn_800269E8(lbl_1_bss_3E80.courseModels[5], ((1 << 30) | (1 << 0)));
        lbl_1_bss_3E80.courseModels[9] = fn_80025A04(
            fn_80008DD0(DATANUM(0x430000, 66), 0x10000000, 2));
        lbl_1_bss_3E80.courseModels[10] = fn_80025A04(
            fn_80008DD0(DATANUM(0x430000, 68), 0x10000000, 2));
        dayLightPosition = lbl_1_rodata_88;
        dayLightDirection = lbl_1_rodata_94;
        unusedLightVector = lbl_1_rodata_A0; /* Retained preset; this vector is not used below. */
        dayLightColor = lbl_1_rodata_AC;
        dayLightId = fn_800286D0(&dayLightPosition, &dayLightDirection, &dayLightColor);
        Hu3DGLightInfinitytSet(dayLightId);
        Hu3DGLightStaticSet(dayLightId, 1);
        lightTarget.x = 0.0f;
        lightTarget.y = 0.0f;
        lightTarget.z = 0.0f;
        lightPosition.x = 1.0f;
        lightPosition.y = 7825.0f;
        lightPosition.z = 9582.0f;
        Hu3DGLightPosAimSetV(dayLightId, &lightPosition, &lightTarget);
        fn_80027080(lbl_1_bss_3E80.sceneModels[0], "kurukuru14");
        fn_80027080(lbl_1_bss_3E80.sceneModels[0], "kurukuru14-hsha");
    } else {
        lbl_1_bss_3E80.sceneModels[0] =
            fn_80025A04(fn_80008DD0(DATANUM(0x430000, 3), 0x10000000, 2));
        lbl_1_bss_3E80.sceneModels[2] =
            fn_80025A04(fn_80008DD0(DATANUM(0x430000, 5), 0x10000000, 2));
        lbl_1_bss_3E80.sceneModels[3] =
            fn_80025A04(fn_80008DD0(DATANUM(0x430000, 2), 0x10000000, 2));
        lbl_1_bss_3E80.sceneModels[4] =
            fn_80025A04(fn_80008DD0(DATANUM(0x430000, 6), 0x10000000, 2));
        fn_80026AE4(lbl_1_bss_3E80.sceneModels[0], 0, ((s32)((3U << 30) | (1U << 0))));
        fn_800269E8(lbl_1_bss_3E80.sceneModels[2], ((1 << 30) | (1 << 0)));
        fn_800269E8(lbl_1_bss_3E80.sceneModels[3], ((1 << 30) | (1 << 0)));
        fn_800269E8(lbl_1_bss_3E80.sceneModels[4], ((1 << 30) | (1 << 0)));
        lbl_1_bss_3E80.courseModels[8] = fn_80025A04(
            fn_80008DD0(DATANUM(0x430000, 65), 0x10000000, 2));
        lbl_1_bss_3E80.courseModels[4] = fn_80025A04(
            fn_80008DD0(DATANUM(0x430000, 73), 0x10000000, 2));
        lbl_1_bss_3E80.courseMotions[0] = fn_80030458(
            lbl_1_bss_3E80.courseModels[4],
            fn_80008DD0(DATANUM(0x430000, 74), 0x10000000, 2));
        lbl_1_bss_3E80.courseMotions[1] = fn_80030458(
            lbl_1_bss_3E80.courseModels[4],
            fn_80008DD0(DATANUM(0x430000, 75), 0x10000000, 2));
        fn_800269E8(lbl_1_bss_3E80.courseModels[4], ((1 << 30) | (1 << 0)));
        lbl_1_bss_3E80.courseModels[6] = fn_80025A04(
            fn_80008DD0(DATANUM(0x430000, 83), 0x10000000, 2));
        lbl_1_bss_3E80.courseMotions[4] = fn_80030458(
            lbl_1_bss_3E80.courseModels[6],
            fn_80008DD0(DATANUM(0x430000, 84), 0x10000000, 2));
        lbl_1_bss_3E80.courseMotions[5] = fn_80030458(
            lbl_1_bss_3E80.courseModels[6],
            fn_80008DD0(DATANUM(0x430000, 85), 0x10000000, 2));
        fn_1_5FDC(5, 4);
        lbl_1_bss_3E80.courseModels[5] = fn_80025A04(
            fn_80008DD0(DATANUM(0x430000, 80), 0x10000000, 2));
        lbl_1_bss_3E80.courseMotions[2] = fn_80030458(
            lbl_1_bss_3E80.courseModels[5],
            fn_80008DD0(DATANUM(0x430000, 81), 0x10000000, 2));
        lbl_1_bss_3E80.courseMotions[3] = fn_80030458(
            lbl_1_bss_3E80.courseModels[5],
            fn_80008DD0(DATANUM(0x430000, 82), 0x10000000, 2));
        fn_1_5FDC(4, 2);
        fn_800269E8(lbl_1_bss_3E80.courseModels[5], ((1 << 30) | (1 << 0)));
        lbl_1_bss_3E80.courseModels[9] = fn_80025A04(
            fn_80008DD0(DATANUM(0x430000, 67), 0x10000000, 2));
        lbl_1_bss_3E80.courseModels[10] = fn_80025A04(
            fn_80008DD0(DATANUM(0x430000, 69), 0x10000000, 2));
        nightLightPosition = lbl_1_rodata_B0;
        nightLightDirection = lbl_1_rodata_BC;
        unusedNightLightVector = lbl_1_rodata_C8; /* Retained preset; this vector is not used
                                                   * below. */
        nightLightColor = lbl_1_rodata_D4;
        nightLightId =
            fn_800286D0(&nightLightPosition, &nightLightDirection, &nightLightColor);
        Hu3DGLightPointSet(nightLightId, 1000.0f, 1.0f, GX_DA_OFF);
        Hu3DGLightStaticSet(nightLightId, 1);
    }
    index = 0;
    while (index < 4) {
        fn_80026E44(lbl_1_bss_3E80.playerModelsA[index]);
        fn_80026E44(lbl_1_bss_3E80.playerModelsB[index]);
        fn_80026E44(lbl_1_bss_3E80.playerModelsC[index]);
        index += 1;
    }
    lbl_1_bss_3E80.courseModels[1] =
        fn_80025A04(fn_80008DD0(DATANUM(0x430000, 60), 0x10000000, 2));
    lbl_1_bss_3E80.courseModels[2] =
        fn_80025A04(fn_80008DD0(DATANUM(0x430000, 61), 0x10000000, 2));
    fn_1_5F90(0);
    fn_1_5F90(1);
    fn_1_5F90(7);
    fn_1_5F90(8);
    fn_1_5F90(9);
    fn_1_5F90(5);
    lbl_1_bss_3E80.featuredRiderIndex = (s32) (frand() & 0x3);
    lbl_1_bss_3E80.courseModels[3] =
        fn_80025A04(fn_80008DD0(DATANUM(0x430000, 76), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_3E80.courseModels[3], ((1 << 30) | (1 << 0)));
    lbl_1_bss_3E80.courseModels[7] =
        fn_80025A04(fn_80008DD0(DATANUM(0x430000, 90), 0x10000000, 2));
    lbl_1_bss_3E80.courseMotions[6] =
        fn_80030458(lbl_1_bss_3E80.courseModels[7],
                        fn_80008DD0(DATANUM(0x430000, 91), 0x10000000, 2));
    lbl_1_bss_3E80.courseMotions[7] =
        fn_80030458(lbl_1_bss_3E80.courseModels[7],
                        fn_80008DD0(DATANUM(0x430000, 92), 0x10000000, 2));
    {
        /* The ten named sockets carry spectators; two use motion 6 and the other eight use motion
         * 7. */
        char *hookNames[10] = {
            "NPC3", "NPC4", "NPC5", "NPC6", "NPC7", "NPC8", "NPC9", "NPC10", "NPC11", "NPC12"
        };
        BOOL useMotion7[10] = { TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE };
    index = 0;
    while (index < 10) {
        linkedModelId = fn_80025D4C(lbl_1_bss_3E80.courseModels[7]);
        npcMotionIndex = useMotion7[index] ? 7 : 6;
        fn_8002741C(lbl_1_bss_3E80.courseModels[3], hookNames[index], linkedModelId);
        fn_8002C95C(linkedModelId, lbl_1_bss_3E80.courseMotions[npcMotionIndex]);
        motionTime = fn_8002D530(linkedModelId);
        fn_8002D414(linkedModelId, motionTime * frandf());
        fn_800269E8(linkedModelId, ((1 << 30) | (1 << 0)));
        localLightId =
            Hu3DLLightCreate(linkedModelId, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 128U, 128U, 128U);
        fn_80028B58(linkedModelId, localLightId);
        index += 1;
    }
    }
    npcModelId = lbl_1_bss_3E80.courseModels[7];
    fn_8002741C(lbl_1_bss_3E80.courseModels[5], "npc1", npcModelId);
    fn_8002C95C(npcModelId, lbl_1_bss_3E80.courseMotions[6]);
    fn_800269E8(npcModelId, ((1 << 30) | (1 << 0)));
    lbl_1_bss_3E80.courseModels[11] =
        fn_80025A04(fn_80008DD0(DATANUM(0x430000, 93), 0x10000000, 2));
    lbl_1_bss_3E80.courseMotions[8] =
        fn_80030458(lbl_1_bss_3E80.courseModels[11],
                        fn_80008DD0(DATANUM(0x430000, 94), 0x10000000, 2));
    lbl_1_bss_3E80.courseModels[12] =
        fn_80025A04(fn_80008DD0(DATANUM(0x430000, 86), 0x10000000, 2));
    lbl_1_bss_3E80.courseModels[13] =
        fn_80025A04(fn_80008DD0(DATANUM(0x430000, 87), 0x10000000, 2));
    lbl_1_bss_3E80.courseModels[14] =
        fn_80025A04(fn_80008DD0(DATANUM(0x430000, 88), 0x10000000, 2));
    lbl_1_bss_3E80.courseModels[15] =
        fn_80025A04(fn_80008DD0(DATANUM(0x430000, 89), 0x10000000, 2));
    courseNpcModelId = lbl_1_bss_3E80.courseModels[11];
    fn_8002741C(lbl_1_bss_3E80.courseModels[6], "npc2", courseNpcModelId);
    fn_8002741C(courseNpcModelId, "itemhook_L", lbl_1_bss_3E80.courseModels[15]);
    fn_8002741C(courseNpcModelId, "itemhook_R", lbl_1_bss_3E80.courseModels[15]);
    fn_8002741C(courseNpcModelId, "itemhook_C", lbl_1_bss_3E80.courseModels[12]);
    fn_800269E8(lbl_1_bss_3E80.courseModels[11], ((1 << 30) | (1 << 0)));
    index = 0;
    while (index < 4) {
        characterNumber = lbl_1_bss_3E80.characterNos[index] = lbl_80265728[index].charNo;
        lbl_1_bss_3E80.padNos[index] = lbl_80265728[index].padNo;
        lbl_1_bss_3E80.characterModels[index] = fn_8005C4E4(
            (s16) characterNumber, 4, lbl_1_data_548, lbl_1_bss_3E80.characterMotions[index]);
        lbl_1_bss_3E80.playerObjects[index] =
            fn_8003AB38(lbl_1_bss_3E80.objectManager, 8704, 1U, 0U, -1, NULL);
        *(lbl_1_bss_3E80.playerObjects[index])->mdlId = lbl_1_bss_3E80.characterModels[index];
        fn_80026E44(lbl_1_bss_3E80.characterModels[index]);
        fn_8005B4BC((s16) characterNumber);
        OSReport(
            "*** %dP ... char %d  pad %d\012", index, characterNumber,
            lbl_1_bss_3E80.padNos[index]);
        index += 1;
    }
    index = 0;
    while (index < 4) {
        lbl_1_bss_3E80.secondaryModels[index] = fn_80025A04(
            fn_80008DD0(DATANUM(0x430000, 63), 0x10000000, 2));
        fn_80026E44(lbl_1_bss_3E80.secondaryModels[index]);
        fn_80026B8C(lbl_1_bss_3E80.secondaryModels[index], 3);
        index += 1;
    }
    lbl_1_bss_3E80.courseModels[0] =
        fn_80025A04(fn_80008DD0(DATANUM(0x430000, 62), 0x10000000, 2));
    fn_80026B8C(lbl_1_bss_3E80.courseModels[0], 3);
    index = 0;
    while (index < 27) {
        lbl_1_bss_3E80.movingCourseModels[index] =
            fn_80025A04(fn_80008DD0(lbl_1_data_694[index], 0x10000000, 2));
        lbl_1_bss_3E80.movingCourseMotions[index] =
            fn_80030458(lbl_1_bss_3E80.movingCourseModels[index],
                            fn_80008DD0(lbl_1_data_694[index], 0x10000000, 2));
        fn_800269E8(lbl_1_bss_3E80.movingCourseModels[index], (1 << 0));
        lbl_1_bss_3E80.movingCourseObjects[index] =
            fn_8003AB38(lbl_1_bss_3E80.objectManager, 32730, 1U, 1U, -1, fn_1_388C);
        (lbl_1_bss_3E80.movingCourseObjects[index])->work[0] = index;
        index += 1;
    }
    index = 0;
    while (index < 4) {
        localLightId = Hu3DLLightCreate(lbl_1_bss_3E80.characterModels[index], 0.0f, 0.0f, 0.0f,
                                        0.0f, -1.0f, -1.0f, 180U, 180U, 180U);
        fn_80028B58(lbl_1_bss_3E80.characterModels[index], localLightId);
        index += 1;
    }
    fn_1_6150();
    fn_1_6534(4, (s16) fn_8003C860(1));
    lbl_1_bss_3E80.record = fn_8003C860(1);
    fn_1_69F4();
    fn_1_6148();
    lbl_1_bss_3E80.courseMusicStreamHandle = -1;
    fn_80070788(&lbl_1_data_0);
}
