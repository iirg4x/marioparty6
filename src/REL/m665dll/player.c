/* Controls the Verbal Assault minigame's vehicle, effects, and stage models. */
#include "REL/m665dll/player.h"
#include "REL/m665dll/stage.h"
#include "math.h"
#include "REL/m665dll/sequence.h"

#include "dolphin/types.h"

#include "dolphin/os.h"

#include "dolphin/gx.h"

#include "dolphin/mtx.h"

#include "string.h"

#include "dolphin.h"

#include "humath.h"

#include "datadir_enum.h"

#include "messdir_enum.h"

#include "game/hu3d.h"

#include "game/object.h"

#include "game/gamework.h"

#include "game/audio.h"

#include "game/sprite.h"

#include "game/memory.h"

#include "game/data.h"

#include "game/main.h"

#include "game/pad.h"

#include "game/window.h"

#include "game/wipe.h"

#include "game/frand.h"

#include "game/charman.h"

#include "game/process.h"

#include "game/hsfex.h"

#include "game/gamemes.h"

#include "game/flag.h"

#include "game/armem.h"

#include "game/mgdata.h"

#include "game/printfunc.h"

#include "game/saveload.h"

#include "game/esprite.h"

#include "game/thpmain.h"

#include "game/mg/seqman.h"

#include "game/mg/actman.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/board/object.h"

#include "game/board/main.h"

#include "game/board/audio.h"

#include "game/board/tutorial.h"

#include "game/board/masu.h"

#include "game/board/player.h"

typedef struct HookNameTable {
    char *names[6]; /* Vehicle attachment names for the six hook effects. */
} HookNameTable;

typedef struct ChildNameTable {
    char *names[2]; /* Vehicle attachment names for the two propellers. */
} ChildNameTable;

typedef struct RandomValueTable {
    s16 values[4]; /* Selection-frame bases indexed by difficulty. */
} RandomValueTable;

#define M665_MEMORY_TAG (1 << 28)

void fn_1_6E8C(void);
void fn_1_5470(void);
void fn_1_98AC(void);
void fn_1_6188(void);
void fn_1_AD40(void);
void fn_1_8E90(void);
void fn_1_B09C(void);

s32 lbl_1_data_100[7] = {
    DATANUM(0x7c0000, 14), DATANUM(0x7c0000, 15), DATANUM(0x7c0000, 16), DATANUM(0x7c0000, 17),
    DATANUM(0x7c0000, 18), DATANUM(0x7c0000, 19), DATANUM(0x7c0000, 20),
};

/* Builds the vehicle and attached effects during the sequence initialization hook. */
void fn_1_1C60(void) {
    M665PlayerWork *playerWork;
    s16 mainPlayerIndex;
    s16 modelId;
    s16 motionId;
    s32 index;
    struct HookNameTable hookNames = { { "itemhook_lc1", "itemhook_lc2", "itemhook_lc3",
                                         "itemhook_lc4", "itemhook_lc5", "itemhook_lc6" } };
    struct ChildNameTable childNames = {{"itemhook_propeller1", "itemhook_propeller2"}};
    struct RandomValueTable randomValues = {{180, 120, 60, 30}};

    mainPlayerIndex = lbl_1_bss_250.mainPlayer;
    playerWork = &lbl_1_bss_10[mainPlayerIndex];
    modelId = fn_80025A04(fn_80008DD0(DATANUM(0x7c0000, 12), M665_MEMORY_TAG,
                                                    2));
    fn_80026760(modelId, 0.0f, 0.0f,
                    -2000.0f);
    fn_80026E44(modelId);
    playerWork->vehicleModel = modelId;
    for (index = 0; index < 7; index++) {
        motionId = fn_80030458(modelId,
            fn_80008DD0(lbl_1_data_100[index], M665_MEMORY_TAG,
                                 2));
        playerWork->models.main.motions[index] = motionId;
    }
    fn_8002C95C(modelId, playerWork->models.main.motions[0]);
    fn_8002D86C(modelId, 0.0f);
    fn_8002741C(playerWork->vehicleModel, "itemhook", playerWork->characterModel);
    for (index = 0; index < 2; index++) {
        modelId = fn_80025A04(fn_80008DD0(DATANUM(0x7c0000, 13),
                                                       M665_MEMORY_TAG,
                                                       2));
        fn_8002741C(playerWork->vehicleModel, childNames.names[index],
                         modelId);
        fn_800269E8(modelId, ((1 << 30) | (1 << 0)));
        fn_80026E44(modelId);
        playerWork->models.main.propellers[index] = modelId;
    }
    for (index = 0; index < 6; index++) {
        modelId = fn_80025A04(
            fn_80008DD0(DATANUM(0x7c0000, 21), M665_MEMORY_TAG, 2));
        fn_8002741C(playerWork->vehicleModel, hookNames.names[index],
                         modelId);
        fn_8002D86C(modelId, 0.0f);
        playerWork->models.main.hookModels[index] = modelId;
        modelId = fn_80025A04(
            fn_80008DD0(DATANUM(0x7c0000, 22), M665_MEMORY_TAG, 2));
        fn_800269E8(modelId, (1 << 0));
        fn_8002D86C(modelId, 0.0f);
        playerWork->models.main.hookEffects[index] = modelId;
    }
    for (index = 0; index < 2; index++) {
        modelId = fn_80025A04(
            fn_80008DD0(DATANUM(0x7c0000, 34), M665_MEMORY_TAG, 2));
        fn_800269E8(modelId, (1 << 0));
        fn_800269E8(modelId, ((1 << 30) | (1 << 0)));
        fn_800268B0(modelId, 0.7f,
                          0.0f, 0.7f);
        fn_80026CBC(modelId, 0.5f);
        playerWork->models.main.damageModels[index] = modelId;
    }
    fn_1_6E8C();
    fn_1_5470();
    fn_1_98AC();
    fn_1_6188();
    fn_1_AD40();
    fn_1_8E90();
    fn_1_B09C();
    /* This setup also indexes the delay table with -1 for a human main player. */
    playerWork->effect.main.selectionFrames = randomValues.values[playerWork->difficulty] +
                       frandmod(randomValues.values[playerWork->difficulty]);
}

/* Starts the main player's vehicle movement when the sequence fade-in hook runs. */
void fn_1_1FFC(void)
{
    s16 currentPlayer = lbl_1_bss_250.mainPlayer;
    M665PlayerWork *playerWork = &lbl_1_bss_10[currentPlayer];

    playerWork->status.main.flags.bits.moving = 1;
    playerWork->speed = 5.0f;
}

void fn_1_170C(s16 player, s16 mode, f32 frame);

/* Forwards the main player's character motion and blend duration during action or winner
 * updates. */
void fn_1_2058(s16 motion, f32 blendDuration)
{
    s16 player = lbl_1_bss_250.mainPlayer;

    fn_1_170C(player, motion, blendDuration);
}

/* Updates the main player's action and clears other-player movement overrides when idle. */
void fn_1_20A0(s16 state)
{
    M665PlayerWork *playerWork;

    playerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    /* Both sequence-mode branches assign the requested action unchanged. */
    if (fn_80071A8C() <= 5U) {
        playerWork->action = state;
    } else {
        playerWork->action = state;
    }

    if (state == 0) {
        s32 playerIndex;

        for (playerIndex = 0; playerIndex < 3; playerIndex++) {
            lbl_1_bss_10[lbl_1_bss_250.otherPlayers[playerIndex]].movementOverride = 0;
        }
    }
}

s16 fn_1_8D54(void);

void fn_1_9AE8(void);

