#include "REL/m633dll.h"

MGSEQ_PARAM lbl_1_data_0 = {
    0,
    0,
    fn_1_224,
    fn_1_310,
    fn_1_55C,
    fn_1_690,
    fn_1_AB0,
    fn_1_C78,
    fn_1_118C,
    fn_1_1284,
    fn_1_1288,
};

char lbl_1_data_28[25] = "633biribiri-EDpattern1P1";

char lbl_1_data_41[25] = "633biribiri-EDpattern2P1";

char lbl_1_data_5A[25] = "633biribiri-EDpattern2P2";

char lbl_1_data_73[25] = "633biribiri-EDpattern3P1";

char lbl_1_data_8C[25] = "633biribiri-EDpattern3P2";

char lbl_1_data_A5[27] = "633biribiri-EDpattern3P3";

char lbl_1_data_C0[] = "";

char lbl_1_data_C1[] = "633biribiri-P2st";

char lbl_1_data_D2[] = "633biribiri-P3st";

char lbl_1_data_E3[] = "633biribiri-P4st";

char *lbl_1_data_F4[4] = { lbl_1_data_C0, lbl_1_data_C1, lbl_1_data_D2, lbl_1_data_E3 };

char lbl_1_data_104[27] = "633biribiri-cannonA_shadow";

char lbl_1_data_11F[23] = "633biribiri-nozzleNull";

char lbl_1_data_136[25] = "633biribiri-stage_shadow";

char lbl_1_data_14F[17] = "633biribiri-P1st";

char lbl_1_data_160[23] = "633biribiri-nozzleNull";

char lbl_1_data_177[17] = "hit!!!!!!!!(%f)\n";

char lbl_1_data_188[24] = "633biribiri-nozzleNull";

char lbl_1_data_1A0[24] = "hit!!!!!!!!(%f)\n";

s32 lbl_1_data_1B8[6] = { 6029324, 6029325, 6029326, 6029327, 6029332, 6029333 };

s32 lbl_1_data_1D0[4] = { 6029337, 6029339, 6029338, 6029336 };

unsigned int lbl_1_data_1E0[16] = {
    9633792,
    9633793,
    9633794,
    9633795,
    9633796,
    9633798,
    9633832,
    9633827,
    9633802,
    9633801,
    9633822,
    9633812,
    9633853,
    9306198,
    9633816,
    0,
};

unsigned int lbl_1_data_220[16] = {
    9633792,
    9633793,
    9633794,
    9633795,
    9633796,
    9633830,
    9633832,
    9633827,
    9633802,
    9633801,
    9633822,
    9633812,
    9633853,
    9306198,
    9633816,
    0,
};

u32 lbl_1_data_260[4] = { 100U, 70U, 30U, 0U };

u32 lbl_1_data_270[4] = { 20U, 45U, 55U, 100U };

u32 lbl_1_data_280[4] = { 15U, 25U, 40U, 100U };

M633Work lbl_1_bss_0;

s32 fn_1_A0(s32 arg0, s32 arg1)
{
    s32 result;

    result = arg0;
    if (result == -1 && (GameMesStatGet(MgSeqGameMesIdGet()) & 16) != 0) {
        result = HuAudBGMPlay((s16) arg1);
    }
    return result;
}

void fn_1_104(s32 arg0)
{
    if (arg0 != -1) {
        HuAudSStreamFadeOut(arg0, 100);
    }
}

void fn_1_140(void)
{
    Point3d pos;
    Point3d projected;
    s32 pan;
    s32 player;

    for (player = 0; player < 4; player++) {
        pos = lbl_1_bss_0.unk040[player]->actor->pos;
        Hu3D3Dto2D(&pos, 1, &projected);
        pan = (s32) projected.x;
        pan /= 5;
        if (pan < 32) {
            pan = 32;
        } else if (pan > 96) {
            pan = 96;
        }
        CharModelVoicePanSet((s16) lbl_1_bss_0.unk050[player], (s16) pan);
    }
}

void fn_1_224(s16 mode, s16 frameNo)
{
    MgActorExec();
    fn_1_140();
    MgSeqModeNext();
}

void fn_1_310(s16 arg1, s16 frameNo)
{
    s32 var_r28;
    s32 var_r31;

    var_r28 = 0;
    fn_1_6DE8();
    lbl_1_bss_0.unk198 -= 1;
    if (lbl_1_bss_0.unkAD4 != 0) {
        HuAudFXPlay(1850);
    }
    lbl_1_bss_0.unkAD4 = 0;
    MgActorExec();
    fn_1_140();
    lbl_1_bss_0.unkADC = 1;
    if (frameNo == 0) {
        fn_1_7E70(0);
        var_r31 = 0;
        while (var_r31 < 4) {
            if ((s32) lbl_1_bss_0.unk070[var_r31] != 0) {
                (lbl_1_bss_0.unk0E4[var_r31])->objFunc = fn_1_32C0;
            } else {
                (lbl_1_bss_0.unk0E4[var_r31])->objFunc = fn_1_26B8;
            }
            var_r31 += 1;
        }
        return;
    }
    if (fn_1_7F40() != 0) {
        var_r31 = 0;
        while (var_r31 < 4) {
            if (((s32) lbl_1_bss_0.unk070[var_r31] == 0) && ((u32) (lbl_1_bss_0.unk0E4[var_r31])->work[1] == 4U)) {
                var_r28 += 1;
            }
            var_r31 += 1;
        }
        if (var_r28 == 3) {
            MgSeqModeNext();
        }
    }
}

