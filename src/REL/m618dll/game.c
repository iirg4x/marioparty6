/* Lift Leapers course setup, movement, CPU decisions, and results. */
#include "REL/m618dll.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/frand.h"
#include "game/gamework.h"
#include "game/hsfex.h"

#define M618_BGM_LIFT_LEAPERS 82 /* Music during the minigame. */
#define M618_SOUND_PLAYER_FALL_1 1749 /* Fall cue for player 1. */
#define M618_SOUND_PLAYER_FALL_2 1750 /* Fall cue for player 2. */
#define M618_SOUND_PLAYER_FALL_3 1751 /* Fall cue for player 3. */
#define M618_SOUND_PLAYER_FALL_4 1752 /* Fall cue for player 4. */
#define M618_SOUND_PLAYER_SECTION_1 1753 /* Course-section cue for player 1. */
#define M618_SOUND_PLAYER_SECTION_2 1754 /* Course-section cue for player 2. */
#define M618_SOUND_PLAYER_SECTION_3 1755 /* Course-section cue for player 3. */
#define M618_SOUND_PLAYER_SECTION_4 1756 /* Course-section cue for player 4. */
#define M618_SOUND_START 1758 /* Cue when players move to the starting position. */
#define M618_CHAR_EFFECT_FALL 576 /* Character effect played when a player falls. */

int lbl_1_data_28[4] = { 1, 2, 4, 8 };
unsigned int lbl_1_data_38[4] = { DATANUM(DATA_mariomot, 6), DATANUM(DATA_mariomot, 34),
                                  DATANUM(DATA_mariomot, 40), DATANUM(DATA_mariomot, 63) };
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
unsigned char m618UnknownBss3C[4]; /* Four bytes whose game use is not established. */
int lbl_1_bss_38;
HuVecF lbl_1_bss_8[4];

/* Called by initialization callback fn_1_DC to load the course, create players, reset state, and
 * create the timer. */
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
    lbl_1_bss_53C = GWRecordGet(GW_RECORD_M618);
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

/* Called by lbl_1_data_0 main callback fn_1_2B4 to update play and detect a finish. */
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

/* Called from fade-in after frame 30: play the start cue once, move each player right until the
 * section start, then enable COMSTK control and save a raised, rearward return position. */
void fn_1_644(void)
{
    int player;
    float destX;
    if (lbl_1_bss_38 == -1) {
        lbl_1_bss_38 = HuAudFXPlay(M618_SOUND_START);
    }
    for (player = 0; player < 4; player++) {
        destX = 160.0f + lbl_1_data_48[lbl_1_bss_114[player].stageSection].x;
        if (lbl_1_bss_114[player].player->actor->pos.x < destX) {
            MgPlayerPadSet(lbl_1_bss_114[player].player, 10, 0, 0, 0);
            /* This clamp follows the pos.x < destX branch, so its >= check is unreachable. */
            if (lbl_1_bss_114[player].player->actor->pos.x >= destX) {
                lbl_1_bss_114[player].player->actor->pos.x = destX;
            }
        } else {
            MgPlayerAttrSet(lbl_1_bss_114[player].player, MGPLAYER_ATTR_COMSTK);
            lbl_1_bss_114[player].recoveryPositionValid = 0;
            lbl_1_bss_114[player].recoveryPosition = lbl_1_bss_114[player].player->actor->pos;
            lbl_1_bss_114[player].recoveryPosition.y += 300.0f;
            lbl_1_bss_114[player].recoveryPosition.z -= 160.0f;
        }
    }
    MgActorExec();
    fn_1_4BE0();
}

/* Starts BGM on the first start-callback call; after 120 calls, clears CPU-stick control and
 * updates actors each call. */
void fn_1_84C(void)
{
    if (++lbl_1_bss_558 >= 120) {
        MgPlayerAttrReset(lbl_1_bss_114[0].player, MGPLAYER_ATTR_COMSTK);
        MgPlayerAttrReset(lbl_1_bss_114[1].player, MGPLAYER_ATTR_COMSTK);
        MgPlayerAttrReset(lbl_1_bss_114[2].player, MGPLAYER_ATTR_COMSTK);
        MgPlayerAttrReset(lbl_1_bss_114[3].player, MGPLAYER_ATTR_COMSTK);
        fn_1_214C();
        MgActorExec();
        fn_1_4BE0();
    }
    if (lbl_1_bss_40 == -1) {
        lbl_1_bss_40 = HuAudBGMPlay(M618_BGM_LIFT_LEAPERS);
    }
}

/* Called by finish callback fn_1_2F0 to record results and settle the players. */
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
            if ((_CheckFlag(FLAG_MG_PRACTICE) == 0) && (GwPlayerConf[lbl_1_bss_54C].type == 0) &&
                (recordTime < lbl_1_bss_53C)) {
                MgSeqRecordSet(recordTime);
                lbl_1_bss_558 += 150;
                GWRecordSet(GW_RECORD_M618, (u32) recordTime);
            }
        }
        MgPlayerAttrSet(lbl_1_bss_114->player, MGPLAYER_ATTR_COMSTK);
        MgPlayerAttrSet(lbl_1_bss_114[1].player, MGPLAYER_ATTR_COMSTK);
        MgPlayerAttrSet(lbl_1_bss_114[2].player, MGPLAYER_ATTR_COMSTK);
        MgPlayerAttrSet(lbl_1_bss_114[3].player, MGPLAYER_ATTR_COMSTK);
        lbl_1_bss_554 = 1;
        fn_1_6298();
        HuAudSStreamFadeOut(lbl_1_bss_40, 100);
    }
    if ((lbl_1_bss_114->fallState != 0) ||
        ((u32) (lbl_1_bss_114->player->actor->colGroundAttr & 0x407F) == 0) ||
        (lbl_1_bss_114[1].fallState != 0) ||
        ((u32) (lbl_1_bss_114[1].player->actor->colGroundAttr & 0x407F) == 0) ||
        (lbl_1_bss_114[2].fallState != 0) ||
        ((u32) (lbl_1_bss_114[2].player->actor->colGroundAttr & 0x407F) == 0) ||
        (lbl_1_bss_114[3].fallState != 0) ||
        ((u32) (lbl_1_bss_114[3].player->actor->colGroundAttr & 0x407F) == 0)) {
        if (lbl_1_bss_114->fallState == 3) {
            MgPlayerAttrReset(lbl_1_bss_114->player, MGPLAYER_ATTR_COMSTK);
        }
        if (lbl_1_bss_114[1].fallState == 3) {
            MgPlayerAttrReset(lbl_1_bss_114[1].player, MGPLAYER_ATTR_COMSTK);
        }
        if (lbl_1_bss_114[2].fallState == 3) {
            MgPlayerAttrReset(lbl_1_bss_114[2].player, MGPLAYER_ATTR_COMSTK);
        }
        if (lbl_1_bss_114[3].fallState == 3) {
            MgPlayerAttrReset(lbl_1_bss_114[3].player, MGPLAYER_ATTR_COMSTK);
        }
        fn_1_214C();
        if (lbl_1_bss_114->fallState != 3) {
            MgPlayerAttrSet(lbl_1_bss_114->player, MGPLAYER_ATTR_COMSTK);
        }
        if (lbl_1_bss_114[1].fallState != 3) {
            MgPlayerAttrSet(lbl_1_bss_114[1].player, MGPLAYER_ATTR_COMSTK);
        }
        if (lbl_1_bss_114[2].fallState != 3) {
            MgPlayerAttrSet(lbl_1_bss_114[2].player, MGPLAYER_ATTR_COMSTK);
        }
        if (lbl_1_bss_114[3].fallState != 3) {
            MgPlayerAttrSet(lbl_1_bss_114[3].player, MGPLAYER_ATTR_COMSTK);
        }
    }
    if (lbl_1_bss_54C != -1) {
        lbl_1_bss_114[lbl_1_bss_54C].player->actor->pos = lbl_1_bss_8[lbl_1_bss_54C];
    }
    MgActorExec();
    fn_1_4BE0();
}

