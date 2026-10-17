/* Player animation data and per-frame movement for Endurance Alley. */
#include "dolphin/math.h"

#include "game/object.h"

#include "game/hu3d.h"

#include "game/charman.h"

#include "game/gamework.h"

#include "game/memory.h"

#include "game/data.h"

#include "datadir_enum.h"

#include "REL/mgmrenshodll/module_types.h"

#include "dolphin/gx.h"

#include "game/main.h"

#include "game/audio.h"

#include "msm_stream.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/mg/seqman.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/sprite.h"

#include "game/wipe.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "game/mgdata.h"

typedef struct MGRenshoPlayerState {
    s16 player_index; /* Player slot whose object owns this state. */
    s16 character_no; /* Character model controlled by the object. */
    s16 controller_index; /* Controller assigned to this player. */
    s16 group_no; /* Team or group assigned for the current minigame. */
    s32 player_type; /* Human or CPU player type copied from player setup. */
    s16 com_difficulty; /* CPU difficulty selected for this game. */
    s16 motion_state; /* Movement and animation state handled by the object callback. */
    s16 current_motion; /* Last motion-setting index; request 0 is stored as 1, and -1 means
                         * unset. */
    Vec position; /* Character position in world units. */
    Vec rotation; /* Character rotation in degrees. */
    u8 reservedPositionBytes[4]; /* Cleared with the state block and unused in this file. */
    f32 landingStateA; /* Cleared when the fall begins and again on landing; other use is not
                        * established. */
    f32 vertical_velocity; /* Vertical movement per frame. */
    f32 landingStateB; /* Cleared when the fall begins and again on landing; other use is not
                        * established. */
    s16 jump_frame; /* Current frame of the scripted jump arc. */
    s16 jump_frames; /* Number of frames in the scripted jump arc. */
    u8 reservedJumpBytes[8]; /* Cleared with the state block and unused in this file. */
    f32 remaining_distance; /* Distance left to move along the course, in world units. */
} MGRenshoPlayerState;

typedef struct MGRenshoMotionSetting {
    s16 motion_index; /* Character motion selected for this player state. */
    f32 duration; /* Blend duration in frames; short values select an immediate change. */
    u32 model_attributes; /* Character model attributes applied with the motion. */
    s32 voice_flag; /* Voice and particle effect enable setting applied when this motion is
                     * selected. */
    s32 motion_option; /* Selects a branch whose current code sets the same motion flag. */
} MGRenshoMotionSetting;

typedef struct MGRenshoDifficultyRow {
    s16 computer_difficulty_by_round[10]; /* CPU difficulty for each play-count tier. */
} MGRenshoDifficultyRow;

#define MGR_RENSHO_PLAYER_OBJECT_PRIORITY 4352
#define MGR_RENSHO_PLAYER_STATE_SIZE 76
#define MGR_RENSHO_JUMP_EFFECT 593
#define MGR_RENSHO_TRANSITION_EFFECT 574
#define MGR_RENSHO_FALL_EFFECT 579

extern OMOBJMAN *lbl_1_bss_158;

extern void fn_1_AE08(OMOBJ *object);

extern s16 fn_1_9BCC(void);

extern s16 fn_1_A01C(s16 player);

extern int MgModeWorkInt[16];

extern s16 MgModeScore[GW_PLAYER_MAX];

s16 fn_1_AD78(s16 player);

f32 fn_1_4A2C(void);

f32 fn_1_4ADC(void);

void fn_1_102C(Vec *);

void fn_1_4F0C(f32);

s16 _CharFXPlay(s16 charNo, s16 seNo, u8 voiceFlag);

extern u8 MgModePlayNum;

void fn_1_7D08(u16 responseActive);

void fn_1_8B1C(s16 advancePhaseTimers);

extern s32 lbl_1_data_518;

extern void fn_1_E9C(void);

extern void fn_1_EEBC(void);

void fn_1_7D94(s16 pathIndex, s16 segmentFrames);

void fn_1_106C(s16 selectedCamera);

s16 fn_1_4E58(void);

void fn_1_4E24(void);

void fn_1_4D58(void);

void fn_1_4C4C(void);

void fn_1_4C70(void);

void fn_1_4C84(void);

void fn_1_4CA8(void);

void fn_1_4D24(void);

void fn_1_4CF0(void);

void fn_1_4CBC(void);

void fn_1_4D8C(void);

void fn_1_7E34(void);

void fn_1_7E5C(void);

s16 fn_1_4E00(void);

void fn_1_A34C(s16 player);

extern s32 lbl_1_data_500;

extern s32 lbl_1_data_504;

extern s32 lbl_1_data_508;

extern s32 lbl_1_data_50C;

extern s32 lbl_1_data_510;

extern s32 lbl_1_data_514;

u32 lbl_1_data_338[10] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mariomot, 2), DATANUM(DATA_mariomot, 3),
    DATANUM(DATA_mariomot, 4), DATANUM(DATA_mariomot, 6),
    DATANUM(DATA_mariomot, 40), DATANUM(DATA_mariomot, 36),
    DATANUM(DATA_mariomot, 37), DATANUM(DATA_mariomot, 21)
};
s16 lbl_1_data_360 = -1;
f32 lbl_1_data_364 = 20.0f;
Vec lbl_1_data_368[2] = {{30.0f, 0.0f, 760.0f}, {30.0f, 0.0f, 160.0f}};
MGRenshoMotionSetting lbl_1_data_380[11] = {
    {0, 0.0f, HU3D_MOTATTR_LOOP, 1, 0},
    {0, 8.0f, HU3D_MOTATTR_LOOP, 1, 0},
    {1, 8.0f, HU3D_MOTATTR_LOOP, 1, 0},
    {2, 8.0f, HU3D_MOTATTR_LOOP, 1, 0},
    {3, 8.0f, HU3D_MOTATTR_LOOP, 1, 0},
    {4, 0.0f, HU3D_MOTATTR_LOOP, 1, 0},
    {5, 8.0f, HU3D_MOTATTR_LOOP, 1, 0},
    {6, 8.0f, HU3D_MOTATTR_LOOP, 1, 0},
    {7, 8.0f, HU3D_MOTATTR_LOOP, 1, 0},
    {8, 8.0f, HU3D_MOTATTR_LOOP, 1, 0},
    {9, 8.0f, HU3D_MOTATTR_LOOP, 1, 0}
};
MGRenshoDifficultyRow lbl_1_data_45C[3] = {
    {{0, 1, 1, 1, 2, 2, 2, 3, 3, 3}},
    {{0, 0, 1, 1, 1, 2, 2, 2, 3, 3}},
    {{0, 0, 0, 1, 1, 1, 2, 2, 2, 3}}
};
OMOBJ *lbl_1_bss_140[4];

