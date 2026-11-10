/* Builds Fruit Talktail's camera, word pillars, players, microphone context, and sequence. */
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
#include "game/window.h"
#include "datadir_enum.h"
#define M670_MCSELWIN_POS_TOP -10002.0f
extern MGSEQ_PARAM lbl_1_data_0;

M670WORK lbl_1_bss_10;

int lbl_1_data_78[6] = {
    DATANUM(0x810000, 10), DATANUM(0x810000, 15),
    DATANUM(0x810000, 20), DATANUM(0x810000, 25),
    DATANUM(0x810000, 30), DATANUM(0x810000, 35)
};
/* Four motion resources are listed for each of the six pillar types. */
int lbl_1_data_90[6][4] = {
    {DATANUM(0x810000, 11), DATANUM(0x810000, 12), DATANUM(0x810000, 13), DATANUM(0x810000, 14)},
    {DATANUM(0x810000, 16), DATANUM(0x810000, 17), DATANUM(0x810000, 18), DATANUM(0x810000, 19)},
    {DATANUM(0x810000, 21), DATANUM(0x810000, 22), DATANUM(0x810000, 23), DATANUM(0x810000, 24)},
    {DATANUM(0x810000, 26), DATANUM(0x810000, 27), DATANUM(0x810000, 28), DATANUM(0x810000, 29)},
    {DATANUM(0x810000, 31), DATANUM(0x810000, 32), DATANUM(0x810000, 33), DATANUM(0x810000, 34)},
    {DATANUM(0x810000, 36), DATANUM(0x810000, 37), DATANUM(0x810000, 38), DATANUM(0x810000, 39)}
};
int lbl_1_data_F0[5] = {
    DATANUM(0x810000, 3), DATANUM(0x810000, 4), DATANUM(0x810000, 6),
    DATANUM(0x810000, 5), DATANUM(0x810000, 9)
};
int lbl_1_data_104[3] = {DATANUM(0x810000, 0), DATANUM(0x810000, 1), DATANUM(0x810000, 2)};
unsigned int lbl_1_data_110[12] = {
    DATANUM(0x930000, 0), DATANUM(0x930000, 1),
    DATANUM(0x930000, 2), DATANUM(0x930000, 3),
    DATANUM(0x930000, 4), DATANUM(0x930000, 6),
    DATANUM(0x930000, 7), DATANUM(0x930000, 94),
    DATANUM(0x930000, 23), DATANUM(0x930000, 34), 0, 0
};
/* Scene setup and CPU movement normalize directions; near zero, this substitutes a random unit
 * direction with negative y and returns FALSE. */
BOOL fn_1_1460(HuVecF *src, HuVecF *dst)
{
    BOOL valid;
    if (PSVECSquareMag(src) < 1e-6) {
        dst->x = .01f * ((float)(u32)frandmod(20) - 10.0f);
        dst->z = .01f * ((float)(u32)frandmod(20) - 10.0f);
        /* frandmod(1) yields zero, so the fallback's vertical component is always negative. */
        dst->y = (u32)frandmod(1) != 0U ? .01f : -.01f;
        PSVECNormalize(dst, dst);
        valid = FALSE;
    } else {
        PSVECNormalize(src, dst);
        valid = TRUE;
    }
    return valid;
}
/* The pillar update uses this to play a sound with left-right panning from a world position. */
void fn_1_15B8(int soundId, HuVecF *worldPos)
{
    HuVecF screen;
    int pan, handle;

    Hu3D3Dto2D(worldPos, 1, &screen);
    pan = screen.x;
    pan /= 5;
    if (pan < 48) {
        pan = 48;
    } else if (pan > 127) {
        pan = 127;
    }
    handle = HuAudFXPlay(soundId);
    HuAudFXPanning(handle, pan);
}
/* Called by the overlay prolog to create the playfield, players, microphone context and selection
 * window, and sequence. */