/* Advances the main player's vehicle actions and their timed stage effects each object update. */
void fn_1_2158(void) {
    M665LaserWork *laser;
    s32 modelIndex;
    Point3d hookPosition;
    Point3d laserPosition;
    Point3d laserTravelPosition;
    Point3d laserTrailPosition;
    s16 mainPlayerIndex;
    s16 selectedKillerIndex;
    s16 rollingEffectIndex;
    f32 motionMaxTime;
    M665PlayerWork *playerWork;

    mainPlayerIndex = lbl_1_bss_250.mainPlayer;
    playerWork = &lbl_1_bss_10[mainPlayerIndex];
    laser = &lbl_1_bss_2E8.laser;
    switch (playerWork->action) {
        case 1:
            motionMaxTime = fn_8005C1FC(playerWork->character);
            if (fn_8005C1B8(playerWork->character) >= motionMaxTime) {
                fn_1_2058(0, 8.0f);
                HuAudFXPlay(0x86E);
                fn_8002C95C(playerWork->vehicleModel, playerWork->models.main.motions[0]);
                fn_8002D414(playerWork->vehicleModel, 0.0f);
                fn_8002D86C(playerWork->vehicleModel, 1.0f);
                fn_1_20A0(2);
                return;
            }
            break;
        case 2:
            if (fn_8002D4E0(playerWork->vehicleModel) >= 40.0f) {
                lbl_1_bss_2E8.fireEnabled = 1;
                lbl_1_bss_2E8.fireSoundHandle = HuAudFXPlay(0x86F);
                if (frandmod(2) == 0) {
                    fn_8002D414(playerWork->vehicleModel, 50.0f);
                    fn_1_20A0(4);
                    return;
                }
                fn_8002D414(playerWork->vehicleModel, 150.0f);
                fn_800269E8(playerWork->vehicleModel, ((1 << 30) | (1 << 2)));
                fn_1_20A0(3);
                return;
            }
            break;
        case 3:
            if (fn_8002D4E0(playerWork->vehicleModel) <= 52.0f) {
                fn_8002D414(playerWork->vehicleModel, 150.0f);
                fn_1_20A0(5);
                return;
            }
            break;
        case 4:
            if (fn_8002D4E0(playerWork->vehicleModel) >= 148.0f) {
                fn_8002D414(playerWork->vehicleModel, 50.0f);
                fn_1_20A0(6);
                return;
            }
            break;
        case 5:
            if (fn_8002D4E0(playerWork->vehicleModel) <= 50.0f) {
                fn_8002D414(playerWork->vehicleModel, 40.0f);
                fn_1_20A0(7);
            }
            if (fn_8002D4E0(playerWork->vehicleModel) == 55.0f) {
                lbl_1_bss_2E8.fireEnabled = 0;
                HuAudFXStop(lbl_1_bss_2E8.fireSoundHandle);
                return;
            }
            break;
        case 6:
            if (fn_8002D4E0(playerWork->vehicleModel) >= 150.0f) {
                fn_800269E8(playerWork->vehicleModel, ((1 << 30) | (1 << 2)));
                fn_8002D414(playerWork->vehicleModel, 40.0f);
                fn_1_20A0(7);
                lbl_1_bss_2E8.fireEnabled = 0;
                HuAudFXStop(lbl_1_bss_2E8.fireSoundHandle);
            }
            if (fn_8002D4E0(playerWork->vehicleModel) == 135.0f) {
                lbl_1_bss_2E8.fireEnabled = 0;
                HuAudFXStop(lbl_1_bss_2E8.fireSoundHandle);
                return;
            }
            break;
        case 7:
            if (fn_8002D75C(playerWork->vehicleModel) == 1) {
                fn_80026A44(playerWork->vehicleModel, ((1 << 30) | (1 << 2)));
                fn_8002D414(playerWork->vehicleModel, 0.0f);
                fn_8002D86C(playerWork->vehicleModel, 0.0f);
                fn_1_20A0(0);
                return;
            }
            if (fn_8002D4E0(playerWork->vehicleModel) == 20.0f) {
                HuAudFXPlay(0x879);
                return;
            }
            break;
        case 13:
            motionMaxTime = fn_8005C1FC(playerWork->character);
            if (fn_8005C1B8(playerWork->character) >= motionMaxTime) {
                fn_1_2058(0, 8.0f);
                fn_8002C95C(playerWork->vehicleModel, playerWork->models.main.motions[1]);
                fn_8002D414(playerWork->vehicleModel, 0.0f);
                fn_8002D86C(playerWork->vehicleModel, 1.0f);
                fn_1_20A0(14);
                HuAudFXPlay(0x86E);
                return;
            }
            break;
        case 14:
            if (fn_8002D4E0(playerWork->vehicleModel) >= 49.0f) {
                fn_1_20A0(15);
                for (modelIndex = 0; modelIndex < 6; modelIndex++) {
                    fn_80026A44(lbl_1_bss_2E8.missiles[modelIndex].modelID, 1U);
                    lbl_1_bss_2E8.missiles[modelIndex].state = 1;
                }
                HuAudFXPlay(0x871);
                playerWork->actionFrames = 75;
                return;
            }
            break;
        case 15:
            if (playerWork->actionFrames-- <= 0) {
                playerWork->actionFrames = 0;
                fn_1_20A0(16);
                fn_800269E8(playerWork->vehicleModel, ((1 << 30) | (1 << 2)));
                fn_8002D414(playerWork->vehicleModel, 49.0f);
                HuAudFXPlay(0x879);
                playerWork->status.main.flags.bits.moving = 1;
                return;
            }
            break;
        case 16:
            if (fn_8002D75C(playerWork->vehicleModel) == 1) {
                fn_80026A44(playerWork->vehicleModel, ((1 << 30) | (1 << 2)));
                fn_8002D414(playerWork->vehicleModel, 0.0f);
                fn_8002D86C(playerWork->vehicleModel, 0.0f);
                fn_1_20A0(0);
                return;
            }
            break;
        case 8:
            motionMaxTime = fn_8005C1FC(playerWork->character);
            if (fn_8005C1B8(playerWork->character) >= motionMaxTime) {
                fn_1_2058(0, 8.0f);
                fn_8002C95C(playerWork->vehicleModel, playerWork->models.main.motions[2]);
                fn_8002D86C(playerWork->vehicleModel, 1.0f);
                fn_8002D414(playerWork->vehicleModel, 0.0f);
                fn_1_20A0(9);
                lbl_1_bss_2E8.laser.scale = 0.0f;
                HuAudFXPlay(0x86E);
                return;
            }
            break;
        case 9:
            if (fn_8002D4E0(playerWork->vehicleModel) >= 39.0f) {
                if (laser->scale == 0.0f) {
                    char *laserHookNames[2] = {
                        "itemhook_laser1", "itemhook_laser2"
                    };
                    for (modelIndex = 0; modelIndex < 2; modelIndex++) {
                        Hu3DModelObjPosGet(playerWork->vehicleModel,
                                            laserHookNames[modelIndex], &hookPosition);
                        fn_80026790(laser->modelIDs[modelIndex], &hookPosition);
                        fn_80026A44(laser->modelIDs[modelIndex], 1U);
                    }
                    HuAudFXPlay(0x870);
                } else {
                    for (modelIndex = 0; modelIndex < 2; modelIndex++) {
                        fn_800267CC(laser->modelIDs[modelIndex], &laserPosition);
                        laserPosition.x +=
                            -playerWork->speed +
                            (2.0f *
                             (playerWork->speed *
                              (f32) (u8) playerWork->status.main.flags.bits.positiveDirection));
                        fn_80026790(laser->modelIDs[modelIndex], &laserPosition);
                    }
                }
                laser->scale += 0.1f;
                if (laser->scale >= 1.0f) {
                    laser->scale = 1.0f;
                    fn_1_20A0(10);
                    laser->timer = 30;
                }
                for (modelIndex = 0; modelIndex < 2; modelIndex++) {
                    fn_800268B0(laser->modelIDs[modelIndex], laser->scale, laser->scale,
                                      laser->scale);
                }
                return;
            }
            break;
        case 10:
            if ((f32) laser->timer-- <= 0.0f) {
                fn_800269E8(playerWork->vehicleModel, ((1 << 30) | (1 << 2)));
                fn_1_20A0(11);
            }
            for (modelIndex = 0; modelIndex < 2; modelIndex++) {
                fn_800267CC(laser->modelIDs[modelIndex], &laserTravelPosition);
                laserTravelPosition.x +=
                    -playerWork->speed +
                    (2.0f * (playerWork->speed *
                             (f32) (u8) playerWork->status.main.flags.bits.positiveDirection));
                fn_80026790(laser->modelIDs[modelIndex], &laserTravelPosition);
            }
            return;
        case 11:
            laser->scale -= 0.1f;
            if (laser->scale <= 0.0f) {
                laser->scale = 0.0f;
                fn_1_20A0(12);
                HuAudFXPlay(0x879);
            }
            for (modelIndex = 0; modelIndex < 2; modelIndex++) {
                fn_800267CC(laser->modelIDs[modelIndex], &laserTrailPosition);
                laserTrailPosition.x +=
                    -playerWork->speed +
                    (2.0f * (playerWork->speed *
                             (f32) (u8) playerWork->status.main.flags.bits.positiveDirection));
                fn_80026790(laser->modelIDs[modelIndex], &laserTrailPosition);
                fn_800268B0(laser->modelIDs[modelIndex], laser->scale, laser->scale,
                                  2.0f * laser->scale);
            }
            return;
        case 12:
            if (fn_8002D75C(playerWork->vehicleModel) == 1) {
                fn_80026A44(playerWork->vehicleModel, ((1 << 30) | (1 << 2)));
                fn_8002D414(playerWork->vehicleModel, 0.0f);
                fn_8002D86C(playerWork->vehicleModel, 0.0f);
                fn_1_20A0(0);
                playerWork->effect.main.effectState = 0;
                return;
            }
            break;
        case 23:
            motionMaxTime = fn_8005C1FC(playerWork->character);
            if (fn_8005C1B8(playerWork->character) >= motionMaxTime) {
                fn_1_2058(0, 8.0f);
                fn_8002C95C(playerWork->vehicleModel, playerWork->models.main.motions[4]);
                fn_8002D414(playerWork->vehicleModel, 0.0f);
                fn_8002D86C(playerWork->vehicleModel, 1.0f);
                fn_1_20A0(24);
        selectedKillerIndex = lbl_1_bss_2E8.selectedKiller = fn_1_8D54();
        lbl_1_bss_2E8.killers[selectedKillerIndex].state = 1;
                return;
            }
            break;
        case 24:
            if (fn_8002D4E0(playerWork->vehicleModel) >= 44.0f) {
                fn_1_20A0(25);
                lbl_1_bss_2E8.killers[lbl_1_bss_2E8.selectedKiller].state = 2;
                return;
            }
            if (fn_8002D4E0(playerWork->vehicleModel) == 20.0f) {
                HuAudFXPlay(0x873);
                return;
            }
            break;
        case 25:
            if (lbl_1_bss_2E8.killers[lbl_1_bss_2E8.selectedKiller].state >= 4) {
                fn_1_20A0(26);
                playerWork->actionFrames = 60;
                playerWork->status.main.flags.bits.moving = 1;
                return;
            }
            break;
        case 26:
            if (playerWork->actionFrames-- <= 0) {
                fn_800269E8(playerWork->vehicleModel, ((1 << 30) | (1 << 2)));
                fn_8002D414(playerWork->vehicleModel, 44.0f);
                fn_1_20A0(27);
                return;
            }
            break;
        case 27:
            if (fn_8002D75C(playerWork->vehicleModel) == 1) {
                fn_80026A44(playerWork->vehicleModel, ((1 << 30) | (1 << 2)));
                fn_8002D414(playerWork->vehicleModel, 0.0f);
                fn_8002D86C(playerWork->vehicleModel, 0.0f);
                fn_1_20A0(0);
                return;
            }
            if (fn_8002D4E0(playerWork->vehicleModel) == 40.0f) {
                HuAudFXPlay(0x879);
                return;
            }
            break;
        case 17:
            motionMaxTime = fn_8005C1FC(playerWork->character);
            if (fn_8005C1B8(playerWork->character) >= motionMaxTime) {
                fn_1_2058(0, 8.0f);
                fn_8002C95C(playerWork->vehicleModel, playerWork->models.main.motions[3]);
                fn_8002D414(playerWork->vehicleModel, 0.0f);
                fn_8002D86C(playerWork->vehicleModel, 1.0f);
                fn_1_20A0(18);
                playerWork->actionFrames = 0;
                return;
            }
            break;
        case 18:
            if (fn_8002D4E0(playerWork->vehicleModel) >= 50.0f) {
                fn_8002D414(playerWork->vehicleModel, 60.0f);
                fn_8002D86C(playerWork->vehicleModel, 2.0f);
                fn_1_20A0(19);
                return;
            }
            if (fn_8002D4E0(playerWork->vehicleModel) == 20.0f) {
                HuAudFXPlay(0x873);
                return;
            }
            break;
        case 19:
            if (fn_8002D4E0(playerWork->vehicleModel) >= 84.0f) {
                fn_8002D414(playerWork->vehicleModel, 60.0f);
                playerWork->actionFrames += 1;
                if (playerWork->actionFrames >= 6) {
                fn_1_20A0(20);
                    return;
                }
            } else if ((fn_8002D4E0(playerWork->vehicleModel) > 64.0f) &&
                       (fn_8002D4E0(playerWork->vehicleModel) <= 66.0f)) {
                fn_1_9AE8();
                return;
            }
            break;
        case 20:
            if (fn_8002D4E0(playerWork->vehicleModel) >= 84.0f) {
                playerWork->actionFrames = 30;
                fn_1_20A0(21);
                return;
            }
            if ((fn_8002D4E0(playerWork->vehicleModel) > 64.0f) &&
                (fn_8002D4E0(playerWork->vehicleModel) <= 66.0f)) {
                fn_1_9AE8();
                return;
            }
            break;
        case 21:
            if (playerWork->actionFrames-- <= 0) {
                playerWork->actionFrames = 0;
                fn_1_20A0(22);
                fn_8002D414(playerWork->vehicleModel, 50.0f);
                fn_8002D86C(playerWork->vehicleModel, 1.0f);
                fn_800269E8(playerWork->vehicleModel, ((1 << 30) | (1 << 2)));
                return;
            }
            break;
        case 22:
            if (fn_8002D4E0(playerWork->vehicleModel) <= 1.0f) {
                fn_80026A44(playerWork->vehicleModel, ((1 << 30) | (1 << 2)));
                fn_8002D414(playerWork->vehicleModel, 0.0f);
                fn_8002D86C(playerWork->vehicleModel, 0.0f);
                fn_1_20A0(0);
                playerWork->status.main.flags.bits.moving = 1;
                return;
            }
            if (fn_8002D4E0(playerWork->vehicleModel) == 40.0f) {
                HuAudFXPlay(0x879);
                return;
            }
            break;
        case 28:
            motionMaxTime = fn_8005C1FC(playerWork->character);
            if (fn_8005C1B8(playerWork->character) >= motionMaxTime) {
                fn_1_2058(0, 8.0f);
                fn_8002C95C(playerWork->vehicleModel, playerWork->models.main.motions[5]);
                fn_8002D414(playerWork->vehicleModel, 0.0f);
                fn_8002D86C(playerWork->vehicleModel, 1.0f);
                fn_1_20A0(29);
                return;
            }
            break;
        case 29:
            if (fn_8002D4E0(playerWork->vehicleModel) >= 39.0f) {
                fn_1_20A0(30);
                return;
            }
            if (fn_8002D4E0(playerWork->vehicleModel) == 20.0f) {
                HuAudFXPlay(0x873);
                return;
            }
            break;
        case 30:
            if ((s32) (lbl_1_bss_2E8.rollingFrames % 10) == 0) {
                rollingEffectIndex = (s16) (lbl_1_bss_2E8.rollingFrames / 10);
                if (rollingEffectIndex > 0) {
                    lbl_1_bss_2E8.rolling[rollingEffectIndex - 1].state = 1;
                }
            }
            if (lbl_1_bss_2E8.rollingFrames-- <= 0) {
                fn_1_20A0(31);
                playerWork->actionFrames = 20;
                return;
            }
            break;
        case 31:
            if (playerWork->actionFrames-- <= 0) {
                playerWork->actionFrames = 0;
                fn_1_20A0(32);
                fn_800269E8(playerWork->vehicleModel, ((1 << 30) | (1 << 2)));
                fn_8002D414(playerWork->vehicleModel, 39.0f);
                return;
            }
            break;
        case 32:
            if (fn_8002D75C(playerWork->vehicleModel) == 1) {
                fn_80026A44(playerWork->vehicleModel, ((1 << 30) | (1 << 2)));
                fn_8002D414(playerWork->vehicleModel, 0.0f);
                fn_8002D86C(playerWork->vehicleModel, 0.0f);
                fn_1_20A0(0);
                return;
            }
            if (fn_8002D4E0(playerWork->vehicleModel) == 20.0f) {
                HuAudFXPlay(0x87A);
                return;
            }
            break;
        case 33:
            fn_8002C95C(playerWork->vehicleModel, playerWork->models.main.motions[6]);
            fn_8002D414(playerWork->vehicleModel, 0.0f);
            fn_8002D86C(playerWork->vehicleModel, 1.0f);
                fn_1_20A0(34);
            return;
        case 34:
            if (fn_8002D4E0(playerWork->vehicleModel) == 15.0f) {
                lbl_1_bss_2E8.bouncing[0].state = 1;
                lbl_1_bss_2E8.bouncing[3].state = 1;
                HuAudFXPlay(0x872);
                return;
            }
            if (fn_8002D4E0(playerWork->vehicleModel) == 38.0f) {
                lbl_1_bss_2E8.bouncing[4].state = 1;
                HuAudFXPlay(0x872);
                return;
            }
            if (fn_8002D4E0(playerWork->vehicleModel) == 62.0f) {
                lbl_1_bss_2E8.bouncing[2].state = 1;
                HuAudFXPlay(0x872);
                return;
            }
            if (fn_8002D4E0(playerWork->vehicleModel) == 74.0f) {
                fn_1_20A0(35);
                fn_8002D414(playerWork->vehicleModel, 28.0f);
                return;
            }
            break;
        case 35:
            if (fn_8002D4E0(playerWork->vehicleModel) == 38.0f) {
                HuAudFXPlay(0x872);
                lbl_1_bss_2E8.bouncing[1].state = 1;
                lbl_1_bss_2E8.bouncing[5].state = 1;
                return;
            }
            if (fn_8002D4E0(playerWork->vehicleModel) == 52.0f) {
                fn_1_20A0(36);
                fn_8002D414(playerWork->vehicleModel, 52.0f);
                return;
            }
            break;
        case 36:
            motionMaxTime = fn_8002D530(playerWork->vehicleModel);
            if (fn_8002D4E0(playerWork->vehicleModel) >= motionMaxTime) {
                fn_8002D86C(playerWork->vehicleModel, 0.0f);
                fn_8002D414(playerWork->vehicleModel, 0.0f);
                fn_1_2058(0, 8.0f);
                fn_1_20A0(0);
            }
            break;
    }
}

void fn_1_5820(void);

void fn_1_648C(void);

void fn_1_8378(void);

void fn_1_AE24(void);

void fn_1_9CF0(void);

void fn_1_8FC8(void);

void fn_1_B1E8(void);

/* Advances the fire, missile, killer, laser, hazard, rolling, and bouncing updates together. */
void fn_1_41C8(void)
{
    fn_1_5820();
    fn_1_648C();
    fn_1_8378();
    fn_1_AE24();
    fn_1_9CF0();
    fn_1_8FC8();
    fn_1_B1E8();
}

typedef struct {
    char *names[6]; /* Vehicle attachment names for the six hook effects. */
} HookNames;

const HookNames lbl_1_rodata_1A8 = {{
    "itemhook_lc1", "itemhook_lc2", "itemhook_lc3",
    "itemhook_lc4", "itemhook_lc5", "itemhook_lc6",
}};

/* Updates vehicle damage models and attachments from fn_1_4768's object update. */
void fn_1_4200(void) {
    M665PlayerWork *playerWork;
    s32 index;
    HookNames hookNames;
    HuVecF scale;
    f32 maxTime;

    playerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    hookNames = lbl_1_rodata_1A8;
    if (playerWork->models.main.damage > 220) {
        fn_80026A44(playerWork->models.main.damageModels[0], (1 << 0));
        fn_8002691C(playerWork->models.main.damageModels[0], &scale);
        if (scale.y < 0.7) {
            scale.y += 0.01;
            fn_800268E0(playerWork->models.main.damageModels[0], &scale);
        }
    } else if (playerWork->models.main.damage > 100) {
        fn_80026A44(playerWork->models.main.damageModels[1], (1 << 0));
        fn_8002691C(playerWork->models.main.damageModels[1], &scale);
        if (scale.y < 0.7) {
            scale.y += 0.01;
            fn_800268E0(playerWork->models.main.damageModels[1], &scale);
        }
    }
    for (index = 0; index < 6; index++) {
        if (playerWork->models.main.damage >= (index + 1) * 50 &&
            playerWork->models.main.brokenHooks <= index) {
            playerWork->models.main.brokenHooks += 1;
            HuAudFXPlay(0x86D);
            fn_800269E8(playerWork->models.main.hookModels[index], (1 << 0));
            fn_800275B4(playerWork->vehicleModel, hookNames.names[index]);
            fn_8002741C(playerWork->vehicleModel, hookNames.names[index],
                             playerWork->models.main.hookEffects[index]);
            fn_8002D86C(playerWork->models.main.hookEffects[index],
                               1.0f);
            fn_80026A44(playerWork->models.main.hookEffects[index], (1 << 0));
            if (HuMCProbe(0) != 0 && HuMCProbe(1) != 0) {
                if (playerWork->models.main.brokenHooks < 6) {
                    fn_8006A030(lbl_1_bss_250.mainPlayer, 20, 7, 3);
                } else {
                    fn_8006A030(lbl_1_bss_250.mainPlayer, 60, 20, 0);
                }
            }
        } else if (playerWork->models.main.damage >= (index + 1) * 50 - 16) {
            fn_8002D414(playerWork->models.main.hookModels[index],
                              30.0f);
        } else if (playerWork->models.main.damage >= (index + 1) * 50 - 32) {
            fn_8002D414(playerWork->models.main.hookModels[index],
                              15.0f);
        }
        maxTime = fn_8002D530(
            playerWork->models.main.hookEffects[index]);
        if (fn_8002D4E0(
                playerWork->models.main.hookEffects[index]) >= maxTime) {
            fn_800269E8(playerWork->models.main.hookEffects[index], (1 << 0));
        }
    }
}

