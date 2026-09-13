#include "REL/m670dll.h"
#include "game/main.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/frand.h"
#include "game/mic.h"
#include "datadir_enum.h"

extern MGSEQ_PARAM lbl_1_data_0;

M670WORK lbl_1_bss_10;

int lbl_1_data_78[6] = {
    DATANUM(DATA_m670, 10), DATANUM(DATA_m670, 15),
    DATANUM(DATA_m670, 20), DATANUM(DATA_m670, 25),
    DATANUM(DATA_m670, 30), DATANUM(DATA_m670, 35)
};
int lbl_1_data_90[6][4] = {
    {DATANUM(DATA_m670, 11), DATANUM(DATA_m670, 12), DATANUM(DATA_m670, 13), DATANUM(DATA_m670, 14)},
    {DATANUM(DATA_m670, 16), DATANUM(DATA_m670, 17), DATANUM(DATA_m670, 18), DATANUM(DATA_m670, 19)},
    {DATANUM(DATA_m670, 21), DATANUM(DATA_m670, 22), DATANUM(DATA_m670, 23), DATANUM(DATA_m670, 24)},
    {DATANUM(DATA_m670, 26), DATANUM(DATA_m670, 27), DATANUM(DATA_m670, 28), DATANUM(DATA_m670, 29)},
    {DATANUM(DATA_m670, 31), DATANUM(DATA_m670, 32), DATANUM(DATA_m670, 33), DATANUM(DATA_m670, 34)},
    {DATANUM(DATA_m670, 36), DATANUM(DATA_m670, 37), DATANUM(DATA_m670, 38), DATANUM(DATA_m670, 39)}
};
int lbl_1_data_F0[5] = {
    DATANUM(DATA_m670, 3), DATANUM(DATA_m670, 4), DATANUM(DATA_m670, 6),
    DATANUM(DATA_m670, 5), DATANUM(DATA_m670, 9)
};
int lbl_1_data_104[3] = {DATANUM(DATA_m670, 0), DATANUM(DATA_m670, 1), DATANUM(DATA_m670, 2)};
unsigned int lbl_1_data_110[12] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mariomot, 2), DATANUM(DATA_mariomot, 3),
    DATANUM(DATA_mariomot, 4), DATANUM(DATA_mariomot, 6),
    DATANUM(DATA_mariomot, 7), DATANUM(DATA_mariomot, 94),
    DATANUM(DATA_mariomot, 23), DATANUM(DATA_mariomot, 34), 0, 0
};

BOOL fn_1_1460(HuVecF *src, HuVecF *dst)
{
    BOOL valid;
    if (PSVECSquareMag(src) < 1e-6) {
        dst->x = .01f * ((float)(u32)frandmod(20) - 10.0f);
        dst->z = .01f * ((float)(u32)frandmod(20) - 10.0f);
        dst->y = (u32)frandmod(1) != 0U ? .01f : -.01f;
        PSVECNormalize(dst, dst);
        valid = FALSE;
    } else {
        PSVECNormalize(src, dst);
        valid = TRUE;
    }
    return valid;
}

void fn_1_15B8(int sound, HuVecF *pos)
{
    HuVecF screen;
    int pan, handle;

    Hu3D3Dto2D(pos, 1, &screen);
    pan = screen.x;
    pan /= 5;
    if (pan < 48) {
        pan = 48;
    } else if (pan > 127) {
        pan = 127;
    }
    handle = HuAudFXPlay(sound);
    HuAudFXPanning(handle, pan);
}

