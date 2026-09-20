#include "REL/m618dll.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/frand.h"
#include "game/gamework.h"
#include "game/hsfex.h"

int lbl_1_data_28[4] = { 1, 2, 4, 8 };
unsigned int lbl_1_data_38[4] = { DATANUM(DATA_mariomot, 6), DATANUM(DATA_mariomot, 34), DATANUM(DATA_mariomot, 40), DATANUM(DATA_mariomot, 63) };
HuVec2f lbl_1_data_48[4] = {
    { 0.0f, 0.0f },
    { 3208.01099f, 0.0f },
    { 4346.36816f, 1672.92297f },
    { 996.393799f, 3609.70898f },
};

int lbl_1_bss_558;
int lbl_1_bss_554;
int lbl_1_bss_550;
int lbl_1_bss_54C;
int lbl_1_bss_548;
MGTIMER *lbl_1_bss_544;
HU3D_MODELID lbl_1_bss_540[2];
int lbl_1_bss_53C;
OM_CAMERA_VIEW lbl_1_bss_4CC[4];
float lbl_1_bss_4C8;
M618Model lbl_1_bss_30C[37];
HU3D_MODELID lbl_1_bss_2F4[4][3];
HU3D_MODELID lbl_1_bss_2CC[4][5];
HU3D_MODELID lbl_1_bss_2BC[4][2];
HU3D_MODELID lbl_1_bss_2B4[2][2];
M618Player lbl_1_bss_114[4];
M618Input lbl_1_bss_84[4];
int lbl_1_bss_44[4][4];
int lbl_1_bss_40;
unsigned char m618UnknownBss3C[4]; /* Retail interval; original purpose unknown. */
int lbl_1_bss_38;
HuVecF lbl_1_bss_8[4];

void fn_1_378(void)
{
    fn_1_52D0();
    fn_1_1CE0();
    lbl_1_bss_558 = 0;
    lbl_1_bss_554 = 0;
    lbl_1_bss_550 = 0;
    lbl_1_bss_54C = -1;
    lbl_1_bss_548 = 0;
    lbl_1_bss_4C8 = 0.0f;
    fn_1_1B40(0);
    lbl_1_bss_53C = GWRecordGet(2);
    if (lbl_1_bss_53C == 0) {
        lbl_1_bss_53C = 7200;
    }
    lbl_1_bss_544 = MgTimerCreate(1);
    MgTimerParamSet(lbl_1_bss_544, 0, 18000, lbl_1_bss_53C);
    MgTimerRecordDispOn(lbl_1_bss_544);
    lbl_1_bss_44[0][0] = lbl_1_bss_44[1][0] = lbl_1_bss_44[2][0] = lbl_1_bss_44[3][0] =
        lbl_1_bss_44[0][1] = lbl_1_bss_44[1][1] = lbl_1_bss_44[2][1] = lbl_1_bss_44[3][1] =
        lbl_1_bss_44[0][2] = lbl_1_bss_44[1][2] = lbl_1_bss_44[2][2] = lbl_1_bss_44[3][2] =
        lbl_1_bss_44[0][3] = lbl_1_bss_44[1][3] = lbl_1_bss_44[2][3] = lbl_1_bss_44[3][3] = -1;
    lbl_1_bss_40 = -1;
    lbl_1_bss_38 = -1;
}

int fn_1_570(void)
{
    int done = 0;
    if (lbl_1_bss_550++ == 1) {
        MgTimerModeOnSet(lbl_1_bss_544, 0);
        lbl_1_bss_550 = 1;
    }
    if (lbl_1_bss_550 > 1) {
        lbl_1_bss_550 = 99;
    }
    fn_1_214C();
    MgActorExec();
    fn_1_4BE0();
    if (lbl_1_bss_54C != -1) {
        lbl_1_bss_4C8 = 0.0f;
        done = 1;
    }
    return done;
}

void fn_1_644(void)
{
    int player;
    float destX;
    if (lbl_1_bss_38 == -1) {
        lbl_1_bss_38 = HuAudFXPlay(1758);
    }
    for (player = 0; player < 4; player++) {
        destX = 160.0f + lbl_1_data_48[lbl_1_bss_114[player].unk5C].x;
        if (lbl_1_bss_114[player].player->actor->pos.x < destX) {
            MgPlayerPadSet(lbl_1_bss_114[player].player, 10, 0, 0, 0);
            if (lbl_1_bss_114[player].player->actor->pos.x >= destX) {
                lbl_1_bss_114[player].player->actor->pos.x = destX;
            }
        } else {
            MgPlayerAttrSet(lbl_1_bss_114[player].player, 1);
            lbl_1_bss_114[player].unk34 = 0;
            lbl_1_bss_114[player].pos38 = lbl_1_bss_114[player].player->actor->pos;
            lbl_1_bss_114[player].pos38.y += 300.0f;
            lbl_1_bss_114[player].pos38.z -= 160.0f;
        }
    }
    MgActorExec();
    fn_1_4BE0();
}

void fn_1_84C(void)
{
    if (++lbl_1_bss_558 >= 120) {
        MgPlayerAttrReset(lbl_1_bss_114[0].player, 1);
        MgPlayerAttrReset(lbl_1_bss_114[1].player, 1);
        MgPlayerAttrReset(lbl_1_bss_114[2].player, 1);
        MgPlayerAttrReset(lbl_1_bss_114[3].player, 1);
        fn_1_214C();
        MgActorExec();
        fn_1_4BE0();
    }
    if (lbl_1_bss_40 == -1) {
        lbl_1_bss_40 = HuAudBGMPlay(82);
    }
}

void fn_1_910(void)
{
    int recordTime;

    if (lbl_1_bss_554 == 0) {
        MgTimerModeOffSet(lbl_1_bss_544);
        if (lbl_1_bss_54C == -1) {
            MgSeqWinnerSet(-1, -1, -1, -1);
            lbl_1_bss_558 = 0;
        } else {
            MgSeqWinnerSet(GwPlayerConf[lbl_1_bss_54C].charNo, -1, -1, -1);
            GWMgCoinBonusSet(lbl_1_bss_54C, 10);
            lbl_1_bss_558 = 120;
            recordTime = MgTimerValueGet(lbl_1_bss_544);
            if ((_CheckFlag(FLAG_MG_PRACTICE) == 0) && (GwPlayerConf[lbl_1_bss_54C].type == 0) && (recordTime < lbl_1_bss_53C)) {
                MgSeqRecordSet(recordTime);
                lbl_1_bss_558 += 150;
                GWRecordSet(GW_RECORD_M618, (u32) recordTime);
            }
        }
        MgPlayerAttrSet(lbl_1_bss_114->player, 1U);
        MgPlayerAttrSet(lbl_1_bss_114[1].player, 1U);
        MgPlayerAttrSet(lbl_1_bss_114[2].player, 1U);
        MgPlayerAttrSet(lbl_1_bss_114[3].player, 1U);
        lbl_1_bss_554 = 1;
        fn_1_6298();
        HuAudSStreamFadeOut(lbl_1_bss_40, 100);
    }
    if ((lbl_1_bss_114->unk08 != 0) || ((u32) (lbl_1_bss_114->player->actor->colGroundAttr & 16511) == 0) || (lbl_1_bss_114[1].unk08 != 0) || ((u32) (lbl_1_bss_114[1].player->actor->colGroundAttr & 16511) == 0) || (lbl_1_bss_114[2].unk08 != 0) || ((u32) (lbl_1_bss_114[2].player->actor->colGroundAttr & 16511) == 0) || (lbl_1_bss_114[3].unk08 != 0) || ((u32) (lbl_1_bss_114[3].player->actor->colGroundAttr & 16511) == 0)) {
        if (lbl_1_bss_114->unk08 == 3) {
            MgPlayerAttrReset(lbl_1_bss_114->player, 1U);
        }
        if (lbl_1_bss_114[1].unk08 == 3) {
            MgPlayerAttrReset(lbl_1_bss_114[1].player, 1U);
        }
        if (lbl_1_bss_114[2].unk08 == 3) {
            MgPlayerAttrReset(lbl_1_bss_114[2].player, 1U);
        }
        if (lbl_1_bss_114[3].unk08 == 3) {
            MgPlayerAttrReset(lbl_1_bss_114[3].player, 1U);
        }
        fn_1_214C();
        if (lbl_1_bss_114->unk08 != 3) {
            MgPlayerAttrSet(lbl_1_bss_114->player, 1U);
        }
        if (lbl_1_bss_114[1].unk08 != 3) {
            MgPlayerAttrSet(lbl_1_bss_114[1].player, 1U);
        }
        if (lbl_1_bss_114[2].unk08 != 3) {
            MgPlayerAttrSet(lbl_1_bss_114[2].player, 1U);
        }
        if (lbl_1_bss_114[3].unk08 != 3) {
            MgPlayerAttrSet(lbl_1_bss_114[3].player, 1U);
        }
    }
    if (lbl_1_bss_54C != -1) {
        lbl_1_bss_114[lbl_1_bss_54C].player->actor->pos = lbl_1_bss_8[lbl_1_bss_54C];
    }
    MgActorExec();
    fn_1_4BE0();
}

