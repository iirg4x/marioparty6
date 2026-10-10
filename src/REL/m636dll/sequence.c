/* Pixel Perfect opening, timed picture rounds and team-result sequence. */
#include "math.h"
#include "REL/m636dll.h"

#define PIXEL_INTRO_START_SOUND 1867
#define PIXEL_INTRO_END_SOUND 1868
#define PIXEL_ROUND_WIN_SOUND 1870
#define PIXEL_GAME_FINISH_SOUND 1871
/* Disable body-contact movement correction while players enter the result lineup. */
#define PIXEL_RESULT_PUSH_CORRECTION_OFF_FLAG 0x8
#define PIXEL_RESULT_COLLISION_ATTRS \
    (COLBODY_ATTR_RESET | COLBODY_ATTR_BODYCOL_OFF | PIXEL_RESULT_PUSH_CORRECTION_OFF_FLAG)

/* Byte positions of the scene state read by the sequence hooks. */
#define PIXEL_CHARACTER_IDS_OFFSET 72
#define PIXEL_PLAYER_SIDE_OFFSET 104
#define PIXEL_PLAYER_RECORD_OFFSET 120
#define PIXEL_LANDING_COUNTER_OFFSET 136
#define PIXEL_CPU_ACTIVE_OFFSET 152
#define PIXEL_CPU_MODE_OFFSET 156
#define PIXEL_CPU_FRAME_COUNTER_OFFSET 172
#define PIXEL_CPU_TARGET_CELL_OFFSET 188
#define PIXEL_LEFT_POINTS_OFFSET 252
#define PIXEL_RIGHT_POINTS_OFFSET 256
#define PIXEL_ROUND_INDEX_OFFSET 260
#define PIXEL_ROUND_OUTCOME_OFFSET 264
#define PIXEL_PICTURE_INTRO_MODELS_OFFSET 274
#define PIXEL_PICTURE_RESULT_MODELS_OFFSET 280
#define PIXEL_PICTURE_FINISH_MODELS_OFFSET 382
#define PIXEL_ROUND_INDICATOR_MODEL_OFFSET 388
#define PIXEL_ROUND_IDLE_MOTION_OFFSET 390
#define PIXEL_ROUND_RESET_MOTION_OFFSET 392
#define PIXEL_ROUND_LEFT_RESULT_MOTION_OFFSET 394
#define PIXEL_ROUND_RIGHT_RESULT_MOTION_OFFSET 396
#define PIXEL_ROUND_INDICATOR_IDLE_OFFSET 472
#define PIXEL_SCENE_MODEL_OFFSET 476
#define PIXEL_SCENE_IDLE_MOTION_OFFSET 478
#define PIXEL_SCENE_LEFT_RESULT_MOTION_OFFSET 480
#define PIXEL_SCENE_RIGHT_RESULT_MOTION_OFFSET 482
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
    if (playbackHandle == -1 && (GameMesStatGet(MgSeqGameMesIdGet()) & GAMEMES_STAT_FXPLAY) != 0) {
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
        playerPosition = (*(MGPLAYER **) ((u8 *) &lbl_1_bss_0 + playerIndex * sizeof(MGPLAYER *) +
                                          PIXEL_PLAYER_RECORD_OFFSET))
                             ->actor->pos;
        Hu3D3Dto2D(&playerPosition, 1, &screenPosition);
        voicePan = (s32)screenPosition.x;
        voicePan /= 5;
        if (voicePan < PIXEL_VOICE_PAN_MIN) {
            voicePan = PIXEL_VOICE_PAN_MIN;
        } else if (voicePan > PIXEL_VOICE_PAN_MAX) {
            voicePan = PIXEL_VOICE_PAN_MAX;
        }
        CharModelVoicePanSet((s16) * (s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * sizeof(s32) +
                                              PIXEL_CHARACTER_IDS_OFFSET),
                             (s16) voicePan);
    }
}

