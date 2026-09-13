#include "REL/m621dll.h"
#include "game/audio.h"
#include "game/frand.h"
#include "game/gamework.h"

void fn_1_5F0(OMOBJ *obj)
{
    s32 i;

    frandom(OSGetTick());
    lbl_1_bss_8 = _CheckFlag(FLAG_INST_DECA);
    fn_1_A0(10, sizeof(M621Camera), fn_1_B4C);
    lbl_1_bss_4C = fn_1_A0(10, 16, fn_1_D28);
    for (i = 0; i < 4; i++) {
        lbl_1_bss_10[i] = fn_1_A0(20, sizeof(M621Player), fn_1_13EC);
        lbl_1_bss_10[i]->playerNo = i;
    }
    lbl_1_bss_C = 0;
    fn_1_27A0();
    lbl_1_bss_20 = fn_1_A0(10, 84, fn_1_5054);
    fn_1_140(obj, fn_1_70C);
}

void fn_1_70C(OMOBJ *obj)
{
    M621Round *work = obj->data;

    if (MgSeqModeGet() == MGSEQ_MODE_FADEIN) {
        work->sound = fn_1_594(64, 1778);
        work->timer = 0;
        fn_1_140(obj, fn_1_778);
        return;
    }
}

void fn_1_778(OMOBJ *obj)
{
    s16 i;
    M621Round *work = obj->data;
    s16 winnerCount;
    s16 bestScore;
    M621Player *winners[4];

    if (MgSeqModeGet() == MGSEQ_MODE_FADEIN) {
        work->timer++;
        if (work->timer == 220) {
            HuAudFXStop(work->sound);
        }
    }
    if (MgSeqModeGet() == MGSEQ_MODE_PREWIN) {
        if (!lbl_1_bss_8) {
            winnerCount = 0;
            bestScore = 0;
            for (i = 0; i < 4; i++) {
                if (lbl_1_bss_10[i]->score > 0) {
                    if (lbl_1_bss_10[i]->score > bestScore) {
                        winners[0] = lbl_1_bss_10[i];
                        winnerCount = 1;
                        bestScore = lbl_1_bss_10[i]->score;
                    } else if (bestScore == lbl_1_bss_10[i]->score) {
                        winners[winnerCount] = lbl_1_bss_10[i];
                        winnerCount++;
                    }
                }
            }
            for (i = 0; i < winnerCount; i++) {
                winners[i]->winnerF = TRUE;
                fn_1_52D4(winners[i]->playerNo, 10);
            }
            switch (winnerCount) {
                case 0:
                    MgSeqWinnerSet(CHARNO_NONE, CHARNO_NONE, CHARNO_NONE, CHARNO_NONE);
                    break;
                case 1:
                    MgSeqWinnerSet1(winners[0]->player->charNo);
                    break;
                case 2:
                    MgSeqWinnerSet2(winners[0]->player->charNo, winners[1]->player->charNo);
                    break;
                case 3:
                    MgSeqWinnerSet3(winners[0]->player->charNo, winners[1]->player->charNo,
                        winners[2]->player->charNo);
                    break;
                case 4:
                    MgSeqWinnerSet(winners[0]->player->charNo, winners[1]->player->charNo,
                        winners[2]->player->charNo, winners[3]->player->charNo);
                    break;
            }
        }
        work->timer = 0;
        fn_1_140(obj, fn_1_A5C);
        return;
    }
}

void fn_1_A5C(OMOBJ *obj)
{
    M621Round *work = obj->data;
    s32 i;

    work->timer++;
    if ((lbl_1_bss_C >= 4 && work->timer >= 60) || work->timer >= 180) {
        if (lbl_1_bss_8) {
            for (i = 0; i < 4; i++) {
                fn_1_5328(lbl_1_bss_10[i]->playerNo, lbl_1_bss_10[i]->score);
            }
            MgSeqModeSet(MGSEQ_MODE_FADEOUT);
        } else {
            MgSeqModeNext();
        }
        fn_1_140(obj, NULL);
        return;
    }
}
