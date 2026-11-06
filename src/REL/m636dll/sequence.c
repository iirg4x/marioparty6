/* Pixel Perfect opening, timed picture rounds and team-result sequence. */
#include "math.h"
#include "REL/m636dll.h"





#define PIXEL_RESULT_COLLISION_ATTRS \
    ((1 << 24) | (1 << 2) | 0x8)

/* Byte positions of the scene state read by the sequence hooks. */
#define PIXEL_CHARACTER_IDS_OFFSET 72
#define PIXEL_PLAYER_SIDE_OFFSET 104
#define PIXEL_PLAYER_RECORD_OFFSET 120
#define PIXEL_LANDING_COUNTER_OFFSET 136
#define PIXEL_CPU_MODE_OFFSET 156
#define PIXEL_CPU_FRAME_COUNTER_OFFSET 172
#define PIXEL_CPU_TARGET_CELL_OFFSET 188
#define PIXEL_PICTURE_INTRO_MODELS_OFFSET 274
#define PIXEL_PICTURE_RESULT_MODELS_OFFSET 280
#define PIXEL_PICTURE_FINISH_MODELS_OFFSET 382
#define PIXEL_FINISH_MODEL_OFFSET 484
#define PIXEL_GAME_FINISH_MOTION_OFFSET 488
#define PIXEL_SELECTED_PICTURE_OFFSET 504
#define PIXEL_ROUND_CELL_CHOICES_OFFSET 508
#define PIXEL_MUSIC_HANDLE_OFFSET 628

/* Player/actor byte positions used by the opening voice-pan update. */
#define PIXEL_PLAYER_ACTOR_OFFSET 60
#define PIXEL_ACTOR_POSITION_OFFSET 80
#define PIXEL_VOICE_PAN_MIN 32
#define PIXEL_VOICE_PAN_MAX 96

/* Hooks run in sequence order from initialization through scene close. */
MGSEQ_PARAM lbl_1_data_0 = {
    0, 0, fn_1_2D0, fn_1_2F4, fn_1_494, fn_1_5F0, fn_1_12E0, fn_1_151C, fn_1_1C74,
    fn_1_1F58, fn_1_1F78
};

/* Unused music helper starts a track at the sequence message audio cue, or keeps its live
 * handle. */
s32 fn_1_A0(s32 musicHandle, s32 musicId)
{
    s32 playbackHandle;

    playbackHandle = musicHandle;
    if (playbackHandle == -1 && (GameMesStatGet(fn_80071DF4()) & GAMEMES_STAT_FXPLAY) != 0) {
        playbackHandle = HuAudBGMPlay((s16) musicId);
    }
    return playbackHandle;
}

/* Unused music helper fades a live track with the fixed fade setting when asked to stop it. */
void fn_1_104(s32 musicHandle)
{
    if (musicHandle != -1) {
        HuAudSStreamFadeOut(musicHandle, 100);
    }
}

/* Unused sound helper pans a supplied world position through the first camera and logs the screen
 * position. */
void fn_1_140(s32 soundHandle, Point3d *soundPosition)
{
    Point3d screenPosition;
    s32 soundPan;

    Hu3D3Dto2D(soundPosition, 1, &screenPosition);
    soundPan = (s32)screenPosition.x;
    soundPan /= 5;
    if (soundPan < PIXEL_VOICE_PAN_MIN) {
        soundPan = PIXEL_VOICE_PAN_MIN;
    } else if (soundPan > PIXEL_VOICE_PAN_MAX) {
        soundPan = PIXEL_VOICE_PAN_MAX;
    }
    HuAudFXPanning(soundHandle, (s16)soundPan);
    OSReport("pan ... %d ( %f, %f, %f )\n", soundPan, screenPosition.x, screenPosition.y,
             screenPosition.z);
}

