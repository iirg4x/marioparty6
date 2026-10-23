/* Verbal Assault lighting, scene setup, player setup, and camera callbacks. */
#include "REL/m665dll/player.h"
#include "dolphin/types.h"

#include "math.h"
#include "REL/m665dll/sequence.h"

#include "dolphin/gx.h"

#include "game/mg/seqman.h"

#include "game/main.h"

#include "game/object.h"

#include "game/audio.h"

#include "game/charman.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/gamework.h"

#include "game/memory.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/sprite.h"

#include "game/wipe.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"
#include "game/hu3d.h"

/* ---- from text_00C64.c ---- */

typedef struct LightMapLevels {
    f32 level[2];
} LightMapLevels;

#define M665_MEMORY_TAG (1 << 28)

u32 lbl_1_data_68[7][2] = {
    {DATANUM(DATA_mariomot, 0), HU3D_MOTATTR_LOOP},
    {DATANUM(DATA_mariomot, 55), HU3D_MOTATTR_LOOP},
    {DATANUM(DATA_mariomot, 94), 0},
    {DATANUM(DATA_mariomot, 64), 0},
    {DATANUM(DATA_mariomot, 6), 0},
    {DATANUM(DATA_mariomot, 7), 0},
    {DATANUM(DATA_mariomot, 34), HU3D_MOTATTR_LOOP},
};

HuVecF lbl_1_data_A0 = {-2000.0f, 10000.0f, -400.0f};

HuVecF lbl_1_data_AC = {-0.4f, -0.8f, -0.8f};

GXColor lbl_1_data_B8[2] = {
    {192, 192, 192, 192},
    {192, 192, 192, 192},
};

s32 lbl_1_data_C0[2] = {DATANUM(DATA_m665, 2), DATANUM(DATA_m665, 3)};

s32 lbl_1_data_C8[2] = {DATANUM(DATA_m665, 4), DATANUM(DATA_m665, 5)};

/* The main sequence callback selects the surviving team or records a draw. */
s16 fn_1_C64(void) {
    s32 player;
    M665PlayerWork *playerWork;
    s16 winner;
    s16 result;

    winner = -1;
    result = 0;
    player = 0;
    while (player < 3) {
        playerWork = &lbl_1_bss_10[lbl_1_bss_250.otherPlayers[player]];
        if (playerWork->status.other.state < 5) {
            break;
        }
        player++;
    }
    if (player == 3) {
        lbl_1_bss_250.winningTeam = winner = 0;
        result = 1;
    }
    playerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    if (playerWork->models.main.damage >= 300) {
        if (winner == 0) {
            winner = -1;
        } else {
            winner = 1;
        }
        result = 1;
        lbl_1_bss_250.winningTeam = winner;
    }
    if (result != 0) {
        switch (lbl_1_bss_250.winningTeam) {
        case -1:
            MgSeqDrawSet();
            break;
        case 0:
            MgSeqWinnerSet(
                lbl_1_bss_10[lbl_1_bss_250.mainPlayer].character,
                -1, -1, -1);
            break;
        case 1:
            MgSeqWinnerSet(
                           lbl_1_bss_10[lbl_1_bss_250.otherPlayers[0]].character,
                           lbl_1_bss_10[lbl_1_bss_250.otherPlayers[1]].character,
                           lbl_1_bss_10[lbl_1_bss_250.otherPlayers[2]].character, -1);
            break;
        }
    }
    return result;
}

/* ---- from fn_1_E3C.c ---- */

#include <types.h>

extern s16 GwMgNightF;

extern void fn_1_EA8(void);

extern void fn_1_1028(void);

extern void fn_1_1470(void);

/* Sequence initialization captures night mode and prepares the scene and players. */
void fn_1_E3C(void) {
    s16 night;

    memset(&lbl_1_bss_250, 0, 36);
    night = GwMgNightF;
    lbl_1_bss_250.nightMode = night;
    lbl_1_bss_250.winningTeam = -1;
    fn_1_EA8();
    fn_1_1028();
    fn_1_1470();
}

extern HUPROCESS *lbl_1_bss_8;