/* Called at player setup and during movement updates to apply the selected character motion. */
void fn_1_927C(OMOBJ *object, s16 motionSettingIndex)
{
    s16 motionIndex;
    MGRenshoPlayerState *motionData;

    motionData = object->data;
    if ((motionData->player_index < 0) || (motionData->player_index >= 4) ||
        (motionSettingIndex < 0) || (motionSettingIndex >= 11)) {
        return;
    }
    if (motionData->current_motion != motionSettingIndex) {
        motionIndex = lbl_1_data_380[motionSettingIndex].motion_index;
        if (motionIndex != -1) {
            if (lbl_1_data_380[motionSettingIndex].duration <= 1.0f) {
                CharMotionSet(motionData->character_no, object->mtnId[motionIndex]);
                Hu3DModelAttrReset(object->mdlId[0],
                                   Hu3DModelMotionAttrGet(object->mdlId[0]) | HU3D_MOTATTR);
                CharModelAttrSet(motionData->character_no,
                                 lbl_1_data_380[motionSettingIndex].model_attributes);
            } else {
                CharModelAttrSet(motionData->character_no, HU3D_MOTATTR_PAUSE);
                CharMotionShiftSet(motionData->character_no, object->mtnId[motionIndex],
                                   0.0f,
                                   lbl_1_data_380[motionSettingIndex].duration,
                                   lbl_1_data_380[motionSettingIndex].model_attributes);
            }
            if (lbl_1_data_380[motionSettingIndex].motion_option != 0) {
                Hu3DMotionAttrSet(object->mtnId[motionIndex], 1U);
            } else {
                Hu3DMotionAttrSet(object->mtnId[motionIndex], 1U);
            }
            CharModelVoiceFlagSet(motionData->character_no,
                                  lbl_1_data_380[motionSettingIndex].voice_flag);
        }
        motionData->current_motion = motionSettingIndex;
        if (motionData->current_motion == 0) {
            motionData->current_motion = 1;
        }
    }
}

/* Called during mode setup to create the player objects and active character model. */
void fn_1_94A8(void)
{
    MGRenshoPlayerState *playerData;
    OMOBJ *object;
    s32 player;
    s32 motion;

    player = 0;
    while (player < 4) {
        object =
            omAddObjEx(lbl_1_bss_158, MGR_RENSHO_PLAYER_OBJECT_PRIORITY, 1U, 10, -1, fn_1_AE08);
        lbl_1_bss_140[player] = object;
        object->stat |= OM_STAT_MODELPAUSE;
        object->work[0] = object->work[1] = object->work[2] = object->work[3] = 0;
        object->data = HuMemDirectMallocNum(HEAP_HEAP, MGR_RENSHO_PLAYER_STATE_SIZE, HU_MEMNUM_OVL);
        memset(object->data, 0, MGR_RENSHO_PLAYER_STATE_SIZE);
        playerData = object->data;
        playerData->player_index = player;
        playerData->character_no = GwPlayerConf[player].charNo;
        playerData->controller_index = GwPlayerConf[player].padNo;
        playerData->group_no = GwPlayerConf[player].grpNo;
        playerData->player_type = GwPlayerConf[player].type;
        playerData->com_difficulty = GwPlayerConf[player].comDif;
        playerData->motion_state = -1;
        /* Setup always selects player slot 0; every other slot is forced to CPU difficulty 1
         * below. */
        lbl_1_data_360 = 0;
        if (lbl_1_data_360 != playerData->player_index) {
            GwPlayerConf[player].grpNo = player;
            GwPlayerConf[player].type = 1;
            GwPlayerConf[player].comDif = 1;
            playerData->group_no = GwPlayerConf[player].grpNo;
            playerData->player_type = GwPlayerConf[player].type;
            playerData->com_difficulty = GwPlayerConf[player].comDif;
        }
        if (lbl_1_data_360 == playerData->player_index) {
            *object->mdlId = CharModelCreate(playerData->character_no, 2);
            motion = 0;
            while (motion < 10) {
                object->mtnId[motion] =
                    CharMotionCreate(playerData->character_no, lbl_1_data_338[motion]);
                motion++;
            }
            CharMotionDataClose(playerData->character_no);
            playerData->current_motion = -1;
            fn_1_927C(object, 0);
            Hu3DModelCameraSet(*object->mdlId, 1U);
            Hu3DModelLayerSet(*object->mdlId, 1);
            Hu3DModelShadowSet(*object->mdlId);
            playerData->position = lbl_1_data_368[0];
            Hu3DModelPosSetV(*object->mdlId, &playerData->position);
            playerData->rotation.x = playerData->rotation.y =
                playerData->rotation.z = 0.0f;
            Hu3DModelRotSetV(*object->mdlId, &playerData->rotation);
            CharModelAttrSet(playerData->character_no, 1U);
            CharMotionVoiceOnSet(playerData->character_no, lbl_1_data_338[7], 0);
            CharMotionVoiceOnSet(playerData->character_no, lbl_1_data_338[3], 0);
        }
        player++;
    }
    CharEffectLayerSet(3);
}

