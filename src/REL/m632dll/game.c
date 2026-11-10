/* Scene and collision setup for the M632 minigame. */
#include "REL/m632dll.h"

static int lbl_1_data_78[5] = { DATANUM(0x5b0000, 0), DATANUM(0x5b0000, 2), DATANUM(0x5b0000, 2),
                                DATANUM(0x5b0000, 3), DATANUM(0x5b0000, 4) };

static int lbl_1_data_8C[5] = { 0, 0, 1, 0, 1 };

static int lbl_1_data_A0[4] = { DATANUM(0x5b0000, 21), DATANUM(0x5b0000, 22),
                                DATANUM(0x5b0000, 23), DATANUM(0x5b0000, 24) };

static int lbl_1_data_B0[3] = { DATANUM(0x5b0000, 5), DATANUM(0x5b0000, 7),
                                DATANUM(0x5b0000, 8) };

static char lbl_1_data_BC[19] = "632hakoniwa-PC01st";

static char lbl_1_data_CF[19] = "632hakoniwa-PC02st";

static char lbl_1_data_E2[19] = "632hakoniwa-PC03st";

static char lbl_1_data_F5[19] = "632hakoniwa-PC04st";

char *lbl_1_data_108[4] = { lbl_1_data_BC, lbl_1_data_CF, lbl_1_data_E2, lbl_1_data_F5 };

unsigned int lbl_1_data_118[4] = { DATANUM(0x5b0000, 10), DATANUM(0x5b0000, 11),
                                   DATANUM(0x5b0000, 12), DATANUM(0x5b0000, 13) };

/* Four 3D points at Y=1200. */
Point3d lbl_1_data_128[4] = {
    { -400.0f, 1200.0f, 0.0f },
    { 400.0f, 1200.0f, 0.0f },
    { 0.0f, 1200.0f, -400.0f },
    { 0.0f, 1200.0f, 400.0f },
};