/* Opening, play and result updates pan all character voices from their current screen positions. */
void fn_1_1EC(void)
{
    Point3d playerPosition;
    Point3d screenPosition;
    s32 voicePan;
    s32 playerIndex;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        playerPosition = lbl_1_bss_0.players[playerIndex]->actor->pos;
        Hu3D3Dto2D(&playerPosition, 1, &screenPosition);
        voicePan = (s32)screenPosition.x;
        voicePan /= 5;
        if (voicePan < PIXEL_VOICE_PAN_MIN) {
            voicePan = PIXEL_VOICE_PAN_MIN;
        } else if (voicePan > PIXEL_VOICE_PAN_MAX) {
            voicePan = PIXEL_VOICE_PAN_MAX;
        }
        fn_80060F14((s16)lbl_1_bss_0.charNo[playerIndex],
                             (s16) voicePan);
    }
}

/* The initialization hook updates players once and immediately requests the opening phase. */
void fn_1_2D0(s16 mode, s16 frameNo)
{
    fn_800856A8();
    fn_80071A9C();
}

/* The fade-in hook plays the chosen picture introduction and switches cameras when its motion
 * ends. */
void fn_1_2F4(s16 mode, s16 frameNo) {
    s16 introModel;
    s32 playerIndex;
    s32 voicePan;
    Point3d screenPosition;
    Point3d playerPosition;

    if (frameNo == 0) {
        /* Seed the floor target from picture 0, cell 0, regardless of the chosen introduction
         * picture. */
        fn_1_3834(0, 0);
        introModel = lbl_1_bss_0.pictureIntroModels[lbl_1_bss_0.selectedPicture];
        fn_8002D414(introModel, 0.0f);
        fn_80026A44(introModel, (1 << 0));
        HuAudFXPlay(MSM_SE_MG636_PICTURE_REVEAL_START);
        fn_1_6398(0);
    } else {
        introModel = lbl_1_bss_0.pictureIntroModels[lbl_1_bss_0.selectedPicture];
        if (fn_8002D75C(introModel) == 1) {
            fn_1_6398(1);
            HuAudFXPlay(MSM_SE_MG636_PICTURE_REVEAL_DONE);
            fn_80071A9C();
        }
    }
    playerIndex = 0;
    while (playerIndex < 4) {
        playerPosition = lbl_1_bss_0.players[playerIndex]->actor->pos;
        Hu3D3Dto2D(&playerPosition, 1, &screenPosition);
        voicePan = (s32)screenPosition.x;
        voicePan /= 5;
        if (voicePan < PIXEL_VOICE_PAN_MIN) {
            voicePan = PIXEL_VOICE_PAN_MIN;
        } else if (voicePan > PIXEL_VOICE_PAN_MAX) {
            voicePan = PIXEL_VOICE_PAN_MAX;
        }
        fn_80060F14(
            (s16)lbl_1_bss_0.charNo[playerIndex],
            (s16) voicePan);
        playerIndex += 1;
    }
}

/* The Start hook resets team scores and CPU state, then starts music at the Start message audio
 * cue. */
void fn_1_494(s16 mode, s16 frameNo)
{
    s32 playerIndex;
    MGPLAYER *player;
    s32 storedMusicHandle;
    s32 previousMusicHandle;
    s32 musicHandle;

    if (frameNo == 0) {
        lbl_1_bss_0.cpuActive = 0;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            lbl_1_bss_0.cpuMode[playerIndex] = 1;
            lbl_1_bss_0.targetCell[playerIndex] = -1;
        }
        lbl_1_bss_0.roundIndex = 0;
        lbl_1_bss_0.teamPoints[0] = lbl_1_bss_0.teamPoints[1] = 0;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            player = lbl_1_bss_0.players[playerIndex];
            fn_80087158(player, (1 << 0));
        }
        lbl_1_bss_0.sequenceMode = 0;
        lbl_1_bss_0.sequenceFrame = 0;
    }
    previousMusicHandle = lbl_1_bss_0.musicHandle;
    musicHandle = previousMusicHandle;
    if (musicHandle == -1 && (GameMesStatGet(fn_80071DF4()) & GAMEMES_STAT_FXPLAY) != 0) {
        musicHandle = HuAudBGMPlay(MSM_STREAM_MGMUS_25);
    }
    storedMusicHandle = musicHandle;
    lbl_1_bss_0.musicHandle = storedMusicHandle;
}