/* Called by pre-winner callback fn_1_310 to advance winner or draw presentation. */
int fn_1_DA8(void)
{
    s32 winner;

    winner = lbl_1_bss_54C;
    if ((s32) lbl_1_bss_54C != -1) {
        fn_1_796C(lbl_1_bss_54C);
        if (++lbl_1_bss_554 >= 30) {
            if ((s32) lbl_1_bss_554 == 30) {
                MgPlayerAttrReset(lbl_1_bss_114[lbl_1_bss_54C].player, MGPLAYER_ATTR_COMSTK);
                MgPlayerPadSet(lbl_1_bss_114[lbl_1_bss_54C].player, 0, 0, PAD_BUTTON_A, 0);
            } else if ((u32) (lbl_1_bss_114[lbl_1_bss_54C].player->actor->colGroundAttr & 0x407F) ==
                       0) {
                MgPlayerPadSet(lbl_1_bss_114[lbl_1_bss_54C].player, 0, 0, 0, PAD_BUTTON_A);
            } else {
                MgPlayerAttrSet(lbl_1_bss_114[lbl_1_bss_54C].player, MGPLAYER_ATTR_COMSTK);
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
            Hu3DModelAttrReset(lbl_1_bss_540[0], HU3D_ATTR_DISPOFF);
            Hu3DMotionSpeedSet(lbl_1_bss_540[0], 1.0f);
        }
        if ((s32) lbl_1_bss_554 == 120) {
            Hu3DModelAttrReset(lbl_1_bss_540[1], HU3D_ATTR_DISPOFF);
            Hu3DMotionSpeedSet(lbl_1_bss_540[1], 1.0f);
        }
        if (((s32) lbl_1_bss_554 == 90) && (_CheckFlag(FLAG_MG_PRACTICE) == 0) &&
            (GwPlayerConf[lbl_1_bss_54C].type == 0) &&
            (MgTimerValueGet(lbl_1_bss_544) < (s32) lbl_1_bss_53C)) {
            MgTimerRecordSet(lbl_1_bss_544, -1);
        }
        lbl_1_bss_558 -= 1;
        if ((s32) lbl_1_bss_558 < 0) {
            lbl_1_bss_558 = -1;
        }
        if ((s32) lbl_1_bss_558 == 0) {
            CharMotionShiftSet(GwPlayerConf[lbl_1_bss_54C].charNo,
                               lbl_1_bss_114[lbl_1_bss_54C].motion[0], 0.0f, 8.0f, 0U);
            winner = -1;
        }
        if ((lbl_1_bss_114->fallState == 1) && (lbl_1_bss_114->routeState == 1)) {
            lbl_1_bss_114->player->actor->velY = 0.0f;
        }
        if ((lbl_1_bss_114[1].fallState == 1) && (lbl_1_bss_114[1].routeState == 1)) {
            lbl_1_bss_114[1].player->actor->velY = 0.0f;
        }
        if ((lbl_1_bss_114[2].fallState == 1) && (lbl_1_bss_114[2].routeState == 1)) {
            lbl_1_bss_114[2].player->actor->velY = 0.0f;
        }
        if ((lbl_1_bss_114[3].fallState == 1) && (lbl_1_bss_114[3].routeState == 1)) {
            lbl_1_bss_114[3].player->actor->velY = 0.0f;
        }
    } else {
        MgPlayerAttrSet(lbl_1_bss_114->player, MGPLAYER_ATTR_COMSTK);
        MgPlayerAttrSet(lbl_1_bss_114[1].player, MGPLAYER_ATTR_COMSTK);
        MgPlayerAttrSet(lbl_1_bss_114[2].player, MGPLAYER_ATTR_COMSTK);
        MgPlayerAttrSet(lbl_1_bss_114[3].player, MGPLAYER_ATTR_COMSTK);
        if ((lbl_1_bss_114->fallState == 0) &&
            ((u32) (lbl_1_bss_114->player->actor->colGroundAttr & 0x407F) != 0) &&
            (lbl_1_bss_114[1].fallState == 0) &&
            ((u32) (lbl_1_bss_114[1].player->actor->colGroundAttr & 0x407F) != 0) &&
            (lbl_1_bss_114[2].fallState == 0) &&
            ((u32) (lbl_1_bss_114[2].player->actor->colGroundAttr & 0x407F) != 0) &&
            (lbl_1_bss_114[3].fallState == 0) &&
            ((u32) (lbl_1_bss_114[3].player->actor->colGroundAttr & 0x407F) != 0)) {
            if (++lbl_1_bss_558 < 15) {
                winner = -2;
                MgSeqModeChangeOff();
            } else {
                CharMotionShiftSet(GwPlayerConf->charNo, lbl_1_bss_114->motion[2], 0.0f, 8.0f, 0U);
                CharMotionShiftSet(GwPlayerConf[1].charNo, lbl_1_bss_114[1].motion[2], 0.0f, 8.0f,
                                   0U);
                CharMotionShiftSet(GwPlayerConf[2].charNo, lbl_1_bss_114[2].motion[2], 0.0f, 8.0f,
                                   0U);
                CharMotionShiftSet(GwPlayerConf[3].charNo, lbl_1_bss_114[3].motion[2], 0.0f, 8.0f,
                                   0U);
                MgSeqModeChangeOn();
            }
        } else {
            winner = -2;
            MgSeqModeChangeOff();
            if (lbl_1_bss_114->fallState == 3) {
                MgPlayerAttrReset(lbl_1_bss_114->player, MGPLAYER_ATTR_COMSTK);
            }
            if (lbl_1_bss_114[1].fallState == 3) {
                MgPlayerAttrReset(lbl_1_bss_114[1].player, MGPLAYER_ATTR_COMSTK);
            }
            if (lbl_1_bss_114[2].fallState == 3) {
                MgPlayerAttrReset(lbl_1_bss_114[2].player, MGPLAYER_ATTR_COMSTK);
            }
            if (lbl_1_bss_114[3].fallState == 3) {
                MgPlayerAttrReset(lbl_1_bss_114[3].player, MGPLAYER_ATTR_COMSTK);
            }
            fn_1_214C();
            if (lbl_1_bss_114->fallState != 3) {
                MgPlayerAttrSet(lbl_1_bss_114->player, MGPLAYER_ATTR_COMSTK);
            }
            if (lbl_1_bss_114[1].fallState != 3) {
                MgPlayerAttrSet(lbl_1_bss_114[1].player, MGPLAYER_ATTR_COMSTK);
            }
            if (lbl_1_bss_114[2].fallState != 3) {
                MgPlayerAttrSet(lbl_1_bss_114[2].player, MGPLAYER_ATTR_COMSTK);
            }
            if (lbl_1_bss_114[3].fallState != 3) {
                MgPlayerAttrSet(lbl_1_bss_114[3].player, MGPLAYER_ATTR_COMSTK);
            }
        }
    }
    MgActorExec();
    fn_1_4BE0();
    return winner;
}

/* Called during setup and fade-in to initialize player views in camera mode 0 and refresh the
 * course camera layout. */
void fn_1_1B40(int cameraMode)
{
    int player;

    if (cameraMode == 0) {
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

/* Called by fn_1_378 to create players, motions, and CPU input state. */
void fn_1_1CE0(void)
{
    MGACTOR_PARAM actorParams;
    HuVecF playerModelPosition;
    s16 characterNo;
    int playersWithFastAiSpeed;
    int motionIndex;
    int playerIndex;

    playersWithFastAiSpeed = 0;
    actorParams.height = 150.0f;
    actorParams.radius = 40.0f;
    actorParams.param = 0;
    actorParams.type = 0;
    actorParams.attr = 4;
    actorParams.narrowHook = NULL;
    actorParams.correctHookParam = 0;
    actorParams.correctHook = NULL;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        lbl_1_bss_114[playerIndex].player =
            MgPlayerCreate(playerIndex, &actorParams, 4, lbl_1_data_28[playerIndex], 3, NULL);
        lbl_1_bss_114[playerIndex].player->actor->pos.x = lbl_1_data_48[0].x;
        lbl_1_bss_114[playerIndex].player->actor->pos.y = lbl_1_data_48[0].y;
        lbl_1_bss_114[playerIndex].player->actor->pos.z = 0.0f;
        lbl_1_bss_114[playerIndex].player->actor->rotY = 90.0f;
        lbl_1_bss_114[playerIndex].moveDirection = 1;
        lbl_1_bss_114[playerIndex].stageSection = 0;
        Hu3DModelCameraSet(lbl_1_bss_114[playerIndex].player->actor->mdlId,
                           lbl_1_data_28[playerIndex]);
        lbl_1_bss_114[playerIndex].fallState = 0;
        lbl_1_bss_114[playerIndex].fallFrames = 0;
        characterNo = GwPlayerConf[playerIndex].charNo;
        for (motionIndex = 0; motionIndex < 4; motionIndex++) {
            lbl_1_bss_114[playerIndex].motion[motionIndex] =
                CharMotionCreate(characterNo, lbl_1_data_38[motionIndex]);
        }
        CharMotionDataClose(characterNo);
        lbl_1_bss_114[playerIndex].model = Hu3DModelCreateData(DATANUM(DATA_m618, 0));
        Hu3DModelCameraSet(lbl_1_bss_114[playerIndex].model, lbl_1_data_28[playerIndex]);
        Hu3DModelPosGet(lbl_1_bss_114[playerIndex].player->actor->mdlId, &playerModelPosition);
        Hu3DModelPosSetV(lbl_1_bss_114[playerIndex].model, &playerModelPosition);
        Hu3DModelRotSet(lbl_1_bss_114[playerIndex].model, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet(lbl_1_bss_114[playerIndex].model, HU3D_MOTATTR_LOOP);
        Hu3DModelLayerSet(lbl_1_bss_114[playerIndex].model, 2);
        Hu3DModelAttrSet(lbl_1_bss_114[playerIndex].model, HU3D_ATTR_DISPOFF);
        lbl_1_bss_84[playerIndex].cpuDifficulty = GwPlayerConf[playerIndex].comDif;
        lbl_1_bss_84[playerIndex].aiSpeed = frandmod(3) == 0U ? 46 : (frandmod(3) == 1U ? 41 : 56);
        if (lbl_1_bss_84[playerIndex].aiSpeed == 56) {
            playersWithFastAiSpeed++;
        }
    }
    if (playersWithFastAiSpeed == 0) {
        /* If no player has the fastest AI speed, assign speed 56 to one randomly selected
         * player. */
        lbl_1_bss_84[frandmod(4)].aiSpeed = 56;
    }
    MgActorExec();
}

/* Called by the play, start, and winner callbacks to update falls, returns, CPU decisions, and goal
 * crossings. */
void fn_1_214C(void)
{
    s32 goalPlayers[4];
    Point3d playerPosition;
    Point3d screenPosition;
    f32 facingAngle;
    f32 sectionMinX;
    f32 sectionMaxX;
    s32 hasRecoveryPoint;
    s32 goalPlayerCount;
    s32 playerIndex;

    goalPlayerCount = 0;
    playerIndex = 0;
    while (playerIndex < 4) {
        if (lbl_1_bss_114[playerIndex].fallState == 3) {
            fn_1_3294(playerIndex);
        } else if (lbl_1_bss_114[playerIndex].fallState == 1) {
            fn_1_44CC(playerIndex);
            lbl_1_bss_44[playerIndex][3] = -1;
        } else {
            if (GwPlayerConf[playerIndex].type != 0) {
                fn_1_8180(playerIndex);
            }
            playerPosition = lbl_1_bss_114[playerIndex].player->actor->pos;
            Hu3D3Dto2D(&playerPosition, (s16) lbl_1_data_28[playerIndex], &screenPosition);
            /* The third section suppresses the fall cue while the player is still high above its
             * lower route. */
            if ((((lbl_1_bss_114[playerIndex].stageSection != 2) && (screenPosition.y > 480.0f)) ||
                 ((lbl_1_bss_114[playerIndex].stageSection == 2) && (screenPosition.y > 480.0f) &&
                  (playerPosition.y < 1600.0f))) &&
                ((s32) lbl_1_bss_44[playerIndex][3] == -1)) {
                lbl_1_bss_44[playerIndex][3] = HuAudFXPlay(
                    playerIndex == 0
                        ? M618_SOUND_PLAYER_FALL_1
                        : (playerIndex == 1 ? M618_SOUND_PLAYER_FALL_2
                                            : (playerIndex == 2 ? M618_SOUND_PLAYER_FALL_3
                                                                : M618_SOUND_PLAYER_FALL_4)));
                MgPlayerAttrSet(lbl_1_bss_114[playerIndex].player, MGPLAYER_ATTR_COMSTK);
                if (lbl_1_bss_114[playerIndex].fallState != 2) {
                    CharFXPlay(GwPlayerConf[playerIndex].charNo, M618_CHAR_EFFECT_FALL);
                }
            }
            if (((u32) (lbl_1_bss_114[playerIndex].player->actor->colGroundAttr & 0x407F) == 0) ||
                (lbl_1_bss_114[playerIndex].fallState == 2)) {
                if (lbl_1_bss_114[playerIndex].fallState != 1) {
                    if (++lbl_1_bss_114[playerIndex].fallFrames == 100) {
                        if (lbl_1_bss_114[playerIndex].fallState == 2) {
                            MgPlayerSpawn(lbl_1_bss_114[playerIndex].player,
                                          &lbl_1_bss_114[playerIndex].player->actor->pos);
                        }
                        lbl_1_bss_114[playerIndex].fallState = 1;
                        lbl_1_bss_114[playerIndex].fallFrames = 0;
                        lbl_1_bss_114[playerIndex].routeState = 0;
                        lbl_1_bss_114[playerIndex].recoveryAngle = 0.0f;
                        MgPlayerAttrSet(lbl_1_bss_114[playerIndex].player, MGPLAYER_ATTR_COMSTK);
                        if ((lbl_1_bss_114[playerIndex].stageSection == 2) &&
                            (lbl_1_bss_114[playerIndex].player->actor->pos.x < 4850.0f)) {
                            lbl_1_bss_114[playerIndex].player->actor->rotY = -90.0f;
                            lbl_1_bss_114[playerIndex].moveDirection = -1;
                        }
                    }
                }
            } else {
                lbl_1_bss_114[playerIndex].fallState = 0;
                lbl_1_bss_114[playerIndex].fallFrames = 0;
                MgPlayerAttrReset(lbl_1_bss_114[playerIndex].player, MGPLAYER_ATTR_COMSTK);
                if (((u32) (lbl_1_bss_114[playerIndex].player->actor->colGroundAttr & 0x407F) !=
                     0) &&
                    (((lbl_1_bss_114[playerIndex].stageSection == 0) &&
                      (lbl_1_bss_114[playerIndex].player->actor->pos.x > 2300.0f)) ||
                     ((lbl_1_bss_114[playerIndex].stageSection == 1) &&
                      (lbl_1_bss_114[playerIndex].player->actor->pos.x >= 5955.0f)) ||
                     ((lbl_1_bss_114[playerIndex].stageSection == 2) &&
                      (lbl_1_bss_114[playerIndex].player->actor->pos.x <= 5030.0f) &&
                      (lbl_1_bss_114[playerIndex].player->actor->pos.y >= 3460.0f)))) {
                    lbl_1_bss_114[playerIndex].fallState = 3;
                    lbl_1_bss_114[playerIndex].routeState = 0;
                    lbl_1_bss_114[playerIndex].actionFrames = 0;
                    omVibrate((s16) playerIndex, 20, 4, 4);
                }
                if (((s32) lbl_1_bss_548 == 0) && (lbl_1_bss_114[playerIndex].stageSection == 3) &&
                    ((u32) (lbl_1_bss_114[playerIndex].player->actor->colGroundAttr & 0x407F) !=
                     0)) {
                    if ((lbl_1_bss_114[playerIndex].player->actor->pos.x > 1400.90002f) &&
                        (lbl_1_bss_114[playerIndex].player->actor->pos.x < 1401.0f)) {
                        /* Snap the narrow seam interval to the goal-side collision boundary before
                         * testing its mesh. */
                        lbl_1_bss_114[playerIndex].player->actor->pos.x = 1401.0f;
                    }
                    if ((lbl_1_bss_114[playerIndex].player->actor->pos.x <= 1400.91199f) &&
                        (lbl_1_bss_114[playerIndex].player->actor->colMesh == 30)) {
                        goalPlayers[goalPlayerCount] = playerIndex;
                        goalPlayerCount += 1;
                    }
                }
                if (lbl_1_bss_114[playerIndex].recoveryPositionValid == 0) {
                    hasRecoveryPoint = 0;
                    if (lbl_1_bss_114[playerIndex].player->actor->colMesh == 1) {
                        lbl_1_bss_114[playerIndex].recoveryPosition =
                            (HuVecF){ 1450.0f, 40.0f, 0.0f };
                        hasRecoveryPoint = 1;
                    } else if (lbl_1_bss_114[playerIndex].player->actor->colMesh == 6) {
                        lbl_1_bss_114[playerIndex].recoveryPosition =
                            (HuVecF){ 4800.0f, 170.0f, 0.0f };
                        hasRecoveryPoint = 1;
                    } else if (lbl_1_bss_114[playerIndex].player->actor->colMesh == 29) {
                        lbl_1_bss_114[playerIndex].recoveryPosition =
                            (HuVecF){ 2500.0f, 3547.5f, 0.0f };
                        hasRecoveryPoint = 1;
                    }
                    if (hasRecoveryPoint != 0) {
                        lbl_1_bss_114[playerIndex].recoveryPositionValid += 1;
                        lbl_1_bss_114[playerIndex].recoveryPosition.y += 300.0f;
                        lbl_1_bss_114[playerIndex].recoveryPosition.z -= 160.0f;
                    }
                }
            }
            lbl_1_bss_114[playerIndex].player->actor->pos.z = 0.0f;
            if (lbl_1_bss_114[playerIndex].fallState == 2) {
                lbl_1_bss_114[playerIndex].player->actor->pos.z += 500.0f;
                lbl_1_bss_114[playerIndex].player->actor->pos.y =
                    lbl_1_bss_114[playerIndex].jumpStartHeight +
                    ((15.0f * (f32) lbl_1_bss_114[playerIndex].jumpFrames) -
                     (0.5f * (f32) lbl_1_bss_114[playerIndex].jumpFrames *
                      (f32) lbl_1_bss_114[playerIndex].jumpFrames));
                lbl_1_bss_114[playerIndex].jumpFrames += 1;
                Hu3DModelPosSetV((s16) lbl_1_bss_114[playerIndex].player->actor->mdlId,
                                 &lbl_1_bss_114[playerIndex].player->actor->pos);
            }
            if ((lbl_1_bss_114[playerIndex].player->actor->rotY >= 0.0f) &&
                (lbl_1_bss_114[playerIndex].player->actor->rotY < 180.0f)) {
                lbl_1_bss_114[playerIndex].moveDirection = 1;
                lbl_1_bss_114[playerIndex].player->actor->rotY = 90.0f;
            } else {
                lbl_1_bss_114[playerIndex].moveDirection = -1;
                lbl_1_bss_114[playerIndex].player->actor->rotY = -90.0f;
            }
            if ((lbl_1_bss_114[playerIndex].player->actor->rotY != 90.0f) &&
                (lbl_1_bss_114[playerIndex].player->actor->rotY != -90.0f)) {
                if (lbl_1_bss_114[playerIndex].moveDirection == 1) {
                    facingAngle = 90.0f;
                } else {
                    facingAngle = -90.0f;
                }
                lbl_1_bss_114[playerIndex].player->actor->rotY = facingAngle;
            }
            if (lbl_1_bss_114[playerIndex].stageSection == 0) {
                sectionMinX = 155.0f;
                sectionMaxX = 2450.0f;
            } else if (lbl_1_bss_114[playerIndex].stageSection == 1) {
                sectionMinX = 155.0f;
                sectionMaxX = 2880.0f;
            } else if (lbl_1_bss_114[playerIndex].stageSection == 2) {
                sectionMinX = 230.0f;
                sectionMaxX = 2050.0f;
            } else if (lbl_1_bss_114[playerIndex].stageSection == 3) {
                sectionMinX = 155.0f;
                sectionMaxX = 2850.0f;
            }
            sectionMinX += lbl_1_data_48[lbl_1_bss_114[playerIndex].stageSection].x;
            sectionMaxX += lbl_1_data_48[lbl_1_bss_114[playerIndex].stageSection].x;
            if (lbl_1_bss_114[playerIndex].player->actor->pos.x < sectionMinX) {
                lbl_1_bss_114[playerIndex].player->actor->pos.x = 0.100000001f + sectionMinX;
            } else if (lbl_1_bss_114[playerIndex].player->actor->pos.x > sectionMaxX) {
                lbl_1_bss_114[playerIndex].player->actor->pos.x = sectionMaxX - 0.100000001f;
            }
            if (((u32) (lbl_1_bss_114[playerIndex].player->actor->colGroundAttr & 0x1000) != 0) &&
                (lbl_1_bss_114[playerIndex].fallState == 0)) {
                lbl_1_bss_114[playerIndex].fallState = 2;
                MgPlayerAttrSet(lbl_1_bss_114[playerIndex].player, MGPLAYER_ATTR_COMSTK);
                CharMotionShiftSet(GwPlayerConf[playerIndex].charNo,
                                   lbl_1_bss_114[playerIndex].motion[1], 0.0f, 8.0f, 0U);
                lbl_1_bss_114[playerIndex].player->actor->pos.y += 50.0f;
                lbl_1_bss_114[playerIndex].player->actor->pos.z += 500.0f;
                omVibrate((s16) playerIndex, 20, 7, 3);
                lbl_1_bss_114[playerIndex].jumpFrames = 0;
                lbl_1_bss_114[playerIndex].jumpStartHeight =
                    lbl_1_bss_114[playerIndex].player->actor->pos.y;
                MgPlayerDespawn(lbl_1_bss_114[playerIndex].player);
            }
        }
        playerIndex += 1;
    }
    if (goalPlayerCount != 0) {
        /* A simultaneous goal crossing selects one of the qualifying players at random. */
        lbl_1_bss_54C = goalPlayers[frandmod(goalPlayerCount)];
        lbl_1_bss_548 = 1;
        MgPlayerAttrSet(lbl_1_bss_114[lbl_1_bss_54C].player, MGPLAYER_ATTR_COMSTK);
        lbl_1_bss_8[lbl_1_bss_54C] = lbl_1_bss_114[lbl_1_bss_54C].player->actor->pos;
    }
}

/* Called by fn_1_214C for finished players to cross the remaining sections and return to active
 * play. */
void fn_1_3294(int playerNo)
{
    f32 endX;

    if (lbl_1_bss_114[playerNo].routeState == 0) {
        if (lbl_1_bss_114[playerNo].stageSection == 0) {
            if (lbl_1_bss_114[playerNo].player->actor->pos.x <
                (2600.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].stageSection].x)) {
                if ((lbl_1_bss_114[playerNo].player->actor->pos.x >= 2500.0f) &&
                    ((s32) lbl_1_bss_44[playerNo][0] == -1)) {
                    lbl_1_bss_44[playerNo][0] = HuAudFXPlay(
                        playerNo == 0
                            ? M618_SOUND_PLAYER_SECTION_1
                            : (playerNo == 1 ? M618_SOUND_PLAYER_SECTION_2
                                             : (playerNo == 2 ? M618_SOUND_PLAYER_SECTION_3
                                                              : M618_SOUND_PLAYER_SECTION_4)));
                }
                MgPlayerPadSet(lbl_1_bss_114[playerNo].player, 30, 0, 0, 0);
                if (lbl_1_bss_114[playerNo].player->actor->pos.x >=
                    (2600.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].stageSection].x)) {
                    lbl_1_bss_114[playerNo].player->actor->pos.x =
                        2600.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].stageSection].x;
                }
                lbl_1_bss_114[playerNo].player->actor->rotY = 90.0f;
                lbl_1_bss_114[playerNo].moveDirection = 1;
                return;
            }
            MgPlayerAttrSet(lbl_1_bss_114[playerNo].player, MGPLAYER_ATTR_COMSTK);
            lbl_1_bss_114[playerNo].stageSection += 1;
            lbl_1_bss_114[playerNo].player->actor->pos.x =
                lbl_1_data_48[lbl_1_bss_114[playerNo].stageSection].x;
            lbl_1_bss_114[playerNo].routeState += 1;
            lbl_1_bss_44[playerNo][0] = -1;
            return;
        }
        if (lbl_1_bss_114[playerNo].stageSection == 1) {
            if (lbl_1_bss_114[playerNo].actionFrames == 0) {
                if (lbl_1_bss_114[playerNo].player->actor->pos.x <
                    (3100.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].stageSection].x)) {
                    MgPlayerPadSet(lbl_1_bss_114[playerNo].player, 30, 0, 0, 0);
                    if (lbl_1_bss_114[playerNo].player->actor->pos.x >=
                        (3100.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].stageSection].x)) {
                        lbl_1_bss_114[playerNo].player->actor->pos.x =
                            3100.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].stageSection].x;
                    }
                    lbl_1_bss_114[playerNo].player->actor->rotY = 90.0f;
                    lbl_1_bss_114[playerNo].moveDirection = 1;
                    return;
                }
                MgPlayerPadSet(lbl_1_bss_114[playerNo].player, 0, 0, PAD_BUTTON_A, 0);
                lbl_1_bss_114[playerNo].actionFrames = 1;
                return;
            }
            if ((lbl_1_bss_114[playerNo].actionFrames == 8) &&
                ((s32) lbl_1_bss_44[playerNo][1] == -1)) {
                lbl_1_bss_44[playerNo][1] = HuAudFXPlay(
                    playerNo == 0
                        ? M618_SOUND_PLAYER_SECTION_1
                        : (playerNo == 1 ? M618_SOUND_PLAYER_SECTION_2
                                         : (playerNo == 2 ? M618_SOUND_PLAYER_SECTION_3
                                                          : M618_SOUND_PLAYER_SECTION_4)));
            }
            MgPlayerPadSet(lbl_1_bss_114[playerNo].player, 0, 0, 0, PAD_BUTTON_A);
            if (++lbl_1_bss_114[playerNo].actionFrames == 15) {
                lbl_1_bss_114[playerNo].player->actor->pos.y =
                    lbl_1_data_48[lbl_1_bss_114[playerNo].stageSection + 1].y;
                lbl_1_bss_114[playerNo].stageSection += 1;
                lbl_1_bss_114[playerNo].routeState += 1;
                lbl_1_bss_114[playerNo].actionFrames = 0;
                lbl_1_bss_44[playerNo][1] = -1;
                return;
            }
        } else if (lbl_1_bss_114[playerNo].stageSection == 2) {
            if (lbl_1_bss_114[playerNo].player->actor->pos.x >
                (230.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].stageSection].x)) {
                if ((lbl_1_bss_114[playerNo].player->actor->pos.x <= 4760.0f) &&
                    ((s32) lbl_1_bss_44[playerNo][2] == -1)) {
                    lbl_1_bss_44[playerNo][2] = HuAudFXPlay(
                        playerNo == 0
                            ? M618_SOUND_PLAYER_SECTION_1
                            : (playerNo == 1 ? M618_SOUND_PLAYER_SECTION_2
                                             : (playerNo == 2 ? M618_SOUND_PLAYER_SECTION_3
                                                              : M618_SOUND_PLAYER_SECTION_4)));
                }
                MgPlayerPadSet(lbl_1_bss_114[playerNo].player, -30, 0, 0, 0);
                if (lbl_1_bss_114[playerNo].player->actor->pos.x <=
                    (230.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].stageSection].x)) {
                    lbl_1_bss_114[playerNo].player->actor->pos.x =
                        230.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].stageSection].x;
                }
                lbl_1_bss_114[playerNo].player->actor->rotY = -90.0f;
                lbl_1_bss_114[playerNo].moveDirection = -1;
                return;
            }
            MgPlayerAttrSet(lbl_1_bss_114[playerNo].player, MGPLAYER_ATTR_COMSTK);
            lbl_1_bss_114[playerNo].stageSection += 1;
            lbl_1_bss_114[playerNo].player->actor->pos.x =
                3200.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].stageSection].x;
            lbl_1_bss_114[playerNo].routeState += 1;
            lbl_1_bss_44[playerNo][2] = -1;
            return;
        }
    } else if (lbl_1_bss_114[playerNo].stageSection == 1) {
        endX = 160.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].stageSection].x;
        MgPlayerAttrReset(lbl_1_bss_114[playerNo].player, MGPLAYER_ATTR_COMSTK);
        if (lbl_1_bss_114[playerNo].player->actor->pos.x < endX) {
            if ((lbl_1_bss_114[playerNo].player->actor->pos.x >= 3240.0f) &&
                ((s32) lbl_1_bss_44[playerNo][0] == -1)) {
                lbl_1_bss_44[playerNo][0] = HuAudFXPlay(
                    playerNo == 0
                        ? M618_SOUND_PLAYER_SECTION_1
                        : (playerNo == 1 ? M618_SOUND_PLAYER_SECTION_2
                                         : (playerNo == 2 ? M618_SOUND_PLAYER_SECTION_3
                                                          : M618_SOUND_PLAYER_SECTION_4)));
            }
            MgPlayerPadSet(lbl_1_bss_114[playerNo].player, 30, 0, 0, 0);
            if (lbl_1_bss_114[playerNo].player->actor->pos.x >= endX) {
                lbl_1_bss_114[playerNo].player->actor->pos.x = endX;
                return;
            }
        } else {
            MgPlayerAttrReset(lbl_1_bss_114[playerNo].player, MGPLAYER_ATTR_COMSTK);
            lbl_1_bss_114[playerNo].fallState = 0;
            lbl_1_bss_114[playerNo].recoveryPositionValid = 0;
            lbl_1_bss_114[playerNo].recoveryPosition = lbl_1_bss_114[playerNo].player->actor->pos;
            lbl_1_bss_114[playerNo].recoveryPosition.y += 300.0f;
            lbl_1_bss_114[playerNo].recoveryPosition.z -= 160.0f;
            lbl_1_bss_44[playerNo][0] = -1;
            return;
        }
    } else {
        if (lbl_1_bss_114[playerNo].stageSection == 2) {
            if (lbl_1_bss_114[playerNo].actionFrames < 30) {
                MgPlayerAttrSet(lbl_1_bss_114[playerNo].player, MGPLAYER_ATTR_COMSTK);
                lbl_1_bss_114[playerNo].player->actor->rotY = -90.0f;
                lbl_1_bss_114[playerNo].moveDirection = -1;
            } else if (lbl_1_bss_114[playerNo].actionFrames == 30) {
                MgPlayerAttrReset(lbl_1_bss_114[playerNo].player, MGPLAYER_ATTR_COMSTK);
                MgPlayerPadSet(lbl_1_bss_114[playerNo].player, 0, 0, PAD_BUTTON_A, 0);
            } else {
                if ((lbl_1_bss_114[playerNo].actionFrames == 35) &&
                    ((s32) lbl_1_bss_44[playerNo][1] == -1)) {
                    lbl_1_bss_44[playerNo][1] = HuAudFXPlay(
                        playerNo == 0
                            ? M618_SOUND_PLAYER_SECTION_1
                            : (playerNo == 1 ? M618_SOUND_PLAYER_SECTION_2
                                             : (playerNo == 2 ? M618_SOUND_PLAYER_SECTION_3
                                                              : M618_SOUND_PLAYER_SECTION_4)));
                }
                MgPlayerPadSet(lbl_1_bss_114[playerNo].player, 0, 0, 0, PAD_BUTTON_A);
                if (lbl_1_bss_114[playerNo].actionFrames == 60) {
                    MgPlayerAttrReset(lbl_1_bss_114[playerNo].player, MGPLAYER_ATTR_COMSTK);
                    lbl_1_bss_114[playerNo].fallState = 0;
                    lbl_1_bss_114[playerNo].recoveryPositionValid = 0;
                    lbl_1_bss_114[playerNo].recoveryPosition =
                        lbl_1_bss_114[playerNo].player->actor->pos;
                    lbl_1_bss_114[playerNo].recoveryPosition.y += 300.0f;
                    lbl_1_bss_114[playerNo].recoveryPosition.z -= 160.0f;
                    lbl_1_bss_44[playerNo][1] = -1;
                }
            }
            lbl_1_bss_114[playerNo].actionFrames += 1;
            return;
        }
        if (lbl_1_bss_114[playerNo].stageSection == 3) {
            endX = (3200.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].stageSection].x) - 350.0f;
            MgPlayerAttrReset(lbl_1_bss_114[playerNo].player, MGPLAYER_ATTR_COMSTK);
            if (lbl_1_bss_114[playerNo].player->actor->pos.x > endX) {
                if ((lbl_1_bss_114[playerNo].player->actor->pos.x <= 4000.0f) &&
                    ((s32) lbl_1_bss_44[playerNo][2] == -1)) {
                    lbl_1_bss_44[playerNo][2] = HuAudFXPlay(
                        playerNo == 0
                            ? M618_SOUND_PLAYER_SECTION_1
                            : (playerNo == 1 ? M618_SOUND_PLAYER_SECTION_2
                                             : (playerNo == 2 ? M618_SOUND_PLAYER_SECTION_3
                                                              : M618_SOUND_PLAYER_SECTION_4)));
                }
                MgPlayerPadSet(lbl_1_bss_114[playerNo].player, -30, 0, 0, 0);
                if (lbl_1_bss_114[playerNo].player->actor->pos.x <= endX) {
                    lbl_1_bss_114[playerNo].player->actor->pos.x = endX;
                    return;
                }
            } else {
                MgPlayerAttrReset(lbl_1_bss_114[playerNo].player, MGPLAYER_ATTR_COMSTK);
                lbl_1_bss_114[playerNo].fallState = 0;
                lbl_1_bss_114[playerNo].recoveryPositionValid = 0;
                lbl_1_bss_114[playerNo].recoveryPosition =
                    lbl_1_bss_114[playerNo].player->actor->pos;
                lbl_1_bss_114[playerNo].recoveryPosition.y += 300.0f;
                lbl_1_bss_114[playerNo].recoveryPosition.z -= 160.0f;
                lbl_1_bss_44[playerNo][2] = -1;
            }
        }
    }
}