/* The initialization hook updates players once and immediately requests the opening phase. */
void fn_1_2D0(s16 mode, s16 frameNo)
{
    MgActorExec();
    MgSeqModeNext();
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
        introModel =
            *(s16 *) ((s8 *) &lbl_1_bss_0 + PIXEL_PICTURE_INTRO_MODELS_OFFSET +
                      (*(s32 *) ((s8 *) &lbl_1_bss_0 + PIXEL_SELECTED_PICTURE_OFFSET) * 2));
        Hu3DMotionTimeSet(introModel, (0.0f));
        Hu3DModelAttrReset(introModel, HU3D_ATTR_DISPOFF);
        HuAudFXPlay(PIXEL_INTRO_START_SOUND);
        fn_1_6398(0);
    } else {
        introModel =
            *(s16 *) ((s8 *) &lbl_1_bss_0 + PIXEL_PICTURE_INTRO_MODELS_OFFSET +
                      (*(s32 *) ((s8 *) &lbl_1_bss_0 + PIXEL_SELECTED_PICTURE_OFFSET) * 2));
        if (Hu3DMotionEndCheck(introModel) == 1) {
            fn_1_6398(1);
            HuAudFXPlay(PIXEL_INTRO_END_SOUND);
            MgSeqModeNext();
        }
    }
    playerIndex = 0;
    while (playerIndex < 4) {
        playerPosition =
            *(Point3d *) ((s8 *) (*(void **) ((s8 *) (*(void **) ((s8 *) &lbl_1_bss_0 +
                                                                  PIXEL_PLAYER_RECORD_OFFSET +
                                                                  playerIndex * 4)) +
                                              PIXEL_PLAYER_ACTOR_OFFSET)) +
                          PIXEL_ACTOR_POSITION_OFFSET);
        Hu3D3Dto2D(&playerPosition, 1, &screenPosition);
        voicePan = (s32)screenPosition.x;
        voicePan /= 5;
        if (voicePan < PIXEL_VOICE_PAN_MIN) {
            voicePan = PIXEL_VOICE_PAN_MIN;
        } else if (voicePan > PIXEL_VOICE_PAN_MAX) {
            voicePan = PIXEL_VOICE_PAN_MAX;
        }
        CharModelVoicePanSet(
            (s16) * (s32 *) ((s8 *) &lbl_1_bss_0 + PIXEL_CHARACTER_IDS_OFFSET + playerIndex * 4),
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
        *(s32 *)((s8 *)&lbl_1_bss_0 + PIXEL_CPU_ACTIVE_OFFSET) = 0;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            *(s32 *)((s8 *)&lbl_1_bss_0 + PIXEL_CPU_MODE_OFFSET + playerIndex * 4) = 1;
            *(s32 *)((s8 *)&lbl_1_bss_0 + PIXEL_CPU_TARGET_CELL_OFFSET + playerIndex * 4) = -1;
        }
        *(s32 *)((s8 *)&lbl_1_bss_0 + PIXEL_ROUND_INDEX_OFFSET) = 0;
        *(s32 *)((s8 *)&lbl_1_bss_0 + PIXEL_LEFT_POINTS_OFFSET) =
            *(s32 *)((s8 *)&lbl_1_bss_0 + PIXEL_RIGHT_POINTS_OFFSET) = 0;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            player =
                *(MGPLAYER **) ((s8 *) &lbl_1_bss_0 + PIXEL_PLAYER_RECORD_OFFSET + playerIndex * 4);
            MgPlayerAttrSet(player, MGPLAYER_ATTR_COMSTK);
        }
        *(s16 *)((s8 *)&lbl_1_bss_0 + 4) = 0;
        *(s32 *)((s8 *)&lbl_1_bss_0 + 8) = 0;
    }
    previousMusicHandle = *(s32 *)((s8 *)&lbl_1_bss_0 + PIXEL_MUSIC_HANDLE_OFFSET);
    musicHandle = previousMusicHandle;
    if (musicHandle == -1 && (GameMesStatGet(MgSeqGameMesIdGet()) & GAMEMES_STAT_FXPLAY) != 0) {
        musicHandle = HuAudBGMPlay(MSM_STREAM_MGMUS_25);
    }
    storedMusicHandle = musicHandle;
    *(s32 *)((s8 *)&lbl_1_bss_0 + PIXEL_MUSIC_HANDLE_OFFSET) = storedMusicHandle;
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
    if (BSS32(PIXEL_CPU_ACTIVE_OFFSET) == 1) {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (GwPlayerConf[playerIndex].type != 0) {
                fn_1_3C98(playerIndex);
            }
        }
    }
    MgActorExec();
    fn_1_1EC();
    switch (BSS16(4)) {
    case 0:
        fn_1_3460(BSS32(PIXEL_SELECTED_PICTURE_OFFSET),
                  (*(s32 *) ((u8 *) &lbl_1_bss_0 + (BSS32(PIXEL_ROUND_INDEX_OFFSET)) * 4 +
                             PIXEL_ROUND_CELL_CHOICES_OFFSET)));
        fn_1_3834(BSS32(PIXEL_SELECTED_PICTURE_OFFSET),
                  (*(s32 *) ((u8 *) &lbl_1_bss_0 + (BSS32(PIXEL_ROUND_INDEX_OFFSET)) * 4 +
                             PIXEL_ROUND_CELL_CHOICES_OFFSET)));
        Hu3DMotionSet(BSS16(PIXEL_ROUND_INDICATOR_MODEL_OFFSET),
                      BSS16(PIXEL_ROUND_RESET_MOTION_OFFSET));
        Hu3DModelAttrSet(BSS16(PIXEL_ROUND_INDICATOR_MODEL_OFFSET), HU3D_MOTATTR_LOOP);
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            MGPLAYER *player;
                player = PLAYER(playerIndex);
            MgPlayerAttrSet(player, MGPLAYER_ATTR_COMSTK);
            (*(s32 *)((u8 *)&lbl_1_bss_0 + (playerIndex) * 4 + PIXEL_CPU_MODE_OFFSET)) = 1;
            (*(s32 *)((u8 *)&lbl_1_bss_0 + (playerIndex) * 4 + PIXEL_CPU_TARGET_CELL_OFFSET)) = -1;
            (*(s32 *)((u8 *)&lbl_1_bss_0 + (playerIndex) * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET)) = 0;
        }
        BSS32(PIXEL_CPU_ACTIVE_OFFSET) = 0;
        BSS16(4) = 1;
        TICK = 0;
        break;
    case 1:
        if (fn_1_35E0(BSS32(PIXEL_SELECTED_PICTURE_OFFSET),
                      (*(s32 *) ((u8 *) &lbl_1_bss_0 + (BSS32(PIXEL_ROUND_INDEX_OFFSET)) * 4 +
                                 PIXEL_ROUND_CELL_CHOICES_OFFSET))) != 0) {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                MGPLAYER *player;
                player = PLAYER(playerIndex);
                MgPlayerAttrReset(player, MGPLAYER_ATTR_COMSTK);
            }
            BSS32(PIXEL_CPU_ACTIVE_OFFSET) = 1;
            MgTimerParamSet(TIMER, 1200, 0, 0);
            MgTimerModeOnSet(TIMER, 1);
            MgTimerRecordDispOn(TIMER);
            BSS32(PIXEL_ROUND_INDICATOR_IDLE_OFFSET) = 0;
            BSS16(4) = 2;
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
                if ((*(s32 *) ((u8 *) &lbl_1_bss_0 + (playerIndex) * 4 +
                               PIXEL_LANDING_COUNTER_OFFSET)) != 0) {
                    (*(s32 *) ((u8 *) &lbl_1_bss_0 + (playerIndex) * 4 +
                               PIXEL_LANDING_COUNTER_OFFSET))--;
                }
            }
            if (BSS32(PIXEL_ROUND_INDICATOR_IDLE_OFFSET) == 0 &&
                fn_1_35E0(BSS32(PIXEL_SELECTED_PICTURE_OFFSET),
                          (*(s32 *) ((u8 *) &lbl_1_bss_0 + (BSS32(PIXEL_ROUND_INDEX_OFFSET)) * 4 +
                                     PIXEL_ROUND_CELL_CHOICES_OFFSET))) != 0) {
                BSS32(PIXEL_ROUND_INDICATOR_IDLE_OFFSET) = 1;
                Hu3DMotionSet(BSS16(PIXEL_ROUND_INDICATOR_MODEL_OFFSET),
                              BSS16(PIXEL_ROUND_IDLE_MOTION_OFFSET));
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
                    BSS32(PIXEL_LEFT_POINTS_OFFSET)++;
                    HuAudFXPlay(PIXEL_ROUND_WIN_SOUND);
                    break;
                case 2:
                    BSS32(PIXEL_RIGHT_POINTS_OFFSET)++;
                    HuAudFXPlay(PIXEL_ROUND_WIN_SOUND);
                    break;
                }
                BSS32(PIXEL_ROUND_OUTCOME_OFFSET) = roundOutcome;
                /* Every outcome advances the round, including a timeout. */
                if (roundOutcome != 3);
                BSS32(PIXEL_ROUND_INDEX_OFFSET)++;
                if (roundOutcome != 3) {
                    MgTimerRecordDispOff(TIMER);
                }
                if (BSS32(PIXEL_LEFT_POINTS_OFFSET) == 2 || BSS32(PIXEL_RIGHT_POINTS_OFFSET) == 2 ||
                    BSS32(PIXEL_ROUND_INDEX_OFFSET) == 5) {
                    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                        MgPlayerAttrSet(PLAYER(playerIndex), MGPLAYER_ATTR_COMSTK);
                    }
                    BSS32(PIXEL_CPU_ACTIVE_OFFSET) = 0;
                    if (BSS32(PIXEL_ROUND_OUTCOME_OFFSET) == 1) {
                        Hu3DMotionSet(BSS16(PIXEL_ROUND_INDICATOR_MODEL_OFFSET),
                                      BSS16(PIXEL_ROUND_LEFT_RESULT_MOTION_OFFSET));
                        Hu3DMotionSet(BSS16(PIXEL_SCENE_MODEL_OFFSET),
                                      BSS16(PIXEL_SCENE_LEFT_RESULT_MOTION_OFFSET));
                    } else if (BSS32(PIXEL_ROUND_OUTCOME_OFFSET) == 2) {
                        Hu3DMotionSet(BSS16(PIXEL_ROUND_INDICATOR_MODEL_OFFSET),
                                      BSS16(PIXEL_ROUND_RIGHT_RESULT_MOTION_OFFSET));
                        Hu3DMotionSet(BSS16(PIXEL_SCENE_MODEL_OFFSET),
                                      BSS16(PIXEL_SCENE_RIGHT_RESULT_MOTION_OFFSET));
                    }
                    MgSeqModeNext();
                    break;
                }
                if (roundOutcome == 3) {
                    nextRoundState = 5;
                } else {
                    nextRoundState = 3;
                }
                BSS16(4) = (s16)nextRoundState;
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
                transitionPlayer = PLAYER(playerIndex);
                MgPlayerAttrSet(PLAYER(playerIndex), MGPLAYER_ATTR_COMSTK);
            }
            BSS32(PIXEL_CPU_ACTIVE_OFFSET) = 0;
            if (BSS32(PIXEL_ROUND_OUTCOME_OFFSET) == 1) {
                Hu3DMotionSet(BSS16(PIXEL_ROUND_INDICATOR_MODEL_OFFSET),
                              BSS16(PIXEL_ROUND_LEFT_RESULT_MOTION_OFFSET));
                Hu3DMotionSet(BSS16(PIXEL_SCENE_MODEL_OFFSET),
                              BSS16(PIXEL_SCENE_LEFT_RESULT_MOTION_OFFSET));
            } else if (BSS32(PIXEL_ROUND_OUTCOME_OFFSET) == 2) {
                Hu3DMotionSet(BSS16(PIXEL_ROUND_INDICATOR_MODEL_OFFSET),
                              BSS16(PIXEL_ROUND_RIGHT_RESULT_MOTION_OFFSET));
                Hu3DMotionSet(BSS16(PIXEL_SCENE_MODEL_OFFSET),
                              BSS16(PIXEL_SCENE_RIGHT_RESULT_MOTION_OFFSET));
            } else {
                Hu3DMotionSet(BSS16(PIXEL_ROUND_INDICATOR_MODEL_OFFSET),
                              BSS16(PIXEL_ROUND_RESET_MOTION_OFFSET));
            }
            Hu3DModelAttrSet(BSS16(PIXEL_ROUND_INDICATOR_MODEL_OFFSET), HU3D_MOTATTR_LOOP);
        } else {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                MGPLAYER *player;
                player = PLAYER(playerIndex);
                {
                s16 idleMotion = player->omObj->mtnId[0];
                if (idleMotion != (s16)Hu3DMotionIDGet((s16)player->actor->mdlId)) {
                    break;
                }
                }
            }
            if (playerIndex == 4) {
                for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                    MGPLAYER *player;
                player = PLAYER(playerIndex);
                if ((*(s32 *) ((u8 *) &lbl_1_bss_0 + (playerIndex) * 4 +
                               PIXEL_PLAYER_SIDE_OFFSET)) == 0) {
                    int resultMotion;
                    if (BSS32(PIXEL_ROUND_OUTCOME_OFFSET) == 1) {
                        resultMotion = 5;
                    } else {
                        resultMotion = 6;
                    }
                    CharMotionShiftSet((s16) (*(s32 *) ((u8 *) &lbl_1_bss_0 + (playerIndex) * 4 +
                                                        PIXEL_CHARACTER_IDS_OFFSET)),
                                       player->omObj->mtnId[resultMotion], (0.0f), (0.80000001f),
                                       0);
                } else {
                    int resultMotion;
                    if (BSS32(PIXEL_ROUND_OUTCOME_OFFSET) == 2) {
                        resultMotion = 5;
                    } else {
                        resultMotion = 6;
                    }
                    CharMotionShiftSet((s16) (*(s32 *) ((u8 *) &lbl_1_bss_0 + (playerIndex) * 4 +
                                                        PIXEL_CHARACTER_IDS_OFFSET)),
                                       player->omObj->mtnId[resultMotion], (0.0f), (0.80000001f),
                                       0);
                }
                }
                OSReport("****************************************\n");
                BSS16(4) = 4;
                TICK = 0;
                break;
            }
        }
        TICK++;
        break;
    case 4:
        if (BSS32(PIXEL_ROUND_OUTCOME_OFFSET) != 3) {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                if (Hu3DMotionEndCheck((s16)PLAYER(playerIndex)->actor->mdlId) == 0) {
                    break;
                }
            }
            if (playerIndex == 4) {
                OSReport("**** all charracter motion end.\n");
                for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                player = PLAYER(playerIndex);
                CharMotionShiftSet((s16) (*(s32 *) ((u8 *) &lbl_1_bss_0 + (playerIndex) * 4 +
                                                    PIXEL_CHARACTER_IDS_OFFSET)),
                                   player->omObj->mtnId[0], (0.0f), (10.0f), 0);
                }
                Hu3DMotionSet(BSS16(PIXEL_ROUND_INDICATOR_MODEL_OFFSET),
                              BSS16(PIXEL_ROUND_IDLE_MOTION_OFFSET));
                Hu3DMotionSet(BSS16(PIXEL_SCENE_MODEL_OFFSET),
                              BSS16(PIXEL_SCENE_IDLE_MOTION_OFFSET));
                /* The next-round transition is unconditional after the reactions end. */
                if (BSS32(PIXEL_ROUND_OUTCOME_OFFSET) == 1 ||
                    BSS32(PIXEL_ROUND_OUTCOME_OFFSET) == 2)
                    ;
                BSS16(4) = 5;
                TICK = 0;
            }
        } else {
            BSS16(4) = 5;
            TICK = 0;
        }
        break;
    case 5:
        if (TICK == 0) {
            fn_1_364C(BSS32(PIXEL_SELECTED_PICTURE_OFFSET),
                      (*(s32 *) ((u8 *) &lbl_1_bss_0 + (BSS32(PIXEL_ROUND_INDEX_OFFSET) - 1) * 4 +
                                 PIXEL_ROUND_CELL_CHOICES_OFFSET)));
        } else if (fn_1_37A8(
                       BSS32(PIXEL_SELECTED_PICTURE_OFFSET),
                       (*(s32 *) ((u8 *) &lbl_1_bss_0 + (BSS32(PIXEL_ROUND_INDEX_OFFSET) - 1) * 4 +
                                  PIXEL_ROUND_CELL_CHOICES_OFFSET))) != 0) {
            BSS16(4) = 0;
            TICK = 0;
            break;
        }
        TICK++;
        break;
    case 6:
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if ((s16)Hu3DMotionShiftIDGet((s16)PLAYER(playerIndex)->actor->mdlId) != -1) {
                break;
            }
        }
        if (playerIndex == 4) {
            Hu3DMotionSet(BSS16(PIXEL_SCENE_MODEL_OFFSET), BSS16(PIXEL_SCENE_IDLE_MOTION_OFFSET));
            BSS16(4) = 0;
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
    MgActorExec();
    fn_1_1EC();
    if (frameNo == 0) {
        fn_1_364C(*(s32 *) ((u8 *) &lbl_1_bss_0 + PIXEL_SELECTED_PICTURE_OFFSET),
                  *(s32 *) ((u8 *) &lbl_1_bss_0 +
                            (*(s32 *) ((u8 *) &lbl_1_bss_0 + PIXEL_ROUND_INDEX_OFFSET) - 1) * 4 +
                            PIXEL_ROUND_CELL_CHOICES_OFFSET));
        Hu3DMotionSet(*(s16 *)((u8 *)&lbl_1_bss_0 + PIXEL_ROUND_INDICATOR_MODEL_OFFSET),
            *(s16 *)((u8 *)&lbl_1_bss_0 + PIXEL_ROUND_RESET_MOTION_OFFSET));
        Hu3DModelAttrSet(*(s16 *) ((u8 *) &lbl_1_bss_0 + PIXEL_ROUND_INDICATOR_MODEL_OFFSET),
                         HU3D_MOTATTR_LOOP);
        Hu3DMotionSet(*(s16 *)((u8 *)&lbl_1_bss_0 + PIXEL_FINISH_MODEL_OFFSET),
            *(s16 *)((u8 *)&lbl_1_bss_0 + PIXEL_GAME_FINISH_MOTION_OFFSET));
        Hu3DModelAttrReset(*(s16 *) ((u8 *) &lbl_1_bss_0 + PIXEL_FINISH_MODEL_OFFSET),
                           HU3D_MOTATTR_LOOP);
        Hu3DModelAttrReset(
            *(s16 *) ((u8 *) &lbl_1_bss_0 +
                      *(s32 *) ((u8 *) &lbl_1_bss_0 + PIXEL_SELECTED_PICTURE_OFFSET) * 2 +
                      PIXEL_PICTURE_FINISH_MODELS_OFFSET),
            HU3D_ATTR_DISPOFF);
        Hu3DMotionTimeSet(
            *(s16 *) ((u8 *) &lbl_1_bss_0 +
                      *(s32 *) ((u8 *) &lbl_1_bss_0 + PIXEL_SELECTED_PICTURE_OFFSET) * 2 +
                      PIXEL_PICTURE_FINISH_MODELS_OFFSET),
            (0.0f));
        /* The finish sound also plays after the final round times out. */
        if (*(s32 *)((u8 *)&lbl_1_bss_0 + PIXEL_ROUND_OUTCOME_OFFSET) != 3);
        HuAudFXPlay(PIXEL_GAME_FINISH_SOUND);
        musicHandle = *(s32 *)((u8 *)&lbl_1_bss_0 + PIXEL_MUSIC_HANDLE_OFFSET);
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
    MgActorExec();
    fn_1_1EC();
    if (frameNo == 0) {
        s32 winnerCount;
        /* The shared-win branch gives all players ten coins outside practice. */
        if (*(s32 *)((u8 *)&lbl_1_bss_0 + PIXEL_LEFT_POINTS_OFFSET) == 2 &&
            *(s32 *)((u8 *)&lbl_1_bss_0 + PIXEL_RIGHT_POINTS_OFFSET) == 2) {
            for (index = 0; index < 4; index++) {
                winnerCharacters[index] =
                    (s16) * (s32 *) ((u8 *) &lbl_1_bss_0 + index * 4 + PIXEL_CHARACTER_IDS_OFFSET);
                if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                    GwPlayer[index].mgCoinBonus = 10;
                }
            }
        } else if (*(s32 *)((u8 *)&lbl_1_bss_0 + PIXEL_LEFT_POINTS_OFFSET) == 2 &&
            *(s32 *)((u8 *)&lbl_1_bss_0 + PIXEL_RIGHT_POINTS_OFFSET) != 2) {
            winnerCount = 0;
            for (index = 0; index < 4; index++) {
                if (*(s32 *)((u8 *)&lbl_1_bss_0 + index * 4 + PIXEL_PLAYER_SIDE_OFFSET) == 0) {
                    winnerCharacters[winnerCount++] =
                        (s16) *
                        (s32 *) ((u8 *) &lbl_1_bss_0 + index * 4 + PIXEL_CHARACTER_IDS_OFFSET);
                    if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                        GwPlayer[index].mgCoinBonus = 10;
                    }
                }
            }
        } else if (*(s32 *)((u8 *)&lbl_1_bss_0 + PIXEL_LEFT_POINTS_OFFSET) != 2 &&
            *(s32 *)((u8 *)&lbl_1_bss_0 + PIXEL_RIGHT_POINTS_OFFSET) == 2) {
            winnerCount = 0;
            for (index = 0; index < 4; index++) {
                if (*(s32 *)((u8 *)&lbl_1_bss_0 + index * 4 + PIXEL_PLAYER_SIDE_OFFSET) == 1) {
                    winnerCharacters[winnerCount++] =
                        (s16) *
                        (s32 *) ((u8 *) &lbl_1_bss_0 + index * 4 + PIXEL_CHARACTER_IDS_OFFSET);
                    if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                        GwPlayer[index].mgCoinBonus = 10;
                    }
                }
            }
        } else {
            isDraw = 1;
        }
        if (isDraw != 0) {
            MgSeqDrawSet();
            *(s32 *)((u8 *)&lbl_1_bss_0 + PIXEL_ROUND_OUTCOME_OFFSET) = 3;
        } else {
            MgSeqWinnerSet(winnerCharacters[0], winnerCharacters[1], winnerCharacters[2],
                           winnerCharacters[3]);
        }
    }
    if (fn_1_37A8(*(s32 *) ((u8 *) &lbl_1_bss_0 + PIXEL_SELECTED_PICTURE_OFFSET),
                  *(s32 *) ((u8 *) &lbl_1_bss_0 +
                            (*(s32 *) ((u8 *) &lbl_1_bss_0 + PIXEL_ROUND_INDEX_OFFSET) - 1) * 4 +
                            PIXEL_ROUND_CELL_CHOICES_OFFSET)) != 0) {
        s16 resultModel =
            *(s16 *) ((u8 *) &lbl_1_bss_0 +
                      *(s32 *) ((u8 *) &lbl_1_bss_0 + PIXEL_SELECTED_PICTURE_OFFSET) * 2 +
                      PIXEL_PICTURE_RESULT_MODELS_OFFSET);
        Hu3DMotionTimeSet(resultModel, (0.0f));
        Hu3DModelAttrReset(resultModel, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(resultModel, HU3D_MOTATTR_LOOP);
    }
    for (index = 0; index < 4; index++) {
        MgPlayerAttrReset(
            *(MGPLAYER **) ((u8 *) &lbl_1_bss_0 + index * 4 + PIXEL_PLAYER_RECORD_OFFSET),
            MGPLAYER_ATTR_COMSTK);
        MgActorColAttrSet(
            (*(MGPLAYER **) ((u8 *) &lbl_1_bss_0 + index * 4 + PIXEL_PLAYER_RECORD_OFFSET))->actor,
            PIXEL_RESULT_COLLISION_ATTRS);
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
            player = *(MGPLAYER **)((u8 *)&lbl_1_bss_0 + index * 4 + PIXEL_PLAYER_RECORD_OFFSET);
            playerPosition = player->actor->pos;
            teamSide = *(s32 *)((u8 *)&lbl_1_bss_0 + index * 4 + PIXEL_PLAYER_SIDE_OFFSET);
            lineupTarget = &lineupPositions[teamSide * 2 + sidePlayerCounts[teamSide]];
            MgActorColAttrSet(
                (*(MGPLAYER **) ((u8 *) &lbl_1_bss_0 + index * 4 + PIXEL_PLAYER_RECORD_OFFSET))
                    ->actor,
                PIXEL_RESULT_COLLISION_ATTRS);
            /* Lineup distance ignores height, using each player's floor position. */
            playerPosition.y = (0.0f);
            PSVECSubtract(&playerPosition, lineupTarget, &moveDirection);
            targetDistance = PSVECMag(&moveDirection);
            if (targetDistance >= (10.0f)) {
                fn_1_5F54(index, &playerPosition, lineupTarget, &moveDirection, 0);
                MgPlayerPadSet(player, (s32)((56.0f) * moveDirection.x),
                    (s32)((56.0f) * moveDirection.z), 0, 0);
            } else {
                MgPlayerPadSet(player, 0, 0, 0, 0);
                MgPlayerAttrSet(player, MGPLAYER_ATTR_COMSTK);
                MgActorRotYSet(player->actor, (0.0f));
                arrivedCount++;
            }
            sidePlayerCounts[teamSide]++;
        }
        MgActorExec();
        fn_1_1EC();
        HuPrcVSleep();
    } while (arrivedCount != 4);
    /* Hold the lineup for two seconds before allowing the winner display. */
    for (index = 0; index < 120; index++) {
        HuPrcVSleep();
        MgActorExec();
    }
    MgSeqModeNext();
}