void fn_1_55C(s16 mode, s16 frameNo)
{
    lbl_1_bss_0.unkAE0 = fn_1_A0(lbl_1_bss_0.unkAE0, 75);
    MgActorExec();
    fn_1_140();
}

void fn_1_690(s16 arg1, s16 frameNo)
{
    s16 model;
    Point3d sp24;
    MGPLAYER *temp_r30;
    s32 var_r31;

    if (frameNo == 0) {
        fn_1_7E70(1);
        fn_1_70A8(0);
        var_r31 = 0;
        while (var_r31 < 4) {
            lbl_1_bss_0.unk080[var_r31] = 0;
            if ((s32) lbl_1_bss_0.unk070[var_r31] == 0) {
                (lbl_1_bss_0.unk0E4[var_r31])->objFunc = fn_1_28A4;
            } else {
                model = (s16) lbl_1_bss_0.unk040[var_r31]->actor->mdlId;
                Hu3DModelPosGet(model, &sp24);
                MgPlayerSpawn(lbl_1_bss_0.unk040[var_r31], &sp24);
                (lbl_1_bss_0.unk0E4[var_r31])->objFunc = fn_1_34AC;
                CharMotionShiftSet((lbl_1_bss_0.unk040[var_r31])->charNo, *((lbl_1_bss_0.unk040[var_r31])->omObj)->mtnId, 0.0f, 6.0f, 0U);
            }
            var_r31 += 1;
        }
        MgTimerParamSet(lbl_1_bss_0.unk02C, 1800, 0, 0);
        MgTimerModeOnSet(lbl_1_bss_0.unk02C, 1);
        MgTimerRecordDispOn(lbl_1_bss_0.unk02C);
        lbl_1_bss_0.unkAD4 = 0;
        lbl_1_bss_0.unkAD8 = 0;
    }
    lbl_1_bss_0.unkADC = 0;
    var_r31 = 0;
    while (var_r31 < 4) {
        fn_1_3830(var_r31);
        var_r31 += 1;
    }
    if (lbl_1_bss_0.unkAD4 != 0) {
        HuAudFXPlay(1850);
    }
    MgActorExec();
    fn_1_140();
    fn_1_6DE8();
    lbl_1_bss_0.unk198 -= 1;
    lbl_1_bss_0.unkAD4 = 0;
    if ((lbl_1_bss_0.unk0E0 == 0) || (MgTimerDoneCheck(lbl_1_bss_0.unk02C) != 0)) {
        var_r31 = 0;
        while (var_r31 < 4) {
            temp_r30 = lbl_1_bss_0.unk040[var_r31];
            MgPlayerDespawn(temp_r30);
            if ((s32) lbl_1_bss_0.unk070[var_r31] == 0) {
                CharMotionShiftSet(temp_r30->charNo, *temp_r30->omObj->mtnId, 0.0f, 6.0f, 0U);
                (lbl_1_bss_0.unk0E4[var_r31])->objFunc = fn_1_26B4;
            } else if ((s32) lbl_1_bss_0.unk080[var_r31] == 0) {
                CharMotionShiftSet(temp_r30->charNo, *temp_r30->omObj->mtnId, 0.0f, 6.0f, 0U);
            }
            var_r31 += 1;
        }
        MgSeqModeNext();
    }
}

void fn_1_AB0(s16 arg1, s16 frameNo)
{

    lbl_1_bss_0.unkADC = 1;
    MgActorExec();
    fn_1_140();
    if (frameNo == 0) {
        fn_1_104(lbl_1_bss_0.unkAE0);
        if (lbl_1_bss_0.unkAE4 != -1) {
            HuAudFXStop(lbl_1_bss_0.unkAE4);
            lbl_1_bss_0.unkAE4 = -1;
        }
    }
    if (MgTimerDoneCheck(lbl_1_bss_0.unk02C) == 0) {
        MgTimerRecordDispOff(lbl_1_bss_0.unk02C);
    }
    if (lbl_1_bss_0.unk198 > 0) {
        lbl_1_bss_0.unk198 = 0;
        return;
    }
    lbl_1_bss_0.unk198 -= 1;
}