unsigned int lbl_1_data_158[16] = {
    DATANUM(0x930000, 0),
    DATANUM(0x930000, 1),
    DATANUM(0x930000, 2),
    DATANUM(0x930000, 3),
    DATANUM(0x930000, 4),
    DATANUM(0x930000, 35),
    DATANUM(0x930000, 34),
    DATANUM(0x930000, 24),
    DATANUM(0x930000, 6),
    DATANUM(0x930000, 40),
    DATANUM(0x930000, 24),
    DATANUM(0x930000, 21),
    DATANUM(0x930000, 32),
    DATANUM(0x8e0000, 44),
    DATANUM(0x8e0000, 200),
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

/* Ground-correction hook installed by ObjectSetup; records a ground-contact flag for that actor. */
void fn_1_3510(MGACTOR *actorP, int param)
{
    if ((u32) (actorP->colGroundAttr & 128) != 0) {
        lbl_1_bss_0.collisionPairs[param][1] = 1;
    }
}

/* Narrow-phase hook installed by ObjectSetup; adjusts arena contact and the active round's
 * player-launch response. */
int fn_1_353C(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b)
{
    MGACTOR_COLMAP_POLY collisionPolygon;
    Point3d contactSeparation;
    Point3d contactNormal;
    Point3d rayTo;
    Point3d rayFrom;
    Point3d motionDirection;
    Point3d launchTarget;
    Point3d playerPosition;
    Point3d screenPosition;
    f32 originalNormalY;
    s32 soundHandle;
    s32 pan;
    MGPLAYER *player;
    MGACTOR *actor;

    if ((a->type == 1) && (b->type == 1)) {
        PSVECSubtract(&b->point, &a->point, &contactSeparation);
        PSVECNormalize(&contactSeparation, &contactSeparation);
        PSVECNormalize(&a->normPos, &contactNormal);
        if (PSVECDotProduct(&contactSeparation, &contactNormal) > 0.1f) {
            originalNormalY = a->normPos.y;
            PSVECScale(&a->normPos, &a->normPos, 0.01f);
            a->normPos.y = 0.05f * originalNormalY;
        }
        lbl_1_bss_0.collisionFlag = 1;
    } else if ((a->type == 1) && (b->type == 0)) {
        if (fn_80071A8C() != 5) {
            return 1;
        }
        actor = lbl_1_bss_0.collisionActors[a->paramB];
        player = lbl_1_bss_0.players[b->paramB];
        if ((lbl_1_bss_0.activeMask & (1 << b->paramB)) != 0) {
            rayTo = actor->pos;
            rayFrom = player->actor->pos;
            rayTo.y = rayFrom.y = 0.0f;
            PSVECSubtract(&rayFrom, &rayTo, &motionDirection);
            PSVECNormalize(&motionDirection, &motionDirection);
            rayTo.x = rayFrom.x + (2000.0f * motionDirection.x);
            rayTo.z = rayFrom.z + (2000.0f * motionDirection.z);
            rayTo.y = rayFrom.y = 600.0f;
            if (fn_80086910(&rayFrom, &rayTo, 1U, &collisionPolygon) != 0) {
                launchTarget = collisionPolygon.pos;
                if (collisionPolygon.obj->mesh.face->faces[collisionPolygon.triNo].nbt[2] < 0.0f) {
                    launchTarget.y = 600.0f;
                } else {
                    launchTarget.y = 1200.0f;
                }
            } else {
                launchTarget.x = 0.0f;
                launchTarget.y = 400.0f;
                launchTarget.z = 400.0f;
            }
            rayFrom.y = player->actor->pos.y;
            PSVECSubtract(&launchTarget, &rayFrom, &motionDirection);
            PSVECNormalize(&motionDirection, &motionDirection);
            lbl_1_bss_0.playerMotionVec[b->paramB] = motionDirection;
            lbl_1_bss_0.playerState[b->paramB] = 1;
            lbl_1_bss_0.activeMask &= ~(1 << b->paramB);
            soundHandle = HuAudFXPlay(MSM_SE_MG632_PLAYER_ELIMINATION);
            playerPosition = player->actor->pos;
            Hu3D3Dto2D(&playerPosition, 1, &screenPosition);
            pan = (s32) screenPosition.x;
            pan /= 5;
            if (pan < 32) {
                pan = 32;
            } else if (pan > 96) {
                pan = 96;
            }
            HuAudFXPanning(soundHandle, pan);
        }
    }
    return 1;
}

/* Called by _prolog during module load to create the arena, players, collision actors, lighting,
 * and sequence state. */
void ObjectSetup(void)
{
    Point3d shadowPosition;
    Point3d shadowDirection;
    Point3d shadowTarget;
    Point3d collisionPosition;
    Point3d playerPosition;
    int index;
    int counter;
    s8 *patternData;
    s8 *positionPatternData;
    s16 linkedModel;
    int mapCount = 0;
    s8 *collisionPatternData;
    int playerIndex;
    s16 collisionModel;
    int playerGroup;
    s16 jointMotion;
    int x1, z1;
    int x2, z2;
    int gridX, gridZ;
    s16 collisionLightId;
    s16 playerLightId;
    f32 collisionRotation;
    f32 floatValue0;

    memset(&lbl_1_bss_0, 0, sizeof(lbl_1_bss_0));
    lbl_1_bss_0.cpuStickX = 0.0f;
    lbl_1_bss_0.cpuStickZ = 0.0f;
    lbl_1_bss_0.objectManager = fn_80080418();
    lbl_1_bss_0.patternIndex = frand() & 3;
    lbl_1_bss_0.timer = MgTimerCreate(0);
    lbl_80266A40.x = -35.0f;
    lbl_80266A40.y = 0.0f;
    lbl_80266A40.z = 0.0f;
    lbl_80266A4C.x = 0.0f;
    lbl_80266A4C.y = 700.0f;
    lbl_80266A4C.z = 400.0f;
    lbl_802C028C = 1000.0f;
    Hu3DCameraCreate(3);
    Hu3DCameraPerspectiveSet(3, 45.0f, 20.0f, 8000.0f, 1.2f);
    Hu3DCameraViewportSet(3, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(2, 640U, 480U, 0U, 0U);
    index = 0;
    while (index < 5) {
        lbl_1_bss_0.cameraMotions[index] =
            fn_8002C4C0(fn_80008DD0(lbl_1_data_78[index], 0x10000000, 2));
        if (lbl_1_data_8C[index] == 0) {
            lbl_1_bss_0.cameraModels[index] =
                fn_800283F4(lbl_1_bss_0.cameraMotions[index], 1U);
        } else {
            lbl_1_bss_0.cameraModels[index] =
                fn_800283F4(lbl_1_bss_0.cameraMotions[index], 2U);
        }
        fn_8002858C(lbl_1_bss_0.cameraModels[index]);
        index += 1;
    }
    index = 0;
    while (index < 2) {
        lbl_1_bss_0.motions020[index] = -1;
        index += 1;
    }
    fn_800284C8(lbl_1_bss_0.cameraModels[0], 1U);
    shadowPosition.x = 0.0f;
    shadowPosition.y = 4000.0f;
    shadowPosition.z = 0.0f;
    shadowDirection.x = 0.0f;
    shadowDirection.y = 0.0f;
    shadowDirection.z = 1.0f;
    shadowTarget.x = 0.0f;
    shadowTarget.y = -10.0f;
    shadowTarget.z = 0.0f;
    fn_8002A3B0(30.0f, 1.0f, 13000.0f, 3);
    fn_8002A7D8(0.7f, 3);
    shadowPosition.y = -1800.0f;
    shadowTarget.y = 100.0f;
    fn_8002A674(&shadowPosition, &shadowDirection, &shadowTarget, 1);
    fn_8002AA3C(25U, 25U, 25U, 1);
    fn_8002A8CC(240U, 1);
    shadowPosition.y = -2000.0f;
    shadowTarget.y = 100.0f;
    fn_8002A674(&shadowPosition, &shadowDirection, &shadowTarget, 2);
    fn_8002AA3C(50U, 50U, 50U, 2);
    fn_8002A8CC(192U, 2);
    index = 0;
    while (index < 4) {
        lbl_1_bss_0.playerModels[index] =
            fn_80025A04(fn_80008DD0(lbl_1_data_A0[index], 0x10000000, 2));
        fn_80026B64(lbl_1_bss_0.playerModels[index], 1U);
        index += 1;
    }
    lbl_1_bss_0.modelId =
        fn_80025A04(fn_80008DD0(DATANUM(0x5b0000, 20), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_0.modelId, (1 << 0));
    index = 0;
    while (index < 3) {
        lbl_1_bss_0.sceneModels[index] =
            fn_80025A04(fn_80008DD0(lbl_1_data_B0[index], 0x10000000, 2));
        index += 1;
    }
    jointMotion =
        fn_80030458(lbl_1_bss_0.sceneModels[0],
                        fn_80008DD0(DATANUM(0x5b0000, 6), 0x10000000, 2));
    fn_8002C95C(lbl_1_bss_0.sceneModels[0], jointMotion);
    (&lbl_1_bss_0.collisionModel)[mapCount] = fn_80025A04(fn_80008DD0(
        (int) lbl_1_data_118[lbl_1_bss_0.patternIndex], 0x10000000, 2));
    fn_800269E8((&lbl_1_bss_0.collisionModel)[mapCount], (1 << 0));
    mapCount += 1;
    fn_80026B64(lbl_1_bss_0.sceneModels[0], 1U);
    fn_80026B64(lbl_1_bss_0.sceneModels[1], 2U);
    fn_80026B64(lbl_1_bss_0.sceneModels[2], 2U);
    fn_80029850(lbl_1_bss_0.sceneModels[0], 1);
    fn_80029850(lbl_1_bss_0.sceneModels[1], 1);
    fn_800269E8(lbl_1_bss_0.sceneModels[1], (1 << 0));
    fn_800269E8(lbl_1_bss_0.sceneModels[2], (1 << 0));
    fn_80027080(lbl_1_bss_0.sceneModels[0], lbl_1_data_420);
    fn_80027080(lbl_1_bss_0.sceneModels[0], lbl_1_data_437);
    fn_80027080(lbl_1_bss_0.sceneModels[0], lbl_1_data_451);
    fn_80027080(lbl_1_bss_0.sceneModels[1], lbl_1_data_468);
    lbl_1_bss_0.attachmentModels[0] =
        fn_80025A04(fn_80008DD0(DATANUM(0x5b0000, 14), 0x10000000, 2));
    lbl_1_bss_0.attachmentModels[1] =
        fn_80025A04(fn_80008DD0(DATANUM(0x5b0000, 16), 0x10000000, 2));
    lbl_1_bss_0.attachmentModels[2] =
        fn_80025A04(fn_80008DD0(DATANUM(0x5b0000, 17), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_0.attachmentModels[0], (1 << 0));
    fn_800269E8(lbl_1_bss_0.attachmentModels[1], (1 << 0));
    fn_800269E8(lbl_1_bss_0.attachmentModels[2], (1 << 0));
    lbl_1_bss_0.hookModelId =
        fn_80025A04(fn_80008DD0(DATANUM(0x5b0000, 19), 0x10000000, 2));
    patternData = (s8 *) lbl_1_data_2A8[lbl_1_bss_0.patternIndex];
    {
        char *collisionHookNames[8] = { lbl_1_data_2B8, lbl_1_data_2CD, lbl_1_data_2E2,
                                        lbl_1_data_2F7, lbl_1_data_30C, lbl_1_data_321,
                                        lbl_1_data_336, lbl_1_data_34B };
        char *playerHookNames[8] = {
            lbl_1_data_360, lbl_1_data_378, lbl_1_data_390, lbl_1_data_3A8,
            lbl_1_data_3C0, lbl_1_data_3D8, lbl_1_data_3F0, lbl_1_data_408
        };
        MGACTOR_PARAM collisionActorParam;
        MGACTOR_PARAM playerActorParam;
        lbl_1_bss_0.linkedModelCount = 0;
        counter = 1;
        index = 0;
        while (index < 64) {
            if ((patternData[0] >= 2) && (patternData[0] <= 3)) {
                x1 = index % 8;
                z1 = index / 8;
                switch (patternData[0]) {
                case 2:
                    floatValue0 = -45.0f;
                    break;
                case 3:
                    floatValue0 = 45.0f;
                    break;
                }
                linkedModel = fn_80025D4C(lbl_1_bss_0.attachmentModels[0]);
                fn_80026B64(linkedModel, 2U);
                fn_80026A44(linkedModel, (1 << 0));
                fn_80027080(linkedModel, lbl_1_data_480);
                fn_8002741C(lbl_1_bss_0.sceneModels[1], collisionHookNames[counter],
                                 linkedModel);
                fn_80026760(linkedModel, (f32) ((x1 * 100) - 350), 0.0f,
                                (f32) ((z1 * 100) - 350));
                fn_80026808(linkedModel, 0.0f, floatValue0, 0.0f);
                lbl_1_bss_0.linkedModels[lbl_1_bss_0.linkedModelCount] = linkedModel;
                counter += 1;
                lbl_1_bss_0.linkedModelCount += 1;
            }
            patternData += 1;
            index += 1;
    }
    fn_800804E8(&lbl_1_bss_0.collisionModel, mapCount, 30);
    collisionPatternData = (s8 *) lbl_1_data_2A8[lbl_1_bss_0.patternIndex];
    lbl_1_bss_0.collisionCount = 0;
    collisionActorParam.height = 50.0f;
    collisionActorParam.radius = 55.0f;
    collisionActorParam.param = 0;
    collisionActorParam.type = 1;
    collisionActorParam.attr = 0;
    collisionActorParam.narrowHook = fn_1_353C;
    collisionActorParam.correctHook = fn_1_3510;
    counter = 0;
    index = 0;
    while (index < 64) {
        if (collisionPatternData[0] == 1) {
            x2 = index % 8;
            z2 = index / 8;
            collisionModel = fn_80025D4C(lbl_1_bss_0.attachmentModels[2]);
            fn_80026B64(collisionModel, 2U);
            fn_80026A44(collisionModel, (1 << 0));
            collisionActorParam.correctHookParam = counter;
            lbl_1_bss_0.collisionActors[counter] =
                fn_80080B04(&collisionActorParam, collisionModel);
            fn_80080EEC(lbl_1_bss_0.collisionActors[counter], 0.0f);
            collisionPosition.x = (f32) ((x2 * 100) - 350);
            collisionPosition.y = 0.0f;
            collisionPosition.z = (f32) ((z2 * 100) - 350);
            fn_80086AA8(lbl_1_bss_0.collisionActors[counter], &collisionPosition);
            fn_800805CC(lbl_1_bss_0.collisionActors[counter], 96U);
            fn_80086CBC(lbl_1_bss_0.collisionActors[counter], 700.0f);
            fn_80026E44(lbl_1_bss_0.collisionActors[counter]->mdlId);
            collisionRotation = (f32) (u32) frandmod(360);
            lbl_1_bss_0.collisionRotY[counter] = collisionRotation;
            lbl_1_bss_0.collisionActors[counter]->rotY = collisionRotation;
            fn_80086CA8(lbl_1_bss_0.collisionActors[counter], collisionRotation);
            collisionLightId = Hu3DLLightCreate(lbl_1_bss_0.collisionActors[counter]->mdlId, 0.0f,
                                                0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 160U, 160U, 160U);
            fn_80028B58(lbl_1_bss_0.collisionActors[counter]->mdlId, collisionLightId);
            collisionActorParam.param += 1;
            counter += 1;
            lbl_1_bss_0.collisionCount += 1;
        }
        collisionPatternData += 1;
        index += 1;
    }
    playerActorParam.height = 150.0f;
    playerActorParam.radius = 55.0f;
    playerActorParam.param = 0;
    playerActorParam.type = 0;
    playerActorParam.attr = 0;
    playerActorParam.narrowHook = 0;
    playerActorParam.correctHook = 0;
    index = 0;
    while (index < 4) {
        lbl_1_bss_0.charNo[index] = lbl_80265728[index].charNo;
        lbl_1_bss_0.padNo[index] = lbl_80265728[index].padNo;
        if (lbl_80265728[index].grpNo == 0) {
            playerGroup = 1;
        } else {
            playerGroup = 2;
        }
        lbl_1_bss_0.players[index] =
            fn_80080728(index, &playerActorParam, 2, (u16) playerGroup, -31U, lbl_1_data_158);
        if (lbl_80265728[index].grpNo == 0) {
            lbl_1_bss_0.group[index] = 0;
        } else {
            lbl_1_bss_0.group[index] = 1;
        }
        playerPosition.x = 0.0f;
        playerPosition.y = -1000.0f;
        playerPosition.z = 0.0f;
        fn_80086AA8((MGACTOR *) lbl_1_bss_0.players[index], &playerPosition);
        fn_80086B04((MGACTOR *) lbl_1_bss_0.players[index], &playerPosition);
        fn_80026790(lbl_1_bss_0.players[index]->actor->mdlId, &playerPosition);
        playerLightId = Hu3DLLightCreate(lbl_1_bss_0.players[index]->actor->mdlId, 0.0f, 0.0f, 0.0f,
                                         0.0f, 0.0f, -1.0f, 160U, 160U, 160U);
        fn_80028B58(lbl_1_bss_0.players[index]->actor->mdlId, playerLightId);
        fn_80026E44(lbl_1_bss_0.players[index]->actor->mdlId);
        fn_8007ED60(lbl_1_bss_0.players[index]);
        fn_80086E3C(lbl_1_bss_0.players[index]);
        playerActorParam.param += 1;
        index += 1;
    }
    positionPatternData = (s8 *) lbl_1_data_2A8[lbl_1_bss_0.patternIndex];
    index = 0;
    while (index < 64) {
        if ((positionPatternData[0] >= 4) && (positionPatternData[0] <= 6)) {
            counter = positionPatternData[0] - 4;
            gridX = index % 8;
            gridZ = index / 8;
            lbl_1_bss_0.positions[counter].x = (f32) ((gridX * 100) - 350);
            lbl_1_bss_0.positions[counter].y = 1500.0f;
            lbl_1_bss_0.positions[counter].z = (f32) ((gridZ * 100) - 350);
        }
        positionPatternData += 1;
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
    playerIndex = 0;
    while (playerIndex < 4) {
        CharFXPlay(lbl_1_bss_0.charNo[playerIndex], MSM_SE_CHARVOICE_EXCITED_SHOUT);
        playerIndex += 1;
    }
    fn_80070788(&lbl_1_data_0);
    }
}
