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
/* Extent follows the observed allocation; only four counters are consumed. */
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
/* Extent follows the observed allocation; only four model IDs are consumed. */
static s32 lbl_1_bss_2C0[10];
/* Path-buffer allocation extent; original declared capacity is unresolved. */
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
static unsigned int lbl_1_data_50[9] = { 9633792, 9633793, 9633798, 9633799, 9633807, 9633826, 9633855, 9633815, 0 };
static s32 lbl_1_data_74[4] = { 1, 2, 4, 8 };
GXColor lbl_1_data_84[2] = { { 255, 255, 255, 0 }, { 208, 208, 208, 0 } };
static s32 lbl_1_data_8C = -1;
static MGSEQ_PARAM lbl_1_data_90 = {
    300,
    0,
    fn_1_1694,
    fn_1_16D4,
    fn_1_1DBC,
    fn_1_1E18,
    fn_1_1FB4,
    fn_1_242C,
    fn_1_2BC8,
    fn_1_2C8C,
    fn_1_2C90,
};

void fn_1_A0(void)
{
    s16 sp8;
    s16 i;
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
    i = 0;
    while (i < 4) {
        lbl_1_bss_42C[i] = 0;
        i += 1;
    }
    fn_1_48D4();
    view = omAddObjEx(lbl_1_bss_4AC.manager, 32730, 0U, 0U, -1, omOutViewMulti);
    view->work[0] = 4;
    MgSeqCreate(&lbl_1_data_90);
    i = 0;
    while (i < 4) {
        lbl_1_bss_49C[i] = (s32) GwPlayerConf[i].charNo;
        lbl_1_bss_43C[i] = (s32) GwPlayerConf[i].padNo;
        lbl_1_bss_48C[i] = (s32) CharModelMotListCreate((s16) lbl_1_bss_49C[i], 4, lbl_1_data_50, lbl_1_bss_44C[i]);
        Hu3DModelPosSet((s16) lbl_1_bss_48C[i], 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet((s16) lbl_1_bss_48C[i], 1073741825U);
        CharMotionSet((s16) lbl_1_bss_49C[i], *lbl_1_bss_44C[i]);
        Hu3DModelCameraSet((s16) lbl_1_bss_48C[i], (u16) lbl_1_data_74[i]);
        lbl_1_bss_3FC[i] = -1;
        lbl_1_bss_30C[i] = 0;
        Hu3DModelAttrSet((s16) lbl_1_bss_48C[i], 1U);
        i += 1;
    }
    {
        HuVecF lightPos = { 0.0f, 1000.0f, 5000.0f };
        HuVecF lightDir = { 0.0f, 0.0f, -1.0f };
        sp8 = CharLightCreateV(&lightPos, &lightDir, &lbl_1_data_84[lbl_1_bss_10]);
    }
    CharLightStaticSet(1);
    i = 0;
    while (i < 4) {
        lbl_1_bss_36C[i] = 0;
        if (i == 0) {
            lbl_1_bss_39C[i] = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653077, 268435456, HEAP_MODEL));
            lbl_1_bss_38C[i] = (s32) Hu3DJointMotion((s16) *lbl_1_bss_39C, HuDataSelHeapReadNum(4653078, 268435456, HEAP_MODEL));
        } else {
            lbl_1_bss_39C[i] = (s32) Hu3DModelLink((s16) *lbl_1_bss_39C);
        }
        {
            HuVecF modelLightPos = { 0.0f, 1000.0f, 5000.0f };
            HuVecF modelLightDir = { 0.0f, 0.0f, -1.0f };
            Hu3DLLightCreateV((s16) lbl_1_bss_39C[i], &modelLightPos, &modelLightDir, &lbl_1_data_84[lbl_1_bss_10]);
        }
        Hu3DMotionSet((s16) lbl_1_bss_39C[i], (s16) *lbl_1_bss_38C);
        Hu3DModelCameraSet((s16) lbl_1_bss_39C[i], (u16) lbl_1_data_74[i]);
        Hu3DModelAttrSet((s16) lbl_1_bss_39C[i], 1U);
        i += 1;
    }
    lbl_1_bss_300 = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653079, 268435456, HEAP_MODEL));
    lbl_1_bss_2FC = (s32) Hu3DJointMotion((s16) lbl_1_bss_300, HuDataSelHeapReadNum(4653080, 268435456, HEAP_MODEL));
    lbl_1_bss_2F8 = (s32) Hu3DJointMotion((s16) lbl_1_bss_300, HuDataSelHeapReadNum(4653081, 268435456, HEAP_MODEL));
    lbl_1_bss_2F4 = (s32) Hu3DJointMotion((s16) lbl_1_bss_300, HuDataSelHeapReadNum(4653082, 268435456, HEAP_MODEL));
    {
        HuVecF resultLightPos = { 0.0f, 1000.0f, 5000.0f };
        HuVecF resultLightDir = { 0.0f, 0.0f, -1.0f };
        Hu3DLLightCreateV((s16) lbl_1_bss_300, &resultLightPos, &resultLightDir, &lbl_1_data_84[lbl_1_bss_10]);
    }
    Hu3DModelPosSet((s16) lbl_1_bss_300, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet((s16) lbl_1_bss_300, 1073741825U);
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

void fn_1_8F0(void)
{
    if ((s32) lbl_1_bss_10 == 0) {
        lbl_1_bss_44 = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653058, 268435456, HEAP_MODEL));
        lbl_1_bss_40 = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653059, 268435456, HEAP_MODEL));
        lbl_1_bss_3C = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653064, 268435456, HEAP_MODEL));
        lbl_1_bss_38 = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653060, 268435456, HEAP_MODEL));
        lbl_1_bss_34 = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653061, 268435456, HEAP_MODEL));
        lbl_1_bss_30 = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653062, 268435456, HEAP_MODEL));
        lbl_1_bss_2C = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653063, 268435456, HEAP_MODEL));
        lbl_1_bss_28 = -1;
        lbl_1_bss_20 = -1;
        lbl_1_bss_24 = -1;
    } else {
        lbl_1_bss_44 = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653067, 268435456, HEAP_MODEL));
        lbl_1_bss_40 = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653068, 268435456, HEAP_MODEL));
        lbl_1_bss_3C = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653073, 268435456, HEAP_MODEL));
        lbl_1_bss_38 = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653069, 268435456, HEAP_MODEL));
        lbl_1_bss_34 = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653070, 268435456, HEAP_MODEL));
        lbl_1_bss_30 = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653071, 268435456, HEAP_MODEL));
        lbl_1_bss_2C = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653072, 268435456, HEAP_MODEL));
        lbl_1_bss_20 = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653074, 268435456, HEAP_MODEL));
        Hu3DModelLayerSet((s16) lbl_1_bss_20, 7);
        Hu3DModelAttrSet((s16) lbl_1_bss_20, 1073741825U);
        Hu3DModelCameraSet((s16) lbl_1_bss_20, 15U);
        lbl_1_bss_28 = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653075, 268435456, HEAP_MODEL));
        Hu3DModelLayerSet((s16) lbl_1_bss_28, 7);
        Hu3DModelAttrSet((s16) lbl_1_bss_28, 1073741825U);
        Hu3DModelCameraSet((s16) lbl_1_bss_28, 15U);
        Hu3DModelAttrSet((s16) lbl_1_bss_28, 1U);
        lbl_1_bss_24 = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653076, 268435456, HEAP_MODEL));
        Hu3DModelLayerSet((s16) lbl_1_bss_24, 7);
        Hu3DModelAttrSet((s16) lbl_1_bss_24, 1073741825U);
        Hu3DModelCameraSet((s16) lbl_1_bss_24, 15U);
        Hu3DModelAttrSet((s16) lbl_1_bss_24, 1U);
    }
    Hu3DModelPosSet((s16) lbl_1_bss_44, 0.0f, 0.0f, 0.0f);
    Hu3DModelCameraSet((s16) lbl_1_bss_44, 15U);
    Hu3DModelLayerSet((s16) lbl_1_bss_44, 1);
    Hu3DModelAttrSet((s16) lbl_1_bss_44, 1073741825U);
    Hu3DModelCameraSet((s16) lbl_1_bss_40, 15U);
    Hu3DModelLayerSet((s16) lbl_1_bss_40, 1);
    Hu3DModelAttrSet((s16) lbl_1_bss_40, 1073741825U);
    Hu3DModelCameraSet((s16) lbl_1_bss_3C, 15U);
    Hu3DModelLayerSet((s16) lbl_1_bss_3C, 0);
    Hu3DModelAttrSet((s16) lbl_1_bss_3C, 1073741826U);
    Hu3DModelAttrSet((s16) lbl_1_bss_3C, 1073741825U);
    Hu3DModelCameraSet((s16) lbl_1_bss_38, 15U);
    Hu3DModelLayerSet((s16) lbl_1_bss_38, 2);
    Hu3DModelAttrSet((s16) lbl_1_bss_38, 1073741825U);
    Hu3DModelLayerSet((s16) lbl_1_bss_34, 6);
    Hu3DModelAttrSet((s16) lbl_1_bss_30, 1U);
    Hu3DModelLayerSet((s16) lbl_1_bss_30, 2);
    Hu3DModelAttrSet((s16) lbl_1_bss_2C, 1U);
    Hu3DModelLayerSet((s16) lbl_1_bss_2C, 3);
}