/* The main hook runs picture reveals, 20-second rounds and team reactions until two points or five
 * rounds. */
void fn_1_5F0(s16 mode, s16 frameNo)
{
    s32 playerIndex;
    s32 unusedRoundCounter;
    MGPLAYER *transitionPlayer;
    s32 leftComplete;
    s32 rightComplete;
    MGPLAYER *player;
    if (frameNo == 0) {
        fn_1_6790(1);
    }
    if (lbl_1_bss_0.cpuActive == 1) {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (lbl_80265728[playerIndex].type != 0) {
                fn_1_3C98(playerIndex);
            }
        }
    }
    fn_800856A8();
    fn_1_1EC();
    switch (lbl_1_bss_0.sequenceMode) {
    case 0:
        fn_1_3460(lbl_1_bss_0.selectedPicture,
                  lbl_1_bss_0.roundCellChoices[lbl_1_bss_0.roundIndex]);
        fn_1_3834(lbl_1_bss_0.selectedPicture,
                  lbl_1_bss_0.roundCellChoices[lbl_1_bss_0.roundIndex]);
        fn_8002C95C(lbl_1_bss_0.roundIndicatorModel,
                      lbl_1_bss_0.roundIndicatorMotions[1]);
        fn_800269E8(lbl_1_bss_0.roundIndicatorModel, ((1 << 30) | (1 << 0)));
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            MGPLAYER *player;
                player = lbl_1_bss_0.players[playerIndex];
            fn_80087158(player, (1 << 0));
            lbl_1_bss_0.cpuMode[playerIndex] = 1;
            lbl_1_bss_0.targetCell[playerIndex] = -1;
            lbl_1_bss_0.cpuFrameCounter[playerIndex] = 0;
        }
        lbl_1_bss_0.cpuActive = 0;
        lbl_1_bss_0.sequenceMode = 1;
        TICK = 0;
        break;
    case 1:
        if (fn_1_35E0(lbl_1_bss_0.selectedPicture,
                      lbl_1_bss_0.roundCellChoices[lbl_1_bss_0.roundIndex]) != 0) {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                MGPLAYER *player;
                player = lbl_1_bss_0.players[playerIndex];
                fn_80087168(player, (1 << 0));
            }
            lbl_1_bss_0.cpuActive = 1;
            MgTimerParamSet(TIMER, 1200, 0, 0);
            MgTimerModeOnSet(TIMER, 1);
            MgTimerRecordDispOn(TIMER);
            lbl_1_bss_0.roundIndicatorState = 0;
            lbl_1_bss_0.sequenceMode = 2;
            TICK = 0;
        }
        break;
    case 2:
        {
            s32 roundOutcome;
            /* This reset is not consulted by the round outcome checks below. */
            unusedRoundCounter = 0;
            roundOutcome = 0;
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                if (lbl_1_bss_0.landingCounter[playerIndex] != 0) {
                    lbl_1_bss_0.landingCounter[playerIndex]--;
                }
            }
            if (lbl_1_bss_0.roundIndicatorState == 0 &&
                fn_1_35E0(lbl_1_bss_0.selectedPicture,
                          lbl_1_bss_0.roundCellChoices[lbl_1_bss_0.roundIndex]) != 0) {
                lbl_1_bss_0.roundIndicatorState = 1;
                fn_8002C95C(lbl_1_bss_0.roundIndicatorModel,
                              lbl_1_bss_0.roundIndicatorMotions[0]);
            }
            leftComplete = fn_1_3984(0);
            rightComplete = fn_1_3984(1);
            /* Simultaneous matches award one randomly chosen team, before checking timeout. */
            if (leftComplete != 0 && rightComplete != 0) {
                int tieWinner;
                if ((u32)frandmod(2) == 0) {
                    tieWinner = 1;
                } else {
                    tieWinner = 2;
                }
                roundOutcome = tieWinner;
            } else if (leftComplete != 0) {
                roundOutcome = 1;
            } else if (rightComplete != 0) {
                roundOutcome = 2;
            } else if (MgTimerDoneCheck(TIMER) != 0) {
                roundOutcome = 3;
            }
            if (roundOutcome != 0) {
                int nextRoundState;
                switch (roundOutcome) {
                case 1:
                    lbl_1_bss_0.teamPoints[0]++;
                    HuAudFXPlay(MSM_SE_MG636_TEAM_POINT);
                    break;
                case 2:
                    lbl_1_bss_0.teamPoints[1]++;
                    HuAudFXPlay(MSM_SE_MG636_TEAM_POINT);
                    break;
                }
                lbl_1_bss_0.mode = roundOutcome;
                /* Every outcome advances the round, including a timeout. */
                if (roundOutcome != 3);
                lbl_1_bss_0.roundIndex++;
                if (roundOutcome != 3) {
                    MgTimerRecordDispOff(TIMER);
                }
                if (lbl_1_bss_0.teamPoints[0] == 2 || lbl_1_bss_0.teamPoints[1] == 2 ||
                    lbl_1_bss_0.roundIndex == 5) {
                    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                        fn_80087158(lbl_1_bss_0.players[playerIndex], (1 << 0));
                    }
                    lbl_1_bss_0.cpuActive = 0;
                    if (lbl_1_bss_0.mode == 1) {
                        fn_8002C95C(lbl_1_bss_0.roundIndicatorModel,
                                      lbl_1_bss_0.roundIndicatorMotions[2]);
                        fn_8002C95C(lbl_1_bss_0.model,
                                      lbl_1_bss_0.motion[1]);
                    } else if (lbl_1_bss_0.mode == 2) {
                        fn_8002C95C(lbl_1_bss_0.roundIndicatorModel,
                                      lbl_1_bss_0.roundIndicatorMotions[3]);
                        fn_8002C95C(lbl_1_bss_0.model,
                                      lbl_1_bss_0.motion[2]);
                    }
                    fn_80071A9C();
                    break;
                }
                if (roundOutcome == 3) {
                    nextRoundState = 5;
                } else {
                    nextRoundState = 3;
                }
                lbl_1_bss_0.sequenceMode = (s16)nextRoundState;
                TICK = 0;
                break;
            } else {
                fn_1_3BA0();
            }
        }
        break;
    case 3:
        if (TICK == 0) {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                /* The player pointer is fetched here without being used afterward. */
                transitionPlayer = lbl_1_bss_0.players[playerIndex];
                fn_80087158(lbl_1_bss_0.players[playerIndex], (1 << 0));
            }
            lbl_1_bss_0.cpuActive = 0;
            if (lbl_1_bss_0.mode == 1) {
                fn_8002C95C(lbl_1_bss_0.roundIndicatorModel,
                              lbl_1_bss_0.roundIndicatorMotions[2]);
                fn_8002C95C(lbl_1_bss_0.model,
                              lbl_1_bss_0.motion[1]);
            } else if (lbl_1_bss_0.mode == 2) {
                fn_8002C95C(lbl_1_bss_0.roundIndicatorModel,
                              lbl_1_bss_0.roundIndicatorMotions[3]);
                fn_8002C95C(lbl_1_bss_0.model,
                              lbl_1_bss_0.motion[2]);
            } else {
                fn_8002C95C(lbl_1_bss_0.roundIndicatorModel,
                              lbl_1_bss_0.roundIndicatorMotions[1]);
            }
            fn_800269E8(lbl_1_bss_0.roundIndicatorModel, ((1 << 30) | (1 << 0)));
        } else {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                MGPLAYER *player;
                player = lbl_1_bss_0.players[playerIndex];
                {
                s16 idleMotion = player->omObj->mtnId[0];
                if (idleMotion != (s16)fn_8002D3C4((s16)player->actor->mdlId)) {
                    break;
                }
                }
            }
            if (playerIndex == 4) {
                for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                    MGPLAYER *player;
                player = lbl_1_bss_0.players[playerIndex];
                if (lbl_1_bss_0.teamSide[playerIndex] == 0) {
                    int resultMotion;
                    if (lbl_1_bss_0.mode == 1) {
                        resultMotion = 5;
                    } else {
                        resultMotion = 6;
                    }
                    fn_8005C2C8((s16)lbl_1_bss_0.charNo[playerIndex],
                                       player->omObj->mtnId[resultMotion], 0.0f, 0.8f,
                                       0);
                } else {
                    int resultMotion;
                    if (lbl_1_bss_0.mode == 2) {
                        resultMotion = 5;
                    } else {
                        resultMotion = 6;
                    }
                    fn_8005C2C8((s16)lbl_1_bss_0.charNo[playerIndex],
                                       player->omObj->mtnId[resultMotion], 0.0f, 0.8f,
                                       0);
                }
                }
                OSReport("****************************************\n");
                lbl_1_bss_0.sequenceMode = 4;
                TICK = 0;
                break;
            }
        }
        TICK++;
        break;
    case 4:
        if (lbl_1_bss_0.mode != 3) {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                if (fn_8002D75C((s16)lbl_1_bss_0.players[playerIndex]->actor->mdlId) == 0) {
                    break;
                }
            }
            if (playerIndex == 4) {
                OSReport("**** all charracter motion end.\n");
                for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                player = lbl_1_bss_0.players[playerIndex];
                fn_8005C2C8((s16)lbl_1_bss_0.charNo[playerIndex],
                                   player->omObj->mtnId[0], 0.0f, 10.0f, 0);
                }
                fn_8002C95C(lbl_1_bss_0.roundIndicatorModel,
                              lbl_1_bss_0.roundIndicatorMotions[0]);
                fn_8002C95C(lbl_1_bss_0.model,
                              lbl_1_bss_0.motion[0]);
                /* The next-round transition is unconditional after the reactions end. */
                if (lbl_1_bss_0.mode == 1 ||
                    lbl_1_bss_0.mode == 2)
                    ;
                lbl_1_bss_0.sequenceMode = 5;
                TICK = 0;
            }
        } else {
            lbl_1_bss_0.sequenceMode = 5;
            TICK = 0;
        }
        break;
    case 5:
        if (TICK == 0) {
            fn_1_364C(lbl_1_bss_0.selectedPicture,
                      lbl_1_bss_0.roundCellChoices[lbl_1_bss_0.roundIndex - 1]);
        } else if (fn_1_37A8(
                       lbl_1_bss_0.selectedPicture,
                       lbl_1_bss_0.roundCellChoices[lbl_1_bss_0.roundIndex - 1]) != 0) {
            lbl_1_bss_0.sequenceMode = 0;
            TICK = 0;
            break;
        }
        TICK++;
        break;
    case 6:
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if ((s16)fn_8002D3EC((s16)lbl_1_bss_0.players[playerIndex]->actor->mdlId) != -1) {
                break;
            }
        }
        if (playerIndex == 4) {
            fn_8002C95C(lbl_1_bss_0.model, lbl_1_bss_0.motion[0]);
            lbl_1_bss_0.sequenceMode = 0;
            TICK = 0;
        }
        break;
    }
    fn_1_67F4();
}