s32 frandmod(s32 max);

/* Selects a CPU attack flag using difficulty weights when its selection delay expires. */
u16 fn_1_4538(M665PlayerWork *work) {
    s16 weightTable[4][7] = {
        {10, 20, 30, 40, 50, 60, 100},
        {15, 25, 35, 50, 60, 70, 100},
        {20, 35, 50, 65, 85, 95, 100},
        {20, 35, 50, 80, 90, 100, 100}
    };
    u16 attackFlags[7] = {(1 << 8), (1 << 9), (1 << 10), 1, (1 << 11), (1 << 14), (1 << 13)};
    u16 delayRanges[4] = {180, 120, 60, 30};
    s16 roll;
    s32 attackIndex;

    if (work->effect.main.selectionFrames-- > 0) {
        return 0;
    }
    work->effect.main.selectionFrames = delayRanges[work->difficulty] +
                 frandmod(delayRanges[work->difficulty]);
    roll = frandmod(100);
    attackIndex = 0;
    while (attackIndex < 7) {
        if (roll < weightTable[work->difficulty][attackIndex]) {
            return attackFlags[attackIndex];
        }
        attackIndex++;
    }
    return 0;
}

extern u16 lbl_1_bss_0;

extern void fn_1_60CC(u16 flags);

extern void fn_1_6DE0(u16 flags);

extern void fn_1_8DA4(u16 flags);

extern void fn_1_9740(u16 flags);

extern void fn_1_AC94(u16 flags);

extern void fn_1_AFF0(u16 flags);

extern void fn_1_B5C4(u16 flags);

void fn_1_2158(void);

extern void fn_1_4200(void);

struct ChanceDurationTable {
    u16 value[4]; /* never read by name */
};

struct ChanceSelectionTable {
    u16 value[7]; /* never read by name */
};

struct ChanceLimitTable {
    s16 value[4][7]; /* never read by name */
};

/* Advances the main vehicle and its effects from fn_1_17E0's object update. */
void fn_1_4768(void) {
    HuVecF position;
    HuVecF rotation;
    M665PlayerWork *playerWork;
    s16 activePlayer;
    s16 sequenceMode;
    s16 frameNumber;
    u16 attackFlags;
    f32 motionSpeed;
    f32 verticalVelocity;

    activePlayer = lbl_1_bss_250.mainPlayer;
    playerWork = &lbl_1_bss_10[activePlayer];
    sequenceMode = fn_80071A8C();
    fn_800267CC(playerWork->vehicleModel, &position);
    if (playerWork->models.main.damage >= 300 && sequenceMode <= 6) {
        if (playerWork->speed > 0.0f) {
            playerWork->speed -= 0.1f;
        }
        if (playerWork->speed < 0.0f) {
            playerWork->speed = 0.0f;
        }
        if (playerWork->effect.main.soundHandle != -1) {
            HuAudFXFadeOut(playerWork->effect.main.soundHandle, 30);
            playerWork->effect.main.soundHandle = -1;
        }
        fn_80026874(playerWork->vehicleModel, &rotation);
        if (rotation.z < 5.0f) {
            rotation.z += 0.25f;
            if (rotation.z == 5.0f) {
                HuAudFXPlay(0x87C);
            }
        }
        if (rotation.z > 5.0f) {
            rotation.z = 5.0f;
        }
        if (position.y > -50.0f) {
            position.y -= 2.5f;
        }
        fn_80026838(playerWork->vehicleModel, &rotation);
        motionSpeed = (15.0f * playerWork->speed) / 5.0f;
        lbl_1_bss_250.scrollSpeed =
            (15.0f * playerWork->speed) / 5.0f;
        fn_8002D86C(playerWork->models.main.propellers[0], motionSpeed);
        fn_8002D86C(playerWork->models.main.propellers[1], motionSpeed);
    } else if (sequenceMode == 8 && lbl_1_bss_250.winningTeam == -1) {
        frameNumber = fn_80071DE4();
        motionSpeed = (100.0f - frameNumber) / 100.0f;
        if (motionSpeed < 0.0f) {
            motionSpeed = 0.0f;
        }
        fn_8002D86C(playerWork->models.main.propellers[0], motionSpeed);
        fn_8002D86C(playerWork->models.main.propellers[1], motionSpeed);
        position.y -= 1.0f;
        if (position.y < -55.0f) {
            position.y = -55.0f;
        }
    } else if (sequenceMode >= 7 && lbl_1_bss_250.winningTeam == 0) {
        playerWork->actionFrames += 3;
        position.y = (f32) ((f64) position.y -
            sin((3.141592653589793 * (f64) playerWork->actionFrames) / 180.0) / 2.0);
    }
    if (playerWork->status.main.flags.bits.moving != 0) {
        if (playerWork->status.main.flags.bits.positiveDirection != 0) {
            position.x += playerWork->speed;
            if (position.x >= 200.0f) {
                position.x = 200.0f;
                playerWork->status.main.flags.bits.positiveDirection = 0;
            }
        } else {
            position.x -= playerWork->speed;
            if (position.x <= -200.0f) {
                position.x = -200.0f;
                playerWork->status.main.flags.bits.positiveDirection = 1;
            }
        }
    }
    fn_80026790(playerWork->vehicleModel, &position);
    fn_80026760(playerWork->models.main.damageModels[0], position.x - 250.0f,
                    position.y + 500.0f, position.z - 350.0f);
    fn_80026760(playerWork->models.main.damageModels[1], position.x + 300.0f,
                    position.y + 400.0f, position.z + 50.0f);
    if (sequenceMode == 5) {
        if (playerWork->difficulty == -1) {
            attackFlags = lbl_1_bss_0;
        } else {
            attackFlags = fn_1_4538(playerWork);
        }
        lbl_1_bss_0 = 0;
        if (playerWork->models.main.actionCooldown == 0) {
            fn_1_60CC(attackFlags);
            fn_1_6DE0(attackFlags);
            fn_1_AFF0(attackFlags);
            fn_1_8DA4(attackFlags);
            fn_1_AC94(attackFlags);
            fn_1_9740(attackFlags);
            fn_1_B5C4(attackFlags);
        } else {
            playerWork->models.main.actionCooldown -= 1;
        }
    }
    fn_1_2158();
    fn_1_5820();
    fn_1_648C();
    fn_1_8378();
    fn_1_AE24();
    fn_1_9CF0();
    fn_1_8FC8();
    fn_1_B1E8();
    fn_1_4200();
    if ((f32) playerWork->models.main.damage >= 180.0f && fn_80071A8C() <= 6U) {
        fn_8002D86C(playerWork->models.main.propellers[0], playerWork->speed / 5.0f);
        fn_8002D86C(playerWork->models.main.propellers[1], playerWork->speed / 5.0f);
    }
}

extern void fn_1_11FD0(M665PlayerWork *playerWork);

/* Tests a point against the main vehicle's bounds for fn_1_112C0. */
s32 fn_1_4FA4(HuVecF *point) {
    HuVecF playerPosition;
    s16 currentPlayer;
    M665PlayerWork *playerWork;

    currentPlayer = lbl_1_bss_250.mainPlayer;
    playerWork = &lbl_1_bss_10[currentPlayer];
    fn_800267CC(playerWork->vehicleModel, &playerPosition);
    if (((120.0f + point->x) > (playerPosition.x - 350.0f)) &&
        ((point->x - 120.0f) < (350.0f + playerPosition.x)) &&
        ((150.0f + point->z) > (playerPosition.z - 600.0f)) &&
        ((point->z - 150.0f) < (180.0f + playerPosition.z))) {
        return 1;
    }
    return 0;
}

s32 fn_1_50D4(void) { return 0; }

/* During main-player winner setup, resets poses and awards 10 bonus coins outside practice. */
void fn_1_50DC(void) {
    s32 mainPlayerIndex;
    M665PlayerWork *playerWork;

    playerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    fn_800275B4((playerWork->vehicleModel), "itemhook");
    fn_80026808((playerWork->vehicleModel), 0.0f, 30.0f, 0.0f);
    fn_80026760((playerWork->vehicleModel), 0.0f, 0.0f, (-2000.0f));
    fn_80026760((playerWork->characterModel), (-300.0f), 0.0f, 300.0f);
    fn_80026E44((playerWork->characterModel));
    mainPlayerIndex = lbl_1_bss_250.mainPlayer;
    if (fn_8006A124(65551) == 0) {
        lbl_80265750[mainPlayerIndex].mgCoinBonus = 10;
    }
}

typedef struct ModelPositions {
    HuVecF position[2]; /* Positions of the two props created for opponent-winner presentation. */
} ModelPositions;

/* Repositions the vehicle, character, and props during the sequence pre-winner hook. */
void fn_1_51F8(void) {
    static const ModelPositions lbl_1_rodata_27C = { { { -100.0f, 500.0f, -2000.0f },
                                                       { -500.0f, 500.0f, -1500.0f } } };
    ModelPositions positions;
    M665PlayerWork *playerWork;
    s16 modelID;
    s32 i;

    playerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    positions = lbl_1_rodata_27C;
    fn_800275B4(playerWork->vehicleModel, "itemhook");
    fn_8002D86C(playerWork->models.main.propellers[0], 0.05f);
    fn_8002D86C(playerWork->models.main.propellers[1], 0.01f);
    fn_8002D414(playerWork->models.main.propellers[0], 0.0f);
    fn_8002D414(playerWork->models.main.propellers[1], 0.0f);
    fn_80026808(playerWork->vehicleModel, 5.0f, 30.0f,
                    15.0f);
    fn_80026760(playerWork->vehicleModel, 0.0f, 0.0f,
                    (-2000.0f));
    fn_80026760(playerWork->characterModel, (-500.0f), 0.0f,
                    (-500.0f));
    fn_80026E44(playerWork->characterModel);
    i = 0;
    while (i < 2) {
        modelID = fn_80025A04(
            fn_80008DD0(DATANUM(0x7c0000, 34), M665_MEMORY_TAG, 2));
        fn_80026790(modelID, &positions.position[i]);
        fn_800269E8(modelID, ((1 << 30) | (1 << 0)));
        i++;
    }
}

/* Detaches and repositions the vehicle and character during the pre-winner hook. */
void fn_1_53C0(void) {
    M665PlayerWork *playerWork;

    playerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    fn_800275B4((playerWork->vehicleModel), "itemhook");
    fn_80026760((playerWork->vehicleModel), 0.0f, 0.0f, (-2000.0f));
    fn_80026760((playerWork->characterModel), (-300.0f), 0.0f, (-200.0f));
    fn_80026E44((playerWork->characterModel));
}

/* Loads the fire-effect models during fn_1_1C60's sequence initialization. */
void fn_1_5470(void) {
    M665PlayerWork *playerWork;
    s16 modelID;
    s32 i;
    HSF_DATA *modelData;

    playerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    i = 0;
    while (i < 16) {
        modelID = fn_80025A04(
            fn_80008DD0(DATANUM(0x7c0000, 24), M665_MEMORY_TAG, 2));
        if (i == 0) {
            modelData = lbl_802C01C8[modelID].hsf;
            lbl_1_bss_2E0 = HuMemDirectMallocNum(
                HEAP_HEAP, modelData->materialNum * 60, M665_MEMORY_TAG);
            memcpy(lbl_1_bss_2E0, modelData->material,
                   modelData->materialNum * 60);
        }
        fn_800269E8(modelID, (1 << 0));
        lbl_1_bss_2E8.fire[i].modelA = modelID;
        lbl_1_bss_2E8.fire[i].active = 0;
        lbl_1_bss_2E8.fire[i].lifetime = 0;
    lbl_1_bss_2E8.fire[i].scale = 2.0f;
        i++;
    }
    i = 0;
    while (i < 16) {
        modelID = fn_80025A04(
            fn_80008DD0(DATANUM(0x7c0000, 24), M665_MEMORY_TAG, 2));
        fn_800269E8(modelID, (1 << 0));
        lbl_1_bss_2E8.fire[i].modelB = modelID;
        i++;
    }
}

void fn_1_5614(void);

/* Spawns one fire effect at a vehicle attachment when called by fn_1_5820. */
static inline void m665FireSpawn(void) {
    HuVecF modelPosition;
    HuVecF objectPosition;
    HuVecF direction;
    M665PlayerWork *playerWork;
    s32 i;

    playerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    fn_800267CC(playerWork->vehicleModel, &modelPosition);
    modelPosition.x += 240.0f;
    modelPosition.z += 200.0f;
    Hu3DModelObjPosGet(playerWork->vehicleModel, "itemhook_fire1", &objectPosition);
    direction.x = objectPosition.x - modelPosition.x;
    direction.y = 0.0f;
    direction.z = objectPosition.z - modelPosition.z;
    PSVECNormalize(&direction, &direction);
    i = 0;
    while (i < 16) {
        if (lbl_1_bss_2E8.fire[i].active == 0) {
            lbl_1_bss_2E8.fire[i].velocityX =
                30.0f * direction.x;
            lbl_1_bss_2E8.fire[i].velocityY =
                0.0f;
            lbl_1_bss_2E8.fire[i].velocityZ =
                30.0f * direction.z;
            lbl_1_bss_2E8.fire[i].lifetime =
                (s16) (frandmod(10) + 30);
            lbl_1_bss_2E8.fire[i].active = 1;
            lbl_1_bss_2E8.fire[i].opacity =
                0.8f;
            fn_80026A44(
                lbl_1_bss_2E8.fire[i].modelA, 1);
            fn_80026A44(
                lbl_1_bss_2E8.fire[i].modelB, 1);
            return;
        }
        i++;
    }
}

typedef struct ModelHookNames {
    char *name[6]; /* Vehicle attachment names for the six effect models. */
} ModelHookNames;

void fn_1_6188(void);

