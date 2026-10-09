/* Memory Lane scene setup, board logic, and per-frame gameplay callbacks. */
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "datadir_enum.h"
#include "string.h"
#include "math.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"
#include "REL/m612dll.h"

/* Memory Lane model and motion resources, named by their use in this scene. */
#define M612_DATA_DAY_TILE DATANUM(DATA_m612, 0)
#define M612_DATA_DAY_CURSOR DATANUM(DATA_m612, 1)
#define M612_DATA_DAY_BOARD_0 DATANUM(DATA_m612, 2)
#define M612_DATA_DAY_BOARD_1 DATANUM(DATA_m612, 3)
#define M612_DATA_DAY_BOARD_2 DATANUM(DATA_m612, 4)
#define M612_DATA_DAY_BOARD_3 DATANUM(DATA_m612, 5)
#define M612_DATA_DAY_BOARD_4 DATANUM(DATA_m612, 6)
#define M612_DATA_DAY_BOARD_5 DATANUM(DATA_m612, 7)
#define M612_DATA_DAY_BOARD_6 DATANUM(DATA_m612, 8)
#define M612_DATA_NIGHT_TILE DATANUM(DATA_m612, 9)
#define M612_DATA_NIGHT_CURSOR DATANUM(DATA_m612, 10)
#define M612_DATA_NIGHT_BOARD_0 DATANUM(DATA_m612, 11)
#define M612_DATA_NIGHT_BOARD_1 DATANUM(DATA_m612, 12)
#define M612_DATA_NIGHT_BOARD_2 DATANUM(DATA_m612, 13)
#define M612_DATA_NIGHT_BOARD_3 DATANUM(DATA_m612, 14)
#define M612_DATA_NIGHT_BOARD_4 DATANUM(DATA_m612, 15)
#define M612_DATA_NIGHT_BOARD_5 DATANUM(DATA_m612, 16)
#define M612_DATA_NIGHT_BOARD_6 DATANUM(DATA_m612, 17)
#define M612_DATA_NIGHT_BOARD_7 DATANUM(DATA_m612, 18)
#define M612_DATA_NIGHT_BOARD_8 DATANUM(DATA_m612, 19)
#define M612_DATA_NIGHT_BOARD_9 DATANUM(DATA_m612, 20)
#define M612_DATA_MARKER_MODEL DATANUM(DATA_m612, 21)
#define M612_DATA_MARKER_MOTION DATANUM(DATA_m612, 22)
#define M612_DATA_RESULT_MODEL DATANUM(DATA_m612, 23)
#define M612_DATA_RESULT_MOTION_0 DATANUM(DATA_m612, 24)
#define M612_DATA_RESULT_MOTION_1 DATANUM(DATA_m612, 25)
#define M612_DATA_RESULT_MOTION_2 DATANUM(DATA_m612, 26)

/* Character motions used by the idle, movement, result, and tile actions. */
#define M612_CHAR_MOTION_IDLE DATANUM(DATA_mariomot, 0)
#define M612_CHAR_MOTION_MOVE DATANUM(DATA_mariomot, 1)
#define M612_CHAR_MOTION_RESULT DATANUM(DATA_mariomot, 6)
#define M612_CHAR_MOTION_RESULT_ALT DATANUM(DATA_mariomot, 7)
#define M612_CHAR_MOTION_STEP DATANUM(DATA_mariomot, 15)
#define M612_CHAR_MOTION_FALL DATANUM(DATA_mariomot, 34)
#define M612_CHAR_MOTION_RISE DATANUM(DATA_mariomot, 63)
#define M612_CHAR_MOTION_EXTRA DATANUM(DATA_mariomot, 23)
#define M612_SE_TILE_MARK 1698
#define M612_SE_MARKER_LAND 1699
#define M612_SE_PLAYER_FALL 1700
#define M612_SE_GUIDE_MOVE 1701
#define M612_SE_GUIDE_STEP 1702
#define M612_RESULT_BGM 83

M612Scene lbl_1_bss_4AC;
static s32 lbl_1_bss_49C[4];
static s32 lbl_1_bss_48C[4];
s16 lbl_1_bss_44C[4][8];
static s32 lbl_1_bss_43C[4];
static s32 lbl_1_bss_42C[4];
static s32 lbl_1_bss_41C[4];
static s32 lbl_1_bss_40C[4];
static s32 lbl_1_bss_3FC[4];
static s32 lbl_1_bss_3EC[4];
static f32 lbl_1_bss_3DC[4];
static s32 lbl_1_bss_3CC[4];
static s32 lbl_1_bss_3BC[4];
static s32 lbl_1_bss_3AC[4];
static s32 lbl_1_bss_39C[4];
static s32 lbl_1_bss_38C[4];
static s32 lbl_1_bss_37C[4];
static s32 lbl_1_bss_36C[4];
/* Per-player wait counters time the tile-mark state before a failed attempt makes the player
 * fall. */
static s32 lbl_1_bss_30C[24];
static s32 lbl_1_bss_308;
static s32 lbl_1_bss_304;
static s32 lbl_1_bss_300;
static s32 lbl_1_bss_2FC;
static s32 lbl_1_bss_2F8;
static s32 lbl_1_bss_2F4;
static s32 lbl_1_bss_2F0;
static f32 lbl_1_bss_2EC;
static f32 lbl_1_bss_2E8;
/* Model IDs for the four player cursor effects. */
static s32 lbl_1_bss_2C0[10];
/* Selected route as cardinal tile directions, terminated by zero. */
static s32 lbl_1_bss_48[158];
static s32 lbl_1_bss_44;
static s32 lbl_1_bss_40;
static s32 lbl_1_bss_3C;
static s32 lbl_1_bss_38;
static s32 lbl_1_bss_34;
static s32 lbl_1_bss_30;
static s32 lbl_1_bss_2C;
static s32 lbl_1_bss_28;
static s32 lbl_1_bss_24;
static s32 lbl_1_bss_20;
static s32 lbl_1_bss_1C;
static s32 lbl_1_bss_18;
static s32 lbl_1_bss_14;
static s32 lbl_1_bss_10;
static s32 lbl_1_bss_C;
static s32 lbl_1_bss_8;
static s32 lbl_1_bss_4;
static s32 lbl_1_bss_0;
static s32 lbl_1_data_0[4] = { 45, 30, 30, 15 };
static s32 lbl_1_data_10[4] = { 45, 30, 15, 15 };
static s32 lbl_1_data_20[4] = { 1, 1, 2, 0 };
static s32 lbl_1_data_30[4] = { 1, 2, 2, 0 };
static s32 lbl_1_data_40[4] = { 0, 0, 0, 0 };
static unsigned int lbl_1_data_50[9] = {
    M612_CHAR_MOTION_IDLE,       M612_CHAR_MOTION_MOVE,  M612_CHAR_MOTION_RESULT,
    M612_CHAR_MOTION_RESULT_ALT, M612_CHAR_MOTION_STEP,  M612_CHAR_MOTION_FALL,
    M612_CHAR_MOTION_RISE,       M612_CHAR_MOTION_EXTRA, 0
};
static s32 lbl_1_data_74[4] = { 1, 2, 4, 8 };
GXColor lbl_1_data_84[2] = { { 255, 255, 255, 0 }, { 208, 208, 208, 0 } };
static s32 lbl_1_data_8C = -1;
static MGSEQ_PARAM lbl_1_data_90 = {
    300, 0, fn_1_1694, fn_1_16D4, fn_1_1DBC, fn_1_1E18, fn_1_1FB4, fn_1_242C,
    fn_1_2BC8, fn_1_2C8C, fn_1_2C90,
};

