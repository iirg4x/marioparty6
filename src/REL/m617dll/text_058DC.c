/* Player movement, light, collision, and audio updates for Light Up My Night. */
#include "dolphin/math.h"

#include "dolphin/types.h"

#include "dolphin/gx.h"

#include "dolphin/os.h"

#include "dolphin/mtx.h"

#include "string.h"

#include "game/main.h"

#include "game/hu3d.h"

#include "game/data.h"

#include "game/memory.h"

#include "game/sprite.h"

#include "REL/m617dll/shared.h"

#include "game/object.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/gamework.h"

#include "game/mg/seqman.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/wipe.h"

#include "game/mg/actman.h"

#include "datadir_enum.h"

#include "game/charman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "dolphin.h"

#include "humath.h"

#include "messdir_enum.h"

#include "game/audio.h"

#include "game/window.h"

#include "game/process.h"

#include "game/flag.h"

#include "game/armem.h"

#include "game/mgdata.h"

#include "game/printfunc.h"

#include "game/saveload.h"

#include "game/esprite.h"

#include "game/thpmain.h"

#include "game/board/main.h"

#include "game/board/audio.h"

#include "game/board/masu.h"

#include "game/board/player.h"

s32 frandmod(s32 maximum);

extern int printf(const char *format, s32 modelId, s32 lightId);

typedef struct M617ParticleVector {
    f32 x;
    f32 y;
    f32 z;
} M617ParticleVector;

typedef struct M617ParticleColor {
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} M617ParticleColor;

typedef struct M617ParticleParameters {
    u32 flags;
    M617ParticleVector playerMovementOffset;
    M617ParticleColor startColor;
    M617ParticleColor targetColor;
    f32 velocityScale[3];
    f32 verticalAcceleration;
    f32 rotationIncrement;
    f32 scaleIncrement;
    f32 scaleIncrementMultiplier;
    f32 alphaIncrement;
    f32 colorProgress;
} M617ParticleParameters;

typedef struct M617ParticleEntry {
    s16 framesRemaining; /* Particle lifetime in updates. */
    s16 playerIndex; /* Player associated with the particle. */
    u32 unreadWord; /* Not accessed by this file. */
    M617ParticleVector velocity;
    M617ParticleVector emissionPosition;
    M617ParticleVector velocityOffset;
    f32 scale;
    f32 scaleY;
    f32 rotationZ;
    M617ParticleVector position;
    M617ParticleColor color;
} M617ParticleEntry;

typedef struct M617ParticleContext {
    u8 unreadPrefix[2]; /* Not accessed by this file. */
    s16 lastEntryIndex; /* Index most recently initialized by the emitter. */
    u8 unreadBeforeParameters[24]; /* Not accessed by this file. */
    M617ParticleParameters *parameters; /* Shared parameters for the particle entries. */
    u8 unreadBeforeCount[16]; /* Not accessed by this file. */
    s16 entryCount; /* Number of entries in the particle array. */
    u8 unreadAfterCount[2]; /* Not accessed by this file. */
    void *unreadPointer; /* Not accessed by this file. */
    u8 unreadBeforeEntries[16]; /* Not accessed by this file. */
    M617ParticleEntry *entries; /* Particle array passed to the update callback. */
} M617ParticleContext;

typedef struct M617CopyColor {
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} M617CopyColor;

typedef struct M617CopyData {
    u32 value00;
    f32 playerOffsetX;
    f32 playerOffsetY;
    f32 playerOffsetZ;
    M617CopyColor color;
    u8 targetColor[4];
    f32 animationParameters[9];
} M617CopyData;

typedef struct M617Vector3 {
    f32 x;
    f32 y;
    f32 z;
} M617Vector3;

typedef struct M617CopyParticleEntry {
    s16 framesRemaining; /* Particle lifetime in updates. */
    s16 playerIndex; /* Player associated with the particle. */
    u32 unreadWord; /* Not accessed by this file. */
    M617Vector3 velocity;
    M617Vector3 emissionPosition;
    f32 offsetX;
    f32 offsetY;
    f32 offsetZ;
    f32 scale;
    u32 scaleYBits;
    u32 rotationZBits;
    M617Vector3 position;
    M617CopyColor color;
} M617CopyParticleEntry;

typedef struct M617CopyParticleContext {
    u8 beforeState[2]; /* Not accessed by this file. */
    s16 lastEntryIndex; /* Index most recently initialized by the emitter. */
    u8 beforeEntryData[24]; /* Not accessed by this file. */
    M617CopyData *entryData; /* Parameters copied when a particle is created. */
    u8 beforeEntryCount[16]; /* Not accessed by this file. */
    s16 entryCount; /* Number of entries in the particle array. */
    u8 beforeEntries[22]; /* Not accessed by this file. */
    M617CopyParticleEntry *entries; /* Destination particle array. */
} M617CopyParticleContext;

void fn_1_484C(HU3D_MODEL *model, M617ParticleContext *context, Mtx matrix);

void fn_1_3DE0(void);

s16 fn_1_463C(s16 modelIndex, s16 playerIndex, M617Vector3 emissionPosition, M617Vector3 velocity,
              f32 initialScale, s16 framesRemaining, const M617CopyData *copyData);

/* Update an active sound slot's pan. */
void fn_1_58DC(s16 slotIndex, s16 pan)
{
    if ((lbl_1_bss_9C8.soundActive[slotIndex] == 1) &&
        (HuAudFXStatusGet(lbl_1_bss_9C8.soundId[slotIndex]) == 2)) {
        HuAudFXPanning(lbl_1_bss_9C8.soundId[slotIndex], pan);
    }
}

