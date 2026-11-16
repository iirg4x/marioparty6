/* Microphone selection, activity startup, player awards, and pause handling. */
#include "REL/mikeactdll/position.h"
#include "REL/mikeactdll/microphone_init.h"
#include "REL/mikeactdll/sound.h"
#include "REL/mikeactdll/module.h"
#include "REL/mikeactdll/model_hook_setup.h"
#include <stdio.h>

#include "math.h"

#include "dolphin/gx.h"

#include "dolphin/types.h"

#include "REL/mikeactdll/module_types.h"

#include "REL/mikeactdll/event_callback.h"

#include "game/main.h"

#include "game/object.h"

#include "game/charman.h"

#include "game/hu3d.h"

#include "game/gamework.h"

#include "game/wipe.h"

#include "game/mg/timer.h"

#include "game/process.h"

#include "game/saveload.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/memory.h"

#include "game/mg/seqman.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/sprite.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

#include "messdir_enum.h"

#include "msm_se.h"

#include "game/mic.h"

#include "game/window.h"

#include "msm_stream.h"

#include "dolphin.h"

#include "game/armem.h"

#include "humath.h"

extern void (*lbl_1_bss_24)(void);

void MikeActPreActorCallbackSet(void (*callback)(void))
{
    lbl_1_bss_24 = callback;
}

extern void (*lbl_1_bss_20)(void);

void MikeActPostActorCallbackSet(void (*callback)(void))
{
    lbl_1_bss_20 = callback;
}

extern u32 lbl_1_bss_30;

extern u16 lbl_1_bss_34;

extern MGTIMER *lbl_1_bss_3C;

extern u16 lbl_1_bss_6A;

extern s16 lbl_1_data_B0[3];

extern void (*lbl_1_data_0[4])(void);

struct MikeActPlayerRecord_s;

void fn_1_634(void);
void fn_1_A10(void);
void fn_1_AC8(void);
void fn_1_B4C(void);
void (*lbl_1_data_0[4])(void) = { fn_1_634, fn_1_A10, fn_1_AC8, fn_1_B4C };
char *lbl_1_data_58[3] = { "/mic/ctx/mgg_map1_words", "/mic/ctx/mgg_map2_words", "/mic/ctx/mgg_map3_words" };
Point3d lbl_1_data_64[4] = { { 50000.0f, 0.0f, 0.0f }, { 49850.0f, 0.0f, -300.0f }, { 49650.0f, 0.0f, -300.0f }, { 50200.0f, 0.0f, -300.0f } };
OM_CAMERA_VIEW lbl_1_data_94 = { { 50000.0f, 100.0f, -150.0f }, { -30.62f, 0.0f, 0.0f }, 2500.0f };
s16 lbl_1_data_B0[3] = { 107, 108, 109 };