/* Spawns a fire effect along a direction from the vehicle's fire attachment. */
void fn_1_5614(void) {
    HuVecF modelPosition;
    HuVecF objectPosition;
    HuVecF direction;
    M665PlayerWork *playerWork;
    s32 i;

    playerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    fn_800267CC(playerWork->vehicleModel, &modelPosition);
    modelPosition.x += 240.0f;
    modelPosition.z += 200.0f;
    Hu3DModelObjPosGet(playerWork->vehicleModel, "itemhook_fire1", &objectPosition);
    direction.x = objectPosition.x - modelPosition.x;
    direction.y = 0.0f;
    direction.z = objectPosition.z - modelPosition.z;
    PSVECNormalize(&direction, &direction);
    i = 0;
    while (i < 16) {
        if (lbl_1_bss_2E8.fire[i].active == 0) {
            lbl_1_bss_2E8.fire[i].velocityX =
                30.0f * direction.x;
            lbl_1_bss_2E8.fire[i].velocityY =
                0.0f;
            lbl_1_bss_2E8.fire[i].velocityZ =
                30.0f * direction.z;
            lbl_1_bss_2E8.fire[i].lifetime =
                (s16) (frandmod(10) + 30);
            lbl_1_bss_2E8.fire[i].active = 1;
            lbl_1_bss_2E8.fire[i].opacity =
                0.8f;
            fn_80026A44(
                lbl_1_bss_2E8.fire[i].modelA, 1);
            fn_80026A44(
                lbl_1_bss_2E8.fire[i].modelB, 1);
            return;
        }
        i++;
    }
}

/* Moves and fades fire effects from the object updates in fn_1_41C8 and fn_1_4768. */
void fn_1_5820(void) {
    HuVecF fireOnePosition;
    HuVecF fireTwoPosition;
    HuVecF playerPosition;
    HuVecF firstCollisionPosition;
    HuVecF secondCollisionPosition;
    M665PlayerWork *mainPlayerWork;
    M665FireEffect *effect;
    HSF_DATA *modelData;
    s32 effectIndex;
    M665PlayerWork *playerWork;
    s32 playerIndex;

    mainPlayerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    switch (lbl_1_bss_2E8.fireEnabled) {
    case 1:
        if ((lbl_1_bss_2E8.fireFrames)++ >= 2) {
            m665FireSpawn();
            lbl_1_bss_2E8.fireFrames = 0;
        }
        break;
    }
    Hu3DModelObjPosGet(mainPlayerWork->vehicleModel, "itemhook_fire1", &fireOnePosition);
    Hu3DModelObjPosGet(mainPlayerWork->vehicleModel, "itemhook_fire2", &fireTwoPosition);
    lbl_1_bss_2E8.firePosition[0][0] = fireOnePosition.x;
    lbl_1_bss_2E8.firePosition[0][1] = fireOnePosition.y;
    lbl_1_bss_2E8.firePosition[0][2] = fireOnePosition.z;
    lbl_1_bss_2E8.firePosition[1][0] = fireTwoPosition.x;
    lbl_1_bss_2E8.firePosition[1][1] = fireTwoPosition.y;
    lbl_1_bss_2E8.firePosition[1][2] = fireTwoPosition.z;
    for (effectIndex = 0; effectIndex < 16; effectIndex++) {
        if (lbl_1_bss_2E8.fire[effectIndex].active == 0) {
            continue;
        }
        effect = &lbl_1_bss_2E8.fire[effectIndex];
        effect->offsetX += (effect->velocityX - 6.0f) + (f32) frandmod(12);
        effect->offsetZ += effect->velocityZ;
        fn_80026760(effect->modelA, fireOnePosition.x + effect->offsetX,
                        fireOnePosition.y, fireOnePosition.z + effect->offsetZ);
        fn_80026760(effect->modelB, fireTwoPosition.x - effect->offsetX,
                        fireTwoPosition.y, fireTwoPosition.z + effect->offsetZ);
        fn_800268B0(effect->modelA, effect->scale, effect->scale,
                          effect->scale);
        fn_800268B0(effect->modelB, effect->scale, effect->scale,
                          effect->scale);
        if (effect->lifetime < 20) {
            effect->opacity -= 0.04f;
        }
        if (effect->opacity < 0.0f) {
            effect->opacity = 0.0f;
        }
        fn_80026CBC(effect->modelA, effect->opacity);
        fn_80026CBC(effect->modelB, effect->opacity);
        effect->scale += (f32) frandmod(5) / 15.0f;
        if (effect->lifetime-- <= 0) {
            effect->lifetime = 0;
            modelData = lbl_802C01C8[effect->modelA].hsf;
            memcpy(modelData->material, lbl_1_bss_2E0,
                   modelData->materialNum * 60);
            fn_800269E8(effect->modelA, (1 << 0));
            fn_800269E8(effect->modelB, (1 << 0));
            modelData = lbl_802C01C8[effect->modelB].hsf;
            memcpy(modelData->material, lbl_1_bss_2E0,
                   modelData->materialNum * 60);
            effect->active = 0;
            effect->scale = 2.0f;
            effect->opacity = 0.8f;
            effect->offsetX = effect->offsetY = effect->offsetZ = 0.0f;
        }
        /* Collision checks still run after expiry above, using the reset offsets. */
        for (playerIndex = 0; playerIndex < 3; playerIndex++) {
            playerWork = &lbl_1_bss_10[lbl_1_bss_250.otherPlayers[playerIndex]];
            if (playerWork->status.other.state == 1) {
                fn_800267CC(playerWork->vehicleModel, &playerPosition);
                firstCollisionPosition.x = fireOnePosition.x + effect->offsetX;
                firstCollisionPosition.y = fireOnePosition.y;
                firstCollisionPosition.z = fireOnePosition.z + effect->offsetZ;
                secondCollisionPosition.x = fireTwoPosition.x - effect->offsetX;
                secondCollisionPosition.y = fireTwoPosition.y;
                secondCollisionPosition.z = fireTwoPosition.z + effect->offsetZ;
                playerPosition.y = fireOnePosition.y;
                if (sqrtf(PSVECSquareDistance(
                        &playerPosition,
                        &firstCollisionPosition)) <
                    100.0f + 30.0f * effect->scale ||
                    sqrtf(PSVECSquareDistance(
                        &playerPosition,
                        &secondCollisionPosition)) <
                    100.0f + 30.0f * effect->scale) {
                    fn_1_11FD0(playerWork);
                    continue;
                }
            }
        }
    }
    if (mainPlayerWork->effect.main.effectState == 1 && mainPlayerWork->action == 0) {
        mainPlayerWork->effect.main.effectState = 0;
    }
}

void fn_1_D25C(void);

/* Starts the selected main-player action when called from fn_1_4768's object update. */
void fn_1_60CC(u16 flags) {
    s16 playerIndex;
    M665PlayerWork *playerWork;

    playerIndex = lbl_1_bss_250.mainPlayer;
    playerWork = &lbl_1_bss_10[playerIndex];
    if ((playerWork->action == 0) && (((lbl_1_bss_0 | flags) & 0x100) != 0)) {
        s16 callPlayerIndex = lbl_1_bss_250.mainPlayer;
        fn_1_170C(callPlayerIndex, 2, 8.0f);
        playerWork->action = 1;
        playerWork->effect.main.effectState = 1;
        fn_1_D25C();
    }
}

/* Creates and attaches missile models during the sequence initialization hook. */
void fn_1_6188(void) {
    ModelHookNames hookNames = {{
        "itemhook_miss1", "itemhook_miss2", "itemhook_miss3",
        "itemhook_miss4", "itemhook_miss5", "itemhook_miss6",
    }};
    M665PlayerWork *playerWork;
    s16 baseModel;
    s16 modelID;
    s32 i;

    playerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    i = 0;
    while (i < 6) {
        if (i == 0) {
            baseModel = fn_80025A04(
                fn_80008DD0(DATANUM(0x7c0000, 25), M665_MEMORY_TAG, 2));
            modelID = baseModel;
        } else {
            modelID = fn_80025D4C(baseModel);
        }
        fn_800269E8(modelID, ((1 << 30) | (1 << 0)));
        fn_8002741C(playerWork->vehicleModel, hookNames.name[i], modelID);
        fn_80026E44(modelID);
        fn_8002D86C(modelID, 0.0f);
        lbl_1_bss_2E8.missiles[i].modelID = modelID;
        i++;
    }
    i = 0;
    while (i < 6) {
        if (i == 0) {
            baseModel = fn_80025A04(
                fn_80008DD0(DATANUM(0x7c0000, 32), M665_MEMORY_TAG, 2));
            modelID = baseModel;
        } else {
            modelID = fn_80025D4C(baseModel);
        }
        fn_800269E8(modelID, (1 << 0));
        fn_8002D86C(modelID, 0.0f);
        lbl_1_bss_2E8.missiles[i].collisionModelID = modelID;
        i++;
    }
}
/* Positions and starts one missile effect when called with its missile index. */
void fn_1_633C(s16 index) {
    HuVecF position;
    M665MissileWork *work;

    work = &lbl_1_bss_2E8.missiles[index];
    fn_800267CC(work->modelID, &position);
    position.x = (f32) ((f64) position.x +
                        (50.0 *
                         sin((3.141592653589793 *
                              (f64) work->angle) /
                             180.0)));
    position.z = (f32) ((f64) position.z +
                        (50.0 *
                         cos((3.141592653589793 *
                              (f64) work->angle) /
                             180.0)));
    fn_80026790(work->collisionModelID, &position);
    fn_8002D86C(work->collisionModelID, 1.0f);
    fn_8002D414(work->collisionModelID, 0.0f);
    fn_80026A44(work->collisionModelID, (1 << 0));
    work->state = 3;
    fn_800268B0(work->modelID, 1.2f, 1.2f,
                      0.8f);
}

/* Starts a missile's impact animation after a collision in fn_1_648C. */
static inline void fn_1_633C_for_648C(s16 index) {
    HuVecF position;
    M665MissileWork *work;

    work = &lbl_1_bss_2E8.missiles[index];
    fn_800267CC(work->modelID, &position);
    position.x = (f32) ((f64) position.x +
                        (50.0 *
                         sin((3.141592653589793 *
                              (f64) work->angle) /
                             180.0)));
    position.z = (f32) ((f64) position.z +
                        (50.0 *
                         cos((3.141592653589793 *
                              (f64) work->angle) /
                             180.0)));
    fn_80026790(work->collisionModelID, &position);
    fn_8002D86C(work->collisionModelID, 1.0f);
    fn_8002D414(work->collisionModelID, 0.0f);
    fn_80026A44(work->collisionModelID, (1 << 0));
    work->state = 3;
    fn_800268B0(work->modelID, 1.2f, 1.2f,
                      0.8f);
}

static inline void DebugValue(s32 value) { }

/* Updates missile flight, collision, and impact effects during its object-manager callback. */
void fn_1_648C(void) {

    s16 *mainPlayerWork;
    M665MissileWork *missileWork;
    M665PlayerWork *playerWork;
    HuVecF position;
    HuVecF motion;
    HuVecF playerPosition;
    HuVecF collisionPosition;
    s32 effectIndex;
    s32 playerIndex;
    f32 distanceToPlayer;
    f32 collisionDistanceSquared;
    f32 motionEndFrame;
    M665PlayerWork *mainPlayerWork2 = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    char *hookNames[6] = {"itemhook_miss1", "itemhook_miss2", "itemhook_miss3",
                          "itemhook_miss4", "itemhook_miss5", "itemhook_miss6"};
    f32 initialAngles[6] = { 22.0f, 5.0f, 45.0f, - 45.0f, - 5.0f, - 22.0f };
    for (effectIndex = 0; 6 > effectIndex; effectIndex++)
    {
        missileWork = &lbl_1_bss_2E8.missiles[effectIndex];
        switch (missileWork->state)
        {
        case 1 :
            Hu3DModelObjPosGet(mainPlayerWork2->vehicleModel, hookNames[effectIndex], &position);
            fn_800275B4(mainPlayerWork2->vehicleModel, hookNames[effectIndex]);
            fn_80026790(missileWork->modelID, &position);
            fn_80026808(missileWork->modelID, 0.0f, initialAngles[effectIndex], 0.0f);
            fn_8002D86C(missileWork->modelID, 1.0f);
            missileWork->state = 2;
            missileWork->angle = initialAngles[effectIndex];
            missileWork->speed = 5.0f;
            missileWork->frames = 0;
            break;
        case 2 :
            missileWork->speed += 0.5f;
            if (missileWork->speed > 30.0f)
            {
                missileWork->speed = 30.0f;
            }
            motion.x = missileWork->speed *
                       sin(3.141592653589793 * missileWork->angle / 180.0);
            motion.y = 0.0f;
            motion.z = missileWork->speed *
                       cos(3.141592653589793 * missileWork->angle / 180.0);
            fn_800267CC(missileWork->modelID, &position);
            position.x += motion.x;
            position.z += motion.z;
            if (++missileWork->frames < 18)
            {
                if ((u8) ((((u32) mainPlayerWork2->status.main.flags.byte >> 6) & 1)) != 0U)
                {
                    position.x += mainPlayerWork2->speed;
                }
                else
                {
                    position.x -= mainPlayerWork2->speed;
                }
            }
            else
            {
                DebugValue(missileWork->frames == 15); /* The frame check has no visible effect. */
            }
            fn_80026790(missileWork->modelID, &position);
            if (position.z >= 1000.0f)
            {
                missileWork->state = 0;
                fn_800269E8(missileWork->modelID, (1 << 0));
                fn_8002D86C(missileWork->modelID, 0.0f);
                fn_8002D414(missileWork->modelID, 0.0f);
            }
            if (position.x <=
                    (-1000.0) - 100.0 * sin(3.141592653589793 *
                                             missileWork->angle / 180.0) ||
                position.x >= 1000.0 - 100.0 * sin(3.141592653589793 *
                                                   missileWork->angle / 180.0)) {
                position.x +=
                    100.0 * sin(3.141592653589793 * missileWork->angle / 180.0);
                position.z +=
                    100.0 * cos(3.141592653589793 * missileWork->angle / 180.0);
                fn_80026790(missileWork->collisionModelID, &position);
                fn_8002D86C(missileWork->collisionModelID, 1.0f);
                fn_8002D414(missileWork->collisionModelID, 0.0f);
                fn_80026A44(missileWork->collisionModelID, (1 << 0));
                missileWork->state = 3;
                fn_800268B0(missileWork->modelID, 1.2f, 1.2f, 0.8f);
            }
                /* State changes above and earlier hits do not stop this scan; one missile can hit
                 * several opponents. */
                for (playerIndex = 0; playerIndex < 3; playerIndex++)
                {
                    playerWork = &lbl_1_bss_10[lbl_1_bss_250.otherPlayers[playerIndex]];
                    if (playerWork->status.other.state == 1)
                    {
                        fn_800267CC(playerWork->vehicleModel, & playerPosition);
                        playerPosition.y = position.y;
                        collisionDistanceSquared =
                        PSVECSquareDistance(&playerPosition, &position);
                        distanceToPlayer = sqrtf(collisionDistanceSquared);
                        if (distanceToPlayer < 145.0f)
                        {
                            fn_1_633C_for_648C((s16)effectIndex);
                            fn_1_11FD0(playerWork);
                        }
                    }
                }
            break;
        case 3 :
            motionEndFrame = fn_8002D530(missileWork->collisionModelID);
            if (fn_8002D4E0(missileWork->collisionModelID) >= motionEndFrame)
            {
                fn_8002D414(missileWork->modelID, 0.0f);
                fn_8002D86C(missileWork->modelID, 0.0f);
                fn_800269E8(missileWork->collisionModelID, (1 << 0));
                fn_8002D86C(missileWork->collisionModelID, 0.0f);
                fn_8002D414(missileWork->collisionModelID, 0.0f);
                missileWork->state = 0;
                fn_800268B0(missileWork->modelID, 1.0f, 1.0f, 1.0f);
            }
            else
            {
                if (fn_8002D4E0(missileWork->collisionModelID) == 5.0f)
                {
                    fn_800269E8(missileWork->modelID, (1 << 0));
                }
            }
            break;
        }
        if (fn_8002D4E0(missileWork->modelID) >= 55.0f)
        {
            fn_8002D730(missileWork->modelID, 55.0f, 107.0f);
        }
    }
    if (mainPlayerWork2->effect.main.effectState == 2 &&
        mainPlayerWork2->action == 0)
    {
        for (effectIndex = 0; 6 > effectIndex; effectIndex++)
        {
            if (lbl_1_bss_2E8.missiles[effectIndex].state != 0)
            {
                break;
            }
        }
        if (effectIndex == 6)
        {
            mainPlayerWork2->effect.main.effectState = 0;
        }
    }
}