/* Update an active sound slot's pitch. */
void fn_1_596C(s16 slotIndex, s16 pitch)
{
    if ((lbl_1_bss_9C8.soundActive[slotIndex] == 1) &&
        (HuAudFXStatusGet(lbl_1_bss_9C8.soundId[slotIndex]) == 2)) {
        HuAudFXPitchSet(lbl_1_bss_9C8.soundId[slotIndex], pitch);
    }
}

/* Update an active sound slot's volume. */
void fn_1_59FC(s16 slotIndex, s16 volume)
{
    if ((lbl_1_bss_9C8.soundActive[slotIndex] == 1) &&
        (HuAudFXStatusGet(lbl_1_bss_9C8.soundId[slotIndex]) == 2)) {
        HuAudFXVolSet(lbl_1_bss_9C8.soundId[slotIndex], volume);
    }
}

void HuAudAllStop(void);

void HuAudFXStop(int seNo);

s32 HuAudFXStatusGet(int seNo);

typedef struct M617CursorPoint {
    f32 x;
    u8 unreadBetweenCoordinates[4]; /* Not accessed by this file. */
    f32 y;
} M617CursorPoint;

/* Stop a tracked sound slot when its handle is in a stoppable status. */
void fn_1_5A8C(s16 slotIndex) {
    if ((lbl_1_bss_9C8.soundActive[slotIndex] == 1) &&
        ((HuAudFXStatusGet(lbl_1_bss_9C8.soundId[slotIndex]) == 2) ||
         (HuAudFXStatusGet(lbl_1_bss_9C8.soundId[slotIndex]) == 3))) {
        lbl_1_bss_9C8.soundActive[slotIndex] = -1;
        HuAudFXStop(lbl_1_bss_9C8.soundId[slotIndex]);
    }
}

/* Stop a tracked sound slot when its handle is in a stoppable status. */
void fn_1_5B54(s16 slotIndex) {
    if ((lbl_1_bss_9C8.soundActive[slotIndex] == 1) &&
        ((HuAudFXStatusGet(lbl_1_bss_9C8.soundId[slotIndex]) == 2) ||
         (HuAudFXStatusGet(lbl_1_bss_9C8.soundId[slotIndex]) == 3))) {
        lbl_1_bss_9C8.soundActive[slotIndex] = -1;
        HuAudFXStop(lbl_1_bss_9C8.soundId[slotIndex]);
    }
}

void fn_1_5C1C(void) {
    HuAudAllStop();
}

extern s16 lbl_1_bss_38E8;

void fn_1_5DB4(void);

void fn_1_2B48(void);

void fn_1_78C0(void);

s16 fn_1_24D8(void);

s16 fn_1_5DE8(void);

/* MgSeq calls this phase callback each frame to update the game and report completion. */
int fn_1_5C3C(void)
{
    s16 result = 0;

    if (lbl_1_bss_38E8 == 0) {
        lbl_1_bss_AB0[1].motionState =
            lbl_1_bss_AB0[0].motionState = 0;

        lbl_1_bss_0[20].tpLevel = 1.0f;
        lbl_1_bss_0[21].tpLevel = 1.0f;
        MgScoreBoxTPLvlSet(lbl_1_bss_0[20].spriteId,
                           lbl_1_bss_0[20].tpLevel);
        MgScoreBoxTPLvlSet(lbl_1_bss_0[21].spriteId,
                           lbl_1_bss_0[21].tpLevel);

        lbl_1_bss_370[1].scaleY =
            lbl_1_bss_370[1].scaleX =
            lbl_1_bss_370[0].scaleY =
            lbl_1_bss_370[0].scaleX = 1.0f;
        HuSprGrpScaleSet(lbl_1_bss_370[0].groupId,
                         lbl_1_bss_370[0].scaleX,
                         lbl_1_bss_370[0].scaleY);
        HuSprGrpScaleSet(lbl_1_bss_370[1].groupId,
                         lbl_1_bss_370[1].scaleX,
                         lbl_1_bss_370[1].scaleY);

        lbl_1_bss_38E8 = 1;
    }

    fn_1_5DB4();
    fn_1_2B48();
    fn_1_78C0();
    fn_1_24D8();
    result = fn_1_5DE8();
    return result;
}

void fn_1_730(void);

void fn_1_16898(void);

void fn_1_5FC0(void);

void fn_1_6DEC(void);

void fn_1_7048(void);

void fn_1_18BD4(void);

/* Run the player, light, and model updates used by the minigame's frame callbacks. */
void fn_1_5DB4(void)
{
    fn_1_730();
    fn_1_16898();
    fn_1_5FC0();
    fn_1_6DEC();
    fn_1_7048();
    fn_1_18BD4();
}

void fn_1_3028(MGTIMER *timer);

s32 fn_1_3054(MGTIMER *timer);

void fn_1_307C(MGTIMER *timer);

