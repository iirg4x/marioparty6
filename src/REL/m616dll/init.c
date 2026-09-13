#include "REL/m616dll.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/pad.h"
#include "game/frand.h"
#include "datadir_enum.h"
#include <string.h>


typedef struct M616CpuParam {
    u32 unk_00;
    u32 repeatPercent;
} M616CpuParam;
extern MGSEQ_PARAM lbl_1_data_0;
extern s32 lbl_1_data_1F0[17];
extern s32 lbl_1_data_234[6];
extern s32 lbl_1_data_24C[13];
extern unsigned int lbl_1_data_280[12];
extern M616CpuParam lbl_1_data_2B0[4];
extern s32 lbl_1_data_2D0[3];
void fn_1_1590(void)
{
    s32 i;
    s32 j;

    memset(&lbl_1_bss_10, 0, sizeof(lbl_1_bss_10));
    lbl_1_bss_10.activeCameraModel = HU3D_MODELID_NONE;
    lbl_1_bss_10.objectManager = omInitObjMan(100, 8192);
    omGameSysInit(lbl_1_bss_10.objectManager);
    lbl_1_bss_10.shadowPos.x = 0.0f;
    lbl_1_bss_10.shadowPos.y = 3000.0f;
    lbl_1_bss_10.shadowPos.z = 0.0f;
    lbl_1_bss_10.shadowUp.x = 0.0f;
    lbl_1_bss_10.shadowUp.y = 0.0f;
    lbl_1_bss_10.shadowUp.z = 1.0f;
    lbl_1_bss_10.shadowTarget.x = 0.0f;
    lbl_1_bss_10.shadowTarget.y = -100.0f;
    lbl_1_bss_10.shadowTarget.z = 0.0f;
    Hu3DShadowCreate(30.0f, 1.0f, 13000.0f);
    Hu3DShadowPosSet(&lbl_1_bss_10.shadowPos, &lbl_1_bss_10.shadowUp, &lbl_1_bss_10.shadowTarget);
    Hu3DShadowColSet(50, 50, 50);
    Hu3DShadowSizeSet(192);
    Hu3DShadowTPLvlSet(0.7f);
    Hu3DCameraCreate(HU3D_CAM0);
    Hu3DCameraPerspectiveSet(HU3D_CAM0, 45.0f, 1.0f, 10000.0f, 1.2f);
    lbl_1_bss_10.cameraObject = omAddObjEx(lbl_1_bss_10.objectManager, 12288, 0, 0, -1, fn_1_1364);
    for (i = 0; i < 17; i++) {
        lbl_1_bss_10.cameraMotions[i] = Hu3DMotionCreateData(lbl_1_data_1F0[i]);
    }
    lbl_1_bss_10.unk_CC = Hu3DModelCreateData(DATANUM(DATA_m616, 20));
    Hu3DModelShadowSet(lbl_1_bss_10.unk_CC);
    lbl_1_bss_10.unk_CE = Hu3DModelCreateData(DATANUM(DATA_m616, 12));
    Hu3DModelShadowSet(lbl_1_bss_10.unk_CE);
    lbl_1_bss_10.resultModels[0] = Hu3DModelCreateData(DATANUM(DATA_m616, 41));
    lbl_1_bss_10.resultModels[1] = Hu3DModelCreateData(DATANUM(DATA_m616, 42));
    lbl_1_bss_10.resultModels[2] = Hu3DModelCreateData(DATANUM(DATA_m616, 43));
    lbl_1_bss_10.resultModels[3] = Hu3DModelCreateData(DATANUM(DATA_m616, 44));
    for (i = 0; i < 4; i++) {
        Hu3DModelAttrSet(lbl_1_bss_10.resultModels[i], HU3D_ATTR_DISPOFF);
    }
    lbl_1_bss_10.stageModels[0] = Hu3DModelCreateData(DATANUM(DATA_m616, 13));
    Hu3DModelAttrSet(lbl_1_bss_10.stageModels[0], HU3D_MOTATTR_LOOP);
    Hu3DModelShadowMapObjSet(lbl_1_bss_10.stageModels[0], "616erande-stage01");
    lbl_1_bss_10.stageModels[1] = Hu3DModelCreateData(DATANUM(DATA_m616, 14));
    Hu3DModelAttrSet(lbl_1_bss_10.stageModels[1], HU3D_MOTATTR_LOOP);
    Hu3DModelShadowMapObjSet(lbl_1_bss_10.stageModels[1], "616erande-stage001");
    for (i = 0; i < 4; i++) {
        lbl_1_bss_10.playerBaseModels[i] = Hu3DModelCreateData(DATANUM(DATA_m616, 21));
    }
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 13; j++) {
            lbl_1_bss_10.playerBaseMotions[i][j] =
                Hu3DJointMotionData(lbl_1_bss_10.playerBaseModels[i], lbl_1_data_24C[j]);
        }
    }
    Hu3DModelHookSet(lbl_1_bss_10.unk_CC, "p1daiiti", lbl_1_bss_10.playerBaseModels[0]);
    Hu3DModelHookSet(lbl_1_bss_10.unk_CC, "p2daiiti", lbl_1_bss_10.playerBaseModels[1]);
    Hu3DModelHookSet(lbl_1_bss_10.unk_CC, "p3daiiti", lbl_1_bss_10.playerBaseModels[2]);
    Hu3DModelHookSet(lbl_1_bss_10.unk_CC, "p4daiiti", lbl_1_bss_10.playerBaseModels[3]);
    lbl_1_bss_10.centerModel = Hu3DModelCreateData(DATANUM(DATA_m616, 15));
    for (i = 0; i < 3; i++) {
        lbl_1_bss_10.centerMotions[i] = Hu3DJointMotionData(lbl_1_bss_10.centerModel, lbl_1_data_2D0[i]);
    }
    lbl_1_bss_10.playerModels[0] = Hu3DModelCreateData(DATANUM(DATA_m616, 19));
    for (i = 1; i < 4; i++) {
        lbl_1_bss_10.playerModels[i] = Hu3DModelLink(lbl_1_bss_10.playerModels[0]);
    }
    for (i = 0; i < 6; i++) {
        lbl_1_bss_10.playerMotions[i] = Hu3DJointMotionData(lbl_1_bss_10.playerModels[0], lbl_1_data_234[i]);
    }
    for (i = 0; i < 4; i++) {
        Hu3DModelShadowMapObjSet(lbl_1_bss_10.playerModels[i], "p1neji_sha");
    }
    Hu3DModelHookSet(lbl_1_bss_10.unk_CC, "p1nejiiti", lbl_1_bss_10.playerModels[0]);
    Hu3DModelHookSet(lbl_1_bss_10.unk_CC, "p2nejiiti", lbl_1_bss_10.playerModels[1]);
    Hu3DModelHookSet(lbl_1_bss_10.unk_CC, "p3nejiiti", lbl_1_bss_10.playerModels[2]);
    Hu3DModelHookSet(lbl_1_bss_10.unk_CC, "p4nejiiti", lbl_1_bss_10.playerModels[3]);
    fn_1_2104(0, 0);
    fn_1_2104(1, 0);
    fn_1_2104(2, 0);
    fn_1_2104(3, 0);
    {
        HU3D_LIGHTID light;
        HuVecF aim;
        HuVecF lightPos;
        HuVecF pos = { 100.0f, 800.0f, 1000.0f };
        HuVecF dir = { 0.3f, -0.8f, 0.3f };
        HuVecF unk_90 = { 20.0f, 45.0f, 1000.0f };
        GXColor color = { 255, 255, 255, 255 };

        light = Hu3DGLightCreateV(&pos, &dir, &color);
        Hu3DGLightInfinitytSet(light);
        Hu3DGLightStaticSet(light, TRUE);
        aim.x = 0.0f;
        aim.y = 0.0f;
        aim.z = 0.0f;
        lightPos.x = 1.0f;
        lightPos.y = 7825.0f;
        lightPos.z = 9582.0f;
        Hu3DGLightPosAimSetV(light, &lightPos, &aim);
    }
    for (i = 0; i < 4; i++) {
        s32 character;
        lbl_1_bss_10.padNos[i] = GwPlayerConf[i].padNo;
        character = lbl_1_bss_10.characterNos[i] = GwPlayerConf[i].charNo;
        lbl_1_bss_10.padNos[i] = GwPlayerConf[i].padNo;
        lbl_1_bss_10.characterModels[i] = CharModelMotListCreate(character, CHAR_MODEL2,
            lbl_1_data_280, lbl_1_bss_10.characterMotions[i]);
        Hu3DModelShadowSet(lbl_1_bss_10.characterModels[i]);
        CharMotionVoiceOnSet(character, 36, FALSE);
        CharMotionVoiceOnSet(character, 37, FALSE);
    }
    lbl_1_bss_10.timer = MgTimerCreate(MGTIMER_TYPE_NORMAL);
    CharEffectLayerSet(3);
    fn_1_1FF4(1);
    lbl_1_bss_10.streamNo = -1;
    MgSeqCreate(&lbl_1_data_0);
}