int fn_1_DA8(void)
{
    s32 winner;

    winner = lbl_1_bss_54C;
    if ((s32) lbl_1_bss_54C != -1) {
        fn_1_796C(lbl_1_bss_54C);
        if (++lbl_1_bss_554 >= 30) {
            if ((s32) lbl_1_bss_554 == 30) {
                MgPlayerAttrReset(lbl_1_bss_114[lbl_1_bss_54C].player, 1U);
                MgPlayerPadSet(lbl_1_bss_114[lbl_1_bss_54C].player, 0, 0, 256, 0);
            } else if ((u32) (lbl_1_bss_114[lbl_1_bss_54C].player->actor->colGroundAttr & 16511) == 0) {
                MgPlayerPadSet(lbl_1_bss_114[lbl_1_bss_54C].player, 0, 0, 0, 256);
            } else {
                MgPlayerAttrSet(lbl_1_bss_114[lbl_1_bss_54C].player, 1U);
            }
            if (lbl_1_bss_114[lbl_1_bss_54C].player->actor->rotY > 0.0f) {
                lbl_1_bss_114[lbl_1_bss_54C].player->actor->rotY -= 5.0f;
                if (lbl_1_bss_114[lbl_1_bss_54C].player->actor->rotY < 0.0f) {
                    lbl_1_bss_114[lbl_1_bss_54C].player->actor->rotY = 0.0f;
                }
            } else if (lbl_1_bss_114[lbl_1_bss_54C].player->actor->rotY < 0.0f) {
                lbl_1_bss_114[lbl_1_bss_54C].player->actor->rotY += 5.0f;
                if (lbl_1_bss_114[lbl_1_bss_54C].player->actor->rotY > 0.0f) {
                    lbl_1_bss_114[lbl_1_bss_54C].player->actor->rotY = 0.0f;
                }
            }
        }
        if ((s32) lbl_1_bss_554 == 60) {
            Hu3DModelCameraSet(lbl_1_bss_30C[28].model, (u16) lbl_1_data_28[lbl_1_bss_54C]);
            Hu3DModelCameraSet(lbl_1_bss_30C[29].model, (u16) lbl_1_data_28[lbl_1_bss_54C]);
            Hu3DModelCameraSet(lbl_1_bss_30C[30].model, (u16) lbl_1_data_28[lbl_1_bss_54C]);
            Hu3DModelCameraSet(lbl_1_bss_30C[31].model, (u16) lbl_1_data_28[lbl_1_bss_54C]);
            Hu3DModelCameraSet(lbl_1_bss_30C[32].model, (u16) lbl_1_data_28[lbl_1_bss_54C]);
            Hu3DModelCameraSet(lbl_1_bss_30C[33].model, (u16) lbl_1_data_28[lbl_1_bss_54C]);
            Hu3DModelCameraSet(lbl_1_bss_30C[34].model, (u16) lbl_1_data_28[lbl_1_bss_54C]);
            Hu3DModelCameraSet(lbl_1_bss_30C[35].model, (u16) lbl_1_data_28[lbl_1_bss_54C]);
            Hu3DModelCameraSet(lbl_1_bss_30C[36].model, (u16) lbl_1_data_28[lbl_1_bss_54C]);
            Hu3DModelCameraSet(lbl_1_bss_2F4[3][0], (u16) lbl_1_data_28[lbl_1_bss_54C]);
            Hu3DModelCameraSet(lbl_1_bss_2F4[3][1], (u16) lbl_1_data_28[lbl_1_bss_54C]);
            Hu3DModelCameraSet(lbl_1_bss_2F4[3][2], (u16) lbl_1_data_28[lbl_1_bss_54C]);
            Hu3DModelCameraSet(lbl_1_bss_2CC[3][0], (u16) lbl_1_data_28[lbl_1_bss_54C]);
            Hu3DModelCameraSet(lbl_1_bss_2CC[3][1], (u16) lbl_1_data_28[lbl_1_bss_54C]);
            Hu3DModelCameraSet(lbl_1_bss_2CC[3][2], (u16) lbl_1_data_28[lbl_1_bss_54C]);
            Hu3DModelCameraSet(lbl_1_bss_2BC[3][0], (u16) lbl_1_data_28[lbl_1_bss_54C]);
            Hu3DModelCameraSet(lbl_1_bss_2B4[1][0], (u16) lbl_1_data_28[lbl_1_bss_54C]);
            Hu3DModelCameraSet(lbl_1_bss_2B4[1][1], (u16) lbl_1_data_28[lbl_1_bss_54C]);
        }
        if ((s32) lbl_1_bss_554 == 80) {
            Hu3DModelAttrReset(lbl_1_bss_540[0], 1U);
            Hu3DMotionSpeedSet(lbl_1_bss_540[0], 1.0f);
        }
        if ((s32) lbl_1_bss_554 == 120) {
            Hu3DModelAttrReset(lbl_1_bss_540[1], 1U);
            Hu3DMotionSpeedSet(lbl_1_bss_540[1], 1.0f);
        }
        if (((s32) lbl_1_bss_554 == 90) && (_CheckFlag(FLAG_MG_PRACTICE) == 0) && (GwPlayerConf[lbl_1_bss_54C].type == 0) && (MgTimerValueGet(lbl_1_bss_544) < (s32) lbl_1_bss_53C)) {
            MgTimerRecordSet(lbl_1_bss_544, -1);
        }
        lbl_1_bss_558 -= 1;
        if ((s32) lbl_1_bss_558 < 0) {
            lbl_1_bss_558 = -1;
        }
        if ((s32) lbl_1_bss_558 == 0) {
            CharMotionShiftSet(GwPlayerConf[lbl_1_bss_54C].charNo, lbl_1_bss_114[lbl_1_bss_54C].motion[0], 0.0f, 8.0f, 0U);
            winner = -1;
        }
        if ((lbl_1_bss_114->unk08 == 1) && (lbl_1_bss_114->unk10 == 1)) {
            lbl_1_bss_114->player->actor->velY = 0.0f;
        }
        if ((lbl_1_bss_114[1].unk08 == 1) && (lbl_1_bss_114[1].unk10 == 1)) {
            lbl_1_bss_114[1].player->actor->velY = 0.0f;
        }
        if ((lbl_1_bss_114[2].unk08 == 1) && (lbl_1_bss_114[2].unk10 == 1)) {
            lbl_1_bss_114[2].player->actor->velY = 0.0f;
        }
        if ((lbl_1_bss_114[3].unk08 == 1) && (lbl_1_bss_114[3].unk10 == 1)) {
            lbl_1_bss_114[3].player->actor->velY = 0.0f;
        }
    } else {
        MgPlayerAttrSet(lbl_1_bss_114->player, 1U);
        MgPlayerAttrSet(lbl_1_bss_114[1].player, 1U);
        MgPlayerAttrSet(lbl_1_bss_114[2].player, 1U);
        MgPlayerAttrSet(lbl_1_bss_114[3].player, 1U);
        if ((lbl_1_bss_114->unk08 == 0) && ((u32) (lbl_1_bss_114->player->actor->colGroundAttr & 16511) != 0) && (lbl_1_bss_114[1].unk08 == 0) && ((u32) (lbl_1_bss_114[1].player->actor->colGroundAttr & 16511) != 0) && (lbl_1_bss_114[2].unk08 == 0) && ((u32) (lbl_1_bss_114[2].player->actor->colGroundAttr & 16511) != 0) && (lbl_1_bss_114[3].unk08 == 0) && ((u32) (lbl_1_bss_114[3].player->actor->colGroundAttr & 16511) != 0)) {
            if (++lbl_1_bss_558 < 15) {
                winner = -2;
                MgSeqModeChangeOff();
            } else {
                CharMotionShiftSet(GwPlayerConf->charNo, lbl_1_bss_114->motion[2], 0.0f, 8.0f, 0U);
                CharMotionShiftSet(GwPlayerConf[1].charNo, lbl_1_bss_114[1].motion[2], 0.0f, 8.0f, 0U);
                CharMotionShiftSet(GwPlayerConf[2].charNo, lbl_1_bss_114[2].motion[2], 0.0f, 8.0f, 0U);
                CharMotionShiftSet(GwPlayerConf[3].charNo, lbl_1_bss_114[3].motion[2], 0.0f, 8.0f, 0U);
                MgSeqModeChangeOn();
            }
        } else {
            winner = -2;
            MgSeqModeChangeOff();
            if (lbl_1_bss_114->unk08 == 3) {
                MgPlayerAttrReset(lbl_1_bss_114->player, 1U);
            }
            if (lbl_1_bss_114[1].unk08 == 3) {
                MgPlayerAttrReset(lbl_1_bss_114[1].player, 1U);
            }
            if (lbl_1_bss_114[2].unk08 == 3) {
                MgPlayerAttrReset(lbl_1_bss_114[2].player, 1U);
            }
            if (lbl_1_bss_114[3].unk08 == 3) {
                MgPlayerAttrReset(lbl_1_bss_114[3].player, 1U);
            }
            fn_1_214C();
            if (lbl_1_bss_114->unk08 != 3) {
                MgPlayerAttrSet(lbl_1_bss_114->player, 1U);
            }
            if (lbl_1_bss_114[1].unk08 != 3) {
                MgPlayerAttrSet(lbl_1_bss_114[1].player, 1U);
            }
            if (lbl_1_bss_114[2].unk08 != 3) {
                MgPlayerAttrSet(lbl_1_bss_114[2].player, 1U);
            }
            if (lbl_1_bss_114[3].unk08 != 3) {
                MgPlayerAttrSet(lbl_1_bss_114[3].player, 1U);
            }
        }
    }
    MgActorExec();
    fn_1_4BE0();
    return winner;
}

void fn_1_1B40(int mode)
{
    int player;

    if (mode == 0) {
        for (player = 0; player < 4; player++) {
            lbl_1_bss_4CC[player].center = lbl_1_bss_114[player].player->actor->pos;
            lbl_1_bss_4CC[player].center.x += 300.0f;
            lbl_1_bss_4CC[player].center.y += 75.0f;
            lbl_1_bss_4CC[player].rot = (HuVecF){ -5.0f, 0.0f, 0.0f };
            lbl_1_bss_4CC[player].zoom = 7000.0f;
            omCameraViewSetMulti(lbl_1_data_28[player], &lbl_1_bss_4CC[player]);
        }
        lbl_1_bss_4C8 = 0.0f;
    }
    fn_1_796C(-1);
}

int fn_1_1CD8(void)
{
    return 60;
}