/* Sequence initialization creates the scene camera and its view-update process. */
void fn_1_EA8(void) {
    OMOBJ *outViewObject;

    Hu3DCameraCreate(1);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(1, 20.0f, 60.0f, 25000.0f, 1.2f);
    outViewObject = omAddObjEx(lbl_1_bss_8, 32730, 0U, 0U, -1, omOutView);
    Center.x = 0.0f;
    Center.y = 250.0f;
    Center.z = -2000.0f;
    CRot.x = -14.0f;
    CRot.y = 0.0f;
    CRot.z = 0.0f;
    CZoom = 2800.0f;
}

void fn_1_13B4(OMOBJ *obj);

/* Sequence initialization sets lighting and shadows, then creates background and stage models. */
void fn_1_1028(void) {
    HuVecF shadowPosition = {500.0f, 6200.0f, -1000.0f};
    HuVecF shadowUp = {0.0f, 1.0f, 0.0f};
    HuVecF shadowTarget = {0.0f, 0.0f, -1000.0f};
    HuVecF characterLightPosition;
    HuVecF characterLightDirection;
    LightMapLevels shadowMapLevels = {{0.8f, 0.3f}};
    GXColor characterLightColor;
    s16 lightId;
    s16 baseModelId;
    s16 modelId;
    s32 modelIndex;

    lightId = Hu3DGLightCreateV(&lbl_1_data_A0,
                                &lbl_1_data_AC,
                                &lbl_1_data_B8[lbl_1_bss_250.nightMode]);
    Hu3DGLightStaticSet(lightId, TRUE);
    Hu3DGLightInfinitytSet(lightId);
    if (lbl_1_bss_250.nightMode != 0) {
        Hu3DAmbColorSet(0.5f, 0.6f, 0.85f);
        characterLightPosition.x = 0.0f;
        characterLightPosition.y = 400.0f;
        characterLightPosition.z = 1000.0f;
        characterLightDirection.x = 0.0f;
        characterLightDirection.y = -0.1f;
        characterLightDirection.z = -1.0f;
        characterLightColor.r = characterLightColor.g =
            characterLightColor.b = characterLightColor.a = 255;
        CharLightCreateV(&characterLightPosition, &characterLightDirection,
                         &characterLightColor);
        CharLightInfinitytSet();
    }
    Hu3DShadowCreate(30.0f, 20.0f, 20000.0f);
    Hu3DShadowPosSet(&shadowPosition, &shadowUp, &shadowTarget);
    for (modelIndex = 0; modelIndex < 4; modelIndex++) {
        if (modelIndex == 0) {
            modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
                lbl_1_data_C0[lbl_1_bss_250.nightMode], M665_MEMORY_TAG, HEAP_MODEL));
            baseModelId = modelId;
        } else {
            modelId = Hu3DModelLink(baseModelId);
        }
        Hu3DModelShadowMapSet(modelId);
        Hu3DModelShadowMapTPLvlSet(modelId,
                                   shadowMapLevels.level[lbl_1_bss_250.nightMode]);
        Hu3DModelPosSet(modelId, 0.0f, 0.0f,
                        -(5000.0f * (f32) modelIndex));
        lbl_1_bss_250.backgroundModels[modelIndex] = modelId;
    }
    modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
                lbl_1_data_C8[lbl_1_bss_250.nightMode], M665_MEMORY_TAG, HEAP_MODEL));
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_LOOP);
    lbl_1_bss_250.modelID = modelId;
    lbl_1_bss_278 = omAddObjEx(lbl_1_bss_8, 80, 0U, 0U, -1, fn_1_13B4);
    lbl_1_bss_250.scrollSpeed = 15.0f;
}

/* Runs every frame to scroll the four background copies and wrap them behind the camera. */
void fn_1_13B4(OMOBJ *obj) {
    Point3d backgroundPosition;
    s32 backgroundIndex;

    backgroundIndex = 0;
    while (backgroundIndex < 4) {
        Hu3DModelPosGet(lbl_1_bss_250.backgroundModels[backgroundIndex],
                        &backgroundPosition);
        backgroundPosition.z += lbl_1_bss_250.scrollSpeed;
        if (backgroundPosition.z >= 500.0f) {
            backgroundPosition.z -= 20000.0f;
        }
        Hu3DModelPosSetV(lbl_1_bss_250.backgroundModels[backgroundIndex],
                         &backgroundPosition);
        backgroundIndex += 1;
    }
}

extern OMOBJMAN *lbl_1_bss_8;

s16 print8(s16 x, s16 y, f32 scale, char *format, ...);