/* Check player results and the timer; fn_1_5C3C uses the return to advance the sequence. */
s16 fn_1_5DE8(void)
{
    s16 result = 0;
    s16 resultCue = 0;
    s16 playerIndex;

    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        M617PlayerStateData *playerWork = &lbl_1_bss_AB0[playerIndex];

        if (playerWork->progress >= 5) {
            s32 characterIndex;

            playerWork->resultState = playerWork->character;
            characterIndex = playerWork->playerIndex;
            if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                GwPlayer[characterIndex].mgCoinBonus = 10;
            }
            result = 1;
        }
    }

    /* Start the countdown after 16200 earlier updates of the active game phase. */
    if (lbl_1_bss_9C8.timerFrames++ == 16200) {
        fn_1_3028(lbl_1_bss_9C8.timer);
    }
    if ((lbl_1_bss_9C8.timerFrames >= 16201) &&
        (fn_1_3054(lbl_1_bss_9C8.timer) != 0)) {
        lbl_1_bss_AB0[0].motionState = 1;
        lbl_1_bss_AB0[1].motionState = 1;
        if ((lbl_1_bss_AB0[0].completionState == 0) &&
            (lbl_1_bss_AB0[1].completionState == 0)) {
            resultCue = 1;
        }
    }

    if (result != 0) {
        fn_1_307C(lbl_1_bss_9C8.timer);
    }

    if ((result != 0) || (resultCue != 0)) {
        lbl_1_bss_AB0[0].motionState = 1;
        lbl_1_bss_AB0[1].motionState = 1;
        MgSeqWinnerSet((s16)lbl_1_bss_AB0[0].resultState,
                       (s16)lbl_1_bss_AB0[1].resultState, -1, -1);
        result = 1;
    }

    return result;
}

s16 fn_1_3BB8(s16, s16);

s32 fn_1_3CD0(s16, s16);

typedef struct M617VoiceWork {
    u8 unreadBeforePosition[52]; /* Player state preceding the sound's source position. */
    Point3d position; /* Position used for positional sound. */
} M617VoiceWork;

void fn_1_4DB0(M617VoiceWork *, s16);

void fn_1_54B8(s16, s16, Vec *);

/* Update player actions, light claims, and their motion phases from the frame callback. */
void fn_1_5FC0(void) {
    Point3d delta;
    f32 distance;
    f32 targetAngle;
    f32 currentAngle;
    s16 itemIndex;
    s16 claimedThisFrame;
    s16 claimedItem;
    s16 modelIndex;
    s16 playerIndex;
    s16 randomPlayer;
    M617LightSource *light;
    M617PlayerStateData *player;
    M617ModelSetupRecord *model;

    claimedThisFrame = 0;
    claimedItem = -1;
    playerIndex = 0;
    while (playerIndex < 2) {
        player = &lbl_1_bss_AB0[playerIndex];
        fn_1_4DB0((M617VoiceWork *)player, 0);
        PSVECSubtract((Point3d *) &player->position.x, &player->target, &delta);
        distance = (f32)sqrt((f64)
            ((delta.x * delta.x) + (delta.z * delta.z)));
        if (distance == 0.0f) {
            player->rangeIndex = 0;
        } else if (distance < 5.0f) {
            player->rangeIndex = 1;
        } else {
            player->rangeIndex = 2;
        }
        modelIndex = 2;
        while (modelIndex <= 14) {
            if (lbl_1_bss_458[modelIndex].modelId != -1) {
                light = &lbl_1_bss_458[modelIndex];
                delta.x = light->position.x - player->position.x;
                delta.z = light->position.z - player->position.z;
                distance = (f32)sqrt((f64)
                    ((delta.x * delta.x) + (delta.z * delta.z)));
                if (distance > 200.0f) {
                    fn_1_3CD0(lbl_1_bss_E88[player->participantIndex].modelId,
                              light->localLightPoolId);
                } else {
                    fn_1_3BB8(lbl_1_bss_E88[player->participantIndex].modelId,
                              light->localLightPoolId);
                }
            }
            modelIndex += 1;
        }
        switch (player->completionState) {
        case 0:
            if ((s32) (player->input.buttonsDown & PAD_BUTTON_A) != 0) {
                fn_1_54B8(1, 0, &player->position);
                player->actionFrame = 0;
                player->motionState = 2;
                player->completionState += 1;
            }
            break;
        case 1:
            if (player->actionFrame++ >= 5) {
                lbl_1_bss_DC0[playerIndex].previousMode = -1;
                lbl_1_bss_DC0[playerIndex].previousAttributes = -1;
                lbl_1_bss_DC0[player->participantIndex].mode = 6;
                lbl_1_bss_DC0[player->participantIndex].speed = 1.0f;
                lbl_1_bss_DC0[player->participantIndex].time = 0.0f;
                lbl_1_bss_DC0[player->participantIndex].attributes = 0;
                lbl_1_bss_DC0[player->participantIndex].attributesToReset = HU3D_MOTATTR_LOOP;
                player->completionState += 1;
            }
            break;
        case 2:
            if (lbl_1_bss_DC0[player->participantIndex].time >= 13.0f) {
                for (modelIndex = 46; modelIndex <= 54; modelIndex++) {
                    model = &lbl_1_bss_E88[modelIndex];
                    delta.x = model->position.x - player->position.x;
                    delta.z = model->position.z - player->position.z;
                    distance = (f32)sqrt((f64)
                        ((delta.x * delta.x) + (delta.z * delta.z)));
                    if (distance < (40.0f + (50.0f + lbl_1_bss_9C8.limit))) {
                        targetAngle = (f32) (180.0 * (atan2((f64) delta.x, (f64) delta.z) /
                                                      3.141592653589793));
                        currentAngle = player->rotation.y;
                        if (((targetAngle > 90.0f) && (currentAngle < -90.0f)) ||
                            ((targetAngle < -90.0f) && (currentAngle > 90.0f))) {
                            targetAngle =
                                targetAngle < 0.0f ? 180.0f + targetAngle : 180.0f - targetAngle;
                            currentAngle =
                                currentAngle < 0.0f ? 180.0f + currentAngle : 180.0f - currentAngle;
                            distance = targetAngle + currentAngle > 0.0f
                                           ? targetAngle + currentAngle
                                           : -1.0f * (targetAngle + currentAngle);
                        } else {
                            distance = targetAngle - currentAngle > 0.0f
                                           ? targetAngle - currentAngle
                                           : -1.0f * (targetAngle - currentAngle);
                        }
                        if (distance < 60.0f) {
                            itemIndex = modelIndex - 44;
                            if (lbl_1_bss_458[itemIndex].enabled == 0) {
                                lbl_1_bss_458[itemIndex].enabled = 1;
                                claimedThisFrame = 1;
                                claimedItem = itemIndex;
                                player->progress += 1;
                                Hu3DModelAttrReset(lbl_1_bss_E88[playerIndex + 94].modelId,
                                                  HU3D_ATTR_DISPOFF);
                                Hu3DMotionTimeSet(lbl_1_bss_E88[playerIndex + 94].modelId, 0.0f);
                                Hu3DMotionSpeedSet(lbl_1_bss_E88[playerIndex + 94].modelId, 1.0f);
                                lbl_1_bss_E88[playerIndex + 94].position.x = model->position.x;
                                lbl_1_bss_E88[playerIndex + 94].position.y =
                                    100.0f + model->position.y;
                                lbl_1_bss_E88[playerIndex + 94].position.z = model->position.z;
                                omVibrate((s16) player->playerIndex, 12, 7, 3);
                                fn_1_54B8(2, 0, &model->position);
                                itemIndex = modelIndex + 37;
                                lbl_1_bss_E88[itemIndex].motionState = 1;
                                lbl_1_bss_E88[itemIndex].position.x = model->position.x;
                                lbl_1_bss_E88[itemIndex].position.y = 150.0f + model->position.y;
                                lbl_1_bss_E88[itemIndex].position.z = 20.0f + model->position.z;
                                break;
                            } else {
                                if ((claimedThisFrame != 0) && (claimedItem != -1) &&
                                    (itemIndex == claimedItem)) {
                                    /* A simultaneous claim rerolls the point awarded to player
                                     * zero. */
                                    lbl_1_bss_AB0->progress -= 1;
                                    randomPlayer = frandmod(2);
                                    lbl_1_bss_AB0[randomPlayer].progress += 1;
                                }
                                continue;
                            }
                        } else {
                            continue;
                        }
                    } else {
                        continue;
                    }
                }
                player->completionState += 1;
            }
            break;
        case 3:
            if (lbl_1_bss_DC0[player->participantIndex].time >= 32.9f) {
                lbl_1_bss_DC0[playerIndex].previousMode = -1;
                lbl_1_bss_DC0[playerIndex].previousAttributes = -1;
                lbl_1_bss_DC0[player->participantIndex].mode = 3;
                lbl_1_bss_DC0[player->participantIndex].speed = 1.0f;
                lbl_1_bss_DC0[player->participantIndex].time = 0.0f;
                lbl_1_bss_DC0[player->participantIndex].attributes = HU3D_MOTATTR_LOOP;
                lbl_1_bss_DC0[player->participantIndex].attributesToReset = 0;
                player->completionState += 1;
            }
            break;
        case 4:
            player->motionState = 0;
            player->completionState = 0;
            break;
        }
        playerIndex += 1;
    }
    playerIndex = 94;
    while (playerIndex <= 95) {
        if (Hu3DMotionTimeGet(lbl_1_bss_E88[playerIndex].modelId) >= 40.0f) {
            Hu3DModelAttrSet(lbl_1_bss_E88[playerIndex].modelId, HU3D_ATTR_DISPOFF);
        }
        playerIndex += 1;
    }
}