void fn_1_1CE0(void)
{
    MGACTOR_PARAM param;
    HuVecF pos;
    s16 character;
    int count;
    int motion;
    int player;

    count = 0;
    param.height = 150.0f;
    param.radius = 40.0f;
    param.param = 0;
    param.type = 0;
    param.attr = 4;
    param.narrowHook = NULL;
    param.correctHookParam = 0;
    param.correctHook = NULL;
    for (player = 0; player < 4; player++) {
        lbl_1_bss_114[player].player = MgPlayerCreate(player, &param, 4, lbl_1_data_28[player], 3, NULL);
        lbl_1_bss_114[player].player->actor->pos.x = lbl_1_data_48[0].x;
        lbl_1_bss_114[player].player->actor->pos.y = lbl_1_data_48[0].y;
        lbl_1_bss_114[player].player->actor->pos.z = 0.0f;
        lbl_1_bss_114[player].player->actor->rotY = 90.0f;
        lbl_1_bss_114[player].unk04 = 1;
        lbl_1_bss_114[player].unk5C = 0;
        Hu3DModelCameraSet(lbl_1_bss_114[player].player->actor->mdlId, lbl_1_data_28[player]);
        lbl_1_bss_114[player].unk08 = 0;
        lbl_1_bss_114[player].unk0C = 0;
        character = GwPlayerConf[player].charNo;
        for (motion = 0; motion < 4; motion++) {
            lbl_1_bss_114[player].motion[motion] = CharMotionCreate(character, lbl_1_data_38[motion]);
        }
        CharMotionDataClose(character);
        lbl_1_bss_114[player].model = Hu3DModelCreateData(DATANUM(DATA_m618, 0));
        Hu3DModelCameraSet(lbl_1_bss_114[player].model, lbl_1_data_28[player]);
        Hu3DModelPosGet(lbl_1_bss_114[player].player->actor->mdlId, &pos);
        Hu3DModelPosSetV(lbl_1_bss_114[player].model, &pos);
        Hu3DModelRotSet(lbl_1_bss_114[player].model, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet(lbl_1_bss_114[player].model, HU3D_MOTATTR_LOOP);
        Hu3DModelLayerSet(lbl_1_bss_114[player].model, 2);
        Hu3DModelAttrSet(lbl_1_bss_114[player].model, HU3D_ATTR_DISPOFF);
        lbl_1_bss_84[player].unk00 = GwPlayerConf[player].comDif;
        lbl_1_bss_84[player].unk20 = frandmod(3) == 0U ? 46 : (frandmod(3) == 1U ? 41 : 56);
        if (lbl_1_bss_84[player].unk20 == 56) {
            count++;
        }
    }
    if (count == 0) {
        lbl_1_bss_84[frandmod(4)].unk20 = 56;
    }
    MgActorExec();
}

void fn_1_214C(void)
{
    s32 finishPlayers[4];
    Point3d worldPos;
    Point3d screenPos;
    f32 rotation;
    f32 minX;
    f32 maxX;
    s32 resetPosition;
    s32 finishCount;
    s32 playerNo;

    finishCount = 0;
    playerNo = 0;
    while (playerNo < 4) {
        if (lbl_1_bss_114[playerNo].unk08 == 3) {
            fn_1_3294(playerNo);
        } else if (lbl_1_bss_114[playerNo].unk08 == 1) {
            fn_1_44CC(playerNo);
            lbl_1_bss_44[playerNo][3] = -1;
        } else {
            if (GwPlayerConf[playerNo].type != 0) {
                fn_1_8180(playerNo);
            }
            worldPos = lbl_1_bss_114[playerNo].player->actor->pos;
            Hu3D3Dto2D(&worldPos, (s16) lbl_1_data_28[playerNo], &screenPos);
            if ((((lbl_1_bss_114[playerNo].unk5C != 2) && (screenPos.y > 480.0f)) || ((lbl_1_bss_114[playerNo].unk5C == 2) && (screenPos.y > 480.0f) && (worldPos.y < 1600.0f))) && ((s32) lbl_1_bss_44[playerNo][3] == -1)) {
                lbl_1_bss_44[playerNo][3] = HuAudFXPlay(playerNo == 0 ? 1749 : (playerNo == 1 ? 1750 : (playerNo == 2 ? 1751 : 1752)));
                MgPlayerAttrSet(lbl_1_bss_114[playerNo].player, 1U);
                if (lbl_1_bss_114[playerNo].unk08 != 2) {
                    CharFXPlay(GwPlayerConf[playerNo].charNo, 576);
                }
            }
            if (((u32) (lbl_1_bss_114[playerNo].player->actor->colGroundAttr & 16511) == 0) || (lbl_1_bss_114[playerNo].unk08 == 2)) {
                if (lbl_1_bss_114[playerNo].unk08 != 1) {
                    if (++lbl_1_bss_114[playerNo].unk0C == 100) {
                        if (lbl_1_bss_114[playerNo].unk08 == 2) {
                            MgPlayerSpawn(lbl_1_bss_114[playerNo].player, &lbl_1_bss_114[playerNo].player->actor->pos);
                        }
                        lbl_1_bss_114[playerNo].unk08 = 1;
                        lbl_1_bss_114[playerNo].unk0C = 0;
                        lbl_1_bss_114[playerNo].unk10 = 0;
                        lbl_1_bss_114[playerNo].unk30 = 0.0f;
                        MgPlayerAttrSet(lbl_1_bss_114[playerNo].player, 1U);
                        if ((lbl_1_bss_114[playerNo].unk5C == 2) && (lbl_1_bss_114[playerNo].player->actor->pos.x < 4850.0f)) {
                            lbl_1_bss_114[playerNo].player->actor->rotY = -90.0f;
                            lbl_1_bss_114[playerNo].unk04 = -1;
                        }
                    }
                }
            } else {
                lbl_1_bss_114[playerNo].unk08 = 0;
                lbl_1_bss_114[playerNo].unk0C = 0;
                MgPlayerAttrReset(lbl_1_bss_114[playerNo].player, 1U);
                if (((u32) (lbl_1_bss_114[playerNo].player->actor->colGroundAttr & 16511) != 0) && (((lbl_1_bss_114[playerNo].unk5C == 0) && (lbl_1_bss_114[playerNo].player->actor->pos.x > 2300.0f)) || ((lbl_1_bss_114[playerNo].unk5C == 1) && (lbl_1_bss_114[playerNo].player->actor->pos.x >= 5955.0f)) || ((lbl_1_bss_114[playerNo].unk5C == 2) && (lbl_1_bss_114[playerNo].player->actor->pos.x <= 5030.0f) && (lbl_1_bss_114[playerNo].player->actor->pos.y >= 3460.0f)))) {
                    lbl_1_bss_114[playerNo].unk08 = 3;
                    lbl_1_bss_114[playerNo].unk10 = 0;
                    lbl_1_bss_114[playerNo].unk14 = 0;
                    omVibrate((s16) playerNo, 20, 4, 4);
                }
                if (((s32) lbl_1_bss_548 == 0) && (lbl_1_bss_114[playerNo].unk5C == 3) && ((u32) (lbl_1_bss_114[playerNo].player->actor->colGroundAttr & 16511) != 0)) {
                    if ((lbl_1_bss_114[playerNo].player->actor->pos.x > 1400.90002f) && (lbl_1_bss_114[playerNo].player->actor->pos.x < 1401.0f)) {
                        lbl_1_bss_114[playerNo].player->actor->pos.x = 1401.0f;
                    }
                    if ((lbl_1_bss_114[playerNo].player->actor->pos.x <= 1400.91199f) && (lbl_1_bss_114[playerNo].player->actor->colMesh == 30)) {
                        finishPlayers[finishCount] = playerNo;
                        finishCount += 1;
                    }
                }
                if (lbl_1_bss_114[playerNo].unk34 == 0) {
                    resetPosition = 0;
                    if (lbl_1_bss_114[playerNo].player->actor->colMesh == 1) {
                        lbl_1_bss_114[playerNo].pos38 = (HuVecF){1450.0f, 40.0f, 0.0f};
                        resetPosition = 1;
                    } else if (lbl_1_bss_114[playerNo].player->actor->colMesh == 6) {
                        lbl_1_bss_114[playerNo].pos38 = (HuVecF){4800.0f, 170.0f, 0.0f};
                        resetPosition = 1;
                    } else if (lbl_1_bss_114[playerNo].player->actor->colMesh == 29) {
                        lbl_1_bss_114[playerNo].pos38 = (HuVecF){2500.0f, 3547.5f, 0.0f};
                        resetPosition = 1;
                    }
                    if (resetPosition != 0) {
                        lbl_1_bss_114[playerNo].unk34 += 1;
                        lbl_1_bss_114[playerNo].pos38.y += 300.0f;
                        lbl_1_bss_114[playerNo].pos38.z -= 160.0f;
                    }
                }
            }
            lbl_1_bss_114[playerNo].player->actor->pos.z = 0.0f;
            if (lbl_1_bss_114[playerNo].unk08 == 2) {
                lbl_1_bss_114[playerNo].player->actor->pos.z += 500.0f;
                lbl_1_bss_114[playerNo].player->actor->pos.y = lbl_1_bss_114[playerNo].unk64 + ((15.0f * (f32) lbl_1_bss_114[playerNo].unk60) - (0.5f * (f32) lbl_1_bss_114[playerNo].unk60 * (f32) lbl_1_bss_114[playerNo].unk60));
                lbl_1_bss_114[playerNo].unk60 += 1;
                Hu3DModelPosSetV((s16) lbl_1_bss_114[playerNo].player->actor->mdlId, &lbl_1_bss_114[playerNo].player->actor->pos);
            }
            if ((lbl_1_bss_114[playerNo].player->actor->rotY >= 0.0f) && (lbl_1_bss_114[playerNo].player->actor->rotY < 180.0f)) {
                lbl_1_bss_114[playerNo].unk04 = 1;
                lbl_1_bss_114[playerNo].player->actor->rotY = 90.0f;
            } else {
                lbl_1_bss_114[playerNo].unk04 = -1;
                lbl_1_bss_114[playerNo].player->actor->rotY = -90.0f;
            }
            if ((lbl_1_bss_114[playerNo].player->actor->rotY != 90.0f) && (lbl_1_bss_114[playerNo].player->actor->rotY != -90.0f)) {
                if (lbl_1_bss_114[playerNo].unk04 == 1) {
                    rotation = 90.0f;
                } else {
                    rotation = -90.0f;
                }
                lbl_1_bss_114[playerNo].player->actor->rotY = rotation;
            }
            if (lbl_1_bss_114[playerNo].unk5C == 0) {
                minX = 155.0f;
                maxX = 2450.0f;
            } else if (lbl_1_bss_114[playerNo].unk5C == 1) {
                minX = 155.0f;
                maxX = 2880.0f;
            } else if (lbl_1_bss_114[playerNo].unk5C == 2) {
                minX = 230.0f;
                maxX = 2050.0f;
            } else if (lbl_1_bss_114[playerNo].unk5C == 3) {
                minX = 155.0f;
                maxX = 2850.0f;
            }
            minX += lbl_1_data_48[lbl_1_bss_114[playerNo].unk5C].x;
            maxX += lbl_1_data_48[lbl_1_bss_114[playerNo].unk5C].x;
            if (lbl_1_bss_114[playerNo].player->actor->pos.x < minX) {
                lbl_1_bss_114[playerNo].player->actor->pos.x = 0.100000001f + minX;
            } else if (lbl_1_bss_114[playerNo].player->actor->pos.x > maxX) {
                lbl_1_bss_114[playerNo].player->actor->pos.x = maxX - 0.100000001f;
            }
            if (((u32) (lbl_1_bss_114[playerNo].player->actor->colGroundAttr & 4096) != 0) && (lbl_1_bss_114[playerNo].unk08 == 0)) {
                lbl_1_bss_114[playerNo].unk08 = 2;
                MgPlayerAttrSet(lbl_1_bss_114[playerNo].player, 1U);
                CharMotionShiftSet(GwPlayerConf[playerNo].charNo, lbl_1_bss_114[playerNo].motion[1], 0.0f, 8.0f, 0U);
                lbl_1_bss_114[playerNo].player->actor->pos.y += 50.0f;
                lbl_1_bss_114[playerNo].player->actor->pos.z += 500.0f;
                omVibrate((s16) playerNo, 20, 7, 3);
                lbl_1_bss_114[playerNo].unk60 = 0;
                lbl_1_bss_114[playerNo].unk64 = lbl_1_bss_114[playerNo].player->actor->pos.y;
                MgPlayerDespawn(lbl_1_bss_114[playerNo].player);
            }
        }
        playerNo += 1;
    }
    if (finishCount != 0) {
        lbl_1_bss_54C = finishPlayers[frandmod(finishCount)];
        lbl_1_bss_548 = 1;
        MgPlayerAttrSet(lbl_1_bss_114[lbl_1_bss_54C].player, 1U);
        lbl_1_bss_8[lbl_1_bss_54C] = lbl_1_bss_114[lbl_1_bss_54C].player->actor->pos;
    }
}

/* Shared target-backed literal pool; names retain address identity. */

void fn_1_3294(int playerNo)
{
    f32 endX;

    if (lbl_1_bss_114[playerNo].unk10 == 0) {
        if (lbl_1_bss_114[playerNo].unk5C == 0) {
            if (lbl_1_bss_114[playerNo].player->actor->pos.x < (2600.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].unk5C].x)) {
                if ((lbl_1_bss_114[playerNo].player->actor->pos.x >= 2500.0f) && ((s32) lbl_1_bss_44[playerNo][0] == -1)) {
                    lbl_1_bss_44[playerNo][0] = HuAudFXPlay(playerNo == 0 ? 1753 : (playerNo == 1 ? 1754 : (playerNo == 2 ? 1755 : 1756)));
                }
                MgPlayerPadSet(lbl_1_bss_114[playerNo].player, 30, 0, 0, 0);
                if (lbl_1_bss_114[playerNo].player->actor->pos.x >= (2600.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].unk5C].x)) {
                    lbl_1_bss_114[playerNo].player->actor->pos.x = 2600.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].unk5C].x;
                }
                lbl_1_bss_114[playerNo].player->actor->rotY = 90.0f;
                lbl_1_bss_114[playerNo].unk04 = 1;
                return;
            }
            MgPlayerAttrSet(lbl_1_bss_114[playerNo].player, 1U);
            lbl_1_bss_114[playerNo].unk5C += 1;
            lbl_1_bss_114[playerNo].player->actor->pos.x = lbl_1_data_48[lbl_1_bss_114[playerNo].unk5C].x;
            lbl_1_bss_114[playerNo].unk10 += 1;
            lbl_1_bss_44[playerNo][0] = -1;
            return;
        }
        if (lbl_1_bss_114[playerNo].unk5C == 1) {
            if (lbl_1_bss_114[playerNo].unk14 == 0) {
                if (lbl_1_bss_114[playerNo].player->actor->pos.x < (3100.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].unk5C].x)) {
                    MgPlayerPadSet(lbl_1_bss_114[playerNo].player, 30, 0, 0, 0);
                    if (lbl_1_bss_114[playerNo].player->actor->pos.x >= (3100.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].unk5C].x)) {
                        lbl_1_bss_114[playerNo].player->actor->pos.x = 3100.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].unk5C].x;
                    }
                    lbl_1_bss_114[playerNo].player->actor->rotY = 90.0f;
                    lbl_1_bss_114[playerNo].unk04 = 1;
                    return;
                }
                MgPlayerPadSet(lbl_1_bss_114[playerNo].player, 0, 0, 256, 0);
                lbl_1_bss_114[playerNo].unk14 = 1;
                return;
            }
            if ((lbl_1_bss_114[playerNo].unk14 == 8) && ((s32) lbl_1_bss_44[playerNo][1] == -1)) {
                    lbl_1_bss_44[playerNo][1] = HuAudFXPlay(playerNo == 0 ? 1753 : (playerNo == 1 ? 1754 : (playerNo == 2 ? 1755 : 1756)));
            }
            MgPlayerPadSet(lbl_1_bss_114[playerNo].player, 0, 0, 0, 256);
            if (++lbl_1_bss_114[playerNo].unk14 == 15) {
                lbl_1_bss_114[playerNo].player->actor->pos.y = lbl_1_data_48[lbl_1_bss_114[playerNo].unk5C + 1].y;
                lbl_1_bss_114[playerNo].unk5C += 1;
                lbl_1_bss_114[playerNo].unk10 += 1;
                lbl_1_bss_114[playerNo].unk14 = 0;
                lbl_1_bss_44[playerNo][1] = -1;
                return;
            }
        } else if (lbl_1_bss_114[playerNo].unk5C == 2) {
            if (lbl_1_bss_114[playerNo].player->actor->pos.x > (230.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].unk5C].x)) {
                if ((lbl_1_bss_114[playerNo].player->actor->pos.x <= 4760.0f) && ((s32) lbl_1_bss_44[playerNo][2] == -1)) {
                    lbl_1_bss_44[playerNo][2] = HuAudFXPlay(playerNo == 0 ? 1753 : (playerNo == 1 ? 1754 : (playerNo == 2 ? 1755 : 1756)));
                }
                MgPlayerPadSet(lbl_1_bss_114[playerNo].player, -30, 0, 0, 0);
                if (lbl_1_bss_114[playerNo].player->actor->pos.x <= (230.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].unk5C].x)) {
                    lbl_1_bss_114[playerNo].player->actor->pos.x = 230.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].unk5C].x;
                }
                lbl_1_bss_114[playerNo].player->actor->rotY = -90.0f;
                lbl_1_bss_114[playerNo].unk04 = -1;
                return;
            }
            MgPlayerAttrSet(lbl_1_bss_114[playerNo].player, 1U);
            lbl_1_bss_114[playerNo].unk5C += 1;
            lbl_1_bss_114[playerNo].player->actor->pos.x = 3200.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].unk5C].x;
            lbl_1_bss_114[playerNo].unk10 += 1;
            lbl_1_bss_44[playerNo][2] = -1;
            return;
        }
    } else if (lbl_1_bss_114[playerNo].unk5C == 1) {
        endX = 160.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].unk5C].x;
        MgPlayerAttrReset(lbl_1_bss_114[playerNo].player, 1U);
        if (lbl_1_bss_114[playerNo].player->actor->pos.x < endX) {
            if ((lbl_1_bss_114[playerNo].player->actor->pos.x >= 3240.0f) && ((s32) lbl_1_bss_44[playerNo][0] == -1)) {
                    lbl_1_bss_44[playerNo][0] = HuAudFXPlay(playerNo == 0 ? 1753 : (playerNo == 1 ? 1754 : (playerNo == 2 ? 1755 : 1756)));
            }
            MgPlayerPadSet(lbl_1_bss_114[playerNo].player, 30, 0, 0, 0);
            if (lbl_1_bss_114[playerNo].player->actor->pos.x >= endX) {
                lbl_1_bss_114[playerNo].player->actor->pos.x = endX;
                return;
            }
        } else {
            MgPlayerAttrReset(lbl_1_bss_114[playerNo].player, 1U);
            lbl_1_bss_114[playerNo].unk08 = 0;
            lbl_1_bss_114[playerNo].unk34 = 0;
            lbl_1_bss_114[playerNo].pos38 = lbl_1_bss_114[playerNo].player->actor->pos;
            lbl_1_bss_114[playerNo].pos38.y += 300.0f;
            lbl_1_bss_114[playerNo].pos38.z -= 160.0f;
            lbl_1_bss_44[playerNo][0] = -1;
            return;
        }
    } else {
        if (lbl_1_bss_114[playerNo].unk5C == 2) {
            if (lbl_1_bss_114[playerNo].unk14 < 30) {
                MgPlayerAttrSet(lbl_1_bss_114[playerNo].player, 1U);
                lbl_1_bss_114[playerNo].player->actor->rotY = -90.0f;
                lbl_1_bss_114[playerNo].unk04 = -1;
            } else if (lbl_1_bss_114[playerNo].unk14 == 30) {
                MgPlayerAttrReset(lbl_1_bss_114[playerNo].player, 1U);
                MgPlayerPadSet(lbl_1_bss_114[playerNo].player, 0, 0, 256, 0);
            } else {
                if ((lbl_1_bss_114[playerNo].unk14 == 35) && ((s32) lbl_1_bss_44[playerNo][1] == -1)) {
                    lbl_1_bss_44[playerNo][1] = HuAudFXPlay(playerNo == 0 ? 1753 : (playerNo == 1 ? 1754 : (playerNo == 2 ? 1755 : 1756)));
                }
                MgPlayerPadSet(lbl_1_bss_114[playerNo].player, 0, 0, 0, 256);
                if (lbl_1_bss_114[playerNo].unk14 == 60) {
                    MgPlayerAttrReset(lbl_1_bss_114[playerNo].player, 1U);
                    lbl_1_bss_114[playerNo].unk08 = 0;
                    lbl_1_bss_114[playerNo].unk34 = 0;
                    lbl_1_bss_114[playerNo].pos38 = lbl_1_bss_114[playerNo].player->actor->pos;
                    lbl_1_bss_114[playerNo].pos38.y += 300.0f;
                    lbl_1_bss_114[playerNo].pos38.z -= 160.0f;
                    lbl_1_bss_44[playerNo][1] = -1;
                }
            }
            lbl_1_bss_114[playerNo].unk14 += 1;
            return;
        }
        if (lbl_1_bss_114[playerNo].unk5C == 3) {
            endX = (3200.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].unk5C].x) - 350.0f;
            MgPlayerAttrReset(lbl_1_bss_114[playerNo].player, 1U);
            if (lbl_1_bss_114[playerNo].player->actor->pos.x > endX) {
                if ((lbl_1_bss_114[playerNo].player->actor->pos.x <= 4000.0f) && ((s32) lbl_1_bss_44[playerNo][2] == -1)) {
                    lbl_1_bss_44[playerNo][2] = HuAudFXPlay(playerNo == 0 ? 1753 : (playerNo == 1 ? 1754 : (playerNo == 2 ? 1755 : 1756)));
                }
                MgPlayerPadSet(lbl_1_bss_114[playerNo].player, -30, 0, 0, 0);
                if (lbl_1_bss_114[playerNo].player->actor->pos.x <= endX) {
                    lbl_1_bss_114[playerNo].player->actor->pos.x = endX;
                    return;
                }
            } else {
                MgPlayerAttrReset(lbl_1_bss_114[playerNo].player, 1U);
                lbl_1_bss_114[playerNo].unk08 = 0;
                lbl_1_bss_114[playerNo].unk34 = 0;
                lbl_1_bss_114[playerNo].pos38 = lbl_1_bss_114[playerNo].player->actor->pos;
                lbl_1_bss_114[playerNo].pos38.y += 300.0f;
                lbl_1_bss_114[playerNo].pos38.z -= 160.0f;
                lbl_1_bss_44[playerNo][2] = -1;
            }
        }
    }
}