/* Called by fn_1_214C each frame to raise a fallen player on a lift and return them to the
 * course. */
void fn_1_44CC(int playerNo)
{
    Point3d playerModelPosition;
    Point3d recoveryDirection;
    f32 distanceToRecoveryPosition;

    if (lbl_1_bss_114[playerNo].routeState == 0) {
        Hu3DModelAttrReset(lbl_1_bss_114[playerNo].model, HU3D_ATTR_DISPOFF);
        Hu3DModelPosGet((s16) lbl_1_bss_114[playerNo].player->actor->mdlId, &playerModelPosition);
        lbl_1_bss_114[playerNo].liftModelPosition = playerModelPosition;
        lbl_1_bss_114[playerNo].liftModelPosition.y =
            (f32) ((f64) (2000.0f + playerModelPosition.y) -
                   (1200.0 *
                    sin((3.1415926535897931 * (f64) lbl_1_bss_114[playerNo].recoveryAngle) /
                        180.0)));
        Hu3DModelPosSet(lbl_1_bss_114[playerNo].model, playerModelPosition.x,
                        lbl_1_bss_114[playerNo].liftModelPosition.y, playerModelPosition.z);
        lbl_1_bss_114[playerNo].recoveryAngle += 1.0f;
        if (lbl_1_bss_114[playerNo].recoveryAngle >= 90.0f) {
            lbl_1_bss_114[playerNo].recoveryAngle = 90.0f;
            lbl_1_bss_114[playerNo].routeState += 1;
            CharMotionShiftSet(GwPlayerConf[playerNo].charNo, lbl_1_bss_114[playerNo].motion[3],
                               0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            lbl_1_bss_114[playerNo].player->actor->rotY = 0.0f;
            return;
        }
    } else if (lbl_1_bss_114[playerNo].routeState == 1) {
        /* Move the raised lift model one twentieth of the remaining distance each frame. */
        PSVECSubtract(&lbl_1_bss_114[playerNo].recoveryPosition,
                      &lbl_1_bss_114[playerNo].liftModelPosition, &recoveryDirection);
        distanceToRecoveryPosition = PSVECMag(&recoveryDirection);
        PSVECNormalize(&recoveryDirection, &recoveryDirection);
        PSVECScale(&recoveryDirection, &recoveryDirection, distanceToRecoveryPosition / 20.0f);
        PSVECAdd(&recoveryDirection, &lbl_1_bss_114[playerNo].liftModelPosition,
                 &lbl_1_bss_114[playerNo].liftModelPosition);
        Hu3DModelPosSetV(lbl_1_bss_114[playerNo].model, &lbl_1_bss_114[playerNo].liftModelPosition);
        Hu3DModelObjPosGet(lbl_1_bss_114[playerNo].model, "itemhook_I", &playerModelPosition);
        lbl_1_bss_114[playerNo].player->actor->pos = playerModelPosition;
        lbl_1_bss_114[playerNo].player->actor->velY = 0.0f;
        if (playerModelPosition.z <= 0.0f) {
            lbl_1_bss_114[playerNo].player->actor->pos.z = 0.0f;
            lbl_1_bss_114[playerNo].routeState += 1;
            return;
        }
    } else if (lbl_1_bss_114[playerNo].routeState == 2) {
        if (lbl_1_bss_114[playerNo].stageSection < 2) {
            lbl_1_bss_114[playerNo].player->actor->rotY += 5.0f;
            if (lbl_1_bss_114[playerNo].player->actor->rotY > 90.0f) {
                lbl_1_bss_114[playerNo].player->actor->rotY = 90.0f;
                lbl_1_bss_114[playerNo].moveDirection = 1;
            }
        } else {
            lbl_1_bss_114[playerNo].player->actor->rotY -= 5.0f;
            if (lbl_1_bss_114[playerNo].player->actor->rotY < -90.0f) {
                lbl_1_bss_114[playerNo].player->actor->rotY = -90.0f;
                lbl_1_bss_114[playerNo].moveDirection = -1;
            }
        }
        if ((u32) (lbl_1_bss_114[playerNo].player->actor->colGroundAttr & 0x407F) != 0) {
            lbl_1_bss_114[playerNo].routeState += 1;
            return;
        }
    } else {
        lbl_1_bss_114[playerNo].liftModelPosition.y += 24.0f;
        Hu3DModelPosSet(lbl_1_bss_114[playerNo].model, lbl_1_bss_114[playerNo].liftModelPosition.x,
                        lbl_1_bss_114[playerNo].liftModelPosition.y,
                        lbl_1_bss_114[playerNo].liftModelPosition.z);
        if (lbl_1_bss_114[playerNo].liftModelPosition.y >
            (800.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].stageSection].y)) {
            Hu3DModelAttrSet(lbl_1_bss_114[playerNo].model, HU3D_ATTR_DISPOFF);
            MgPlayerAttrReset(lbl_1_bss_114[playerNo].player, MGPLAYER_ATTR_COMSTK);
            lbl_1_bss_114[playerNo].fallState = 0;
        }
    }
}