void fn_1_F0C(void)
{
    s32 sp8;
    s32 player;
    s32 row;
    s32 column;

    if ((s32) lbl_1_bss_10 == 0) {
        lbl_1_bss_4AC.tiles[0][0] = Hu3DModelCreate(HuDataSelHeapReadNum(4653056, 268435456, HEAP_MODEL));
    } else {
        lbl_1_bss_4AC.tiles[0][0] = Hu3DModelCreate(HuDataSelHeapReadNum(4653065, 268435456, HEAP_MODEL));
    }
    for (player = 0; player < 4; player++) {
        for (row = 0; row < 7; row++) {
            for (column = 0; column < 7; column++) {
                if (((player == 0) & ((column == 0) & (row == 0))) == 0) {
                    lbl_1_bss_4AC.tiles[player][column + row * 7] = Hu3DModelLink(lbl_1_bss_4AC.tiles[0][0]);
                }
                Hu3DModelAttrSet(lbl_1_bss_4AC.tiles[player][column + row * 7], 1073741826U);
                Hu3DModelPosSet(lbl_1_bss_4AC.tiles[player][column + row * 7], (float) (column * 200 - 600), 0.0f, (float) (row * 200 - 1400));
                Hu3DModelCameraSet(lbl_1_bss_4AC.tiles[player][column + row * 7], (u16) lbl_1_data_74[player]);
                Hu3DModelLayerSet(lbl_1_bss_4AC.tiles[player][column + row * 7], 5);
            }
        }
    }
    sp8 = 0;
    lbl_1_bss_4AC.borders[0] = Hu3DModelLink(lbl_1_bss_4AC.tiles[0][0]);
    Hu3DModelPosSet(lbl_1_bss_4AC.borders[0], 0.0f, 0.0f, 0.0f);
    Hu3DModelLayerSet(lbl_1_bss_4AC.borders[0], 5);
    Hu3DModelAttrReset(lbl_1_bss_4AC.borders[0], 1073741826U);
    lbl_1_bss_4AC.borders[1] = Hu3DModelLink(lbl_1_bss_4AC.tiles[0][0]);
    Hu3DModelPosSet(lbl_1_bss_4AC.borders[1], 0.0f, 0.0f, -1600.0f);
    Hu3DModelLayerSet(lbl_1_bss_4AC.borders[1], 5);
    Hu3DModelAttrReset(lbl_1_bss_4AC.borders[1], 1073741826U);
    if ((s32) lbl_1_bss_10 == 0) {
        *lbl_1_bss_2C0 = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653057, 268435456, HEAP_MODEL));
    } else {
        *lbl_1_bss_2C0 = (s32) Hu3DModelCreate(HuDataSelHeapReadNum(4653066, 268435456, HEAP_MODEL));
    }
    for (player = 0; player < 4; player++) {
        if (player != 0) {
            lbl_1_bss_2C0[player] = (s32) Hu3DModelLink((s16) *lbl_1_bss_2C0);
        }
        Hu3DModelAttrSet((s16) lbl_1_bss_2C0[player], 1U);
        Hu3DModelAttrSet((s16) lbl_1_bss_2C0[player], 1073741826U);
        Hu3DModelCameraSet((s16) lbl_1_bss_2C0[player], (u16) lbl_1_data_74[player]);
        Hu3DModelLayerSet((s16) lbl_1_bss_2C0[player], 4);
    }
}

