#include "REL/m640/m640.h"

void fn_1_A0(void);
void fn_1_F0(s16 mode, s16 frame);
void fn_1_114(s16 mode, s16 frame);
void fn_1_13C(s16 mode, s16 frame);
void fn_1_170(s16 mode, s16 frame);
void fn_1_1B8(s16 mode, s16 frame);
void fn_1_220(s16 mode, s16 frame);
void fn_1_240(s16 mode, s16 frame);
void fn_1_268(s16 mode, s16 frame);
void fn_1_26C(s16 mode, s16 frame);
void fn_1_270(s16 frame);
void fn_1_60C(void);
void fn_1_868(s16 frame);
void fn_1_86C(void);
void fn_1_8D4(void);
s32 fn_1_BEC(void);
s16 fn_1_EC8(void);
s16 fn_1_112C(s16 frame);
void fn_1_15C8(void);
s32 fn_1_1C48(s32 frame);
void fn_1_26B8(s16 player, s16 motion);
void fn_1_2820(void);
s16 fn_1_2FF8(void);
void fn_1_3090(void);
s16 fn_1_3274(M640Player *player, HuVecF *rot);
s16 fn_1_33D0(float angle);
void fn_1_345C(s16 side, s16 slot, s16 index);
void fn_1_351C(s16 side, s16 slot);
void fn_1_355C(void);
void fn_1_36D8(M640Player *player);
s16 fn_1_37F4(s16 side, M640Player *player);
u16 fn_1_39F0(s16 side, M640Player *player);
void fn_1_3C24(s16 side, s16 slot);
void fn_1_4C90(s16 side);
void fn_1_4CD8(void);
void fn_1_4DDC(s16 side, s16 part);
void fn_1_4FFC(s16 side, s16 part);
void fn_1_519C(s16 side, s16 part);
void fn_1_52D0(OMOBJ *obj);
void fn_1_5A28(void);
void fn_1_5DF0(void);
void fn_1_60AC(OMOBJ *obj);
void fn_1_623C(void);

MGSEQ_PARAM lbl_1_data_0 = {
    300,
    0,
    fn_1_F0,
    fn_1_114,
    fn_1_13C,
    fn_1_170,
    fn_1_1B8,
    fn_1_220,
    fn_1_240,
    fn_1_268,
    fn_1_26C,
};
HUPROCESS *lbl_1_bss_4;
s32 lbl_1_bss_0;

void fn_1_A0(void)
{
    lbl_1_bss_4 = omInitObjMan(256, 8192);
    omGameSysInit(lbl_1_bss_4);
    MgSeqCreate(&lbl_1_data_0);
}

void fn_1_F0(s16 mode, s16 frame)
{
    fn_1_86C();
    MgSeqModeNext();
}

void fn_1_114(s16 mode, s16 frame)
{
    fn_1_270(frame);
}

void fn_1_13C(s16 mode, s16 frame)
{
    if (frame == 0) {
        HuAudBGMPlay(83);
    }
}

void fn_1_170(s16 mode, s16 frame)
{
    if (frame == 0) {
        fn_1_351C(0, 0);
        fn_1_351C(1, 0);
    }
    fn_1_4CD8();
}

void fn_1_1B8(s16 mode, s16 frame)
{
    lbl_1_bss_4E8.state = 0;
    lbl_1_bss_4E8.frame = 0;
    if (frame == 0) {
        fn_1_3090();
        HuAudSStreamFadeOut(lbl_1_bss_4E8.stream, 100);
    }
    fn_1_355C();
}

void fn_1_220(s16 mode, s16 frame)
{
    fn_1_60C();
}

void fn_1_240(s16 mode, s16 frame)
{
    fn_1_868(frame);
}

void fn_1_268(s16 mode, s16 frame)
{
}

void fn_1_26C(s16 mode, s16 frame)
{
}

void fn_1_270(s16 frame)
{
    OM_CAMERA_VIEW view;
    s16 times[] = { 40, 80, 120, 160 };
    u16 cameras[] = { 1, 2 };
    int side, part;

    if (frame == 30) {
        lbl_1_bss_0 = HuAudFXPlay(1919);
    }
    switch (lbl_1_bss_4E8.state) {
    case 0:
        MgSeqModeChangeOff();
        lbl_1_bss_4E8.state++;
        break;
    case 1:
        if (lbl_1_bss_4E8.frame++ >= 200) {
            lbl_1_bss_4E8.frame = 0;
            lbl_1_bss_4E8.state++;
        }
        for (side = 0; side < 2; side++) {
            for (part = 0; part < 4; part++) {
                if (lbl_1_bss_4E8.frame == times[part]) {
                    fn_1_519C(side, part);
                    view.center.x = side * 1300;
                    view.center.y = 150.0f + part * 10;
                    view.center.z = 0.0f;
                    view.rot.x = (-5.0f - 2.0f * side) + 0.5f * part;
                    view.rot.y = 0.0f;
                    view.rot.z = 0.0f;
                    view.zoom = 1400.0f;
                    omCameraViewMoveSimpleMulti(cameras[side], &view, 40);
                }
            }
        }
        break;
    case 2:
        if (fn_1_112C(lbl_1_bss_4E8.frame++)) {
            HuAudFXPlay(1908);
            lbl_1_bss_4E8.state++;
        }
        break;
    case 3:
        if (fn_1_BEC()) {
            HuAudFXFadeOut(lbl_1_bss_0, 500);
            lbl_1_bss_4E8.state++;
        }
        break;
    case 4:
        if (fn_1_EC8()) {
            lbl_1_bss_4E8.state++;
        }
        break;
    case 5:
        MgSeqModeNext();
        break;
    }
}

void fn_1_60C(void)
{
    int i;
    int player1, player2;
    switch (lbl_1_bss_4E8.state) {
    case 0:
        MgSeqModeChangeOff();
        lbl_1_bss_4E8.state++;
        break;
    case 1:
        if (lbl_1_bss_4E8.finishCount == 0 || lbl_1_bss_4E8.finishCount == 2) {
            MgSeqWinnerSet(-1, -1, -1, -1);
            for (i = 0; i < 4; i++) {
                fn_1_26B8(i, 3);
            }
            MgSeqModeNext();
            return;
        }
        MgSeqWinnerSet(lbl_1_bss_4FC[lbl_1_bss_4E8.winnerSide].players[0]->charNo,
                      lbl_1_bss_4FC[lbl_1_bss_4E8.winnerSide].players[1]->charNo, -1, -1);
        player1 = lbl_1_bss_4FC[lbl_1_bss_4E8.winnerSide].players[0]->playerNo;
        if (!_CheckFlag(65551)) {
            GwPlayer[player1].mgCoinBonus = 10;
        }
        player2 = lbl_1_bss_4FC[lbl_1_bss_4E8.winnerSide].players[1]->playerNo;
        if (!_CheckFlag(65551)) {
            GwPlayer[player2].mgCoinBonus = 10;
        }
        lbl_1_bss_4E8.frame = 0;
        lbl_1_bss_4E8.state++;
        break;
    case 2:
        if (fn_1_1C48(lbl_1_bss_4E8.frame++)) {
            lbl_1_bss_4E8.state++;
        }
        break;
    case 3:
        MgSeqModeNext();
        break;
    }
}

void fn_1_868(s16 frame)
{
}