void fn_1_320(void)
{
    u32 tick;
    u32 previousTick;

    MikeActActivityInit();
    fn_8003AB38(lbl_1_bss_6C, 0x2000, 0, 0, -1, fn_1_250);
    lbl_1_bss_30 = 0;
    WipeCreate(1, 0, 0x1E);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    tick = OSGetTick();
    fn_1_3150(lbl_1_data_B0[lbl_1_bss_34]);
    fn_1_30A0(&lbl_1_bss_F0->position, 0x536);
    while (omSysExitReq == 0) {
        if (lbl_1_bss_6A >= 4) {
            break;
        }
        lbl_1_data_0[lbl_1_bss_6A]();
        lbl_1_bss_30++;
        HuPrcVSleep();
        previousTick = tick;
        tick = OSGetTick();
    }
    fn_1_5AD8(lbl_1_bss_C4[0], 0);
    HuAudFadeOut(0x3E8);
    WipeCreate(2, 0, 0x1E);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    fn_1_1AE4();
    MgTimerKill(lbl_1_bss_3C);
    SLSaveModeExec(0);
    if (lbl_1_bss_6A == 4) {
        fn_8003A5E4(0, 1);
    } else {
        fn_8003A5E4(1, 1);
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

extern u16 lbl_1_bss_38;

extern u16 lbl_1_bss_3A;

/* Sample the controller button states until the activity exits. */
void fn_1_534(void) {
    while (omSysExitReq == 0) {
        lbl_1_bss_3A = lbl_802C0010[0];
        lbl_1_bss_38 = lbl_802C0008[0];
        HuPrcVSleep();
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Create and position the activity timer display. */
void fn_1_598(void) {
    lbl_1_bss_3C = MgTimerCreate(1);
    MgTimerPosSet(lbl_1_bss_3C, 87.0f, 84.0f);
    MgTimerParamSet(lbl_1_bss_3C, 0, 0x8C9F, fn_8003D420((s16) lbl_1_bss_34));
    MgTimerRecordDispOff(lbl_1_bss_3C);
}

extern u16 lbl_1_bss_66;

extern u16 lbl_1_bss_68;

void fn_1_634(void)
{
    Point3d destination;
    s32 playerIndex;

    switch (lbl_1_bss_68) {
    case 0:
        if ((lbl_1_bss_F0->actor->colGroundAttr & 0x407F) == 0) {
            return;
        }
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            fn_8007EEBC(lbl_1_bss_C4[playerIndex]->player, 4);
        }
        lbl_1_bss_66 = 0;
        fn_80086CBC(lbl_1_bss_F0->actor, 130.0f);
        lbl_1_bss_68++;
        return;
    case 1:
        if (lbl_1_bss_C4[0]->player->mode == 0) {
            lbl_1_bss_68++;
        }
        if (lbl_1_bss_34 != 0) {
            fn_1_F2D8(1);
            lbl_1_bss_68 = 4;
            return;
        }
        break;
    case 2:
        destination = lbl_1_data_2E0[0];
        destination.x += 100.0f;
        if (fn_8007E8B4(lbl_1_bss_C4[0]->player, &destination, 5.0f, 5.0f) != 0) {
            lbl_1_bss_66 = 0;
            fn_1_1B40(2, 0);
            fn_1_1E1C(MESSNUM(MESS_MICGOSUPORT, 0), 1);
            lbl_1_bss_68++;
            return;
        }
        break;
    case 3:
        if (lbl_1_bss_C4[0]->actionState == 3) {
            lbl_1_bss_68 = 0x63;
        }
        if (lbl_1_bss_66 == 0) {
            fn_1_F2D8(1);
            fn_1_F370(MESSNUM(MESS_MICGO, 0), 0xB4);
            HuAudFXPlay(MSM_SE_GUIDE_DAY_EXPLAIN);
        } else if (lbl_1_bss_66 == 0xB4) {
            fn_1_F370(MESSNUM(MESS_MICGO, 1), 0xB4);
        } else if (lbl_1_bss_66 == 0x168) {
            fn_1_F420(MESSNUM(MESS_MICGO, 33), 0xB4);
            for (playerIndex = 1; playerIndex < 4; playerIndex++) {
                fn_8007EEBC(lbl_1_bss_C4[playerIndex]->player, 5);
            }
        } else if (lbl_1_bss_66 == 0x21C) {
            lbl_1_bss_68 = 0x63;
        }
        lbl_1_bss_66++;
        return;
    case 4:
        fn_1_1B40(2, 0);
        fn_1_1E1C(MESSNUM(MESS_MICGOSUPORT, 0), 1);
        /* fall through */
    case 0x63:
        fn_1_F598();
        fn_1_2164(1);
        fn_1_45C4(lbl_1_bss_C4[0], 0x2000);
        fn_80087168(lbl_1_bss_C4[0]->player, 1U);
        fn_1_E648(1);
        MgTimerRecordDispOn(lbl_1_bss_3C);
        lbl_1_bss_68 = 0;
        lbl_1_bss_6A = 1;
        break;
    }
}

/* Start the timer and activity once the selected player enters the course. */
void fn_1_A10(void) {
    Point3d position;

    if (lbl_1_bss_F0->owner) {
        fn_80086BC8(lbl_1_bss_F0->owner->player->actor, &position);
        if (position.x > 0.0f) {
            GameMesCreate(2, 0);
            omSysPauseEnable(1U);
            MgTimerModeOnSet(lbl_1_bss_3C, 0);
            HuAudFXPlay(MSM_SE_CMN_TIMER_START);
            HuAudFXPlay(MSM_SE_GUIDE_DAY_INTRO);
            lbl_1_bss_6A = 2;
        }
    }
}

/* Signal when the current player passes the course endpoint. */
void fn_1_AC8(void) {
    Point3d position;

    if (lbl_1_bss_F0->owner) {
        fn_80086BC8(lbl_1_bss_F0->owner->player->actor, &position);
        if (position.x > 49000.0f) {
            HuAudFXPlay(MSM_SE_CMN_TIMER_START);
            lbl_1_bss_6A = 3;
        }
    }
}

extern s16 lbl_1_bss_5A[5];

extern MCResponseCallback lbl_1_bss_1C;

extern s16 lbl_1_bss_58;

extern MCResponseCallback lbl_1_bss_54;

extern s32 lbl_1_bss_50;

extern s16 lbl_1_bss_4E;

extern s16 lbl_1_bss_4C;

extern MCResponseCallback lbl_1_bss_48;

extern s32 lbl_1_bss_44;

extern s16 lbl_1_bss_40;

extern s32 lbl_1_bss_14;

extern u16 lbl_1_bss_36;

extern MikeActPlayerRecord *lbl_1_bss_B8;

extern char lbl_1_bss_28[4];

extern s16 lbl_1_bss_2C;

extern u16 lbl_1_bss_64;

extern s32 lbl_1_bss_8;

extern s16 lbl_1_bss_C;

extern s16 lbl_1_bss_1B4;

extern MikeActPlayerRecord *lbl_1_bss_C4[5];

extern MikeActStateRecord lbl_1_bss_1B8[0x100];

extern void HuMCSelWinKill(void);

extern void HuMCSessionKill(s32 session);

void fn_1_B4C(void)
{
    s32 starAwards[3][4] = { { 0, 1, 2, 3 }, { 0, 2, 4, 6 }, { 0, 3, 6, 10 } };
    OM_CAMERA_VIEW cameraView;
    Point3d spawnPosition;
    HUWIN * choiceWindow;
    f32 cameraHeight;
    s32 awardRank;
    s32 playerIndex;

    switch (lbl_1_bss_68)
    {
    case 0 :
        HuAudSStreamFadeOut(0, 0xC8);
        HuAudFXPlay(MSM_SE_GUIDE_DAY_INTRO);
        GameMesCreate(2, 1);
        MgTimerModeOffSet(lbl_1_bss_3C);
        fn_1_F370(MESSNUM(MESS_MICGO, 0x20), 0);
        fn_1_10770(1);
        HuMCSelWinKill();
        fn_1_2164(0);
        fn_80080D7C(1, 0);
        fn_1_1BF4();
        lbl_1_bss_66 = 0;
        lbl_1_bss_68++;
        break;
    case 1 :
        if (lbl_1_bss_66++ == 0x78)
        {
            WipeCreate(2, 0, 0x1E);
            lbl_1_bss_68++;
        }
        break;
    case 2 :
        if (WipeCheck() == 0)
        {
            (* lbl_1_bss_C4)->speedIndex = 0;
            cameraHeight = 2 == lbl_1_bss_34 ? 300.0f : 0.0f;
            fn_1_E648(0);
            MgTimerPosSet(lbl_1_bss_3C, 288.0f, 84.0f);
            fn_1_F2D8(0);
            cameraView = lbl_1_data_94;
            cameraView.center.y += cameraHeight;
            fn_8003E5A4(& cameraView);
            fn_1_36E4(1);
            spawnPosition.x = 200.0f + lbl_1_data_64[3].x;
            spawnPosition.y = 50.0f + cameraHeight;
            spawnPosition.z = -500.0f;
            fn_80026790(lbl_1_bss_1B8[lbl_1_bss_1B4].model_id, & spawnPosition);
            fn_1_10770(0);
            fn_1_45B0(lbl_1_bss_C4[0], 0x2000);
            fn_80087158(lbl_1_bss_C4[0]->player, 1U);
            playerIndex = 0;
            while (playerIndex < 4)
            {
                fn_80026A44(lbl_1_bss_C4[playerIndex]->player->actor->mdlId, 1);
                spawnPosition = lbl_1_data_64[playerIndex];
                spawnPosition.y += cameraHeight;
                fn_80086F00(lbl_1_bss_C4[playerIndex]->player, & spawnPosition);
                fn_80086CA8(lbl_1_bss_C4[playerIndex]->player->actor, 0.0f);
                fn_8007EEBC(lbl_1_bss_C4[playerIndex]->player, 0);
                if (playerIndex != 0)
                {
                    fn_1_665C(lbl_1_bss_C4[playerIndex], 0);
                }
                playerIndex++;
            }
            HuPrcSleep(0xA);
            WipeCreate(1, 0, 0x1E);
            lbl_1_bss_68++;
        }
        break;
    case 3 :
        if (WipeCheck() == 0)
        {
            MgTimerRecordSet(lbl_1_bss_3C, - 1);
            if (fn_8003D420((s16) lbl_1_bss_34) > (u32) MgTimerValueGet(lbl_1_bss_3C))
            {
                fn_8003D404((s16) lbl_1_bss_34, MgTimerValueGet(lbl_1_bss_3C));
                lbl_1_bss_64 = GameMesCreate(0xE, MgTimerValueGet(lbl_1_bss_3C));
                HuAudJinglePlay(MSM_STREAM_SUCCESS);
            }
            fn_8003D174(1);
            lbl_1_bss_68++;
        }
        break;
    case 4 :
        if (GameMesStatGet((s16) lbl_1_bss_64) == 0)
        {
            lbl_1_bss_66 = 0;
            ++lbl_1_bss_68;
        }
        break;
    case 5 :
        if (lbl_1_bss_66++ == 0x3C)
        {
            HuAudJinglePlay(MSM_STREAM_GETTING_A_STAR);
            fn_8007EEBC((* lbl_1_bss_C4)->player, 6);
            for (playerIndex = 1; playerIndex < 4; playerIndex++)
            {
                fn_8007EEBC(lbl_1_bss_C4[playerIndex]->player, 5);
            }
            lbl_1_bss_66 = 0;
            lbl_1_bss_68++;
        }
        break;
    case 6 :
        if (HuAudSStreamStatGet(2) == 0)
        {
            HuAudBGMPlay(MSM_STREAM_WAY_TO_GO);
            HuAudFXPlay(MSM_SE_GUIDE_DAY_INTRO);
            lbl_1_bss_2C = HuWinExCreateFrame(-10000.0f, 372.0f, 0x220, 0x44, - 1, 4);
            HuWinAttrSet(lbl_1_bss_2C, 0x800);
            HuWinExOpen(lbl_1_bss_2C);
            HuWinMesSet(lbl_1_bss_2C, MESSNUM(MESS_MICGO, 0x1C));
            fn_1_10770(2);
            HuAudFXPlay(MSM_SE_CMN_STAR_GET);
            lbl_1_bss_66 = 0;
            lbl_1_bss_68++;
        }
        break;
    case 7 :
        if (lbl_1_bss_66++ == 0xB4)
        {
            lbl_1_bss_66 = 0;
            HuWinExClose(lbl_1_bss_2C);
            lbl_1_bss_68++;
        }
        break;
    case 8 :
        playerIndex = MgTimerValueGet(lbl_1_bss_3C);
        if (playerIndex >= 0x2A30)
        {
            awardRank = 0;
            lbl_1_bss_68 = 0x62;
            break;
        }
        if (playerIndex >= 0x2328)
        {
            awardRank = 1;
        }
        else
        {
            if (! (! (playerIndex >= 0x1C20)))
            {
                awardRank = 2;
            }
            else
            {
                awardRank = 3;
            }
        }
        lbl_1_bss_2C = HuWinExCreateFrame(-10000.0f, 372.0f, 0x220, 0x44, - 1, 4);
        HuWinAttrSet(lbl_1_bss_2C, 0x800);
        HuWinAttrSet(lbl_1_bss_2C, 0x10);
        HuWinExOpen(lbl_1_bss_2C);
        for (playerIndex = 0; playerIndex < 3; playerIndex++)
        {
            lbl_1_bss_28[playerIndex] = 0;
        }
        sprintf(lbl_1_bss_28, "%d", starAwards[lbl_1_bss_34][awardRank]);
        HuWinInsertMesSet(lbl_1_bss_2C, (u32) lbl_1_bss_28, 0);
        switch (awardRank)
        {
        case 1 :
            HuWinMesSet(lbl_1_bss_2C, MESSNUM(MESS_MICGO, 0x24));
            break;
        case 2 :
            HuWinMesSet(lbl_1_bss_2C, MESSNUM(MESS_MICGO, 0x23));
            break;
        case 3 :
            HuWinMesSet(lbl_1_bss_2C, MESSNUM(MESS_MICGO, 0x22));
            break;
        }
        fn_8003D174(starAwards[lbl_1_bss_34][awardRank]);
        HuAudFXPlay(MSM_SE_CMN_STAR_GET);
        lbl_1_bss_68++;
        break;
    case 9 :
        if (lbl_1_bss_66++ == 0xB4)
        {
            HuWinExClose(lbl_1_bss_2C);
            lbl_1_bss_68 = 0x62;
        }
        break;
    case 0x62 :
        HuAudFXPlay(MSM_SE_GUIDE_DAY_INTRO);
        lbl_1_bss_2C = HuWinExCreateFrame(-10000.0f, 372.0f, 0x220, 0x44, - 1, 4);
        HuWinAttrSet(lbl_1_bss_2C, 0x800);
        HuWinAttrSet(lbl_1_bss_2C, 0x4010);
        HuWinPadMaskSet(lbl_1_bss_2C, 1);
        HuWinExOpen(lbl_1_bss_2C);
        HuWinMesSet(lbl_1_bss_2C, MESSNUM(MESS_MICGO, 0x1D));
        fn_1_10770(0);
        HuMCSessionKill(0);
        lbl_1_bss_68 = 0x63;
        break;
    case 0x63 :
        choiceWindow = & winData[lbl_1_bss_2C];
        fn_1_1B40(4, fn_1_2FB4);
        HuWinChoiceSet(lbl_1_bss_2C, - 1);
        lbl_1_bss_C = - 1;
        lbl_1_bss_8 = 1;
        while (lbl_1_bss_C == - 1)
        {
            if (HuWinStatGet(lbl_1_bss_2C) == 0)
            {
                lbl_1_bss_8 = 0;
                lbl_1_bss_C = choiceWindow->choice;
            }
            HuPrcVSleep();
        }
        fn_1_1BF4();
        if (lbl_1_bss_8 != 0)
        {
            if (choiceWindow->choice != lbl_1_bss_C)
            {
                choiceWindow->choice = lbl_1_bss_C;
                HuAudFXPlay(MSM_SE_CMN_CURSOR_MOVE);
                HuPrcSleep(0x1E);
            }
            HuAudFXPlay(MSM_SE_CMN_MENU_ADVANCE);
        }
        HuPrcSleep(0x1E);
        lbl_1_bss_6A = lbl_1_bss_C == 0 ? 4 : 5;
        HuWinKill(lbl_1_bss_2C);
        break;
    }
}

void fn_1_17C4(MCResponse_s *update)
{
    s16 selection;
    s32 expectedSelection;
    s32 microphoneSelection;

    if (update->status == -4 || update->status == -2) {
        if (HuMCMicGet() != 2 && lbl_1_bss_14 == 0) {
            HuAudFXPlay(MSM_SE_GUIDE_DAY_DISAPPOINTED);
            fn_1_F4F4(MESSNUM(MESS_MICGO, 0x1E));
        }
    }

    if (update->status == 0 && update->count != 0) {
        if (lbl_1_bss_1C != 0) {
            if (fn_1_1C5C() == 1) {
                if (lbl_1_bss_34 != 2) {
                    expectedSelection = 5;
                } else {
                    expectedSelection = 6;
                }
                if (*update->result == expectedSelection) {
                    fn_1_21A0();
                }
            }
            lbl_1_bss_1C(update);
        } else {
            if (HuMCMicGet() != 2) {
                if (fn_1_1C5C() == 2) {
                    microphoneSelection = 1;
                } else {
                    microphoneSelection = 7;
                }
                if (*update->result == microphoneSelection) {
                    fn_1_21A0();
                    return;
                }
            }

            switch (lbl_1_bss_36) {
            case 0: {
                selection = *update->result;
                fn_1_55E8(lbl_1_bss_B8, selection);
                break;
            }
            case 1:
                selection = HuMCMicGet() == 2 ? 0 : 6;
                if (*update->result == selection) {
                    lbl_1_bss_36 = 2;
                    return;
                }
            }
        }
    }
}

void fn_1_1998(void)
{
    HuMCInit(0);
    HuMCSessionSet(0);
    lbl_1_bss_5A[0] = HuMCContextCreate("/mic/ctx/mgg_words");
    lbl_1_bss_5A[1] = HuMCContextCreate(lbl_1_data_58[lbl_1_bss_34]);
    lbl_1_bss_5A[2] = HuMCContextCreate("/mic/ctx/mgg_star");
    lbl_1_bss_5A[3] = HuMCContextCreate("/mic/ctx/mgg_pause");
    lbl_1_bss_5A[4] = HuMCContextCreate("/mic/ctx/mic_yesno");
    lbl_1_bss_1C = 0;
    lbl_1_bss_58 = -1;
    lbl_1_bss_54 = 0;
    lbl_1_bss_50 = -1;
    lbl_1_bss_4E = -1;
    lbl_1_bss_4C = -1;
    lbl_1_bss_48 = 0;
    lbl_1_bss_44 = -1;
    lbl_1_bss_40 = -1;
}

void fn_1_1AE4(void)
{
    s32 window;

    HuMCListenerKill();
    for (window = 0; window < 5; window++) {
        HuMCContextKill(lbl_1_bss_5A[window]);
    }
    HuMCSessionClose();
    HuMCClose();
}

void fn_1_1B40(s16 playerNo, MCResponseCallback callback)
{
    lbl_1_bss_1C = callback;
    HuMCListenerCreate(lbl_1_bss_5A[playerNo], fn_1_17C4, 0);
    if ((u32)(lbl_1_bss_50 + 0x10000) == 0xFFFFU) {
        HuMCSelWinCreate(-10001.0f, -10003.0f);
    }
    lbl_1_bss_58 = playerNo;
    lbl_1_bss_54 = callback;
}

void fn_1_2220(void);

/* Stop microphone recognition and clear the active selection state. */
void fn_1_1BF4(void) {
    HuMCSessionKill(0);
    HuMCListenerKill();
    lbl_1_bss_58 = -1;
    lbl_1_bss_54 = 0;
    lbl_1_bss_50 = -1;
    lbl_1_bss_4E = -1;
}

s16 fn_1_1C5C(void)
{
    return lbl_1_bss_58;
}

/* Save the current microphone selection and message state. */
void fn_1_1C6C(void) {
    lbl_1_bss_4C = lbl_1_bss_58;
    lbl_1_bss_48 = lbl_1_bss_54;
    lbl_1_bss_44 = lbl_1_bss_50;
    lbl_1_bss_40 = lbl_1_bss_4E;
}

extern void HuMCSelWinCreate(f32 x, f32 y);

void fn_1_1CD0(void)
{
    s16 selectionIndex;
    MCResponseCallback callback;

    if (lbl_1_bss_4C != -1) {
        callback = lbl_1_bss_48;
        selectionIndex = lbl_1_bss_4C;
        fn_1_1B40(selectionIndex, callback);
    }
    if ((u32)(lbl_1_bss_44 + 0x10000) != 0xFFFFU) {
        fn_1_1E1C(lbl_1_bss_44, lbl_1_bss_40);
    }
    lbl_1_bss_4C = -1;
    lbl_1_bss_48 = 0;
    lbl_1_bss_44 = -1;
    lbl_1_bss_40 = -1;
}

void fn_1_1E1C(s32 messageNo, s16 itemNo) {
    HuMCSelWinItemSet((u32) messageNo, itemNo, 0);
    lbl_1_bss_50 = messageNo;
    lbl_1_bss_4E = itemNo;
}

void fn_1_1E78(void)
{
    lbl_1_bss_36 = 1;
    lbl_1_bss_1C = 0;
    HuMCListenerCreate(lbl_1_bss_5A[0], fn_1_17C4, 0);
    if ((u32) (lbl_1_bss_50 + 0x10000) == 0xFFFFU) {
        HuMCSelWinCreate(-10001.0f, -10003.0f);
    }
    lbl_1_bss_58 = 0;
    lbl_1_bss_54 = 0;
    HuMCSelWinItemSet(0x250007, 1, 0);
    lbl_1_bss_50 = 0x250007;
    lbl_1_bss_4E = 1;
}

void fn_1_1F5C(void)
{
    lbl_1_bss_36 = 0;
}

/* Report whether the microphone selection has completed. */
s32 fn_1_1F70(void) {
    if (lbl_1_bss_36 != 2) {
        return 0;
    }
    return 1;
}

void fn_1_1F94(void) {
    fn_1_49D0(lbl_1_bss_B8);
    fn_1_45B0(lbl_1_bss_B8, 0x2000);
}

void fn_1_1FD4(void)
{
    fn_1_49D0(lbl_1_bss_B8);
    fn_1_45C4(lbl_1_bss_B8, 0x2000);
    lbl_1_bss_1C = 0;
    HuMCListenerCreate(lbl_1_bss_5A[0], fn_1_17C4, 0);
    if ((u32) (lbl_1_bss_50 + 0x10000) == 0xFFFFU) {
        HuMCSelWinCreate(-10001.0f, -10003.0f);
    }
    lbl_1_bss_58 = 0;
    lbl_1_bss_54 = 0;
    HuMCSelWinItemSet(0x250001, 6, 0);
    lbl_1_bss_50 = 0x250001;
    lbl_1_bss_4E = 6;
    fn_1_34C0();
    fn_1_36E4(0);
    lbl_1_bss_36 = 0;
}

extern HUPROCESS *lbl_1_bss_18;

/* Start the pause process and enable system pausing. */
void fn_1_20E8(void) {
    lbl_1_bss_18 = fn_8000DAC0(fn_1_2390, 0xFDE8U, 0x2000U, 0, lbl_1_bss_6C);
    HuPrcSetStat(lbl_1_bss_18, 4U);
    lbl_1_bss_14 = 0;
    fn_1_2164(0);
}

void fn_1_2164(s32 pauseEnabled) {
    lbl_1_bss_10 = pauseEnabled;
    omSysPauseEnable((u8) pauseEnabled);
}

/* Pause only after wipes, transitions, and messages have finished. */
void fn_1_21A0(void) {
    if ((WipeCheck() != 0) || (fn_8003B774() == -1) || (GameMesStatGet(-1) != 0)) {
        return;
    }
    if ((lbl_1_bss_10 != 0) && (lbl_1_bss_14 == 0)) {
        fn_8003E3F0(1);
    }
}

extern void fn_1_2548(void);

extern u32 lbl_1_bss_4;

HUPROCESS *lbl_1_bss_6C;
u16 lbl_1_bss_6A;
u16 lbl_1_bss_68;
u16 lbl_1_bss_66;
u16 lbl_1_bss_64;
s16 lbl_1_bss_5A[5];
s16 lbl_1_bss_58;
MCResponseCallback lbl_1_bss_54;
s32 lbl_1_bss_50;
s16 lbl_1_bss_4E;
s16 lbl_1_bss_4C;
MCResponseCallback lbl_1_bss_48;
s32 lbl_1_bss_44;
s16 lbl_1_bss_40;
MGTIMER *lbl_1_bss_3C;
u16 lbl_1_bss_3A;
u16 lbl_1_bss_38;
u16 lbl_1_bss_36;
u16 lbl_1_bss_34;
u32 lbl_1_bss_30;
s16 lbl_1_bss_2C;
char lbl_1_bss_28[4];
void (*lbl_1_bss_24)(void);
void (*lbl_1_bss_20)(void);
MCResponseCallback lbl_1_bss_1C;
HUPROCESS *lbl_1_bss_18;
s32 lbl_1_bss_14;
s32 lbl_1_bss_10;
s16 lbl_1_bss_E;
s16 lbl_1_bss_C;
s32 lbl_1_bss_8;

u32 lbl_1_bss_4;

void fn_1_2220(void)
{
    HUPROCESS *process;

    HuAudFXPlay(MSM_SE_CMN_START_BUTTON_CONFIRM);
    lbl_1_bss_4C = lbl_1_bss_58;
    lbl_1_bss_48 = lbl_1_bss_54;
    lbl_1_bss_44 = lbl_1_bss_50;
    lbl_1_bss_40 = lbl_1_bss_4E;
    fn_1_1BF4();
    process = fn_8000DAC0(fn_1_2548, 0xFDE8U, 0x2000U, 0, lbl_1_bss_6C);
    HuPrcSetStat(process, 0xCU);
    if ((u32)lbl_802C0260->work[1] != 0xFFFFU) {
        lbl_1_bss_4 = lbl_802C0260->work[1];
    } else {
        lbl_1_bss_4 = 0;
    }
    lbl_1_bss_14 = 1;
    omSysPauseEnable(0U);
}

void fn_1_2390(void)
{
    for (;;) {
        do {
            HuPrcVSleep();
        } while (lbl_1_bss_10 == 0);
        if (lbl_1_bss_14 != 0 || (u8)fn_8003B748() == 0) {
            continue;
        }
        fn_1_2220();
        while (lbl_1_bss_14 != 0) {
            HuPrcVSleep();
        }
    }
}

extern u16 lbl_802C0008[];

void fn_1_2548(void)
{
    GAMEMES gameMes;
    s16 windowId[2];
    HUWIN *messageWindow;
    f32 animation;
    s16 index;
    for (index = 0; index < 16; index++) {
        gameMes.sprId[index] = gameMes.grpId[index] = -1;
    }
    GameMesStrWinCreate(&gameMes, 0x420010U);
    for (index = 0; index < gameMes.charNum; index++) {
        HuSprPriSet(gameMes.grpId[0], index, 0);
    }

    windowId[0] = HuWinExCreateFrame(-10000.0f, 132.0f, 0x154, 0xC6, -1, 0);
    HuWinPriSet(windowId[0], 5);
    HuWinDispOn(windowId[0]);
    HuWinMesSpeedSet(windowId[0], 0);
    HuWinMesSet(windowId[0], 0x240018U);
    windowId[1] = HuWinExCreateFrame(-10000.0f, 346.0f, 0x154, 0x5E, -1, 0);
    HuWinPriSet(windowId[1], 5);
    HuWinDispOn(windowId[1]);
    HuWinMesSpeedSet(windowId[1], 0);
    HuWinMesSet(windowId[1], 0x240019U);
    HuWinAttrSet(windowId[1], 0x10U);
    HuWinPadMaskSet(windowId[1], 1 << lbl_1_bss_4);

    for (index = 1; index <= 20; index++) {
        animation = (f32)sin((M_PI * (f64)(4.5f * (f32)index)) / 180.0);
        HuSprGrpPosSet(gameMes.grpId[0], 288.0f, (150.0f * animation) - 50.0f);
        HuWinPosSet(windowId[0], (458.0f * animation) - 340.0f, 132.0f);
        HuWinPosSet(windowId[1], 340.0f + (-222.0f * animation), 346.0f);
        HuPrcVSleep();
    }

    lbl_1_bss_E = -1;
    messageWindow = &winData[windowId[1]];
    while (lbl_1_bss_E == -1) {
            HuWinAttrSet(windowId[1], 0x10U);
            fn_1_1B40(3, fn_1_2FB4);
            HuWinChoiceSet(windowId[1], -1);
            lbl_1_bss_8 = 1;
            while (lbl_1_bss_E == -1) {
                if (HuWinStatGet(windowId[1]) == 0) {
                    lbl_1_bss_8 = 0;
                    lbl_1_bss_E = messageWindow->choice;
                }
                if ((lbl_802C0008[lbl_1_bss_4] & 0x1000) != 0) {
                    lbl_1_bss_E = 0;
                }
                HuPrcVSleep();
            }
            fn_1_1BF4();
            if (lbl_1_bss_E != 0) {
                if (lbl_1_bss_8 != 0) {
                    if (messageWindow->choice != lbl_1_bss_E) {
                        messageWindow->choice = lbl_1_bss_E;
                        HuAudFXPlay(MSM_SE_CMN_CURSOR_MOVE);
                        HuPrcSleep(0x1E);
                    }
                    HuAudFXPlay(MSM_SE_CMN_MENU_ADVANCE);
                }
                if (lbl_1_bss_E == 1) {
                    HuWinMesSet(windowId[1], 0x24001AU);
                } else {
                    HuWinMesSet(windowId[1], 0x24001BU);
                }
                HuWinAttrReset(windowId[1], 0x10U);
                fn_1_1B40(4, fn_1_2FB4);
                lbl_1_bss_C = -1;
                HuWinChoiceSet(windowId[1], -1);
                lbl_1_bss_8 = 1;
                while (lbl_1_bss_C == -1) {
                    if (HuWinStatGet(windowId[1]) == 0) {
                        lbl_1_bss_8 = 0;
                        if (messageWindow->choice == -1) {
                            lbl_1_bss_C = 1;
                        } else {
                            lbl_1_bss_C = messageWindow->choice;
                        }
                    }
                    HuPrcVSleep();
                }
                fn_1_1BF4();
                if (lbl_1_bss_8 != 0) {
                    if (messageWindow->choice != lbl_1_bss_C) {
                        messageWindow->choice = lbl_1_bss_C;
                        HuAudFXPlay(MSM_SE_CMN_CURSOR_MOVE);
                        HuPrcSleep(0x1E);
                    }
                    HuAudFXPlay(MSM_SE_CMN_MENU_ADVANCE);
                }
                if (lbl_1_bss_C == 1) {
                    HuWinMesSet(windowId[1], 0x240019U);
                    lbl_1_bss_E = -1;
                } else {
                    HuPrcSleep(0x1E);
                }
            } else {
                DebugValue(HuMCMicGet() != 2);
            }
    }

    for (index = 1; index <= 10; index++) {
        animation = (f32)cos((M_PI * (f64)(9.0f * (f32)index)) / 180.0);
        HuSprGrpPosSet(gameMes.grpId[0], 288.0f, (150.0f * animation) - 50.0f);
        HuWinPosSet(windowId[0], (458.0f * animation) - 340.0f, 132.0f);
        HuWinPosSet(windowId[1], 340.0f + (-222.0f * animation), 346.0f);
        HuPrcVSleep();
    }

    fn_1_1BF4();
    HuWinKill(windowId[0]);
    HuWinKill(windowId[1]);
    HuSprGrpKill(gameMes.grpId[0]);
    omSysPauseEnable(1U);
    fn_8003E3F0(0);
    if (lbl_1_bss_E > 0) {
        lbl_1_bss_6A = lbl_1_bss_E == 1 ? 4 : 5;
    }
    if (lbl_1_bss_4C != -1) {
        fn_1_1B40(lbl_1_bss_4C, lbl_1_bss_48);
    }
    if ((u32)(lbl_1_bss_44 + 0x10000) != 0xFFFFU) {
        fn_1_1E1C(lbl_1_bss_44, lbl_1_bss_40);
    }
    lbl_1_bss_4C = -1;
    lbl_1_bss_48 = 0;
    lbl_1_bss_44 = -1;
    lbl_1_bss_40 = -1;
    lbl_1_bss_14 = 0;
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

u8 lbl_1_bss_0[0x4];