/* Starts the main player's selected effect from fn_1_4768 when its flag is set. */
void fn_1_6DE0(u16 flags) {
    s16 playerIndex;
    M665PlayerWork *playerWork;

    playerIndex = lbl_1_bss_250.mainPlayer;
    playerWork = &lbl_1_bss_10[playerIndex];
    if ((playerWork->action == 0) && ((flags & 0x200) != 0)) {
        s16 callPlayerIndex = lbl_1_bss_250.mainPlayer;
        fn_1_170C(callPlayerIndex, 2, 8.0f);
        playerWork->action = 13;
        playerWork->effect.main.effectState = 2;
        fn_1_D25C();
    }
}

/* Creates the minigame's stage models during the sequence initialization hook. */
void fn_1_6E8C(void) {
    s32 killerIndex;
    s32 modelIndex;
    s16 modelID;

    for (killerIndex = 0; killerIndex < 3; killerIndex++) {
        modelID = fn_80025A04(
            fn_80008DD0(DATANUM(0x7c0000, 0), M665_MEMORY_TAG, 2));
        fn_800269E8(modelID, (1 << 0));
        lbl_1_bss_2E8.killers[killerIndex].state = 0;
        lbl_1_bss_2E8.killers[killerIndex].modelID = modelID;
        fn_80026E44(modelID);
        modelID = fn_80025A04(
            fn_80008DD0(DATANUM(0x7c0000, 32), M665_MEMORY_TAG, 2));
        fn_800269E8(modelID, (1 << 0));
        fn_8002D86C(modelID, 0.0f);
        lbl_1_bss_2E8.killers[killerIndex].collisionModelID = modelID;
        lbl_1_bss_2E8.killers[killerIndex].soundHandle = -1;
        for (modelIndex = 0; modelIndex < 32; modelIndex++) {
            modelID = fn_80025A04(
                fn_80008DD0(DATANUM(0x7c0000, 35), M665_MEMORY_TAG, 2));
            fn_800269E8(modelID, (1 << 0));
            lbl_1_bss_2E8.killers[killerIndex].trail[modelIndex].modelID = modelID;
            lbl_1_bss_2E8.killers[killerIndex].trail[modelIndex].active = 0;
        }
    }
}

/* Selects the nearest active opponent within 10000 units; the -1 lookup otherwise reads
 * mainPlayer. */
s16 fn_1_700C(s16 killerIndex) {
    HuVecF playerPosition;
    HuVecF modelPosition;
    M665KillerWork *work;
    M665PlayerWork *playerWork;
    s16 nearestPlayer;
    s32 playerIndex;
    f32 candidateDistance;
    f32 closestDistance;

    work = &lbl_1_bss_2E8.killers[killerIndex];
    nearestPlayer = -1;
    closestDistance = 10000.0f;
    fn_800267CC(work->modelID, &modelPosition);
    playerIndex = 0;
    while (playerIndex < 3) {
        playerWork = &lbl_1_bss_10[lbl_1_bss_250.otherPlayers[playerIndex]];
        if (playerWork->status.other.state == 1) {
            fn_800267CC(playerWork->vehicleModel, &playerPosition);
            candidateDistance = PSVECSquareDistance(
                &modelPosition, &playerPosition);
            candidateDistance = sqrtf(candidateDistance);
            if (candidateDistance < closestDistance) {
                closestDistance = candidateDistance;
                nearestPlayer = (s16) playerIndex;
            }
        }
        playerIndex++;
    }
    if (nearestPlayer != -1) {
        work->targetFrames = 20;
    }
    return (&lbl_1_bss_250.otherPlayers[0])[nearestPlayer];
}

/* Turns and moves one killer toward its target during fn_1_8378's object update. */
void fn_1_7214(s16 killerIndex) {
    HuVecF targetOffset;
    HuVecF currentPosition;
    HuVecF targetPosition;
    HuVecF playerPosition;
    M665KillerWork *killerWork;
    M665PlayerWork *targetWork;
    f32 targetAngle;
    f32 angleDifference;

    killerWork = &lbl_1_bss_2E8.killers[killerIndex];
    if (killerWork->targetPlayer == -1 ||
        (killerWork->targetFrames)-- <= 0) {
        killerWork->targetPlayer = fn_1_700C(killerIndex);
    }
    if (killerWork->targetPlayer == -1) {
        fn_800267CC(killerWork->modelID, &currentPosition);
        currentPosition.x += 8.333333015441895 *
            sin(3.141592653589793 * killerWork->angle /
                180.0);
        currentPosition.z += 8.333333015441895 *
            cos(3.141592653589793 * killerWork->angle /
                180.0);
        if (currentPosition.z > 200.0f) {
            currentPosition.z -= 3.0f;
        }
        fn_80026790(killerWork->modelID, &currentPosition);
        fn_80026808(killerWork->modelID, 0.0f,
                        killerWork->angle, 0.0f);
        return;
    }
    targetWork = &lbl_1_bss_10[killerWork->targetPlayer];
    fn_800267CC(targetWork->vehicleModel, &targetOffset);
    targetOffset.y += 100.0f;
    fn_800267CC(killerWork->modelID, &currentPosition);
    targetPosition.x = targetOffset.x - currentPosition.x;
    targetPosition.y = 0.0f;
    targetPosition.z = targetOffset.z - currentPosition.z;
    targetAngle = (f32) (180.0 *
        (atan2((f64) targetPosition.x, (f64) targetPosition.z) /
         3.141592653589793));
    angleDifference = targetAngle - killerWork->angle;
    while (angleDifference < 0.0f) {
        angleDifference += 360.0f;
    }
    while (angleDifference >= 360.0f) {
        angleDifference -= 360.0f;
    }
    if (angleDifference < 1.0f || angleDifference > 359.0f) {
        killerWork->angle = targetAngle;
    } else if (angleDifference < 180.0f) {
        killerWork->angle += 1.0f;
    } else {
        killerWork->angle -= 1.0f;
    }
    currentPosition.x += 8.333333015441895 *
        sin(3.141592653589793 * killerWork->angle /
            180.0);
    currentPosition.z += 8.333333015441895 *
        cos(3.141592653589793 * killerWork->angle /
            180.0);
    currentPosition.y -= killerWork->verticalSpeed;
    if (currentPosition.y > 250.0f &&
        killerWork->verticalSpeed < 9.0f) {
        killerWork->verticalSpeed += 1.0f;
    }
    if (currentPosition.y < 250.0f &&
        killerWork->verticalSpeed > 1.0f) {
        killerWork->verticalSpeed -= 1.0f;
    }
    if (currentPosition.y < 200.0f) {
        currentPosition.y = 200.0f;
    }
    fn_80026790(killerWork->modelID, &currentPosition);
    fn_80026808(killerWork->modelID, 0.0f,
                    killerWork->angle, 0.0f);
    if (currentPosition.z > 100.0f) {
        killerWork->state = 5;
    }
}

/* Turns the killer one degree toward heading zero each update (snapping within one degree), moves
 * it forward, and hides it at Z 1000. */
static inline void updateKillerMotionInline(s16 killerIndex) {
    HuVecF position;
    M665KillerWork *killerWork;

    killerWork = &lbl_1_bss_2E8.killers[killerIndex];
    fn_800267CC(killerWork->modelID, &position);
    while (killerWork->angle < 0.0f) {
        killerWork->angle += 360.0f;
    }
    while (killerWork->angle >= 360.0f) {
        killerWork->angle -= 360.0f;
    }
    if ((killerWork->angle < 1.0f) ||
        (killerWork->angle > 359.0f)) {
        killerWork->angle = 0.0f;
    } else if (killerWork->angle < 180.0f) {
        killerWork->angle -= 1.0f;
    } else {
        killerWork->angle += 1.0f;
    }
    position.x = position.x +
                 8.333333015441895 *
                     sin(3.141592653589793 *
                          killerWork->angle /
                         180.0);
    position.z = position.z +
                 8.333333015441895 *
                     cos(3.141592653589793 *
                          killerWork->angle /
                         180.0);
    fn_80026790(killerWork->modelID, &position);
    fn_80026808(killerWork->modelID, 0.0f,
                    killerWork->angle, 0.0f);
    if (position.z >= 1000.0f) {
        killerWork->state = 0;
        fn_800269E8(killerWork->modelID, (1 << 0));
        HuAudFXStop(killerWork->soundHandle);
        killerWork->soundHandle = -1;
    }
}

void fn_1_78AC(s16 killerIndex) {
    updateKillerMotionInline(killerIndex);
}

/* Stops a killer's sound and starts its impact animation when fn_1_7BA0 detects a collision. */
static inline void resetPlayerEffect(s16 killerIndex) {
    M665KillerWork *effectWork =
        &lbl_1_bss_2E8.killers[killerIndex];
    HuVecF resetPosition;

    HuAudFXStop(effectWork->soundHandle);
    effectWork->soundHandle = -1;
    fn_800267CC(effectWork->modelID, &resetPosition);
    fn_80026790(effectWork->collisionModelID, &resetPosition);
    fn_80026A44(effectWork->collisionModelID, (1 << 0));
    fn_8002D86C(effectWork->collisionModelID, 1.0f);
    fn_8002D414(effectWork->collisionModelID, 0.0f);
    effectWork->targetFrames = 5;
    effectWork->state = 6;
}

/* Stops a killer's sound and resets its collision model before setting state 6. */
void fn_1_7AF4(s16 killerIndex) {
    M665KillerWork *killerWork = &lbl_1_bss_2E8.killers[killerIndex];
    Point3d modelPosition;
    HuAudFXStop(killerWork->soundHandle);
    killerWork->soundHandle = -1;
    fn_800267CC(killerWork->modelID, &modelPosition);
    fn_80026790(killerWork->collisionModelID, &modelPosition);
    fn_80026A44(killerWork->collisionModelID, (1 << 0));
    fn_8002D86C(killerWork->collisionModelID, 1.0f);
    fn_8002D414(killerWork->collisionModelID, 0.0f);
    killerWork->targetFrames = 5;
    killerWork->state = 6;
}
/* Checks a killer against players and starts the collision response during fn_1_8378. */
void fn_1_7BA0(s16 killerIndex) {
    HuVecF playerPosition;
    HuVecF otherPosition;
    HuVecF collisionRadius;
    M665KillerWork *killerWork;
    M665PlayerWork *otherPlayerWork;
    M665PlayerWork *mainPlayerWork;
    s32 playerIndex;

    killerWork = &lbl_1_bss_2E8.killers[killerIndex];
    fn_800267CC(killerWork->modelID, &playerPosition);
    if (killerWork->soundHandle != -1) {
        HuAudFXPanning(killerWork->soundHandle,
                       (s16) ((s32) (playerPosition.x / 40.0f) + 64));
    }
    collisionRadius.x = fabs(25.0 *
                             sin(3.141592653589793 *
                                 killerWork->angle /
                                 180.0));
    collisionRadius.y = 0.0f;
    collisionRadius.z = fabs(25.0 *
                             cos(3.141592653589793 *
                                 killerWork->angle /
                                 180.0));
    if ((playerPosition.x - collisionRadius.x <= (-1000.0f)) ||
        (playerPosition.x + collisionRadius.x >= 1000.0f)) {
        resetPlayerEffect(killerIndex);
        return;
    }
    mainPlayerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    fn_800267CC(mainPlayerWork->vehicleModel, &otherPosition);
    if ((playerPosition.x + collisionRadius.x > otherPosition.x - 350.0f) &&
        (playerPosition.x - collisionRadius.x < 350.0f + otherPosition.x) &&
        (playerPosition.z + collisionRadius.z > otherPosition.z - 600.0f) &&
        (playerPosition.z - collisionRadius.z < 180.0f + otherPosition.z) &&
        (playerPosition.y < 300.0f)) {
        if (fn_80071A8C() == 5) {
            mainPlayerWork->models.main.damage += 10;
        }
        resetPlayerEffect(killerIndex);
        return;
    }
    /* A hit sets the killer to state 6 but does not stop tests against the remaining opponents. */
    for (playerIndex = 0; playerIndex < 3; playerIndex++) {
        otherPlayerWork = &lbl_1_bss_10[lbl_1_bss_250.otherPlayers[playerIndex]];
        if (otherPlayerWork->status.other.state == 1) {
            fn_800267CC(otherPlayerWork->vehicleModel, &otherPosition);
            if ((playerPosition.x + collisionRadius.x > otherPosition.x - 90.0f) &&
                (playerPosition.x - collisionRadius.x < 90.0f + otherPosition.x) &&
                (playerPosition.z + collisionRadius.z > otherPosition.z - 120.0f) &&
                (playerPosition.z - collisionRadius.z < 120.0f + otherPosition.z)) {
                fn_1_11FD0(otherPlayerWork);
                resetPlayerEffect(killerIndex);
            }
        }
    }
}