void fn_1_1658(void)
{
    MGACTOR_PARAM param;
    HuVecF lightPos, lightAim, lightDir;
    HuVecF shadowPos, shadowUp, shadowTarget;
    HuVecF playerPos;
    int i, j;
    s8 *pattern;
    HU3D_MODELID model;
    MGPLAYER *player;
    OMOBJ *obj;
    int away;
    s8 type;
    HU3D_LIGHTID lightId;

    lbl_1_bss_10.objman = MgActorObjectSetup();
    lbl_1_bss_10.pattern = frandmod(5);
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
    for (i = 0; i < 3; i++) {
        lbl_1_bss_10.cameraMotion[i] = Hu3DMotionCreate(HuDataSelHeapReadNum(lbl_1_data_104[i], HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_10.cameraModel[i] = Hu3DModelCameraCreate(lbl_1_bss_10.cameraMotion[i], 1);
        Hu3DCameraMotionOff(lbl_1_bss_10.cameraModel[i]);
    }
    lightPos.x = 0.0f;
    lightPos.y = 600.0f;
    lightPos.z = 1000.0f;
    lightAim.x = 0.0f;
    lightAim.y = 0.0f;
    lightAim.z = 0.0f;
    PSVECSubtract(&lightAim, &lightPos, &lightDir);
    fn_1_1460(&lightDir, &lightDir);
    lightId = Hu3DGLightCreate(lightPos.x, lightPos.y, lightPos.z,
        lightDir.x, lightDir.y, lightDir.z, 255, 255, 255);
    Hu3DGLightInfinitytSet(lightId);
    Hu3DGLightStaticSet(0, 1);
    shadowPos.x = 0.0f;
    shadowPos.y = 10000.0f;
    shadowPos.z = 0.0f;
    shadowUp.x = 0.0f;
    shadowUp.y = 1.0f;
    shadowUp.z = 0.0f;
    shadowTarget.x = 0.0f;
    shadowTarget.y = 0.0f;
    shadowTarget.z = -0.1f;
    Hu3DShadowCreate(8.5f, 5000.0f, 11000.0f);
    Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);
    for (i = 0; i < 3; i++) {
        lbl_1_bss_10.models[i] = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_F0[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    Hu3DModelShadowMapObjSet(lbl_1_bss_10.models[2], "Cylinder12");
    Hu3DModelLayerSet(lbl_1_bss_10.models[2], 2);
    lbl_1_bss_10.collisionCount = 0;
    lbl_1_bss_10.positionModel = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m670, 7), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_10.positionModel, HU3D_ATTR_DISPOFF);
    lbl_1_bss_10.winnerModel = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m670, 8), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_10.winnerModel, HU3D_ATTR_DISPOFF);
    {
        char *positionNames[24] = {"D01", "D02", "D03", "D04", "D05", "D06", "D07", "D08",
            "D09", "D10", "D11", "D12", "D13", "D14", "D15", "D16", "D17", "D18",
            "D19", "D20", "D21", "D22", "D23", "D24"};
    for (i = 0; i < 24; i++) {
        Hu3DModelObjPosGet(lbl_1_bss_10.positionModel, positionNames[i], &lbl_1_bss_10.pillarPos[i]);
    }
    }
    pattern = lbl_1_data_250[lbl_1_bss_10.pattern];
    for (i = 0; i < 24; i++) {
        type = *pattern;
        model = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_78[type], HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_10.pillarModel[i] = model;
        Hu3DModelShadowMapObjSet(model, "dai");
        Hu3DModelLayerSet(model, 5);
        for (j = 0; j < 4; j++) {
            lbl_1_bss_10.pillarMotion[i][j] = Hu3DJointMotion(model, HuDataSelHeapReadNum(lbl_1_data_90[type][j], HU_MEMNUM_OVL, HEAP_MODEL));
        }
        lbl_1_bss_10.collisionModel[i] = lbl_1_bss_10.collisionModels[lbl_1_bss_10.collisionCount] =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m670, 9), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_10.collisionModel[i], HU3D_ATTR_DISPOFF);
        fn_1_22F0(i, -1500.0f);
        pattern++;
        lbl_1_bss_10.collisionCount++;
    }
    lbl_1_bss_10.state = 0;
    lbl_1_bss_10.pillarObject = omAddObjEx(lbl_1_bss_10.objman, 8192, 0, 0, -1, fn_1_3098);
    for (i = 0; i < 24; i++) {
        Hu3DMotionSet(lbl_1_bss_10.pillarModel[i], lbl_1_bss_10.pillarMotion[i][0]);
        Hu3DModelAttrReset(lbl_1_bss_10.pillarModel[i], HU3D_MOTATTR_LOOP | HU3D_ATTR_DISPOFF);
    }
    MgActorColMapInit(lbl_1_bss_10.collisionModels, lbl_1_bss_10.collisionCount, 50);
    {
    char *spawnNames[3] = {"P01", "P02", "P03"};
    param.height = 150.0f;
    param.radius = 60.0f;
    param.param = 0;
    param.type = 0;
    param.attr = 0;
    param.narrowHook = NULL;
    param.correctHook = fn_1_2384;
    away = 0;
    for (i = 0; i < 4; i++) {
        lbl_1_bss_10.characterNo[i] = GwPlayerConf[i].charNo;
        lbl_1_bss_10.padNo[i] = GwPlayerConf[i].padNo;
        lbl_1_bss_10.playerState[i] = 0;
        param.correctHookParam = i;
        lbl_1_bss_10.players[i] = player = MgPlayerCreate(i, &param, 4, 1,
            ~(MGPLAYER_ACTFLAG_PUNCH | MGPLAYER_ACTFLAG_KICK | MGPLAYER_ACTFLAG_HIPDROP), lbl_1_data_110);
        MgPlayerVibrateCreate(player);
        if (GwPlayerConf[i].grpNo == 0) {
            lbl_1_bss_10.group[i] = 0;
            lbl_1_bss_10.soloPlayer = i;
            lbl_1_bss_10.soloPad = lbl_1_bss_10.padNo[i];
            lbl_1_bss_10.soloCharacter = GwPlayerConf[i].charNo;
            Hu3DModelObjPosGet(lbl_1_bss_10.models[2], "P00", &playerPos);
            MgPlayerPosSet(player, &playerPos);
            Hu3DModelPosSetV(lbl_1_bss_10.players[i]->actor->mdlId, &playerPos);
            MgPlayerDespawn(player);
            lbl_1_bss_10.playerObjects[i] = omAddObjEx(lbl_1_bss_10.objman, 16384, 0, 0, -1, fn_1_2690);
            Hu3DModelLayerSet(lbl_1_bss_10.players[i]->actor->mdlId, 2);
        } else {
            lbl_1_bss_10.group[i] = 1;
            Hu3DModelObjPosGet(lbl_1_bss_10.positionModel, spawnNames[away], &playerPos);
            playerPos.y = -1500.0f;
            MgPlayerPosSet(player, &playerPos);
            lbl_1_bss_10.playerObjects[i] = omAddObjEx(lbl_1_bss_10.objman, 16384, 0, 0, -1, fn_1_28E8);
            Hu3DModelLayerSet(lbl_1_bss_10.players[i]->actor->mdlId, 5);
            away++;
        }
        obj = lbl_1_bss_10.playerObjects[i];
        obj->work[0] = i;
        obj->work[1] = (u32)player;
        Hu3DModelShadowSet(lbl_1_bss_10.players[i]->actor->mdlId);
        MgPlayerAttrSet(lbl_1_bss_10.players[i], MGPLAYER_ATTR_COMSTK);
        param.param++;
    }
    CharEffectLayerSet(5);
    lbl_1_bss_10.timer = MgTimerCreate(0);
    Hu3DZClearLayerSet(5);
    HuMCInit(0);
    HuMCSelWinCreate(-10000.0f, -10002.0f);
    lbl_1_bss_10.micContext = HuMCContextCreate("/mic/ctx/m670_words");
    fn_1_3B30();
    lbl_1_bss_10.music = -1;
    MgSeqCreate(&lbl_1_data_0);
    }
}
