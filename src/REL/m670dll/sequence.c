#include "REL/m670dll.h"
#include "game/main.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/gamework.h"
#include "game/mg/seqman.h"
#include "game/mic.h"
#include "game/wipe.h"

typedef struct M670MicResponse_s {
    s16 status;
    u16 confidence;
    s16 count;
    s16 unk06;
    s16 *values;
} M670MICRESPONSE;

extern char *lbl_1_data_28[];

static int lbl_1_bss_8;
static int lbl_1_bss_4;
static int lbl_1_bss_0;

void fn_1_A0(u16 *response)
{
    if (((M670MICRESPONSE *)response)->status != 0 || ((M670MICRESPONSE *)response)->count == 0) {
        return;
    }
    if (((M670MICRESPONSE *)response)->confidence >= 3000) {
        lbl_1_data_28[1] = "a";
        if (lbl_1_bss_10.playerState[lbl_1_bss_10.soloPlayer] == 1) {
            switch (*((M670MICRESPONSE *)response)->values) {
            case 0: lbl_1_bss_10.selectedWord = 0; break;
            case 1: lbl_1_bss_10.selectedWord = 1; break;
            case 2: lbl_1_bss_10.selectedWord = 2; break;
            case 3: lbl_1_bss_10.selectedWord = 3; break;
            case 4: lbl_1_bss_10.selectedWord = 4; break;
            case 5: lbl_1_bss_10.selectedWord = 5; break;
            }
            fn_1_2460(lbl_1_bss_10.soloPlayer, 2);
            HuAudFXPlay(2202);
        }
    }
}

void fn_1_1F8(s16 mode, s16 frameNo)
{
    MgSeqModeNext();
}

void fn_1_218(s16 mode, s16 frameNo)
{
    if (frameNo == 0) {
        lbl_1_bss_8 = 0;
        Hu3DCameraMotionStart(lbl_1_bss_10.cameraMotion[0], 1);
        HuMCSelWinItemSet(4522007, 6, lbl_1_bss_10.soloPad);
        fn_1_2A24(1);
    }
    if (frameNo == 50) {
        HuAudFXPlay(2198);
    }
    MgActorExec();
}

int fn_1_2AC(int music, int id)
{
    int result = music;
    if (result == -1 && GameMesFXPlayCheck(MgSeqGameMesIdGet())) {
        result = HuAudBGMPlay(id);
    }
    return result;
}

void fn_1_310(int music)
{
    if (music != -1) {
        HuAudSStreamFadeOut(music, 100);
    }
}

void fn_1_34C(void)
{
    int i, pan;
    HuVecF pos, screen;
    for (i = 0; i < 4; i++) {
        pos = lbl_1_bss_10.players[i]->actor->pos;
        Hu3D3Dto2D(&pos, 1, &screen);
        pan = screen.x;
        pan /= 5;
        if (pan < 32) {
            pan = 32;
        } else if (pan > 96) {
            pan = 96;
        }
        CharModelVoicePanSet(lbl_1_bss_10.characterNo[i], pan);
    }
}

void fn_1_42C(s16 mode, s16 frameNo)
{
    int i;
    if (frameNo == 0) {
        lbl_1_bss_10.selectedWord = -1;
        fn_1_2A24(2);
        for (i = 0; i < 24; i++) {
            fn_1_22F0(i, 100.0f);
        }
    }
    lbl_1_bss_10.music = fn_1_2AC(lbl_1_bss_10.music, 73);
    MgActorExec();
    fn_1_34C();
}