/* Called when the selected player enters the course to place them at the start. */
void fn_1_9978(void) {
    OMOBJ *object;
    MGRenshoPlayerState *playerData;

    object = lbl_1_bss_140[lbl_1_data_360];
    playerData = (MGRenshoPlayerState *)object->data;
    playerData->motion_state = 0;
    CharModelAttrReset(playerData->character_no, 1);

    playerData->position = lbl_1_data_368[1];
    Hu3DModelPosSetV(*object->mdlId, &playerData->position);

    playerData->rotation.x = playerData->rotation.z = 0.0f;
    playerData->rotation.y = 180.0f;
    Hu3DModelRotSetV(*object->mdlId, &playerData->rotation);

    playerData = (MGRenshoPlayerState *)lbl_1_bss_140[lbl_1_data_360]->data;
    CharModelAttrReset(playerData->character_no, 1);
}

/* During mode setup, reorders cached character motions, shifts the opponent characters,
 * and adds a random replacement. */
void fn_1_9A70(void) {
    MGRenshoPlayerState *playerData;
    s16 replacementCharacter;
    s16 characterOrder[4];

    replacementCharacter = fn_1_9BCC();
    characterOrder[0] = fn_1_A01C(0);
    characterOrder[1] = fn_1_A01C(2);
    characterOrder[2] = fn_1_A01C(3);
    characterOrder[3] = fn_1_A01C(1);
    CharMotionLoad(characterOrder);
    CharDataClose(fn_1_A01C(1));
    CharMotionInit(replacementCharacter);

    playerData = lbl_1_bss_140[2]->data;
    GwPlayerConf[1].charNo = playerData->character_no;
    playerData = lbl_1_bss_140[3]->data;
    GwPlayerConf[2].charNo = playerData->character_no;
    GwPlayerConf[3].charNo = replacementCharacter;
    playerData = lbl_1_bss_140[1]->data;
    playerData->character_no = GwPlayerConf[1].charNo;
    playerData = lbl_1_bss_140[2]->data;
    playerData->character_no = GwPlayerConf[2].charNo;
    playerData = lbl_1_bss_140[3]->data;
    playerData->character_no = GwPlayerConf[3].charNo;
}

/* Called after asynchronous character data reads to wait for the DMA transfer to finish. */
void fn_1_9B9C(void) {
    while (HuARDMACheck() != 0) {
        HuPrcVSleep();
    }
}

/* Called during character setup to choose a random character not already in use. */
s16 fn_1_9BCC(void) {
    s16 available[14];
    MGRenshoPlayerState *playerData;
    s16 index;
    s16 availableCount;

    for (index = 0; index < 14; index++) {
        available[index] = 1;
    }
    available[11] = 0;
    available[12] = 0;
    available[13] = 0;
    if (GWBankFlagGet(2) == 0) {
        available[10] = 0;
    }

    for (index = 0; index < 4; index++) {
        playerData = lbl_1_bss_140[index]->data;
        available[playerData->character_no] = 0;
    }

    availableCount = 0;
    for (index = 0; index < 14; index++) {
        if (available[index] != 0) {
            availableCount++;
        }
    }

    availableCount = frandmod(availableCount);
    for (index = 0; index < 14; index++) {
        if (available[index] != 0) {
            availableCount--;
            if (availableCount < 0) {
                break;
            }
        }
    }
    return index;
}

/* Called after a round to mark whether the selected player reached the score limit. */
void fn_1_9D2C(void)
{
    s16 player;

    MgModeWorkInt[0] = 0;
    player = 0;
    /* The flag was just cleared, so this first score scan is always skipped. */
    while ((player < 4) && (MgModeWorkInt[0] != 0)) {
        if (MgModeScore[player] >= 10) {
            break;
        }
        player++;
    }
    if (player >= 4) {
        MgModeWorkInt[0] = 0;
    }
    player = 0;
    while (player < 4) {
        if ((player == lbl_1_data_360) && (MgModeScore[lbl_1_data_360] >= 10)) {
            MgModeWorkInt[0] = 1;
        }
        player++;
    }
}

/* Before each game, forces the active slot to human/team 0 and assigns CPU opponents
 * their teams and play-count difficulty. */
void fn_1_9E20(void)
{
    MGRenshoPlayerState *playerData;
    s16 row;
    s16 player;

    row = 0;
    player = 0;
    while (player < 4) {
        playerData = lbl_1_bss_140[player]->data;
        if (lbl_1_data_360 == player) {
            playerData->group_no = 0;
            playerData->player_type = 0;
        } else {
            if ((MgDataTbl[MgModeWork[MgModePlayNum]].type == 0) ||
                (MgDataTbl[MgModeWork[MgModePlayNum]].type == 6)) {
                playerData->group_no = player;
            }
            if (MgDataTbl[MgModeWork[MgModePlayNum]].type == 1) {
                playerData->group_no = 1;
            }
            playerData->player_type = 1;
            playerData->com_difficulty =
                lbl_1_data_45C[row]
                    .computer_difficulty_by_round[MgModePlayNum / 10];
            row++;
        }
        GwPlayerConf[player].grpNo = playerData->group_no;
        GwPlayerConf[player].type = (s16)playerData->player_type;
        GwPlayerConf[player].comDif = playerData->com_difficulty;
        player++;
    }
}

/* Called by result and movement routines to retrieve the selected player slot. */
s16 fn_1_A00C(void)
{
    return lbl_1_data_360;
}

