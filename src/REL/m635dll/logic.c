#include "REL/m635dll.h"
#include "game/audio.h"
#include "game/pad.h"
#include "game/gamework.h"
#include "game/flag.h"
#include <string.h>

extern u16 lbl_1_data_0[6];
extern s16 lbl_1_data_12[4][2];
extern OM_CAMERA_VIEW lbl_1_data_24[3];
M635Work lbl_1_bss_4;
s32 lbl_1_bss_0;

void fn_1_408(s16 frameNo)
{
    OM_CAMERA_VIEW view;
    int i;
    s16 charNo;

    if (frameNo == 30) {
        if (lbl_1_bss_4.nightF == 0) {
            lbl_1_bss_0 = HuAudFXPlay(1865);
        } else {
            lbl_1_bss_0 = HuAudFXPlay(1866);
        }
    } else if (frameNo == 90) {
        HuAudFXStop(lbl_1_bss_0);
    }
    switch (lbl_1_bss_4.state) {
        case 0:
            MgSeqModeChangeOff();
            lbl_1_bss_4.state++;
            break;
        case 1:
            if (frameNo == 5) {
                view.center.x = 0.0f;
                view.center.y = 20.0f;
                view.center.z = -400.0f;
                view.rot.x = -25.0f;
                view.rot.y = view.rot.z = 0.0f;
                view.zoom = 2500.0f;
                omCameraViewMoveSimple(&view, 120);
                lbl_1_bss_4.state++;
            }
            break;
        case 2:
            if (omCameraViewCheck(1)) {
                for (i = 0; i < 4; i++) {
                    charNo = lbl_1_bss_BC[i].charNo;
                    fn_1_30C8(i, charNo);
                }
                for (i = 0; i < 2; i++) {
                    fn_1_33FC(i);
                }
                for (i = 0; i < 4; i++) {
                    fn_1_2F40(i);
                }
                lbl_1_bss_4.state++;
            }
            break;
        case 3:
            MgSeqModeNext();
            lbl_1_bss_4.state = 0;
            break;
    }
}