/* Called during play and result callbacks to ease each camera toward its player and keep the view
 * in its course section. */
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
        if (lbl_1_bss_114[player].stageSection == 0) {
            minX = 700.0f;
            maxX = 1800.0f;
            minY = -200.0f;
        } else if (lbl_1_bss_114[player].stageSection == 1) {
            minX = 700.0f;
            maxX = 2600.0f;
            minY = -200.0f;
        } else if (lbl_1_bss_114[player].stageSection == 2) {
            minX = 1100.0f;
            maxX = 1500.0f;
            minY = -10.0f;
        } else if (lbl_1_bss_114[player].stageSection == 3) {
            minX = 800.0f;
            maxX = 2300.0f;
            minY = -370.0f;
        }
        minY += 100.0f;
        minX += lbl_1_data_48[lbl_1_bss_114[player].stageSection].x;
        maxX += lbl_1_data_48[lbl_1_bss_114[player].stageSection].x;
        minY += lbl_1_data_48[lbl_1_bss_114[player].stageSection].y;
        cameraX = lbl_1_bss_4CC[player].center.x;
        actorX = lbl_1_bss_114[player].player->actor->pos.x;
        cameraY = lbl_1_bss_4CC[player].center.y;
        actorY = lbl_1_bss_114[player].player->actor->pos.y;
        if (lbl_1_bss_114[player].moveDirection > 0) {
            /* Clamp the camera target at the section edge; this does not move the actor. */
            if (actorX > maxX) {
                actorX = maxX;
            }
            if (cameraX < (300.0f + actorX)) {
                lbl_1_bss_4CC[player].center.x += ((300.0f + actorX) - cameraX) / 10.0f;
            }
            if ((actorX >= (minX - 300.0f)) && (cameraX > (300.0f + actorX))) {
                lbl_1_bss_4CC[player].center.x -= (cameraX - (300.0f + actorX)) / 10.0f;
            }
            if ((lbl_1_bss_114[player].fallState == 1) && (cameraX > (300.0f + actorX))) {
                lbl_1_bss_4CC[player].center.x -= (cameraX - (300.0f + actorX)) / 10.0f;
            }
        } else if (lbl_1_bss_114[player].moveDirection < 0) {
            if (actorX < minX) {
                actorX = minX;
            }
            if (cameraX > (actorX - 300.0f)) {
                lbl_1_bss_4CC[player].center.x -= (cameraX - (actorX - 300.0f)) / 10.0f;
            }
            if ((actorX <= (300.0f + maxX)) && (cameraX < (actorX - 300.0f))) {
                lbl_1_bss_4CC[player].center.x += ((actorX - 300.0f) - cameraX) / 10.0f;
            }
            if ((lbl_1_bss_114[player].fallState == 1) && (cameraX < (actorX - 300.0f))) {
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
            /* A deep fall is held below the view and behind the course while the return state
             * advances. */
            lbl_1_bss_114[player].player->actor->pos.y = minY - 1000.0f;
            lbl_1_bss_114[player].player->actor->pos.z = 1000.0f;
        }
        player += 1;
    }
}

