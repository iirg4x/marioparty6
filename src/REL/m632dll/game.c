/* Scene and collision setup for the M632 minigame. */
#define _MATH_H
#include "REL/m632dll.h"

static int lbl_1_data_78[5] = { DATANUM(DATA_m632, 0), DATANUM(DATA_m632, 2), DATANUM(DATA_m632, 2), DATANUM(DATA_m632, 3), DATANUM(DATA_m632, 4) };

static int lbl_1_data_8C[5] = { 0, 0, 1, 0, 1 };

static int lbl_1_data_A0[4] = { DATANUM(DATA_m632, 21), DATANUM(DATA_m632, 22), DATANUM(DATA_m632, 23), DATANUM(DATA_m632, 24) };

static int lbl_1_data_B0[3] = { DATANUM(DATA_m632, 5), DATANUM(DATA_m632, 7), DATANUM(DATA_m632, 8) };

static char lbl_1_data_BC[19] = "632hakoniwa-PC01st";

static char lbl_1_data_CF[19] = "632hakoniwa-PC02st";

static char lbl_1_data_E2[19] = "632hakoniwa-PC03st";

static char lbl_1_data_F5[19] = "632hakoniwa-PC04st";

char *lbl_1_data_108[4] = { lbl_1_data_BC, lbl_1_data_CF, lbl_1_data_E2, lbl_1_data_F5 };

unsigned int lbl_1_data_118[4] = { DATANUM(DATA_m632, 10), DATANUM(DATA_m632, 11), DATANUM(DATA_m632, 12), DATANUM(DATA_m632, 13) };

/* Initialized 48-byte table data; this module does not read it. */
u8 lbl_1_data_128[48] = {
    195, 200, 0, 0,
    68, 150, 0, 0,
    0, 0, 0, 0,
    67, 200, 0, 0,
    68, 150, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    68, 150, 0, 0,
    195, 200, 0, 0,
    0, 0, 0, 0,
    68, 150, 0, 0,
    67, 200, 0, 0,
};

unsigned int lbl_1_data_158[16] = {
    DATANUM(DATA_mariomot, 0),
    DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mariomot, 2),
    DATANUM(DATA_mariomot, 3),
    DATANUM(DATA_mariomot, 4),
    DATANUM(DATA_mariomot, 35),
    DATANUM(DATA_mariomot, 34),
    DATANUM(DATA_mariomot, 24),
    DATANUM(DATA_mariomot, 6),
    DATANUM(DATA_mariomot, 40),
    DATANUM(DATA_mariomot, 24),
    DATANUM(DATA_mariomot, 21),
    DATANUM(DATA_mariomot, 32),
    DATANUM(DATA_mario, 44),
    DATANUM(DATA_mario, 200),
    0,
};

s32 lbl_1_data_198[4] = { 128, 80, 40, 32 };

static u8 lbl_1_data_1A8[64] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    0,
    6,
    0,
    0,
    0,
    2,
    0,
    0,
    3,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    0,
    0,
    3,
    0,
    0,
    2,
    0,
    0,
    0,
    4,
    0,
    0,
    0,
    0,
    5,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

static u8 lbl_1_data_1E8[64] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    3,
    0,
    0,
    6,
    0,
    2,
    0,
    0,
    0,
    5,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    4,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    2,
    0,
    1,
    0,
    0,
    3,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

static u8 lbl_1_data_228[64] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    3,
    0,
    0,
    2,
    0,
    0,
    0,
    4,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    6,
    0,
    0,
    0,
    2,
    0,
    0,
    3,
    0,
    0,
    0,
    1,
    0,
    0,
    5,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

static u8 lbl_1_data_268[64] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    2,
    0,
    5,
    0,
    0,
    3,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    4,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    6,
    0,
    0,
    0,
    3,
    1,
    0,
    0,
    0,
    2,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

u8 *lbl_1_data_2A8[4] = { lbl_1_data_1A8, lbl_1_data_1E8, lbl_1_data_228, lbl_1_data_268 };