/* Advance the scale animations for player and stage models from fn_1_5DB4. */
void fn_1_6DEC(void)
{
    M617PlayerStateData *player;
    s16 modelIndex;

    for (modelIndex = 81; modelIndex <= 91; modelIndex++) {
        M617ModelSetupRecord *model = &lbl_1_bss_E88[modelIndex];
        Point3d scale;

        if (modelIndex <= 82) {
            player = &lbl_1_bss_AB0[modelIndex - 81];
        }

        Hu3DModelScaleGet(model->modelId, &scale);
        switch (model->motionState) {
        case 0:
            break;
        case 1:
            Hu3DModelAttrReset(lbl_1_bss_E88[modelIndex].modelId, HU3D_ATTR_DISPOFF);
            model->motionState++;
            /* The first visible frame also starts the scale increase. */
        case 2:
            scale.x += 0.05f;
            scale.y += 0.05f;
            scale.z += 0.05f;
            if (scale.x >= 1.0f) {
                model->scaleStep = 0.0125f;
                model->motionState++;
            }
            break;
        case 3:
            if (modelIndex <= 82) {
                if (scale.x > player->upperScaleLimit[player->rangeIndex]) {
                    model->scaleStep = -0.0125f;
                } else if (scale.x < player->lowerScaleLimit[player->rangeIndex]) {
                    model->scaleStep = 0.0125f;
                }
            } else if ((scale.x > 1.05f) ||
                       (scale.x < 0.95f)) {
                model->scaleStep *= -1.0f;
            }
            scale.x += model->scaleStep;
            scale.y += model->scaleStep;
            scale.z += model->scaleStep;
            break;
        }
        Hu3DModelScaleSetV(model->modelId, &scale);
    }
}

typedef struct M617LightOffsets {
    Vec values[4]; /* Particle origins for the four corner lights. */
} M617LightOffsets;

static const M617LightOffsets lightOffsetsSource = {
    {{-788.0f, 60.0f, -788.0f}, {788.0f, 60.0f, -788.0f},
     {-788.0f, 60.0f, 788.0f}, {788.0f, 60.0f, 788.0f}}
};

