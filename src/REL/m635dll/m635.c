#include "REL/m635dll.h"
#include "game/audio.h"
#include "game/wipe.h"

MGSEQ_PARAM lbl_1_data_78 = {
    300, 0, fn_1_F0, fn_1_134, fn_1_164, fn_1_1AC, fn_1_224,
    fn_1_2D0, fn_1_3D8, fn_1_3DC, fn_1_3E0
};

void fn_1_A0(void)
{
    lbl_1_bss_64 = omInitObjMan(256, 8192);
    omGameSysInit(lbl_1_bss_64);
    MgSeqCreate(&lbl_1_data_78);
}

void fn_1_F0(s16 mode, s16 frameNo)
{
    fn_1_10A0();
    fn_1_1774(lbl_1_bss_64);
    fn_1_195C();
    fn_1_2954();
    fn_1_4114();
    fn_1_2018();
    MgSeqModeNext();
}

void fn_1_134(s16 mode, s16 frameNo)
{
    fn_1_408(frameNo);
    fn_1_2CE8();
    fn_1_3C34();
}

void fn_1_164(s16 mode, s16 frameNo)
{
    if (frameNo == 0) {
        lbl_1_bss_4.music = HuAudBGMPlay(78);
    }
    fn_1_2CE8();
    fn_1_3C34();
}

void fn_1_1AC(s16 mode, s16 frameNo)
{
    int team;

    if (frameNo == 0) {
        for (team = 0; team < 2; team++) {
            fn_1_25EC(team, 1, 1);
        }
    }
    fn_1_954(frameNo);
    fn_1_2CE8();
    fn_1_3C34();
    fn_1_42A0();
    fn_1_2330();
}

void fn_1_224(s16 mode, s16 frameNo)
{
    int team;
    int player;

    if (frameNo == 0) {
        for (team = 0; team < 2; team++) {
            for (player = 0; player < 2; player++) {
                fn_1_25EC(team, player, 5);
            }
        }
        if (lbl_1_bss_4.winner == -1) {
            MgSeqDrawSet();
        }
        HuAudSStreamFadeOut(lbl_1_bss_4.music, 100);
    }
    fn_1_2CE8();
    fn_1_3C34();
    fn_1_42A0();
}

void fn_1_2D0(s16 mode, s16 frameNo)
{
    switch (lbl_1_bss_4.state) {
        case 0:
            MgSeqModeChangeOff();
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
            lbl_1_bss_4.state++;
            break;
        case 1:
            if (!WipeCheck()) {
                fn_1_638();
                fn_1_448C();
                lbl_1_bss_4.state++;
            }
            break;
        case 2:
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREV, 60);
            lbl_1_bss_4.state++;
            break;
        case 3:
            if (!WipeCheck()) {
                lbl_1_bss_4.state = 0;
                fn_1_8B0();
                MgSeqModeNext();
            }
            break;
    }
    fn_1_2CE8();
    fn_1_42A0();
    fn_1_3C34();
}

void fn_1_3D8(s16 mode, s16 frameNo) {}
void fn_1_3DC(s16 mode, s16 frameNo) {}

void fn_1_3E0(s16 mode, s16 frameNo)
{
    fn_1_2014();
    fn_1_33F8();
    fn_1_2904();
}