s32 fn_1_13A4(s32 x, s32 y)
{
    s32 bitNo;
    s8 row;
    s32 bitMask;

    row = lbl_1_data_16B0[lbl_1_bss_8][y];
    if (lbl_1_bss_C != 0) {
        bitNo = x;
    } else {
        bitNo = 6 - x;
    }
    bitMask = 1 << bitNo;
    if ((bitMask & row) != 0) {
        return 1;
    }
    return 0;
}

void fn_1_1430(void)
{
    s32 i;

    lbl_1_bss_8 = rand8() % 8;
    lbl_1_bss_4 = lbl_1_bss_8 * 9 + rand8() % 4;
    lbl_1_bss_C = rand8() & 1;
    for (i = 0; lbl_1_data_198[lbl_1_bss_4][i] != 0; i++) {
        if ((s32) lbl_1_bss_C != 0) {
            switch (lbl_1_data_198[lbl_1_bss_4][i]) {
            case 1:
                lbl_1_bss_48[i] = 2;
                break;
            case 2:
                lbl_1_bss_48[i] = 1;
                break;
            default:
                lbl_1_bss_48[i] = lbl_1_data_198[lbl_1_bss_4][i];
                break;
            }
        } else {
            lbl_1_bss_48[i] = lbl_1_data_198[lbl_1_bss_4][i];
        }
    }
    lbl_1_bss_48[i] = 0;
    for (i = 0; i < 4; i++) {
        lbl_1_bss_3BC[i] = (lbl_1_bss_3BC[i] & 15) | 48;
        lbl_1_bss_3BC[i] = (lbl_1_bss_3BC[i] & 240) | 7;
    }
}

void fn_1_1694(s16 mode, s16 frameNo)
{
    lbl_1_bss_14 = 0;
    lbl_1_bss_18 = 0;
    MgSeqModeNext();
}

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
            HuAudFXPlay(1701);
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
                HuAudFXPlay(1701);
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
                    Hu3DModelAttrSet(lbl_1_bss_4AC.tiles[0][column + (row * 7)], 1073741826U);
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

void fn_1_1DBC(s16 mode, s16 frameNo)
{
    if (((s32) lbl_1_data_8C == -1) && ((s32) (GameMesStatGet(MgSeqGameMesIdGet()) & 16) != 0)) {
        lbl_1_data_8C = HuAudBGMPlay(83);
    }
}

void fn_1_1E18(s16 mode, s16 frameNo)
{
    s32 candidates[4];
    s32 count;
    s32 i;

    switch ((s32) lbl_1_bss_4AC.state) {            /* irregular */
    case 9:
        lbl_1_bss_0 = -1;
        lbl_1_bss_4AC.state = 10;
        /* fallthrough */
    case 10:
        i = 0;
        count = 0;
        for (; i < 4; i++) {
            if ((s32) lbl_1_bss_42C[i] == 11) {
                candidates[count] = i;
                count++;
            }
        }
        if (count != 0) {
            if (count != 1) {
                lbl_1_bss_0 = candidates[rand8() % count];
            } else {
                lbl_1_bss_0 = candidates[0];
            }
            lbl_1_bss_14 = 0;
            for (i = 0; i < 4; i++) {
                if ((s32) lbl_1_bss_42C[i] == 2) {
                    lbl_1_bss_42C[i] = 1;
                    CharMotionSet((s16) lbl_1_bss_49C[i], lbl_1_bss_44C[i][0]);
                }
            }
            lbl_1_bss_4AC.state = 12;
            MgSeqModeNext();
        }
        break;
    }
}