s16 fn_1_435C(s16 playerIndex, const Vec *position);

s16 fn_1_4070(const Vec *position);

/* Update stage lights and their attached models from the minigame frame callbacks. */
void fn_1_7048(void)
{
    M617LightOffsets lightOffsets;
    f32 movementSpeed;
    f32 lightOffsetScale;
    f32 adjustedHeight;
    f32 wave;
    s16 lightIndex;
    M617ModelSetupRecord *model;

    lightOffsets = lightOffsetsSource;
    lightIndex = 0;
    while (lightIndex < 15) {
        if (lbl_1_bss_458[lightIndex].modelId != -1) {
            M617LightSource *light = &lbl_1_bss_458[lightIndex];

            if (lightIndex >= 0 && lightIndex <= 1) {
                M617PlayerStateData *player = &lbl_1_bss_AB0[lightIndex];

                if (lbl_1_bss_DC0[lightIndex].mode != 6) {
                    player->lightRadius = 50.9f;
                    player->lightAngle = -63.14f;
                    player->lightHeight = 136.0f;
                } else {
                    if (lbl_1_bss_DC0[lightIndex].time >=
                        17.0f) {
                        adjustedHeight = lbl_1_bss_DC0[lightIndex].time -
                            (2.0f *
                             (lbl_1_bss_DC0[lightIndex].time -
                              17.0f));
                    } else {
                        adjustedHeight =
                            lbl_1_bss_DC0[lightIndex].time;
                    }
                    lightOffsetScale = adjustedHeight;
                    player->lightRadius = 50.9f +
                        0.94705874f * lightOffsetScale;
                    player->lightAngle = -63.14 +
                        3.7141175f * lightOffsetScale;
                    player->lightHeight = 136.0f +
                        -2.1176472f * lightOffsetScale;
                }
                light->position.x = (f32)((f64)player->position.x +
                    (f64)player->lightRadius *
                    sin((3.141592653589793 *
                         (f64)(player->rotation.y + player->lightAngle)) /
                        180.0));
                light->position.y = player->lightHeight;
                light->position.z = (f32)((f64)player->position.z +
                    (f64)player->lightRadius *
                    cos((3.141592653589793 *
                         (f64)(player->rotation.y + player->lightAngle)) /
                        180.0));
                light->rotation.y = player->rotation.y;
                light->distance = light->position.y;
                lbl_1_bss_E88[lightIndex + 81].position.x =
                    light->position.x;
                lbl_1_bss_E88[lightIndex + 81].position.y =
                    2.0f + (f32)lightIndex;
                lbl_1_bss_E88[lightIndex + 81].position.z =
                    light->position.z;
                Hu3DModelObjPosGet(lbl_1_bss_E88[lightIndex].modelId,
                    "new_torch-sentan",
                    (Point3d *)&light->objectPosition);
                player->snapshotPosition = player->previousPosition;
                player->previousPosition = light->objectPosition;
                PSVECSubtract((Vec *)&player->previousPosition,
                    (Vec *)&player->snapshotPosition,
                    (Vec *)&player->movement);
                movementSpeed = light->intensity -
                    player->upperIntensityLimit[player->rangeIndex];
                if (movementSpeed > 0.0f) {
                    wave = movementSpeed;
                } else {
                    wave = -1.0f * movementSpeed;
                }
                movementSpeed = wave;
                if (light->intensity >
                    player->upperIntensityLimit[player->rangeIndex]) {
                    f32 aboveTargetIntensity;

                    if (movementSpeed >= 0.1f) {
                        aboveTargetIntensity = -0.011f;
                    } else {
                        aboveTargetIntensity = -0.005f;
                    }
                    light->intensityStep = aboveTargetIntensity;
                } else if (light->intensity <
                    player->lowerIntensityLimit[player->rangeIndex]) {
                    f32 belowTargetIntensity;

                    if (movementSpeed >= 0.1f) {
                        belowTargetIntensity = 0.011f;
                    } else {
                        belowTargetIntensity = 0.005f;
                    }
                    light->intensityStep = belowTargetIntensity;
                }
            }
            if (lightIndex >= 2 && lightIndex <= 10) {
                if (light->enabled != 0) {
                    model = &lbl_1_bss_E88[light->modelIndex];
                    light->position.x = model->position.x +
                        light->referencePosition.x;
                    light->position.z = model->position.z +
                        light->referencePosition.z;
                    light->distance = light->position.y;
                    light->objectPosition = model->position;
                    light->objectPosition.y = 100.0f +
                        model->position.y;
                    if (light->intensity > 0.52f || light->intensity < 0.48f) {
                        light->intensityStep *= -1.0f;
                    }
                } else {
                    /* Park an unlit stage light outside the arena. */
                    light->position.x = 10000.0f;
                }
            }
            if (lightIndex >= 11 && lightIndex <= 14) {
                model = &lbl_1_bss_E88[light->modelIndex];
                light->position.x = model->position.x +
                    light->referencePosition.x;
                light->position.z = model->position.z +
                    light->referencePosition.z;
                light->distance = light->position.y;
                light->enabled = 1;
                light->objectPosition = lightOffsets.values[lightIndex - 11];
                if (light->intensity > 0.42f || light->intensity < 0.38f) {
                    light->intensityStep *= -1.0f;
                }
            }
            if (light->enabled != 0) {
                light->intensity += light->intensityStep;
                if (lightIndex <= 1) {
                    fn_1_435C(lightIndex, (Vec *)&light->objectPosition);
                } else {
                    if (light->frame++ >= 1) {
                        fn_1_4070((Vec *)&light->objectPosition);
                        light->frame = 0;
                    }
                }
            }
            Hu3DLLightPosSetV(light->modelId, light->lightId,
                (Point3d *)&light->position,
                (Point3d *)&light->direction);
            Hu3DLLightPointSet(light->modelId, light->lightId,
                light->distance, light->intensity, GX_DA_STEEP);
        }
        lightIndex++;
    }
}

