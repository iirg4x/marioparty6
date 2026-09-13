#include "REL/m651dll.h"
#include "game/audio.h"
#include "game/gamework.h"
#include "game/flag.h"
#include "math.h"

s16 lbl_1_bss_4;
OMOBJMAN *lbl_1_bss_0;
s16 lbl_1_data_0[6] = { -1, -1, -1, -1, 260, 0 };
s32 lbl_1_data_C = -1;
s32 lbl_1_data_10 = -1;
MGSEQ_PARAM lbl_1_data_14 = {
    300, 0,
    fn_1_1B0, fn_1_1D0, fn_1_240, fn_1_244, fn_1_268,
    fn_1_3D4, fn_1_400, fn_1_404, fn_1_408
};

OMOBJMAN *fn_1_A0(void)
{
    return lbl_1_bss_0;
}

void fn_1_B0(s16 playerNo)
{
    if (lbl_1_data_0[playerNo] != -1) {
        lbl_1_data_0[playerNo] = -1;
        lbl_1_bss_4++;
    }
}

void fn_1_10C(s16 playerNo)
{
    OSReport("player no : ( %d ) char no : ( %d )\n", playerNo, GwPlayerConf[playerNo].charNo);
    lbl_1_data_0[playerNo] = GwPlayerConf[playerNo].charNo;
}

void fn_1_190(void)
{
    MgSeqModeNext();
}

void fn_1_1B0(s16 mode, s16 frameNo)
{
    MgSeqModeNext();
}

void fn_1_1D0(s16 mode, s16 frameNo)
{
    if (MgSeqFrameNoGet() == 0) {
        lbl_1_data_10 = HuAudFXPlay(2052);
        lbl_1_data_C = HuAudSStreamPlay(87);
    }
    if (MgSeqFrameNoGet() == 180) {
        HuAudFXFadeOut(lbl_1_data_10, 1000);
    }
}

void fn_1_240(s16 mode, s16 frameNo)
{
}

void fn_1_244(s16 mode, s16 frameNo)
{
    fn_1_4A0();
    fn_1_1E60();
}

void fn_1_268(s16 mode, s16 frameNo)
{
    int i;

    if (MgSeqFrameNoGet() == 0) {
        if (lbl_1_data_C != -1) {
            HuAudSStreamFadeOut(lbl_1_data_C, 100);
            lbl_1_data_C = -1;
        }
        fn_1_62C();
        if (lbl_1_bss_4 == 2) {
            MgSeqDrawSet();
        } else {
            MgSeqWinnerSet(lbl_1_data_0[0], lbl_1_data_0[1], lbl_1_data_0[2], lbl_1_data_0[3]);
            for (i = 0; i < 4; i++) {
                if (lbl_1_data_0[i] != -1) {
                    OSReport("player no : ( %d ) char no : ( %d )\n", i, GwPlayerConf[i].charNo);
                    if (!_CheckFlag(FLAG_MG_PRACTICE)) {
                        GwPlayer[i].mgCoinBonus = 10;
                    }
                }
            }
        }
    }
    OSReport("cnt : ( %d )\n", MgSeqFrameNoGet());
    if (MgSeqFrameNoGet() == 90) {
        fn_1_36F8();
        MgSeqModeNext();
    }
}

void fn_1_3D4(s16 mode, s16 frameNo)
{
    if (MgSeqFrameNoGet() == 1) {
        MgSeqModeNext();
    }
}

void fn_1_400(s16 mode, s16 frameNo)
{
}

void fn_1_404(s16 mode, s16 frameNo)
{
}

void fn_1_408(s16 mode, s16 frameNo)
{
}

void fn_1_40C(void)
{
    lbl_1_bss_0 = omInitObjMan(30, 8192);
    omGameSysInit(lbl_1_bss_0);
    MgSeqCreate(&lbl_1_data_14);
    lbl_1_data_0[0] = -1;
    lbl_1_data_0[1] = -1;
    lbl_1_data_0[2] = -1;
    lbl_1_data_0[3] = -1;
    fn_1_3F30();
}