/* Spawns one trail model when its delay expires; fn_1_8378 advances active trails separately. */
static inline void updateKillerEffectsInline(s16 killerIndex) {
    HuVecF position;
    M665KillerWork *killerWork;
    s32 effectIndex;

    killerWork = &lbl_1_bss_2E8.killers[killerIndex];
    if (killerWork->trailFrames > 0) {
        killerWork->trailFrames -= 1;
        return;
    }
    fn_800267CC(killerWork->modelID, &position);
    position.x -= 100.0 *
                  sin(3.141592653589793 *
                      killerWork->angle / 180.0);
    position.z -= 100.0 *
                  cos(3.141592653589793 *
                      killerWork->angle / 180.0);
    for (effectIndex = 0; effectIndex < 32; effectIndex++) {
        if (killerWork->trail[effectIndex].active == 0) {
            killerWork->trail[effectIndex].active = 1;
            killerWork->trailFrames = frandmod(5) + 5;
            fn_80026790(killerWork->trail[effectIndex].modelID, &position);
            killerWork->trail[effectIndex].scale = 0.4f +
                ((f32) frandmod(200) / 1000.0f);
            fn_800268B0(killerWork->trail[effectIndex].modelID,
                              killerWork->trail[effectIndex].scale,
                              killerWork->trail[effectIndex].scale,
                              killerWork->trail[effectIndex].scale);
            killerWork->trail[effectIndex].opacity = (f32) (
                0.9 +
                (f64) ((f32) frandmod(10) / 100.0f));
            fn_80026CBC(killerWork->trail[effectIndex].modelID,
                              killerWork->trail[effectIndex].opacity);
            fn_80026A44(killerWork->trail[effectIndex].modelID, (1 << 0));
            return;
        }
    }
}

void fn_1_80FC(s16 killerIndex) {
    updateKillerEffectsInline(killerIndex);
}

extern void fn_1_7214(s16 killerIndex);

/* Updates active killers and their trails from the object updates in fn_1_41C8 and fn_1_4768. */
void fn_1_8378(void) {
    HuVecF position;
    HuVecF playerPosition;
    HuVecF effectPosition;
    M665PlayerWork *mainPlayerWork;
    M665KillerWork *killerWork;
    s16 *effectWork;
    s32 killerIndex;
    s32 index;
    f32 motionValue;

    mainPlayerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    for (killerIndex = 0; killerIndex < 3; killerIndex++) {
        killerWork = &lbl_1_bss_2E8.killers[killerIndex];
        for (index = 0; index < 32; index++) {
            if (killerWork->trail[index].active != 0) {
                killerWork->trail[index].scale +=
                    0.01 + (f64) ((f32) frandmod(100) / 10000.0f);
                fn_800268B0(
                    killerWork->trail[index].modelID,
                    killerWork->trail[index].scale,
                    killerWork->trail[index].scale,
                    killerWork->trail[index].scale);
                killerWork->trail[index].opacity -= 0.005;
                if (killerWork->trail[index].opacity <= 0.0f) {
                    killerWork->trail[index].opacity = 0.0f;
                    killerWork->trail[index].active = 0;
                    fn_800269E8(
                        killerWork->trail[index].modelID, 1);
                }
                fn_80026CBC(
                    killerWork->trail[index].modelID,
                    killerWork->trail[index].opacity);
            }
        }

        switch (killerWork->state) {
        case 1:
            fn_80026A44(killerWork->modelID, (1 << 0));
            fn_8002741C(mainPlayerWork->vehicleModel, "itemhook_killer",
                             killerWork->modelID);
            fn_80026760(killerWork->modelID, 0.0f, 0.0f, 0.0f);
            fn_80026808(killerWork->modelID, 0.0f, 0.0f, 0.0f);
            killerWork->angle = 0.0f;
            killerWork->targetPlayer = -1;
            killerWork->elapsed = 0;
            break;
        case 2:
            fn_800275B4(mainPlayerWork->vehicleModel, "itemhook_killer");
            Hu3DModelObjPosGet(mainPlayerWork->vehicleModel, "itemhook_killer",
                               &position);
            fn_80026790(killerWork->modelID, &position);
            HuAudFXPlay(0x876);
            killerWork->soundHandle = HuAudFXPlay(0x87B);
            killerWork->state = 3;
            break;
        case 3:
            fn_800267CC(killerWork->modelID, &position);
            position.z += 8.333333f;
            if (position.z >= -1850.0f) {
                killerWork->state = 4;
                killerWork->elapsed = 0;
                killerWork->verticalSpeed = 1.0f;
            }
            if ((killerWork->elapsed)++ < 10) {
                position.x +=
                    -mainPlayerWork->speed +
                    (2.0f * (mainPlayerWork->speed *
                             (f32) (u8) ((mainPlayerWork->status.main.flags.byte >> 6) & 1)));
            /* The preceding post-increment means elapsed is already above 10 here. */
            } else if (killerWork->elapsed == 10) {
                killerWork->horizontalSpeed =
                    -mainPlayerWork->speed +
                    (2.0f * (mainPlayerWork->speed *
                             (f32) (u8) ((mainPlayerWork->status.main.flags.byte >> 6) & 1)));
            } else {
                position.x += killerWork->horizontalSpeed;
            }
            fn_80026790(killerWork->modelID, &position);
            break;
        case 4:
            fn_1_7214((s16) killerIndex);
            if ((killerWork->elapsed)++ >= 300) {
                killerWork->state = 5;
            }
            break;
        case 5:
            updateKillerMotionInline((s16) killerIndex);
            break;
        case 6:
            if ((killerWork->targetFrames)-- <= 0) {
                fn_800269E8(killerWork->modelID, (1 << 0));
            }
            fn_800267CC(killerWork->collisionModelID, &position);
            position.z += lbl_1_bss_250.scrollSpeed;
            fn_80026790(killerWork->collisionModelID, &position);
            motionValue = fn_8002D530(killerWork->collisionModelID);
            if (fn_8002D4E0(killerWork->collisionModelID) >= motionValue) {
                fn_800269E8(killerWork->collisionModelID, (1 << 0));
                killerWork->state = 0;
            }
            break;
        }

        if (killerWork->state == 3 ||
            killerWork->state == 4 ||
            killerWork->state == 5) {
            updateKillerEffectsInline((s16) killerIndex);
            fn_1_7BA0((s16) killerIndex);
        }
        if (mainPlayerWork->effect.main.effectState == 3 && mainPlayerWork->action == 0 &&
            killerWork->state == 0) {
            mainPlayerWork->effect.main.effectState = 0;
        }
    }
}

/* Finds a free killer slot when fn_1_2158 starts the launch action. */
s16 fn_1_8D54(void)
{
    s32 index;

    for (index = 0; index < 3; index++) {
        if (lbl_1_bss_2E8.killers[index].state == 0) {
            return index;
        }
    }
    return -1;
}
/* Starts the main player's selected action during fn_1_4768's object update. */
void fn_1_8DA4(u16 flags) {
    s16 currentPlayer;
    s16 foundKiller;
    s32 killerIndex;
    M665PlayerWork *playerWork;

    currentPlayer = lbl_1_bss_250.mainPlayer;
    playerWork = &lbl_1_bss_10[currentPlayer];
    if ((playerWork->action == 0) && ((flags & 1) != 0)) {
        killerIndex = 0;
        goto scanCheck;
    scanPlayers:
        if (lbl_1_bss_2E8.killers[killerIndex].state == 0) {
            foundKiller = killerIndex;
            goto scanDone;
        } else {
            killerIndex++;
            goto scanCheck;
        }
    scanCheck:
        if (killerIndex < 3) {
            goto scanPlayers;
        }
        foundKiller = -1;
    scanDone:
        if (foundKiller != -1) {
            s16 callPlayerIndex = lbl_1_bss_250.mainPlayer;
            fn_1_170C(callPlayerIndex, 2, 8.0f);
            playerWork->action = 23;
            playerWork->effect.main.effectState = 3;
            fn_1_D25C();
        }
    }
}

/* Creates ten rolling-effect models during the sequence initialization hook. */
void fn_1_8E90(void) {
    s16 baseModel;
    s16 model;
    s32 modelIndex;

    modelIndex = 0;
    goto checkModels;
createModel:
    if (modelIndex == 0) {
        model = fn_80025A04(
            fn_80008DD0(DATANUM(0x7c0000, 1), M665_MEMORY_TAG, 2));
        baseModel = model;
    } else {
        model = fn_80025D4C(baseModel);
    }
    fn_800269E8(model, (1 << 0));
    fn_800268B0(model, 1.2f, 1.2f, 1.2f);
    lbl_1_bss_2E8.rolling[modelIndex].modelID = model;
    lbl_1_bss_2E8.rolling[modelIndex].state = 0;
    fn_80026E44(model);
    fn_80026808(model, frandmod(360), 0.0f, 0.0f);
    modelIndex++;
checkModels:
    if (modelIndex < 10) {
        goto createModel;
    }
}

/* Advances rolling effects from the object updates in fn_1_41C8 and fn_1_4768. */
void fn_1_8FC8(void) {
    HuVecF position;
    HuVecF rotation;
    HuVecF playerPosition;
    M665PlayerWork *currentPlayerWork;
    M665RollingWork *effectWork;
    M665PlayerWork *otherPlayerWork;
    s32 effectIndex;
    s32 playerIndex;

    currentPlayerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    for (effectIndex = 0; effectIndex < 10; effectIndex++) {
        effectWork = &lbl_1_bss_2E8.rolling[effectIndex];
        switch (effectWork->state) {
        case 1:
            HuAudFXPlay(0x874);
            fn_80026A44(effectWork->modelID, (1 << 0));
            fn_800267CC(currentPlayerWork->vehicleModel, &position);
            position.z -= 350.0f;
            position.y += 400.0f;
            fn_80026790(effectWork->modelID, &position);
            effectWork->horizontalSpeed =
                (f32) ((10.0 *
                        sin((3.141592653589793 * (f64) (frandmod(90) - 45)) /
                            180.0)) -
                       (f64) (currentPlayerWork->speed *
                        (f32) (1 - (((currentPlayerWork->status.main.flags.byte >> 6) & 1) * 2))));
            effectWork->verticalSpeed = 25.0f;
            effectWork->depthSpeed = 15.0f;
            effectWork->state = 2;
            break;
        case 2:
            fn_800267CC(effectWork->modelID, &position);
            fn_80026874(effectWork->modelID, &rotation);
            position.x += effectWork->horizontalSpeed;
            position.y += effectWork->verticalSpeed;
            position.z += effectWork->depthSpeed;
            if (position.x < (-1000.0f)) {
                position.x = (-1000.0f);
            }
            if (position.x > 1000.0f) {
                position.x = 1000.0f;
            }
            rotation.x += 10.0f;
            if (position.y <= 50.0 -
                              50.0 *
                                  cos(3.141592653589793 * rotation.x /
                                      180.0)) {
                position.y = 50.0 -
                             50.0 *
                                 cos(3.141592653589793 * rotation.x /
                                     180.0);
                effectWork->state = 3;
                HuAudFXPlay(0x877);
            }
            fn_80026838(effectWork->modelID, &rotation);
            fn_80026790(effectWork->modelID, &position);
            effectWork->verticalSpeed -= 1.5f;
            break;
        case 3:
            fn_800267CC(effectWork->modelID, &position);
            fn_80026874(effectWork->modelID, &rotation);
            rotation.x += 10.0f;
            position.z += 8.333333f + lbl_1_bss_250.scrollSpeed;
            fn_80026838(effectWork->modelID, &rotation);
            fn_80026760(effectWork->modelID, position.x,
                (f32) (50.0 - 50.0 *
                    cos(3.141592653589793 * rotation.x / 180.0)),
                position.z);
            if (position.z >= 600.0f) {
                fn_800269E8(effectWork->modelID, (1 << 0));
                effectWork->state = 0;
            }
            for (playerIndex = 0; playerIndex < 3; playerIndex++) {
                otherPlayerWork = &lbl_1_bss_10[(&lbl_1_bss_250.otherPlayers[0])[playerIndex]];
                if (otherPlayerWork->status.other.state == 1) {
                    fn_800267CC(otherPlayerWork->vehicleModel, &playerPosition);
                    position.y = 0.0f;
                    /* This Z offset accumulates once per active opponent because position is reused
                     * in this loop. */
                    position.z += 50.0 *
                        sin(3.141592653589793 * rotation.x / 180.0);
                    if (sqrtf(PSVECSquareDistance(&position, &playerPosition)) <
                        120.0f) {
                        fn_1_11FD0(otherPlayerWork);
                    }
                }
            }
            break;
        case 4:
            fn_800267CC(effectWork->modelID, &position);
            fn_80026874(effectWork->modelID, &rotation);
            position.x += effectWork->horizontalSpeed;
            position.y += effectWork->verticalSpeed;
            position.z += effectWork->depthSpeed;
            effectWork->verticalSpeed -= 0.5;
            if (effectWork->verticalSpeed < 0.0f) {
                effectWork->verticalSpeed = 0.0f;
            }
            fn_80026790(effectWork->modelID, &position);
            fn_80026838(effectWork->modelID, &rotation);
            if (fabs(position.x) > 1300.0) {
                fn_800269E8(effectWork->modelID, (1 << 0));
                effectWork->state = 0;
            }
            break;
        }
    }
    if (currentPlayerWork->effect.main.effectState == 6 && currentPlayerWork->action == 0) {
        for (effectIndex = 0; effectIndex < 10; effectIndex++) {
            if (lbl_1_bss_2E8.rolling[effectIndex].state == 0) {
                continue;
            }
            break;
        }
        if (effectIndex == 10) {
            currentPlayerWork->effect.main.effectState = 0;
        }
    }
}

typedef struct S16Triple {
    s16 values[3]; /* Table entries are compared and stored as signed 16-bit values. */
} S16Triple;

const S16Triple lbl_1_rodata_3A0 = {{2, 5, 10}};

const S16Triple lbl_1_rodata_3A6 = {{10, 70, 100}};

/* Applies the selected main-player action from fn_1_4768's object update. */
void fn_1_9740(u16 flags) {
    s16 currentPlayer;
    s16 randomValue;
    s16 callPlayerIndex;
    M665PlayerWork *playerWork;
    S16Triple values;
    S16Triple thresholds;
    s16 selected;
    s32 thresholdIndex;

        currentPlayer = lbl_1_bss_250.mainPlayer;
    playerWork = &lbl_1_bss_10[currentPlayer];
    values = lbl_1_rodata_3A0;
    thresholds = lbl_1_rodata_3A6;
    if ((playerWork->action == 0) && ((flags & 0x4000) != 0)) {
        callPlayerIndex = lbl_1_bss_250.mainPlayer;
        fn_1_170C(callPlayerIndex, 2, 8.0f);
        playerWork->action = 28;
        playerWork->effect.main.effectState = 6;
        randomValue = frandmod(100);
        for (thresholdIndex = 0; thresholdIndex < 3; thresholdIndex++) {
            if (randomValue < thresholds.values[thresholdIndex]) {
                selected = thresholdIndex;
                break;
            }
        }
        if (thresholdIndex == 3) {
            selected = 0;
        }
        lbl_1_bss_2E8.rollingCount = values.values[selected];
        lbl_1_bss_2E8.rollingFrames =
            (s16) (lbl_1_bss_2E8.rollingCount * 10);
        fn_1_D25C();
    }
}