void fn_1_44CC(int playerNo)
{
    Point3d pos;
    Point3d direction;
    f32 distance;

    if (lbl_1_bss_114[playerNo].unk10 == 0) {
        Hu3DModelAttrReset(lbl_1_bss_114[playerNo].model, 1U);
        Hu3DModelPosGet((s16) lbl_1_bss_114[playerNo].player->actor->mdlId, &pos);
        lbl_1_bss_114[playerNo].pos24 = pos;
        lbl_1_bss_114[playerNo].pos24.y = (f32) ((f64) (2000.0f + pos.y) - (1200.0 * sin((3.1415926535897931 * (f64) lbl_1_bss_114[playerNo].unk30) / 180.0)));
        Hu3DModelPosSet(lbl_1_bss_114[playerNo].model, pos.x, lbl_1_bss_114[playerNo].pos24.y, pos.z);
        lbl_1_bss_114[playerNo].unk30 += 1.0f;
        if (lbl_1_bss_114[playerNo].unk30 >= 90.0f) {
            lbl_1_bss_114[playerNo].unk30 = 90.0f;
            lbl_1_bss_114[playerNo].unk10 += 1;
            CharMotionShiftSet(GwPlayerConf[playerNo].charNo, lbl_1_bss_114[playerNo].motion[3], 0.0f, 8.0f, 1073741825U);
            lbl_1_bss_114[playerNo].player->actor->rotY = 0.0f;
            return;
        }
    } else if (lbl_1_bss_114[playerNo].unk10 == 1) {
        PSVECSubtract(&lbl_1_bss_114[playerNo].pos38, &lbl_1_bss_114[playerNo].pos24, &direction);
        distance = PSVECMag(&direction);
        PSVECNormalize(&direction, &direction);
        PSVECScale(&direction, &direction, distance / 20.0f);
        PSVECAdd(&direction, &lbl_1_bss_114[playerNo].pos24, &lbl_1_bss_114[playerNo].pos24);
        Hu3DModelPosSetV(lbl_1_bss_114[playerNo].model, &lbl_1_bss_114[playerNo].pos24);
        Hu3DModelObjPosGet(lbl_1_bss_114[playerNo].model, "itemhook_I", &pos);
        lbl_1_bss_114[playerNo].player->actor->pos = pos;
        lbl_1_bss_114[playerNo].player->actor->velY = 0.0f;
        if (pos.z <= 0.0f) {
            lbl_1_bss_114[playerNo].player->actor->pos.z = 0.0f;
            lbl_1_bss_114[playerNo].unk10 += 1;
            return;
        }
    } else if (lbl_1_bss_114[playerNo].unk10 == 2) {
        if (lbl_1_bss_114[playerNo].unk5C < 2) {
            lbl_1_bss_114[playerNo].player->actor->rotY += 5.0f;
            if (lbl_1_bss_114[playerNo].player->actor->rotY > 90.0f) {
                lbl_1_bss_114[playerNo].player->actor->rotY = 90.0f;
                lbl_1_bss_114[playerNo].unk04 = 1;
            }
        } else {
            lbl_1_bss_114[playerNo].player->actor->rotY -= 5.0f;
            if (lbl_1_bss_114[playerNo].player->actor->rotY < -90.0f) {
                lbl_1_bss_114[playerNo].player->actor->rotY = -90.0f;
                lbl_1_bss_114[playerNo].unk04 = -1;
            }
        }
        if ((u32) (lbl_1_bss_114[playerNo].player->actor->colGroundAttr & 16511) != 0) {
            lbl_1_bss_114[playerNo].unk10 += 1;
            return;
        }
    } else {
        lbl_1_bss_114[playerNo].pos24.y += 24.0f;
        Hu3DModelPosSet(lbl_1_bss_114[playerNo].model, lbl_1_bss_114[playerNo].pos24.x, lbl_1_bss_114[playerNo].pos24.y, lbl_1_bss_114[playerNo].pos24.z);
        if (lbl_1_bss_114[playerNo].pos24.y > (800.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].unk5C].y)) {
            Hu3DModelAttrSet(lbl_1_bss_114[playerNo].model, 1U);
            MgPlayerAttrReset(lbl_1_bss_114[playerNo].player, 1U);
            lbl_1_bss_114[playerNo].unk08 = 0;
        }
    }
}

void fn_1_4BE0(void)
{
    float cameraY;
    float cameraX;
    float actorY;
    float minX;
    float maxX;
    float minY;
    float actorX;
    int player;

    player = 0;
    while (player < 4) {
        if (lbl_1_bss_114[player].unk5C == 0) {
            minX = 700.0f;
            maxX = 1800.0f;
            minY = -200.0f;
        } else if (lbl_1_bss_114[player].unk5C == 1) {
            minX = 700.0f;
            maxX = 2600.0f;
            minY = -200.0f;
        } else if (lbl_1_bss_114[player].unk5C == 2) {
            minX = 1100.0f;
            maxX = 1500.0f;
            minY = -10.0f;
        } else if (lbl_1_bss_114[player].unk5C == 3) {
            minX = 800.0f;
            maxX = 2300.0f;
            minY = -370.0f;
        }
        minY += 100.0f;
        minX += lbl_1_data_48[lbl_1_bss_114[player].unk5C].x;
        maxX += lbl_1_data_48[lbl_1_bss_114[player].unk5C].x;
        minY += lbl_1_data_48[lbl_1_bss_114[player].unk5C].y;
        cameraX = lbl_1_bss_4CC[player].center.x;
        actorX = lbl_1_bss_114[player].player->actor->pos.x;
        cameraY = lbl_1_bss_4CC[player].center.y;
        actorY = lbl_1_bss_114[player].player->actor->pos.y;
        if (lbl_1_bss_114[player].unk04 > 0) {
            if (actorX > maxX) {
                actorX = maxX;
            }
            if (cameraX < (300.0f + actorX)) {
                lbl_1_bss_4CC[player].center.x += ((300.0f + actorX) - cameraX) / 10.0f;
            }
            if ((actorX >= (minX - 300.0f)) && (cameraX > (300.0f + actorX))) {
                lbl_1_bss_4CC[player].center.x -= (cameraX - (300.0f + actorX)) / 10.0f;
            }
            if ((lbl_1_bss_114[player].unk08 == 1) && (cameraX > (300.0f + actorX))) {
                lbl_1_bss_4CC[player].center.x -= (cameraX - (300.0f + actorX)) / 10.0f;
            }
        } else if (lbl_1_bss_114[player].unk04 < 0) {
            if (actorX < minX) {
                actorX = minX;
            }
            if (cameraX > (actorX - 300.0f)) {
                lbl_1_bss_4CC[player].center.x -= (cameraX - (actorX - 300.0f)) / 10.0f;
            }
            if ((actorX <= (300.0f + maxX)) && (cameraX < (actorX - 300.0f))) {
                lbl_1_bss_4CC[player].center.x += ((actorX - 300.0f) - cameraX) / 10.0f;
            }
            if ((lbl_1_bss_114[player].unk08 == 1) && (cameraX < (actorX - 300.0f))) {
                lbl_1_bss_4CC[player].center.x += ((actorX - 300.0f) - cameraX) / 10.0f;
            }
        }
        if (actorY < minY) {
            actorY = minY;
        }
        if (cameraY < (75.0f + actorY)) {
            lbl_1_bss_4CC[player].center.y += ((75.0f + actorY) - cameraY) / 20.0f;
        } else if (cameraY > (75.0f + actorY)) {
            lbl_1_bss_4CC[player].center.y -= (cameraY - (75.0f + actorY)) / 20.0f;
        }
        omCameraViewSetMulti((s16) lbl_1_data_28[player], &lbl_1_bss_4CC[player]);
        if (lbl_1_bss_114[player].player->actor->pos.y < (minY - 1000.0f)) {
            lbl_1_bss_114[player].player->actor->pos.y = minY - 1000.0f;
            lbl_1_bss_114[player].player->actor->pos.z = 1000.0f;
        }
        player += 1;
    }
}