/* Called while loading character motions to retrieve a player's character number. */
s16 fn_1_A01C(s16 player)
{
    MGRenshoPlayerState *playerData;
    s16 characterNo;

    playerData = lbl_1_bss_140[player]->data;
    characterNo = playerData->character_no;
    return characterNo;
}

/* Returns the player slot's human or CPU type for window-input and rumble checks. */
s16 fn_1_A05C(s16 player)
{
    MGRenshoPlayerState *playerData = lbl_1_bss_140[player]->data;
    return (s16)playerData->player_type;
}

/* Called by window-input setup to retrieve the controller port assigned to a player slot. */
s16 fn_1_A094(s16 player) {
    MGRenshoPlayerState *playerData;
    playerData = lbl_1_bss_140[player]->data;
    return playerData->controller_index;
}

void fn_1_A0C8(void) {}

/* In submode 2, walks the selected player from the preview position back to the course start. */
void fn_1_A0CC(void)
{
    MGRenshoPlayerState *motionData;
    OMOBJ *object;
    s16 keepWaiting;

    motionData = lbl_1_bss_140[lbl_1_data_360]->data;
    CharModelAttrReset(motionData->character_no, 1U);
    motionData->motion_state = 0;
    HuPrcSleep(20);
    motionData->motion_state = -1;

    object = lbl_1_bss_140[lbl_1_data_360];
    fn_1_927C(object, 3);

    HuPrcVSleep();
    keepWaiting = 1;
    while (keepWaiting != 0) {
        keepWaiting = 0;
        motionData->position.z -= 10.0f;
        if (motionData->position.z > lbl_1_data_368[1].z) {
            keepWaiting = 1;
        } else {
            motionData->position.z = lbl_1_data_368[1].z;
        }
        HuPrcVSleep();
    }
    motionData->motion_state = 0;
    HuPrcVSleep();
}

void fn_1_A348(void) {}

/* Called after a turn to script the active player's route across the course. */
void fn_1_A34C(s16 player)
{
    MGRenshoPlayerState *animation;
    s16 wait;
    f32 remainingDepth;

    animation = (MGRenshoPlayerState *)lbl_1_bss_140[player]->data;
    if ((MgModePlayNum == 4) || (MgModePlayNum == 16) ||
        (MgModePlayNum == 24) || (MgModePlayNum == 32) ||
        (MgModePlayNum == 34) || (MgModePlayNum == 73) ||
        (MgModePlayNum == 88)) {
        animation->motion_state = 1;
        animation->remaining_distance = 900.0f;
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(lbl_1_data_360) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
        animation->motion_state = 4;
        animation->remaining_distance = 1420.0f;
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(lbl_1_data_360) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
        animation->motion_state = 1;
        animation->remaining_distance = fn_1_4A2C() - fn_1_4ADC();
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(lbl_1_data_360) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
    } else if ((MgModePlayNum == 36) || (MgModePlayNum == 53) ||
               (MgModePlayNum == 56)) {
        animation->motion_state = 1;
        animation->remaining_distance = 900.0f;
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(lbl_1_data_360) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
        animation->motion_state = 4;
        animation->remaining_distance = 1100.0f;
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(lbl_1_data_360) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
        animation->motion_state = 4;
        animation->remaining_distance = fn_1_4A2C() - fn_1_4ADC();
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(lbl_1_data_360) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
    } else if ((MgModePlayNum == 38) || (MgModePlayNum == 54) ||
               (MgModePlayNum == 58)) {
        animation->motion_state = 4;
        animation->remaining_distance = 1200.0f;
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(lbl_1_data_360) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
        animation->motion_state = 4;
        animation->remaining_distance = 1050.0f;
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(lbl_1_data_360) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
        animation->motion_state = 1;
        animation->remaining_distance = fn_1_4A2C() - fn_1_4ADC();
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(lbl_1_data_360) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
    } else if ((MgModePlayNum == 37) || (MgModePlayNum == 57)) {
        animation->motion_state = 4;
        animation->remaining_distance = 1500.0f;
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(lbl_1_data_360) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
        animation->motion_state = 4;
        animation->remaining_distance = fn_1_4A2C() - fn_1_4ADC();
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(lbl_1_data_360) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
    } else if (MgModePlayNum == 61) {
        animation->motion_state = 1;
        animation->remaining_distance = 1500.0f;
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(lbl_1_data_360) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
        animation->motion_state = 4;
        animation->remaining_distance = fn_1_4A2C() - fn_1_4ADC();
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(lbl_1_data_360) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
    } else if (MgModePlayNum == 82) {
        animation->motion_state = 4;
        animation->remaining_distance = 1600.0f;
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(lbl_1_data_360) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
        animation->motion_state = 1;
        animation->remaining_distance = fn_1_4A2C() - fn_1_4ADC();
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(lbl_1_data_360) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
    } else if (MgModePlayNum == 91) {
        animation->motion_state = 1;
        animation->remaining_distance = 900.0f;
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(lbl_1_data_360) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
        animation->motion_state = 4;
        animation->remaining_distance = 1550.0f;
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(lbl_1_data_360) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
        animation->motion_state = 1;
        animation->remaining_distance = fn_1_4A2C() - fn_1_4ADC();
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(lbl_1_data_360) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
    } else {
        animation->motion_state = 1;
        remainingDepth = fn_1_4A2C() - fn_1_4ADC();
        animation->remaining_distance = remainingDepth;
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(lbl_1_data_360) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
    }
    HuPrcVSleep();
}

/* Selects a reaction starting at state 10 or 13; the full-streak ending uses state 10. */
void fn_1_AB9C(s16 player, s16 useFirstState) {
    MGRenshoPlayerState *playerData;

    playerData = lbl_1_bss_140[player]->data;
    if (useFirstState != 0) {
        playerData->motion_state = 10;
        return;
    }
    playerData->motion_state = 13;
}