/* Creates seven hazards and their two effect models during sequence initialization. */
void fn_1_98AC(void) {
    s32 modelIndex;
    s16 modelID;
    s16 sharedModelID;

    lbl_1_bss_2E4 = frandmod(7);
    modelIndex = 0;
    while (modelIndex < 7) {
        if (modelIndex == 0) {
            modelID = fn_80025A04(
                fn_80008DD0(DATANUM(0x7c0000, 27), M665_MEMORY_TAG, 2));
            sharedModelID = modelID;
        } else {
            modelID = fn_80025D4C(sharedModelID);
        }
        fn_800269E8(modelID, (1 << 0));
        fn_80026808(modelID, 0.0f, (f32) frandmod(360),
                        0.0f);
        lbl_1_bss_2E8.hazards[modelIndex].model = modelID;
        lbl_1_bss_2E8.hazards[modelIndex].state = 0;
        modelIndex++;
    }
    modelIndex = 0;
    while (modelIndex < 7) {
        if (modelIndex == 0) {
            modelID = fn_80025A04(
                fn_80008DD0(DATANUM(0x7c0000, 28), M665_MEMORY_TAG, 2));
            sharedModelID = modelID;
        } else {
            modelID = fn_80025D4C(sharedModelID);
        }
        fn_800269E8(modelID, (1 << 0));
        fn_8002D86C(modelID, 0.0f);
        lbl_1_bss_2E8.hazards[modelIndex].firstAnimation = modelID;
        modelIndex++;
    }
    modelIndex = 0;
    while (modelIndex < 7) {
        if (modelIndex == 0) {
            modelID = fn_80025A04(
                fn_80008DD0(DATANUM(0x7c0000, 32), M665_MEMORY_TAG, 2));
            sharedModelID = modelID;
        } else {
            modelID = fn_80025D4C(sharedModelID);
        }
        fn_800269E8(modelID, (1 << 0));
        fn_8002D86C(modelID, 0.0f);
        lbl_1_bss_2E8.hazards[modelIndex].secondAnimation = modelID;
        fn_80026B8C(modelID, 1);
        lbl_1_bss_2E8.hazards[modelIndex].secondAnimation = modelID;
        modelIndex++;
    }
}

/* Activates the next available hazard when called by fn_1_2158. */
void fn_1_9AE8(void) {
    s32 attempt;
    M665Hazard *leaf;

    attempt = 0;
    while (attempt < 7) {
        leaf = &lbl_1_bss_2E8.hazards[lbl_1_bss_2E4];
        if (leaf->state == 0) {
            HuAudFXPlay(0x875);
            leaf->state = 1;
            return;
        }
        attempt++;
        lbl_1_bss_2E4 = (lbl_1_bss_2E4 + 2) % 7;
    }
}

/* Starts a hazard's second animation and sound when fn_1_9CF0 dispatches its reaction. */
void fn_1_9BAC(s16 hazardIndex) {
    Point3d hazardPosition;

    fn_800267CC(lbl_1_bss_2E8.hazards[hazardIndex].model, &hazardPosition);
    fn_80026790(lbl_1_bss_2E8.hazards[hazardIndex].secondAnimation, &hazardPosition);
    fn_80026A44(lbl_1_bss_2E8.hazards[hazardIndex].secondAnimation, 1U);
    fn_800269E8(lbl_1_bss_2E8.hazards[hazardIndex].model, 1U);
    fn_8002D86C(lbl_1_bss_2E8.hazards[hazardIndex].secondAnimation, 1.0f);
    fn_8002D414(lbl_1_bss_2E8.hazards[hazardIndex].secondAnimation, 0.0f);
    lbl_1_bss_2E8.hazards[hazardIndex].state = 5;
    HuAudFXPlayPan(0x878, (s16) ((s32) (hazardPosition.x / 40.0f) + 64));
}

extern void OSReport(const char *format, ...);

typedef struct ResourcePointers {
    char *first; /* First attachment name in the pair. */
    char *second; /* Second attachment name, read as part of the pointer pair. */
} ResourcePointers;

typedef Vec M665VecF;

extern s16 lbl_1_data_364[];

void fn_1_AD40(void);

typedef struct ResourceIdList {
    s32 first; /* First model resource ID in the three-entry selection table. */
    s32 second; /* Read through indexed access to the resource ID table. */
    s32 third; /* Read through indexed access to the resource ID table. */
} ResourceIdList;

void fn_1_B09C(void);

/* Advances hazards and their collisions from object updates in fn_1_41C8 and fn_1_4768. */
void fn_1_9CF0(void) {
    M665VecF position;
    M665VecF playerPosition;
    M665VecF previousPosition;
    M665VecF objectPosition;
    M665VecF impactPosition;
    M665Hazard *hazard;
    M665PlayerWork *mainPlayerWork;
    M665PlayerWork *otherPlayerWork;
    M665PlayerWork *playerWork;
    f32 progressDistance;
    s32 hazardIndex;
    s32 playerIndex;
    s16 forwardIndex;
    s16 backwardIndex;

    mainPlayerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    for (hazardIndex = 0; hazardIndex < 6; hazardIndex++) {
        lbl_1_bss_280[hazardIndex].gapDistance = -1.0f;
        lbl_1_bss_280[hazardIndex].horizontalDistance =
            lbl_1_bss_280[hazardIndex].reserved =
                lbl_1_bss_280[hazardIndex].depthDistance = 0.0f;
    }
    previousPosition.x = (-1000.0f);
    previousPosition.y = 0.0f;
    previousPosition.z = 0.0f;
    for (hazardIndex = 0; hazardIndex < 7; hazardIndex++) {
        hazard = &lbl_1_bss_2E8.hazards[hazardIndex];
        switch (hazard->state) {
        case 1:
            fn_80026A44(hazard->model, (1 << 0));
            fn_800267CC(mainPlayerWork->vehicleModel, &position);
            fn_800268B0(hazard->model, 0.5f, 0.5f, 0.5f);
            fn_800268B0(hazard->firstAnimation, 0.5f, 1.0f, 0.5f);
            position.z -= 350.0f;
            position.y += 300.0f;
            fn_80026790(hazard->model, &position);
            hazard->state = 2;
            hazard->elapsed = 0;
            break;
        case 2:
            fn_800267CC(hazard->model, &position);
            position.y += 40.0f;
            if (hazard->elapsed++ < 10) {
                position.x += -mainPlayerWork->speed +
                    (2.0f * (mainPlayerWork->speed *
                    (f32) (u8) ((mainPlayerWork->status.main.flags.byte >> 6) & 1)));
            } else {
                /* The preceding post-increment means elapsed is already above 10 here. */
                if (hazard->elapsed == 10) {
            hazard->horizontalSpeed =
                        -mainPlayerWork->speed +
                        (2.0f * (mainPlayerWork->speed *
                        (f32) (u8) ((mainPlayerWork->status.main.flags.byte >> 6) & 1)));
                }
            position.x += hazard->horizontalSpeed;
            }
            if (position.y >= 1500.0f) {
                fn_800268B0(hazard->model, 0.6f, 0.6f, 0.6f);
                hazard->state = 3;
                fn_80026E44(hazard->model);
                hazard->fallSpeed = 0.0f;
                position.x = -900.0f + (f32) (hazardIndex * 300) -
                             30.0f + (f32) frandmod(30);
                position.z = -1500.0f + (f32) frandmod(400);
            }
            fn_80026790(hazard->model, &position);
            break;
        case 3:
            fn_800267CC(hazard->model, &position);
            hazard->fallSpeed -= 2.0f;
            position.y += hazard->fallSpeed;
            if (position.y <= 64.0f) {
                OSReport("%d\n",
                         (s16) (position.x / 40.0f) + 64);
                HuAudFXPlayPan(0x878,
                    (s16) ((s32) (position.x / 40.0f) + 64));
                position.y = 64.0f;
                hazard->state = 4;
                fn_80026760(hazard->firstAnimation, position.x, 0.0f,
                                position.z);
                fn_80026760(hazard->secondAnimation, position.x, 30.0f,
                                position.z);
                fn_80026A44(hazard->firstAnimation, (1 << 0));
                fn_80026A44(hazard->secondAnimation, (1 << 0));
                fn_800269E8(hazard->model, (1 << 0));
                fn_800269E8(hazard->firstAnimation, ((1 << 30) | (1 << 0)));
                fn_8002D86C(hazard->firstAnimation, 1.0f);
                fn_8002D86C(hazard->secondAnimation, 1.0f);
                fn_8002D414(hazard->firstAnimation, 0.0f);
                fn_8002D414(hazard->secondAnimation, 0.0f);
                fn_800268B0(hazard->firstAnimation, 0.6f, 0.0f, 0.6f);
                fn_800268B0(hazard->secondAnimation, 1.0f, 1.0f, 1.0f);
            hazard->landingScaleY = 0.0f;
            }
            fn_80026790(hazard->model, &position);
            if (hazardIndex != 0) {
                lbl_1_bss_280[hazardIndex - 1].gapDistance =
                    sqrtf(PSVECSquareDistance(&previousPosition,
                                              &position));
                lbl_1_bss_280[hazardIndex - 1].horizontalDistance =
                    position.x - previousPosition.x;
                lbl_1_bss_280[hazardIndex - 1].reserved = 0.0f;
                lbl_1_bss_280[hazardIndex - 1].depthDistance =
                    position.z - previousPosition.z;
            }
            previousPosition.x = position.x;
            previousPosition.y = 0.0f;
            previousPosition.z = position.z;
            for (playerIndex = 0; playerIndex < 3; playerIndex++) {
                otherPlayerWork = &lbl_1_bss_10[lbl_1_bss_250.otherPlayers[playerIndex]];
                if (otherPlayerWork->status.other.state == 1) {
                    objectPosition.x = position.x;
                    objectPosition.y = 0.0f;
                    objectPosition.z = position.z;
                    fn_800267CC(otherPlayerWork->vehicleModel, &playerPosition);
                    if (sqrtf(PSVECSquareDistance(&objectPosition,
                                                 &playerPosition)) <
                            185.0f && position.y <= 300.0f) {
                        fn_1_11FD0(otherPlayerWork);
                        fn_1_9BAC((s16) hazardIndex);
                    }
                }
            }
            break;
        case 4:
            if (fn_8002D4E0(hazard->firstAnimation) >= 60.0f) {
                fn_8002D730(hazard->firstAnimation, 61.0f, 120.0f);
            }
            hazard->landingScaleY += 0.02;
            if (hazard->landingScaleY > 1.0f) {
                hazard->landingScaleY = 1.0f;
            }
            fn_800268B0(hazard->firstAnimation, 0.6f,
                             hazard->landingScaleY, 0.6f);
            fn_800267CC(hazard->firstAnimation, &position);
            position.z += lbl_1_bss_250.scrollSpeed;
            if (position.z >= 1000.0f) {
                hazard->state = 0;
                fn_8002D86C(hazard->firstAnimation, 0.0f);
                fn_8002D414(hazard->firstAnimation, 0.0f);
                fn_800269E8(hazard->firstAnimation, (1 << 0));
                fn_800269E8(hazard->secondAnimation, (1 << 0));
            }
            fn_80026790(hazard->firstAnimation, &position);
            fn_80026760(hazard->secondAnimation, position.x, 30.0f,
                            position.z);
            if (hazardIndex != 0) {
                lbl_1_bss_280[hazardIndex - 1].gapDistance =
                    sqrtf(PSVECSquareDistance(&previousPosition,
                                              &position));
                lbl_1_bss_280[hazardIndex - 1].horizontalDistance =
                    position.x - previousPosition.x;
                lbl_1_bss_280[hazardIndex - 1].reserved = 0.0f;
                lbl_1_bss_280[hazardIndex - 1].depthDistance =
                    position.z - previousPosition.z;
            }
            previousPosition.x = position.x;
            previousPosition.y = 0.0f;
            previousPosition.z = position.z;
            for (playerIndex = 0; playerIndex < 3; playerIndex++) {
                playerWork = &lbl_1_bss_10[lbl_1_bss_250.otherPlayers[playerIndex]];
                if (playerWork->status.other.state == 1) {
                    fn_800267CC(playerWork->vehicleModel, &playerPosition);
                    if (sqrtf(PSVECSquareDistance(&position,
                                                 &playerPosition)) <
                        185.0f) {
                        fn_1_11FD0(playerWork);
                    }
                }
            }
            break;
        case 5:
            progressDistance = fn_8002D530(hazard->secondAnimation);
            if (fn_8002D4E0(hazard->secondAnimation) >= progressDistance) {
                hazard->state = 0;
                fn_800269E8(hazard->secondAnimation, (1 << 0));
            }
            break;
        }
    }
    if (mainPlayerWork->effect.main.effectState == 5 && mainPlayerWork->action == 0) {
        for (hazardIndex = 0; hazardIndex < 7; hazardIndex++) {
            if (lbl_1_bss_2E8.hazards[hazardIndex].state ==
                0) {
                continue;
            }
            break;
        }
        if (hazardIndex == 7) {
            mainPlayerWork->effect.main.effectState = 0;
        }
    }
}

/* Starts the main player's action 17 on the 2048 input in fn_1_4768. */
void fn_1_AC94(u16 flags) {
    s16 playerIndex;
    M665PlayerWork *playerWork;

    playerIndex = lbl_1_bss_250.mainPlayer;
    playerWork = &lbl_1_bss_10[playerIndex];
    if ((playerWork->action == 0) && ((flags & 0x800) != 0)) {
        s16 callPlayerIndex = lbl_1_bss_250.mainPlayer;
        fn_1_170C(callPlayerIndex, 2, 8.0f);
        playerWork->action = 17;
        playerWork->effect.main.effectState = 5;
        fn_1_D25C();
    }
}

/* Creates the laser models during the sequence initialization hook. */
void fn_1_AD40(void) {
    static const ResourcePointers lbl_1_rodata_3D0 = {"itemhook_laser1", "itemhook_laser2"};
    ResourcePointers resourcePointers;
    M665PlayerWork *currentPlayerWork;
    s32 modelIndex;
    s16 model;

    currentPlayerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    resourcePointers = lbl_1_rodata_3D0;
    for (modelIndex = 0; modelIndex < 2; modelIndex++) {
        model = fn_80025A04(
            fn_80008DD0(DATANUM(0x7c0000, 26), M665_MEMORY_TAG, 2));
        fn_800268B0(model, 0.0f, 0.0f, 1.0f);
        fn_800269E8(model, ((1 << 30) | (1 << 0)));
        lbl_1_bss_2E8.laser.modelIDs[modelIndex] = model;
    }
}

typedef struct ModelNames {
    char *name[6];
} ModelNames;