void fn_1_52D0(void)
{
    HU3D_MODELID collisionModels[38];
    int modelFiles[4][3] = {{75, 82, 83}, {76, 85, 89}, {77, 91, 96}, {79, 98, 101}};
    HuVecF pos;
    s32 collisionCount;
    s16 extraModel;
    s32 i;

    collisionCount = 0;
    i = 0;
    while (i < 37) {
        lbl_1_bss_30C[i].model = Hu3DModelCreate(HuDataSelHeapReadNum(i + DATANUM(DATA_m618, 1), 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_30C[i].model, 15U);
        if (i <= 4) {
            pos.x = lbl_1_data_48->x;
            pos.y = lbl_1_data_48->y;
        } else if (i <= 12) {
            pos.x = lbl_1_data_48[1].x;
            pos.y = lbl_1_data_48[1].y;
        } else if (i <= 27) {
            pos.x = lbl_1_data_48[2].x;
            pos.y = lbl_1_data_48[2].y;
        } else if (i <= 36) {
            pos.x = lbl_1_data_48[3].x;
            pos.y = lbl_1_data_48[3].y;
        }
        if ((i == 8) || (i == 31)) {
            pos.y -= 100.0f;
        }
        Hu3DModelPosSet(lbl_1_bss_30C[i].model, pos.x, pos.y, 0.0f);
        Hu3DModelRotSet(lbl_1_bss_30C[i].model, 0.0f, 0.0f, 0.0f);
        Hu3DModelLayerSet(lbl_1_bss_30C[i].model, 1);
        Hu3DModelAttrSet(lbl_1_bss_30C[i].model, 1073741825U);
        lbl_1_bss_30C[i].colModel = Hu3DModelCreate(HuDataSelHeapReadNum(i + DATANUM(DATA_m618, 38), 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_30C[i].colModel, 15U);
        fn_1_6704(i);
        Hu3DModelLayerSet(lbl_1_bss_30C[i].colModel, 1);
        Hu3DModelAttrSet(lbl_1_bss_30C[i].colModel, 1U);
        collisionModels[collisionCount++] = lbl_1_bss_30C[i].colModel;
        i += 1;
    }
    i = 0;
    while (i < 4) {
        lbl_1_bss_2F4[i][0] = Hu3DModelCreate(HuDataSelHeapReadNum(modelFiles[i][0] + DATANUM(DATA_m618, 0), 268435456, HEAP_MODEL));
        Hu3DModelPosSet(lbl_1_bss_2F4[i][0], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
        Hu3DModelCameraSet(lbl_1_bss_2F4[i][0], 15U);
        Hu3DModelAttrSet(lbl_1_bss_2F4[i][0], 1073741825U);
        if ((i == 2) || (i == 3)) {
            lbl_1_bss_2F4[i][1] = Hu3DModelCreate(HuDataSelHeapReadNum(modelFiles[i][0] + DATANUM(DATA_m618, 1), 268435456, HEAP_MODEL));
            Hu3DModelPosSet(lbl_1_bss_2F4[i][1], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
            Hu3DModelCameraSet(lbl_1_bss_2F4[i][1], 15U);
            Hu3DModelAttrSet(lbl_1_bss_2F4[i][1], 1073741825U);
        }
        if (i == 3) {
            lbl_1_bss_2F4[i][2] = Hu3DModelCreate(HuDataSelHeapReadNum(modelFiles[i][0] + DATANUM(DATA_m618, 2), 268435456, HEAP_MODEL));
            Hu3DModelPosSet(lbl_1_bss_2F4[i][2], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
            Hu3DModelCameraSet(lbl_1_bss_2F4[i][2], 15U);
            Hu3DModelAttrSet(lbl_1_bss_2F4[i][2], 1073741825U);
        }
        if ((i == 1) || (i == 3)) {
              lbl_1_bss_2B4[(i == 1 ? 0 : 1)][0] = Hu3DModelCreate(HuDataSelHeapReadNum((i == 1 ? 9 : 32) + DATANUM(DATA_m618, 0), 268435456, HEAP_MODEL));
             Hu3DModelPosSet(lbl_1_bss_2B4[(i == 1 ? 0 : 1)][0], lbl_1_data_48[i].x, (lbl_1_data_48[i].y - 150.0f) - 100.0f, 0.0f);
             Hu3DModelCameraSet(lbl_1_bss_2B4[(i == 1 ? 0 : 1)][0], 15U);
             Hu3DModelAttrSet(lbl_1_bss_2B4[(i == 1 ? 0 : 1)][0], 1073741825U);
              lbl_1_bss_2B4[(i == 1 ? 0 : 1)][1] = Hu3DModelCreate(HuDataSelHeapReadNum((i == 1 ? 9 : 32) + DATANUM(DATA_m618, 0), 268435456, HEAP_MODEL));
             Hu3DModelPosSet(lbl_1_bss_2B4[(i == 1 ? 0 : 1)][1], lbl_1_data_48[i].x, (lbl_1_data_48[i].y - 300.0f) - 100.0f, 0.0f);
             Hu3DModelCameraSet(lbl_1_bss_2B4[(i == 1 ? 0 : 1)][1], 15U);
             Hu3DModelAttrSet(lbl_1_bss_2B4[(i == 1 ? 0 : 1)][1], 1073741825U);
        }
        lbl_1_bss_2CC[i][0] = Hu3DModelCreate(HuDataSelHeapReadNum(modelFiles[i][1] + DATANUM(DATA_m618, 0), 268435456, HEAP_MODEL));
        Hu3DModelPosSet(lbl_1_bss_2CC[i][0], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
        Hu3DModelCameraSet(lbl_1_bss_2CC[i][0], 15U);
        Hu3DModelAttrSet(lbl_1_bss_2CC[i][0], 1073741825U);
        if (i > 0) {
            lbl_1_bss_2CC[i][1] = Hu3DModelCreate(HuDataSelHeapReadNum(modelFiles[i][1] + DATANUM(DATA_m618, 1), 268435456, HEAP_MODEL));
            Hu3DModelPosSet(lbl_1_bss_2CC[i][1], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
            Hu3DModelCameraSet(lbl_1_bss_2CC[i][1], 15U);
            Hu3DModelAttrSet(lbl_1_bss_2CC[i][1], 1073741825U);
            lbl_1_bss_2CC[i][2] = Hu3DModelCreate(HuDataSelHeapReadNum(modelFiles[i][1] + DATANUM(DATA_m618, 2), 268435456, HEAP_MODEL));
            Hu3DModelPosSet(lbl_1_bss_2CC[i][2], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
            Hu3DModelCameraSet(lbl_1_bss_2CC[i][2], 15U);
            Hu3DModelAttrSet(lbl_1_bss_2CC[i][2], 1073741825U);
            if (i != 3) {
                lbl_1_bss_2CC[i][3] = Hu3DModelCreate(HuDataSelHeapReadNum(modelFiles[i][1] + DATANUM(DATA_m618, 3), 268435456, HEAP_MODEL));
                Hu3DModelPosSet(lbl_1_bss_2CC[i][3], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
                Hu3DModelCameraSet(lbl_1_bss_2CC[i][3], 15U);
                Hu3DModelAttrSet(lbl_1_bss_2CC[i][3], 1073741825U);
            }
        }
        if (i == 2) {
            lbl_1_bss_2CC[i][4] = Hu3DModelCreate(HuDataSelHeapReadNum(modelFiles[i][1] + DATANUM(DATA_m618, 4), 268435456, HEAP_MODEL));
            Hu3DModelPosSet(lbl_1_bss_2CC[i][4], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
            Hu3DModelCameraSet(lbl_1_bss_2CC[i][4], 15U);
            Hu3DModelAttrSet(lbl_1_bss_2CC[i][4], 1073741825U);
        }
        lbl_1_bss_2BC[i][0] = Hu3DModelCreate(HuDataSelHeapReadNum(modelFiles[i][2] + DATANUM(DATA_m618, 0), 268435456, HEAP_MODEL));
        Hu3DModelPosSet(lbl_1_bss_2BC[i][0], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
        Hu3DModelCameraSet(lbl_1_bss_2BC[i][0], 15U);
        if (i < 3) {
            lbl_1_bss_2BC[i][1] = Hu3DModelCreate(HuDataSelHeapReadNum(modelFiles[i][2] + DATANUM(DATA_m618, 1), 268435456, HEAP_MODEL));
            Hu3DModelPosSet(lbl_1_bss_2BC[i][1], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
            Hu3DModelCameraSet(lbl_1_bss_2BC[i][1], 15U);
            if (i == 2) {
                extraModel = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m618, 104), 268435456, HEAP_MODEL));
                Hu3DModelPosGet(lbl_1_bss_2BC[2][1], &pos);
                pos.x -= 15.0f;
                Hu3DModelPosSetV(extraModel, &pos);
                Hu3DModelCameraSet(extraModel, 15U);
                Hu3DModelLayerSet(extraModel, 1);
                Hu3DModelAttrSet(extraModel, 1U);
                collisionModels[collisionCount++] = extraModel;
            }
        }
        i += 1;
    }
    lbl_1_bss_540[0] = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m618, 102), 268435456, HEAP_MODEL));
    Hu3DModelPosSet(lbl_1_bss_540[0], lbl_1_data_48[3].x, lbl_1_data_48[3].y, 0.0f);
    Hu3DModelCameraSet(lbl_1_bss_540[0], 15U);
    Hu3DModelAttrSet(lbl_1_bss_540[0], 1U);
    Hu3DMotionSpeedSet(lbl_1_bss_540[0], 0.0f);
    lbl_1_bss_540[1] = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m618, 103), 268435456, HEAP_MODEL));
    Hu3DModelPosSet(lbl_1_bss_540[1], lbl_1_data_48[3].x, lbl_1_data_48[3].y, 0.0f);
    Hu3DModelCameraSet(lbl_1_bss_540[1], 15U);
    Hu3DModelAttrSet(lbl_1_bss_540[1], 1U);
    Hu3DMotionSpeedSet(lbl_1_bss_540[1], 0.0f);
    MgActorColMapInit(collisionModels, (s16) collisionCount, 8);
    Hu3DBGColorSet(0U, 0U, 0U);
    HuPrcChildCreate(fn_1_6C0C, 4096U, 8192U, 0, HuPrcCurrentGet());
}

void fn_1_6298(void)
{
    int group;

    Hu3DMotionSpeedSet(lbl_1_bss_30C[3].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[4].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[8].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[9].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[10].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[11].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[12].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[15].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[16].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[17].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[18].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[19].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[20].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[21].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[22].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[23].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[24].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[25].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[26].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[27].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[31].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[32].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[33].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[34].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[35].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_30C[36].model, 0.0f);
    group = 0;
    while (group < 4) {
        if ((group == 1) || (group == 3)) {
            Hu3DMotionSpeedSet(lbl_1_bss_2B4[group == 1 ? 0 : 1][0], 0.0f);
            Hu3DMotionSpeedSet(lbl_1_bss_2B4[group == 1 ? 0 : 1][1], 0.0f);
        }
        Hu3DMotionSpeedSet(lbl_1_bss_2CC[group][0], 0.0f);
        if (group > 0) {
            Hu3DMotionSpeedSet(lbl_1_bss_2CC[group][1], 0.0f);
            if (group != 3) {
                Hu3DMotionSpeedSet(lbl_1_bss_2CC[group][2], 0.0f);
                Hu3DMotionSpeedSet(lbl_1_bss_2CC[group][3], 0.0f);
            }
        }
        if (group == 2) {
            Hu3DMotionSpeedSet(lbl_1_bss_2CC[group][4], 0.0f);
        }
        group += 1;
    }
}

void fn_1_6704(int arg0)
{
    Point3d sp14;
    Point3d sp8;

    Hu3DMotionCalc(lbl_1_bss_30C[arg0].model);
    switch (arg0) {
    case 0:
    case 1:
    case 2:
        Hu3DModelPosGet(lbl_1_bss_30C[arg0].model, &sp14);
        break;
    case 3:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "1st_lift01", &sp14);
        break;
    case 4:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "1st_lift02", &sp14);
        break;
    case 5:
    case 6:
    case 7:
    case 8:
        Hu3DModelPosGet(lbl_1_bss_30C[arg0].model, &sp14);
        break;
    case 9:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "2st_lift01", &sp14);
        break;
    case 10:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "2st_lift02", &sp14);
        break;
    case 11:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "2st_lift03", &sp14);
        break;
    case 12:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "2st_lift04", &sp14);
        break;
    case 13:
    case 14:
        Hu3DModelPosGet(lbl_1_bss_30C[arg0].model, &sp14);
        break;
    case 15:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "3st_lift_a00", &sp14);
        break;
    case 16:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "3st_lift_a01", &sp14);
        break;
    case 17:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "3st_lift_a02", &sp14);
        break;
    case 18:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "3st_lift_b00", &sp14);
        break;
    case 19:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "3st_lift_b01", &sp14);
        break;
    case 20:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "3st_lift_b02", &sp14);
        break;
    case 21:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "3st_lift_b03", &sp14);
        break;
    case 22:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "3st_lift_c00", &sp14);
        break;
    case 23:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "3st_lift_c01", &sp14);
        break;
    case 24:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "3st_lift_c02", &sp14);
        break;
    case 25:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "3st_lift_c03", &sp14);
        break;
    case 26:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "3st_lift_d00", &sp14);
        break;
    case 27:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "3st_lift_d01", &sp14);
        break;
    case 28:
    case 29:
    case 30:
    case 31:
        Hu3DModelPosGet(lbl_1_bss_30C[arg0].model, &sp14);
        break;
    case 32:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "4st_lift00", &sp14);
        break;
    case 33:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "4st_lift01", &sp14);
        break;
    case 34:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "4st_lift02", &sp14);
        break;
    case 35:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "4st_lift03", &sp14);
        break;
    case 36:
        Hu3DModelObjPosGet(lbl_1_bss_30C[arg0].model, "4st_lift04", &sp14);
        break;
    }
    Hu3DModelPosSetV(lbl_1_bss_30C[arg0].colModel, &sp14);
    lbl_1_bss_30C[arg0].unk04 = sp14.y;
    Hu3DModelRotGet(lbl_1_bss_30C[arg0].model, &sp8);
    Hu3DModelRotSetV(lbl_1_bss_30C[arg0].colModel, &sp8);
}

