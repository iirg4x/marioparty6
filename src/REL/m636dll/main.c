/* Pixel Perfect scene, floor panel and player setup. */
#include <math.h>
#include "REL/m636dll.h"

/* Player action mask with punch and kick cleared. */


#define PIXEL_MUSIC_HANDLE_OFFSET 628
#define PIXEL_EXTRA_SCENE_MODEL_OFFSET 632
#define PIXEL_EXTRA_SCENE_MOTION_OFFSET 634




#define PIXEL_ACTOR_INITIAL_FLAGS 0

#define PIXEL_STAGE_RESOURCE DATANUM(0x5f0000, 0)
#define PIXEL_COLLISION_RESOURCE DATANUM(0x5f0000, 1)
#define PIXEL_REFERENCE_RESOURCE DATANUM(0x5f0000, 3)
#define PIXEL_REFERENCE_CELL_0_RESOURCE DATANUM(0x5f0000, 5)
#define PIXEL_REFERENCE_CELL_1_RESOURCE DATANUM(0x5f0000, 6)
#define PIXEL_REFERENCE_CELL_2_RESOURCE DATANUM(0x5f0000, 7)
#define PIXEL_REFERENCE_CELL_3_RESOURCE DATANUM(0x5f0000, 8)
#define PIXEL_REFERENCE_CELL_4_RESOURCE DATANUM(0x5f0000, 9)
#define PIXEL_REFERENCE_CELL_5_RESOURCE DATANUM(0x5f0000, 10)
#define PIXEL_REFERENCE_CELL_6_RESOURCE DATANUM(0x5f0000, 11)
#define PIXEL_REFERENCE_CELL_7_RESOURCE DATANUM(0x5f0000, 12)
#define PIXEL_REFERENCE_CELL_8_RESOURCE DATANUM(0x5f0000, 13)
#define PIXEL_REFERENCE_CELL_9_RESOURCE DATANUM(0x5f0000, 14)
#define PIXEL_REFERENCE_CELL_10_RESOURCE DATANUM(0x5f0000, 15)
#define PIXEL_REFERENCE_CELL_11_RESOURCE DATANUM(0x5f0000, 16)
#define PIXEL_REFERENCE_CELL_12_RESOURCE DATANUM(0x5f0000, 17)
#define PIXEL_REFERENCE_CELL_13_RESOURCE DATANUM(0x5f0000, 18)
#define PIXEL_REFERENCE_CELL_14_RESOURCE DATANUM(0x5f0000, 19)
#define PIXEL_REFERENCE_CELL_15_RESOURCE DATANUM(0x5f0000, 20)
#define PIXEL_SCENE_RESOURCE DATANUM(0x5f0000, 21)
#define PIXEL_ROUND_INDICATOR_RESOURCE DATANUM(0x5f0000, 34)
#define PIXEL_ROUND_IDLE_RESOURCE DATANUM(0x5f0000, 35)
#define PIXEL_ROUND_RESET_RESOURCE DATANUM(0x5f0000, 36)
#define PIXEL_ROUND_LEFT_RESULT_RESOURCE DATANUM(0x5f0000, 37)
#define PIXEL_ROUND_RIGHT_RESULT_RESOURCE DATANUM(0x5f0000, 38)
#define PIXEL_FINISH_MODEL_RESOURCE DATANUM(0x5f0000, 39)
#define PIXEL_FINISH_INITIAL_MOTION_RESOURCE DATANUM(0x5f0000, 40)
#define PIXEL_FINISH_END_MOTION_RESOURCE DATANUM(0x5f0000, 41)
#define PIXEL_PANEL_MODEL_RESOURCE DATANUM(0x5f0000, 50)
#define PIXEL_PANEL_OFF_RESOURCE DATANUM(0x5f0000, 51)
#define PIXEL_PANEL_ON_RESOURCE DATANUM(0x5f0000, 52)
#define PIXEL_PANEL_TURN_ON_RESOURCE DATANUM(0x5f0000, 53)
#define PIXEL_PANEL_TURN_OFF_RESOURCE DATANUM(0x5f0000, 54)
#define PIXEL_EXTRA_SCENE_MODEL_RESOURCE DATANUM(0x5f0000, 55)
#define PIXEL_EXTRA_SCENE_MOTION_RESOURCE DATANUM(0x5f0000, 56)