/* Advances the laser collision effects from the object updates in fn_1_41C8 and fn_1_4768. */
void fn_1_AE24(void) {
    M665PlayerWork *currentWork;
    M665PlayerWork *playerWork;
    HuVecF targetPosition;
    HuVecF playerPosition;
    s32 modelIndex;
    s32 playerIndex;
    f32 distanceX;
    f32 distance;
    f32 absoluteDistance;

    currentWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    if (currentWork->action < 9 || currentWork->action > 11) {
        return;
    }
    if (lbl_1_bss_2E8.laser.scale <= 0.1f) {
        return;
    }
    modelIndex = 0;
    while (modelIndex < 2) {
        fn_800267CC(lbl_1_bss_2E8.laser.modelIDs[modelIndex],
                        &targetPosition);
        playerIndex = 0;
        while (playerIndex < 3) {
            playerWork = &lbl_1_bss_10[lbl_1_bss_250.otherPlayers[playerIndex]];
            if (playerWork->status.other.state == 1) {
                fn_800267CC(playerWork->vehicleModel, &playerPosition);
                distanceX = targetPosition.x - playerPosition.x;
                distance = __fabsf(distanceX);
                absoluteDistance = distance;
                if (absoluteDistance <
                        (120.0f +
                         100.0f *
                             lbl_1_bss_2E8.laser.scale) &&
                    playerPosition.z <=
                        (targetPosition.z +
                         2500.0f *
                             lbl_1_bss_2E8.laser.scale)) {
                    fn_1_11FD0(playerWork);
                }
            }
            playerIndex++;
        }
        modelIndex++;
    }
}

/* Applies a selected action to the main player when called by fn_1_4768. */
void fn_1_AFF0(u16 flags) {
    s16 playerIndex;
    M665PlayerWork *playerWork;

    playerIndex = lbl_1_bss_250.mainPlayer;
    playerWork = &lbl_1_bss_10[playerIndex];
    if ((playerWork->action == 0) && ((flags & 0x400) != 0)) {
        playerWork->action = 8;
        {
            s16 callPlayerIndex = lbl_1_bss_250.mainPlayer;
            fn_1_170C(callPlayerIndex, 2, 8.0f);
        }
        playerWork->effect.main.effectState = 4;
        fn_1_D25C();
    }
}

/* Creates the bouncing-effect models during the sequence initialization hook. */
void fn_1_B09C(void) {
    static const ResourceIdList lbl_1_rodata_3DC = {
        DATANUM(0x7c0000, 29), DATANUM(0x7c0000, 30), DATANUM(0x7c0000, 31)
    };
    M665PlayerWork *currentPlayerWork;
    ResourceIdList resourceIds;
    s32 modelIndex;
    s16 modelID;
    s16 resourceIndex;

    currentPlayerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    resourceIds = lbl_1_rodata_3DC;
    for (modelIndex = 0; modelIndex < 6; modelIndex++) {
        resourceIndex = (modelIndex + frandmod(3)) % 3;
        modelID = fn_80025A04(fn_80008DD0((&resourceIds.first)[resourceIndex],
                                                      M665_MEMORY_TAG, 2));
        lbl_1_bss_2E8.bouncing[modelIndex].modelID = modelID;
        fn_800269E8(modelID, (1 << 0));
        fn_80026E44(modelID);
        lbl_1_bss_2E8.bouncing[modelIndex].state = 0;
        if (resourceIndex == 0) {
            fn_80026808(modelID, 0.0f, 0.0f,
                            90.0f);
        }
    }
}

/* Advances bouncing effects from the object updates in fn_1_41C8 and fn_1_4768. */
void fn_1_B1E8(void) {
    static const ModelNames lbl_1_rodata_3E8 = {{
    "itemhook_bane1", "itemhook_bane2", "itemhook_bane3",
    "itemhook_bane4", "itemhook_bane5", "itemhook_bane6",
}};
    ModelNames models;
    s16 mainPlayerIndex;
    M665PlayerWork *playerWork;
    M665BouncingWork *objectWork;
    HuVecF position;
    s32 objectIndex;
    s16 angle;

    mainPlayerIndex = lbl_1_bss_250.mainPlayer;
    playerWork = &lbl_1_bss_10[mainPlayerIndex];
    models = lbl_1_rodata_3E8;
    objectIndex = 0;
    while (objectIndex < 6) {
        objectWork = &lbl_1_bss_2E8.bouncing[objectIndex];
        switch (objectWork->state) {
        case 1:
            angle = (s16) (frandmod(90) - 45);
            Hu3DModelObjPosGet(playerWork->vehicleModel,
                               models.name[objectIndex],
                               &position);
            fn_80026A44(objectWork->modelID, (1 << 0));
            fn_80026790(objectWork->modelID, &position);
            objectWork->state = 2;
            objectWork->horizontalSpeed =
                (f32) (15.0 * sin(
                    3.141592653589793 * angle / 180.0));
            objectWork->verticalSpeed = 15.0f;
            objectWork->depthSpeed =
                (f32) (15.0 * cos(
                    3.141592653589793 * angle / 180.0));
            break;
        case 2:
            fn_800267CC(objectWork->modelID, &position);
            position.x += objectWork->horizontalSpeed;
            position.y += objectWork->verticalSpeed;
            position.z += objectWork->depthSpeed;
            if (position.y <= 0.0f) {
                position.y = 0.0f;
                objectWork->horizontalSpeed *= 0.5f;
                objectWork->verticalSpeed *= (-0.5f);
                objectWork->depthSpeed *= 0.5f;
                if (objectWork->verticalSpeed <= 1.0f) {
                    objectWork->state = 3;
                    objectWork->frames = 30;
                }
            }
            fn_80026790(objectWork->modelID, &position);
            objectWork->verticalSpeed -= 2.0f;
            break;
        case 3:
            fn_800267CC(objectWork->modelID, &position);
            position.z += lbl_1_bss_250.scrollSpeed;
            fn_80026790(objectWork->modelID, &position);
            if (objectWork->frames & 1) {
                fn_80026A44(objectWork->modelID, (1 << 0));
            } else {
                fn_800269E8(objectWork->modelID, (1 << 0));
            }
            if (objectWork->frames-- <= 0) {
                objectWork->state = 0;
                fn_800269E8(objectWork->modelID, (1 << 0));
            }
        }
        objectIndex++;
    }
    if (playerWork->effect.main.effectState == 7 && playerWork->action == 0) {
        objectIndex = 0;
        while (objectIndex < 6) {
            if (lbl_1_bss_2E8.bouncing[objectIndex].state != 0) {
                break;
            }
            objectIndex++;
        }
        if (objectIndex == 6) {
            playerWork->effect.main.effectState = 0;
        }
    }
}

/* Applies the selected action to the main player when called by fn_1_4768. */
void fn_1_B5C4(u16 flags) {
    s16 playerIndex;
    M665PlayerWork *playerWork;

    playerIndex = lbl_1_bss_250.mainPlayer;
    playerWork = &lbl_1_bss_10[playerIndex];
    if (((flags & 2) != 0) && (playerWork->action == 0)) {
        s16 callPlayerIndex = lbl_1_bss_250.mainPlayer;
        fn_1_170C(callPlayerIndex, 3, 8.0f);
        playerWork->action = 33;
        playerWork->effect.main.effectState = 7;
    }
}

/* Tests a position against the selected effect group; killer slots are tested regardless of
 * state. */
s32 fn_1_B66C(Point3d *position) {
    M665PlayerWork *work;
    s16 *playerWork;
    Point3d modelPosition;
    Point3d modelRotation;
    s32 index;

    work = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
    switch (work->effect.main.effectState) {
    case 2: {

        for (index = 0; index < 6; index++) {
            switch (lbl_1_bss_2E8.missiles[index].state) {
            case 2:
                break;
            default:
                continue;
            }
            fn_800267CC(
                lbl_1_bss_2E8.missiles[index].modelID,
                &modelPosition);
            fn_80026874(
                lbl_1_bss_2E8.missiles[index].modelID,
                &modelRotation);
            if (position->x <= modelPosition.x + fabs(80.0 * sin(3.141592653589793 *
                                            modelRotation.y / 180.0)) + 25.0) {
                if (position->x >= modelPosition.x -
                        fabs(80.0 * sin(3.141592653589793 *
                                          modelRotation.y / 180.0)) - 25.0) {
                        if (position->z <= modelPosition.z +
                            fabs(80.0 * cos(3.141592653589793 *
                                              modelRotation.y / 180.0)) +
                            25.0) {
                        if (position->z >= modelPosition.z -
                                fabs(80.0 * cos(3.141592653589793 *
                                                  modelRotation.y / 180.0)) -
                                    25.0) {
                            return 1;
                        }
                    }
                }
            }
        }
        break;
    }
    case 3:
        for (index = 0; index < 3; index++) {
            fn_800267CC(lbl_1_bss_2E8.killers[index].modelID,
                            &modelPosition);
            if (sqrtf(PSVECSquareDistance(&modelPosition, position)) <= 55.0f) {
                return 1;
            }
        }
        break;
    case 4:
        for (index = 0; index < 2; index++) {
            fn_800267CC(
                lbl_1_bss_2E8.laser.modelIDs[index],
                &modelPosition);
            if (position->x <= modelPosition.x +
                    100.0f * lbl_1_bss_2E8.laser.scale &&
                position->x >= modelPosition.x -
                    100.0f * lbl_1_bss_2E8.laser.scale &&
                modelPosition.z <= position->z +
                                   2500.0f * lbl_1_bss_2E8.laser.scale) {
                return 1;
            }
        }
        break;
    case 5:
        for (index = 0; index < 7; index++) {
            switch (lbl_1_bss_2E8.hazards[index].state) {
            case 3:
                fn_800267CC(
                    lbl_1_bss_2E8.hazards[index].model,
                    &modelPosition);
                if (sqrtf(PSVECSquareDistance(&modelPosition, position)) <= 64.0f) {
                    return 1;
                }
                break;
            case 4:
                fn_800267CC(
                    lbl_1_bss_2E8.hazards[index].firstAnimation,
                    &modelPosition);
                if (position->x <= modelPosition.x + 64.0f &&
                    position->x >= modelPosition.x - 64.0f &&
                    position->z <= modelPosition.z + 64.0f &&
                    position->z >= modelPosition.z - 64.0f) {
                    return 1;
                }
                break;
            }
        }
        break;
    case 6:
        for (index = 0; index < 10; index++) {
            if (lbl_1_bss_2E8.rolling[index].state != 2) {
                continue;
            }
            fn_80026874(
                lbl_1_bss_2E8.rolling[index].modelID,
                &modelRotation);
            fn_800267CC(
                lbl_1_bss_2E8.rolling[index].modelID,
                &modelPosition);
            modelPosition.y -= 50.0f *
                cos(3.141592653589793 * modelRotation.x / 180.0);
            modelPosition.z += 50.0f *
                sin(3.141592653589793 * modelRotation.x / 180.0);
            if (sqrtf(PSVECSquareDistance(&modelPosition, position)) <= 50.0f) {
                return 1;
            }
        }
        break;
    }
    return 0;
}

/* Selects the main player's work record during the sequence finish hook. */
void fn_1_BFD8(void) {
    M665PlayerWork *playerWork;

    playerWork = &lbl_1_bss_10[lbl_1_bss_250.mainPlayer];
}

/* Starts the main player's opening action from the sequence fade-in hook. */
void fn_1_C004(void) {
    s16 playerIndex;
    M665PlayerWork *playerWork;

    playerIndex = lbl_1_bss_250.mainPlayer;
    playerWork = &lbl_1_bss_10[playerIndex];
    if (playerWork->action == 0) {
        s16 callPlayerIndex = lbl_1_bss_250.mainPlayer;
        fn_1_170C(callPlayerIndex, 2, 8.0f);
        playerWork->action = 1;
    }
}

/* Hit processing in fn_1_13024 adds one damage point to the main vehicle during sequence mode 5. */
void fn_1_C090(void) {
    void *unused;
    if (fn_80071A8C() == 5)
    {
        lbl_1_bss_10[lbl_1_bss_250.mainPlayer].models.main.damage += 1;
    }
}

/* Hides stage effects and resets the main vehicle during the pre-winner hook. */
void fn_1_C0E0(void) {
    s32 index;
    s32 effectIndex;

    for (index = 0; index < 16; index++) {
        fn_800269E8(lbl_1_bss_2E8.fire[index].modelA, (1 << 0));
        fn_800269E8(lbl_1_bss_2E8.fire[index].modelB, (1 << 0));
    }
    for (index = 0; index < 6; index++) {
        fn_800269E8(lbl_1_bss_2E8.missiles[index].modelID, (1 << 0));
        fn_800269E8(lbl_1_bss_2E8.missiles[index].collisionModelID, (1 << 0));
        lbl_1_bss_2E8.missiles[index].state = 0;
    }
    for (index = 0; index < 3; index++) {
        fn_800269E8(lbl_1_bss_2E8.killers[index].modelID, (1 << 0));
        for (effectIndex = 0; effectIndex < 32; effectIndex++) {
            fn_800269E8(lbl_1_bss_2E8.killers[index].trail[effectIndex].modelID,
                             (1 << 0));
        }
        lbl_1_bss_2E8.killers[index].state = 0;
    }
    for (index = 0; index < 10; index++) {
        fn_800269E8(lbl_1_bss_2E8.rolling[index].modelID, (1 << 0));
        lbl_1_bss_2E8.rolling[index].state = 0;
    }
    for (index = 0; index < 2; index++) {
        fn_800269E8(lbl_1_bss_2E8.laser.modelIDs[index], (1 << 0));
    }
    for (index = 0; index < 7; index++) {
        fn_800269E8(lbl_1_bss_2E8.hazards[index].model, (1 << 0));
        fn_800269E8(lbl_1_bss_2E8.hazards[index].firstAnimation, (1 << 0));
        fn_800269E8(lbl_1_bss_2E8.hazards[index].secondAnimation, (1 << 0));
        lbl_1_bss_2E8.hazards[index].state = 0;
    }
    for (index = 0; index < 6; index++) {
        fn_800269E8(lbl_1_bss_2E8.bouncing[index].modelID, (1 << 0));
    }
    fn_8002D414(lbl_1_bss_10[lbl_1_bss_250.mainPlayer].vehicleModel, 0.0f);
    fn_8002D86C(lbl_1_bss_10[lbl_1_bss_250.mainPlayer].vehicleModel, 0.0f);
    lbl_1_bss_10[lbl_1_bss_250.mainPlayer].action = 0;
    lbl_1_bss_10[lbl_1_bss_250.mainPlayer].actionFrames = 0;
}

M665StageWork lbl_1_bss_2E8;
s16 lbl_1_bss_2E4;
void *lbl_1_bss_2E0;
M665StageProgress lbl_1_bss_280[6];