void fn_1_6C0C(void)
{
    s32 var_r31;

    var_r31 = 0;
    while (var_r31 < 37) {
        if ((var_r31 >= 0) && (var_r31 <= 4)) {
            if ((lbl_1_bss_114->unk5C != 0) && (lbl_1_bss_114[1].unk5C != 0) && (lbl_1_bss_114[2].unk5C != 0) && (lbl_1_bss_114[3].unk5C != 0)) {
                if (((lbl_1_bss_114->unk5C != 1) || (lbl_1_bss_114->unk08 != 3)) && ((lbl_1_bss_114[1].unk5C != 1) || (lbl_1_bss_114[1].unk08 != 3)) && ((lbl_1_bss_114[2].unk5C != 1) || (lbl_1_bss_114[2].unk08 != 3)) && ((lbl_1_bss_114[3].unk5C != 1) || (lbl_1_bss_114[3].unk08 != 3))) {
                    Hu3DModelAttrSet(lbl_1_bss_30C[var_r31].model, 1U);
                }
            } else {
                Hu3DModelAttrReset(lbl_1_bss_30C[var_r31].model, 1U);
            }
        } else if ((var_r31 >= 5) && (var_r31 <= 12)) {
            if ((lbl_1_bss_114->unk5C != 1) && (lbl_1_bss_114[1].unk5C != 1) && (lbl_1_bss_114[2].unk5C != 1) && (lbl_1_bss_114[3].unk5C != 1)) {
                if (((lbl_1_bss_114->unk5C != 2) || (lbl_1_bss_114->unk08 != 3)) && ((lbl_1_bss_114[1].unk5C != 2) || (lbl_1_bss_114[1].unk08 != 3)) && ((lbl_1_bss_114[2].unk5C != 2) || (lbl_1_bss_114[2].unk08 != 3)) && ((lbl_1_bss_114[3].unk5C != 2) || (lbl_1_bss_114[3].unk08 != 3))) {
                    Hu3DModelAttrSet(lbl_1_bss_30C[var_r31].model, 1U);
                }
            } else {
                Hu3DModelAttrReset(lbl_1_bss_30C[var_r31].model, 1U);
            }
        } else if ((var_r31 >= 13) && (var_r31 <= 27)) {
            if ((lbl_1_bss_114->unk5C != 2) && (lbl_1_bss_114[1].unk5C != 2) && (lbl_1_bss_114[2].unk5C != 2) && (lbl_1_bss_114[3].unk5C != 2)) {
                if (((lbl_1_bss_114->unk5C != 3) || (lbl_1_bss_114->unk08 != 3)) && ((lbl_1_bss_114[1].unk5C != 3) || (lbl_1_bss_114[1].unk08 != 3)) && ((lbl_1_bss_114[2].unk5C != 3) || (lbl_1_bss_114[2].unk08 != 3)) && ((lbl_1_bss_114[3].unk5C != 3) || (lbl_1_bss_114[3].unk08 != 3))) {
                    Hu3DModelAttrSet(lbl_1_bss_30C[var_r31].model, 1U);
                }
            } else {
                Hu3DModelAttrReset(lbl_1_bss_30C[var_r31].model, 1U);
            }
        } else if ((var_r31 >= 28) && (var_r31 <= 36)) {
            if ((lbl_1_bss_114->unk5C != 3) && (lbl_1_bss_114[1].unk5C != 3) && (lbl_1_bss_114[2].unk5C != 3) && (lbl_1_bss_114[3].unk5C != 3)) {
                Hu3DModelAttrSet(lbl_1_bss_30C[var_r31].model, 1U);
            } else {
                Hu3DModelAttrReset(lbl_1_bss_30C[var_r31].model, 1U);
            }
        }
        var_r31 += 1;
    }
    if ((lbl_1_bss_114->unk5C != 0) && (lbl_1_bss_114[1].unk5C != 0) && (lbl_1_bss_114[2].unk5C != 0) && (lbl_1_bss_114[3].unk5C != 0)) {
        if (((lbl_1_bss_114->unk5C != 1) || (lbl_1_bss_114->unk08 != 3)) && ((lbl_1_bss_114[1].unk5C != 1) || (lbl_1_bss_114[1].unk08 != 3)) && ((lbl_1_bss_114[2].unk5C != 1) || (lbl_1_bss_114[2].unk08 != 3)) && ((lbl_1_bss_114[3].unk5C != 1) || (lbl_1_bss_114[3].unk08 != 3))) {
            Hu3DModelAttrSet(lbl_1_bss_2F4[0][0], 1U);
            Hu3DModelAttrSet(lbl_1_bss_2CC[0][0], 1U);
            Hu3DModelAttrSet(lbl_1_bss_2BC[0][0], 1U);
            Hu3DModelAttrSet(lbl_1_bss_2BC[0][1], 1U);
        }
    } else {
        Hu3DModelAttrReset(lbl_1_bss_2F4[0][0], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2CC[0][0], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2BC[0][0], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2BC[0][1], 1U);
        fn_1_6704(3);
        fn_1_6704(4);
    }
    if ((lbl_1_bss_114->unk5C != 1) && (lbl_1_bss_114[1].unk5C != 1) && (lbl_1_bss_114[2].unk5C != 1) && (lbl_1_bss_114[3].unk5C != 1)) {
        if (((lbl_1_bss_114->unk5C != 2) || (lbl_1_bss_114->unk08 != 3)) && ((lbl_1_bss_114[1].unk5C != 2) || (lbl_1_bss_114[1].unk08 != 3)) && ((lbl_1_bss_114[2].unk5C != 2) || (lbl_1_bss_114[2].unk08 != 3)) && ((lbl_1_bss_114[3].unk5C != 2) || (lbl_1_bss_114[3].unk08 != 3))) {
            Hu3DModelAttrSet(lbl_1_bss_2F4[1][0], 1U);
            Hu3DModelAttrSet(lbl_1_bss_2CC[1][0], 1U);
            Hu3DModelAttrSet(lbl_1_bss_2CC[1][1], 1U);
            Hu3DModelAttrSet(lbl_1_bss_2CC[1][2], 1U);
            Hu3DModelAttrSet(lbl_1_bss_2CC[1][3], 1U);
            Hu3DModelAttrSet(lbl_1_bss_2BC[1][0], 1U);
            Hu3DModelAttrSet(lbl_1_bss_2BC[1][1], 1U);
            Hu3DModelAttrSet(lbl_1_bss_2B4[0][0], 1U);
            Hu3DModelAttrSet(lbl_1_bss_2B4[0][1], 1U);
        }
    } else {
        Hu3DModelAttrReset(lbl_1_bss_2F4[1][0], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2CC[1][0], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2CC[1][1], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2CC[1][2], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2CC[1][3], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2BC[1][0], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2BC[1][1], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2B4[0][0], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2B4[0][1], 1U);
        fn_1_6704(9);
        fn_1_6704(10);
        fn_1_6704(11);
        fn_1_6704(12);
    }
    if ((lbl_1_bss_114->unk5C != 2) && (lbl_1_bss_114[1].unk5C != 2) && (lbl_1_bss_114[2].unk5C != 2) && (lbl_1_bss_114[3].unk5C != 2)) {
        if (((lbl_1_bss_114->unk5C != 3) || (lbl_1_bss_114->unk08 != 3)) && ((lbl_1_bss_114[1].unk5C != 3) || (lbl_1_bss_114[1].unk08 != 3)) && ((lbl_1_bss_114[2].unk5C != 3) || (lbl_1_bss_114[2].unk08 != 3)) && ((lbl_1_bss_114[3].unk5C != 3) || (lbl_1_bss_114[3].unk08 != 3))) {
            Hu3DModelAttrSet(lbl_1_bss_2F4[2][0], 1U);
            Hu3DModelAttrSet(lbl_1_bss_2F4[2][1], 1U);
            Hu3DModelAttrSet(lbl_1_bss_2CC[2][0], 1U);
            Hu3DModelAttrSet(lbl_1_bss_2CC[2][1], 1U);
            Hu3DModelAttrSet(lbl_1_bss_2CC[2][2], 1U);
            Hu3DModelAttrSet(lbl_1_bss_2CC[2][3], 1U);
            Hu3DModelAttrSet(lbl_1_bss_2CC[2][4], 1U);
            Hu3DModelAttrSet(lbl_1_bss_2BC[2][0], 1U);
            Hu3DModelAttrSet(lbl_1_bss_2BC[2][1], 1U);
        }
    } else {
        Hu3DModelAttrReset(lbl_1_bss_2F4[2][0], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2F4[2][1], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2CC[2][0], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2CC[2][1], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2CC[2][2], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2CC[2][3], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2CC[2][4], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2BC[2][0], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2BC[2][1], 1U);
        fn_1_6704(15);
        fn_1_6704(16);
        fn_1_6704(17);
        fn_1_6704(18);
        fn_1_6704(19);
        fn_1_6704(20);
        fn_1_6704(21);
        fn_1_6704(22);
        fn_1_6704(23);
        fn_1_6704(24);
        fn_1_6704(25);
        fn_1_6704(26);
        fn_1_6704(27);
    }
    if ((lbl_1_bss_114->unk5C != 3) && (lbl_1_bss_114[1].unk5C != 3) && (lbl_1_bss_114[2].unk5C != 3) && (lbl_1_bss_114[3].unk5C != 3)) {
        Hu3DModelAttrSet(lbl_1_bss_2F4[3][0], 1U);
        Hu3DModelAttrSet(lbl_1_bss_2F4[3][1], 1U);
        Hu3DModelAttrSet(lbl_1_bss_2F4[3][2], 1U);
        Hu3DModelAttrSet(lbl_1_bss_2CC[3][0], 1U);
        Hu3DModelAttrSet(lbl_1_bss_2CC[3][1], 1U);
        Hu3DModelAttrSet(lbl_1_bss_2CC[3][2], 1U);
        Hu3DModelAttrSet(lbl_1_bss_2BC[3][0], 1U);
        Hu3DModelAttrSet(lbl_1_bss_2B4[1][0], 1U);
        Hu3DModelAttrSet(lbl_1_bss_2B4[1][1], 1U);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_2F4[3][0], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2F4[3][1], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2F4[3][2], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2CC[3][0], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2CC[3][1], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2CC[3][2], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2BC[3][0], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2B4[1][0], 1U);
        Hu3DModelAttrReset(lbl_1_bss_2B4[1][1], 1U);
        fn_1_6704(32);
        fn_1_6704(33);
        fn_1_6704(34);
        fn_1_6704(35);
        fn_1_6704(36);
    }
    HuPrcVSleep();
}

void fn_1_796C(int winner)
{
    float viewportX[2];
    float viewportY[2];
    float splitX;
    float splitY;
    s32 camera;
    int insetX;
    int insetY;
    int insetWidth;
    int insetHeight;

    insetX = 2;
    insetY = 2;
    insetWidth = insetX * 2;
    insetHeight = insetY * 2;
    camera = 0;
    while (camera < 4) {
        if (winner == 0) {
            splitX = 320.0f + (5.3333335f * lbl_1_bss_4C8);
            splitY = 240.0f + (4.0f * lbl_1_bss_4C8);
        } else if (winner == 1) {
            splitX = 320.0f - (5.3333335f * lbl_1_bss_4C8);
            splitY = 240.0f + (4.0f * lbl_1_bss_4C8);
        } else if (winner == 2) {
            splitX = 320.0f + (5.3333335f * lbl_1_bss_4C8);
            splitY = 240.0f - (4.0f * lbl_1_bss_4C8);
        } else if (winner == 3) {
            splitX = 320.0f - (5.3333335f * lbl_1_bss_4C8);
            splitY = 240.0f - (4.0f * lbl_1_bss_4C8);
        } else {
            splitX = 640.0f - (5.3333335f * lbl_1_bss_4C8);
            splitY = 480.0f - (4.0f * lbl_1_bss_4C8);
        }
        if ((s32) lbl_1_data_28[camera] == 1) {
            viewportX[0] = 0.0f;
            viewportY[0] = 0.0f;
            viewportX[1] = 16.0f + splitX;
            viewportY[1] = 40.0f + splitY;
            Hu3DCameraScissorSet(lbl_1_data_28[0], insetX, insetY, (u32) (splitX - insetWidth), (u32) (splitY - insetHeight));
        } else if ((s32) lbl_1_data_28[camera] == 2) {
            viewportX[0] = splitX - 16.0f;
            viewportY[0] = 0.0f;
            viewportX[1] = 16.0f + (640.0f - splitX);
            viewportY[1] = 40.0f + splitY;
            Hu3DCameraScissorSet(lbl_1_data_28[1], (u32) (splitX + insetX), insetY, (u32) ((640.0f - splitX) - insetWidth), (u32) (splitY - insetHeight));
        } else if ((s32) lbl_1_data_28[camera] == 4) {
            viewportX[0] = 0.0f;
            viewportY[0] = splitY - 40.0f;
            viewportX[1] = 16.0f + splitX;
            viewportY[1] = 40.0f + (480.0f - splitY);
            Hu3DCameraScissorSet(lbl_1_data_28[2], insetX, (u32) (splitY + insetY), (u32) (splitX - insetWidth), (u32) ((480.0f - splitY) - insetHeight));
        } else if ((s32) lbl_1_data_28[camera] == 8) {
            viewportX[0] = splitX - 16.0f;
            viewportY[0] = splitY - 40.0f;
            viewportX[1] = 16.0f + (640.0f - splitX);
            viewportY[1] = 40.0f + (480.0f - splitY);
            Hu3DCameraScissorSet(lbl_1_data_28[3], (u32) (splitX + insetX), (u32) (splitY + insetY), (u32) ((640.0f - splitX) - insetWidth), (u32) ((480.0f - splitY) - insetHeight));
        }
        Hu3DCameraPerspectiveSet(lbl_1_data_28[camera], 10.0f, 20.0f, 30000.0f, 1.2f);
        Hu3DCameraViewportSet(lbl_1_data_28[camera], viewportX[0], viewportY[0], viewportX[1], viewportY[1], 0.0f, 1.0f);
        camera += 1;
    }
    if (++lbl_1_bss_4C8 > 60.0f) {
        lbl_1_bss_4C8 = 60.0f;
    }
}