/* Player movement, result and ground-pound motions; the final zero terminates the list. */
unsigned int lbl_1_data_E0[11] = {
    CHARMOT_HSF_c000m1_300, CHARMOT_HSF_c000m1_301, CHARMOT_HSF_c000m1_302, CHARMOT_HSF_c000m1_303,
    CHARMOT_HSF_c000m1_304, CHARMOT_HSF_c000m1_306, CHARMOT_HSF_c000m1_307, CHARMOT_HSF_c000m1_308,
    CHARMOT_HSF_c000m1_310, CHARMOT_HSF_c000m1_309
};

/* At overlay startup, creates the actor manager, camera, update objects and minigame sequence. */
void fn_1_1F98(void) {
    s16 lightId;

    lbl_1_bss_0.process = fn_80080418();
    lbl_80266A40.x = -35.0f;
    lbl_80266A40.y = 0.0f;
    lbl_80266A40.z = 0.0f;
    lbl_80266A4C.x = 0.0f;
    lbl_80266A4C.y = 700.0f;
    lbl_80266A4C.z = 400.0f;
    lbl_802C028C = 1000.0f;
    Hu3DCameraCreate((1 << 0));
    Hu3DCameraPerspectiveSet((1 << 0), 45.0f, 20.0f, 15000.0f, 1.2f);
    Hu3DCameraViewportSet((1 << 0), 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    lbl_1_bss_0.cameraObject =
        fn_8003AB38(lbl_1_bss_0.process, 32730,
                   0U, 0U, -1, fn_1_6360);
    lbl_1_bss_0.cameraModel = -1;
    lightId =
        Hu3DGLightCreate(0.0f, 1000.0f, 1000.0f, 0.0f, -1.0f, -1.0f,
                         255U, 255U, 255U);
    lbl_1_bss_0.timer = MgTimerCreate(0);
    fn_8003AB38(lbl_1_bss_0.process, 10,
               10U, 0U, -1, fn_1_21E4);
    fn_80070788(&lbl_1_data_0);
}

/* The scene object loads models and motions, builds both floor grids and creates all four players
 * once. */
void fn_1_21E4(OMOBJ *setupObject)
{
    s32 setupIndex;
    s32 column;
    s32 row;
    s32 cell;
    s16 panelModel;
    s16 lightId;
    f32 boardX;
    f32 boardY;
    f32 boardZ;
    s32 side;
    MGACTOR_PARAM actorParam;
    s16 collisionModel;

    for (setupIndex = 0; setupIndex < 2; setupIndex++) {
        lbl_1_bss_0.cameraMotions[setupIndex] = fn_8002C4C0(
            fn_80008DD0(lbl_1_data_280[setupIndex], 0x10000000, 2));
    }
    {
    Point3d lightPos = {0.0f, 500.0f, 0.0f};
    Point3d lightDir = {0.0f, -1.0f, -1.0f};
    GXColor lightColor = {255, 255, 255, 255};
    Point3d shadowPos;
    Point3d shadowUp;
    Point3d shadowTarget;
    lightId = fn_800286D0(&lightPos, &lightDir, &lightColor);
    Hu3DGLightInfinitytSet(lightId);
    shadowPos.x = 0.0f;
    shadowPos.y = 10000.0f;
    shadowPos.z = 0.0f;
    shadowUp.x = 0.0f;
    shadowUp.y = 1.0f;
    shadowUp.z = 0.0f;
    shadowTarget.x = 0.0f;
    shadowTarget.y = 0.0f;
    shadowTarget.z = -0.1f;
    Hu3DShadowCreate(8.5f, 5000.0f, 13000.0f);
    Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);
    fn_1_6478();
    fn_1_6790(1);

    lbl_1_bss_0.roundIndicatorModel = READ_MODEL(PIXEL_ROUND_INDICATOR_RESOURCE);
    lbl_1_bss_0.roundIndicatorMotions[0] =
        READ_MOTION(lbl_1_bss_0.roundIndicatorModel, PIXEL_ROUND_IDLE_RESOURCE);
    lbl_1_bss_0.roundIndicatorMotions[1] =
        READ_MOTION(lbl_1_bss_0.roundIndicatorModel, PIXEL_ROUND_RESET_RESOURCE);
    lbl_1_bss_0.roundIndicatorMotions[2] =
        READ_MOTION(lbl_1_bss_0.roundIndicatorModel, PIXEL_ROUND_LEFT_RESULT_RESOURCE);
    lbl_1_bss_0.roundIndicatorMotions[3] =
        READ_MOTION(lbl_1_bss_0.roundIndicatorModel, PIXEL_ROUND_RIGHT_RESULT_RESOURCE);
    setupObject->mdlId[0] = READ_MODEL(PIXEL_STAGE_RESOURCE);
    setupObject->mdlId[1] = collisionModel = READ_MODEL(PIXEL_COLLISION_RESOURCE);
    fn_800269E8(setupObject->mdlId[1], (1 << 0));
    lbl_1_bss_0.model = READ_MODEL(PIXEL_SCENE_RESOURCE);
    for (setupIndex = 0; setupIndex < 3; setupIndex++) {
        lbl_1_bss_0.motion[setupIndex] = READ_MOTION(lbl_1_bss_0.model, lbl_1_data_36C[setupIndex]);
    }
    fn_8002C95C(lbl_1_bss_0.model, lbl_1_bss_0.motion[0]);
    fn_800269E8(lbl_1_bss_0.model, ((1 << 30) | (1 << 0)));
    lbl_1_bss_0.referenceModel = READ_MODEL(PIXEL_REFERENCE_RESOURCE);
    {
    u32 referenceCellResources[16] = {
        PIXEL_REFERENCE_CELL_0_RESOURCE, PIXEL_REFERENCE_CELL_1_RESOURCE,
        PIXEL_REFERENCE_CELL_2_RESOURCE, PIXEL_REFERENCE_CELL_3_RESOURCE,
        PIXEL_REFERENCE_CELL_4_RESOURCE, PIXEL_REFERENCE_CELL_5_RESOURCE,
        PIXEL_REFERENCE_CELL_6_RESOURCE, PIXEL_REFERENCE_CELL_7_RESOURCE,
        PIXEL_REFERENCE_CELL_8_RESOURCE, PIXEL_REFERENCE_CELL_9_RESOURCE,
        PIXEL_REFERENCE_CELL_10_RESOURCE, PIXEL_REFERENCE_CELL_11_RESOURCE,
        PIXEL_REFERENCE_CELL_12_RESOURCE, PIXEL_REFERENCE_CELL_13_RESOURCE,
        PIXEL_REFERENCE_CELL_14_RESOURCE, PIXEL_REFERENCE_CELL_15_RESOURCE
    };
    for (setupIndex = 0; setupIndex < 16; setupIndex++) {
        lbl_1_bss_0.referenceCellModels[setupIndex] =
            READ_MODEL(referenceCellResources[setupIndex]);
        fn_800269E8(lbl_1_bss_0.referenceCellModels[setupIndex], (1 << 0));
    }
    lbl_1_bss_0.finishModel = READ_MODEL(PIXEL_FINISH_MODEL_RESOURCE);
    lbl_1_bss_0.finishMotions[0] =
        READ_MOTION(lbl_1_bss_0.finishModel, PIXEL_FINISH_INITIAL_MOTION_RESOURCE);
    lbl_1_bss_0.finishMotions[1] =
        READ_MOTION(lbl_1_bss_0.finishModel, PIXEL_FINISH_END_MOTION_RESOURCE);
    fn_800269E8(lbl_1_bss_0.finishModel, ((1 << 30) | (1 << 0)));
    fn_8002C95C(lbl_1_bss_0.finishModel, lbl_1_bss_0.finishMotions[0]);
    lbl_1_bss_0.extraSceneModel = READ_MODEL(PIXEL_EXTRA_SCENE_MODEL_RESOURCE);
    lbl_1_bss_0.extraSceneMotion =
        READ_MOTION(lbl_1_bss_0.extraSceneModel, PIXEL_EXTRA_SCENE_MOTION_RESOURCE);
    lbl_1_bss_0.panelBaseModel = READ_MODEL(PIXEL_PANEL_MODEL_RESOURCE);
    lbl_1_bss_0.panelMotions[0] = READ_MOTION(lbl_1_bss_0.panelBaseModel, PIXEL_PANEL_OFF_RESOURCE);
    lbl_1_bss_0.panelMotions[1] = READ_MOTION(lbl_1_bss_0.panelBaseModel, PIXEL_PANEL_ON_RESOURCE);
    lbl_1_bss_0.panelMotions[2] =
        READ_MOTION(lbl_1_bss_0.panelBaseModel, PIXEL_PANEL_TURN_ON_RESOURCE);
    lbl_1_bss_0.panelMotions[3] =
        READ_MOTION(lbl_1_bss_0.panelBaseModel, PIXEL_PANEL_TURN_OFF_RESOURCE);
    fn_80027080(lbl_1_bss_0.panelBaseModel, "636dot-StagePCPanel");
    boardY = 0.0f;
    /* The two 4-by-4 floors are separated along X and share linked panel geometry. */
    for (setupIndex = 0; setupIndex < 2; setupIndex++) {
        if (setupIndex == 0) {
            boardX = -850.0f;
            boardY = 0.02f;
            boardZ = -400.0f;
        } else {
            boardX = 50.0f;
            boardY = 0.02f;
            boardZ = -400.0f;
        }
        for (row = 0; row < 4; row++) {
            for (column = 0; column < 4; column++) {
                panelModel = fn_80025D4C(lbl_1_bss_0.panelBaseModel);
                fn_80026760(panelModel, boardX + column * 200, boardY, boardZ + row * 200);
                lbl_1_bss_0.panelModels[setupIndex][row * 4 + column] = panelModel;
            }
        }
    }
    for (setupIndex = 0; setupIndex < 3; setupIndex++) {
        lbl_1_bss_0.pictureIntroModels[setupIndex] = READ_MODEL(lbl_1_data_288[setupIndex]);
        lbl_1_bss_0.pictureResultModels[setupIndex] = READ_MODEL(lbl_1_data_294[setupIndex]);
        lbl_1_bss_0.pictureFinishModels[setupIndex] = READ_MODEL(lbl_1_data_2A0[setupIndex]);
        fn_800269E8(lbl_1_bss_0.pictureIntroModels[setupIndex], (1 << 0));
        fn_800269E8(lbl_1_bss_0.pictureResultModels[setupIndex], (1 << 0));
        fn_800269E8(lbl_1_bss_0.pictureFinishModels[setupIndex], (1 << 0));
        for (cell = 0; cell < 16; cell++) {
            lbl_1_bss_0.pictureCellModels[setupIndex][cell] =
                READ_MODEL(lbl_1_data_2AC[setupIndex][cell]);
            fn_800269E8(lbl_1_bss_0.pictureCellModels[setupIndex][cell], (1 << 0));
        }
    }
    fn_800804E8(&collisionModel, 1, 5);
    {
    s32 teamPlayerCounts[2] = {0, 0};
    actorParam.height = 150.0f;
    actorParam.radius = 50.0f;
    actorParam.param = 0;
    actorParam.type = 0;
    actorParam.attr = PIXEL_ACTOR_INITIAL_FLAGS;
    actorParam.narrowHook = NULL;
    actorParam.correctHook = fn_1_2F10;
    for (setupIndex = 0; setupIndex < 4; setupIndex++) {
        actorParam.correctHookParam = setupIndex;
        lbl_1_bss_0.charNo[setupIndex] = lbl_80265728[setupIndex].charNo;
        lbl_1_bss_0.padNo[setupIndex] = lbl_80265728[setupIndex].padNo;
        side = lbl_80265728[setupIndex].grpNo;
        lbl_1_bss_0.teamSide[setupIndex] = side;
        OSReport("player %d -> pad %d, group %d\n", setupIndex, lbl_1_bss_0.padNo[setupIndex],
                 lbl_1_bss_0.teamSide[setupIndex]);
        /* Player creation replaces the landing motion in the shared motion list. */
        lbl_1_bss_0.players[setupIndex] = fn_80080728(
            (s16) setupIndex, &actorParam, 8, (1 << 0), 0xFFFFFFF3, lbl_1_data_E0);
        fn_8007ED60(lbl_1_bss_0.players[setupIndex]);
        actorParam.param++;
        lbl_1_bss_0.players[setupIndex]->actor->pos.x =
            lbl_1_data_240[side * 2 + teamPlayerCounts[side]].x;
        lbl_1_bss_0.players[setupIndex]->actor->pos.y =
            lbl_1_data_240[side * 2 + teamPlayerCounts[side]].y;
        lbl_1_bss_0.players[setupIndex]->actor->pos.z =
            lbl_1_data_240[side * 2 + teamPlayerCounts[side]].z;
        teamPlayerCounts[side]++;
        /* The yaw setter receives the player base here rather than its actor. */
        fn_80086CA8((MGACTOR *)lbl_1_bss_0.players[setupIndex], 90.0f);
        fn_80026E44((s16)lbl_1_bss_0.players[setupIndex]->actor->mdlId);
        fn_80087158(lbl_1_bss_0.players[setupIndex], (1 << 0));
    }
    fn_1_31BC();
    /* Scene setup creates another timer and replaces the earlier stored timer pointer. */
    lbl_1_bss_0.timer = MgTimerCreate(0);
    fn_8002C2AC(8);
    fn_1_6790(0);
    lbl_1_bss_0.musicHandle = -1;
    setupObject->objFunc = NULL;
    }
    }
    }
}