static char lbl_1_data_2B8[21] = "632hakoniwaB-BaHOOK1";

static char lbl_1_data_2CD[21] = "632hakoniwaB-BaHOOK2";

static char lbl_1_data_2E2[21] = "632hakoniwaB-BaHOOK3";

static char lbl_1_data_2F7[21] = "632hakoniwaB-BaHOOK4";

static char lbl_1_data_30C[21] = "632hakoniwaB-BaHOOK5";

static char lbl_1_data_321[21] = "632hakoniwaB-BaHOOK6";

static char lbl_1_data_336[21] = "632hakoniwaB-BaHOOK7";

static char lbl_1_data_34B[21] = "632hakoniwaB-BaHOOK8";

static char lbl_1_data_360[24] = "632hakoniwaB-BaCOLHOOK1";

static char lbl_1_data_378[24] = "632hakoniwaB-BaCOLHOOK2";

static char lbl_1_data_390[24] = "632hakoniwaB-BaCOLHOOK3";

static char lbl_1_data_3A8[24] = "632hakoniwaB-BaCOLHOOK4";

static char lbl_1_data_3C0[24] = "632hakoniwaB-BaCOLHOOK5";

static char lbl_1_data_3D8[24] = "632hakoniwaB-BaCOLHOOK6";

static char lbl_1_data_3F0[24] = "632hakoniwaB-BaCOLHOOK7";

static char lbl_1_data_408[24] = "632hakoniwaB-BaCOLHOOK8";

static char lbl_1_data_420[23] = "632hakoniwaA-stageAsha";

static char lbl_1_data_437[26] = "632hakoniwaA-stageAshaADD";

static char lbl_1_data_451[23] = "632hakoniwaA-stageCsha";

static char lbl_1_data_468[24] = "632hakoniwaB-stageBaSha";

static char lbl_1_data_480[21] = "632hakoniwaB-stageBb";

M632State lbl_1_bss_0;

/* Ground-correction hook installed by fn_1_3934; records a ground-contact flag for that actor. */
void fn_1_3510(MGACTOR *actorP, int param)
{
    if ((u32) (actorP->colGroundAttr & 128) != 0) {
        lbl_1_bss_0.collisionPairs[param][1] = 1;
    }
}

/* Narrow-phase hook installed by fn_1_3934; adjusts arena contact and the active round's player-launch response. */
int fn_1_353C(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b)
{
    MGACTOR_COLMAP_POLY localValue11;
    Point3d localValue10;
    Point3d localValue9;
    Point3d localValue8;
    Point3d localValue7;
    Point3d localValue6;
    Point3d localValue5;
    Point3d localValue4;
    Point3d localValue0;
    f32 temporaryFloat0;
    s32 temporary1;
    s32 pan;
    MGPLAYER *player;
    MGACTOR *actor;

    if ((a->type == 1) && (b->type == 1)) {
        PSVECSubtract(&b->point, &a->point, &localValue10);
        PSVECNormalize(&localValue10, &localValue10);
        PSVECNormalize(&a->normPos, &localValue9);
        if (PSVECDotProduct(&localValue10, &localValue9) > 0.1f) {
            temporaryFloat0 = a->normPos.y;
            PSVECScale(&a->normPos, &a->normPos, 0.01f);
            a->normPos.y = 0.05f * temporaryFloat0;
        }
        lbl_1_bss_0.collisionFlag = 1;
        goto block_20;
    }
    if ((a->type == 1) && (b->type == 0)) {
        if (MgSeqModeGet() != 5) {
            return 1;
        }
        actor = lbl_1_bss_0.collisionActors[a->paramB];
        player = lbl_1_bss_0.players[b->paramB];
        if ((lbl_1_bss_0.activeMask & (1 << b->paramB)) != 0) {
            localValue8 = actor->pos;
            localValue7 = player->actor->pos;
            localValue8.y = localValue7.y = 0.0f;
            PSVECSubtract(&localValue7, &localValue8, &localValue6);
            PSVECNormalize(&localValue6, &localValue6);
            localValue8.x = localValue7.x + (2000.0f * localValue6.x);
            localValue8.z = localValue7.z + (2000.0f * localValue6.z);
            localValue8.y = localValue7.y = 600.0f;
            if (MgActorColMapPolyGet(&localValue7, &localValue8, 1U, &localValue11) != 0) {
                localValue5 = localValue11.pos;
                if (((HSF_FACE *) localValue11.obj->mesh.face->data)[localValue11.triNo].nbt[2] < 0.0f) {
                    localValue5.y = 600.0f;
                } else {
                    localValue5.y = 1200.0f;
                }
            } else {
                localValue5.x = 0.0f;
                localValue5.y = 400.0f;
                localValue5.z = 400.0f;
            }
            localValue7.y = player->actor->pos.y;
            PSVECSubtract(&localValue5, &localValue7, &localValue6);
            PSVECNormalize(&localValue6, &localValue6);
            lbl_1_bss_0.playerMotionVec[b->paramB] = localValue6;
            lbl_1_bss_0.playerState[b->paramB] = 1;
            lbl_1_bss_0.activeMask &= ~(1 << b->paramB);
            temporary1 = HuAudFXPlay(1847);
            localValue4 = player->actor->pos;
            Hu3D3Dto2D(&localValue4, 1, &localValue0);
            pan = (s32) localValue0.x;
            pan /= 5;
            if (pan < 32) {
                pan = 32;
            } else if (pan > 96) {
                pan = 96;
            }
            HuAudFXPanning(temporary1, pan);
        }
        goto block_20;
    }
block_20:
    return 1;
}