void fn_1_8180(int playerNo)
{
    Point3d delta;
    Point3d playerPos;
    Point3d targetPos;
    Point3d savedPos;
    Point3d liftPos;
    s32 unknownInit;
    float xBoundary;
    float distance;

    /* Retail initializes this local but never reads it; its purpose is unknown. */
    unknownInit = 0;
    playerPos = lbl_1_bss_114[playerNo].player->actor->pos;
    if ((u32) (lbl_1_bss_114[playerNo].player->actor->colGroundAttr & 16511) != 0) {
        fn_1_C450(&lbl_1_bss_84[playerNo], 0, 0, 0, 0);
        lbl_1_bss_114[playerNo].unk44 = 0;
        switch (lbl_1_bss_114[playerNo].player->actor->colMesh) {
        case 0:
            xBoundary = 350.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].unk5C].x;
            if (playerPos.x < xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
                Hu3DModelObjPosGet(lbl_1_bss_30C[3].model, "1st_lift01", &lbl_1_bss_114[playerNo].pos50);
                lbl_1_bss_84[playerNo].unk04 = 0;
                lbl_1_bss_84[playerNo].unk08 = 0;
                lbl_1_bss_84[playerNo].unk0C = (lbl_1_bss_84[playerNo].unk00 == 0) ? frandmod(4) :
                    ((lbl_1_bss_84[playerNo].unk00 == 1) ? frandmod(3) :
                    ((lbl_1_bss_84[playerNo].unk00 == 2) ? frandmod(2) : frandmod(2)));
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[3].model, "1st_lift01", &targetPos);
                targetPos.y = targetPos.z = 0.0f;
                playerPos.y = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                if ((distance < 500.0f) && (targetPos.x < lbl_1_bss_114[playerNo].pos50.x)) {
                    if (lbl_1_bss_84[playerNo].unk0C != 0) {
                        lbl_1_bss_84[playerNo].unk04 = 1;
                    }
                    if (lbl_1_bss_84[playerNo].unk08 == lbl_1_bss_84[playerNo].unk0C) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, 256, 0);
                        lbl_1_bss_114[playerNo].unk44 = 1;
                        lbl_1_bss_114[playerNo].unk48 = 18;
                        lbl_1_bss_114[playerNo].unk48 = (lbl_1_bss_84[playerNo].unk00 == 0) ? ((frandmod(100) < 50U) ? (lbl_1_bss_114[playerNo].unk48 + 5) : lbl_1_bss_114[playerNo].unk48) :
                            ((lbl_1_bss_84[playerNo].unk00 == 1) ? ((frandmod(100) < 35U) ? (lbl_1_bss_114[playerNo].unk48 + 5) : lbl_1_bss_114[playerNo].unk48) :
                            ((lbl_1_bss_84[playerNo].unk00 == 2) ? ((frandmod(100) < 20U) ? (lbl_1_bss_114[playerNo].unk48 + 5) : lbl_1_bss_114[playerNo].unk48) :
                            ((frandmod(100) < 5U) ? (lbl_1_bss_114[playerNo].unk48 + 5) : lbl_1_bss_114[playerNo].unk48)));
                    }
                } else if (lbl_1_bss_84[playerNo].unk04 != 0) {
                    lbl_1_bss_84[playerNo].unk04 = 0;
                    lbl_1_bss_84[playerNo].unk08 += 1;
                }
                lbl_1_bss_114[playerNo].pos50 = targetPos;
            }
            break;
        case 3:
            if (--lbl_1_bss_114[playerNo].unk48 > 0) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[1].model, "1st_middle", &targetPos);
                targetPos.y = targetPos.z = 0.0f;
                playerPos.y = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 450.0f) {
                    if ((s32) ((lbl_1_bss_84[playerNo].unk00 == 0) ? ((frandmod(100) < 50U) ? 1 : 0) :
                        ((lbl_1_bss_84[playerNo].unk00 == 1) ? ((frandmod(100) < 35U) ? 1 : 0) :
                        ((lbl_1_bss_84[playerNo].unk00 == 2) ? ((frandmod(100) < 20U) ? 1 : 0) :
                        ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 14, 0, 256, 0);
                    } else {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, 256, 0);
                    }
                    lbl_1_bss_114[playerNo].unk44 = 1;
                }
            }
            break;
        case 1:
            Hu3DModelObjPosGet(lbl_1_bss_30C[1].model, "1st_middle", &targetPos);
            xBoundary = 100.0f + targetPos.x;
            if (playerPos.x < xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
                lbl_1_bss_84[playerNo].unk04 = 0;
                lbl_1_bss_84[playerNo].unk08 = 0;
                lbl_1_bss_84[playerNo].unk0C = (lbl_1_bss_84[playerNo].unk00 == 0) ? frandmod(4) :
                    ((lbl_1_bss_84[playerNo].unk00 == 1) ? frandmod(3) :
                    ((lbl_1_bss_84[playerNo].unk00 == 2) ? frandmod(2) : frandmod(2)));
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[4].model, "1st_lift02", &targetPos);
                if ((playerPos.y < (100.0f + targetPos.y)) && (playerPos.y > (targetPos.y - 100.0f))) {
                    if (lbl_1_bss_84[playerNo].unk0C != 0) {
                        lbl_1_bss_84[playerNo].unk04 = 1;
                    }
                    if (lbl_1_bss_84[playerNo].unk08 == lbl_1_bss_84[playerNo].unk0C) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, 256, 0);
                        lbl_1_bss_114[playerNo].unk44 = 1;
                        lbl_1_bss_114[playerNo].unk48 = 10;
                        lbl_1_bss_114[playerNo].unk48 = (lbl_1_bss_84[playerNo].unk00 == 0) ? ((frandmod(100) < 50U) ? (lbl_1_bss_114[playerNo].unk48 + 5) : lbl_1_bss_114[playerNo].unk48) :
                            ((lbl_1_bss_84[playerNo].unk00 == 1) ? ((frandmod(100) < 35U) ? (lbl_1_bss_114[playerNo].unk48 + 5) : lbl_1_bss_114[playerNo].unk48) :
                            ((lbl_1_bss_84[playerNo].unk00 == 2) ? ((frandmod(100) < 20U) ? (lbl_1_bss_114[playerNo].unk48 + 5) : lbl_1_bss_114[playerNo].unk48) :
                            ((frandmod(100) < 5U) ? (lbl_1_bss_114[playerNo].unk48 + 5) : lbl_1_bss_114[playerNo].unk48)));
                    }
                } else if (lbl_1_bss_84[playerNo].unk04 != 0) {
                    lbl_1_bss_84[playerNo].unk04 = 0;
                    lbl_1_bss_84[playerNo].unk08 += 1;
                }
            }
            break;
        case 4:
            if (--lbl_1_bss_114[playerNo].unk48 > 0) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
            } else {
                if ((s32) ((lbl_1_bss_84[playerNo].unk00 == 0) ? ((frandmod(100) < 50U) ? 1 : 0) :
                    ((lbl_1_bss_84[playerNo].unk00 == 1) ? ((frandmod(100) < 35U) ? 1 : 0) :
                    ((lbl_1_bss_84[playerNo].unk00 == 2) ? ((frandmod(100) < 20U) ? 1 : 0) :
                    ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], 14, 0, 256, 0);
                } else {
                    fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, 256, 0);
                }
                lbl_1_bss_114[playerNo].unk44 = 1;
            }
            break;
        case 2:
            fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
            break;
        case 5:
            xBoundary = 350.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].unk5C].x;
            if (playerPos.x < xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
                lbl_1_bss_84[playerNo].unk04 = 0;
                lbl_1_bss_84[playerNo].unk08 = 0;
                lbl_1_bss_84[playerNo].unk0C = (lbl_1_bss_84[playerNo].unk00 == 0) ? frandmod(4) :
                    ((lbl_1_bss_84[playerNo].unk00 == 1) ? frandmod(3) :
                    ((lbl_1_bss_84[playerNo].unk00 == 2) ? frandmod(2) : frandmod(2)));
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[9].model, "2st_lift01", &targetPos);
                if ((playerPos.y < (100.0f + targetPos.y)) && (playerPos.y > (targetPos.y - 100.0f))) {
                    if (lbl_1_bss_84[playerNo].unk0C != 0) {
                        lbl_1_bss_84[playerNo].unk04 = 1;
                    }
                    if (lbl_1_bss_84[playerNo].unk08 == lbl_1_bss_84[playerNo].unk0C) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, 256, 0);
                        lbl_1_bss_114[playerNo].unk44 = 1;
                    }
                } else if (lbl_1_bss_84[playerNo].unk04 != 0) {
                    lbl_1_bss_84[playerNo].unk04 = 0;
                    lbl_1_bss_84[playerNo].unk08 += 1;
                }
            }
            break;
        case 9:
            Hu3DModelObjPosGet(lbl_1_bss_30C[9].model, "2st_lift01", &targetPos);
            xBoundary = 80.0f + targetPos.x;
            if (playerPos.x < xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
                lbl_1_bss_84[playerNo].unk04 = 0;
                lbl_1_bss_84[playerNo].unk08 = 0;
                lbl_1_bss_84[playerNo].unk0C = (lbl_1_bss_84[playerNo].unk00 == 0) ? frandmod(4) :
                    ((lbl_1_bss_84[playerNo].unk00 == 1) ? frandmod(3) :
                    ((lbl_1_bss_84[playerNo].unk00 == 2) ? frandmod(2) : frandmod(2)));
            } else if (playerPos.y > 270.0f) {
                if (lbl_1_bss_84[playerNo].unk0C != 0) {
                    lbl_1_bss_84[playerNo].unk04 = 1;
                }
                if (lbl_1_bss_84[playerNo].unk08 == lbl_1_bss_84[playerNo].unk0C) {
                    if ((s32) ((lbl_1_bss_84[playerNo].unk00 == 0) ? ((frandmod(100) < 50U) ? 1 : 0) :
                        ((lbl_1_bss_84[playerNo].unk00 == 1) ? ((frandmod(100) < 35U) ? 1 : 0) :
                        ((lbl_1_bss_84[playerNo].unk00 == 2) ? ((frandmod(100) < 20U) ? 1 : 0) :
                        ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 28, 0, 256, 0);
                    } else {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, 256, 0);
                    }
                    lbl_1_bss_114[playerNo].unk44 = 1;
                }
            } else if (lbl_1_bss_84[playerNo].unk04 != 0) {
                lbl_1_bss_84[playerNo].unk04 = 0;
                lbl_1_bss_84[playerNo].unk08 += 1;
            }
            break;
        case 10:
            Hu3DModelObjPosGet(lbl_1_bss_30C[10].model, "2st_lift02", &targetPos);
            xBoundary = 80.0f + targetPos.x;
            if (playerPos.x < xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
                lbl_1_bss_84[playerNo].unk04 = 0;
                lbl_1_bss_84[playerNo].unk08 = 0;
                lbl_1_bss_84[playerNo].unk0C = (lbl_1_bss_84[playerNo].unk00 == 0) ? frandmod(4) :
                    ((lbl_1_bss_84[playerNo].unk00 == 1) ? frandmod(3) :
                    ((lbl_1_bss_84[playerNo].unk00 == 2) ? frandmod(2) : frandmod(2)));
            } else if (playerPos.y > -50.0f) {
                if (lbl_1_bss_84[playerNo].unk0C != 0) {
                    lbl_1_bss_84[playerNo].unk04 = 1;
                }
                if (lbl_1_bss_84[playerNo].unk08 == lbl_1_bss_84[playerNo].unk0C) {
                    if ((s32) ((lbl_1_bss_84[playerNo].unk00 == 0) ? ((frandmod(100) < 50U) ? 1 : 0) :
                        ((lbl_1_bss_84[playerNo].unk00 == 1) ? ((frandmod(100) < 35U) ? 1 : 0) :
                        ((lbl_1_bss_84[playerNo].unk00 == 2) ? ((frandmod(100) < 20U) ? 1 : 0) :
                        ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 28, 0, 256, 0);
                    } else {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, 256, 0);
                    }
                    lbl_1_bss_114[playerNo].unk44 = 1;
                }
            } else if (lbl_1_bss_84[playerNo].unk04 != 0) {
                lbl_1_bss_84[playerNo].unk04 = 0;
                lbl_1_bss_84[playerNo].unk08 += 1;
            }
            break;
        case 6:
            Hu3DModelObjPosGet(lbl_1_bss_30C[6].model, "2st_middle", &targetPos);
            xBoundary = 80.0f + targetPos.x;
            if (playerPos.x < xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[11].model, "2st_lift03", &targetPos);
                savedPos = targetPos;
                targetPos.y = targetPos.z = 0.0f;
                playerPos.y = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                Hu3DModelObjPosGet(lbl_1_bss_30C[12].model, "2st_lift04", &targetPos);
                if ((distance < 380.0f) && (savedPos.y < targetPos.y)) {
                    if ((s32) ((lbl_1_bss_84[playerNo].unk00 == 0) ? ((frandmod(100) < 50U) ? 1 : 0) :
                        ((lbl_1_bss_84[playerNo].unk00 == 1) ? ((frandmod(100) < 35U) ? 1 : 0) :
                        ((lbl_1_bss_84[playerNo].unk00 == 2) ? ((frandmod(100) < 20U) ? 1 : 0) :
                        ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 28, 0, 256, 0);
                    } else {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, 256, 0);
                    }
                    lbl_1_bss_114[playerNo].unk44 = 1;
                    targetPos = savedPos;
                } else {
                    Hu3DModelObjPosGet(lbl_1_bss_30C[12].model, "2st_lift04", &targetPos);
                    savedPos = targetPos;
                    targetPos.y = targetPos.z = 0.0f;
                    playerPos.y = playerPos.z = 0.0f;
                    PSVECSubtract(&targetPos, &playerPos, &delta);
                    distance = PSVECMag(&delta);
                    Hu3DModelObjPosGet(lbl_1_bss_30C[11].model, "2st_lift03", &targetPos);
                    if ((distance < 380.0f) && (savedPos.y < targetPos.y)) {
                        if ((s32) ((lbl_1_bss_84[playerNo].unk00 == 0) ? ((frandmod(100) < 50U) ? 1 : 0) :
                            ((lbl_1_bss_84[playerNo].unk00 == 1) ? ((frandmod(100) < 35U) ? 1 : 0) :
                            ((lbl_1_bss_84[playerNo].unk00 == 2) ? ((frandmod(100) < 20U) ? 1 : 0) :
                            ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                            fn_1_C450(&lbl_1_bss_84[playerNo], 28, 0, 256, 0);
                        } else {
                            fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, 256, 0);
                        }
                        lbl_1_bss_114[playerNo].unk44 = 1;
                        targetPos = savedPos;
                    }
                }
            }
            break;
        case 11:
        case 12:
            if ((s32) ((lbl_1_bss_84[playerNo].unk00 == 0) ? ((frandmod(100) < 50U) ? 1 : 0) :
                ((lbl_1_bss_84[playerNo].unk00 == 1) ? ((frandmod(100) < 35U) ? 1 : 0) :
                ((lbl_1_bss_84[playerNo].unk00 == 2) ? ((frandmod(100) < 20U) ? 1 : 0) :
                ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                fn_1_C450(&lbl_1_bss_84[playerNo], 28, 0, 256, 0);
            } else {
                fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, 256, 0);
            }
            lbl_1_bss_114[playerNo].unk44 = 1;
            break;
        case 7:
            fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
            break;
        case 37:
            xBoundary = lbl_1_bss_114[playerNo].pos38.x - 120.0f;
            if (playerPos.x > xBoundary) {
                if (playerPos.x <= xBoundary) {
                    playerPos.x = xBoundary;
                }
                fn_1_C450(&lbl_1_bss_84[playerNo], -lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
                lbl_1_bss_84[playerNo].unk04 = 0;
                lbl_1_bss_84[playerNo].unk08 = 0;
                lbl_1_bss_84[playerNo].unk0C = (lbl_1_bss_84[playerNo].unk00 == 0) ? frandmod(4) :
                    ((lbl_1_bss_84[playerNo].unk00 == 1) ? frandmod(3) :
                    ((lbl_1_bss_84[playerNo].unk00 == 2) ? frandmod(2) : frandmod(2)));
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[22].model, "3st_lift_c00", &targetPos);
                if (targetPos.y < 1850.0f) {
                    if (lbl_1_bss_84[playerNo].unk0C != 0) {
                        lbl_1_bss_84[playerNo].unk04 = 1;
                    }
                    if (lbl_1_bss_84[playerNo].unk08 == lbl_1_bss_84[playerNo].unk0C) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, 256, 0);
                        lbl_1_bss_114[playerNo].unk44 = 1;
                    }
                } else if (lbl_1_bss_84[playerNo].unk04 != 0) {
                    lbl_1_bss_84[playerNo].unk04 = 0;
                    lbl_1_bss_84[playerNo].unk08 += 1;
                }
            }
            break;
        case 22:
            Hu3DModelObjPosGet(lbl_1_bss_30C[22].model, "3st_lift_c00", &targetPos);
            xBoundary = targetPos.x - 100.0f;
            if (playerPos.x > xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], -lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[19].model, "3st_lift_b01", &targetPos);
                targetPos.x = targetPos.z = 0.0f;
                playerPos.x = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 150.0f) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, 256, 0);
                    lbl_1_bss_114[playerNo].unk44 = 1;
                }
            }
            break;
        case 19:
            Hu3DModelObjPosGet(lbl_1_bss_30C[19].model, "3st_lift_b01", &targetPos);
            xBoundary = 100.0f + targetPos.x;
            if (playerPos.x < xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[23].model, "3st_lift_c01", &targetPos);
                targetPos.x = targetPos.z = 0.0f;
                playerPos.x = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 150.0f) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, 256, 0);
                    lbl_1_bss_114[playerNo].unk44 = 1;
                }
            }
            break;
        case 23:
            Hu3DModelObjPosGet(lbl_1_bss_30C[23].model, "3st_lift_c01", &targetPos);
            xBoundary = targetPos.x - 100.0f;
            if (playerPos.x > xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], -lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[20].model, "3st_lift_b02", &targetPos);
                targetPos.x = targetPos.z = 0.0f;
                playerPos.x = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 150.0f) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, 256, 0);
                    lbl_1_bss_114[playerNo].unk44 = 1;
                }
            }
            break;
        case 20:
            Hu3DModelObjPosGet(lbl_1_bss_30C[20].model, "3st_lift_b02", &targetPos);
            xBoundary = 100.0f + targetPos.x;
            if (playerPos.x < xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[24].model, "3st_lift_c02", &targetPos);
                targetPos.x = targetPos.z = 0.0f;
                playerPos.x = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 150.0f) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, 256, 0);
                    lbl_1_bss_114[playerNo].unk44 = 1;
                }
            }
            break;
        case 24:
            Hu3DModelObjPosGet(lbl_1_bss_30C[24].model, "3st_lift_c02", &targetPos);
            xBoundary = targetPos.x - 100.0f;
            if (playerPos.x > xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], -lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[21].model, "3st_lift_b03", &targetPos);
                targetPos.x = targetPos.z = 0.0f;
                playerPos.x = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 150.0f) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, 256, 0);
                    lbl_1_bss_114[playerNo].unk44 = 1;
                }
            }
            break;
        case 21:
            Hu3DModelObjPosGet(lbl_1_bss_30C[21].model, "3st_lift_b03", &targetPos);
            xBoundary = targetPos.x - 100.0f;
            if (playerPos.x > xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], -lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
                lbl_1_bss_84[playerNo].unk04 = 0;
                lbl_1_bss_84[playerNo].unk08 = 0;
                lbl_1_bss_84[playerNo].unk0C = (lbl_1_bss_84[playerNo].unk00 == 0) ? frandmod(4) :
                    ((lbl_1_bss_84[playerNo].unk00 == 1) ? frandmod(3) :
                    ((lbl_1_bss_84[playerNo].unk00 == 2) ? frandmod(2) : frandmod(2)));
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[14].model, "3st_goal", &targetPos);
                targetPos.x = targetPos.z = 0.0f;
                playerPos.x = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 120.0f) {
                    if (lbl_1_bss_84[playerNo].unk0C != 0) {
                        lbl_1_bss_84[playerNo].unk04 = 1;
                    }
                    if (lbl_1_bss_84[playerNo].unk08 == lbl_1_bss_84[playerNo].unk0C) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, 256, 0);
                        lbl_1_bss_114[playerNo].unk44 = 1;
                    }
                } else if (lbl_1_bss_84[playerNo].unk04 != 0) {
                    lbl_1_bss_84[playerNo].unk04 = 0;
                    lbl_1_bss_84[playerNo].unk08 += 1;
                }
            }
            break;
        case 14:
            fn_1_C450(&lbl_1_bss_84[playerNo], -lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
            break;
        case 28:
            Hu3DModelObjPosGet(lbl_1_bss_30C[35].model, "4st_lift03", &targetPos);
            savedPos = targetPos;
            targetPos.y = targetPos.z = 0.0f;
            playerPos.y = playerPos.z = 0.0f;
            PSVECSubtract(&targetPos, &playerPos, &delta);
            distance = PSVECMag(&delta);
            if (distance < 348.0f) {
                fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, 256, 0);
                lbl_1_bss_114[playerNo].unk44 = 1;
                targetPos = savedPos;
                lbl_1_bss_114[playerNo].unk48 = 12;
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[36].model, "4st_lift04", &targetPos);
                savedPos = targetPos;
                targetPos.y = targetPos.z = 0.0f;
                playerPos.y = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 348.0f) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, 256, 0);
                    lbl_1_bss_114[playerNo].unk44 = 1;
                    targetPos = savedPos;
                    lbl_1_bss_114[playerNo].unk48 = 12;
                }
            }
            break;
        case 35:
        case 36:
            if (--lbl_1_bss_114[playerNo].unk48 > 0) {
                fn_1_C450(&lbl_1_bss_84[playerNo], -lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
            } else if ((playerPos.x < 3100.0f) && (playerPos.y < 3700.0f)) {
                if ((s32) ((lbl_1_bss_84[playerNo].unk00 == 0) ? ((frandmod(100) < 50U) ? 1 : 0) :
                    ((lbl_1_bss_84[playerNo].unk00 == 1) ? ((frandmod(100) < 35U) ? 1 : 0) :
                    ((lbl_1_bss_84[playerNo].unk00 == 2) ? ((frandmod(100) < 20U) ? 1 : 0) :
                    ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -28, 0, 256, 0);
                } else {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, 256, 0);
                }
                lbl_1_bss_114[playerNo].unk44 = 1;
            }
            break;
        case 29:
            Hu3DModelObjPosGet(lbl_1_bss_30C[29].model, "4st_middle", &targetPos);
            xBoundary = targetPos.x - 80.0f;
            if (playerPos.x > xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], -lbl_1_bss_84[playerNo].unk20, 0, 0, 0);
            } else {
                fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, 256, 0);
                lbl_1_bss_114[playerNo].unk44 = 2;
            }
            break;
        case 32:
            Hu3DModelObjPosGet(lbl_1_bss_30C[32].model, "4st_lift00", &savedPos);
            Hu3DModelObjPosGet(lbl_1_bss_30C[33].model, "4st_lift01", &targetPos);
            if (savedPos.y >= targetPos.y) {
                if ((s32) ((lbl_1_bss_84[playerNo].unk00 == 0) ? ((frandmod(100) < 50U) ? 1 : 0) :
                    ((lbl_1_bss_84[playerNo].unk00 == 1) ? ((frandmod(100) < 35U) ? 1 : 0) :
                    ((lbl_1_bss_84[playerNo].unk00 == 2) ? ((frandmod(100) < 20U) ? 1 : 0) :
                    ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -14, 0, 256, 0);
                } else {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, 256, 0);
                }
                lbl_1_bss_114[playerNo].unk44 = 3;
            } else {
                targetPos.x = targetPos.z = 0.0f;
                savedPos.x = savedPos.z = 0.0f;
                PSVECSubtract(&targetPos, &savedPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 160.0f) {
                    if ((s32) ((lbl_1_bss_84[playerNo].unk00 == 0) ? ((frandmod(100) < 50U) ? 1 : 0) :
                        ((lbl_1_bss_84[playerNo].unk00 == 1) ? ((frandmod(100) < 35U) ? 1 : 0) :
                        ((lbl_1_bss_84[playerNo].unk00 == 2) ? ((frandmod(100) < 20U) ? 1 : 0) :
                        ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], -14, 0, 256, 0);
                    } else {
                        fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, 256, 0);
                    }
                    lbl_1_bss_114[playerNo].unk44 = 3;
                }
            }
            break;
        case 33:
            Hu3DModelObjPosGet(lbl_1_bss_30C[33].model, "4st_lift01", &savedPos);
            Hu3DModelObjPosGet(lbl_1_bss_30C[34].model, "4st_lift02", &targetPos);
            if (savedPos.y >= targetPos.y) {
                if ((s32) ((lbl_1_bss_84[playerNo].unk00 == 0) ? ((frandmod(100) < 50U) ? 1 : 0) :
                    ((lbl_1_bss_84[playerNo].unk00 == 1) ? ((frandmod(100) < 35U) ? 1 : 0) :
                    ((lbl_1_bss_84[playerNo].unk00 == 2) ? ((frandmod(100) < 20U) ? 1 : 0) :
                    ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -14, 0, 256, 0);
                } else {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, 256, 0);
                }
                lbl_1_bss_114[playerNo].unk44 = 4;
            } else {
                targetPos.x = targetPos.z = 0.0f;
                savedPos.x = savedPos.z = 0.0f;
                PSVECSubtract(&targetPos, &savedPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 160.0f) {
                    if ((s32) ((lbl_1_bss_84[playerNo].unk00 == 0) ? ((frandmod(100) < 50U) ? 1 : 0) :
                        ((lbl_1_bss_84[playerNo].unk00 == 1) ? ((frandmod(100) < 35U) ? 1 : 0) :
                        ((lbl_1_bss_84[playerNo].unk00 == 2) ? ((frandmod(100) < 20U) ? 1 : 0) :
                        ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], -14, 0, 256, 0);
                    } else {
                        fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, 256, 0);
                    }
                    lbl_1_bss_114[playerNo].unk44 = 4;
                }
            }
            break;
        case 34:
            Hu3DModelObjPosGet(lbl_1_bss_30C[34].model, "4st_lift02", &savedPos);
            Hu3DModelObjPosGet(lbl_1_bss_30C[30].model, "4st_goal", &targetPos);
            if (savedPos.y >= targetPos.y) {
                if ((s32) ((lbl_1_bss_84[playerNo].unk00 == 0) ? ((frandmod(100) < 50U) ? 1 : 0) :
                    ((lbl_1_bss_84[playerNo].unk00 == 1) ? ((frandmod(100) < 35U) ? 1 : 0) :
                    ((lbl_1_bss_84[playerNo].unk00 == 2) ? ((frandmod(100) < 20U) ? 1 : 0) :
                    ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -14, 0, 256, 0);
                } else {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, 256, 0);
                }
                lbl_1_bss_114[playerNo].unk44 = 1;
            } else {
                targetPos.x = targetPos.z = 0.0f;
                savedPos.x = savedPos.z = 0.0f;
                PSVECSubtract(&targetPos, &savedPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 160.0f) {
                    if ((s32) ((lbl_1_bss_84[playerNo].unk00 == 0) ? ((frandmod(100) < 50U) ? 1 : 0) :
                        ((lbl_1_bss_84[playerNo].unk00 == 1) ? ((frandmod(100) < 35U) ? 1 : 0) :
                        ((lbl_1_bss_84[playerNo].unk00 == 2) ? ((frandmod(100) < 20U) ? 1 : 0) :
                        ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], -14, 0, 256, 0);
                    } else {
                        fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, 256, 0);
                    }
                    lbl_1_bss_114[playerNo].unk44 = 1;
                }
            }
            break;
        }
    }
    if (lbl_1_bss_114[playerNo].unk44 != 0) {
        lbl_1_bss_84[playerNo].unk1C = 256;
        if (lbl_1_bss_114[playerNo].unk44 == 2) {
            if ((s32) ((lbl_1_bss_84[playerNo].unk00 == 0) ? ((frandmod(100) < 50U) ? 1 : 0) :
                ((lbl_1_bss_84[playerNo].unk00 == 1) ? ((frandmod(100) < 35U) ? 1 : 0) :
                ((lbl_1_bss_84[playerNo].unk00 == 2) ? ((frandmod(100) < 20U) ? 1 : 0) :
                (((frandmod(100) < 5U) ? 1 : 0) == 0)))) != 0) {
                Hu3DModelObjPosGet(lbl_1_bss_30C[32].model, "4st_lift00", &liftPos);
                if (playerPos.x <= liftPos.x) {
                    lbl_1_bss_84[playerNo].stickX = 0;
                    lbl_1_bss_114[playerNo].unk44 = 1;
                }
            }
        } else if (lbl_1_bss_114[playerNo].unk44 == 3) {
            if ((s32) ((lbl_1_bss_84[playerNo].unk00 == 0) ? ((frandmod(100) < 50U) ? 1 : 0) :
                ((lbl_1_bss_84[playerNo].unk00 == 1) ? ((frandmod(100) < 35U) ? 1 : 0) :
                ((lbl_1_bss_84[playerNo].unk00 == 2) ? ((frandmod(100) < 20U) ? 1 : 0) :
                (((frandmod(100) < 5U) ? 1 : 0) == 0)))) != 0) {
                Hu3DModelObjPosGet(lbl_1_bss_30C[33].model, "4st_lift01", &liftPos);
                if (playerPos.x <= liftPos.x) {
                    lbl_1_bss_84[playerNo].stickX = 0;
                    lbl_1_bss_114[playerNo].unk44 = 1;
                }
            }
        } else if (lbl_1_bss_114[playerNo].unk44 == 4) {
            if ((s32) ((lbl_1_bss_84[playerNo].unk00 == 0) ? ((frandmod(100) < 50U) ? 1 : 0) :
                ((lbl_1_bss_84[playerNo].unk00 == 1) ? ((frandmod(100) < 35U) ? 1 : 0) :
                ((lbl_1_bss_84[playerNo].unk00 == 2) ? ((frandmod(100) < 20U) ? 1 : 0) :
                (((frandmod(100) < 5U) ? 1 : 0) == 0)))) != 0) {
                Hu3DModelObjPosGet(lbl_1_bss_30C[34].model, "4st_lift02", &liftPos);
                if (playerPos.x <= liftPos.x) {
                    lbl_1_bss_84[playerNo].stickX = 0;
                    lbl_1_bss_114[playerNo].unk44 = 1;
                }
            }
        }
    }
    MgPlayerPadSet(lbl_1_bss_114[playerNo].player, lbl_1_bss_84[playerNo].stickX, lbl_1_bss_84[playerNo].stickY, lbl_1_bss_84[playerNo].button, lbl_1_bss_84[playerNo].unk1C);
}

void fn_1_C450(M618Input *input, int stickX, int stickY, int button, int unk1C)
{
    input->stickX = stickX;
    input->stickY = stickY;
    input->button = button;
    input->unk1C = unk1C;
}