void fn_1_1FB4(s16 mode, s16 frameNo)
{
    s16 sp8[4];
    s16 var_r30;
    s16 var_r31;
    s32 temp_r29;

    if ((MgSeqFrameNoGet() == 0) && ((s32) lbl_1_data_8C != -1)) {
        HuAudSStreamFadeOut(lbl_1_data_8C, 100);
        lbl_1_data_8C = -1;
    }
    switch ((s32) lbl_1_bss_4AC.state) {            /* irregular */
    case 10:
        var_r31 = 0;
        while (var_r31 < 4) {
            (&sp8[0])[var_r31] = -1;
            var_r31 += 1;
        }
        MgSeqWinnerSet(sp8[0], sp8[1], sp8[2], sp8[3]);
        lbl_1_bss_14 = 0;
        var_r31 = 0;
        while (var_r31 < 4) {
            if (((s32) lbl_1_bss_42C[var_r31] == 2) || ((s32) lbl_1_bss_42C[var_r31] == 3)) {
                CharMotionShiftSet((s16) lbl_1_bss_49C[var_r31], lbl_1_bss_44C[var_r31][0], 0.0f, 5.0f, 0U);
                lbl_1_bss_42C[var_r31] = 1;
            }
            var_r31 += 1;
        }
        lbl_1_bss_4AC.state = 18;
        break;
    case 11:
        var_r31 = 0;
        var_r30 = 0;
        while (var_r31 < 4) {
            if ((s32) lbl_1_bss_42C[var_r31] == 1) {
                var_r30 += 1;
            }
            var_r31 += 1;
        }
        if (var_r30 == 4) {
            var_r31 = 0;
            while (var_r31 < 4) {
                CharMotionSet((s16) lbl_1_bss_49C[var_r31], lbl_1_bss_44C[var_r31][3]);
                Hu3DModelRotSet((s16) lbl_1_bss_48C[var_r31], 0.0f, 0.0f, 0.0f);
                Hu3DModelAttrReset((s16) lbl_1_bss_48C[var_r31], 1073741825U);
                var_r31 += 1;
            }
            lbl_1_bss_4AC.state = 19;
        }
        break;
    case 12:
        lbl_1_bss_14 = 0;
        var_r31 = 0;
        while (var_r31 < 4) {
            sp8[var_r31] = -1;
            if (((s32) lbl_1_bss_42C[var_r31] == 2) || ((s32) lbl_1_bss_42C[var_r31] == 3)) {
                CharMotionShiftSet((s16) lbl_1_bss_49C[var_r31], lbl_1_bss_44C[var_r31][0], 0.0f, 10.0f, 1073741825U);
                lbl_1_bss_42C[var_r31] = 1;
            }
            var_r31 += 1;
        }
        (&sp8[0])[lbl_1_bss_0] = (s16) lbl_1_bss_49C[lbl_1_bss_0];
        MgSeqWinnerSet(sp8[0], sp8[1], sp8[2], sp8[3]);
        lbl_1_bss_4AC.state = 13;
        temp_r29 = lbl_1_bss_0;
        if (_CheckFlag(65551U) == 0) {
            GwPlayer[temp_r29].mgCoinBonus = 10;
        }
        break;
    }
}