void fn_1_5AC(s16 mode, s16 frameNo)
{
    int i;
    int unk = 1;
    if (frameNo == 0) {
        MgTimerParamSet(lbl_1_bss_10.timer, 3600, 0, 0);
        MgTimerModeOnSet(lbl_1_bss_10.timer, 1);
        MgTimerRecordDispOn(lbl_1_bss_10.timer);
        if (GwPlayerConf[lbl_1_bss_10.soloPlayer].type != 1) {
            HuMCListenerCreate(lbl_1_bss_10.micContext, fn_1_A0, lbl_1_bss_10.soloPad);
        }
        lbl_1_bss_10.remainingPlayers = 0;
        for (i = 0; i < 4; i++) {
            if (lbl_1_bss_10.soloPlayer == i) {
                fn_1_2460(i, 1);
            } else {
                fn_1_2460(i, 5);
                lbl_1_bss_10.remainingPlayers |= 1 << i;
                OSReport("away\n");
            }
            MgPlayerAttrReset(lbl_1_bss_10.players[i], MGPLAYER_ATTR_COMSTK);
        }
        lbl_1_bss_4 = 0;
        lbl_1_bss_0 = 0;
        lbl_1_bss_10.speedLevel = 1;
    }
    fn_1_3BDC();
    {
        int state = lbl_1_bss_10.state;
        MGPLAYER *player;
        HuVecF pos, below;
        MGACTOR_COLMAP_POLY poly;
        for (i = 0; i < 4; i++) {
            if (i != lbl_1_bss_10.soloPlayer) {
                player = lbl_1_bss_10.players[i];
                below = pos = lbl_1_bss_10.players[i]->actor->pos;
                below.y -= 350.0f;
                if ((state == 0 || state == 2 || MgActorColMapPolyGet(&pos, &below, 255, &poly))
                    && pos.z >= -400.0f) {
                    Hu3DModelShadowSet(player->actor->mdlId);
                } else {
                    Hu3DModelShadowReset(player->actor->mdlId);
                }
            }
        }
    }
    if (MgTimerDoneCheck(lbl_1_bss_10.timer)) {
        lbl_1_bss_8++;
    }
    if (lbl_1_bss_4 == 0 && MgTimerDoneCheck(lbl_1_bss_10.timer)) {
        if (GwPlayerConf[lbl_1_bss_10.soloPlayer].type != 1) {
            HuMCListenerKill();
        }
        for (i = 0; i < 4; i++) {
            if (lbl_1_bss_10.group[i] == 0) {
                MgPlayerAttrSet(lbl_1_bss_10.players[i], MGPLAYER_ATTR_COMSTK);
            }
        }
        if (lbl_1_bss_10.selectedWord == -1) {
            fn_1_2460(lbl_1_bss_10.soloPlayer, 1);
        }
        lbl_1_bss_4 = 1;
    }
    if (lbl_1_bss_8 >= 60) {
        for (i = 0; i < 4; i++) {
            MgPlayerAttrSet(lbl_1_bss_10.players[i], MGPLAYER_ATTR_COMSTK);
        }
    }
    if (lbl_1_bss_8 >= 60 || lbl_1_bss_10.remainingPlayers == 0) {
        if (lbl_1_bss_4 == 0) {
            if (!MgTimerDoneCheck(lbl_1_bss_10.timer)) {
                MgTimerRecordDispOff(lbl_1_bss_10.timer);
            }
            for (i = 0; i < 4; i++) {
                MgPlayerAttrSet(lbl_1_bss_10.players[i], MGPLAYER_ATTR_COMSTK);
            }
            if (lbl_1_bss_4 == 0 && GwPlayerConf[lbl_1_bss_10.soloPlayer].type != 1) {
                HuMCListenerKill();
            }
        }
        MgSeqModeNext();
    }
    MgActorExec();
    fn_1_34C();
}

void fn_1_B4C(s16 mode, s16 frameNo)
{
    if (frameNo == 0) {
        fn_1_310(lbl_1_bss_10.music);
    }
    MgActorExec();
    fn_1_34C();
}

