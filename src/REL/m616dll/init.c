/* Builds M616's camera, stage, character, and timer resources before starting its game sequence. */
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

/* Called from the REL prolog after constructors; create resources and register the sequence
 * callbacks. */
void ObjectSetup(void)
{
    s32 assetIndex;
    s32 motionIndex;

    memset(&lbl_1_bss_10, 0, sizeof(lbl_1_bss_10));
    lbl_1_bss_10.activeCameraModel = -1;
    lbl_1_bss_10.objectManager = fn_8003A8BC(100, 8192);
    fn_80069F1C(lbl_1_bss_10.objectManager);
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
    fn_8002AAB4(50, 50, 50);
    Hu3DShadowSizeSet(192);
    Hu3DShadowTPLvlSet(0.7f);
    Hu3DCameraCreate((1 << 0));
    Hu3DCameraPerspectiveSet((1 << 0), 45.0f, 1.0f, 10000.0f, 1.2f);
    lbl_1_bss_10.cameraObject = fn_8003AB38(lbl_1_bss_10.objectManager, 12288, 0, 0, -1, fn_1_1364);
    for (assetIndex = 0; assetIndex < 17; assetIndex++) {
        lbl_1_bss_10.cameraMotions[assetIndex] = fn_8002C4C0(fn_80008DD0(lbl_1_data_1F0[assetIndex],
            0x10000000, 2));
    }
    lbl_1_bss_10.playerHookRoot = fn_80025A04(fn_80008DD0((DATANUM(0x4b0000, 20)), 0x10000000, 2));
    fn_80026E44(lbl_1_bss_10.playerHookRoot);
    lbl_1_bss_10.openingMotionModel = fn_80025A04(fn_80008DD0((DATANUM(0x4b0000, 12)), 0x10000000,
        2));
    fn_80026E44(lbl_1_bss_10.openingMotionModel);
    lbl_1_bss_10.resultModels[0] = fn_80025A04(fn_80008DD0((DATANUM(0x4b0000, 41)), 0x10000000, 2));
    lbl_1_bss_10.resultModels[1] = fn_80025A04(fn_80008DD0((DATANUM(0x4b0000, 42)), 0x10000000, 2));
    lbl_1_bss_10.resultModels[2] = fn_80025A04(fn_80008DD0((DATANUM(0x4b0000, 43)), 0x10000000, 2));
    lbl_1_bss_10.resultModels[3] = fn_80025A04(fn_80008DD0((DATANUM(0x4b0000, 44)), 0x10000000, 2));
    for (assetIndex = 0; assetIndex < 4; assetIndex++) {
        fn_800269E8(lbl_1_bss_10.resultModels[assetIndex], (1 << 0));
    }
    lbl_1_bss_10.stageModels[0] = fn_80025A04(fn_80008DD0((DATANUM(0x4b0000, 13)), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_10.stageModels[0], ((1 << 30) | (1 << 0)));
    fn_80027080(lbl_1_bss_10.stageModels[0], "616erande-stage01");
    lbl_1_bss_10.stageModels[1] = fn_80025A04(fn_80008DD0((DATANUM(0x4b0000, 14)), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_10.stageModels[1], ((1 << 30) | (1 << 0)));
    fn_80027080(lbl_1_bss_10.stageModels[1], "616erande-stage001");
    for (assetIndex = 0; assetIndex < 4; assetIndex++) {
        lbl_1_bss_10.playerBaseModels[assetIndex] = fn_80025A04(fn_80008DD0((DATANUM(0x4b0000, 21)),
            0x10000000, 2));
    }
    for (assetIndex = 0; assetIndex < 4; assetIndex++) {
        for (motionIndex = 0; motionIndex < 13; motionIndex++) {
            lbl_1_bss_10.playerBaseMotions[assetIndex][motionIndex] =
                fn_80030458(lbl_1_bss_10.playerBaseModels[assetIndex],
                fn_80008DD0(lbl_1_data_24C[motionIndex], 0x10000000, 2));
        }
    }
    fn_8002741C(lbl_1_bss_10.playerHookRoot, "p1daiiti", lbl_1_bss_10.playerBaseModels[0]);
    fn_8002741C(lbl_1_bss_10.playerHookRoot, "p2daiiti", lbl_1_bss_10.playerBaseModels[1]);
    fn_8002741C(lbl_1_bss_10.playerHookRoot, "p3daiiti", lbl_1_bss_10.playerBaseModels[2]);
    fn_8002741C(lbl_1_bss_10.playerHookRoot, "p4daiiti", lbl_1_bss_10.playerBaseModels[3]);
    lbl_1_bss_10.centerModel = fn_80025A04(fn_80008DD0((DATANUM(0x4b0000, 15)), 0x10000000, 2));
    for (assetIndex = 0; assetIndex < 3; assetIndex++) {
        lbl_1_bss_10.centerMotions[assetIndex] =
            fn_80030458(lbl_1_bss_10.centerModel, fn_80008DD0(lbl_1_data_2D0[assetIndex],
                0x10000000, 2));
    }
    lbl_1_bss_10.playerModels[0] = fn_80025A04(fn_80008DD0((DATANUM(0x4b0000, 19)), 0x10000000, 2));
    for (assetIndex = 1; assetIndex < 4; assetIndex++) {
        lbl_1_bss_10.playerModels[assetIndex] = fn_80025D4C(lbl_1_bss_10.playerModels[0]);
    }
    for (assetIndex = 0; assetIndex < 6; assetIndex++) {
        lbl_1_bss_10.playerMotions[assetIndex] =
            fn_80030458(lbl_1_bss_10.playerModels[0], fn_80008DD0(lbl_1_data_234[assetIndex],
                0x10000000, 2));
    }
    for (assetIndex = 0; assetIndex < 4; assetIndex++) {
        fn_80027080(lbl_1_bss_10.playerModels[assetIndex], "p1neji_sha");
    }
    fn_8002741C(lbl_1_bss_10.playerHookRoot, "p1nejiiti", lbl_1_bss_10.playerModels[0]);
    fn_8002741C(lbl_1_bss_10.playerHookRoot, "p2nejiiti", lbl_1_bss_10.playerModels[1]);
    fn_8002741C(lbl_1_bss_10.playerHookRoot, "p3nejiiti", lbl_1_bss_10.playerModels[2]);
    fn_8002741C(lbl_1_bss_10.playerHookRoot, "p4nejiiti", lbl_1_bss_10.playerModels[3]);
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
        HuVecF lightPosition = { 20.0f, 45.0f, 1000.0f };
        GXColor color = { 255, 255, 255, 255 };

        light = fn_800286D0(&pos, &dir, &color);
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
    for (assetIndex = 0; assetIndex < 4; assetIndex++) {
        s32 characterNo;
        lbl_1_bss_10.padNos[assetIndex] = lbl_80265728[assetIndex].padNo;
        characterNo = lbl_1_bss_10.characterNos[assetIndex] = lbl_80265728[assetIndex].charNo;
        /* The configured controller port is assigned a second time before model creation. */
        lbl_1_bss_10.padNos[assetIndex] = lbl_80265728[assetIndex].padNo;
        lbl_1_bss_10.characterModels[assetIndex] = fn_8005C4E4(characterNo, (1 << 2),
            lbl_1_data_280, lbl_1_bss_10.characterMotions[assetIndex]);
        fn_80026E44(lbl_1_bss_10.characterModels[assetIndex]);
        /* Disable character voice playback for motions 36 and 37 in this minigame. */
        fn_8005DEB0(characterNo, FILENUM(CHARMOT_HSF_c000m1_346), FALSE);
        fn_8005DEB0(characterNo, FILENUM(CHARMOT_HSF_c000m1_348), FALSE);
    }
    lbl_1_bss_10.timer = MgTimerCreate(MGTIMER_TYPE_NORMAL);
    fn_8005DDEC(3);
    fn_1_1FF4(1);
    lbl_1_bss_10.streamNo = -1;
    fn_80070788(&lbl_1_data_0);
}