/* Selects the player's reaction sequence starting at state 16 or 19. */
void fn_1_ABEC(s16 player, s16 useFirstState) {
    MGRenshoPlayerState *playerData;

    playerData = lbl_1_bss_140[player]->data;
    if (useFirstState != 0) {
        playerData->motion_state = 16;
        return;
    }
    playerData->motion_state = 19;
}

/* Called after a full streak to launch the player toward the raised finish position. */
void fn_1_AC3C(s16 player)
{
    MGRenshoPlayerState *playerData;
    playerData = lbl_1_bss_140[player]->data;
    playerData->motion_state = 22;
}

/* Resets the listed reaction and landing states to idle, including ones still in progress. */
void fn_1_AC74(s16 player) {
    MGRenshoPlayerState *playerData = lbl_1_bss_140[player]->data;

    if ((playerData->motion_state == 10) || (playerData->motion_state == 11) ||
        (playerData->motion_state == 12) || (playerData->motion_state == 16) ||
        (playerData->motion_state == 17) || (playerData->motion_state == 18)) {
        playerData->motion_state = 0;
        return;
    }
    if ((playerData->motion_state == 13) || (playerData->motion_state == 14) ||
        (playerData->motion_state == 15) || (playerData->motion_state == 19) ||
        (playerData->motion_state == 20) || (playerData->motion_state == 21)) {
        playerData->motion_state = 0;
        return;
    }
    if ((playerData->motion_state == 7) || (playerData->motion_state == 8) ||
        (playerData->motion_state == 9)) {
        playerData->motion_state = 0;
    }
}

/* Movement and ending scripts accept idle, externally controlled (-1), or terminal reaction
 * states. */
s16 fn_1_AD78(s16 player)
{
    MGRenshoPlayerState *playerData;

    playerData = lbl_1_bss_140[player]->data;
    if ((playerData->motion_state == -1) || (playerData->motion_state == 0) ||
        (playerData->motion_state == 12) || (playerData->motion_state == 15) ||
        (playerData->motion_state == 18) || (playerData->motion_state == 21) ||
        (playerData->motion_state == 9)) {
        return 1;
    }
    return 0;
}