void fn_1_1C60(void);

void fn_1_C40C(void);

void fn_1_17E0(OMOBJ *object);

void fn_1_4768(void);

void fn_1_11C04(void);

void fn_1_170C(s16 playerNo, s16 mode, f32 frame);

extern s32 lbl_1_data_100[7];

void fn_1_6E8C(void);

void fn_1_5470(void);

void fn_1_98AC(void);

void fn_1_6188(void);

void fn_1_AD40(void);

void fn_1_8E90(void);

void fn_1_B09C(void);

/* Sequence initialization loads player configuration, character models, and seven motion slots. */
void fn_1_1470(void) {
    s32 playerIndex;
    s32 groupCount;
    s16 character;
    s16 modelID;
    s32 motionIndex;

    memset(&lbl_1_bss_10, 0, 576U);
    playerIndex = 0;
    groupCount = 0;
    while (playerIndex < 4) {
        lbl_1_bss_10[playerIndex].playerIndex = playerIndex;
        character = GwPlayerConf[playerIndex].charNo;
        lbl_1_bss_10[playerIndex].character = character;
        lbl_1_bss_10[playerIndex].controller =
            GwPlayerConf[playerIndex].padNo;
        lbl_1_bss_10[playerIndex].team =
            GwPlayerConf[playerIndex].grpNo;
        if (GwPlayerConf[playerIndex].type == 0) {
            lbl_1_bss_10[playerIndex].difficulty = -1;
        } else {
            lbl_1_bss_10[playerIndex].difficulty =
                GwPlayerConf[playerIndex].comDif;
        }
        if (lbl_1_bss_10[playerIndex].team == 0) {
            lbl_1_bss_250.mainPlayer = playerIndex;
        } else {
            lbl_1_bss_250.otherPlayers[groupCount] = playerIndex;
            lbl_1_bss_10[playerIndex].groupIndex =
                groupCount;
            groupCount += 1;
        }
        modelID = CharModelCreate(character, 4);
        Hu3DModelScaleSet(modelID, 1.2f, 1.2f,
                          1.2f);
        lbl_1_bss_10[playerIndex].characterModel = modelID;
        for (motionIndex = 0; motionIndex < 7; motionIndex++) {
            lbl_1_bss_10[playerIndex].characterMotions[motionIndex] =
                CharMotionCreate(character, lbl_1_data_68[motionIndex][0]);
        }
        fn_1_170C(playerIndex, 0, 0.0f);
        playerIndex += 1;
    }
    fn_1_1C60();
    fn_1_C40C();
    lbl_1_bss_274 = omAddObjEx(lbl_1_bss_8, 70, 0U, 0U, -1, fn_1_17E0);
}

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];

extern const VoidFunc _dtors[];

void fn_1_A0(void);

void fn_1_F0(s16 mode, s16 frame);

void fn_1_168(s16 mode, s16 frame);

void fn_1_188(s16 mode, s16 frame);

void fn_1_1C8(s16 mode, s16 frame);

void fn_1_200(s16 mode, s16 frame);

void fn_1_304(s16 mode, s16 frame);

void fn_1_3AC(s16 mode, s16 frame);

void fn_1_3B0(s16 mode, s16 frame);

void fn_1_3B4(s16 mode, s16 frame);

void fn_1_3B8(void);

void fn_1_484(void);

void fn_1_550(void);

void fn_1_8EC(void);

void fn_1_F90(OMOBJ *obj);

void fn_1_1228(void);

void fn_1_14FC(void);

void fn_1_1830(void);

void fn_1_1AE4(void);

void fn_1_2010(s16);

void fn_1_21CC(OMOBJ *obj);

void fn_1_22A8(s16 player, s16 motion);

void fn_1_2320(s16, s16);

void fn_1_23C4(s16 player, s16 motion, float blend);

void fn_1_2464(s16 player);

void fn_1_2558(void);

void fn_1_25B0(void);

void fn_1_26D0(void);

void fn_1_273C(s16);

void fn_1_289C(s16 player);

s16 fn_1_2CAC(s16);

void fn_1_2F1C(s16);

void fn_1_33F8(s16 player);

void fn_1_3F28(s16 player);

void fn_1_4430(s16 player);

void fn_1_473C(s16);

void fn_1_48C0(s16);

void fn_1_49C8(s16);