/* The winner hook starts each team member's win or lose motion when the result display begins. */
void fn_1_1C74(s16 mode, s16 frameNo)
{
    s32 playerIndex;
    MgActorExec();
    if (frameNo == 0) {
        Hu3DMotionSet(lbl_1_bss_0.model, lbl_1_bss_0.motion[0]);
        if (lbl_1_bss_0.mode == 1) {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                MGPLAYER *player = lbl_1_bss_0.players[playerIndex];
                if (lbl_1_bss_0.teamSide[playerIndex] == 0) {
                    CharMotionShiftSet(lbl_1_bss_0.charNo[playerIndex], player->omObj->mtnId[5],
                                       (0.0f), (0.80000001f), 0);
                } else {
                    CharMotionShiftSet(lbl_1_bss_0.charNo[playerIndex], player->omObj->mtnId[6],
                                       (0.0f), (0.80000001f), 0);
                }
            }
        } else if (lbl_1_bss_0.mode == 2) {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                MGPLAYER *player = lbl_1_bss_0.players[playerIndex];
                if (lbl_1_bss_0.teamSide[playerIndex] == 0) {
                    CharMotionShiftSet(lbl_1_bss_0.charNo[playerIndex], player->omObj->mtnId[6],
                                       (0.0f), (0.80000001f), 0);
                    OSReport("### lose ... %d\n", playerIndex);
                } else {
                    CharMotionShiftSet(lbl_1_bss_0.charNo[playerIndex], player->omObj->mtnId[5],
                                       (0.0f), (0.80000001f), 0);
                    OSReport("### win ... %d\n", playerIndex);
                }
            }
        } else {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                MGPLAYER *player = lbl_1_bss_0.players[playerIndex];
                CharMotionShiftSet(lbl_1_bss_0.charNo[playerIndex], player->omObj->mtnId[6], (0.0f),
                                   (0.80000001f), 0);
                OSReport("### timeover ... ", playerIndex);
            }
        }
    }
}

/* The fade-out hook keeps the actor manager running while the scene fades away. */
void fn_1_1F58(s16 mode, s16 frameNo)
{
    MgActorExec();
}

/* The close hook updates actors once before overlay return, both after fade-out and on system
 * exit. */
void fn_1_1F78(s16 mode, s16 frameNo)
{
    MgActorExec();
}