/* Called by fn_1_378 to load course and section models, position their collision models, and start
 * platform updates. */
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
        lbl_1_bss_30C[i].model = Hu3DModelCreate(
            HuDataSelHeapReadNum(i + DATANUM(DATA_m618, 1), HU_MEMNUM_OVL, HEAP_MODEL));
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
        Hu3DModelAttrSet(lbl_1_bss_30C[i].model, HU3D_MOTATTR_LOOP);
        lbl_1_bss_30C[i].colModel = Hu3DModelCreate(
            HuDataSelHeapReadNum(i + DATANUM(DATA_m618, 38), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_30C[i].colModel, 15U);
        fn_1_6704(i);
        Hu3DModelLayerSet(lbl_1_bss_30C[i].colModel, 1);
        Hu3DModelAttrSet(lbl_1_bss_30C[i].colModel, HU3D_ATTR_DISPOFF);
        collisionModels[collisionCount++] = lbl_1_bss_30C[i].colModel;
        i += 1;
    }
    i = 0;
    while (i < 4) {
        lbl_1_bss_2F4[i][0] = Hu3DModelCreate(HuDataSelHeapReadNum(
            modelFiles[i][0] + DATANUM(DATA_m618, 0), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelPosSet(lbl_1_bss_2F4[i][0], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
        Hu3DModelCameraSet(lbl_1_bss_2F4[i][0], 15U);
        Hu3DModelAttrSet(lbl_1_bss_2F4[i][0], HU3D_MOTATTR_LOOP);
        if ((i == 2) || (i == 3)) {
            lbl_1_bss_2F4[i][1] = Hu3DModelCreate(HuDataSelHeapReadNum(
                modelFiles[i][0] + DATANUM(DATA_m618, 1), HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DModelPosSet(lbl_1_bss_2F4[i][1], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
            Hu3DModelCameraSet(lbl_1_bss_2F4[i][1], 15U);
            Hu3DModelAttrSet(lbl_1_bss_2F4[i][1], HU3D_MOTATTR_LOOP);
        }
        if (i == 3) {
            lbl_1_bss_2F4[i][2] = Hu3DModelCreate(HuDataSelHeapReadNum(
                modelFiles[i][0] + DATANUM(DATA_m618, 2), HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DModelPosSet(lbl_1_bss_2F4[i][2], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
            Hu3DModelCameraSet(lbl_1_bss_2F4[i][2], 15U);
            Hu3DModelAttrSet(lbl_1_bss_2F4[i][2], HU3D_MOTATTR_LOOP);
        }
        if ((i == 1) || (i == 3)) {
            lbl_1_bss_2B4[(i == 1 ? 0 : 1)][0] = Hu3DModelCreate(HuDataSelHeapReadNum(
                (i == 1 ? 9 : 32) + DATANUM(DATA_m618, 0), HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DModelPosSet(lbl_1_bss_2B4[(i == 1 ? 0 : 1)][0], lbl_1_data_48[i].x,
                            (lbl_1_data_48[i].y - 150.0f) - 100.0f, 0.0f);
            Hu3DModelCameraSet(lbl_1_bss_2B4[(i == 1 ? 0 : 1)][0], 15U);
            Hu3DModelAttrSet(lbl_1_bss_2B4[(i == 1 ? 0 : 1)][0], HU3D_MOTATTR_LOOP);
            lbl_1_bss_2B4[(i == 1 ? 0 : 1)][1] = Hu3DModelCreate(HuDataSelHeapReadNum(
                (i == 1 ? 9 : 32) + DATANUM(DATA_m618, 0), HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DModelPosSet(lbl_1_bss_2B4[(i == 1 ? 0 : 1)][1], lbl_1_data_48[i].x,
                            (lbl_1_data_48[i].y - 300.0f) - 100.0f, 0.0f);
            Hu3DModelCameraSet(lbl_1_bss_2B4[(i == 1 ? 0 : 1)][1], 15U);
            Hu3DModelAttrSet(lbl_1_bss_2B4[(i == 1 ? 0 : 1)][1], HU3D_MOTATTR_LOOP);
        }
        lbl_1_bss_2CC[i][0] = Hu3DModelCreate(HuDataSelHeapReadNum(
            modelFiles[i][1] + DATANUM(DATA_m618, 0), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelPosSet(lbl_1_bss_2CC[i][0], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
        Hu3DModelCameraSet(lbl_1_bss_2CC[i][0], 15U);
        Hu3DModelAttrSet(lbl_1_bss_2CC[i][0], HU3D_MOTATTR_LOOP);
        if (i > 0) {
            lbl_1_bss_2CC[i][1] = Hu3DModelCreate(HuDataSelHeapReadNum(
                modelFiles[i][1] + DATANUM(DATA_m618, 1), HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DModelPosSet(lbl_1_bss_2CC[i][1], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
            Hu3DModelCameraSet(lbl_1_bss_2CC[i][1], 15U);
            Hu3DModelAttrSet(lbl_1_bss_2CC[i][1], HU3D_MOTATTR_LOOP);
            lbl_1_bss_2CC[i][2] = Hu3DModelCreate(HuDataSelHeapReadNum(
                modelFiles[i][1] + DATANUM(DATA_m618, 2), HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DModelPosSet(lbl_1_bss_2CC[i][2], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
            Hu3DModelCameraSet(lbl_1_bss_2CC[i][2], 15U);
            Hu3DModelAttrSet(lbl_1_bss_2CC[i][2], HU3D_MOTATTR_LOOP);
            if (i != 3) {
                lbl_1_bss_2CC[i][3] = Hu3DModelCreate(HuDataSelHeapReadNum(
                    modelFiles[i][1] + DATANUM(DATA_m618, 3), HU_MEMNUM_OVL, HEAP_MODEL));
                Hu3DModelPosSet(lbl_1_bss_2CC[i][3], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
                Hu3DModelCameraSet(lbl_1_bss_2CC[i][3], 15U);
                Hu3DModelAttrSet(lbl_1_bss_2CC[i][3], HU3D_MOTATTR_LOOP);
            }
        }
        if (i == 2) {
            lbl_1_bss_2CC[i][4] = Hu3DModelCreate(HuDataSelHeapReadNum(
                modelFiles[i][1] + DATANUM(DATA_m618, 4), HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DModelPosSet(lbl_1_bss_2CC[i][4], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
            Hu3DModelCameraSet(lbl_1_bss_2CC[i][4], 15U);
            Hu3DModelAttrSet(lbl_1_bss_2CC[i][4], HU3D_MOTATTR_LOOP);
        }
        lbl_1_bss_2BC[i][0] = Hu3DModelCreate(HuDataSelHeapReadNum(
            modelFiles[i][2] + DATANUM(DATA_m618, 0), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelPosSet(lbl_1_bss_2BC[i][0], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
        Hu3DModelCameraSet(lbl_1_bss_2BC[i][0], 15U);
        if (i < 3) {
            lbl_1_bss_2BC[i][1] = Hu3DModelCreate(HuDataSelHeapReadNum(
                modelFiles[i][2] + DATANUM(DATA_m618, 1), HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DModelPosSet(lbl_1_bss_2BC[i][1], lbl_1_data_48[i].x, lbl_1_data_48[i].y, 0.0f);
            Hu3DModelCameraSet(lbl_1_bss_2BC[i][1], 15U);
            if (i == 2) {
                extraModel = Hu3DModelCreate(
                    HuDataSelHeapReadNum(DATANUM(DATA_m618, 104), HU_MEMNUM_OVL, HEAP_MODEL));
                Hu3DModelPosGet(lbl_1_bss_2BC[2][1], &pos);
                pos.x -= 15.0f;
                Hu3DModelPosSetV(extraModel, &pos);
                Hu3DModelCameraSet(extraModel, 15U);
                Hu3DModelLayerSet(extraModel, 1);
                Hu3DModelAttrSet(extraModel, HU3D_ATTR_DISPOFF);
                collisionModels[collisionCount++] = extraModel;
            }
        }
        i += 1;
    }
    lbl_1_bss_540[0] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m618, 102), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelPosSet(lbl_1_bss_540[0], lbl_1_data_48[3].x, lbl_1_data_48[3].y, 0.0f);
    Hu3DModelCameraSet(lbl_1_bss_540[0], 15U);
    Hu3DModelAttrSet(lbl_1_bss_540[0], HU3D_ATTR_DISPOFF);
    Hu3DMotionSpeedSet(lbl_1_bss_540[0], 0.0f);
    lbl_1_bss_540[1] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m618, 103), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelPosSet(lbl_1_bss_540[1], lbl_1_data_48[3].x, lbl_1_data_48[3].y, 0.0f);
    Hu3DModelCameraSet(lbl_1_bss_540[1], 15U);
    Hu3DModelAttrSet(lbl_1_bss_540[1], HU3D_ATTR_DISPOFF);
    Hu3DMotionSpeedSet(lbl_1_bss_540[1], 0.0f);
    MgActorColMapInit(collisionModels, (s16) collisionCount, 8);
    Hu3DBGColorSet(0U, 0U, 0U);
    HuPrcChildCreate(fn_1_6C0C, 4096U, 8192U, 0, HuPrcCurrentGet());
}

/* Called by fn_1_910 to stop course and lift motion during the results. */
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

/* Synchronizes a course object's collision model with its animated platform position and
 * rotation. */
void fn_1_6704(int modelIndex)
{
    Point3d modelPosition;
    Point3d modelRotation;

    Hu3DMotionCalc(lbl_1_bss_30C[modelIndex].model);
    switch (modelIndex) {
    case 0:
    case 1:
    case 2:
        Hu3DModelPosGet(lbl_1_bss_30C[modelIndex].model, &modelPosition);
        break;
    case 3:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "1st_lift01", &modelPosition);
        break;
    case 4:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "1st_lift02", &modelPosition);
        break;
    case 5:
    case 6:
    case 7:
    case 8:
        Hu3DModelPosGet(lbl_1_bss_30C[modelIndex].model, &modelPosition);
        break;
    case 9:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "2st_lift01", &modelPosition);
        break;
    case 10:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "2st_lift02", &modelPosition);
        break;
    case 11:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "2st_lift03", &modelPosition);
        break;
    case 12:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "2st_lift04", &modelPosition);
        break;
    case 13:
    case 14:
        Hu3DModelPosGet(lbl_1_bss_30C[modelIndex].model, &modelPosition);
        break;
    case 15:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "3st_lift_a00", &modelPosition);
        break;
    case 16:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "3st_lift_a01", &modelPosition);
        break;
    case 17:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "3st_lift_a02", &modelPosition);
        break;
    case 18:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "3st_lift_b00", &modelPosition);
        break;
    case 19:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "3st_lift_b01", &modelPosition);
        break;
    case 20:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "3st_lift_b02", &modelPosition);
        break;
    case 21:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "3st_lift_b03", &modelPosition);
        break;
    case 22:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "3st_lift_c00", &modelPosition);
        break;
    case 23:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "3st_lift_c01", &modelPosition);
        break;
    case 24:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "3st_lift_c02", &modelPosition);
        break;
    case 25:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "3st_lift_c03", &modelPosition);
        break;
    case 26:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "3st_lift_d00", &modelPosition);
        break;
    case 27:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "3st_lift_d01", &modelPosition);
        break;
    case 28:
    case 29:
    case 30:
    case 31:
        Hu3DModelPosGet(lbl_1_bss_30C[modelIndex].model, &modelPosition);
        break;
    case 32:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "4st_lift00", &modelPosition);
        break;
    case 33:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "4st_lift01", &modelPosition);
        break;
    case 34:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "4st_lift02", &modelPosition);
        break;
    case 35:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "4st_lift03", &modelPosition);
        break;
    case 36:
        Hu3DModelObjPosGet(lbl_1_bss_30C[modelIndex].model, "4st_lift04", &modelPosition);
        break;
    }
    Hu3DModelPosSetV(lbl_1_bss_30C[modelIndex].colModel, &modelPosition);
    lbl_1_bss_30C[modelIndex].collisionHeight = modelPosition.y;
    Hu3DModelRotGet(lbl_1_bss_30C[modelIndex].model, &modelRotation);
    Hu3DModelRotSetV(lbl_1_bss_30C[modelIndex].colModel, &modelRotation);
}

/* Child process created by fn_1_52D0; updates course visibility and synchronizes lift collision
 * models each frame. */
void fn_1_6C0C(void)
{
    s32 modelIndex;

    modelIndex = 0;
    while (modelIndex < 37) {
        if ((modelIndex >= 0) && (modelIndex <= 4)) {
            if ((lbl_1_bss_114->stageSection != 0) && (lbl_1_bss_114[1].stageSection != 0) &&
                (lbl_1_bss_114[2].stageSection != 0) && (lbl_1_bss_114[3].stageSection != 0)) {
                if (((lbl_1_bss_114->stageSection != 1) || (lbl_1_bss_114->fallState != 3)) &&
                    ((lbl_1_bss_114[1].stageSection != 1) || (lbl_1_bss_114[1].fallState != 3)) &&
                    ((lbl_1_bss_114[2].stageSection != 1) || (lbl_1_bss_114[2].fallState != 3)) &&
                    ((lbl_1_bss_114[3].stageSection != 1) || (lbl_1_bss_114[3].fallState != 3))) {
                    Hu3DModelAttrSet(lbl_1_bss_30C[modelIndex].model, HU3D_ATTR_DISPOFF);
                }
            } else {
                Hu3DModelAttrReset(lbl_1_bss_30C[modelIndex].model, HU3D_ATTR_DISPOFF);
            }
        } else if ((modelIndex >= 5) && (modelIndex <= 12)) {
            if ((lbl_1_bss_114->stageSection != 1) && (lbl_1_bss_114[1].stageSection != 1) &&
                (lbl_1_bss_114[2].stageSection != 1) && (lbl_1_bss_114[3].stageSection != 1)) {
                if (((lbl_1_bss_114->stageSection != 2) || (lbl_1_bss_114->fallState != 3)) &&
                    ((lbl_1_bss_114[1].stageSection != 2) || (lbl_1_bss_114[1].fallState != 3)) &&
                    ((lbl_1_bss_114[2].stageSection != 2) || (lbl_1_bss_114[2].fallState != 3)) &&
                    ((lbl_1_bss_114[3].stageSection != 2) || (lbl_1_bss_114[3].fallState != 3))) {
                    Hu3DModelAttrSet(lbl_1_bss_30C[modelIndex].model, HU3D_ATTR_DISPOFF);
                }
            } else {
                Hu3DModelAttrReset(lbl_1_bss_30C[modelIndex].model, HU3D_ATTR_DISPOFF);
            }
        } else if ((modelIndex >= 13) && (modelIndex <= 27)) {
            if ((lbl_1_bss_114->stageSection != 2) && (lbl_1_bss_114[1].stageSection != 2) &&
                (lbl_1_bss_114[2].stageSection != 2) && (lbl_1_bss_114[3].stageSection != 2)) {
                if (((lbl_1_bss_114->stageSection != 3) || (lbl_1_bss_114->fallState != 3)) &&
                    ((lbl_1_bss_114[1].stageSection != 3) || (lbl_1_bss_114[1].fallState != 3)) &&
                    ((lbl_1_bss_114[2].stageSection != 3) || (lbl_1_bss_114[2].fallState != 3)) &&
                    ((lbl_1_bss_114[3].stageSection != 3) || (lbl_1_bss_114[3].fallState != 3))) {
                    Hu3DModelAttrSet(lbl_1_bss_30C[modelIndex].model, HU3D_ATTR_DISPOFF);
                }
            } else {
                Hu3DModelAttrReset(lbl_1_bss_30C[modelIndex].model, HU3D_ATTR_DISPOFF);
            }
        } else if ((modelIndex >= 28) && (modelIndex <= 36)) {
            if ((lbl_1_bss_114->stageSection != 3) && (lbl_1_bss_114[1].stageSection != 3) &&
                (lbl_1_bss_114[2].stageSection != 3) && (lbl_1_bss_114[3].stageSection != 3)) {
                Hu3DModelAttrSet(lbl_1_bss_30C[modelIndex].model, HU3D_ATTR_DISPOFF);
            } else {
                Hu3DModelAttrReset(lbl_1_bss_30C[modelIndex].model, HU3D_ATTR_DISPOFF);
            }
        }
        modelIndex += 1;
    }
    if ((lbl_1_bss_114->stageSection != 0) && (lbl_1_bss_114[1].stageSection != 0) &&
        (lbl_1_bss_114[2].stageSection != 0) && (lbl_1_bss_114[3].stageSection != 0)) {
        if (((lbl_1_bss_114->stageSection != 1) || (lbl_1_bss_114->fallState != 3)) &&
            ((lbl_1_bss_114[1].stageSection != 1) || (lbl_1_bss_114[1].fallState != 3)) &&
            ((lbl_1_bss_114[2].stageSection != 1) || (lbl_1_bss_114[2].fallState != 3)) &&
            ((lbl_1_bss_114[3].stageSection != 1) || (lbl_1_bss_114[3].fallState != 3))) {
            Hu3DModelAttrSet(lbl_1_bss_2F4[0][0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_2CC[0][0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_2BC[0][0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_2BC[0][1], HU3D_ATTR_DISPOFF);
        }
    } else {
        Hu3DModelAttrReset(lbl_1_bss_2F4[0][0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2CC[0][0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2BC[0][0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2BC[0][1], HU3D_ATTR_DISPOFF);
        fn_1_6704(3);
        fn_1_6704(4);
    }
    if ((lbl_1_bss_114->stageSection != 1) && (lbl_1_bss_114[1].stageSection != 1) &&
        (lbl_1_bss_114[2].stageSection != 1) && (lbl_1_bss_114[3].stageSection != 1)) {
        if (((lbl_1_bss_114->stageSection != 2) || (lbl_1_bss_114->fallState != 3)) &&
            ((lbl_1_bss_114[1].stageSection != 2) || (lbl_1_bss_114[1].fallState != 3)) &&
            ((lbl_1_bss_114[2].stageSection != 2) || (lbl_1_bss_114[2].fallState != 3)) &&
            ((lbl_1_bss_114[3].stageSection != 2) || (lbl_1_bss_114[3].fallState != 3))) {
            Hu3DModelAttrSet(lbl_1_bss_2F4[1][0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_2CC[1][0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_2CC[1][1], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_2CC[1][2], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_2CC[1][3], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_2BC[1][0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_2BC[1][1], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_2B4[0][0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_2B4[0][1], HU3D_ATTR_DISPOFF);
        }
    } else {
        Hu3DModelAttrReset(lbl_1_bss_2F4[1][0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2CC[1][0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2CC[1][1], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2CC[1][2], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2CC[1][3], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2BC[1][0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2BC[1][1], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2B4[0][0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2B4[0][1], HU3D_ATTR_DISPOFF);
        fn_1_6704(9);
        fn_1_6704(10);
        fn_1_6704(11);
        fn_1_6704(12);
    }
    if ((lbl_1_bss_114->stageSection != 2) && (lbl_1_bss_114[1].stageSection != 2) &&
        (lbl_1_bss_114[2].stageSection != 2) && (lbl_1_bss_114[3].stageSection != 2)) {
        if (((lbl_1_bss_114->stageSection != 3) || (lbl_1_bss_114->fallState != 3)) &&
            ((lbl_1_bss_114[1].stageSection != 3) || (lbl_1_bss_114[1].fallState != 3)) &&
            ((lbl_1_bss_114[2].stageSection != 3) || (lbl_1_bss_114[2].fallState != 3)) &&
            ((lbl_1_bss_114[3].stageSection != 3) || (lbl_1_bss_114[3].fallState != 3))) {
            Hu3DModelAttrSet(lbl_1_bss_2F4[2][0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_2F4[2][1], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_2CC[2][0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_2CC[2][1], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_2CC[2][2], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_2CC[2][3], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_2CC[2][4], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_2BC[2][0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_2BC[2][1], HU3D_ATTR_DISPOFF);
        }
    } else {
        Hu3DModelAttrReset(lbl_1_bss_2F4[2][0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2F4[2][1], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2CC[2][0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2CC[2][1], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2CC[2][2], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2CC[2][3], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2CC[2][4], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2BC[2][0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2BC[2][1], HU3D_ATTR_DISPOFF);
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
    if ((lbl_1_bss_114->stageSection != 3) && (lbl_1_bss_114[1].stageSection != 3) &&
        (lbl_1_bss_114[2].stageSection != 3) && (lbl_1_bss_114[3].stageSection != 3)) {
        Hu3DModelAttrSet(lbl_1_bss_2F4[3][0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_2F4[3][1], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_2F4[3][2], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_2CC[3][0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_2CC[3][1], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_2CC[3][2], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_2BC[3][0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_2B4[1][0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_2B4[1][1], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_2F4[3][0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2F4[3][1], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2F4[3][2], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2CC[3][0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2CC[3][1], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2CC[3][2], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2BC[3][0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2B4[1][0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2B4[1][1], HU3D_ATTR_DISPOFF);
        fn_1_6704(32);
        fn_1_6704(33);
        fn_1_6704(34);
        fn_1_6704(35);
        fn_1_6704(36);
    }
    HuPrcVSleep();
}

/* Called by setup and winner callbacks to animate the four split-screen viewports around the
 * selected winner. */
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
            Hu3DCameraScissorSet(lbl_1_data_28[0], insetX, insetY, (u32) (splitX - insetWidth),
                                 (u32) (splitY - insetHeight));
        } else if ((s32) lbl_1_data_28[camera] == 2) {
            viewportX[0] = splitX - 16.0f;
            viewportY[0] = 0.0f;
            viewportX[1] = 16.0f + (640.0f - splitX);
            viewportY[1] = 40.0f + splitY;
            Hu3DCameraScissorSet(lbl_1_data_28[1], (u32) (splitX + insetX), insetY,
                                 (u32) ((640.0f - splitX) - insetWidth),
                                 (u32) (splitY - insetHeight));
        } else if ((s32) lbl_1_data_28[camera] == 4) {
            viewportX[0] = 0.0f;
            viewportY[0] = splitY - 40.0f;
            viewportX[1] = 16.0f + splitX;
            viewportY[1] = 40.0f + (480.0f - splitY);
            Hu3DCameraScissorSet(lbl_1_data_28[2], insetX, (u32) (splitY + insetY),
                                 (u32) (splitX - insetWidth),
                                 (u32) ((480.0f - splitY) - insetHeight));
        } else if ((s32) lbl_1_data_28[camera] == 8) {
            viewportX[0] = splitX - 16.0f;
            viewportY[0] = splitY - 40.0f;
            viewportX[1] = 16.0f + (640.0f - splitX);
            viewportY[1] = 40.0f + (480.0f - splitY);
            Hu3DCameraScissorSet(lbl_1_data_28[3], (u32) (splitX + insetX), (u32) (splitY + insetY),
                                 (u32) ((640.0f - splitX) - insetWidth),
                                 (u32) ((480.0f - splitY) - insetHeight));
        }
        Hu3DCameraPerspectiveSet(lbl_1_data_28[camera], 10.0f, 20.0f, 30000.0f, 1.2f);
        Hu3DCameraViewportSet(lbl_1_data_28[camera], viewportX[0], viewportY[0], viewportX[1],
                              viewportY[1], 0.0f, 1.0f);
        camera += 1;
    }
    if (++lbl_1_bss_4C8 > 60.0f) {
        lbl_1_bss_4C8 = 60.0f;
    }
}

/* Called by fn_1_214C each frame for CPU players to choose a lift route and supply controller
 * inputs. */
void fn_1_8180(int playerNo)
{
    Point3d delta;
    Point3d playerPos;
    Point3d targetPos;
    Point3d savedPos;
    Point3d liftPos;
    s32 unusedInitialization;
    float xBoundary;
    float distance;

    /* The original update initializes this value but never reads it; route choices use player and
     * lift state. */
    unusedInitialization = 0;
    playerPos = lbl_1_bss_114[playerNo].player->actor->pos;
    if ((u32) (lbl_1_bss_114[playerNo].player->actor->colGroundAttr & 0x407F) != 0) {
        fn_1_C450(&lbl_1_bss_84[playerNo], 0, 0, 0, 0);
        lbl_1_bss_114[playerNo].aiRouteChoice = 0;
        switch (lbl_1_bss_114[playerNo].player->actor->colMesh) {
        case 0:
            xBoundary = 350.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].stageSection].x;
            if (playerPos.x < xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
                Hu3DModelObjPosGet(lbl_1_bss_30C[3].model, "1st_lift01",
                                   &lbl_1_bss_114[playerNo].nearbyLiftPosition);
                lbl_1_bss_84[playerNo].decisionPending = 0;
                lbl_1_bss_84[playerNo].decisionIndex = 0;
                lbl_1_bss_84[playerNo].decisionCount =
                    (lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                        ? frandmod(4)
                        : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                               ? frandmod(3)
                               : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2) ? frandmod(2)
                                                                              : frandmod(2)));
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[3].model, "1st_lift01", &targetPos);
                targetPos.y = targetPos.z = 0.0f;
                playerPos.y = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                if ((distance < 500.0f) &&
                    (targetPos.x < lbl_1_bss_114[playerNo].nearbyLiftPosition.x)) {
                    if (lbl_1_bss_84[playerNo].decisionCount != 0) {
                        lbl_1_bss_84[playerNo].decisionPending = 1;
                    }
                    if (lbl_1_bss_84[playerNo].decisionIndex ==
                        lbl_1_bss_84[playerNo].decisionCount) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, PAD_BUTTON_A, 0);
                        lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                        lbl_1_bss_114[playerNo].aiWaitFrames = 18;
                        lbl_1_bss_114[playerNo].aiWaitFrames =
                            (lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                                ? ((frandmod(100) < 50U)
                                       ? (lbl_1_bss_114[playerNo].aiWaitFrames + 5)
                                       : lbl_1_bss_114[playerNo].aiWaitFrames)
                                : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                                       ? ((frandmod(100) < 35U)
                                              ? (lbl_1_bss_114[playerNo].aiWaitFrames + 5)
                                              : lbl_1_bss_114[playerNo].aiWaitFrames)
                                       : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2)
                                              ? ((frandmod(100) < 20U)
                                                     ? (lbl_1_bss_114[playerNo].aiWaitFrames + 5)
                                                     : lbl_1_bss_114[playerNo].aiWaitFrames)
                                              : ((frandmod(100) < 5U)
                                                     ? (lbl_1_bss_114[playerNo].aiWaitFrames + 5)
                                                     : lbl_1_bss_114[playerNo].aiWaitFrames)));
                    }
                } else if (lbl_1_bss_84[playerNo].decisionPending != 0) {
                    lbl_1_bss_84[playerNo].decisionPending = 0;
                    lbl_1_bss_84[playerNo].decisionIndex += 1;
                }
                lbl_1_bss_114[playerNo].nearbyLiftPosition = targetPos;
            }
            break;
        case 3:
            if (--lbl_1_bss_114[playerNo].aiWaitFrames > 0) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[1].model, "1st_middle", &targetPos);
                targetPos.y = targetPos.z = 0.0f;
                playerPos.y = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 450.0f) {
                    if ((s32) ((lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                                   ? ((frandmod(100) < 50U) ? 1 : 0)
                                   : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                                          ? ((frandmod(100) < 35U) ? 1 : 0)
                                          : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2)
                                                 ? ((frandmod(100) < 20U) ? 1 : 0)
                                                 : ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 14, 0, PAD_BUTTON_A, 0);
                    } else {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, PAD_BUTTON_A, 0);
                    }
                    lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                }
            }
            break;
        case 1:
            Hu3DModelObjPosGet(lbl_1_bss_30C[1].model, "1st_middle", &targetPos);
            xBoundary = 100.0f + targetPos.x;
            if (playerPos.x < xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
                lbl_1_bss_84[playerNo].decisionPending = 0;
                lbl_1_bss_84[playerNo].decisionIndex = 0;
                lbl_1_bss_84[playerNo].decisionCount =
                    (lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                        ? frandmod(4)
                        : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                               ? frandmod(3)
                               : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2) ? frandmod(2)
                                                                              : frandmod(2)));
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[4].model, "1st_lift02", &targetPos);
                if ((playerPos.y < (100.0f + targetPos.y)) &&
                    (playerPos.y > (targetPos.y - 100.0f))) {
                    if (lbl_1_bss_84[playerNo].decisionCount != 0) {
                        lbl_1_bss_84[playerNo].decisionPending = 1;
                    }
                    if (lbl_1_bss_84[playerNo].decisionIndex ==
                        lbl_1_bss_84[playerNo].decisionCount) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, PAD_BUTTON_A, 0);
                        lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                        lbl_1_bss_114[playerNo].aiWaitFrames = 10;
                        lbl_1_bss_114[playerNo].aiWaitFrames =
                            (lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                                ? ((frandmod(100) < 50U)
                                       ? (lbl_1_bss_114[playerNo].aiWaitFrames + 5)
                                       : lbl_1_bss_114[playerNo].aiWaitFrames)
                                : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                                       ? ((frandmod(100) < 35U)
                                              ? (lbl_1_bss_114[playerNo].aiWaitFrames + 5)
                                              : lbl_1_bss_114[playerNo].aiWaitFrames)
                                       : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2)
                                              ? ((frandmod(100) < 20U)
                                                     ? (lbl_1_bss_114[playerNo].aiWaitFrames + 5)
                                                     : lbl_1_bss_114[playerNo].aiWaitFrames)
                                              : ((frandmod(100) < 5U)
                                                     ? (lbl_1_bss_114[playerNo].aiWaitFrames + 5)
                                                     : lbl_1_bss_114[playerNo].aiWaitFrames)));
                    }
                } else if (lbl_1_bss_84[playerNo].decisionPending != 0) {
                    lbl_1_bss_84[playerNo].decisionPending = 0;
                    lbl_1_bss_84[playerNo].decisionIndex += 1;
                }
            }
            break;
        case 4:
            if (--lbl_1_bss_114[playerNo].aiWaitFrames > 0) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
            } else {
                if ((s32) ((lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                               ? ((frandmod(100) < 50U) ? 1 : 0)
                               : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                                      ? ((frandmod(100) < 35U) ? 1 : 0)
                                      : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2)
                                             ? ((frandmod(100) < 20U) ? 1 : 0)
                                             : ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], 14, 0, PAD_BUTTON_A, 0);
                } else {
                    fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, PAD_BUTTON_A, 0);
                }
                lbl_1_bss_114[playerNo].aiRouteChoice = 1;
            }
            break;
        case 2:
            fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
            break;
        case 5:
            xBoundary = 350.0f + lbl_1_data_48[lbl_1_bss_114[playerNo].stageSection].x;
            if (playerPos.x < xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
                lbl_1_bss_84[playerNo].decisionPending = 0;
                lbl_1_bss_84[playerNo].decisionIndex = 0;
                lbl_1_bss_84[playerNo].decisionCount =
                    (lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                        ? frandmod(4)
                        : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                               ? frandmod(3)
                               : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2) ? frandmod(2)
                                                                              : frandmod(2)));
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[9].model, "2st_lift01", &targetPos);
                if ((playerPos.y < (100.0f + targetPos.y)) &&
                    (playerPos.y > (targetPos.y - 100.0f))) {
                    if (lbl_1_bss_84[playerNo].decisionCount != 0) {
                        lbl_1_bss_84[playerNo].decisionPending = 1;
                    }
                    if (lbl_1_bss_84[playerNo].decisionIndex ==
                        lbl_1_bss_84[playerNo].decisionCount) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, PAD_BUTTON_A, 0);
                        lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                    }
                } else if (lbl_1_bss_84[playerNo].decisionPending != 0) {
                    lbl_1_bss_84[playerNo].decisionPending = 0;
                    lbl_1_bss_84[playerNo].decisionIndex += 1;
                }
            }
            break;
        case 9:
            Hu3DModelObjPosGet(lbl_1_bss_30C[9].model, "2st_lift01", &targetPos);
            xBoundary = 80.0f + targetPos.x;
            if (playerPos.x < xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
                lbl_1_bss_84[playerNo].decisionPending = 0;
                lbl_1_bss_84[playerNo].decisionIndex = 0;
                lbl_1_bss_84[playerNo].decisionCount =
                    (lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                        ? frandmod(4)
                        : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                               ? frandmod(3)
                               : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2) ? frandmod(2)
                                                                              : frandmod(2)));
            } else if (playerPos.y > 270.0f) {
                if (lbl_1_bss_84[playerNo].decisionCount != 0) {
                    lbl_1_bss_84[playerNo].decisionPending = 1;
                }
                if (lbl_1_bss_84[playerNo].decisionIndex == lbl_1_bss_84[playerNo].decisionCount) {
                    if ((s32) ((lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                                   ? ((frandmod(100) < 50U) ? 1 : 0)
                                   : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                                          ? ((frandmod(100) < 35U) ? 1 : 0)
                                          : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2)
                                                 ? ((frandmod(100) < 20U) ? 1 : 0)
                                                 : ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 28, 0, PAD_BUTTON_A, 0);
                    } else {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, PAD_BUTTON_A, 0);
                    }
                    lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                }
            } else if (lbl_1_bss_84[playerNo].decisionPending != 0) {
                lbl_1_bss_84[playerNo].decisionPending = 0;
                lbl_1_bss_84[playerNo].decisionIndex += 1;
            }
            break;
        case 10:
            Hu3DModelObjPosGet(lbl_1_bss_30C[10].model, "2st_lift02", &targetPos);
            xBoundary = 80.0f + targetPos.x;
            if (playerPos.x < xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
                lbl_1_bss_84[playerNo].decisionPending = 0;
                lbl_1_bss_84[playerNo].decisionIndex = 0;
                lbl_1_bss_84[playerNo].decisionCount =
                    (lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                        ? frandmod(4)
                        : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                               ? frandmod(3)
                               : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2) ? frandmod(2)
                                                                              : frandmod(2)));
            } else if (playerPos.y > -50.0f) {
                if (lbl_1_bss_84[playerNo].decisionCount != 0) {
                    lbl_1_bss_84[playerNo].decisionPending = 1;
                }
                if (lbl_1_bss_84[playerNo].decisionIndex == lbl_1_bss_84[playerNo].decisionCount) {
                    if ((s32) ((lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                                   ? ((frandmod(100) < 50U) ? 1 : 0)
                                   : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                                          ? ((frandmod(100) < 35U) ? 1 : 0)
                                          : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2)
                                                 ? ((frandmod(100) < 20U) ? 1 : 0)
                                                 : ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 28, 0, PAD_BUTTON_A, 0);
                    } else {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, PAD_BUTTON_A, 0);
                    }
                    lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                }
            } else if (lbl_1_bss_84[playerNo].decisionPending != 0) {
                lbl_1_bss_84[playerNo].decisionPending = 0;
                lbl_1_bss_84[playerNo].decisionIndex += 1;
            }
            break;
        case 6:
            Hu3DModelObjPosGet(lbl_1_bss_30C[6].model, "2st_middle", &targetPos);
            xBoundary = 80.0f + targetPos.x;
            if (playerPos.x < xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[11].model, "2st_lift03", &targetPos);
                savedPos = targetPos;
                targetPos.y = targetPos.z = 0.0f;
                playerPos.y = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                Hu3DModelObjPosGet(lbl_1_bss_30C[12].model, "2st_lift04", &targetPos);
                if ((distance < 380.0f) && (savedPos.y < targetPos.y)) {
                    if ((s32) ((lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                                   ? ((frandmod(100) < 50U) ? 1 : 0)
                                   : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                                          ? ((frandmod(100) < 35U) ? 1 : 0)
                                          : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2)
                                                 ? ((frandmod(100) < 20U) ? 1 : 0)
                                                 : ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 28, 0, PAD_BUTTON_A, 0);
                    } else {
                        fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, PAD_BUTTON_A, 0);
                    }
                    lbl_1_bss_114[playerNo].aiRouteChoice = 1;
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
                        if ((s32) ((lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                                       ? ((frandmod(100) < 50U) ? 1 : 0)
                                       : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                                              ? ((frandmod(100) < 35U) ? 1 : 0)
                                              : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2)
                                                     ? ((frandmod(100) < 20U) ? 1 : 0)
                                                     : ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                            fn_1_C450(&lbl_1_bss_84[playerNo], 28, 0, PAD_BUTTON_A, 0);
                        } else {
                            fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, PAD_BUTTON_A, 0);
                        }
                        lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                        targetPos = savedPos;
                    }
                }
            }
            break;
        case 11:
        case 12:
            if ((s32) ((lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                           ? ((frandmod(100) < 50U) ? 1 : 0)
                           : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                                  ? ((frandmod(100) < 35U) ? 1 : 0)
                                  : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2)
                                         ? ((frandmod(100) < 20U) ? 1 : 0)
                                         : ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                fn_1_C450(&lbl_1_bss_84[playerNo], 28, 0, PAD_BUTTON_A, 0);
            } else {
                fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, PAD_BUTTON_A, 0);
            }
            lbl_1_bss_114[playerNo].aiRouteChoice = 1;
            break;
        case 7:
            fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
            break;
        case 37:
            xBoundary = lbl_1_bss_114[playerNo].recoveryPosition.x - 120.0f;
            if (playerPos.x > xBoundary) {
                if (playerPos.x <= xBoundary) {
                    playerPos.x = xBoundary;
                }
                fn_1_C450(&lbl_1_bss_84[playerNo], -lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
                lbl_1_bss_84[playerNo].decisionPending = 0;
                lbl_1_bss_84[playerNo].decisionIndex = 0;
                lbl_1_bss_84[playerNo].decisionCount =
                    (lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                        ? frandmod(4)
                        : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                               ? frandmod(3)
                               : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2) ? frandmod(2)
                                                                              : frandmod(2)));
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[22].model, "3st_lift_c00", &targetPos);
                if (targetPos.y < 1850.0f) {
                    if (lbl_1_bss_84[playerNo].decisionCount != 0) {
                        lbl_1_bss_84[playerNo].decisionPending = 1;
                    }
                    if (lbl_1_bss_84[playerNo].decisionIndex ==
                        lbl_1_bss_84[playerNo].decisionCount) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, PAD_BUTTON_A, 0);
                        lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                    }
                } else if (lbl_1_bss_84[playerNo].decisionPending != 0) {
                    lbl_1_bss_84[playerNo].decisionPending = 0;
                    lbl_1_bss_84[playerNo].decisionIndex += 1;
                }
            }
            break;
        case 22:
            Hu3DModelObjPosGet(lbl_1_bss_30C[22].model, "3st_lift_c00", &targetPos);
            xBoundary = targetPos.x - 100.0f;
            if (playerPos.x > xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], -lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[19].model, "3st_lift_b01", &targetPos);
                targetPos.x = targetPos.z = 0.0f;
                playerPos.x = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 150.0f) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, PAD_BUTTON_A, 0);
                    lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                }
            }
            break;
        case 19:
            Hu3DModelObjPosGet(lbl_1_bss_30C[19].model, "3st_lift_b01", &targetPos);
            xBoundary = 100.0f + targetPos.x;
            if (playerPos.x < xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[23].model, "3st_lift_c01", &targetPos);
                targetPos.x = targetPos.z = 0.0f;
                playerPos.x = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 150.0f) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, PAD_BUTTON_A, 0);
                    lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                }
            }
            break;
        case 23:
            Hu3DModelObjPosGet(lbl_1_bss_30C[23].model, "3st_lift_c01", &targetPos);
            xBoundary = targetPos.x - 100.0f;
            if (playerPos.x > xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], -lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[20].model, "3st_lift_b02", &targetPos);
                targetPos.x = targetPos.z = 0.0f;
                playerPos.x = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 150.0f) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, PAD_BUTTON_A, 0);
                    lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                }
            }
            break;
        case 20:
            Hu3DModelObjPosGet(lbl_1_bss_30C[20].model, "3st_lift_b02", &targetPos);
            xBoundary = 100.0f + targetPos.x;
            if (playerPos.x < xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[24].model, "3st_lift_c02", &targetPos);
                targetPos.x = targetPos.z = 0.0f;
                playerPos.x = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 150.0f) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], 56, 0, PAD_BUTTON_A, 0);
                    lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                }
            }
            break;
        case 24:
            Hu3DModelObjPosGet(lbl_1_bss_30C[24].model, "3st_lift_c02", &targetPos);
            xBoundary = targetPos.x - 100.0f;
            if (playerPos.x > xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], -lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[21].model, "3st_lift_b03", &targetPos);
                targetPos.x = targetPos.z = 0.0f;
                playerPos.x = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 150.0f) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, PAD_BUTTON_A, 0);
                    lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                }
            }
            break;
        case 21:
            Hu3DModelObjPosGet(lbl_1_bss_30C[21].model, "3st_lift_b03", &targetPos);
            xBoundary = targetPos.x - 100.0f;
            if (playerPos.x > xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], -lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
                lbl_1_bss_84[playerNo].decisionPending = 0;
                lbl_1_bss_84[playerNo].decisionIndex = 0;
                lbl_1_bss_84[playerNo].decisionCount =
                    (lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                        ? frandmod(4)
                        : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                               ? frandmod(3)
                               : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2) ? frandmod(2)
                                                                              : frandmod(2)));
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[14].model, "3st_goal", &targetPos);
                targetPos.x = targetPos.z = 0.0f;
                playerPos.x = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 120.0f) {
                    if (lbl_1_bss_84[playerNo].decisionCount != 0) {
                        lbl_1_bss_84[playerNo].decisionPending = 1;
                    }
                    if (lbl_1_bss_84[playerNo].decisionIndex ==
                        lbl_1_bss_84[playerNo].decisionCount) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, PAD_BUTTON_A, 0);
                        lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                    }
                } else if (lbl_1_bss_84[playerNo].decisionPending != 0) {
                    lbl_1_bss_84[playerNo].decisionPending = 0;
                    lbl_1_bss_84[playerNo].decisionIndex += 1;
                }
            }
            break;
        case 14:
            fn_1_C450(&lbl_1_bss_84[playerNo], -lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
            break;
        case 28:
            Hu3DModelObjPosGet(lbl_1_bss_30C[35].model, "4st_lift03", &targetPos);
            savedPos = targetPos;
            targetPos.y = targetPos.z = 0.0f;
            playerPos.y = playerPos.z = 0.0f;
            PSVECSubtract(&targetPos, &playerPos, &delta);
            distance = PSVECMag(&delta);
            if (distance < 348.0f) {
                fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, PAD_BUTTON_A, 0);
                lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                targetPos = savedPos;
                lbl_1_bss_114[playerNo].aiWaitFrames = 12;
            } else {
                Hu3DModelObjPosGet(lbl_1_bss_30C[36].model, "4st_lift04", &targetPos);
                savedPos = targetPos;
                targetPos.y = targetPos.z = 0.0f;
                playerPos.y = playerPos.z = 0.0f;
                PSVECSubtract(&targetPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 348.0f) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, PAD_BUTTON_A, 0);
                    lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                    targetPos = savedPos;
                    lbl_1_bss_114[playerNo].aiWaitFrames = 12;
                }
            }
            break;
        case 35:
        case 36:
            if (--lbl_1_bss_114[playerNo].aiWaitFrames > 0) {
                fn_1_C450(&lbl_1_bss_84[playerNo], -lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
            } else if ((playerPos.x < 3100.0f) && (playerPos.y < 3700.0f)) {
                if ((s32) ((lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                               ? ((frandmod(100) < 50U) ? 1 : 0)
                               : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                                      ? ((frandmod(100) < 35U) ? 1 : 0)
                                      : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2)
                                             ? ((frandmod(100) < 20U) ? 1 : 0)
                                             : ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -28, 0, PAD_BUTTON_A, 0);
                } else {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, PAD_BUTTON_A, 0);
                }
                lbl_1_bss_114[playerNo].aiRouteChoice = 1;
            }
            break;
        case 29:
            Hu3DModelObjPosGet(lbl_1_bss_30C[29].model, "4st_middle", &targetPos);
            xBoundary = targetPos.x - 80.0f;
            if (playerPos.x > xBoundary) {
                fn_1_C450(&lbl_1_bss_84[playerNo], -lbl_1_bss_84[playerNo].aiSpeed, 0, 0, 0);
            } else {
                fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, PAD_BUTTON_A, 0);
                lbl_1_bss_114[playerNo].aiRouteChoice = 2;
            }
            break;
        case 32:
            Hu3DModelObjPosGet(lbl_1_bss_30C[32].model, "4st_lift00", &savedPos);
            Hu3DModelObjPosGet(lbl_1_bss_30C[33].model, "4st_lift01", &targetPos);
            if (savedPos.y >= targetPos.y) {
                if ((s32) ((lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                               ? ((frandmod(100) < 50U) ? 1 : 0)
                               : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                                      ? ((frandmod(100) < 35U) ? 1 : 0)
                                      : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2)
                                             ? ((frandmod(100) < 20U) ? 1 : 0)
                                             : ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -14, 0, PAD_BUTTON_A, 0);
                } else {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, PAD_BUTTON_A, 0);
                }
                lbl_1_bss_114[playerNo].aiRouteChoice = 3;
            } else {
                targetPos.x = targetPos.z = 0.0f;
                savedPos.x = savedPos.z = 0.0f;
                PSVECSubtract(&targetPos, &savedPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 160.0f) {
                    if ((s32) ((lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                                   ? ((frandmod(100) < 50U) ? 1 : 0)
                                   : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                                          ? ((frandmod(100) < 35U) ? 1 : 0)
                                          : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2)
                                                 ? ((frandmod(100) < 20U) ? 1 : 0)
                                                 : ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], -14, 0, PAD_BUTTON_A, 0);
                    } else {
                        fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, PAD_BUTTON_A, 0);
                    }
                    lbl_1_bss_114[playerNo].aiRouteChoice = 3;
                }
            }
            break;
        case 33:
            Hu3DModelObjPosGet(lbl_1_bss_30C[33].model, "4st_lift01", &savedPos);
            Hu3DModelObjPosGet(lbl_1_bss_30C[34].model, "4st_lift02", &targetPos);
            if (savedPos.y >= targetPos.y) {
                if ((s32) ((lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                               ? ((frandmod(100) < 50U) ? 1 : 0)
                               : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                                      ? ((frandmod(100) < 35U) ? 1 : 0)
                                      : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2)
                                             ? ((frandmod(100) < 20U) ? 1 : 0)
                                             : ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -14, 0, PAD_BUTTON_A, 0);
                } else {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, PAD_BUTTON_A, 0);
                }
                lbl_1_bss_114[playerNo].aiRouteChoice = 4;
            } else {
                targetPos.x = targetPos.z = 0.0f;
                savedPos.x = savedPos.z = 0.0f;
                PSVECSubtract(&targetPos, &savedPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 160.0f) {
                    if ((s32) ((lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                                   ? ((frandmod(100) < 50U) ? 1 : 0)
                                   : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                                          ? ((frandmod(100) < 35U) ? 1 : 0)
                                          : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2)
                                                 ? ((frandmod(100) < 20U) ? 1 : 0)
                                                 : ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], -14, 0, PAD_BUTTON_A, 0);
                    } else {
                        fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, PAD_BUTTON_A, 0);
                    }
                    lbl_1_bss_114[playerNo].aiRouteChoice = 4;
                }
            }
            break;
        case 34:
            Hu3DModelObjPosGet(lbl_1_bss_30C[34].model, "4st_lift02", &savedPos);
            Hu3DModelObjPosGet(lbl_1_bss_30C[30].model, "4st_goal", &targetPos);
            if (savedPos.y >= targetPos.y) {
                if ((s32) ((lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                               ? ((frandmod(100) < 50U) ? 1 : 0)
                               : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                                      ? ((frandmod(100) < 35U) ? 1 : 0)
                                      : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2)
                                             ? ((frandmod(100) < 20U) ? 1 : 0)
                                             : ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -14, 0, PAD_BUTTON_A, 0);
                } else {
                    fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, PAD_BUTTON_A, 0);
                }
                lbl_1_bss_114[playerNo].aiRouteChoice = 1;
            } else {
                targetPos.x = targetPos.z = 0.0f;
                savedPos.x = savedPos.z = 0.0f;
                PSVECSubtract(&targetPos, &savedPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 160.0f) {
                    if ((s32) ((lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                                   ? ((frandmod(100) < 50U) ? 1 : 0)
                                   : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                                          ? ((frandmod(100) < 35U) ? 1 : 0)
                                          : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2)
                                                 ? ((frandmod(100) < 20U) ? 1 : 0)
                                                 : ((frandmod(100) < 5U) ? 1 : 0)))) != 0) {
                        fn_1_C450(&lbl_1_bss_84[playerNo], -14, 0, PAD_BUTTON_A, 0);
                    } else {
                        fn_1_C450(&lbl_1_bss_84[playerNo], -56, 0, PAD_BUTTON_A, 0);
                    }
                    lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                }
            }
            break;
        }
    }
    if (lbl_1_bss_114[playerNo].aiRouteChoice != 0) {
        lbl_1_bss_84[playerNo].buttonHeld = PAD_BUTTON_A;
        if (lbl_1_bss_114[playerNo].aiRouteChoice == 2) {
            if ((s32) ((lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                           ? ((frandmod(100) < 50U) ? 1 : 0)
                           : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                                  ? ((frandmod(100) < 35U) ? 1 : 0)
                                  : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2)
                                         ? ((frandmod(100) < 20U) ? 1 : 0)
                                         : (((frandmod(100) < 5U) ? 1 : 0) == 0)))) != 0) {
                Hu3DModelObjPosGet(lbl_1_bss_30C[32].model, "4st_lift00", &liftPos);
                if (playerPos.x <= liftPos.x) {
                    lbl_1_bss_84[playerNo].stickX = 0;
                    lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                }
            }
        } else if (lbl_1_bss_114[playerNo].aiRouteChoice == 3) {
            if ((s32) ((lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                           ? ((frandmod(100) < 50U) ? 1 : 0)
                           : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                                  ? ((frandmod(100) < 35U) ? 1 : 0)
                                  : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2)
                                         ? ((frandmod(100) < 20U) ? 1 : 0)
                                         : (((frandmod(100) < 5U) ? 1 : 0) == 0)))) != 0) {
                Hu3DModelObjPosGet(lbl_1_bss_30C[33].model, "4st_lift01", &liftPos);
                if (playerPos.x <= liftPos.x) {
                    lbl_1_bss_84[playerNo].stickX = 0;
                    lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                }
            }
        } else if (lbl_1_bss_114[playerNo].aiRouteChoice == 4) {
            if ((s32) ((lbl_1_bss_84[playerNo].cpuDifficulty == 0)
                           ? ((frandmod(100) < 50U) ? 1 : 0)
                           : ((lbl_1_bss_84[playerNo].cpuDifficulty == 1)
                                  ? ((frandmod(100) < 35U) ? 1 : 0)
                                  : ((lbl_1_bss_84[playerNo].cpuDifficulty == 2)
                                         ? ((frandmod(100) < 20U) ? 1 : 0)
                                         : (((frandmod(100) < 5U) ? 1 : 0) == 0)))) != 0) {
                Hu3DModelObjPosGet(lbl_1_bss_30C[34].model, "4st_lift02", &liftPos);
                if (playerPos.x <= liftPos.x) {
                    lbl_1_bss_84[playerNo].stickX = 0;
                    lbl_1_bss_114[playerNo].aiRouteChoice = 1;
                }
            }
        }
    }
    MgPlayerPadSet(lbl_1_bss_114[playerNo].player, lbl_1_bss_84[playerNo].stickX,
                   lbl_1_bss_84[playerNo].stickY, lbl_1_bss_84[playerNo].buttonDown,
                   lbl_1_bss_84[playerNo].buttonHeld);
}

/* Called by fn_1_8180 to store controller values for the player update. */
void fn_1_C450(M618Input *input, int stickX, int stickY, int buttonDown, int buttonHeld)
{
    input->stickX = stickX;
    input->stickY = stickY;
    input->buttonDown = buttonDown;
    input->buttonHeld = buttonHeld;
}