void fn_1_4BD4(s16);

s16 fn_1_4D58(s16);

void fn_1_5258(void);

void fn_1_5308(void);

void fn_1_5354(void);

void fn_1_5394(void);

void fn_1_54D4(void);

void fn_1_5544(s16);

void fn_1_5600(s16 unusedPlayer, f32 value, f32 *out0, f32 *out1);

s16 fn_1_5728(void);

void fn_1_5A24(s16);

void fn_1_5E40(void);

void fn_1_62E0(void);

void fn_1_6478(HU3D_MODEL *modelP, Mtx *mtx);

s16 fn_1_69B0(s16 player);

s16 fn_1_6D88(s16 player, HuVecF *pos);

s16 fn_1_6EE4(HuVecF *a, HuVecF *b, float radius);

void fn_1_6F54(void);

s16 fn_1_7000(s16 unusedPlayer, HuVecF *pos, float angle, s16 candidate);

extern GXColor lbl_1_data_40[2];

extern Point3d lbl_1_data_28;

extern Point3d lbl_1_data_34;

extern s16 lbl_1_bss_4AC[4];

extern s16 lbl_1_bss_4B4;

extern s16 lbl_1_bss_4B6;

extern s32 lbl_1_bss_2A8;

extern s32 lbl_1_data_48[4];

extern s32 lbl_1_data_80[2];

extern s32 lbl_1_data_88[2];

extern s32 lbl_1_data_90[2];

extern s32 lbl_1_data_98[2];

extern MGSEQ_PARAM lbl_1_data_0;

extern u8 lbl_1_bss_20[8];

/* Starts a character motion at frame zero with a nonnegative blend duration. */
void fn_1_170C(s16 player, s16 motion, f32 blendDuration)
{
    if (blendDuration <= 0.0f) {
        blendDuration = 0.0f;
    }
    CharMotionShiftSet(lbl_1_bss_10[player].character,
                       lbl_1_bss_10[player].characterMotions[motion],
                       0.0f, blendDuration, lbl_1_data_68[motion][1]);
}

/* ---- from take_017E0_text_00001470.c ---- */

/* Installed by player setup; runs each frame to update player and scene state. */
void fn_1_17E0(OMOBJ *object) {
    fn_1_4768();
    fn_1_11C04();
}

/* ---- from take_01804_text_00001470.c ---- */

/* Debug camera controls adjust and print the view while the modifier is held. */
void fn_1_1804(void) {
    if ((HuPadBtn[0] & 0x4000) != 0) {
        if ((HuPadBtn[0] & 0x400) != 0) {
            CZoom += 5.0f;
        }
        if ((HuPadBtn[0] & 0x800) != 0) {
            CZoom -= 5.0f;
        }
        if (HuPadSubStkY[0] > 32) {
            CRot.x += 1.0f;
        }
        if (HuPadSubStkY[0] < -32) {
            CRot.x -= 1.0f;
        }
        if (HuPadSubStkX[0] > 32) {
            CRot.y += 1.0f;
        }
        if (HuPadSubStkX[0] < -32) {
            CRot.y -= 1.0f;
        }
        if ((HuPadStkX[0] > 5) || (HuPadStkX[0] < -5)) {
            Center.x += HuPadStkX[0];
        }
        if ((HuPadStkY[0] > 5) || (HuPadStkY[0] < -5)) {
            Center.z -= HuPadStkY[0];
        }
        if ((HuPadBtn[0] & 8) != 0) {
            Center.y += 5.0f;
        }
        if ((HuPadBtn[0] & 4) != 0) {
            Center.y -= 5.0f;
        }
        print8(100, 300, 1.8f, "CnrX:%f", Center.x);
        print8(100, 316, 1.8f, "CnrY:%f", Center.y);
        print8(100, 332, 1.8f, "CnrZ:%f", Center.z);
        print8(100, 348, 1.8f, "RotZ:%f", CRot.x);
        print8(100, 364, 1.8f, "RotY:%f", CRot.y);
        print8(100, 380, 1.8f, "RotZ:%f", CRot.z);
        print8(100, 396, 1.8f, "Zoom:%f", CZoom);
    }
}

OMOBJ *lbl_1_bss_278;
OMOBJ *lbl_1_bss_274;
M665SequenceState lbl_1_bss_250;
M665PlayerWork lbl_1_bss_10[4];