void fn_1_C78(s16 mode, s16 frameNo)
{
    s16 winners[4] = { -1, -1, -1, -1 };
    char *oneResult[1] = { lbl_1_data_28 };
    char *twoResults[2] = {
        lbl_1_data_41,
        lbl_1_data_5A,
    };
    char *threeResults[3] = {
        lbl_1_data_73,
        lbl_1_data_8C,
        lbl_1_data_A5,
    };
    s16 playerBonus[4] = { 0, 0, 0, 0 };
    char **results;
    Point3d pos;
    MGPLAYER *player;
    s16 coinBonus;
    s32 resultCount;
    s32 playerNo;

    if (frameNo == 0) {
        WipeCreate(2, 0, 60);
        for (playerNo = 0; playerNo < 60; playerNo++) {
            HuPrcVSleep();
        }

        switch (lbl_1_bss_0.unk0E0) {
        case 1:
            results = oneResult;
            break;
        case 2:
            results = twoResults;
            break;
        case 3:
            results = threeResults;
            break;
        }

        resultCount = 0;
        for (playerNo = 0; playerNo < 4; playerNo++) {
            player = lbl_1_bss_0.unk040[playerNo];
            if (lbl_1_bss_0.unk070[playerNo] == 0) {
                (lbl_1_bss_0.unk0E4[playerNo])->objFunc = fn_1_26B4;
                Hu3DModelRotSet(lbl_1_bss_0.unk110, 0.0f, 0.0f, 0.0f);
                Hu3DModelRotSet(lbl_1_bss_0.unk112, 0.0f, 0.0f, 0.0f);
                Hu3DMotionSet(lbl_1_bss_0.unk110, lbl_1_bss_0.unk114);
            } else if (lbl_1_bss_0.unk080[playerNo] == 0) {
                Hu3DModelObjPosGet(lbl_1_bss_0.unk03E, results[resultCount], &pos);
                Hu3DModelPosSetV((s16)player->actor->mdlId, &pos);
                Hu3DModelAttrReset((s16)player->actor->mdlId, 1U);
                CharMotionSet((s16)lbl_1_bss_0.unk050[playerNo],
                    *((player)->omObj)->mtnId);
                Hu3DModelRotSet((s16)player->actor->mdlId, 0.0f, 0.0f, 0.0f);
                resultCount++;
            }
        }

        Hu3DModelAttrSet(lbl_1_bss_0.unk180[0], 1U);
        Hu3DModelAttrSet(lbl_1_bss_0.unk180[1], 1U);
        fn_1_70A8(3);
        if (lbl_1_bss_0.unk0E0 == 0) {
            fn_1_7E70(2);
        } else {
            fn_1_7E70(3);
        }

        WipeCreate(1, 5, 60);
        for (playerNo = 0; playerNo < 60; playerNo++) {
            HuPrcVSleep();
        }

        for (playerNo = 0; playerNo < 4; playerNo++) {
            lbl_1_bss_0.unk0F8[playerNo] = 0;
        }

        if (lbl_1_bss_0.unk0E0 == 0) {
            for (playerNo = 0; playerNo < 4; playerNo++) {
                if (lbl_1_bss_0.unk070[playerNo] == 0) {
                    winners[0] = (s16)lbl_1_bss_0.unk050[playerNo];
                    lbl_1_bss_0.unk0F8[playerNo] = 1;
                    playerBonus[playerNo] = 10;
                    break;
                }
            }
        } else {
            resultCount = 0;
            for (playerNo = 0; playerNo < 4; playerNo++) {
                if (lbl_1_bss_0.unk070[playerNo] != 0) {
                    winners[resultCount++] = (s16)lbl_1_bss_0.unk050[playerNo];
                    lbl_1_bss_0.unk0F8[playerNo] = 1;
                    playerBonus[playerNo] = 10;
                }
            }
        }

        MgSeqWinnerSet(winners[0], winners[1], winners[2], winners[3]);
        for (playerNo = 0; playerNo < 4; playerNo++) {
            coinBonus = playerBonus[playerNo];
            if (_CheckFlag(65551U) == 0) {
                GwPlayer[playerNo].mgCoinBonus = coinBonus;
            }
        }
        MgSeqModeNext();
    }
}

void fn_1_118C(s16 mode, s16 frameNo)
{
    s32 playerNo;
    MGPLAYER *player;
    s32 motion;

    if (frameNo == 0) {
        for (playerNo = 0; playerNo < 4; playerNo++) {
            player = lbl_1_bss_0.unk040[playerNo];
            if (lbl_1_bss_0.unk080[playerNo] == 0) {
                if (lbl_1_bss_0.unk0F8[playerNo] == 1) {
                    motion = 5;
                } else {
                    motion = 6;
                }
                CharMotionShiftSet((s16)lbl_1_bss_0.unk050[playerNo],
                    player->omObj->mtnId[motion], 0.0f, 6.0f, 0);
            }
        }
    }
}

void fn_1_1284(s16 mode, s16 frameNo)
{

}

void fn_1_1288(s16 mode, s16 frameNo)
{

}