/* The Finish hook retracts the final picture cell, starts the closing models and fades the game
 * music. */
void fn_1_12E0(s16 mode, s16 frameNo)
{
    s32 musicHandle;
    fn_800856A8();
    fn_1_1EC();
    if (frameNo == 0) {
        fn_1_364C(lbl_1_bss_0.selectedPicture,
                  lbl_1_bss_0.roundCellChoices[lbl_1_bss_0.roundIndex - 1]);
        fn_8002C95C(lbl_1_bss_0.roundIndicatorModel,
            lbl_1_bss_0.roundIndicatorMotions[1]);
        fn_800269E8(lbl_1_bss_0.roundIndicatorModel,
                         ((1 << 30) | (1 << 0)));
        fn_8002C95C(lbl_1_bss_0.finishModel, lbl_1_bss_0.finishMotions[1]);
        fn_80026A44(lbl_1_bss_0.finishModel,
                           ((1 << 30) | (1 << 0)));
        fn_80026A44(
            lbl_1_bss_0.pictureFinishModels[lbl_1_bss_0.selectedPicture],
            (1 << 0));
        fn_8002D414(
            lbl_1_bss_0.pictureFinishModels[lbl_1_bss_0.selectedPicture],
            0.0f);
        /* The finish sound also plays after the final round times out. */
        if (lbl_1_bss_0.mode != 3);
        HuAudFXPlay(MSM_SE_MG636_GAME_FINISH);
        musicHandle = lbl_1_bss_0.musicHandle;
        if (musicHandle != -1) {
            HuAudSStreamFadeOut(musicHandle, 100);
        }
    }
}