void ObjectSetup(void)
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

    lbl_1_bss_10.objman = fn_80080418();
    lbl_1_bss_10.pattern = frandmod(5);
    lbl_80266A40.x = -35.0f;
    lbl_80266A40.y = 0.0f;
    lbl_80266A40.z = 0.0f;
    lbl_80266A4C.x = 0.0f;
    lbl_80266A4C.y = 700.0f;
    lbl_80266A4C.z = 400.0f;
    lbl_802C028C = 1000.0f;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 20.0f, 8000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    for (i = 0; i < 3; i++) {
        lbl_1_bss_10.cameraMotion[i] =
            fn_8002C4C0(fn_80008DD0(lbl_1_data_104[i], 0x10000000, 2));
        lbl_1_bss_10.cameraModel[i] = fn_800283F4(lbl_1_bss_10.cameraMotion[i], 1);
        fn_8002858C(lbl_1_bss_10.cameraModel[i]);
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
        lbl_1_bss_10.models[i] =
            fn_80025A04(fn_80008DD0(lbl_1_data_F0[i], 0x10000000, 2));
    }
    fn_80027080(lbl_1_bss_10.models[2], "Cylinder12");
    fn_80026B8C(lbl_1_bss_10.models[2], 2);
    lbl_1_bss_10.collisionCount = 0;
    lbl_1_bss_10.positionModel =
        fn_80025A04(fn_80008DD0(DATANUM(0x810000, 7), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_10.positionModel, (1 << 0));
    lbl_1_bss_10.winnerModel =
        fn_80025A04(fn_80008DD0(DATANUM(0x810000, 8), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_10.winnerModel, (1 << 0));
    {
        char *positionNames[24] = {"D01", "D02", "D03", "D04", "D05", "D06", "D07", "D08",
            "D09", "D10", "D11", "D12", "D13", "D14", "D15", "D16", "D17", "D18",
            "D19", "D20", "D21", "D22", "D23", "D24"};
    for (i = 0; i < 24; i++) {
        Hu3DModelObjPosGet(lbl_1_bss_10.positionModel, positionNames[i],
                           &lbl_1_bss_10.pillarPos[i]);
    }
    }
    pattern = lbl_1_data_250[lbl_1_bss_10.pattern];
    for (i = 0; i < 24; i++) {
        type = *pattern;
        model =
            fn_80025A04(fn_80008DD0(lbl_1_data_78[type], 0x10000000, 2));
        lbl_1_bss_10.pillarModel[i] = model;
        fn_80027080(model, "dai");
        fn_80026B8C(model, 5);
        for (j = 0; j < 4; j++) {
            lbl_1_bss_10.pillarMotion[i][j] = fn_80030458(
                model, fn_80008DD0(lbl_1_data_90[type][j], 0x10000000, 2));
        }
        lbl_1_bss_10.collisionModel[i] = lbl_1_bss_10.collisionModels[lbl_1_bss_10.collisionCount] =
            fn_80025A04(fn_80008DD0(DATANUM(0x810000, 9), 0x10000000, 2));
        fn_800269E8(lbl_1_bss_10.collisionModel[i], (1 << 0));
        fn_1_22F0(i, -1500.0f);
        pattern++;
        lbl_1_bss_10.collisionCount++;
    }
    lbl_1_bss_10.state = 0;
    lbl_1_bss_10.pillarObject = fn_8003AB38(lbl_1_bss_10.objman, 8192, 0, 0, -1, fn_1_3098);
    for (i = 0; i < 24; i++) {
        fn_8002C95C(lbl_1_bss_10.pillarModel[i], lbl_1_bss_10.pillarMotion[i][0]);
        fn_80026A44(lbl_1_bss_10.pillarModel[i], ((1 << 30) | (1 << 0)) | (1 << 0));
    }
    fn_800804E8(lbl_1_bss_10.collisionModels, lbl_1_bss_10.collisionCount, 50);
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
        lbl_1_bss_10.characterNo[i] = lbl_80265728[i].charNo;
        lbl_1_bss_10.padNo[i] = lbl_80265728[i].padNo;
        lbl_1_bss_10.playerState[i] = 0;
        param.correctHookParam = i;
        lbl_1_bss_10.players[i] = player = fn_80080728(
            i, &param, 4, 1,
            ~((1 << 2) | (1 << 3) | (1 << 4)),
            lbl_1_data_110);
        fn_8007ED60(player);
        if (lbl_80265728[i].grpNo == 0) {
            lbl_1_bss_10.group[i] = 0;
            lbl_1_bss_10.soloPlayer = i;
            lbl_1_bss_10.soloPad = lbl_1_bss_10.padNo[i];
            lbl_1_bss_10.soloCharacter = lbl_80265728[i].charNo;
            Hu3DModelObjPosGet(lbl_1_bss_10.models[2], "P00", &playerPos);
            fn_8008697C(player, &playerPos);
            fn_80026790(lbl_1_bss_10.players[i]->actor->mdlId, &playerPos);
            fn_80086E3C(player);
            lbl_1_bss_10.playerObjects[i] =
                fn_8003AB38(lbl_1_bss_10.objman, 16384, 0, 0, -1, fn_1_2690);
            fn_80026B8C(lbl_1_bss_10.players[i]->actor->mdlId, 2);
        } else {
            lbl_1_bss_10.group[i] = 1;
            Hu3DModelObjPosGet(lbl_1_bss_10.positionModel, spawnNames[away], &playerPos);
            playerPos.y = -1500.0f;
            fn_8008697C(player, &playerPos);
            lbl_1_bss_10.playerObjects[i] =
                fn_8003AB38(lbl_1_bss_10.objman, 16384, 0, 0, -1, fn_1_28E8);
            fn_80026B8C(lbl_1_bss_10.players[i]->actor->mdlId, 5);
            away++;
        }
        obj = lbl_1_bss_10.playerObjects[i];
        obj->work[0] = i;
        obj->work[1] = (u32)player;
        fn_80026E44(lbl_1_bss_10.players[i]->actor->mdlId);
        fn_80087158(lbl_1_bss_10.players[i], (1 << 0));
        param.param++;
    }
    fn_8005DDEC(5);
    lbl_1_bss_10.timer = MgTimerCreate(0);
    fn_8002C2AC(5);
    HuMCInit(0);
    /* The selection-window helper maps this y sentinel to 40 pixels from the top. */
    HuMCSelWinCreate(HUWIN_POS_CENTER, M670_MCSELWIN_POS_TOP);
    lbl_1_bss_10.micContext = HuMCContextCreate("/mic/ctx/m670_words");
    fn_1_3B30();
    lbl_1_bss_10.music = -1;
    fn_80070788(&lbl_1_data_0);
    }
}