/* Called by _prolog during module load to create the arena, players, collision actors, lighting, and sequence state. */
void fn_1_3934(void)
{
    Point3d localValue8;
    Point3d localValue7;
    Point3d localValue6;
    Point3d localValue5;
    Point3d localValue4;
    int index;
    int counter;
    s8 *value4;
    s8 *value3;
    s16 temporary0;
    int mapCount = 0;
    s8 *value2;
    int value1;
    s16 temporary2;
    int value0;
    s16 jointMotion;
    int x1, z1;
    int x2, z2;
    int localValue3, localValue2;
    s16 localValue1;
    s16 localValue0;
    f32 temporaryFloat0;
    f32 floatValue0;

    memset(&lbl_1_bss_0, 0, sizeof(lbl_1_bss_0));
    lbl_1_bss_0.cpuStickX = 0.0f;
    lbl_1_bss_0.cpuStickZ = 0.0f;
    lbl_1_bss_0.objectManager = MgActorObjectSetup();
    lbl_1_bss_0.patternIndex = frand() & 3;
    lbl_1_bss_0.timer = MgTimerCreate(0);
    CRot.x = -35.0f;
    CRot.y = 0.0f;
    CRot.z = 0.0f;
    Center.x = 0.0f;
    Center.y = 700.0f;
    Center.z = 400.0f;
    CZoom = 1000.0f;
    Hu3DCameraCreate(3);
    Hu3DCameraPerspectiveSet(3, 45.0f, 20.0f, 8000.0f, 1.2f);
    Hu3DCameraViewportSet(3, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(2, 640U, 480U, 0U, 0U);
    index = 0;
    while (index < 5) {
        lbl_1_bss_0.cameraMotions[index] = Hu3DMotionCreate(HuDataSelHeapReadNum(lbl_1_data_78[index], HU_MEMNUM_OVL, HEAP_MODEL));
        if (lbl_1_data_8C[index] == 0) {
            lbl_1_bss_0.cameraModels[index] = Hu3DModelCameraCreate(lbl_1_bss_0.cameraMotions[index], 1U);
        } else {
            lbl_1_bss_0.cameraModels[index] = Hu3DModelCameraCreate(lbl_1_bss_0.cameraMotions[index], 2U);
        }
        Hu3DCameraMotionOff(lbl_1_bss_0.cameraModels[index]);
        index += 1;
    }
    index = 0;
    while (index < 2) {
        lbl_1_bss_0.motions020[index] = -1;
        index += 1;
    }
    Hu3DCameraMotionStart(lbl_1_bss_0.cameraModels[0], 1U);
    localValue8.x = 0.0f;
    localValue8.y = 4000.0f;
    localValue8.z = 0.0f;
    localValue7.x = 0.0f;
    localValue7.y = 0.0f;
    localValue7.z = 1.0f;
    localValue6.x = 0.0f;
    localValue6.y = -10.0f;
    localValue6.z = 0.0f;
    Hu3DShadowMultiCreate(30.0f, 1.0f, 13000.0f, 3);
    Hu3DShadowMultiTPLvlSet(0.7f, 3);
    localValue8.y = -1800.0f;
    localValue6.y = 100.0f;
    Hu3DShadowMultiPosSet(&localValue8, &localValue7, &localValue6, 1);
    Hu3DShadowMultiColSet(25U, 25U, 25U, 1);
    Hu3DShadowMultiSizeSet(240U, 1);
    localValue8.y = -2000.0f;
    localValue6.y = 100.0f;
    Hu3DShadowMultiPosSet(&localValue8, &localValue7, &localValue6, 2);
    Hu3DShadowMultiColSet(50U, 50U, 50U, 2);
    Hu3DShadowMultiSizeSet(192U, 2);
    index = 0;
    while (index < 4) {
        lbl_1_bss_0.playerModels[index] = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_A0[index], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_0.playerModels[index], 1U);
        index += 1;
    }
    lbl_1_bss_0.modelId = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m632, 20), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_0.modelId, HU3D_ATTR_DISPOFF);
    index = 0;
    while (index < 3) {
        lbl_1_bss_0.sceneModels[index] = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_B0[index], HU_MEMNUM_OVL, HEAP_MODEL));
        index += 1;
    }
    jointMotion = Hu3DJointMotion(lbl_1_bss_0.sceneModels[0], HuDataSelHeapReadNum(DATANUM(DATA_m632, 6), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(lbl_1_bss_0.sceneModels[0], jointMotion);
    (&lbl_1_bss_0.collisionModel)[mapCount] = Hu3DModelCreate(HuDataSelHeapReadNum((int) lbl_1_data_118[lbl_1_bss_0.patternIndex], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet((&lbl_1_bss_0.collisionModel)[mapCount], HU3D_ATTR_DISPOFF);
    mapCount += 1;
    Hu3DModelCameraSet(lbl_1_bss_0.sceneModels[0], 1U);
    Hu3DModelCameraSet(lbl_1_bss_0.sceneModels[1], 2U);
    Hu3DModelCameraSet(lbl_1_bss_0.sceneModels[2], 2U);
    Hu3DModelLightInfoSet(lbl_1_bss_0.sceneModels[0], 1);
    Hu3DModelLightInfoSet(lbl_1_bss_0.sceneModels[1], 1);
    Hu3DModelAttrSet(lbl_1_bss_0.sceneModels[1], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_0.sceneModels[2], HU3D_ATTR_DISPOFF);
    Hu3DModelShadowMapObjSet(lbl_1_bss_0.sceneModels[0], lbl_1_data_420);
    Hu3DModelShadowMapObjSet(lbl_1_bss_0.sceneModels[0], lbl_1_data_437);
    Hu3DModelShadowMapObjSet(lbl_1_bss_0.sceneModels[0], lbl_1_data_451);
    Hu3DModelShadowMapObjSet(lbl_1_bss_0.sceneModels[1], lbl_1_data_468);
    lbl_1_bss_0.attachmentModels[0] = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m632, 14), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_0.attachmentModels[1] = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m632, 16), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_0.attachmentModels[2] = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m632, 17), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_0.attachmentModels[0], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_0.attachmentModels[1], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_0.attachmentModels[2], HU3D_ATTR_DISPOFF);
    lbl_1_bss_0.hookModelId = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m632, 19), HU_MEMNUM_OVL, HEAP_MODEL));
    value4 = (s8 *) lbl_1_data_2A8[lbl_1_bss_0.patternIndex];
    {
    char *localValue14[8] = { lbl_1_data_2B8,lbl_1_data_2CD,lbl_1_data_2E2,lbl_1_data_2F7,lbl_1_data_30C,lbl_1_data_321,lbl_1_data_336,lbl_1_data_34B };
    char *localValue13[8] = { lbl_1_data_360,lbl_1_data_378,lbl_1_data_390,lbl_1_data_3A8,lbl_1_data_3C0,lbl_1_data_3D8,lbl_1_data_3F0,lbl_1_data_408 };
    MGACTOR_PARAM localValue12;
    MGACTOR_PARAM localValue9;
    lbl_1_bss_0.linkedModelCount = 0;
    counter = 1;
    index = 0;
    while (index < 64) {
        if ((value4[0] >= 2) && (value4[0] <= 3)) {
            x1 = index % 8;
            z1 = index / 8;
            switch (value4[0]) {
            case 2:
                floatValue0 = -45.0f;
                break;
            case 3:
                floatValue0 = 45.0f;
                break;
            }
            temporary0 = Hu3DModelLink(lbl_1_bss_0.attachmentModels[0]);
            Hu3DModelCameraSet(temporary0, 2U);
            Hu3DModelAttrReset(temporary0, HU3D_ATTR_DISPOFF);
            Hu3DModelShadowMapObjSet(temporary0, lbl_1_data_480);
            Hu3DModelHookSet(lbl_1_bss_0.sceneModels[1], localValue14[counter], temporary0);
            Hu3DModelPosSet(temporary0, (f32) ((x1 * 100) - 350), 0.0f, (f32) ((z1 * 100) - 350));
            Hu3DModelRotSet(temporary0, 0.0f, floatValue0, 0.0f);
            lbl_1_bss_0.linkedModels[lbl_1_bss_0.linkedModelCount] = temporary0;
            counter += 1;
            lbl_1_bss_0.linkedModelCount += 1;
        }
        value4 += 1;
        index += 1;
    }
    MgActorColMapInit(&lbl_1_bss_0.collisionModel, mapCount, 30);
    value2 = (s8 *) lbl_1_data_2A8[lbl_1_bss_0.patternIndex];
    lbl_1_bss_0.collisionCount = 0;
    localValue12.height = 50.0f;
    localValue12.radius = 55.0f;
    localValue12.param = 0;
    localValue12.type = 1;
    localValue12.attr = 0;
    localValue12.narrowHook = fn_1_353C;
    localValue12.correctHook = fn_1_3510;
    counter = 0;
    index = 0;
    while (index < 64) {
        if (value2[0] == 1) {
            x2 = index % 8;
            z2 = index / 8;
            temporary2 = Hu3DModelLink(lbl_1_bss_0.attachmentModels[2]);
            Hu3DModelCameraSet(temporary2, 2U);
            Hu3DModelAttrReset(temporary2, HU3D_ATTR_DISPOFF);
            localValue12.correctHookParam = counter;
            lbl_1_bss_0.collisionActors[counter] = MgActorCreate(&localValue12, temporary2);
            MgActorColBounceSet(lbl_1_bss_0.collisionActors[counter], 0.0f);
            localValue5.x = (f32) ((x2 * 100) - 350);
            localValue5.y = 0.0f;
            localValue5.z = (f32) ((z2 * 100) - 350);
            MgActorPosSet(lbl_1_bss_0.collisionActors[counter], &localValue5);
            MgActorColAttrSet(lbl_1_bss_0.collisionActors[counter], 96U);
            MgActorGravitySet(lbl_1_bss_0.collisionActors[counter], 700.0f);
            Hu3DModelShadowSet(lbl_1_bss_0.collisionActors[counter]->mdlId);
            temporaryFloat0 = (f32) (u32) frandmod(360);
            lbl_1_bss_0.collisionRotY[counter] = temporaryFloat0;
            lbl_1_bss_0.collisionActors[counter]->rotY = temporaryFloat0;
            MgActorRotYSet(lbl_1_bss_0.collisionActors[counter], temporaryFloat0);
            localValue1 = Hu3DLLightCreate(lbl_1_bss_0.collisionActors[counter]->mdlId, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 160U, 160U, 160U);
            Hu3DLLightInfinitytSet(lbl_1_bss_0.collisionActors[counter]->mdlId, localValue1);
            localValue12.param += 1;
            counter += 1;
            lbl_1_bss_0.collisionCount += 1;
        }
        value2 += 1;
        index += 1;
    }
    localValue9.height = 150.0f;
    localValue9.radius = 55.0f;
    localValue9.param = 0;
    localValue9.type = 0;
    localValue9.attr = 0;
    localValue9.narrowHook = 0;
    localValue9.correctHook = 0;
    index = 0;
    while (index < 4) {
        lbl_1_bss_0.charNo[index] = GwPlayerConf[index].charNo;
        lbl_1_bss_0.padNo[index] = GwPlayerConf[index].padNo;
        if (GwPlayerConf[index].grpNo == 0) {
            value0 = 1;
        } else {
            value0 = 2;
        }
        lbl_1_bss_0.players[index] = MgPlayerCreate(index, &localValue9, 2, (u16) value0, -31U, lbl_1_data_158);
        if (GwPlayerConf[index].grpNo == 0) {
            lbl_1_bss_0.group[index] = 0;
        } else {
            lbl_1_bss_0.group[index] = 1;
        }
        localValue4.x = 0.0f;
        localValue4.y = -1000.0f;
        localValue4.z = 0.0f;
        MgActorPosSet((MGACTOR *) lbl_1_bss_0.players[index], &localValue4);
        MgActorPosSetRaw((MGACTOR *) lbl_1_bss_0.players[index], &localValue4);
        Hu3DModelPosSetV(lbl_1_bss_0.players[index]->actor->mdlId, &localValue4);
        localValue0 = Hu3DLLightCreate(lbl_1_bss_0.players[index]->actor->mdlId, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 160U, 160U, 160U);
        Hu3DLLightInfinitytSet(lbl_1_bss_0.players[index]->actor->mdlId, localValue0);
        Hu3DModelShadowSet(lbl_1_bss_0.players[index]->actor->mdlId);
        MgPlayerVibrateCreate(lbl_1_bss_0.players[index]);
        MgPlayerDespawn(lbl_1_bss_0.players[index]);
        localValue9.param += 1;
        index += 1;
    }
    value3 = (s8 *) lbl_1_data_2A8[lbl_1_bss_0.patternIndex];
    index = 0;
    while (index < 64) {
        if ((value3[0] >= 4) && (value3[0] <= 6)) {
            counter = value3[0] - 4;
            localValue3 = index % 8;
            localValue2 = index / 8;
            lbl_1_bss_0.positions[counter].x = (f32) ((localValue3 * 100) - 350);
            lbl_1_bss_0.positions[counter].y = 1500.0f;
            lbl_1_bss_0.positions[counter].z = (f32) ((localValue2 * 100) - 350);
        }
        value3 += 1;
        index += 1;
    }
    lbl_1_bss_0.playerState[0] = 0;
    lbl_1_bss_0.playerState[1] = 0;
    lbl_1_bss_0.playerState[2] = 0;
    lbl_1_bss_0.playerState[3] = 0;
    lbl_1_bss_0.previousCollisionFlag = 0;
    lbl_1_bss_0.collisionFlag = 0;
    lbl_1_bss_0.soundCooldownFrames = 0;
    index = 0;
    while (index < lbl_1_bss_0.collisionCount) {
        lbl_1_bss_0.collisionPairs[index][0] = 0;
        lbl_1_bss_0.collisionPairs[index][1] = 0;
        lbl_1_bss_0.collisionTimers[index] = 0;
        index += 1;
    }
    fn_1_4C90();
    lbl_1_bss_0.stream = -1;
    value1 = 0;
    while (value1 < 4) {
        CharFXPlay(lbl_1_bss_0.charNo[value1], 581);
        value1 += 1;
    }
    MgSeqCreate(&lbl_1_data_0);
    }
}