/* The pre-winner hook chooses winners, awards coins outside practice mode and moves all players
 * into the result lineup. */
void fn_1_151C(s16 mode, s16 frameNo)
{
    s16 winnerCharacters[4] = {-1, -1, -1, -1};
    s32 isDraw;
    s32 index;
    s32 arrivedCount;

    isDraw = 0;
    fn_800856A8();
    fn_1_1EC();
    if (frameNo == 0) {
        s32 winnerCount;
        /* The shared-win branch gives all players ten coins outside practice. */
        if (lbl_1_bss_0.teamPoints[0] == 2 && lbl_1_bss_0.teamPoints[1] == 2) {
            for (index = 0; index < 4; index++) {
                winnerCharacters[index] = (s16)lbl_1_bss_0.charNo[index];
                if (fn_8006A124(65551) == 0) {
                    lbl_80265750[index].mgCoinBonus = 10;
                }
            }
        } else if (lbl_1_bss_0.teamPoints[0] == 2 && lbl_1_bss_0.teamPoints[1] != 2) {
            winnerCount = 0;
            for (index = 0; index < 4; index++) {
                if (lbl_1_bss_0.teamSide[index] == 0) {
                    winnerCharacters[winnerCount++] = (s16)lbl_1_bss_0.charNo[index];
                    if (fn_8006A124(65551) == 0) {
                        lbl_80265750[index].mgCoinBonus = 10;
                    }
                }
            }
        } else if (lbl_1_bss_0.teamPoints[0] != 2 && lbl_1_bss_0.teamPoints[1] == 2) {
            winnerCount = 0;
            for (index = 0; index < 4; index++) {
                if (lbl_1_bss_0.teamSide[index] == 1) {
                    winnerCharacters[winnerCount++] = (s16)lbl_1_bss_0.charNo[index];
                    if (fn_8006A124(65551) == 0) {
                        lbl_80265750[index].mgCoinBonus = 10;
                    }
                }
            }
        } else {
            isDraw = 1;
        }
        if (isDraw != 0) {
            fn_80071C70();
            lbl_1_bss_0.mode = 3;
        } else {
            fn_80071C18(winnerCharacters[0], winnerCharacters[1], winnerCharacters[2],
                           winnerCharacters[3]);
        }
    }
    if (fn_1_37A8(lbl_1_bss_0.selectedPicture,
                  lbl_1_bss_0.roundCellChoices[lbl_1_bss_0.roundIndex - 1]) != 0) {
        s16 resultModel = lbl_1_bss_0.pictureResultModels[lbl_1_bss_0.selectedPicture];
        fn_8002D414(resultModel, 0.0f);
        fn_80026A44(resultModel, (1 << 0));
        fn_80026A44(resultModel, ((1 << 30) | (1 << 0)));
    }
    for (index = 0; index < 4; index++) {
        fn_80087168(lbl_1_bss_0.players[index], (1 << 0));
        fn_800805CC(lbl_1_bss_0.players[index]->actor, PIXEL_RESULT_COLLISION_ATTRS);
    }
    /* This hook yields inside its own loop until all four players reach their lineup positions. */
    arrivedCount = 0;
    do {
        s32 sidePlayerCounts[2] = {0, 0};
        Point3d lineupPositions[4] = {
            {-650.0f, 0.0f, 0.0f}, {-250.0f, 0.0f, 0.0f},
            {250.0f, 0.0f, 0.0f}, {650.0f, 0.0f, 0.0f}
        };
        Point3d playerPosition;
        Point3d moveDirection;
        MGPLAYER *player;
        Point3d *lineupTarget;
        f32 targetDistance;
        s32 teamSide;
        arrivedCount = 0;
        for (index = 0; index < 4; index++) {
            player = lbl_1_bss_0.players[index];
            playerPosition = player->actor->pos;
            teamSide = lbl_1_bss_0.teamSide[index];
            lineupTarget = &lineupPositions[teamSide * 2 + sidePlayerCounts[teamSide]];
            fn_800805CC(
                lbl_1_bss_0.players[index]->actor,
                PIXEL_RESULT_COLLISION_ATTRS);
            /* Lineup distance ignores height, using each player's floor position. */
            playerPosition.y = 0.0f;
            PSVECSubtract(&playerPosition, lineupTarget, &moveDirection);
            targetDistance = PSVECMag(&moveDirection);
            if (targetDistance >= 10.0f) {
                fn_1_5F54(index, &playerPosition, lineupTarget, &moveDirection, 0);
                fn_80080520(player, (s32)(56.0f * moveDirection.x),
                    (s32)(56.0f * moveDirection.z), 0, 0);
            } else {
                fn_80080520(player, 0, 0, 0, 0);
                fn_80087158(player, (1 << 0));
                fn_80086CA8(player->actor, 0.0f);
                arrivedCount++;
            }
            sidePlayerCounts[teamSide]++;
        }
        fn_800856A8();
        fn_1_1EC();
        HuPrcVSleep();
    } while (arrivedCount != 4);
    /* Hold the lineup for two seconds before allowing the winner display. */
    for (index = 0; index < 120; index++) {
        HuPrcVSleep();
        fn_800856A8();
    }
    fn_80071A9C();
}