void fn_1_638(void)
{
    int i;
    M635Team *teams;

    for (i = 0; i < 4; i++) {
        fn_1_3218(i, 1, 0.0f);
    }
    Center.x = lbl_1_data_24[lbl_1_bss_4.winner + 1].center.x;
    Center.y = lbl_1_data_24[lbl_1_bss_4.winner + 1].center.y;
    Center.z = lbl_1_data_24[lbl_1_bss_4.winner + 1].center.z;
    CRot.x = lbl_1_data_24[lbl_1_bss_4.winner + 1].rot.x;
    CRot.y = lbl_1_data_24[lbl_1_bss_4.winner + 1].rot.y;
    CRot.z = lbl_1_data_24[lbl_1_bss_4.winner + 1].rot.z;
    CZoom = lbl_1_data_24[lbl_1_bss_4.winner + 1].zoom;
    if (lbl_1_bss_4.winner == -1) {
        teams = lbl_1_bss_4.team;
        fn_1_3340();
        for (i = 0; i < 2; i++) {
            Hu3DModelAttrSet(teams[i].unk_14.model, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(teams[i].unk_18.model, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(teams[i].unk_1C.model, HU3D_ATTR_DISPOFF);
        }
    } else {
        for (i = 0; i < 4; i++) {
            if (lbl_1_bss_4.winner != lbl_1_bss_BC[i].teamNo) {
                Hu3DModelAttrSet(lbl_1_bss_BC[i].model, HU3D_ATTR_DISPOFF);
            }
        }
    }
}

void fn_1_8B0(void)
{
    int i;

    if (lbl_1_bss_4.winner == -1) {
        for (i = 0; i < 4; i++) {
            fn_1_3218(i, 3, 8.0f);
        }
    } else {
        for (i = 0; i < 2; i++) {
            fn_1_3218(fn_1_32E0(lbl_1_bss_4.winner, i), 2, 8.0f);
        }
    }
}

void fn_1_954(s16 frameNo)
{
    int team;
    s16 winner;

    for (team = 0; team < 2; team++) {
        fn_1_9B8(team);
    }
    winner = fn_1_D84();
    if (winner != -1) {
        fn_1_F44(winner);
    }
}

void fn_1_9B8(s16 team)
{
    s16 pads[2];
    s16 players[2];
    s16 chars[2];
    u16 buttons[2];
    M635Team *work;
    int i;
    s16 player;
    s16 total;

    work = &lbl_1_bss_4.team[team];
    for (i = 0; i < 2; i++) {
        player = lbl_1_bss_B4[team][i];
        players[i] = lbl_1_bss_BC[player].playerNo;
        pads[i] = lbl_1_bss_BC[player].padNo;
        chars[i] = lbl_1_bss_BC[player].charNo;
        if (lbl_1_bss_BC[lbl_1_bss_B4[team][i]].comF == 0) {
            buttons[i] = HuPadBtnDown[pads[i]];
            buttons[i] &= (u16)~(PAD_TRIGGER_L | PAD_TRIGGER_R);
        } else {
            buttons[i] = fn_1_1168(team, i);
        }
    }
    switch (work->unk_0C) {
        case 0:
            if (fn_1_27E4(team, work->unk_0A) != 3 &&
                buttons[work->unk_0A] == lbl_1_data_0[work->buttonIndex[work->unk_0A]]) {
                fn_1_25EC(team, work->unk_0A, 3);
                fn_1_30C8(players[work->unk_0A], chars[work->unk_0A]);
                fn_1_2F40(players[work->unk_0A]);
                work->unk_08++;
                work->unk_0A = 1 - work->unk_0A;
                fn_1_33FC(team);
                if (work->unk_08 >= 8) {
                    work->unk_0C = 1;
                    for (i = 0; i < 2; i++) {
                        work->buttonIndex[i] = fn_1_EC0(work->buttonIndex[i], work->unk_0C);
                    }
                } else {
                    work->buttonIndex[work->unk_0A] = fn_1_EC0(work->buttonIndex[work->unk_0A], work->unk_0C);
                    fn_1_25EC(team, work->unk_0A, 1);
                }
            }
            break;
        case 1:
            if (fn_1_27E4(team, 0) != 4 && fn_1_27E4(team, 0) != 3) {
                fn_1_25EC(team, 0, 4);
                fn_1_25EC(team, 1, 4);
            }
            total = 0;
            for (i = 0; i < 2; i++) {
                if (buttons[i] == lbl_1_data_0[work->buttonIndex[i]]) {
                    fn_1_31BC(chars[i]);
                    work->pressCount[i]++;
                }
                total += work->pressCount[i];
            }
            fn_1_3738(team, total / 10 + 9);
            break;
    }
}

s16 fn_1_D84(void)
{
    s16 totals[2];
    s16 team;
    s16 player;

    if (lbl_1_bss_4.winner != -1) {
        return -1;
    }
    for (team = 0; team < 2; team++) {
        totals[team] = 0;
        if (lbl_1_bss_4.team[team].unk_0C != 0) {
            for (player = 0; player < 2; player++) {
                totals[team] += lbl_1_bss_4.team[team].pressCount[player];
            }
        }
    }
    if (totals[0] >= 100 && totals[1] >= 100) {
        return frandmod(2);
    }
    if (totals[0] >= 100) {
        return 0;
    }
    if (totals[1] >= 100) {
        return 1;
    }
    return -1;
}

u16 fn_1_EC0(u16 previous, s16 phase)
{
    u16 button;
    s16 count;
    int i;

    if (phase == 0) {
        count = 6;
    } else {
        count = 4;
    }
    for (i = 0; i < 5; i++) {
        button = frandmod(count);
        if (button != previous) {
            break;
        }
    }
    return button;
}

void fn_1_F44(s16 team)
{
    s16 chars[2];
    int i;
    int player;
    int player2;

    lbl_1_bss_4.winner = team;
    fn_1_33FC(team);
    for (i = 0; i < 2; i++) {
        chars[i] = lbl_1_bss_BC[lbl_1_bss_B4[team][i]].charNo;
    }
    if (team == -1) {
        MgSeqDrawSet();
    } else {
        MgSeqWinnerSet(chars[0], chars[1], -1, -1);
        player = lbl_1_bss_B4[team][0];
        if (!_CheckFlag(FLAG_MG_PRACTICE)) {
            GwPlayer[player].mgCoinBonus = 10;
        }
        player2 = lbl_1_bss_B4[team][1];
        if (!_CheckFlag(FLAG_MG_PRACTICE)) {
            GwPlayer[player2].mgCoinBonus = 10;
        }
    }
    MgSeqModeNext();
}

void fn_1_10A0(void)
{
    int team;
    s16 nightF;

    memset(&lbl_1_bss_4, 0, sizeof(M635Work));
    memset(lbl_1_bss_BC, 0, sizeof(lbl_1_bss_BC));
    lbl_1_bss_4.winner = -1;
    nightF = GwMgNightF;
    lbl_1_bss_4.nightF = nightF;
    for (team = 0; team < 2; team++) {
        lbl_1_bss_4.team[team].unk_0A = 1;
        lbl_1_bss_4.team[team].buttonIndex[1] = frandmod(6);
    }
}

u16 fn_1_1168(s16 team, s16 member)
{
    M635Team *work;
    s16 player;
    s16 difficulty;

    player = lbl_1_bss_B4[team][member];
    difficulty = lbl_1_bss_BC[player].difficulty;
    work = &lbl_1_bss_4.team[team];
    switch (work->unk_0C) {
        case 0:
            if (work->unk_0A != member) {
                return 0;
            }
            if (lbl_1_bss_BC[player].timer-- <= 0) {
                lbl_1_bss_BC[player].timer = fn_1_1308(difficulty);
                return lbl_1_data_0[work->buttonIndex[member]];
            }
            break;
        case 1:
            if (lbl_1_bss_BC[player].timer-- <= 0) {
                lbl_1_bss_BC[player].timer = fn_1_13F8(difficulty);
                return lbl_1_data_0[work->buttonIndex[member]];
            }
            break;
    }
    return 0;
}

s16 fn_1_1308(s16 difficulty)
{
    if (difficulty < 0 || difficulty > 3) {
        return 0;
    }
    return 0.75 * lbl_1_data_12[difficulty][0] + frandmod(lbl_1_data_12[difficulty][0] / 2);
}

s16 fn_1_13F8(s16 difficulty)
{
    if (difficulty < 0 || difficulty > 3) {
        return 0;
    }
    return 0.75 * lbl_1_data_12[difficulty][1] + frandmod(lbl_1_data_12[difficulty][1] / 2);
}

void fn_1_14E8(void)
{
    if (HuPadBtn[0] & PAD_BUTTON_UP) {
        CZoom -= 10.0f;
    }
    if (HuPadBtn[0] & PAD_BUTTON_DOWN) {
        CZoom += 10.0f;
    }
    if (HuPadSubStkX[0] > 20) {
        CRot.y += 1.0f;
    }
    if (HuPadSubStkX[0] < -20) {
        CRot.y -= 1.0f;
    }
    if (HuPadSubStkY[0] > 20) {
        CRot.x += 1.0f;
    }
    if (HuPadSubStkY[0] < -20) {
        CRot.x -= 1.0f;
    }
    if (HuPadStkX[0] > 5 || HuPadStkX[0] < -5) {
        Center.x += HuPadStkX[0] / 2;
    }
    if (HuPadStkY[0] > 5 || HuPadStkY[0] < -5) {
        Center.z -= HuPadStkY[0] / 2;
    }
}