void fn_1_C68(s16 mode, s16 frameNo)
{
    int i, survivorCount, slot;
    MGPLAYER *player, *shadowPlayer;
    char **names;
    if (frameNo == 0) {
        survivorCount = 0;
        {
            s16 winners[4] = {-1, -1, -1, -1};
            if (lbl_1_bss_10.remainingPlayers == 0) {
                winners[0] = lbl_1_bss_10.soloCharacter;
                GWMgCoinBonusSet(lbl_1_bss_10.soloPlayer, 10);
            } else {
                int count = 0;
                for (i = 0; i < 4; i++) {
                    if (i != lbl_1_bss_10.soloPlayer) {
                        winners[count++] = lbl_1_bss_10.characterNo[i];
                        GWMgCoinBonusSet(i, 10);
                        if (lbl_1_bss_10.remainingPlayers & (1 << i)) {
                            survivorCount++;
                        }
                    }
                }
            }
            MgSeqWinnerSet(winners[0], winners[1], winners[2], winners[3]);
            WipeCreate(2, 0, 60);
            for (i = 0; i < 60; i++) {
                MgActorExec();
                fn_1_34C();
                HuPrcVSleep();
            }
            fn_1_2A24(0);
            for (i = 0; i < 24; i++) {
                Hu3DMotionSet(lbl_1_bss_10.pillarModel[i], lbl_1_bss_10.pillarMotion[i][1]);
                fn_1_22F0(i, 100.0f);
            }
            for (i = 0; i < 4; i++) {
                MgPlayerDespawn(lbl_1_bss_10.players[i]);
            }
            for (i = 0; i < 4; i++) {
                if (i != lbl_1_bss_10.soloPlayer) {
                    shadowPlayer = lbl_1_bss_10.players[i];
                    Hu3DModelShadowSet(shadowPlayer->actor->mdlId);
                }
            }
            if (lbl_1_bss_10.remainingPlayers != 0) {
                char *one[1] = {"T02"};
                char *two[2] = {"T04", "T05"};
                char *three[3] = {"T01", "T02", "T03"};
                HuVecF resultPos;
                switch (survivorCount) {
                case 1: names = one; break;
                case 2: names = two; break;
                case 3: names = three; break;
                }
                slot = 0;
                for (i = 0; i < 4; i++) {
                    if (lbl_1_bss_10.remainingPlayers & (1 << i)) {
                        player = lbl_1_bss_10.players[i];
                        Hu3DModelObjPosGet(lbl_1_bss_10.winnerModel, names[slot], &resultPos);
                        OSReport("pos ... %f, %f, %f\n", resultPos.x, resultPos.y, resultPos.z);
                        Hu3DModelAttrSet(player->actor->mdlId, HU3D_MOTATTR_LOOP | HU3D_ATTR_DISPOFF);
                        CharMotionSet(player->charNo, player->omObj->mtnId[0]);
                        Hu3DModelPosSetV(player->actor->mdlId, &resultPos);
                        Hu3DModelRotSet(player->actor->mdlId, 0.0f, 0.0f, 0.0f);
                        slot++;
                    }
                }
            }
            for (i = 0; i < 4; i++) {
                MGPLAYER *motionPlayer = lbl_1_bss_10.players[i];
                CharMotionSet(lbl_1_bss_10.characterNo[i], motionPlayer->omObj->mtnId[0]);
            }
            if (lbl_1_bss_10.remainingPlayers == 0) {
                Hu3DCameraMotionStart(lbl_1_bss_10.cameraMotion[1], 1);
            } else {
                Hu3DCameraMotionStart(lbl_1_bss_10.cameraMotion[2], 1);
            }
            WipeCreate(1, 5, 60);
            for (i = 0; i < 60; i++) {
                MgActorExec();
                fn_1_34C();
                HuPrcVSleep();
            }
            if (lbl_1_bss_10.remainingPlayers != 0) {
                fn_1_2460(lbl_1_bss_10.soloPlayer, 4);
            }
            MgSeqModeNext();
        }
    }
}

void fn_1_12C4(s16 mode, s16 frameNo)
{
    int i, motion;
    MGPLAYER *player;
    if (frameNo == 0) {
        for (i = 0; i < 4; i++) {
            player = lbl_1_bss_10.players[i];
            if (lbl_1_bss_10.group[i] == 0) {
                if (lbl_1_bss_10.remainingPlayers == 0) {
                    CharMotionShiftSet(lbl_1_bss_10.characterNo[i], player->omObj->mtnId[5], 0.0f, 6.0f, 0);
                }
            } else if (lbl_1_bss_10.remainingPlayers & (1 << i)) {
                if (lbl_1_bss_10.remainingPlayers == 0) {
                    motion = 6;
                } else {
                    motion = 5;
                }
                CharMotionShiftSet(lbl_1_bss_10.characterNo[i], player->omObj->mtnId[motion], 0.0f, 6.0f, 0);
            }
        }
    }
}

void fn_1_1428(s16 mode, s16 frameNo)
{
}

void fn_1_142C(s16 mode, s16 frameNo)
{
    if (frameNo == 0) {
        HuMCResponseGet();
        HuMCClose();
    }
}