/* Called by _prolog to create the Memory Lane scene and register its callbacks. */
void fn_1_A0(void)
{
    s16 lightId;
    s16 playerIndex;
    OMOBJ *view = NULL;
    s16 night;

    lbl_1_bss_4AC.flag = 0;
    lbl_1_bss_4AC.manager = omInitObjMan(80, 8192);
    omGameSysInit(lbl_1_bss_4AC.manager);
    night = GwMgNightF;
    lbl_1_bss_10 = night;
    fn_1_1430();
    fn_1_8F0();
    fn_1_F0C();
    playerIndex = 0;
    while (playerIndex < 4) {
        lbl_1_bss_42C[playerIndex] = 0;
        playerIndex += 1;
    }
    fn_1_48D4();
    view = omAddObjEx(lbl_1_bss_4AC.manager, 32730, 0U, 0U, -1, omOutViewMulti);
    view->work[0] = 4;
    MgSeqCreate(&lbl_1_data_90);
    playerIndex = 0;
    while (playerIndex < 4) {
        lbl_1_bss_49C[playerIndex] = (s32) GwPlayerConf[playerIndex].charNo;
        lbl_1_bss_43C[playerIndex] = (s32) GwPlayerConf[playerIndex].padNo;
        lbl_1_bss_48C[playerIndex] = (s32) CharModelMotListCreate(
            (s16) lbl_1_bss_49C[playerIndex], 4, lbl_1_data_50, lbl_1_bss_44C[playerIndex]);
        Hu3DModelPosSet((s16) lbl_1_bss_48C[playerIndex], 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet((s16) lbl_1_bss_48C[playerIndex], HU3D_MOTATTR_LOOP);
        CharMotionSet((s16) lbl_1_bss_49C[playerIndex], *lbl_1_bss_44C[playerIndex]);
        Hu3DModelCameraSet((s16) lbl_1_bss_48C[playerIndex], (u16) lbl_1_data_74[playerIndex]);
        lbl_1_bss_3FC[playerIndex] = -1;
        lbl_1_bss_30C[playerIndex] = 0;
        Hu3DModelAttrSet((s16) lbl_1_bss_48C[playerIndex], 1U);
        playerIndex += 1;
    }
    {
        HuVecF lightPos = { 0.0f, 1000.0f, 5000.0f };
        HuVecF lightDir = { 0.0f, 0.0f, -1.0f };
        /* The returned light ID is kept only in this local and is not reused. */
        lightId = CharLightCreateV(&lightPos, &lightDir, &lbl_1_data_84[lbl_1_bss_10]);
    }
    CharLightStaticSet(1);
    playerIndex = 0;
    while (playerIndex < 4) {
        lbl_1_bss_36C[playerIndex] = 0;
        if (playerIndex == 0) {
            lbl_1_bss_39C[playerIndex] = (s32) Hu3DModelCreate(
                HuDataSelHeapReadNum(M612_DATA_MARKER_MODEL, HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_bss_38C[playerIndex] = (s32) Hu3DJointMotion(
                (s16) *lbl_1_bss_39C,
                HuDataSelHeapReadNum(M612_DATA_MARKER_MOTION, HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            /* Later players pass the first player's marker model ID to Hu3DModelLink. */
            lbl_1_bss_39C[playerIndex] = (s32) Hu3DModelLink((s16) *lbl_1_bss_39C);
        }
        {
            HuVecF modelLightPos = { 0.0f, 1000.0f, 5000.0f };
            HuVecF modelLightDir = { 0.0f, 0.0f, -1.0f };
            Hu3DLLightCreateV((s16) lbl_1_bss_39C[playerIndex], &modelLightPos, &modelLightDir,
                              &lbl_1_data_84[lbl_1_bss_10]);
        }
        Hu3DMotionSet((s16) lbl_1_bss_39C[playerIndex], (s16) *lbl_1_bss_38C);
        Hu3DModelCameraSet((s16) lbl_1_bss_39C[playerIndex], (u16) lbl_1_data_74[playerIndex]);
        Hu3DModelAttrSet((s16) lbl_1_bss_39C[playerIndex], 1U);
        playerIndex += 1;
    }
    lbl_1_bss_300 = (s32) Hu3DModelCreate(
        HuDataSelHeapReadNum(M612_DATA_RESULT_MODEL, HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_2FC =
        (s32) Hu3DJointMotion((s16) lbl_1_bss_300, HuDataSelHeapReadNum(M612_DATA_RESULT_MOTION_0,
                                                                        HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_2F8 =
        (s32) Hu3DJointMotion((s16) lbl_1_bss_300, HuDataSelHeapReadNum(M612_DATA_RESULT_MOTION_1,
                                                                        HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_2F4 =
        (s32) Hu3DJointMotion((s16) lbl_1_bss_300, HuDataSelHeapReadNum(M612_DATA_RESULT_MOTION_2,
                                                                        HU_MEMNUM_OVL, HEAP_MODEL));
    {
        HuVecF resultLightPos = { 0.0f, 1000.0f, 5000.0f };
        HuVecF resultLightDir = { 0.0f, 0.0f, -1.0f };
        Hu3DLLightCreateV((s16) lbl_1_bss_300, &resultLightPos, &resultLightDir,
                          &lbl_1_data_84[lbl_1_bss_10]);
    }
    Hu3DModelPosSet((s16) lbl_1_bss_300, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet((s16) lbl_1_bss_300, HU3D_MOTATTR_LOOP);
    Hu3DMotionSet((s16) lbl_1_bss_300, (s16) lbl_1_bss_2FC);
    lbl_1_bss_308 = 0;
    lbl_1_bss_2F0 = 0;
    lbl_1_bss_304 = 0;
    CenterM->y = 1000.0f;
    CenterM->z = 400.0f;
    CRotM->x = 0.0f;
    lbl_1_bss_4AC.state = 0;
    lbl_1_bss_4AC.guide = omAddObjEx(lbl_1_bss_4AC.manager, 32730, 1U, 0U, -1, fn_1_5924);
    omAddObjEx(lbl_1_bss_4AC.manager, 32730, 1U, 0U, -1, fn_1_54D4);
}

/* Called during scene setup to load the day or night board models. */
void fn_1_8F0(void)
{
    if ((s32) lbl_1_bss_10 == 0) {
        lbl_1_bss_44 = (s32) Hu3DModelCreate(
            HuDataSelHeapReadNum(M612_DATA_DAY_BOARD_0, HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_40 = (s32) Hu3DModelCreate(
            HuDataSelHeapReadNum(M612_DATA_DAY_BOARD_1, HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3C = (s32) Hu3DModelCreate(
            HuDataSelHeapReadNum(M612_DATA_DAY_BOARD_6, HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_38 = (s32) Hu3DModelCreate(
            HuDataSelHeapReadNum(M612_DATA_DAY_BOARD_2, HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_34 = (s32) Hu3DModelCreate(
            HuDataSelHeapReadNum(M612_DATA_DAY_BOARD_3, HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_30 = (s32) Hu3DModelCreate(
            HuDataSelHeapReadNum(M612_DATA_DAY_BOARD_4, HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_2C = (s32) Hu3DModelCreate(
            HuDataSelHeapReadNum(M612_DATA_DAY_BOARD_5, HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_28 = -1;
        lbl_1_bss_20 = -1;
        lbl_1_bss_24 = -1;
    } else {
        lbl_1_bss_44 = (s32) Hu3DModelCreate(
            HuDataSelHeapReadNum(M612_DATA_NIGHT_BOARD_0, HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_40 = (s32) Hu3DModelCreate(
            HuDataSelHeapReadNum(M612_DATA_NIGHT_BOARD_1, HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_3C = (s32) Hu3DModelCreate(
            HuDataSelHeapReadNum(M612_DATA_NIGHT_BOARD_6, HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_38 = (s32) Hu3DModelCreate(
            HuDataSelHeapReadNum(M612_DATA_NIGHT_BOARD_2, HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_34 = (s32) Hu3DModelCreate(
            HuDataSelHeapReadNum(M612_DATA_NIGHT_BOARD_3, HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_30 = (s32) Hu3DModelCreate(
            HuDataSelHeapReadNum(M612_DATA_NIGHT_BOARD_4, HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_2C = (s32) Hu3DModelCreate(
            HuDataSelHeapReadNum(M612_DATA_NIGHT_BOARD_5, HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_20 = (s32) Hu3DModelCreate(
            HuDataSelHeapReadNum(M612_DATA_NIGHT_BOARD_7, HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelLayerSet((s16) lbl_1_bss_20, 7);
        Hu3DModelAttrSet((s16) lbl_1_bss_20, HU3D_MOTATTR_LOOP);
        Hu3DModelCameraSet((s16) lbl_1_bss_20, 15U);
        lbl_1_bss_28 = (s32) Hu3DModelCreate(
            HuDataSelHeapReadNum(M612_DATA_NIGHT_BOARD_8, HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelLayerSet((s16) lbl_1_bss_28, 7);
        Hu3DModelAttrSet((s16) lbl_1_bss_28, HU3D_MOTATTR_LOOP);
        Hu3DModelCameraSet((s16) lbl_1_bss_28, 15U);
        Hu3DModelAttrSet((s16) lbl_1_bss_28, 1U);
        lbl_1_bss_24 = (s32) Hu3DModelCreate(
            HuDataSelHeapReadNum(M612_DATA_NIGHT_BOARD_9, HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelLayerSet((s16) lbl_1_bss_24, 7);
        Hu3DModelAttrSet((s16) lbl_1_bss_24, HU3D_MOTATTR_LOOP);
        Hu3DModelCameraSet((s16) lbl_1_bss_24, 15U);
        Hu3DModelAttrSet((s16) lbl_1_bss_24, 1U);
    }
    Hu3DModelPosSet((s16) lbl_1_bss_44, 0.0f, 0.0f, 0.0f);
    Hu3DModelCameraSet((s16) lbl_1_bss_44, 15U);
    Hu3DModelLayerSet((s16) lbl_1_bss_44, 1);
    Hu3DModelAttrSet((s16) lbl_1_bss_44, HU3D_MOTATTR_LOOP);
    Hu3DModelCameraSet((s16) lbl_1_bss_40, 15U);
    Hu3DModelLayerSet((s16) lbl_1_bss_40, 1);
    Hu3DModelAttrSet((s16) lbl_1_bss_40, HU3D_MOTATTR_LOOP);
    Hu3DModelCameraSet((s16) lbl_1_bss_3C, 15U);
    Hu3DModelLayerSet((s16) lbl_1_bss_3C, 0);
    Hu3DModelAttrSet((s16) lbl_1_bss_3C, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet((s16) lbl_1_bss_3C, HU3D_MOTATTR_LOOP);
    Hu3DModelCameraSet((s16) lbl_1_bss_38, 15U);
    Hu3DModelLayerSet((s16) lbl_1_bss_38, 2);
    Hu3DModelAttrSet((s16) lbl_1_bss_38, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet((s16) lbl_1_bss_34, 6);
    Hu3DModelAttrSet((s16) lbl_1_bss_30, 1U);
    Hu3DModelLayerSet((s16) lbl_1_bss_30, 2);
    Hu3DModelAttrSet((s16) lbl_1_bss_2C, 1U);
    Hu3DModelLayerSet((s16) lbl_1_bss_2C, 3);
}

/* Called during scene setup to create the four tile grids, borders, and cursors. */
void fn_1_F0C(void)
{
    s32 unusedSetupValue;
    s32 player;
    s32 row;
    s32 column;

    if ((s32) lbl_1_bss_10 == 0) {
        lbl_1_bss_4AC.tiles[0][0] =
            Hu3DModelCreate(HuDataSelHeapReadNum(M612_DATA_DAY_TILE, HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        lbl_1_bss_4AC.tiles[0][0] =
            Hu3DModelCreate(HuDataSelHeapReadNum(M612_DATA_NIGHT_TILE, HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (player = 0; player < 4; player++) {
        for (row = 0; row < 7; row++) {
            for (column = 0; column < 7; column++) {
                if (((player == 0) & ((column == 0) & (row == 0))) == 0) {
                    lbl_1_bss_4AC.tiles[player][column + row * 7] =
                        Hu3DModelLink(lbl_1_bss_4AC.tiles[0][0]);
                }
                Hu3DModelAttrSet(lbl_1_bss_4AC.tiles[player][column + row * 7], HU3D_MOTATTR_PAUSE);
                Hu3DModelPosSet(lbl_1_bss_4AC.tiles[player][column + row * 7],
                                (float) (column * 200 - 600), 0.0f, (float) (row * 200 - 1400));
                Hu3DModelCameraSet(lbl_1_bss_4AC.tiles[player][column + row * 7],
                                   (u16) lbl_1_data_74[player]);
                Hu3DModelLayerSet(lbl_1_bss_4AC.tiles[player][column + row * 7], 5);
            }
        }
    }
    /* This local is cleared by the original setup but is never read. */
    unusedSetupValue = 0;
    lbl_1_bss_4AC.borders[0] = Hu3DModelLink(lbl_1_bss_4AC.tiles[0][0]);
    Hu3DModelPosSet(lbl_1_bss_4AC.borders[0], 0.0f, 0.0f, 0.0f);
    Hu3DModelLayerSet(lbl_1_bss_4AC.borders[0], 5);
    Hu3DModelAttrReset(lbl_1_bss_4AC.borders[0], HU3D_MOTATTR_PAUSE);
    lbl_1_bss_4AC.borders[1] = Hu3DModelLink(lbl_1_bss_4AC.tiles[0][0]);
    Hu3DModelPosSet(lbl_1_bss_4AC.borders[1], 0.0f, 0.0f, -1600.0f);
    Hu3DModelLayerSet(lbl_1_bss_4AC.borders[1], 5);
    Hu3DModelAttrReset(lbl_1_bss_4AC.borders[1], HU3D_MOTATTR_PAUSE);
    if ((s32) lbl_1_bss_10 == 0) {
        *lbl_1_bss_2C0 = (s32) Hu3DModelCreate(
            HuDataSelHeapReadNum(M612_DATA_DAY_CURSOR, HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        *lbl_1_bss_2C0 = (s32) Hu3DModelCreate(
            HuDataSelHeapReadNum(M612_DATA_NIGHT_CURSOR, HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (player = 0; player < 4; player++) {
        if (player != 0) {
            lbl_1_bss_2C0[player] = (s32) Hu3DModelLink((s16) *lbl_1_bss_2C0);
        }
        Hu3DModelAttrSet((s16) lbl_1_bss_2C0[player], 1U);
        Hu3DModelAttrSet((s16) lbl_1_bss_2C0[player], HU3D_MOTATTR_PAUSE);
        Hu3DModelCameraSet((s16) lbl_1_bss_2C0[player], (u16) lbl_1_data_74[player]);
        Hu3DModelLayerSet((s16) lbl_1_bss_2C0[player], 4);
    }
}

/* No current Memory Lane callback calls this helper; it tests whether the selected pattern marks a
 * board cell. */
s32 fn_1_13A4(s32 column, s32 row)
{
    s32 bitIndex;
    s8 occupancyMask;
    s32 cellMask;

    occupancyMask = lbl_1_data_16B0[lbl_1_bss_8][row];
    if (lbl_1_bss_C != 0) {
        bitIndex = column;
    } else {
        bitIndex = 6 - column;
    }
    cellMask = (1 << bitIndex);
    if ((cellMask & occupancyMask) != 0) {
        return 1;
    }
    return 0;
}
/* Called by scene setup to choose a path and reset each player's cursor. */
void fn_1_1430(void)
{
    s32 index;

    lbl_1_bss_8 = rand8() % 8;
    lbl_1_bss_4 = lbl_1_bss_8 * 9 + rand8() % 4;
    lbl_1_bss_C = rand8() & 0x1;
    for (index = 0; lbl_1_data_198[lbl_1_bss_4][index] != 0; index++) {
        /* Mirroring the path swaps left and right while preserving vertical moves. */
        if ((s32) lbl_1_bss_C != 0) {
            switch (lbl_1_data_198[lbl_1_bss_4][index]) {
            case 1:
                lbl_1_bss_48[index] = 2;
                break;
            case 2:
                lbl_1_bss_48[index] = 1;
                break;
            default:
                lbl_1_bss_48[index] = lbl_1_data_198[lbl_1_bss_4][index];
                break;
            }
        } else {
            lbl_1_bss_48[index] = lbl_1_data_198[lbl_1_bss_4][index];
        }
    }
    lbl_1_bss_48[index] = 0;
    /* Start each cursor at column 3, row 7, just below the seven-row grid. */
    for (index = 0; index < 4; index++) {
        lbl_1_bss_3BC[index] = (lbl_1_bss_3BC[index] & 0xF) | 0x30;
        lbl_1_bss_3BC[index] = (lbl_1_bss_3BC[index] & 0xF0) | 0x7;
    }
}
/* Registered as initHook: clear sequence counters and request the next mode. */
void fn_1_1694(s16 mode, s16 frameNo)
{
    lbl_1_bss_14 = 0;
    lbl_1_bss_18 = 0;
    MgSeqModeNext();
}

/* Registered as fadeInHook: reveal the board and enter player selection. */
void fn_1_16D4(s16 mode, s16 frameNo)
{
    s32 playerIndex;
    s32 column;
    s32 row;
    s32 hideFrame;
    s32 revealFrame;

        switch (lbl_1_bss_4AC.state) {
        case 0:
            MgSeqModeChangeOff();
            lbl_1_bss_2EC = 0.0f;
            lbl_1_bss_2E8 = 0.0f;
            lbl_1_bss_14 = 0;
            lbl_1_bss_4AC.state = 1;
            HuAudFXPlay(M612_SE_GUIDE_MOVE);
            return;
        case 1:
            lbl_1_bss_2EC = 0.016666668f * (f32) lbl_1_bss_14;
            CenterM->z = (1000.0f * lbl_1_bss_2EC) - 600.0f;
            lbl_1_bss_14 += 1;
            if ((s32) lbl_1_bss_14 == 60) {
                lbl_1_bss_14 = 0;
                lbl_1_bss_4AC.state = 2;
                return;
            }
            break;
        case 2:
            lbl_1_bss_2E8 = 0.016666668f * (f32) lbl_1_bss_14;
            CRotM->x = -40.0f * lbl_1_bss_2E8;
            lbl_1_bss_14 += 1;
            if ((s32) lbl_1_bss_14 == 60) {
                lbl_1_bss_14 = 0;
                lbl_1_bss_308 = 2;
                lbl_1_bss_4AC.state = 3;
                return;
            }
            break;
        case 3:
            if ((s32) (lbl_1_bss_14 % 120) == 0) {
                HuAudFXPlay(M612_SE_GUIDE_MOVE);
            }
            lbl_1_bss_14 += 1;
            if ((s32) lbl_1_bss_308 == 6) {
                lbl_1_bss_4AC.state = 4;
                lbl_1_bss_14 = 0;
                return;
            }
            break;
        case 4:
            lbl_1_bss_14 += 1;
            if ((s32) lbl_1_bss_14 > 60) {
                lbl_1_bss_14 = 0;
                lbl_1_bss_4AC.state = 5;
                return;
            }
            break;
        case 5:
            WipeCreate(2, 0, 60);
            hideFrame = 0;
            while (hideFrame < 60) {
                HuPrcVSleep();
                hideFrame += 1;
            }
            for (row = 0; row < 7; row++) {
                for (column = 0; column < 7; column++) {
                    Hu3DModelAttrSet(lbl_1_bss_4AC.tiles[0][column + (row * 7)],
                                     HU3D_MOTATTR_PAUSE);
                    Hu3DMotionTimeSet(lbl_1_bss_4AC.tiles[0][column + (row * 7)], 0.0f);
                }
            }
            Hu3DModelAttrSet((s16) lbl_1_bss_300, 1U);
            Hu3DModelAttrSet((s16) lbl_1_bss_38, 1U);
            Hu3DModelAttrReset((s16) lbl_1_bss_30, 1U);
            Hu3DModelAttrReset((s16) lbl_1_bss_2C, 1U);
            if ((s32) lbl_1_bss_10 == 1) {
                Hu3DModelAttrSet((s16) lbl_1_bss_20, 1U);
                Hu3DModelAttrReset((s16) lbl_1_bss_28, 1U);
                Hu3DModelAttrReset((s16) lbl_1_bss_24, 1U);
            }
            lbl_1_bss_4AC.players = omAddObjEx(lbl_1_bss_4AC.manager, 32730, 1U, 0U, -1, fn_1_2C94);
            lbl_1_bss_4AC.state = 6;
            return;
        case 6:
            lbl_1_bss_14 = 60;
            Hu3DModelAttrSet((s16) lbl_1_bss_40, 1U);
            fn_1_51D8();
            lbl_1_bss_4AC.state = 8;
            playerIndex = 0;
            while (playerIndex < 4) {
                Hu3DModelAttrReset((s16) lbl_1_bss_48C[playerIndex], 1U);
                playerIndex += 1;
            }
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                CenterM[playerIndex].x = 0.0f;
                CenterM[playerIndex].y = 150.0f;
                CenterM[playerIndex].z = 0.0f;
                CRotM[playerIndex].x = -40.0f;
                CRotM[playerIndex].y = 0.0f;
                CRotM[playerIndex].z = 0.0f;
                CZoomM[playerIndex] = 800.0f;
            }
            WipeCreate(1, 5, 60);
            revealFrame = 0;
            while (revealFrame < 60) {
                HuPrcVSleep();
                revealFrame += 1;
            }
            return;
        case 7:
            lbl_1_bss_14 -= 1;
            fn_1_4B94(0);
            if ((s32) lbl_1_bss_14 == 0) {
                lbl_1_bss_4AC.state = 8;
                return;
            }
            break;
        case 8:
            MgSeqModeNext();
            lbl_1_bss_4AC.state = 9;
            break;
        }
}

/* Sequence start hook: start the result music once the current message begins sound playback. */
void fn_1_1DBC(s16 mode, s16 frameNo)
{
    if (((s32) lbl_1_data_8C == -1) && ((s32) (GameMesStatGet(MgSeqGameMesIdGet()) & 0x10) != 0)) {
        lbl_1_data_8C = HuAudBGMPlay(M612_RESULT_BGM);
    }
}

/* Registered in the sequence callback table; randomly choose among players that reach the
 * finish. */
void fn_1_1E18(s16 mode, s16 frameNo)
{
    s32 completedPlayers[4];
    s32 completedPlayerCount;
    s32 playerIndex;

    switch ((s32) lbl_1_bss_4AC.state) {
    case 9:
        lbl_1_bss_0 = -1;
        lbl_1_bss_4AC.state = 10;
        /* Check for completed players immediately after resetting the winner. */

    case 10:
        playerIndex = 0;
        completedPlayerCount = 0;
        for (; playerIndex < 4; playerIndex++) {
            if ((s32) lbl_1_bss_42C[playerIndex] == 11) {
                completedPlayers[completedPlayerCount] = playerIndex;
                completedPlayerCount++;
            }
        }
        if (completedPlayerCount != 0) {
            if (completedPlayerCount != 1) {
                lbl_1_bss_0 = completedPlayers[rand8() % completedPlayerCount];
            } else {
                lbl_1_bss_0 = completedPlayers[0];
            }
            lbl_1_bss_14 = 0;
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                if ((s32) lbl_1_bss_42C[playerIndex] == 2) {
                    lbl_1_bss_42C[playerIndex] = 1;
                    CharMotionSet((s16) lbl_1_bss_49C[playerIndex], lbl_1_bss_44C[playerIndex][0]);
                }
            }
            lbl_1_bss_4AC.state = 12;
            MgSeqModeNext();
        }
        break;
    }
}

/* Registered in the sequence callback table; restore player motions and record the winner for
 * results. */
void fn_1_1FB4(s16 mode, s16 frameNo)
{
    s16 winnerCharacterByPlayer[4];
    s16 playerIndex;
    s16 restoredPlayerCount;
    s32 winningPlayer;

    if ((MgSeqFrameNoGet() == 0) && ((s32) lbl_1_data_8C != -1)) {
        HuAudSStreamFadeOut(lbl_1_data_8C, 100);
        lbl_1_data_8C = -1;
    }
    switch ((s32) lbl_1_bss_4AC.state) {
    case 10:
        playerIndex = 0;
        while (playerIndex < 4) {
            (&winnerCharacterByPlayer[0])[playerIndex] = -1;
            playerIndex += 1;
        }
        MgSeqWinnerSet(winnerCharacterByPlayer[0], winnerCharacterByPlayer[1],
                       winnerCharacterByPlayer[2], winnerCharacterByPlayer[3]);
        lbl_1_bss_14 = 0;
        playerIndex = 0;
        while (playerIndex < 4) {
            if (((s32) lbl_1_bss_42C[playerIndex] == 2) ||
                ((s32) lbl_1_bss_42C[playerIndex] == 3)) {
                CharMotionShiftSet((s16) lbl_1_bss_49C[playerIndex], lbl_1_bss_44C[playerIndex][0],
                                   0.0f, 5.0f, 0U);
                lbl_1_bss_42C[playerIndex] = 1;
            }
            playerIndex += 1;
        }
        lbl_1_bss_4AC.state = 18;
        break;
    case 11:
        playerIndex = 0;
        restoredPlayerCount = 0;
        while (playerIndex < 4) {
            if ((s32) lbl_1_bss_42C[playerIndex] == 1) {
                restoredPlayerCount += 1;
            }
            playerIndex += 1;
        }
        if (restoredPlayerCount == 4) {
            playerIndex = 0;
            while (playerIndex < 4) {
                CharMotionSet((s16) lbl_1_bss_49C[playerIndex], lbl_1_bss_44C[playerIndex][3]);
                Hu3DModelRotSet((s16) lbl_1_bss_48C[playerIndex], 0.0f, 0.0f, 0.0f);
                Hu3DModelAttrReset((s16) lbl_1_bss_48C[playerIndex], HU3D_MOTATTR_LOOP);
                playerIndex += 1;
            }
            lbl_1_bss_4AC.state = 19;
        }
        break;
    case 12:
        lbl_1_bss_14 = 0;
        playerIndex = 0;
        while (playerIndex < 4) {
            winnerCharacterByPlayer[playerIndex] = -1;
            if (((s32) lbl_1_bss_42C[playerIndex] == 2) ||
                ((s32) lbl_1_bss_42C[playerIndex] == 3)) {
                CharMotionShiftSet((s16) lbl_1_bss_49C[playerIndex], lbl_1_bss_44C[playerIndex][0],
                                   0.0f, 10.0f, HU3D_MOTATTR_LOOP);
                lbl_1_bss_42C[playerIndex] = 1;
            }
            playerIndex += 1;
        }
        (&winnerCharacterByPlayer[0])[lbl_1_bss_0] = (s16) lbl_1_bss_49C[lbl_1_bss_0];
        MgSeqWinnerSet(winnerCharacterByPlayer[0], winnerCharacterByPlayer[1],
                       winnerCharacterByPlayer[2], winnerCharacterByPlayer[3]);
        lbl_1_bss_4AC.state = 13;
        winningPlayer = lbl_1_bss_0;
        /* Practice mode suppresses the ten-coin winner bonus. */
        if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
            GwPlayer[winningPlayer].mgCoinBonus = 10;
        }
        break;
    }
}

/* Sequence callback: reveal the winner, switch to the result camera, then advance. */
void fn_1_242C(s16 mode, s16 frameNo)
{
    s32 transitionFrame;
    s32 playerIndex;

    switch (lbl_1_bss_4AC.state) {
    case 13:
        lbl_1_bss_14 = 0;
        lbl_1_bss_4AC.state = 14;
        /* Begin the timed transition on this same callback. */
    case 14:
        if ((s32) lbl_1_bss_14 >= 60) {
            lbl_1_bss_4AC.state = 15;
        }
        lbl_1_bss_14 += 1;
        return;
    case 15:
        lbl_1_bss_14 = 0;
        Hu3DModelAttrReset((s16) lbl_1_bss_40, 1U);
        if ((s32) lbl_1_bss_10 == 1) {
            Hu3DModelAttrSet((s16) lbl_1_bss_28, 1U);
            Hu3DModelAttrSet((s16) lbl_1_bss_24, 1U);
            Hu3DModelAttrReset((s16) lbl_1_bss_20, 1U);
        }
        WipeCreate(2, 0, 60);
        transitionFrame = 0;
        while (transitionFrame < 60) {
            lbl_1_bss_14 += 1;
            if ((s32) lbl_1_bss_14 <= 60) {
                fn_1_4B94(lbl_1_bss_0);
            }
            HuPrcVSleep();
            transitionFrame += 1;
        }
        lbl_1_bss_14 = 0;
        Hu3DModelAttrSet((s16) lbl_1_bss_30, 1U);
        Hu3DModelAttrSet((s16) lbl_1_bss_2C, 1U);
        Hu3DModelAttrReset((s16) lbl_1_bss_38, 1U);
        {
        HuVecF lightPos = { 0.0f, 3000.0f, 3000.0f };
        HuVecF lightDir = { 0.0f, 0.0f, -1.0f };
        HuVecF modelPos;
        CharLightCreateV(&lightPos, &lightDir, &lbl_1_data_84[lbl_1_bss_10]);
        CharLightStaticSet(1);
        Hu3DCameraKill(-2);
        modelPos.x = -130.0f;
        modelPos.y = 1519.668f;
        modelPos.z = -2800.8f;
        Hu3DModelCameraSet((s16) lbl_1_bss_48C[lbl_1_bss_0], 1U);
        Hu3DModelPosSetV((s16) lbl_1_bss_48C[lbl_1_bss_0], &modelPos);
        CenterM->x = 0.0f;
        CenterM->y = 1800.0f;
        CenterM->z = -2500.0f;
        CRotM->x = -20.0f;
        Hu3DCameraPerspectiveSet(1, 45.0f, 20.0f, 15000.0f, 1.2f);
        Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
        Hu3DCameraScissorSet(1, 0U, 0U, 640U, 480U);
        Hu3DModelAttrReset((s16) lbl_1_bss_48C[lbl_1_bss_0], HU3D_MOTATTR_LOOP);
        /* Keep the winner model's y and z, changing only x for the result model. */
        modelPos.x = 150.0f;
        Hu3DModelPosSetV((s16) lbl_1_bss_300, &modelPos);
        Hu3DModelAttrReset((s16) lbl_1_bss_300, 1U);
        Hu3DModelRotSet((s16) lbl_1_bss_300, 0.0f, 0.0f, 0.0f);
        Hu3DMotionSet((s16) lbl_1_bss_300, (s16) lbl_1_bss_2F4);
        Hu3DModelAttrReset((s16) lbl_1_bss_3C, HU3D_MOTATTR_PAUSE);
        Hu3DModelAttrSet((s16) lbl_1_bss_3C, HU3D_MOTATTR_LOOP);
        {
        HuVecF shadowPos = { 0.0f, 5000.0f, -2000.0f };
        HuVecF shadowUp = { 0.0f, 1.0f, 0.0f };
        HuVecF shadowTarget = { 0.0f, 0.0f, -2600.0f };
        Hu3DShadowCreate(30.0f, 20.0f, 5000.0f);
        Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);
        }
        if ((s32) lbl_1_bss_10 == 0) {
            Hu3DShadowTPLvlSet(0.7f);
            Hu3DShadowColSet(128U, 128U, 128U);
        } else {
            Hu3DShadowTPLvlSet(0.3f);
            Hu3DShadowColSet(16U, 16U, 16U);
        }
        Hu3DModelShadowMapSet((s16) lbl_1_bss_40);
        Hu3DModelShadowSet((s16) lbl_1_bss_300);
        Hu3DModelShadowSet((s16) lbl_1_bss_48C[lbl_1_bss_0]);
        lbl_1_bss_4AC.state = 16;
        return;
        }
    case 16:
        WipeCreate(1, 5, 60);
        transitionFrame = 0;
        while (transitionFrame < 60) {
            HuPrcVSleep();
            transitionFrame += 1;
        }
        lbl_1_bss_14 = 0;
        lbl_1_bss_4AC.state = 17;
        return;
    case 17:
        lbl_1_bss_14 = 0;
        MgSeqModeNext();
        return;
    case 18:
        if ((s32) lbl_1_bss_14 >= 60) {
            playerIndex = 0;
            while (playerIndex < 4) {
                CharMotionSet((s16) lbl_1_bss_49C[playerIndex], lbl_1_bss_44C[playerIndex][3]);
                Hu3DModelRotSet((s16) lbl_1_bss_48C[playerIndex], 0.0f, 0.0f, 0.0f);
                Hu3DModelAttrReset((s16) lbl_1_bss_48C[playerIndex], HU3D_MOTATTR_LOOP);
                playerIndex += 1;
            }
            MgSeqModeNext();
        }
        lbl_1_bss_14 += 1;
        /* State 19 continues to wait while the sequence timer advances. */
    case 19:
    default:
        break;
    }
}

/* Registered in the sequence callback table; play the selected winner's motion. */
void fn_1_2BC8(s16 mode, s16 frameNo)
{
    if ((s32) lbl_1_bss_0 != -1) {
        if ((s32) lbl_1_bss_14 == 0) {
            CharMotionShiftSet((s16) lbl_1_bss_49C[lbl_1_bss_0], lbl_1_bss_44C[lbl_1_bss_0][2],
                               0.0f, 10.0f, 0U);
        }
        lbl_1_bss_14 += 1;
    }
}

void fn_1_2C8C(s16 mode, s16 frameNo)
{

}

void fn_1_2C90(s16 mode, s16 frameNo)
{

}

/* Object-manager callback added during the board transition; advance each player's action state
 * except while the scene is in state 9. */
void fn_1_2C94(OMOBJ *obj)
{
    s32 playerIndex;

    playerIndex = 0;
    while (playerIndex < 4) {
        switch (lbl_1_bss_4AC.state) {
            case 9:
                break;
            default:
                fn_1_2CEC(playerIndex);
                break;
        }
        playerIndex += 1;
    }
}

/* Called once per frame by fn_1_2C94 to advance this player's board action. */
void fn_1_2CEC(s32 player)
{
    Point3d movePos;
    Point3d effectPos;
    Point3d fallPos;
    Point3d risePos;
    Point3d bouncePos;
    f32 angle;
    f32 fallTime;
    s32 nextColumn;
    s32 nextRow;
    s32 effectX;
    s32 effectY;
    s32 tentativeX;
    s32 tentativeY;
    s32 actionResult;

    switch (lbl_1_bss_42C[player]) {
    case 0:
        lbl_1_bss_3BC[player] = 55;
        lbl_1_bss_42C[player] = 1;
        lbl_1_bss_3AC[player] = 0;
        lbl_1_bss_3CC[player] = 0;
        break;
    case 1:
    case 3:
        if (lbl_1_bss_4AC.state != 10) {
            break;
        }
        if (GwPlayerConf[player].type == 0) {
            lbl_1_bss_41C[player] = fn_1_4038(player);
        } else {
            lbl_1_bss_41C[player] = fn_1_4480(player);
        }
        if (lbl_1_bss_41C[player] != 0) {
            tentativeX = (lbl_1_bss_3BC[player] & 0xF0) >> 4;
            tentativeY = lbl_1_bss_3BC[player] & 0xF;
            angle = fn_1_5E00(lbl_1_bss_41C[player]);
            switch (lbl_1_bss_41C[player]) {
            case 2:
                tentativeX++;
                break;
            case 1:
                tentativeX--;
                break;
            case 4:
                tentativeY++;
                break;
            case 3:
                tentativeY--;
                break;
            }
            if (fn_1_4150(player) != 0) {
                if (lbl_1_bss_42C[player] == 1) {
                    CharMotionShiftSet((s16) lbl_1_bss_49C[player], lbl_1_bss_44C[player][1], 0.0f,
                                       10.0f, HU3D_MOTATTR_LOOP);
                }
                lbl_1_bss_40C[player] = 20;
                lbl_1_bss_42C[player] = 2;
                Hu3DModelRotSet((s16) lbl_1_bss_48C[player], 0.0f, angle, 0.0f);
            } else {
                Hu3DModelRotSet((s16) lbl_1_bss_48C[player], 0.0f, angle, 0.0f);
                if (lbl_1_bss_44C[player][0] != CharMotionShiftIDGet((s16) lbl_1_bss_49C[player])) {
                    CharMotionShiftSet((s16) lbl_1_bss_49C[player], lbl_1_bss_44C[player][0], 0.0f,
                                       5.0f, 0U);
                    lbl_1_bss_42C[player] = 1;
                }
                return;
            }
        } else {
            if (lbl_1_bss_42C[player] == 3) {
                CharMotionSet((s16) lbl_1_bss_49C[player], lbl_1_bss_44C[player][0]);
                lbl_1_bss_42C[player] = 1;
            }
            return;
        }
        /* A newly accepted move performs the first case-2 update immediately. */
    case 2:
        lbl_1_bss_40C[player]--;
        if (lbl_1_bss_40C[player] >= 0) {
            Hu3DModelPosGet((s16) lbl_1_bss_48C[player], &movePos);
            switch (lbl_1_bss_41C[player]) {
            case 1:
                movePos.x -= 10.0f;
                CenterM[player].x -= 10.0f;
                break;
            case 2:
                movePos.x += 10.0f;
                CenterM[player].x += 10.0f;
                break;
            case 3:
                movePos.z -= 10.0f;
                CenterM[player].z -= 10.0f;
                break;
            case 4:
                movePos.z += 10.0f;
                CenterM[player].z += 10.0f;
                break;
            }
            Hu3DModelPosSetV((s16) lbl_1_bss_48C[player], &movePos);
        }
        if (lbl_1_bss_40C[player] == 0) {
            actionResult = fn_1_42C4(player);
            if (actionResult == 2) {
                nextColumn = (lbl_1_bss_3BC[player] & 0xF0) >> 4;
                nextRow = lbl_1_bss_3BC[player] & 0xF;
                switch (lbl_1_bss_41C[player]) {
                case 2:
                    nextColumn++;
                    break;
                case 1:
                    nextColumn--;
                    break;
                case 4:
                    nextRow++;
                    break;
                case 3:
                    nextRow--;
                    break;
                }
                if (nextColumn >= 0 && nextColumn < 7 && nextRow >= 0 && nextRow < 7 &&
                    (Hu3DModelAttrGet(lbl_1_bss_4AC.tiles[player][nextColumn + nextRow * 7]) &
                     HU3D_ATTR_DISPOFF) == 0) {
                    effectX = (lbl_1_bss_3BC[player] & 0xF0) >> 4;
                    effectY = lbl_1_bss_3BC[player] & 0xF;
                    switch (lbl_1_bss_41C[player]) {
                    case 2:
                        effectX++;
                        break;
                    case 1:
                        effectX--;
                        break;
                    case 3:
                        effectY--;
                        break;
                    case 4:
                        effectY++;
                        break;
                    }
                    effectPos.x = effectX * 200 - 600;
                    effectPos.y = 0.0f;
                    effectPos.z = effectY * 200 - 1400;
                    Hu3DModelPosSetV((s16) lbl_1_bss_2C0[player], &effectPos);
                    Hu3DModelAttrReset((s16) lbl_1_bss_2C0[player], 1U);
                    Hu3DModelAttrReset((s16) lbl_1_bss_2C0[player], HU3D_MOTATTR_PAUSE);
                    Hu3DModelAttrSet(lbl_1_bss_4AC.tiles[player][effectX + effectY * 7], 1U);
                    lbl_1_bss_30C[player] = 0;
                    CharMotionShiftSet((s16) lbl_1_bss_49C[player], lbl_1_bss_44C[player][4], 0.0f,
                                       10.0f, 0U);
                    lbl_1_bss_42C[player] = 4;
                    HuAudFXPlay(M612_SE_TILE_MARK);
                    lbl_1_bss_1C = 1;
                } else {
                    CharMotionShiftSet((s16) lbl_1_bss_49C[player], lbl_1_bss_44C[player][5], 0.0f,
                                       10.0f, 0U);
                    omVibrate((s16) player, 20, 7, 3);
                    lbl_1_bss_42C[player] = 5;
                    lbl_1_bss_1C = 0;
                }
                lbl_1_bss_3EC[player] = 0;
                break;
            }
            nextColumn = (lbl_1_bss_3BC[player] & 0xF0) >> 4;
            nextRow = lbl_1_bss_3BC[player] & 0xF;
            switch (lbl_1_bss_41C[player]) {
            case 2:
                nextColumn++;
                break;
            case 1:
                nextColumn--;
                break;
            case 4:
                nextRow++;
                break;
            case 3:
                nextRow--;
                break;
            }
            lbl_1_bss_3BC[player] = (nextColumn << 4) | (nextRow & 0xF);
            if (actionResult == 3) {
                Hu3DModelRotSet((s16) lbl_1_bss_48C[player], 0.0f, 0.0f, 0.0f);
                CharMotionSet((s16) lbl_1_bss_49C[player], lbl_1_bss_44C[player][0]);
                lbl_1_bss_42C[player] = 11;
                break;
            }
            if (nextColumn >= 0 && nextColumn < 7 && nextRow >= 0 && nextRow < 7) {
                Hu3DModelAttrReset(lbl_1_bss_4AC.tiles[player][nextColumn + nextRow * 7],
                                   HU3D_MOTATTR_PAUSE);
            }
            if (actionResult == 0) {
                lbl_1_bss_3AC[player]++;
            } else {
                lbl_1_bss_3AC[player]--;
            }
            lbl_1_bss_42C[player] = 3;
            lbl_1_bss_3CC[player]++;
        }
        break;
    case 4:
        if (lbl_1_bss_30C[player] > 40) {
            omVibrate((s16) player, 20, 7, 3);
            CharMotionShiftSet((s16) lbl_1_bss_49C[player], lbl_1_bss_44C[player][5], 0.0f, 10.0f,
                               0U);
            lbl_1_bss_42C[player] = 5;
        }
        lbl_1_bss_30C[player]++;
        break;
    case 5:
        fallTime = 0.0f;
        Hu3DModelPosGet((s16) lbl_1_bss_48C[player], &fallPos);
        fallTime = 0.033333335f * (f32) lbl_1_bss_3EC[player];
        fallPos.y = -(800.0f * fallTime);
        Hu3DModelPosSetV((s16) lbl_1_bss_48C[player], &fallPos);
        if (lbl_1_bss_3EC[player] == 0 && lbl_1_bss_1C == 1) {
            HuAudFXPlay(M612_SE_PLAYER_FALL);
        }
        lbl_1_bss_3EC[player]++;
        if (lbl_1_bss_3EC[player] == 30) {
            lbl_1_bss_36C[player] = 2;
            lbl_1_bss_3EC[player] = 0;
            lbl_1_bss_42C[player] = 6;
            Hu3DModelAttrSet((s16) lbl_1_bss_48C[player], 1U);
        }
        lbl_1_bss_3CC[player] = 0;
        break;
    case 6:
        if (lbl_1_bss_36C[player] == 5) {
            Hu3DModelAttrSet((s16) lbl_1_bss_2C0[player], 1U);
            Hu3DModelAttrSet((s16) lbl_1_bss_2C0[player], HU3D_MOTATTR_PAUSE);
            Hu3DMotionTimeSet((s16) lbl_1_bss_2C0[player], 0.0f);
            CharMotionSet((s16) lbl_1_bss_49C[player], lbl_1_bss_44C[player][6]);
            lbl_1_bss_3EC[player] = 0;
            lbl_1_bss_42C[player] = 7;
            Hu3DModelAttrReset((s16) lbl_1_bss_48C[player], 1U);
        }
        break;
    case 7:
        Hu3DModelPosGet((s16) lbl_1_bss_39C[player], &risePos);
        risePos.y -= 300.0f;
        if (risePos.y > 0.0f) {
            risePos.y = 0.0f;
        }
        Hu3DModelPosSetV((s16) lbl_1_bss_48C[player], &risePos);
        if (risePos.y >= 0.0f) {
            lbl_1_bss_40C[player] = 20;
            lbl_1_bss_42C[player] = 8;
            lbl_1_bss_3DC[player] = 30.0f;
            lbl_1_bss_3EC[player] = 0;
        }
        break;
    case 8:
        lbl_1_bss_40C[player]--;
        if (lbl_1_bss_40C[player] >= 0) {
            Hu3DModelPosGet((s16) lbl_1_bss_48C[player], &bouncePos);
            lbl_1_bss_3DC[player] -= 3.0f;
            bouncePos.y += lbl_1_bss_3DC[player];
            if (bouncePos.y < 0.0f) {
                bouncePos.y = 0.0f;
            }
            switch (lbl_1_bss_41C[player]) {
            case 1:
                bouncePos.x += 10.0f;
                CenterM[player].x += 10.0f;
                break;
            case 2:
                bouncePos.x -= 10.0f;
                CenterM[player].x -= 10.0f;
                break;
            case 3:
                bouncePos.z += 10.0f;
                CenterM[player].z += 10.0f;
                break;
            case 4:
                bouncePos.z -= 10.0f;
                CenterM[player].z -= 10.0f;
                break;
            }
            Hu3DModelPosSetV((s16) lbl_1_bss_48C[player], &bouncePos);
        } else {
            CharMotionShiftSet((s16) lbl_1_bss_49C[player], lbl_1_bss_44C[player][0], 0.0f, 10.0f,
                               0U);
            lbl_1_bss_42C[player] = 1;
        }
        break;
    case 10:
        lbl_1_bss_40C[player]--;
        if (lbl_1_bss_40C[player] == 0) {
            lbl_1_bss_42C[player] = 1;
        }
        break;
    case 9:
    case 11:
    default:
        break;
    }
}

/* Return a route direction from the player's stick, using a 30-unit deadzone and preferring
 * horizontal input when both axes are active. */
s32 fn_1_4038(s32 player)
{
    s32 moveDirection;
    s16 stickX;
    s16 stickY;

    moveDirection = 0;
    stickX = HuPadStkX[lbl_1_bss_43C[player]];
    stickY = HuPadStkY[lbl_1_bss_43C[player]];
    if (((s16) stickX < 30) && ((s16) stickX > -30)) {
        stickX = 0;
    }
    if (((s16) stickY < 30) && ((s16) stickY > -30)) {
        stickY = 0;
    }
    if (((s16) stickX != 0) || ((s16) stickY != 0)) {
        if ((s16) stickX > 0) {
            moveDirection = 2;
        } else if ((s16) stickX < 0) {
            moveDirection = 1;
        } else if ((s16) stickY < 0) {
            moveDirection = 4;
        } else if ((s16) stickY > 0) {
            moveDirection = 3;
        }
    }
    return moveDirection;
}

/* Allow only an initial upward move, then reject destinations marked occupied by the mirrored row
 * mask. */
s32 fn_1_4150(s32 player)
{
    s32 column;
    s32 row;
    s32 isOccupied;
    s32 bitIndex;
    s32 cellMask;
    s8 rowOccupancyMask;

    if (((s32) lbl_1_bss_3AC[player] == 0) && ((s32) lbl_1_bss_41C[player] != 3)) {
        return 0;
    }
    column = (s32) (lbl_1_bss_3BC[player] & 0xF0) >> 4;
    row = lbl_1_bss_3BC[player] & 0xF;
    switch (lbl_1_bss_41C[player]) {
    case 2:
        column += 1;
        break;
    case 1:
        column -= 1;
        break;
    case 4:
        row += 1;
        break;
    case 3:
        row -= 1;
        break;
    }
    rowOccupancyMask = lbl_1_data_16B0[lbl_1_bss_8][row];
    /* Mirroring reverses the row's bit order, so the column maps differently. */
    if ((s32) lbl_1_bss_C != 0) {
        bitIndex = column;
    } else {
        bitIndex = 6 - column;
    }
    cellMask = 1 << bitIndex;
    if ((cellMask & rowOccupancyMask) != 0) {
        isOccupied = 1;
    } else {
        isOccupied = 0;
    }
    if (isOccupied != 0) {
        return 0;
    }
    return 1;
}

/* Classify the step as following the route, reversing, choosing another direction, or finishing the
 * route. */
s32 fn_1_42C4(s32 player)
{
    if ((s32) lbl_1_bss_41C[player] == (s32) lbl_1_bss_48[lbl_1_bss_3AC[player]]) {
        if ((s32) lbl_1_bss_48[lbl_1_bss_3AC[player] + 1] == 0) {
            return 3;
        }
        return 0;
    }
    switch (lbl_1_bss_41C[player]) {
    case 2:
        if ((s32) lbl_1_bss_48[lbl_1_bss_3AC[player] - 1] == 1) {
            return 1;
        }
        break;
    case 1:
        if ((s32) lbl_1_bss_48[lbl_1_bss_3AC[player] - 1] == 2) {
            return 1;
        }
        break;
    case 4:
        if ((s32) lbl_1_bss_48[lbl_1_bss_3AC[player] - 1] == 3) {
            return 1;
        }
        break;
    case 3:
        if ((s32) lbl_1_bss_48[lbl_1_bss_3AC[player] - 1] == 4) {
            return 1;
        }
        break;
    }
    return 2;
}

/* Choose the computer player's next direction; difficulty settings can trigger a random detour that
 * avoids the immediate reverse, out-of-bounds cells, and already marked cells. */
s32 fn_1_4480(s32 player)
{
    s32 candidateColumn;
    s32 candidateRow;
    s32 difficulty;
    s32 candidateDirection;
    s32 chosenDirection;

    difficulty = GwPlayerConf[player].comDif;
    chosenDirection = lbl_1_bss_48[lbl_1_bss_3AC[player]];
    if ((s32) lbl_1_bss_3FC[player] > 0) {
        lbl_1_bss_3FC[player]--;
    }
    if ((s32) lbl_1_bss_3AC[player] != 0) {
        if (((s32) lbl_1_bss_48[lbl_1_bss_3AC[player]] ==
             (s32) lbl_1_bss_48[lbl_1_bss_3AC[player] - 1]) &&
            (((s32) lbl_1_data_30[difficulty] == 0) ||
             ((s32) lbl_1_data_30[difficulty] != (s32) lbl_1_bss_3CC[player]))) {
            lbl_1_bss_3FC[player] = -1;
            return chosenDirection;
        }
        if ((s32) lbl_1_bss_3FC[player] == 0) {
            if (((s32) lbl_1_data_20[difficulty] != 0) &&
                ((s32) lbl_1_bss_48[lbl_1_bss_3AC[player] + 1] != 0) &&
                ((s32) (rand8() % lbl_1_data_20[difficulty]) == 0)) {
                while (1) {
                candidateDirection = rand8() % 4 + 1;
                switch (lbl_1_bss_48[lbl_1_bss_3AC[player] - 1]) {
                case 1:
                    if (candidateDirection == 2) {
                        continue;
                    }
                    break;
                case 2:
                    if (candidateDirection == 1) {
                        continue;
                    }
                    break;
                case 3:
                    if (candidateDirection == 4) {
                        continue;
                    }
                    break;
                case 4:
                    if (candidateDirection == 3) {
                        continue;
                    }
                    break;
                }
                candidateColumn = (s32) (lbl_1_bss_3BC[player] & 0xF0) >> 4;
                candidateRow = lbl_1_bss_3BC[player] & 0xF;
                fn_1_5EF0(candidateDirection, &candidateColumn, &candidateRow);
                if (candidateColumn < 0) {
                    continue;
                }
                if (candidateColumn > 6) {
                    continue;
                }
                if (candidateRow < 0) {
                    continue;
                }
                if (candidateRow > 6) {
                    continue;
                }
                if (((s32) lbl_1_data_40[difficulty] == 0) &&
                    ((u32) (Hu3DModelAttrGet(
                                lbl_1_bss_4AC.tiles[player][candidateColumn + candidateRow * 7]) &
                            HU3D_ATTR_DISPOFF) != 0)) {
                    continue;
                }
                break;
                }
                chosenDirection = candidateDirection;
            }
            lbl_1_bss_3CC[player] = 0;
            lbl_1_bss_3FC[player] = -1;
            return chosenDirection;
        }
        if ((s32) lbl_1_bss_3FC[player] == -1) {
            lbl_1_bss_3FC[player] =
                lbl_1_data_0[difficulty] + (rand8() % lbl_1_data_10[difficulty]);
        }
        return 0;
    }
    return chosenDirection;
}

static s32 lbl_1_data_128[4] = { 1, 2, 4, 8 };
static s32 lbl_1_data_138[4] = { 0, 640, 0, 640 };
static s32 lbl_1_data_148[4] = { 0, 0, 480, 480 };
static s32 lbl_1_data_158[4] = { 640, 0, 640, 0 };
static s32 lbl_1_data_168[4] = { 480, 480, 0, 0 };

/* Called during scene setup to create and position the four player cameras. */
void fn_1_48D4(void)
{
    s32 viewIndex;

    Hu3DCameraCreate(15);
    viewIndex = 0;
    while (viewIndex < 4) {
        Hu3DCameraPerspectiveSet(lbl_1_data_128[viewIndex], 45.0f, 20.0f, 15000.0f, 1.2f);
        Hu3DCameraViewportSet(lbl_1_data_128[viewIndex], (f32) lbl_1_data_138[viewIndex],
                              (f32) lbl_1_data_148[viewIndex], (f32) lbl_1_data_158[viewIndex],
                              (f32) lbl_1_data_168[viewIndex], 0.0f, 1.0f);
        Hu3DCameraScissorSet(lbl_1_data_128[viewIndex], (u32) lbl_1_data_138[viewIndex],
                             (u32) lbl_1_data_148[viewIndex], 640U, 480U);
        CenterM[viewIndex].x = 0.0f;
        CenterM[viewIndex].y = 150.0f;
        CenterM[viewIndex].z = 0.0f;
        CRotM[viewIndex].x = -50.0f;
        CRotM[viewIndex].y = 0.0f;
        CRotM[viewIndex].z = 0.0f;
        CZoomM[viewIndex] = 700.0f;
        viewIndex += 1;
    }
}

/* Called during the sequence transition to animate all four camera viewports. */
void fn_1_4B94(s32 cameraIndex)
{
    s32 scissor[4][4];
    s32 viewport[4][4];
    f32 time;
    f32 offsetTime;
    s32 width;
    s32 height;
    s32 viewHeight;
    s32 viewIndex;
    s32 shiftY;

    time = 0.016666668f * (f32) (lbl_1_bss_14 + 60);
    offsetTime = 0.016666666666666666 * (f64) lbl_1_bss_14;
    width = 320.0f * time;
    height = 240.0f * time;
    viewHeight = 280.0f * time;
    shiftY = 280.0f * offsetTime;
    switch (cameraIndex) {
    case 0:
        scissor[0][0] = width;
        scissor[0][1] = height;
        scissor[0][2] = 0;
        scissor[0][3] = 0;
        scissor[1][0] = 640 - width;
        scissor[1][1] = height;
        scissor[1][2] = width;
        scissor[1][3] = 0;
        scissor[2][0] = width;
        scissor[2][1] = 480 - height;
        scissor[2][2] = 0;
        scissor[2][3] = height;
        scissor[3][0] = 640 - width;
        scissor[3][1] = 480 - height;
        scissor[3][2] = width;
        scissor[3][3] = height;
        viewport[0][0] = width;
        viewport[0][1] = viewHeight;
        viewport[0][2] = 0;
        viewport[0][3] = 0;
        viewport[1][0] = 640 - width;
        viewport[1][1] = viewHeight;
        viewport[1][2] = width;
        viewport[1][3] = 0;
        viewport[2][0] = width;
        viewport[2][1] = 560 - viewHeight;
        viewport[2][2] = 0;
        viewport[2][3] = shiftY + 200;
        viewport[3][0] = 640 - width;
        viewport[3][1] = 560 - viewHeight;
        viewport[3][2] = width;
        viewport[3][3] = shiftY + 200;
        break;
    case 1:
        scissor[0][0] = 640 - width;
        scissor[0][1] = height;
        scissor[0][2] = 0;
        scissor[0][3] = 0;
        scissor[1][0] = width;
        scissor[1][1] = height;
        scissor[1][2] = 640 - width;
        scissor[1][3] = 0;
        scissor[2][0] = 640 - width;
        scissor[2][1] = 480 - height;
        scissor[2][2] = 0;
        scissor[2][3] = height;
        scissor[3][0] = width;
        scissor[3][1] = 480 - height;
        scissor[3][2] = 640 - width;
        scissor[3][3] = height;
        viewport[0][0] = 640 - width;
        viewport[0][1] = viewHeight;
        viewport[0][2] = 0;
        viewport[0][3] = 0;
        viewport[1][0] = width;
        viewport[1][1] = viewHeight;
        viewport[1][2] = 640 - width;
        viewport[1][3] = 0;
        viewport[2][0] = 640 - width;
        viewport[2][1] = 560 - viewHeight;
        viewport[2][2] = 0;
        viewport[2][3] = shiftY + 200;
        viewport[3][0] = width;
        viewport[3][1] = 560 - viewHeight;
        viewport[3][2] = 640 - width;
        viewport[3][3] = shiftY + 200;
        break;
    case 2:
        scissor[0][0] = width;
        scissor[0][1] = 480 - height;
        scissor[0][2] = 0;
        scissor[0][3] = 0;
        scissor[1][0] = 640 - width;
        scissor[1][1] = 480 - height;
        scissor[1][2] = width;
        scissor[1][3] = 0;
        scissor[2][0] = width;
        scissor[2][1] = height;
        scissor[2][2] = 0;
        scissor[2][3] = 480 - height;
        scissor[3][0] = 640 - width;
        scissor[3][1] = height;
        scissor[3][2] = width;
        scissor[3][3] = 480 - height;
        viewport[0][0] = width;
        viewport[0][1] = 560 - viewHeight;
        viewport[0][2] = 0;
        viewport[0][3] = 0;
        viewport[1][0] = 640 - width;
        viewport[1][1] = 560 - viewHeight;
        viewport[1][2] = width;
        viewport[1][3] = 0;
        viewport[2][0] = width;
        viewport[2][1] = viewHeight;
        viewport[2][2] = 0;
        viewport[2][3] = 200 - shiftY;
        viewport[3][0] = 640 - width;
        viewport[3][1] = viewHeight;
        viewport[3][2] = width;
        viewport[3][3] = 200 - shiftY;
        break;
    case 3:
        scissor[0][0] = 640 - width;
        scissor[0][1] = 480 - height;
        scissor[0][2] = 0;
        scissor[0][3] = 0;
        scissor[1][0] = width;
        scissor[1][1] = 480 - height;
        scissor[1][2] = 640 - width;
        scissor[1][3] = 0;
        scissor[2][0] = 640 - width;
        scissor[2][1] = height;
        scissor[2][2] = 0;
        scissor[2][3] = 480 - height;
        scissor[3][0] = width;
        scissor[3][1] = height;
        scissor[3][2] = 640 - width;
        scissor[3][3] = 480 - height;
        viewport[0][0] = 640 - width;
        viewport[0][1] = 560 - viewHeight;
        viewport[0][2] = 0;
        viewport[0][3] = 0;
        viewport[1][0] = width;
        viewport[1][1] = 560 - viewHeight;
        viewport[1][2] = 640 - width;
        viewport[1][3] = 0;
        viewport[2][0] = 640 - width;
        viewport[2][1] = viewHeight;
        viewport[2][2] = 0;
        viewport[2][3] = 200 - shiftY;
        viewport[3][0] = width;
        viewport[3][1] = viewHeight;
        viewport[3][2] = 640 - width;
        viewport[3][3] = 200 - shiftY;
        break;
    }
    viewIndex = 0;
    while (viewIndex < 4) {
        Hu3DCameraViewportSet(lbl_1_data_74[viewIndex], viewport[viewIndex][2],
                              viewport[viewIndex][3], viewport[viewIndex][0],
                              viewport[viewIndex][1], 0.0f, 1.0f);
        Hu3DCameraScissorSet(lbl_1_data_74[viewIndex], scissor[viewIndex][2] + 2,
                             scissor[viewIndex][3] + 2, scissor[viewIndex][0] - 2,
                             scissor[viewIndex][1] - 2);
        viewIndex += 1;
    }
}

/* Called before the boards appear to restore four equal camera viewports. */
void fn_1_51D8(void)
{
    Hu3DCameraViewportSet(lbl_1_data_74[0], 0.0f, 0.0f, 320.0f, 280.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(lbl_1_data_74[0], 0U, 0U, 318U, 238U);
    Hu3DCameraPerspectiveSet(lbl_1_data_74[0], 45.0f, 20.0f, 15000.0f, 1.1428572f);
    Hu3DCameraViewportSet(lbl_1_data_74[1], 320.0f, 0.0f, 320.0f, 280.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(lbl_1_data_74[1], 322U, 0U, 318U, 238U);
    Hu3DCameraPerspectiveSet(lbl_1_data_74[1], 45.0f, 20.0f, 15000.0f, 1.1428572f);
    Hu3DCameraViewportSet(lbl_1_data_74[2], 0.0f, 200.0f, 320.0f, 280.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(lbl_1_data_74[2], 0U, 242U, 318U, 238U);
    Hu3DCameraPerspectiveSet(lbl_1_data_74[2], 45.0f, 20.0f, 15000.0f, 1.1428572f);
    Hu3DCameraViewportSet(lbl_1_data_74[3], 320.0f, 200.0f, 320.0f, 280.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(lbl_1_data_74[3], 322U, 242U, 318U, 238U);
    Hu3DCameraPerspectiveSet(lbl_1_data_74[3], 45.0f, 20.0f, 15000.0f, 1.1428572f);
}

/* Object-manager callback from scene setup; stage each player's marker through its reveal, fall,
 * and rise animation. */
void fn_1_54D4(OMOBJ *obj)
{
    Point3d markerPosition;
    f32 fallProgress;
    f32 riseProgress;
    s32 playerIndex;

    playerIndex = 0;
    while (playerIndex < 4) {
        switch (lbl_1_bss_36C[playerIndex]) {
        case 0:
            lbl_1_bss_36C[playerIndex] = 1;
            lbl_1_bss_37C[playerIndex] = 0;
            break;
        case 2:
            Hu3DModelPosGet((s16) lbl_1_bss_48C[playerIndex], &markerPosition);
            markerPosition.y = 600.0f;
            Hu3DModelPosSetV((s16) lbl_1_bss_39C[playerIndex], &markerPosition);
            lbl_1_bss_37C[playerIndex] = 0;
            lbl_1_bss_36C[playerIndex] = 3;
            break;
        case 3:
            lbl_1_bss_37C[playerIndex]++;
            if ((s32) lbl_1_bss_37C[playerIndex] > 30) {
                lbl_1_bss_37C[playerIndex] = 0;
                lbl_1_bss_36C[playerIndex] = 4;
                Hu3DModelAttrReset((s16) lbl_1_bss_39C[playerIndex], 1U);
            }
            break;
        case 4:
            fallProgress = 0.033333335f * (f32) lbl_1_bss_37C[playerIndex];
            Hu3DModelPosGet((s16) lbl_1_bss_39C[playerIndex], &markerPosition);
            markerPosition.y = (1300.0f - (1300.0f * fallProgress)) - 700.0f;
            Hu3DModelPosSetV((s16) lbl_1_bss_39C[playerIndex], &markerPosition);
            if ((s32) lbl_1_bss_37C[playerIndex] == 30) {
                lbl_1_bss_37C[playerIndex] = 0;
                lbl_1_bss_36C[playerIndex] = 5;
                HuAudFXPlay(M612_SE_MARKER_LAND);
            } else {
                lbl_1_bss_37C[playerIndex]++;
            }
            break;
        case 5:
            riseProgress = 0.033333335f * (f32) lbl_1_bss_37C[playerIndex];
            Hu3DModelPosGet((s16) lbl_1_bss_39C[playerIndex], &markerPosition);
            markerPosition.y = (1300.0f * riseProgress) - 700.0f;
            Hu3DModelPosSetV((s16) lbl_1_bss_39C[playerIndex], &markerPosition);
            if ((s32) lbl_1_bss_37C[playerIndex] == 30) {
                Hu3DModelAttrSet((s16) lbl_1_bss_39C[playerIndex], 1U);
                lbl_1_bss_37C[playerIndex] = 0;
                lbl_1_bss_36C[playerIndex] = 1;
            } else {
                lbl_1_bss_37C[playerIndex]++;
            }
            break;
        }
        playerIndex += 1;
    }
}

/* Object-manager callback from scene setup; move the shared guide along the chosen route. */
void fn_1_5924(OMOBJ *obj)
{
    Point3d guidePosition;
    s32 column;
    s32 row;
    float angle;

        switch (lbl_1_bss_308) {
        case 0:
            Hu3DMotionSet((s16) lbl_1_bss_300, (s16) lbl_1_bss_2F8);
            *lbl_1_bss_3BC = 55;
            lbl_1_bss_308 = 1;
            break;
        case 1:
            break;
        case 2:
            if ((s32) lbl_1_bss_48[lbl_1_bss_2F0] != 0) {
                angle = fn_1_5E00(lbl_1_bss_48[lbl_1_bss_2F0]);
                Hu3DModelRotSet((s16) lbl_1_bss_300, 0.0f, angle, 0.0f);
                lbl_1_bss_304 = 10;
                lbl_1_bss_308 = 3;
                return;
            }
            lbl_1_bss_308 = 4;
            return;
        case 3:
            lbl_1_bss_304 -= 1;
            if ((s32) lbl_1_bss_304 >= 0) {
                Hu3DModelPosGet((s16) lbl_1_bss_300, &guidePosition);
                switch (lbl_1_bss_48[lbl_1_bss_2F0]) {
                case 1:
                    guidePosition.x -= 20.0f;
                    break;
                case 2:
                    guidePosition.x += 20.0f;
                    break;
                case 3:
                    guidePosition.z -= 20.0f;
                    break;
                case 4:
                    guidePosition.z += 20.0f;
                    break;
                }
                Hu3DModelPosSetV((s16) lbl_1_bss_300, (Point3d *) &guidePosition);
                return;
            }
            lbl_1_bss_308 = 2;
            column = (s32) (*lbl_1_bss_3BC & 0xF0) >> 4;
            row = *lbl_1_bss_3BC & 0xF;
            switch (lbl_1_bss_48[lbl_1_bss_2F0]) {
            case 1:
                column -= 1;
                break;
            case 2:
                column += 1;
                break;
            case 4:
                row += 1;
                break;
            case 3:
                row -= 1;
                break;
            }
            if ((column >= 0) && (column < 7) && (row >= 0) && (row < 7)) {
                Hu3DModelAttrReset(lbl_1_bss_4AC.tiles[0][column + (row * 7)], HU3D_MOTATTR_PAUSE);
                *lbl_1_bss_3BC = (column << 4) | (row & 0xF);
            }
            lbl_1_bss_2F0 += 1;
            HuAudFXPlayVolPan(M612_SE_GUIDE_STEP, (s16) (32.0f + (11.142858f * (f32) row)),
                              (s16) (48.0f + (4.571429f * (f32) column)));
            return;
        case 4:
            Hu3DModelRotSet((s16) lbl_1_bss_300, 0.0f, 0.0f, 0.0f);
            lbl_1_bss_14 = 0;
            lbl_1_bss_308 = 6;
            return;
        case 5:
            if ((s32) lbl_1_bss_14 > 300) {
                lbl_1_bss_308 = 6;
            }
            lbl_1_bss_14 += 1;

        case 6:
        default:
            break;
    }
}

/* Called by the player and guide updates to face a character along its route direction. */
float fn_1_5E00(s32 direction)
{
    f32 facingAngle;

    facingAngle = 0.0f;
    switch (direction) {
    case 1:
        facingAngle = 270.0f;
        break;
    case 2:
        facingAngle = 90.0f;
        break;
    case 3:
        facingAngle = 180.0f;
        break;
    case 4:
        facingAngle = 0.0f;
        break;
    }
    return facingAngle;
}

/* No current Memory Lane callback calls this helper; it adjusts a position along one cardinal
 * direction. */
void fn_1_5E90(s32 direction, Point3d *position, f32 distance)
{
    switch (direction) {
    case 1:
        position->x -= distance;
        break;
    case 2:
        /* Direction 2 replaces x with distance rather than adding to it. */
        position->x = distance;
        break;
    case 3:
        position->z -= distance;
        break;
    case 4:
        position->z += distance;
        break;
    }
}

/* Called by computer route selection to test a candidate direction's neighboring cell. */
void fn_1_5EF0(s32 direction, s32 *column, s32 *row)
{
    switch (direction) {
    case 1:
        *column -= 1;
        break;
    case 2:
        *column += 1;
        break;
    case 3:
        *row -= 1;
        break;
    case 4:
        *row += 1;
        break;
    }
}

/* Unused helper; if called, reveal each in-bounds tile along the selected route on the specified
 * player's board. */
void fn_1_5F58(s32 player)
{
    s32 routeIndex;
    s32 column;
    s32 row;

    routeIndex = 0;
    column = 3;
    row = 7;
    while ((s32) lbl_1_bss_48[routeIndex] != 0) {
        switch (lbl_1_bss_48[routeIndex]) {
        case 1:
            column -= 1;
            break;
        case 2:
            column += 1;
            break;
        case 3:
            row -= 1;
            break;
        case 4:
            row += 1;
            break;
        }
        if ((column >= 0) && (column < 7) && (row >= 0) && (row < 7)) {
            Hu3DModelAttrReset(lbl_1_bss_4AC.tiles[player][column + row * 7], HU3D_MOTATTR_PAUSE);
        }
        routeIndex += 1;
    }
}