/* The winner hook starts each team member's win or lose motion when the result display begins. */
void fn_1_1C74(s16 mode, s16 frameNo)
{
    s32 playerIndex;
    fn_800856A8();
    if (frameNo == 0) {
        fn_8002C95C(lbl_1_bss_0.model, lbl_1_bss_0.motion[0]);
        if (lbl_1_bss_0.mode == 1) {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                MGPLAYER *player = lbl_1_bss_0.players[playerIndex];
                if (lbl_1_bss_0.teamSide[playerIndex] == 0) {
                    fn_8005C2C8(lbl_1_bss_0.charNo[playerIndex], player->omObj->mtnId[5],
                                       0.0f, 0.8f, 0);
                } else {
                    fn_8005C2C8(lbl_1_bss_0.charNo[playerIndex], player->omObj->mtnId[6],
                                       0.0f, 0.8f, 0);
                }
            }
        } else if (lbl_1_bss_0.mode == 2) {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                MGPLAYER *player = lbl_1_bss_0.players[playerIndex];
                if (lbl_1_bss_0.teamSide[playerIndex] == 0) {
                    fn_8005C2C8(lbl_1_bss_0.charNo[playerIndex], player->omObj->mtnId[6],
                                       0.0f, 0.8f, 0);
                    OSReport("### lose ... %d\n", playerIndex);
                } else {
                    fn_8005C2C8(lbl_1_bss_0.charNo[playerIndex], player->omObj->mtnId[5],
                                       0.0f, 0.8f, 0);
                    OSReport("### win ... %d\n", playerIndex);
                }
            }
        } else {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                MGPLAYER *player = lbl_1_bss_0.players[playerIndex];
                fn_8005C2C8(lbl_1_bss_0.charNo[playerIndex], player->omObj->mtnId[6], 0.0f,
                                   0.8f, 0);
                OSReport("### timeover ... ", playerIndex);
            }
        }
    }
}

/* The fade-out hook keeps the actor manager running while the scene fades away. */
void fn_1_1F58(s16 mode, s16 frameNo)
{
    fn_800856A8();
}

/* The close hook updates actors once before overlay return, both after fade-out and on system
 * exit. */
void fn_1_1F78(s16 mode, s16 frameNo)
{
    fn_800856A8();
}