void fn_1_242C(s16 mode, s16 frameNo)
{
    s32 var_r31;
    s32 var_r30;

    switch (lbl_1_bss_4AC.state) {
    case 13:
        lbl_1_bss_14 = 0;
        lbl_1_bss_4AC.state = 14;
        /* fallthrough */
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
        var_r31 = 0;
        while (var_r31 < 60) {
            lbl_1_bss_14 += 1;
            if ((s32) lbl_1_bss_14 <= 60) {
                fn_1_4B94(lbl_1_bss_0);
            }
            HuPrcVSleep();
            var_r31 += 1;
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
        Hu3DModelAttrReset((s16) lbl_1_bss_48C[lbl_1_bss_0], 1073741825U);
        modelPos.x = 150.0f;
        Hu3DModelPosSetV((s16) lbl_1_bss_300, &modelPos);
        Hu3DModelAttrReset((s16) lbl_1_bss_300, 1U);
        Hu3DModelRotSet((s16) lbl_1_bss_300, 0.0f, 0.0f, 0.0f);
        Hu3DMotionSet((s16) lbl_1_bss_300, (s16) lbl_1_bss_2F4);
        Hu3DModelAttrReset((s16) lbl_1_bss_3C, 1073741826U);
        Hu3DModelAttrSet((s16) lbl_1_bss_3C, 1073741825U);
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
        var_r31 = 0;
        while (var_r31 < 60) {
            HuPrcVSleep();
            var_r31 += 1;
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
            var_r30 = 0;
            while (var_r30 < 4) {
                CharMotionSet((s16) lbl_1_bss_49C[var_r30], lbl_1_bss_44C[var_r30][3]);
                Hu3DModelRotSet((s16) lbl_1_bss_48C[var_r30], 0.0f, 0.0f, 0.0f);
                Hu3DModelAttrReset((s16) lbl_1_bss_48C[var_r30], 1073741825U);
                var_r30 += 1;
            }
            MgSeqModeNext();
        }
        lbl_1_bss_14 += 1;
        /* fallthrough */
    case 19:
    default:
        break;
    }
}

void fn_1_2BC8(s16 mode, s16 frameNo)
{
    if ((s32) lbl_1_bss_0 != -1) {
        if ((s32) lbl_1_bss_14 == 0) {
            CharMotionShiftSet((s16) lbl_1_bss_49C[lbl_1_bss_0], lbl_1_bss_44C[lbl_1_bss_0][2], 0.0f, 10.0f, 0U);
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

void fn_1_2C94(OMOBJ *obj)
{
    s32 var_r31;

    var_r31 = 0;
    while (var_r31 < 4) {
        switch (lbl_1_bss_4AC.state) {
            case 9:
                break;
            default:
                fn_1_2CEC(var_r31);
                break;
        }
        var_r31 += 1;
    }
}

void fn_1_2CEC(s32 player)
{
    Point3d movePos;
    Point3d effectPos;
    Point3d fallPos;
    Point3d risePos;
    Point3d bouncePos;
    f32 angle;
    f32 fallTime;
    s32 x;
    s32 y;
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
            tentativeX = (lbl_1_bss_3BC[player] & 240) >> 4;
            tentativeY = lbl_1_bss_3BC[player] & 15;
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
                    CharMotionShiftSet((s16) lbl_1_bss_49C[player], lbl_1_bss_44C[player][1], 0.0f, 10.0f, 1073741825U);
                }
                lbl_1_bss_40C[player] = 20;
                lbl_1_bss_42C[player] = 2;
                Hu3DModelRotSet((s16) lbl_1_bss_48C[player], 0.0f, angle, 0.0f);
            } else {
                Hu3DModelRotSet((s16) lbl_1_bss_48C[player], 0.0f, angle, 0.0f);
                if (lbl_1_bss_44C[player][0] != CharMotionShiftIDGet((s16) lbl_1_bss_49C[player])) {
                    CharMotionShiftSet((s16) lbl_1_bss_49C[player], lbl_1_bss_44C[player][0], 0.0f, 5.0f, 0U);
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
                x = (lbl_1_bss_3BC[player] & 240) >> 4;
                y = lbl_1_bss_3BC[player] & 15;
                switch (lbl_1_bss_41C[player]) {
                case 2:
                    x++;
                    break;
                case 1:
                    x--;
                    break;
                case 4:
                    y++;
                    break;
                case 3:
                    y--;
                    break;
                }
                if (x >= 0 && x < 7 && y >= 0 && y < 7 &&
                    (Hu3DModelAttrGet(lbl_1_bss_4AC.tiles[player][x + y * 7]) & 1) == 0) {
                    effectX = (lbl_1_bss_3BC[player] & 240) >> 4;
                    effectY = lbl_1_bss_3BC[player] & 15;
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
                    Hu3DModelAttrReset((s16) lbl_1_bss_2C0[player], 1073741826U);
                    Hu3DModelAttrSet(lbl_1_bss_4AC.tiles[player][effectX + effectY * 7], 1U);
                    lbl_1_bss_30C[player] = 0;
                    CharMotionShiftSet((s16) lbl_1_bss_49C[player], lbl_1_bss_44C[player][4], 0.0f, 10.0f, 0U);
                    lbl_1_bss_42C[player] = 4;
                    HuAudFXPlay(1698);
                    lbl_1_bss_1C = 1;
                } else {
                    CharMotionShiftSet((s16) lbl_1_bss_49C[player], lbl_1_bss_44C[player][5], 0.0f, 10.0f, 0U);
                    omVibrate((s16) player, 20, 7, 3);
                    lbl_1_bss_42C[player] = 5;
                    lbl_1_bss_1C = 0;
                }
                lbl_1_bss_3EC[player] = 0;
                break;
            }
            x = (lbl_1_bss_3BC[player] & 240) >> 4;
            y = lbl_1_bss_3BC[player] & 15;
            switch (lbl_1_bss_41C[player]) {
            case 2:
                x++;
                break;
            case 1:
                x--;
                break;
            case 4:
                y++;
                break;
            case 3:
                y--;
                break;
            }
            lbl_1_bss_3BC[player] = (x << 4) | (y & 15);
            if (actionResult == 3) {
                Hu3DModelRotSet((s16) lbl_1_bss_48C[player], 0.0f, 0.0f, 0.0f);
                CharMotionSet((s16) lbl_1_bss_49C[player], lbl_1_bss_44C[player][0]);
                lbl_1_bss_42C[player] = 11;
                break;
            }
            if (x >= 0 && x < 7 && y >= 0 && y < 7) {
                Hu3DModelAttrReset(lbl_1_bss_4AC.tiles[player][x + y * 7], 1073741826U);
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
            CharMotionShiftSet((s16) lbl_1_bss_49C[player], lbl_1_bss_44C[player][5], 0.0f, 10.0f, 0U);
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
            HuAudFXPlay(1700);
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
            Hu3DModelAttrSet((s16) lbl_1_bss_2C0[player], 1073741826U);
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
            CharMotionShiftSet((s16) lbl_1_bss_49C[player], lbl_1_bss_44C[player][0], 0.0f, 10.0f, 0U);
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

s32 fn_1_4038(s32 player)
{
    s32 var_r29;
    s16 var_r31;
    s16 var_r30;

    var_r29 = 0;
    var_r31 = HuPadStkX[lbl_1_bss_43C[player]];
    var_r30 = HuPadStkY[lbl_1_bss_43C[player]];
    if (((s16) var_r31 < 30) && ((s16) var_r31 > -30)) {
        var_r31 = 0;
    }
    if (((s16) var_r30 < 30) && ((s16) var_r30 > -30)) {
        var_r30 = 0;
    }
    if (((s16) var_r31 != 0) || ((s16) var_r30 != 0)) {
        if ((s16) var_r31 > 0) {
            var_r29 = 2;
        } else if ((s16) var_r31 < 0) {
            var_r29 = 1;
        } else if ((s16) var_r30 < 0) {
            var_r29 = 4;
        } else if ((s16) var_r30 > 0) {
            var_r29 = 3;
        }
    }
    return var_r29;
}

s32 fn_1_4150(s32 player)
{
    s32 x, y;
    s32 blocked;
    s32 bit;
    s32 mask;
    s8 rowMask;

    if (((s32) lbl_1_bss_3AC[player] == 0) && ((s32) lbl_1_bss_41C[player] != 3)) {
        return 0;
    }
    x = (s32) (lbl_1_bss_3BC[player] & 240) >> 4;
    y = lbl_1_bss_3BC[player] & 15;
    switch (lbl_1_bss_41C[player]) {
    case 2:
        x += 1;
        break;
    case 1:
        x -= 1;
        break;
    case 4:
        y += 1;
        break;
    case 3:
        y -= 1;
        break;
    }
    rowMask = lbl_1_data_16B0[lbl_1_bss_8][y];
    if ((s32) lbl_1_bss_C != 0) {
        bit = x;
    } else {
        bit = 6 - x;
    }
    mask = 1 << bit;
    if ((mask & rowMask) != 0) {
        blocked = 1;
    } else {
        blocked = 0;
    }
    if (blocked != 0) {
        return 0;
    }
    return 1;
}

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

s32 fn_1_4480(s32 player)
{
    s32 spC;
    s32 sp8;
    s32 temp_r30;
    s32 temp_r29;
    s32 var_r28;

    temp_r30 = GwPlayerConf[player].comDif;
    var_r28 = lbl_1_bss_48[lbl_1_bss_3AC[player]];
    if ((s32) lbl_1_bss_3FC[player] > 0) {
        lbl_1_bss_3FC[player]--;
    }
    if ((s32) lbl_1_bss_3AC[player] != 0) {
        if (((s32) lbl_1_bss_48[lbl_1_bss_3AC[player]] == (s32) lbl_1_bss_48[lbl_1_bss_3AC[player] - 1]) && (((s32) lbl_1_data_30[temp_r30] == 0) || ((s32) lbl_1_data_30[temp_r30] != (s32) lbl_1_bss_3CC[player]))) {
            lbl_1_bss_3FC[player] = -1;
            return var_r28;
        }
        if ((s32) lbl_1_bss_3FC[player] == 0) {
            if (((s32) lbl_1_data_20[temp_r30] != 0) && ((s32) lbl_1_bss_48[lbl_1_bss_3AC[player] + 1] != 0) && ((s32) (rand8() % lbl_1_data_20[temp_r30]) == 0)) {
                while (1) {
                temp_r29 = rand8() % 4 + 1;
                switch (lbl_1_bss_48[lbl_1_bss_3AC[player] - 1]) {
                case 1:
                    if (temp_r29 == 2) {
                        continue;
                    }
                    break;
                case 2:
                    if (temp_r29 == 1) {
                        continue;
                    }
                    break;
                case 3:
                    if (temp_r29 == 4) {
                        continue;
                    }
                    break;
                case 4:
                    if (temp_r29 == 3) {
                        continue;
                    }
                    break;
                }
                spC = (s32) (lbl_1_bss_3BC[player] & 240) >> 4;
                sp8 = lbl_1_bss_3BC[player] & 15;
                fn_1_5EF0(temp_r29, &spC, &sp8);
                if (spC < 0) {
                    continue;
                }
                if (spC > 6) {
                    continue;
                }
                if (sp8 < 0) {
                    continue;
                }
                if (sp8 > 6) {
                    continue;
                }
                if (((s32) lbl_1_data_40[temp_r30] == 0) && ((u32) (Hu3DModelAttrGet(lbl_1_bss_4AC.tiles[player][spC + sp8 * 7]) & 1) != 0)) {
                    continue;
                }
                break;
                }
                var_r28 = temp_r29;
            }
            lbl_1_bss_3CC[player] = 0;
            lbl_1_bss_3FC[player] = -1;
            return var_r28;
        }
        if ((s32) lbl_1_bss_3FC[player] == -1) {
            lbl_1_bss_3FC[player] = lbl_1_data_0[temp_r30] + (rand8() % lbl_1_data_10[temp_r30]);
        }
        return 0;
    }
    return var_r28;
}

static s32 lbl_1_data_128[4] = { 1, 2, 4, 8 };
static s32 lbl_1_data_138[4] = { 0, 640, 0, 640 };
static s32 lbl_1_data_148[4] = { 0, 0, 480, 480 };
static s32 lbl_1_data_158[4] = { 640, 0, 640, 0 };
static s32 lbl_1_data_168[4] = { 480, 480, 0, 0 };

void fn_1_48D4(void)
{
    s32 var_r31;

    Hu3DCameraCreate(15);
    var_r31 = 0;
    while (var_r31 < 4) {
        Hu3DCameraPerspectiveSet(lbl_1_data_128[var_r31], 45.0f, 20.0f, 15000.0f, 1.2f);
        Hu3DCameraViewportSet(lbl_1_data_128[var_r31], (f32) lbl_1_data_138[var_r31], (f32) lbl_1_data_148[var_r31], (f32) lbl_1_data_158[var_r31], (f32) lbl_1_data_168[var_r31], 0.0f, 1.0f);
        Hu3DCameraScissorSet(lbl_1_data_128[var_r31], (u32) lbl_1_data_138[var_r31], (u32) lbl_1_data_148[var_r31], 640U, 480U);
        CenterM[var_r31].x = 0.0f;
        CenterM[var_r31].y = 150.0f;
        CenterM[var_r31].z = 0.0f;
        CRotM[var_r31].x = -50.0f;
        CRotM[var_r31].y = 0.0f;
        CRotM[var_r31].z = 0.0f;
        CZoomM[var_r31] = 700.0f;
        var_r31 += 1;
    }
}

void fn_1_4B94(s32 arg0)
{
    s32 scissor[4][4];
    s32 viewport[4][4];
    f32 time;
    f32 offsetTime;
    s32 width;
    s32 height;
    s32 viewHeight;
    s32 i;
    s32 shiftY;

    time = 0.016666668f * (f32) (lbl_1_bss_14 + 60);
    offsetTime = 0.016666666666666666 * (f64) lbl_1_bss_14;
    width = 320.0f * time;
    height = 240.0f * time;
    viewHeight = 280.0f * time;
    shiftY = 280.0f * offsetTime;
    switch (arg0) {
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
    i = 0;
    while (i < 4) {
        Hu3DCameraViewportSet(lbl_1_data_74[i], viewport[i][2], viewport[i][3], viewport[i][0], viewport[i][1], 0.0f, 1.0f);
        Hu3DCameraScissorSet(lbl_1_data_74[i], scissor[i][2] + 2, scissor[i][3] + 2, scissor[i][0] - 2, scissor[i][1] - 2);
        i += 1;
    }
}

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

void fn_1_54D4(OMOBJ *obj)
{
    Point3d sp8;
    f32 temp_f31;
    f32 temp_f30;
    s32 var_r31;

    var_r31 = 0;
    while (var_r31 < 4) {
        switch (lbl_1_bss_36C[var_r31]) {
        case 0:
            lbl_1_bss_36C[var_r31] = 1;
            lbl_1_bss_37C[var_r31] = 0;
            break;
        case 2:
            Hu3DModelPosGet((s16) lbl_1_bss_48C[var_r31], &sp8);
            sp8.y = 600.0f;
            Hu3DModelPosSetV((s16) lbl_1_bss_39C[var_r31], &sp8);
            lbl_1_bss_37C[var_r31] = 0;
            lbl_1_bss_36C[var_r31] = 3;
            break;
        case 3:
            lbl_1_bss_37C[var_r31]++;
            if ((s32) lbl_1_bss_37C[var_r31] > 30) {
                lbl_1_bss_37C[var_r31] = 0;
                lbl_1_bss_36C[var_r31] = 4;
                Hu3DModelAttrReset((s16) lbl_1_bss_39C[var_r31], 1U);
            }
            break;
        case 4:
            temp_f31 = 0.033333335f * (f32) lbl_1_bss_37C[var_r31];
            Hu3DModelPosGet((s16) lbl_1_bss_39C[var_r31], &sp8);
            sp8.y = (1300.0f - (1300.0f * temp_f31)) - 700.0f;
            Hu3DModelPosSetV((s16) lbl_1_bss_39C[var_r31], &sp8);
            if ((s32) lbl_1_bss_37C[var_r31] == 30) {
                lbl_1_bss_37C[var_r31] = 0;
                lbl_1_bss_36C[var_r31] = 5;
                HuAudFXPlay(1699);
            } else {
                lbl_1_bss_37C[var_r31]++;
            }
            break;
        case 5:
            temp_f30 = 0.033333335f * (f32) lbl_1_bss_37C[var_r31];
            Hu3DModelPosGet((s16) lbl_1_bss_39C[var_r31], &sp8);
            sp8.y = (1300.0f * temp_f30) - 700.0f;
            Hu3DModelPosSetV((s16) lbl_1_bss_39C[var_r31], &sp8);
            if ((s32) lbl_1_bss_37C[var_r31] == 30) {
                Hu3DModelAttrSet((s16) lbl_1_bss_39C[var_r31], 1U);
                lbl_1_bss_37C[var_r31] = 0;
                lbl_1_bss_36C[var_r31] = 1;
            } else {
                lbl_1_bss_37C[var_r31]++;
            }
            break;
        }
        var_r31 += 1;
    }
}

void fn_1_5924(OMOBJ *obj)
{
    Point3d sp8;
    s32 var_r31;
    s32 var_r30;
    float angle;

        switch (lbl_1_bss_308) {
        case 0:                                     /* switch 1 */
            Hu3DMotionSet((s16) lbl_1_bss_300, (s16) lbl_1_bss_2F8);
            *lbl_1_bss_3BC = 55;
            lbl_1_bss_308 = 1;
            break;
        case 1:
            break;
        case 2:                                     /* switch 1 */
            if ((s32) lbl_1_bss_48[lbl_1_bss_2F0] != 0) {
                angle = fn_1_5E00(lbl_1_bss_48[lbl_1_bss_2F0]);
                Hu3DModelRotSet((s16) lbl_1_bss_300, 0.0f, angle, 0.0f);
                lbl_1_bss_304 = 10;
                lbl_1_bss_308 = 3;
                return;
            }
            lbl_1_bss_308 = 4;
            return;
        case 3:                                     /* switch 1 */
            lbl_1_bss_304 -= 1;
            if ((s32) lbl_1_bss_304 >= 0) {
                Hu3DModelPosGet((s16) lbl_1_bss_300, &sp8);
                switch (lbl_1_bss_48[lbl_1_bss_2F0]) {
                case 1:                             /* switch 2 */
                    sp8.x -= 20.0f;
                    break;
                case 2:                             /* switch 2 */
                    sp8.x += 20.0f;
                    break;
                case 3:                             /* switch 2 */
                    sp8.z -= 20.0f;
                    break;
                case 4:                             /* switch 2 */
                    sp8.z += 20.0f;
                    break;
                }
                Hu3DModelPosSetV((s16) lbl_1_bss_300, (Point3d *) &sp8);
                return;
            }
            lbl_1_bss_308 = 2;
            var_r31 = (s32) (*lbl_1_bss_3BC & 240) >> 4;
            var_r30 = *lbl_1_bss_3BC & 15;
            switch (lbl_1_bss_48[lbl_1_bss_2F0]) {
            case 1:                                 /* switch 3 */
                var_r31 -= 1;
                break;
            case 2:                                 /* switch 3 */
                var_r31 += 1;
                break;
            case 4:                                 /* switch 3 */
                var_r30 += 1;
                break;
            case 3:                                 /* switch 3 */
                var_r30 -= 1;
                break;
            }
            if ((var_r31 >= 0) && (var_r31 < 7) && (var_r30 >= 0) && (var_r30 < 7)) {
                Hu3DModelAttrReset(lbl_1_bss_4AC.tiles[0][var_r31 + (var_r30 * 7)], 1073741826U);
                *lbl_1_bss_3BC = (var_r31 << 4) | (var_r30 & 15);
            }
            lbl_1_bss_2F0 += 1;
            HuAudFXPlayVolPan(1702, (s16) (32.0f + (11.142858f * (f32) var_r30)), (s16) (48.0f + (4.571429f * (f32) var_r31)));
            return;
        case 4:                                     /* switch 1 */
            Hu3DModelRotSet((s16) lbl_1_bss_300, 0.0f, 0.0f, 0.0f);
            lbl_1_bss_14 = 0;
            lbl_1_bss_308 = 6;
            return;
        case 5:                                     /* switch 1 */
            if ((s32) lbl_1_bss_14 > 300) {
                lbl_1_bss_308 = 6;
            }
            lbl_1_bss_14 += 1;
            /* fallthrough */
        case 6:
        default:
            break;
    }
}

float fn_1_5E00(s32 direction)
{
    f32 var_f31;

    var_f31 = 0.0f;
    switch (direction) {                            /* irregular */
    case 1:
        var_f31 = 270.0f;
        break;
    case 2:
        var_f31 = 90.0f;
        break;
    case 3:
        var_f31 = 180.0f;
        break;
    case 4:
        var_f31 = 0.0f;
        break;
    }
    return var_f31;
}

void fn_1_5E90(s32 direction, Point3d *pos, f32 distance)
{
    switch (direction) {                            /* irregular */
    case 1:
        pos->x -= distance;
        break;
    case 2:
        pos->x = distance;
        break;
    case 3:
        pos->z -= distance;
        break;
    case 4:
        pos->z += distance;
        break;
    }
}

void fn_1_5EF0(s32 direction, s32 *x, s32 *y)
{
    switch (direction) {                            /* irregular */
    case 1:
        *x -= 1;
        break;
    case 2:
        *x += 1;
        break;
    case 3:
        *y -= 1;
        break;
    case 4:
        *y += 1;
        break;
    }
}

void fn_1_5F58(s32 player)
{
    s32 var_r29;
    s32 var_r31;
    s32 var_r30;

    var_r29 = 0;
    var_r31 = 3;
    var_r30 = 7;
    while ((s32) lbl_1_bss_48[var_r29] != 0) {
        switch (lbl_1_bss_48[var_r29]) {
        case 1:
            var_r31 -= 1;
            break;
        case 2:
            var_r31 += 1;
            break;
        case 3:
            var_r30 -= 1;
            break;
        case 4:
            var_r30 += 1;
            break;
        }
        if ((var_r31 >= 0) && (var_r31 < 7) && (var_r30 >= 0) && (var_r30 < 7)) {
            Hu3DModelAttrReset(lbl_1_bss_4AC.tiles[player][var_r31 + var_r30 * 7], 1073741826U);
        }
        var_r29 += 1;
    }
}