typedef struct M617PlayerTask {
    u8 firstParameterBytes[4]; /* First narrow-phase parameter, not read here. */
    s32 playerIndex; /* Second narrow-phase parameter, set to the player slot. */
    s32 collisionType; /* Narrow-phase category; zero identifies another player. */
    u8 unreadCollisionData[104]; /* Not accessed by this file. */
    u16 unreadTrailingValue; /* Not accessed by this file. */
} M617PlayerTask;

typedef struct M617PlayerIndexTask {
    u8 actorHeader[12]; /* Actor fields before its game-owned parameter. */
    s32 playerIndex; /* Actor parameter set to the player's slot during setup. */
    u8 actorMotionData[100]; /* Actor fields between its parameter and collision mesh. */
    u16 collisionMeshIndex; /* Actor collision mesh, copied after collision correction. */
} M617PlayerIndexTask;

typedef struct M617PlayerSetupParam {
    f32 height;
    f32 radius;
    s32 param;
    s32 type;
    COL_NARROW_HOOK narrowHook;
    u32 attr;
    s32 correctHookParam;
    MGACTOR_CORRECT_HOOK correctHook;
    MGACTOR_CORRECT_HOOK correctionCallbacks[16];
} M617PlayerSetupParam;

typedef struct M617BoxEdgeFlags {
    s8 flags[2][4]; /* Per-layout edge settings; -1 disables an edge. */
} M617BoxEdgeFlags;

MGTIMER *fn_1_2FB4(int type, int duration, int end, f32 x, f32 y);

void fn_1_3E4(void);

void fn_1_A438(void);

void fn_1_AA84(void);

void fn_1_187C8(void);

void fn_1_BA64();

void fn_1_E1F0();

void fn_1_ECC(void);

/* Collision narrow-phase hook: link the players for type zero; all cases return zero. */
s32 fn_1_780C(M617PlayerTask *first, M617PlayerTask *second)
{
    switch (second->collisionType) {
    case 0:
        lbl_1_bss_AB0[first->playerIndex].linkedPlayer = (s16) second->playerIndex;
        lbl_1_bss_AB0[second->playerIndex].linkedPlayer = (s16) first->playerIndex;
        lbl_1_bss_AB0[first->playerIndex].orbitCounter++;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        break;
    }
    return 0;
}

/* The actor correction hook copies this player's collision-mesh index into its work record. */
void fn_1_788C(M617PlayerIndexTask *task)
{
    s32 playerIndex;

    playerIndex = task->playerIndex;
    lbl_1_bss_AB0[playerIndex].collisionMeshIndex = (s16) task->collisionMeshIndex;
}