/* Called by the object manager each frame to update the active player's movement and animation. */
void fn_1_AE08(OMOBJ *playerObject) {
    s32 playerIndex;
    MGRenshoPlayerState *unusedPlayerStateA;
    Point3d cameraDirection;
    s16 motionReady;
    s16 unusedLocalA;
    s16 unusedLocalB;
    s16 unusedLocalC;
    s16 unusedLocalD;
    s16 unusedLocalE;
    s16 unusedLocalF;
    s16 unusedLocalG;
    s16 unusedLocalH;
    s16 unusedLocalI;
    s16 unusedLocalJ;
    s16 unusedLocalK;
    s16 unusedLocalL;
    s16 unusedLocalM;
    s16 unusedLocalN;
    s16 unusedLocalO;
    s16 unusedLocalP;
    f32 movementOrHeading;
    MGRenshoPlayerState *unusedPlayerStateB;
    MGRenshoPlayerState *unusedPlayerStateC;
    MGRenshoPlayerState *unusedPlayerStateD;
    MGRenshoPlayerState *unusedPlayerStateE;
    MGRenshoPlayerState *unusedPlayerStateF;
    MGRenshoPlayerState *unusedPlayerStateG;
    MGRenshoPlayerState *unusedPlayerStateH;
    MGRenshoPlayerState *unusedPlayerStateI;
    MGRenshoPlayerState *unusedPlayerStateJ;
    MGRenshoPlayerState *unusedPlayerStateK;
    MGRenshoPlayerState *unusedPlayerStateL;
    MGRenshoPlayerState *unusedPlayerStateM;
    MGRenshoPlayerState *unusedPlayerStateN;
    MGRenshoPlayerState *unusedPlayerStateO;
    MGRenshoPlayerState *unusedPlayerStateP;
    MGRenshoPlayerState *playerState;
    MGRenshoPlayerState *unusedPlayerStateQ;

    playerState = playerObject->data;
    if (playerState->player_index == lbl_1_data_360) {
        switch (playerState->motion_state) {
            case 0:
                if ((165.0f < playerState->rotation.y) && (playerState->rotation.y < 195.0f)) {
                    playerState->rotation.y = 180.0f;
                    fn_1_927C(playerObject, 1);
                } else if (((CharMotionShiftIDGet(playerState->character_no) < 0) &&
                            ((playerState->current_motion == 2) ||
                             (playerState->current_motion == 1))) ||
                           ((CharMotionShiftIDGet(playerState->character_no) >= 0) &&
                            (playerState->current_motion == 2))) {
                    /* Both heading branches below add 15 degrees, even when the yaw already exceeds
                     * 180. */
                    if (playerState->rotation.y < 180.0f) {
                        playerState->rotation.y += 15.0f;
                    }
                    if (playerState->rotation.y > 180.0f) {
                        playerState->rotation.y += 15.0f;
                    }
                    lbl_1_data_380[2].duration = 4.0f;
                    fn_1_927C(playerObject, 2);
                    lbl_1_data_380[2].duration = 8.0f;
                } else {
                    fn_1_927C(playerObject, 1);
                }
                break;
            case 1:
                fn_1_927C(playerObject, 3);
                playerState->motion_state = 2;
                break;
            case 2:
                movementOrHeading = 0.1f * playerState->remaining_distance;
                if (movementOrHeading >= lbl_1_data_364) {
                    movementOrHeading = lbl_1_data_364;
                }
                if (movementOrHeading < 1.0f) {
                    movementOrHeading = 1.0f;
                }
                if (playerState->remaining_distance < 1.0f) {
                    fn_1_4F0C(playerState->remaining_distance);
                    playerState->remaining_distance = 0.0f;
                    playerState->motion_state = 3;
                } else {
                    fn_1_4F0C(movementOrHeading);
                    playerState->remaining_distance -= movementOrHeading;
                }
                break;
            case 3:
                playerState->motion_state = 0;
                break;
            case 4:
                fn_1_927C(playerObject, 4);
                _CharFXPlay(playerState->character_no, MGR_RENSHO_JUMP_EFFECT, 0);
                playerState->motion_state = 5;
                playerState->jump_frame = 0;
                playerState->jump_frames = (s16) (playerState->remaining_distance / lbl_1_data_364);
                break;
            case 5:
                if ((playerState->current_motion == 4) &&
                    (CharMotionShiftIDGet(playerState->character_no) < 0)) {
                    if ((Hu3DData[playerObject->mdlId[0]].motWork.speed +
                         CharMotionTimeGet(playerState->character_no)) >=
                        CharMotionMaxTimeGet(playerState->character_no)) {
                        Hu3DModelAttrSet(playerObject->mdlId[0],
                                         HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                                             HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
                    }
                }
                if (playerState->remaining_distance <= lbl_1_data_364) {
                    fn_1_4F0C(playerState->remaining_distance);
                    playerState->remaining_distance = 0.0f;
                } else {
                    fn_1_4F0C(lbl_1_data_364);
                    playerState->remaining_distance -= lbl_1_data_364;
                }
                playerState->jump_frame += 1;
                playerState->position.y = (f32) sin(
                    (3.141592653589793 * (f64) ((180.0f / (f32) playerState->jump_frames) *
                                                (f32) playerState->jump_frame)) /
                    180.0);
                playerState->position.y *= 400.0f;
                if (playerState->position.y < 0.0f) {
                    playerState->position.y = 0.0f;
                }
                if (playerState->jump_frame >= playerState->jump_frames) {
                    playerState->jump_frame = playerState->jump_frames = 0;
                    playerState->vertical_velocity =
                        playerState->position.y = 0.0f;
                    Hu3DModelAttrReset(playerObject->mdlId[0],
                                       HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                                           HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
                    fn_1_927C(playerObject, 5);
                    playerState->motion_state = 6;
                }
                break;
            case 6:
                motionReady = 0;
                if ((playerState->current_motion != 1) &&
                    (CharMotionShiftIDGet(playerState->character_no) < 0) &&
                    ((Hu3DData[playerObject->mdlId[0]].motWork.speed +
                      CharMotionTimeGet(playerState->character_no)) >=
                     CharMotionMaxTimeGet(playerState->character_no))) {
                    fn_1_927C(playerObject, 1);
                    motionReady = 1;
                } else if (playerState->current_motion == 1) {
                    motionReady = 1;
                }
                if (playerState->remaining_distance <= lbl_1_data_364) {
                    fn_1_4F0C(playerState->remaining_distance);
                    playerState->remaining_distance = 0.0f;
                    if (motionReady != 0) {
                        playerState->motion_state = 0;
                    }
                } else {
                    fn_1_4F0C(lbl_1_data_364);
                    playerState->remaining_distance -= lbl_1_data_364;
                }
                break;
            case 7:
                fn_1_927C(playerObject, 5);
                if (CharMotionShiftIDGet(playerState->character_no) < 0) {
                    playerState->motion_state = 8;
                    Hu3DModelAttrSet(playerObject->mdlId[0],
                                     HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                                         HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
                    playerState->landingStateA =
                        playerState->vertical_velocity =
                            playerState->landingStateB = 0.0f;
                }
                break;
            case 8:
                playerState->vertical_velocity -= 0.4f;
                playerState->position.y += playerState->vertical_velocity;
                if (playerState->position.y < 0.0f) {
                    Hu3DModelAttrReset(playerObject->mdlId[0],
                                       HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                                           HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
                    if (playerState->current_motion == 5) {
                        if ((Hu3DData[playerObject->mdlId[0]].motWork.speed +
                             CharMotionTimeGet(playerState->character_no)) >=
                            CharMotionMaxTimeGet(playerState->character_no)) {
                            fn_1_927C(playerObject, 10);
                            playerState->motion_state = 9;
                        }
                    }
                    playerState->landingStateA =
                        playerState->vertical_velocity =
                            playerState->landingStateB = 0.0f;
                    playerState->position.y = 0.0f;
                }
                break;
            case 10:
                fn_1_102C(&cameraDirection);
                PSVECScale(&cameraDirection, &cameraDirection, -1.0f);
                movementOrHeading =
                    (f32) (180.0 * (atan2((f64) cameraDirection.x, (f64) cameraDirection.z) /
                                    3.141592653589793));
                playerState->rotation.y -= 10.0f;
                if (playerState->rotation.y <= movementOrHeading) {
                    playerState->rotation.y = movementOrHeading;
                    playerState->motion_state = 11;
                }
                break;
            case 11:
                fn_1_927C(playerObject, 6);
                if (CharMotionShiftIDGet(playerState->character_no) < 0) {
                    if ((Hu3DData[playerObject->mdlId[0]].motWork.speed +
                         CharMotionTimeGet(playerState->character_no)) >=
                        CharMotionMaxTimeGet(playerState->character_no)) {
                        Hu3DModelAttrSet(playerObject->mdlId[0], HU3D_MOTATTR_PAUSE);
                        playerState->motion_state = 12;
                    }
                }
                break;
            case 13:
                fn_1_927C(playerObject, 7);
                playerState->motion_state = 14;
                CharFXPlay(playerState->character_no, MGR_RENSHO_TRANSITION_EFFECT);
                break;
            case 14:
                if (CharMotionShiftIDGet(playerState->character_no) < 0) {
                    if ((Hu3DData[playerObject->mdlId[0]].motWork.speed +
                         CharMotionTimeGet(playerState->character_no)) >=
                        CharMotionMaxTimeGet(playerState->character_no)) {
                        Hu3DModelAttrSet(playerObject->mdlId[0], HU3D_MOTATTR_PAUSE);
                        playerState->motion_state = 15;
                    }
                }
                break;
            case 16:
                fn_1_102C(&cameraDirection);
                PSVECScale(&cameraDirection, &cameraDirection, -1.0f);
                movementOrHeading =
                    (f32) (180.0 * (atan2((f64) cameraDirection.x, (f64) cameraDirection.z) /
                                    3.141592653589793));
                playerState->rotation.y -= 10.0f;
                if (playerState->rotation.y <= movementOrHeading) {
                    playerState->rotation.y = movementOrHeading;
                    playerState->motion_state = 17;
                }
                break;
            case 17:
                if (playerState->current_motion != 8) {
                    _CharFXPlay(playerState->character_no, MGR_RENSHO_FALL_EFFECT, 0);
                }
                fn_1_927C(playerObject, 8);
                if (CharMotionShiftIDGet(playerState->character_no) < 0) {
                    if ((Hu3DData[playerObject->mdlId[0]].motWork.speed +
                         CharMotionTimeGet(playerState->character_no)) >=
                        CharMotionMaxTimeGet(playerState->character_no)) {
                        Hu3DModelAttrSet(playerObject->mdlId[0], HU3D_MOTATTR_PAUSE);
                        playerState->motion_state = 18;
                    }
                }
                break;
            case 19:
                fn_1_927C(playerObject, 9);
                playerState->motion_state = 20;
                break;
            case 20:
                if (CharMotionShiftIDGet(playerState->character_no) < 0) {
                    if ((Hu3DData[playerObject->mdlId[0]].motWork.speed +
                         CharMotionTimeGet(playerState->character_no)) >=
                        CharMotionMaxTimeGet(playerState->character_no)) {
                        Hu3DModelAttrSet(playerObject->mdlId[0], HU3D_MOTATTR_PAUSE);
                        playerState->motion_state = 21;
                    }
                }
                break;
            case 22:
                fn_1_927C(playerObject, 4);
                playerState->jump_frame = playerState->jump_frames = 0;
                playerState->vertical_velocity = 21.0f;
                playerState->motion_state = 23;
                break;
            case 23:
                if ((playerState->current_motion == 4) &&
                    (CharMotionShiftIDGet(playerState->character_no) < 0)) {
                    if ((Hu3DData[playerObject->mdlId[0]].motWork.speed +
                         CharMotionTimeGet(playerState->character_no)) >=
                        CharMotionMaxTimeGet(playerState->character_no)) {
                        Hu3DModelAttrSet(playerObject->mdlId[0],
                                         HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                                             HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
                    }
                }
                playerState->position.x -= 3.0f;
                playerState->position.z -= 18.0f;
                if (playerState->position.z <= -3000.0f) {
                    playerState->position.z = -3000.0f;
                }
                playerState->position.y += playerState->vertical_velocity;
                playerState->vertical_velocity -= 0.4f;
                if ((playerState->position.y < 225.0f) && (playerState->vertical_velocity < 0.0f)) {
                    playerState->vertical_velocity = 0.0f;
                    playerState->position.y = 225.0f;
                    Hu3DModelAttrReset(playerObject->mdlId[0],
                                       HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                                           HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
                    fn_1_927C(playerObject, 5);
                    playerState->motion_state = 24;
                }
                break;
            case 24:
                if (CharMotionShiftIDGet(playerState->character_no) < 0) {
                    if ((Hu3DData[playerObject->mdlId[0]].motWork.speed +
                         CharMotionTimeGet(playerState->character_no)) >=
                        CharMotionMaxTimeGet(playerState->character_no)) {
                        playerState->motion_state = 0;
                        fn_1_927C(playerObject, 1);
                    }
                }
                break;
            default:
                break;
            }
        Hu3DModelRotSetV(playerObject->mdlId[0], &playerState->rotation);
        Hu3DModelPosSetV(playerObject->mdlId[0], &playerState->position);
    }
}

/* In submode 8, selects the player's transition from the round flag and starts the sign response
 * when it is set. */
void fn_1_D2BC(void)
{
    MGRenshoPlayerState *activeData;
    MGRenshoPlayerState *idleData;
    s16 activeObjectIndex;
    s16 idleObjectIndex;
    s16 activePlayer;
    s16 idlePlayer;

    if (MgModeWorkInt[0] != 0) {
        activePlayer = lbl_1_data_360;
        activeObjectIndex = activePlayer;
        activeData = lbl_1_bss_140[activeObjectIndex]->data;
        activeData->motion_state = 16;
        fn_1_7D08(1);
        if (MgModePlayNum == 78) {
            fn_1_8B1C(1);
        }
        if (MgModePlayNum == 80) {
            fn_1_8B1C(1);
        }
    } else {
        idlePlayer = lbl_1_data_360;
        idleObjectIndex = idlePlayer;
        idleData = lbl_1_bss_140[idleObjectIndex]->data;
        idleData->motion_state = 13;
    }
}

/* Called in submode 8 to wait for the player's movement state to settle. */
void fn_1_D3A8(void)
{
    s16 waitPlayer;
    s16 repeat;
    s16 finished;
    MGRenshoPlayerState *playerData;
    s16 activePlayerIndex;
    s16 activePlayer;
    MGRenshoPlayerState *activeData;

    repeat = 1;
    while (repeat != 0) {
        repeat = 0;
        waitPlayer = lbl_1_data_360;
        playerData = lbl_1_bss_140[waitPlayer]->data;
        if ((playerData->motion_state == -1) ||
            (playerData->motion_state == 0) ||
            (playerData->motion_state == 12) ||
            (playerData->motion_state == 15) ||
            (playerData->motion_state == 18) ||
            (playerData->motion_state == 21) ||
            (playerData->motion_state == 9)) {
            finished = 1;
        } else {
            finished = 0;
        }
        if (finished == 0) {
            repeat = 1;
        }
        HuPrcVSleep();
    }

    HuPrcSleep(10);
    activePlayer = lbl_1_data_360;
    activePlayerIndex = activePlayer;
    activeData = lbl_1_bss_140[activePlayerIndex]->data;
    if ((activeData->motion_state == 10) ||
        (activeData->motion_state == 11) ||
        (activeData->motion_state == 12) ||
        (activeData->motion_state == 16) ||
        (activeData->motion_state == 17) ||
        (activeData->motion_state == 18)) {
        activeData->motion_state = 0;
    } else if ((activeData->motion_state == 13) ||
               (activeData->motion_state == 14) ||
               (activeData->motion_state == 15) ||
               (activeData->motion_state == 19) ||
               (activeData->motion_state == 20) ||
               (activeData->motion_state == 21)) {
        activeData->motion_state = 0;
    } else if ((activeData->motion_state == 7) ||
               (activeData->motion_state == 8) ||
               (activeData->motion_state == 9)) {
        activeData->motion_state = 0;
    }
    fn_1_7D08(0);
    HuPrcSleep(30);
}

/* Called in submode 9 to finish the turn, run mode scripts, and resume play. */
void fn_1_D5A0(void)
{
    s16 eventWait;
    s16 activePlayer;

    if (MgModePlayNum == 82) {
        fn_1_4D58();
    }
    eventWait = 1;
    while (eventWait != 0) {
        eventWait = 0;
        if (fn_1_4E58() == 0) {
            eventWait = 1;
        }
        HuPrcVSleep();
    }
    if (MgModePlayNum == 46) {
        fn_1_4C4C();
        fn_1_4C70();
    }
    if (MgModePlayNum == 47) {
        fn_1_4C84();
        fn_1_4CA8();
    }
    if (MgModePlayNum == 69) {
        fn_1_4D24();
    }
    if (MgModePlayNum == 74) {
        fn_1_4CF0();
    }
    if (MgModePlayNum == 61) {
        fn_1_4CBC();
    }
    if (MgModePlayNum == 90) {
        fn_1_4D8C();
    }
    if (MgModePlayNum == 31) {
        HuAudSStreamFadeOut(lbl_1_data_500, 1000);
        lbl_1_data_504 = HuAudJinglePlay(MSM_STREAM_MGMUS_DONKEY);
        HuAudSStreamParamSet(lbl_1_data_504, 0U, 0U);
        HuAudSStreamParamSet(lbl_1_data_504, 127U, 1000U);
    }
    if (MgModePlayNum == 41) {
        HuAudSStreamFadeOut(lbl_1_data_504, 1000);
        lbl_1_data_508 = HuAudJinglePlay(MSM_STREAM_MGMUS_7);
        HuAudSStreamParamSet(lbl_1_data_508, 0U, 0U);
        HuAudSStreamParamSet(lbl_1_data_508, 127U, 1000U);
    }
    if (MgModePlayNum == 61) {
        HuAudSStreamFadeOut(lbl_1_data_508, 1000);
        lbl_1_data_50C = HuAudJinglePlay(MSM_STREAM_MGMUS_8);
        HuAudSStreamParamSet(lbl_1_data_50C, 0U, 0U);
        HuAudSStreamParamSet(lbl_1_data_50C, 127U, 1000U);
    }
    if (MgModePlayNum == 81) {
        HuAudSStreamFadeOut(lbl_1_data_50C, 1000);
        lbl_1_data_510 = HuAudJinglePlay(MSM_STREAM_MGMUS_9);
        HuAudSStreamParamSet(lbl_1_data_510, 0U, 0U);
        HuAudSStreamParamSet(lbl_1_data_510, 127U, 1000U);
    }
    fn_1_7E34();
    if ((MgModePlayNum == 4) || (MgModePlayNum == 24) ||
        (MgModePlayNum == 34) || (MgModePlayNum == 88)) {
        fn_1_7D94(1, 40);
    }
    activePlayer = lbl_1_data_360;
    fn_1_A34C(activePlayer);
    fn_1_7D94(0, 30);
    fn_1_7E5C();
    if (MgModePlayNum == 78) {
        fn_1_8B1C(0);
    }
    if (MgModePlayNum == 80) {
        fn_1_8B1C(0);
    }
    HuPrcSleep(30);
    if (MgModePlayNum == 90) {
        eventWait = 1;
        while (eventWait != 0) {
            eventWait = 0;
            if (fn_1_4E00() == 0) {
                eventWait = 1;
            }
            HuPrcVSleep();
        }
    }
}

/* In submode 12, plays the final animation and stream only after the full streak is completed. */
void fn_1_D964(void)
{
    s16 wait;

    if ((MgModeWorkInt[0] != 0) && (MgModePlayNum >= MgGameNo)) {
        fn_1_AC3C(fn_1_A00C());
        CharFXPlay(fn_1_A01C(fn_1_A00C()), MGR_RENSHO_FALL_EFFECT);
        fn_1_7D94(4, 40);
        fn_1_106C(3);
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(fn_1_A00C()) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
        lbl_1_data_518 = HuAudSStreamPlay(MSM_STREAM_MGMUS_10);
        fn_1_AB9C(fn_1_A00C(), 1);
        fn_1_7D08(1);
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_AD78(fn_1_A00C()) == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
        fn_1_4E24();
        wait = 1;
        while (wait != 0) {
            wait = 0;
            if (fn_1_4E58() == 0) {
                wait = 1;
            }
            HuPrcVSleep();
        }
        HuPrcSleep(30);
        fn_1_7D08(0);
        return;
    }
    HuPrcVSleep();
}