/* Resolve player overlap with other players, stage obstacles, and arena boundaries each frame. */
void fn_1_78C0(void)
{
    f32 edges[4][4];
    Point3d separation;
    Point3d movement;
    Point3d nearestPoint;
    Point3d halfSize;
    Point3d normalized;
    f32 lengthSquared;
    f32 projectionNumerator;
    f32 projection;
    f32 movementDifference;
    f32 distance;
    f32 minimumDistance;
    f32 pushScale;
    f32 obstacleAngle;
    f32 playerAngle;
    s16 playerIndex;
    s16 modelIndex;
    s16 edgeIndex;
    s16 collisionCount;
    s16 hit;
    s16 obstacleFrames;
    M617PlayerStateData *player;
    M617ModelSetupRecord *model;

    lbl_1_bss_9C8.limit = 30.0f;
    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        player = &lbl_1_bss_AB0[playerIndex];
        if (player->completionState != 0) {
            player->position = player->target;
            modelIndex = playerIndex ^ 1;
            separation.x = lbl_1_bss_AB0[modelIndex].position.x - player->position.x;
            separation.z = lbl_1_bss_AB0[modelIndex].position.z - player->position.z;
            distance = (f32) sqrt(
                (f64) (separation.x * separation.x + separation.z * separation.z));
            minimumDistance = 105.0f;
            if (distance < minimumDistance) {
                if (distance == 0.0f) distance = 1.0f;
                pushScale = minimumDistance / distance;
                separation.x *= pushScale;
                separation.z *= pushScale;
                lbl_1_bss_AB0[modelIndex].position.x = player->position.x + separation.x;
                lbl_1_bss_AB0[modelIndex].position.z = player->position.z + separation.z;
            }
        }
        player->previousObstacle = player->obstacle;
        player->obstacle = -1;
        minimumDistance = 50.0f + lbl_1_bss_9C8.limit;
        modelIndex = 46;
        player->targetObstructed = 0;
        for (; modelIndex <= 54; modelIndex++) {
            model = &lbl_1_bss_E88[modelIndex];
            separation.x = player->position.x - model->position.x;
            separation.z = player->position.z - model->position.z;
            distance = (f32) sqrt(
                (f64) (separation.x * separation.x + separation.z * separation.z));
            if (distance < minimumDistance) {
                if (distance == 0.0f) distance = 1.0f;
                pushScale = minimumDistance / distance;
                separation.x *= pushScale;
                separation.z *= pushScale;
                player->position.x = model->position.x + separation.x;
                player->position.z = model->position.z + separation.z;
                player->obstacle = modelIndex;
                player->selectionFrames = (s16) player->previousObstacle == player->obstacle
                                              ? player->selectionFrames + 1
                                              : 0;
                if (player->computerControlled != 0 && (s16)player->selectedModel != modelIndex) {
                    obstacleAngle = (f32) (180.0 * (atan2((f64) separation.x, (f64) separation.z) /
                                                    3.141592653589793));
                    separation.x = player->position.x - player->targetPosition.x;
                    separation.z = player->position.z - player->targetPosition.z;
                    playerAngle = (f32) (180.0 * (atan2((f64) separation.x, (f64) separation.z) /
                                                  3.141592653589793));
                    if ((obstacleAngle > 90.0f && playerAngle < -90.0f) ||
                        (obstacleAngle < -90.0f && playerAngle > 90.0f)) {
                        obstacleAngle =
                            obstacleAngle < 0.0f ? 180.0f + obstacleAngle : 180.0f - obstacleAngle;
                        playerAngle =
                            playerAngle < 0.0f ? 180.0f + playerAngle : 180.0f - playerAngle;
                        obstacleAngle = obstacleAngle + playerAngle > 0.0f
                                            ? obstacleAngle + playerAngle
                                            : -1.0f * (obstacleAngle + playerAngle);
                    } else {
                        obstacleAngle = obstacleAngle - playerAngle > 0.0f
                                            ? obstacleAngle - playerAngle
                                            : -1.0f * (obstacleAngle - playerAngle);
                    }
                    if (obstacleAngle < 5.0f) player->targetObstructed = 1;
                }
            }
        }
        player->previousCollisionModels[0] = player->collisionModels[0];
        player->previousCollisionModels[1] = player->collisionModels[1];
        player->previousCollisionEdges[0] = player->collisionEdges[0];
        player->previousCollisionEdges[1] = player->collisionEdges[1];
        player->collisionModels[1] = player->collisionModels[0] = -1;
        player->collisionEdges[1] = player->collisionEdges[0] = -1;
        collisionCount = 0;
        for (modelIndex = 55; modelIndex <= 78; modelIndex++) {
            model = &lbl_1_bss_E88[modelIndex];
            if (model->position.x != 0.0f) {
                if (model->rotation.y >= 90.0f) {
                    halfSize.x = model->dimensions.z;
                    halfSize.z = model->dimensions.x;
                } else halfSize = model->dimensions;
                edges[0][0] = model->position.x + halfSize.x;
                edges[0][1] = model->position.x - halfSize.x;
                edges[0][2] = model->position.z + halfSize.z;
                edges[0][3] = model->position.z + halfSize.z;
                edges[1][0] = model->position.x + halfSize.x;
                edges[1][1] = model->position.x - halfSize.x;
                edges[1][2] = model->position.z - halfSize.z;
                edges[1][3] = model->position.z - halfSize.z;
                edges[2][0] = model->position.x + halfSize.x;
                edges[2][1] = model->position.x + halfSize.x;
                edges[2][2] = model->position.z + halfSize.z;
                edges[2][3] = model->position.z - halfSize.z;
                edges[3][0] = model->position.x - halfSize.x;
                edges[3][1] = model->position.x - halfSize.x;
                edges[3][2] = model->position.z + halfSize.z;
                edges[3][3] = model->position.z - halfSize.z;
                if (49.0f + edges[0][0] > player->position.x &&
                    edges[0][1] - 49.0f < player->position.x &&
                    49.0f + edges[2][2] > player->position.z &&
                    edges[2][3] - 49.0f < player->position.z) {
                    for (edgeIndex = 0, hit = 0; edgeIndex < 4; edgeIndex++) {
                        if (lbl_1_bss_398[modelIndex - 55]
                                .links[lbl_1_bss_9C8.layout][edgeIndex] != -1) {
                            separation.x = edges[edgeIndex][0] - edges[edgeIndex][1];
                            separation.z = edges[edgeIndex][2] - edges[edgeIndex][3];
                            lengthSquared =
                                separation.x * separation.x + separation.z * separation.z;
                            projectionNumerator =
                                -1.0f * (separation.x * (edges[edgeIndex][0] - player->position.x) +
                                         separation.z * (edges[edgeIndex][2] - player->position.z));
                            projection = lengthSquared ? projectionNumerator / lengthSquared : 0.0f;
                            nearestPoint.x = separation.x * projection + edges[edgeIndex][0];
                            nearestPoint.z = separation.z * projection + edges[edgeIndex][2];
                            separation.x = player->position.x - nearestPoint.x;
                            separation.y = 0.0f;
                            separation.z = player->position.z - nearestPoint.z;
                            minimumDistance = (f32) sqrt(
                                (f64) (separation.x * separation.x + separation.z * separation.z));
                            if (separation.x == 0.0f && separation.z == 0.0f)
                                obstacleAngle = 999.0f;
                            else {
                                PSVECNormalize(&separation, &normalized);
                                obstacleAngle =
                                    (f32) (180.0 * (atan2((f64) normalized.x, (f64) normalized.z) /
                                                    3.141592653589793));
                                playerAngle =
                                    (f32) (180.0 *
                                           (atan2((f64) model->edgeDirections[edgeIndex].x,
                                                  (f64) model->edgeDirections[edgeIndex].z) /
                                            3.141592653589793));
                                if (playerAngle == 180.0f && obstacleAngle < 0.0f)
                                    playerAngle = -180.0f;
                                obstacleAngle = obstacleAngle - playerAngle > 0.0f
                                                    ? obstacleAngle - playerAngle
                                                    : -1.0f * (obstacleAngle - playerAngle);
                            }
                            movement.x = player->target.x - player->position.x;
                            movement.y = 0.0f;
                            movement.z = player->target.z - player->position.z;
                            if (movement.x == 0.0f && movement.z == 0.0f)
                                movementDifference = 999.0f;
                            else {
                                PSVECNormalize(&movement, &normalized);
                                movementDifference =
                                    (f32) (180.0 * (atan2((f64) normalized.x, (f64) normalized.z) /
                                                    3.141592653589793));
                                playerAngle =
                                    (f32) (180.0 *
                                           (atan2((f64) model->edgeDirections[edgeIndex].x,
                                                  (f64) model->edgeDirections[edgeIndex].z) /
                                            3.141592653589793));
                                if (playerAngle == 180.0f && movementDifference < 0.0f)
                                    playerAngle = -180.0f;
                                movementDifference =
                                    movementDifference - playerAngle > 0.0f
                                        ? movementDifference - playerAngle
                                        : -1.0f * (movementDifference - playerAngle);
                            }
                            if (minimumDistance < 50.0f && obstacleAngle < 10.0f &&
                                movementDifference < 90.0f) {
                                if (collisionCount < 2) {
                                    player->collisionModels[collisionCount] = modelIndex;
                                    player->collisionEdges[collisionCount] = edgeIndex;
                                    player->collisionDirections[collisionCount] =
                                        model->edgeDirections[collisionCount];
                                    collisionCount++;
                                }
                                pushScale = minimumDistance ? 50.0f / minimumDistance : 0.0f;
                                if (edgeIndex == 0) {
                                    separation.z =
                                        separation.z < 0.0f ? -1.0f * separation.z : separation.z;
                                } else if (edgeIndex == 1) {
                                    separation.z =
                                        separation.z > 0.0f ? -1.0f * separation.z : separation.z;
                                } else if (edgeIndex == 2) {
                                    separation.x =
                                        separation.x < 0.0f ? -1.0f * separation.x : separation.x;
                                } else if (edgeIndex == 3) {
                                    separation.x =
                                        separation.x > 0.0f ? -1.0f * separation.x : separation.x;
                                }
                                separation.x *= pushScale;
                                separation.z *= pushScale;
                                player->position.x = nearestPoint.x + separation.x;
                                player->position.z = nearestPoint.z + separation.z;
                                hit = 1;
                            }
                            if (hit != 0) break;
                        }
                    }
                }
            }
        }
        /* These first boundary segments are overwritten by the narrower pair below. */
        edges[0][0] = -670.0f; edges[0][1] = -870.0f; edges[0][2] = -870.0f; edges[0][3] = -670.0f;
        edges[1][0] = 670.0f; edges[1][1] = 870.0f; edges[1][2] = -870.0f; edges[1][3] = -670.0f;
        edges[0][0] = -631.0f; edges[0][1] = -831.0f; edges[0][2] = -831.0f; edges[0][3] = -631.0f;
        edges[1][0] = 631.0f; edges[1][1] = 831.0f; edges[1][2] = -831.0f; edges[1][3] = -631.0f;
        for (edgeIndex = 0; edgeIndex < 2; edgeIndex++) {
            separation.x = edges[edgeIndex][0] - edges[edgeIndex][1];
            separation.z = edges[edgeIndex][2] - edges[edgeIndex][3];
            lengthSquared = separation.x * separation.x + separation.z * separation.z;
            projectionNumerator =
                -1.0f * (separation.x * (edges[edgeIndex][0] - player->position.x) +
                         separation.z * (edges[edgeIndex][2] - player->position.z));
            projection = lengthSquared ? projectionNumerator / lengthSquared : 0.0f;
            nearestPoint.x = separation.x * projection + edges[edgeIndex][0];
            nearestPoint.z = separation.z * projection + edges[edgeIndex][2];
            separation.x = player->position.x - nearestPoint.x;
            separation.y = 0.0f;
            separation.z = player->position.z - nearestPoint.z;
            minimumDistance = (f32) sqrt(
                (f64) (separation.x * separation.x + separation.z * separation.z));
            if (minimumDistance < 50.0f) {
                pushScale = minimumDistance ? 50.0f / minimumDistance : 0.0f;
                separation.x *= pushScale;
                separation.z *= pushScale;
                player->position.x = nearestPoint.x + separation.x;
                player->position.z = nearestPoint.z + separation.z;
            }
        }
        edges[0][0] = -1110.0f; edges[0][1] = 860.0f;
        edges[1][0] = 1110.0f; edges[1][1] = 860.0f;
        for (edgeIndex = 0, hit = 0; edgeIndex < 2; edgeIndex++) {
            separation.x = player->position.x - edges[edgeIndex][0];
            separation.z = player->position.z - edges[edgeIndex][1];
            distance = (f32) sqrt(
                (f64) (separation.x * separation.x + separation.z * separation.z));
            minimumDistance = 500.0f;
            if (distance < minimumDistance) {
                hit = 1;
                if (distance == 0.0f) distance = 1.0f;
                pushScale = minimumDistance / distance;
                separation.x *= pushScale;
                separation.z *= pushScale;
                player->position.x = separation.x + edges[edgeIndex][0];
                player->position.z = separation.z + edges[edgeIndex][1];
            }
        }
        player->stageFrame = hit != 0 ? player->stageFrame + 1 : 0;
        if (player->position.x > 780.0f) player->position.x = 780.0f;
        if (player->position.x < -780.0f) player->position.x = -780.0f;
        if (player->position.z > 780.0f) player->position.z = 780.0f;
        if (player->position.z < -780.0f) player->position.z = -780.0f;
    }
}
