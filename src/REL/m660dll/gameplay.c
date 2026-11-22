/* Runs the stage's gameplay sequence, player actions, moving objects and collision updates. */
#include "REL/m660dll/effects.h"
#include "REL/m660dll/path_data.h"
#include "dolphin/math.h"
#include "game/hu3d.h"
#include "game/mg/actman.h"
#include "game/mg/timer.h"
#include "game/mg/seqman.h"
#include "game/audio.h"
#include "game/gamework.h"
#include "game/wipe.h"
#include "game/frand.h"
#include "REL/m660dll/scene.h"
#include "REL/m660dll/player_slots.h"
#include "REL/m660dll.h"
#include "humath.h"

s32 fn_1_7080(void);
void fn_1_71F0(void);
void fn_1_818C(void);
void fn_1_9050(void);
struct VoicePosition;
s32 fn_1_7940(s16 effectIndex);

/* Updates the pan of an active stage sound when its effect handle is playing. */
void fn_1_6BF8(s16 soundIndex, s16 pan)
{
    if (lbl_1_bss_1F4.soundStatus[soundIndex] == 1 &&
        HuAudFXStatusGet(lbl_1_bss_1F4.soundEffectHandle[soundIndex]) == 2) {
        HuAudFXPanning(lbl_1_bss_1F4.soundEffectHandle[soundIndex], pan);
    }
}

/* Updates the pitch of an active stage sound when its effect handle is playing. */
void fn_1_6C88(s16 soundIndex, s16 pitch)
{
    if (lbl_1_bss_1F4.soundStatus[soundIndex] == 1 &&
        HuAudFXStatusGet(lbl_1_bss_1F4.soundEffectHandle[soundIndex]) == 2) {
        HuAudFXPitchSet(lbl_1_bss_1F4.soundEffectHandle[soundIndex], pitch);
    }
}

/* Updates the volume of an active stage sound when its effect handle is playing. */
void fn_1_6D18(s16 soundIndex, s16 volume)
{
    if (lbl_1_bss_1F4.soundStatus[soundIndex] == 1 &&
        HuAudFXStatusGet(lbl_1_bss_1F4.soundEffectHandle[soundIndex]) == 2) {
        HuAudFXVolSet(lbl_1_bss_1F4.soundEffectHandle[soundIndex], volume);
    }
}

/* When its effect is playing or paused, marks the slot unused, then stops the effect. */
void fn_1_6DA8(s16 soundSlot) {
    if ((lbl_1_bss_1F4.soundStatus[soundSlot] == 1) &&
        ((HuAudFXStatusGet(lbl_1_bss_1F4.soundEffectHandle[soundSlot]) == 2) ||
         (HuAudFXStatusGet(lbl_1_bss_1F4.soundEffectHandle[soundSlot]) == 3))) {
        lbl_1_bss_1F4.soundStatus[soundSlot] = -1;
        HuAudFXStop(lbl_1_bss_1F4.soundEffectHandle[soundSlot]);
    }
}

/* Stops the sound in the specified stage audio slot. */
void fn_1_6E70(s16 soundSlot)
{
    fn_1_6DA8(soundSlot);
}

/* Stops all audio. */
void fn_1_6F38(void) {
    HuAudAllStop();
}

/* Runs one gameplay tick, initializing player collision state on the first tick. */
s32 fn_1_6F58(void)
{
    s32 result = 0;

    if (lbl_1_bss_2680[0] == 0) {
        lbl_1_bss_2DC[1].inputDisabled =
            lbl_1_bss_2DC[0].inputDisabled = 0;
        /* Both assignments set player 1's collision flag; player 0's flag is left unchanged. */
        lbl_1_bss_2DC[1].collisionActive = lbl_1_bss_2DC[1].collisionActive = 1;
        fn_1_3014(lbl_1_bss_1F4.gameTimer);
        lbl_1_bss_2680[0] = 1;
    }

    fn_1_7008();
    fn_1_2B34();
    fn_1_915C();
    fn_1_24C4();
    result = fn_1_7080();
    return (s16)result;
}

/* Updates player input and AI, actions, moving objects and stage lights, then emits particles
 * from two fixed positions. */
void fn_1_7008(void)
{
    Vec parameters[2] = {{-788.0f, 60.0f, -788.0f}, {788.0f, 60.0f, -788.0f}};

    fn_1_71C();
    fn_1_11F50();
    fn_1_71F0();
    fn_1_818C();
    fn_1_9050();
    fn_1_4004(parameters);
    fn_1_4004(&parameters[1]);
}

/* Checks for a winner or timer expiry and disables further player collisions. */
s32 fn_1_7080(void)
{
    s32 result = 0;
    s16 playerIndex = 0;

    while (playerIndex < 2) {
        if ((lbl_1_bss_2DC[playerIndex].actionState >= 2) &&
            (lbl_1_bss_2DC[playerIndex ^ 1].mode == -1)) {
            lbl_1_bss_2DC[playerIndex ^ 1].mode = 1;
            lbl_1_bss_1F4.tie = 0;
            result = 2;
        }
        playerIndex++;
    }

    if ((fn_1_3040(lbl_1_bss_1F4.gameTimer) != 0) && ((s16)result == 0)) {
        lbl_1_bss_1F4.tie = 1;
        result = 1;
    }

    if ((s16)result != 0) {
        if ((s16)result == 2) {
            fn_1_3068(lbl_1_bss_1F4.gameTimer);
        }
        lbl_1_bss_2DC[0].inputDisabled =
            lbl_1_bss_2DC[1].inputDisabled = 1;
        lbl_1_bss_2DC[0].collisionActive = lbl_1_bss_2DC[1].collisionActive = 0;
        lbl_1_bss_1F4.resultStarted = 1;
    }

    return result;
}

/* Advances each player's action, including the selected player's exit animation. */
void fn_1_71F0(void)
{
    Vec spawnPosition;
    Vec offsetPosition;
    f32 spawnDistance[3] = {20.0f, 150.0f, 300.0f};
    f32 horizontalOffsets[3] = {100.0f, 270.0f, 480.0f};
    f32 effectValues[3] = {60.0f, 120.0f, 200.0f};
    f32 heights[3] = {230.0f, 230.0f, 330.0f};
    s16 playerIndex;
    M660Model *target;
    M660Slot *slot;
    f32 distance;
    f32 travel;

    if (lbl_1_bss_2DC[0].actionState == 1 && lbl_1_bss_2DC[1].actionState == 1) {
        playerIndex = frandmod(2);
        lbl_1_bss_2DC[playerIndex].actionState = 1;
        lbl_1_bss_2DC[playerIndex ^ 1].actionState = 0;
    }

    playerIndex = 0;
    while (playerIndex < 2) {
        M660Player *player = &lbl_1_bss_2DC[playerIndex];
        fn_1_601C(player, 6);
        if (lbl_1_bss_2DC[(s8)player->playerIndex ^ 1].actionState >= 2) {
            player->actionState = 0;
        }

        switch (player->actionState) {
        case 0:
            break;
        case 1:
        {
            Vec direction;
            Vec *directionPointer;

            fn_1_67D4(3, 5, 0);
            player->hitVoiceRequest = 1;
            lbl_1_bss_1F4.selectedSlot = player->actionSlot;
            fn_80086E3C(lbl_1_bss_1E8.players[playerIndex]);
            player->state = 0;
            player->collisionActive = 0;
            lbl_1_bss_544[playerIndex].shiftState = 1;
            lbl_1_bss_544[playerIndex].shiftMotion = 6;
            lbl_1_bss_544[playerIndex].shiftStart = 0.0f;
            lbl_1_bss_544[playerIndex].shiftEnd = 5.0f;
            lbl_1_bss_544[playerIndex].shiftFlags = ((1 << 30) | (1 << 0));
            lbl_1_bss_544[playerIndex].attributesToReset = 0;

            target = &lbl_1_bss_60C[lbl_1_bss_1F4.selectedSlot + 45];
            slot = &lbl_1_bss_80[lbl_1_bss_1F4.selectedSlot];
            offsetPosition.x = spawnPosition.x =
                target->position.x - player->position.x;
            offsetPosition.z = spawnPosition.z =
                target->position.z - player->position.z;
            distance = sqrt((f64)((spawnPosition.x * spawnPosition.x) +
                                  (spawnPosition.z * spawnPosition.z)));
            distance = distance == 0.0f ? 1.0f : distance;
            travel = spawnDistance[slot->effectIndex] /
                     distance;
            spawnPosition.x = target->position.x -
                              (spawnPosition.x * travel);
            spawnPosition.y = heights[slot->effectIndex];
            spawnPosition.z = target->position.z -
                              (spawnPosition.z * travel);
            lbl_1_bss_60C[playerIndex].attachment = spawnPosition;
            player->actionOrigin = player->position;

            travel = horizontalOffsets[slot->effectIndex] /
                          distance;
            offsetPosition.x = target->position.x -
                               (offsetPosition.x * travel);
            offsetPosition.y = heights[slot->effectIndex];
            offsetPosition.z = target->position.z -
                               (offsetPosition.z * travel);
            direction = offsetPosition;
            directionPointer = &direction;
            fn_1_4F64(directionPointer, slot->effectIndex);
            fn_1_55DC((s16)(lbl_1_bss_1F4.selectedSlot + 45),
                      slot->effectIndex);
            fn_8006A030(player->selectedPlayer, 24, 20, 0);
            player->actionState++;
            break;
        }
        case 2:
            if (lbl_1_bss_1F4.phase >= 2) {
                player->actionState++;
            } else {
                s16 remainingStepEstimate;
                f32 interpolationSteps;
                target =
                    &lbl_1_bss_60C[lbl_1_bss_1F4.selectedSlot + 45];
                slot =
                    &lbl_1_bss_80[lbl_1_bss_1F4.selectedSlot];
                interpolationSteps =
                    effectValues[slot->effectIndex];
                /* Use 30 interpolation steps for every effect. */
                interpolationSteps = 30.0f;
                remainingStepEstimate = fn_1_2E80(&player->position.x, interpolationSteps,
                                      player->actionOrigin.x,
                                      lbl_1_bss_60C[playerIndex].attachment.x);
                remainingStepEstimate += fn_1_2E80(&player->position.y, interpolationSteps,
                                       player->actionOrigin.y,
                                       lbl_1_bss_60C[playerIndex].attachment.y);
                remainingStepEstimate += fn_1_2E80(&player->position.z, interpolationSteps,
                                       player->actionOrigin.z,
                                       lbl_1_bss_60C[playerIndex].attachment.z);
            }
            break;
        case 3:
            break;
        }
        playerIndex++;
    }
}

/* Chooses paths and initializes the moving slots for a new effect set. */
s32 fn_1_7940(s16 effectIndex)
{
    Vec delta;
    s16 usedPaths[5];
    s32 result = -1;
    M660MovingSlotRange movingSlotRanges[3] = {{0, 4}, {5, 7}, {8, 8}};
    M660PathChoiceRange pathChoiceRanges[3] = {{13, 0}, {8, 13}, {7, 21}};
    s32 effectLayers[3] = {1, 1, 1};
    s16 entryIndex;
    s16 index;
    f32 extraAngle;

    for (index = 0; index < 5; index++) {
        usedPaths[index] = -1;
    }

    for (index = 14; index <= 17; index++) {
        fn_80026B8C(lbl_1_bss_60C[index].modelId,
                          (s16)effectLayers[effectIndex]);
    }

    {
    f32 level;
    index = movingSlotRanges[effectIndex].firstSlotIndex;
    level = 0.0f;
    entryIndex = 0;
    while (index <= movingSlotRanges[effectIndex].lastSlotIndex) {
        M660Slot *slot = &lbl_1_bss_80[index];

        if (slot->active == 0) {
            s16 choice;
            s16 side;
            s16 choiceTry;
            s16 usedIndex;
            f32 travelSpeed;
            M660Model *object;

            choiceTry = 0;
            do {
                choice = pathChoiceRanges[effectIndex].firstPathIndex +
                         frandmod(pathChoiceRanges[effectIndex].pathCount);
                for (usedIndex = 0; usedIndex < 5; usedIndex++) {
                    if (usedPaths[usedIndex] != -1 &&
                        (choice == usedPaths[usedIndex] ||
                         choice == usedPaths[usedIndex] + 1 ||
                         choice == usedPaths[usedIndex] - 1)) {
                        usedIndex = 99;
                        break;
                    }
                }
                if (usedIndex != 99) {
                    usedPaths[entryIndex] = choice;
                    break;
                }
            } while (choiceTry++ <= 100);

            side = frandmod(2);
            extraAngle = lbl_1_data_100[choice].angles[side];
            delta.x = lbl_1_data_100[choice].points[0].x - lbl_1_data_100[choice].points[1].x;
            delta.z = lbl_1_data_100[choice].points[0].z - lbl_1_data_100[choice].points[1].z;
            travelSpeed = sqrt((f64)((delta.x * delta.x) + (delta.z * delta.z)));
            travelSpeed /= lbl_1_bss_1F4.interval;

            slot->active = 1;
            slot->unk02 = 0;
            slot->delay = entryIndex == 0 ? 0 : frandmod(30);
            slot->pathIndex = choice;
            slot->effectIndex = effectIndex;
            slot->travelSpeed = travelSpeed +
                             fn_1_3C54(lbl_1_bss_1F4.speedRandomRange);
            slot->velocity.x = (f32)((f64)slot->travelSpeed *
                                    sin((M_PI * extraAngle) / 180.0));
            slot->velocity.z = (f32)((f64)slot->travelSpeed *
                                    cos((M_PI * extraAngle) / 180.0));
            slot->phase = fn_1_3C54(50.0f);

            object = &lbl_1_bss_60C[index + 45];
            object->position = lbl_1_data_100[choice].points[side];
            object->position.y = level;
            object->rotation.y = extraAngle;
            fn_80026A44(lbl_1_bss_60C[index + 45].modelId, 1U);

            lbl_1_bss_60C[slot->modelA].position = object->position;
            lbl_1_bss_60C[slot->modelA].rotation = object->rotation;
            fn_80026A44(lbl_1_bss_60C[slot->modelA].modelId, 1U);

            lbl_1_bss_60C[slot->modelB].position = object->position;
            lbl_1_bss_60C[slot->modelB].rotation = object->rotation;
            fn_80026A44(lbl_1_bss_60C[slot->modelB].modelId, 1U);
        }
        index++;
        level += 1.0f;
        entryIndex++;
    }
    }

    return result;
}

/* Returns whether an active moving object is still inside the stage bounds. */
s32 fn_1_8084(void)
{
    M660Slot *slot;
    s32 found = 0;
    s16 index = 0;
    M660Model *object;

    while (index < 9) {
        slot = &lbl_1_bss_80[index];
        object = &lbl_1_bss_60C[index + 45];
        if (slot->active != 0 &&
            object->position.x < 1000.0f + object->bounds.x &&
            object->position.x > -1000.0f - object->bounds.x &&
            object->position.z < 1000.0f + object->bounds.x &&
            object->position.z > -1000.0f - object->bounds.x) {
            found = 1;
        }
        index++;
    }

    return found;
}

/* Called by fn_1_7008 each gameplay tick to advance moving objects and stage effects. */
void fn_1_818C(void)
{
    Vec limits[3] = {{1700.0f, 0.0f, 1400.0f}, {1800.0f, 0.0f, 1500.0f}, {1900.0f, 0.0f, 1600.0f}};
    f32 phases[6][2] = { { 0.0f, 1.5f },   { 13.0f, -1.7f }, { 37.0f, -3.4f },
                         { 50.0f, -1.7f }, { 37.0f, 1.5f },  { 13.0f, 3.2f } };
    s32 layers[5] = {2, 3, 4, 5, 6};
    Vec interpolationDelta;
    s16 order[5] = {0, 1, 2, 3, 4};
    s16 index;
    s16 other;
    s16 anyActive;
    s16 nextEffect;
    s16 phaseIndex;
    s16 nextPhase;
    f32 phaseFrame;
    f32 scale;
    M660Slot *slot;
    M660Model *object;

    if (lbl_1_bss_1F4.soundPlayed == 0) {
        s16 found = 0;
        M660Slot *scanSlot;
        s16 scanIndex = 0;
        M660Model *scanObject;

        while (scanIndex < 9) {
            scanSlot = &lbl_1_bss_80[scanIndex];
            scanObject = &lbl_1_bss_60C[scanIndex + 45];
            if (scanSlot->active != 0 &&
                scanObject->position.x < 1000.0f + scanObject->bounds.x &&
                scanObject->position.x > -1000.0f - scanObject->bounds.x &&
                scanObject->position.z < 1000.0f + scanObject->bounds.x &&
                scanObject->position.z > -1000.0f - scanObject->bounds.x) {
                found = 1;
            }
            scanIndex++;
        }
        if (found != 0) {
            lbl_1_bss_1F4.soundPlayed = 1;
            fn_1_67D4((s16)lbl_1_bss_1F4.effectIndex, 5, 0);
        }
    }
    index = 0;
    anyActive = 0;
    while (index < 9) {
        slot = &lbl_1_bss_80[index];
        object = &lbl_1_bss_60C[index + 45];
        if (slot->active != 0) {
            if (slot->delay-- < 0) {
                if (lbl_1_bss_1F4.resultStarted != 0 && lbl_1_bss_1F4.selectedSlot != index &&
                    fn_8002D4E0(object->modelId) <= 0.0f) {
                    fn_80026A44(object->modelId, ((1 << 30) | (1 << 0)));
                    fn_8002D414(object->modelId, 1.0f);
                    fn_8002D86C(object->modelId, 2.0f);
                    fn_80026A44(lbl_1_bss_60C[slot->modelA].modelId, ((1 << 30) | (1 << 0)));
                    fn_8002D414(lbl_1_bss_60C[slot->modelA].modelId, 1.0f);
                    fn_8002D86C(lbl_1_bss_60C[slot->modelA].modelId, 2.0f);
                    fn_80026A44(lbl_1_bss_60C[slot->modelB].modelId, ((1 << 30) | (1 << 0)));
                    fn_8002D414(lbl_1_bss_60C[slot->modelB].modelId, 1.0f);
                    fn_8002D86C(lbl_1_bss_60C[slot->modelB].modelId, 2.0f);
                }
                if (lbl_1_bss_1F4.selectedSlot != index) {
                    object->position.x += slot->velocity.x;
                    object->position.z += slot->velocity.z;
                    lbl_1_bss_60C[slot->modelA].position = object->position;
                    lbl_1_bss_60C[slot->modelA].rotation = object->rotation;
                    lbl_1_bss_60C[slot->modelB].position = object->position;
                    scale = 12.5f;
                    if (slot->effectIndex == 1) scale = 14.0f;
                    lbl_1_bss_60C[slot->modelB].position.x +=
                        lbl_1_bss_60C[slot->modelB].position.x / scale;
                    lbl_1_bss_60C[slot->modelB].position.z +=
                        lbl_1_bss_60C[slot->modelB].position.z / scale;
                    lbl_1_bss_60C[slot->modelB].rotation = object->rotation;
                }
                if ((lbl_1_bss_1F4.resultStarted == 0 || lbl_1_bss_1F4.selectedSlot == index) &&
                    lbl_1_bss_1F4.phase <= 1) {
                    fn_1_42E0(object, slot->effectIndex);
                }
                phaseIndex = (s16)slot->phase;
                phaseFrame = phaseIndex % 10;
                phaseIndex /= 10;
                nextPhase = phaseIndex + 1 >= 6 ? 0 : phaseIndex + 1;
                interpolationDelta.y = (phases[nextPhase][0] - phases[phaseIndex][0]) / 10.0f;
                object->position.y = phases[phaseIndex][0];
                object->position.y += interpolationDelta.y * phaseFrame;
                if (slot->effectIndex == 1) object->position.y += 50.0f;
                else if (slot->effectIndex == 0) object->position.y += 250.0f;
                lbl_1_bss_60C[slot->modelA].position.y = object->position.y;
                interpolationDelta.y = (phases[nextPhase][1] - phases[phaseIndex][1]) / 10.0f;
                lbl_1_bss_60C[slot->modelA].rotation.x = phases[phaseIndex][1];
                lbl_1_bss_60C[slot->modelA].rotation.x += interpolationDelta.y * phaseFrame;
                if ((slot->phase += 1.0f) >= 59.0f) slot->phase = 0.0f;
                if (object->position.x > limits[slot->effectIndex].x ||
                    object->position.x < -limits[slot->effectIndex].x ||
                    object->position.z > limits[slot->effectIndex].z ||
                    object->position.z < -limits[slot->effectIndex].z) {
                    slot->active = 0;
                    fn_800269E8(object->modelId, 1);
                    fn_800269E8(lbl_1_bss_60C[slot->modelA].modelId, 1);
                    fn_800269E8(lbl_1_bss_60C[slot->modelB].modelId, 1);
                }
            }
            if (slot->active != 0) anyActive = 1;
        }
        index++;
    }
    if (lbl_1_bss_1F4.resultStarted == 0) {
        if (lbl_1_bss_1F4.count-- < 0 && anyActive == 0) {
            phaseIndex = 0;
            for (;;) {
                nextEffect = frandmod(3);
                if (lbl_1_bss_1F4.historyCount >= 2) {
                    if (lbl_1_bss_1F4.effectHistory[lbl_1_bss_1F4.historyCount - 1] != nextEffect ||
                        lbl_1_bss_1F4.effectHistory[lbl_1_bss_1F4.historyCount - 2] != nextEffect)
                        break;
                } else if (nextEffect != 2) break;
                if (phaseIndex++ > 100) break;
            }
            if ((lbl_1_bss_1F4.interval -= 12.0f) <= 240.0f) lbl_1_bss_1F4.interval = 240.0f;
            if ((lbl_1_bss_1F4.speedRandomRange += 1.0f) >= 5.0f)
                lbl_1_bss_1F4.speedRandomRange = 5.0f;
            fn_1_7940(nextEffect);
            fn_1_15484(nextEffect);
            fn_1_15484(4);
            fn_1_142F8(nextEffect);
            lbl_1_bss_1F4.effectIndex = nextEffect;
            lbl_1_bss_1F4.count = 240;
            lbl_1_bss_1F4.effectHistory[lbl_1_bss_1F4.historyCount] = nextEffect;
            if (++lbl_1_bss_1F4.historyCount >= 10) lbl_1_bss_1F4.historyCount = 0;
            lbl_1_bss_1F4.soundPlayed = 0;
        }
        if (lbl_1_bss_1F4.count == 220) {
            lbl_1_bss_2DC[0].external = 1;
            lbl_1_bss_2DC[1].external = 1;
        }
    }
    for (index = 0; index < 5; index++) {
        for (other = 0; other < 5; other++) {
            if (lbl_1_bss_60C[order[index] + 45].position.y <
                lbl_1_bss_60C[order[other] + 45].position.y) {
                nextEffect = order[index];
                order[index] = order[other];
                order[other] = nextEffect;
            }
        }
    }
    index = 0;
    phaseFrame = 0.0f;
    while (index < 5) {
        fn_80026B8C(lbl_1_bss_60C[order[index] + 45].modelId, layers[index]);
        lbl_1_bss_60C[order[index] + 45].position.y += phaseFrame;
        index++;
        phaseFrame += 0.1f;
    }
    for (index = 0; index < 5; index++) order[index] = index;
    layers[0] = 3;
    layers[1] = 4;
    layers[2] = 5;
    for (index = 0; index < 3; index++) {
        for (other = 0; other < 3; other++) {
            if (lbl_1_bss_60C[order[index] + 50].position.y <
                lbl_1_bss_60C[order[other] + 50].position.y) {
                nextEffect = order[index];
                order[index] = order[other];
                order[other] = nextEffect;
            }
        }
    }
    index = 0;
    phaseFrame = 0.0f;
    while (index < 3) {
        fn_80026B8C(lbl_1_bss_60C[order[index] + 50].modelId, layers[index]);
        lbl_1_bss_60C[order[index] + 50].position.y += phaseFrame;
        index++;
        phaseFrame += 0.1f;
    }
}

/* Updates the stage point light from its stored position, direction and attenuation. */
void fn_1_9050(void)
{
    M660SceneLight *light = &lbl_1_bss_38;
    fn_80028F14(light->modelId, light->lightSlot, &light->position, &light->direction);
    fn_80028C1C(light->modelId, light->lightSlot, light->referenceDistance,
        light->referenceBrightness, GX_DA_STEEP);
}

/* The scene collision system calls this narrow hook to pair players that contact each other. */
int fn_1_90A8(COL_NARROW_PARAM *first,
              COL_NARROW_PARAM *second)
{
    switch (second->type) {
    case 0:
        lbl_1_bss_2DC[first->paramB].partnerIndex =
            (s16)second->paramB;
        lbl_1_bss_2DC[second->paramB].partnerIndex =
            (s16)first->paramB;
        lbl_1_bss_2DC[first->paramB].collisionCount++;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    default:
        break;
    }

    return 0;
}

/* The scene collision system calls this correction hook to save the actor's current mesh. */
void fn_1_9128(MGACTOR *actor, int parameter)
{
    s32 playerIndex = actor->param;
    lbl_1_bss_2DC[playerIndex].storedMesh = actor->colMesh;
}

/* Called by gameplay and result callbacks to resolve stage contacts and start nearby actions. */
void fn_1_915C(void)
{
    f32 sides[4][4];
    Vec delta;
    Vec movement;
    Vec projected;
    Vec normalized;
    s16 edgeIndex;
    s16 objectIndex;
    s16 contactCount;
    s16 playerIndex;
    s16 collided;
    f32 normalAngle;
    f32 distance;
    f32 collisionAngle;
    f32 movementAngle;
    f32 pushScale;
    f32 edgeSquared;
    f32 factor;
    f32 finalDistance;
    f32 numerator;
    M660Player *player;
    M660Model *object;

    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        player = &lbl_1_bss_2DC[playerIndex];
        if (player->collisionActive != 0) {
            player->oldContacts[0] = player->contacts[0];
            player->oldContacts[1] = player->contacts[1];
            player->oldContactEdges[0] = player->contactEdges[0];
            player->oldContactEdges[1] = player->contactEdges[1];
            player->contacts[1] = player->contacts[0] = -1;
            player->contactEdges[1] = player->contactEdges[0] = -1;
            contactCount = 0;
            for (objectIndex = 31; objectIndex < 45; objectIndex++) {
                object = &lbl_1_bss_60C[objectIndex];
                if (0.0f != object->position.x) {
                    sides[0][0] = object->position.x + object->bounds.x;
                    sides[0][1] = object->position.x - object->bounds.x;
                    sides[0][2] = object->position.z + object->bounds.z;
                    sides[0][3] = object->position.z + object->bounds.z;
                    sides[1][0] = object->position.x + object->bounds.x;
                    sides[1][1] = object->position.x - object->bounds.x;
                    sides[1][2] = object->position.z - object->bounds.z;
                    sides[1][3] = object->position.z - object->bounds.z;
                    sides[2][0] = object->position.x + object->bounds.x;
                    sides[2][1] = object->position.x + object->bounds.x;
                    sides[2][2] = object->position.z + object->bounds.z;
                    sides[2][3] = object->position.z - object->bounds.z;
                    sides[3][0] = object->position.x - object->bounds.x;
                    sides[3][1] = object->position.x - object->bounds.x;
                    sides[3][2] = object->position.z + object->bounds.z;
                    sides[3][3] = object->position.z - object->bounds.z;
                    if (49.0f + sides[0][0] > player->position.x &&
                        sides[0][1] - 49.0f < player->position.x &&
                        49.0f + sides[2][2] > player->position.z &&
                        sides[2][3] - 49.0f < player->position.z) {
                        edgeIndex = 0;
                        collided = 0;
                        while (edgeIndex < 4) {
                            if (lbl_1_bss_0[objectIndex - 31][edgeIndex] != -1) {
                                delta.x = sides[edgeIndex][0] - sides[edgeIndex][1];
                                delta.z = sides[edgeIndex][2] - sides[edgeIndex][3];
                                edgeSquared = delta.x * delta.x + delta.z * delta.z;
                                numerator =
                                    -1.0f * (delta.x * (sides[edgeIndex][0] - player->position.x) +
                                             delta.z * (sides[edgeIndex][2] - player->position.z));
                                factor = edgeSquared ? numerator / edgeSquared : 0.0f;
                                projected.x = delta.x * factor + sides[edgeIndex][0];
                                projected.z = delta.z * factor + sides[edgeIndex][2];
                                delta.x = player->position.x - projected.x;
                                delta.y = 0.0f;
                                delta.z = player->position.z - projected.z;
                                distance = sqrt(delta.x * delta.x + delta.z * delta.z);
                                if (delta.x == 0.0f && delta.z == 0.0f) {
                                    collisionAngle = 999.0f;
                                } else {
                                    VECNormalize(&delta, &normalized);
                                    collisionAngle = 180.0 * (atan2(normalized.x, normalized.z) /
                                                              M_PI);
                                    normalAngle = 180.0 * (atan2(object->normals[edgeIndex].x,
                                        object->normals[edgeIndex].z) / M_PI);
                                    if (normalAngle == 180.0f && collisionAngle < 0.0f)
                                        normalAngle = -180.0f;
                                    collisionAngle = collisionAngle - normalAngle > 0.0f
                                                         ? collisionAngle - normalAngle
                                                         : -1.0f * (collisionAngle - normalAngle);
                                }
                                movement.x = player->previousPosition.x - player->position.x;
                                movement.y = 0.0f;
                                movement.z = player->previousPosition.z - player->position.z;
                                if (movement.x == 0.0f && movement.z == 0.0f) {
                                    movementAngle = 999.0f;
                                } else {
                                    VECNormalize(&movement, &normalized);
                                    movementAngle = 180.0 * (atan2(normalized.x, normalized.z) /
                                                             M_PI);
                                    normalAngle = 180.0 * (atan2(object->normals[edgeIndex].x,
                                        object->normals[edgeIndex].z) / M_PI);
                                    if (normalAngle == 180.0f && movementAngle < 0.0f)
                                        normalAngle = -180.0f;
                                    movementAngle = movementAngle - normalAngle > 0.0f
                                                        ? movementAngle - normalAngle
                                                        : -1.0f * (movementAngle - normalAngle);
                                }
                                if (distance < 50.0f && collisionAngle < 10.0f &&
                                    movementAngle < 90.0f) {
                                    if (contactCount < 2) {
                                        player->contacts[contactCount] = objectIndex;
                                        player->contactEdges[contactCount] = edgeIndex;
                                        /* Index the copied normal by contact ordinal. */
                                        player->unkF8[contactCount] =
                                            object->normals[contactCount];
                                        contactCount++;
                                    }
                                    pushScale = distance ? 50.0f / distance : 0.0f;
                                    if (edgeIndex == 0) {
                                        delta.z = delta.z < 0.0f ? -1.0f * delta.z : delta.z;
                                    } else if (edgeIndex == 1) {
                                        delta.z = delta.z > 0.0f ? -1.0f * delta.z : delta.z;
                                    } else if (edgeIndex == 2) {
                                        delta.x = delta.x < 0.0f ? -1.0f * delta.x : delta.x;
                                    } else if (edgeIndex == 3) {
                                        delta.x = delta.x > 0.0f ? -1.0f * delta.x : delta.x;
                                    }
                                    delta.x *= pushScale;
                                    delta.z *= pushScale;
                                    player->position.x = projected.x + delta.x;
                                    player->position.z = projected.z + delta.z;
                                    collided = 1;
                                }
                                if (collided != 0) break;
                            }
                            edgeIndex++;
                        }
                    }
                }
            }
            sides[0][0] = -631.0f;
            sides[0][1] = -831.0f;
            sides[0][2] = -831.0f;
            sides[0][3] = -631.0f;
            sides[1][0] = 631.0f;
            sides[1][1] = 831.0f;
            sides[1][2] = -831.0f;
            sides[1][3] = -631.0f;
            sides[2][0] = -831.0f;
            sides[2][1] = -631.0f;
            sides[2][2] = 631.0f;
            sides[2][3] = 831.0f;
            sides[3][0] = 831.0f;
            sides[3][1] = 631.0f;
            sides[3][2] = 631.0f;
            sides[3][3] = 831.0f;
            for (edgeIndex = 0; edgeIndex < 4; edgeIndex++) {
                delta.x = sides[edgeIndex][0] - sides[edgeIndex][1];
                delta.z = sides[edgeIndex][2] - sides[edgeIndex][3];
                edgeSquared = delta.x * delta.x + delta.z * delta.z;
                numerator = -1.0f * (delta.x * (sides[edgeIndex][0] - player->position.x) +
                    delta.z * (sides[edgeIndex][2] - player->position.z));
                factor = edgeSquared ? numerator / edgeSquared : 0.0f;
                projected.x = delta.x * factor + sides[edgeIndex][0];
                projected.z = delta.z * factor + sides[edgeIndex][2];
                delta.x = player->position.x - projected.x;
                delta.y = 0.0f;
                delta.z = player->position.z - projected.z;
                distance = sqrt(delta.x * delta.x + delta.z * delta.z);
                if (distance < 50.0f) {
                    pushScale = distance ? 50.0f / distance : 0.0f;
                    delta.x *= pushScale;
                    delta.z *= pushScale;
                    player->position.x = projected.x + delta.x;
                    player->position.z = projected.z + delta.z;
                }
            }
            if (player->position.x > 780.0f) player->position.x = 780.0f;
            if (player->position.x < -780.0f) player->position.x = -780.0f;
            if (player->position.z > 780.0f) player->position.z = 780.0f;
            if (player->position.z < -780.0f) player->position.z = -780.0f;
            for (objectIndex = 63; objectIndex <= 71; objectIndex++) {
                if (lbl_1_bss_80[objectIndex - 63].active != 0) {
                    object = &lbl_1_bss_60C[objectIndex];
                    delta.x = player->position.x - object->position.x;
                    delta.z = player->position.z - object->position.z;
                    finalDistance = sqrt(delta.x * delta.x + delta.z * delta.z);
                    if (finalDistance < object->bounds.x && player->actionState == 0) {
                        player->actionState = 1;
                        player->actionSlot = objectIndex - 63;
                    }
                }
            }
        }
    }
}

M660PathSegment lbl_1_data_100[28] = {
    {{{-485.4f, 0.0f, -1400.0f}, {-814.8f, 0.0f, 1400.0f}}, {353.3f, 173.3f}},
    {{{-383.9f, 0.0f, -1400.0f}, {-216.3f, 0.0f, 1400.0f}}, {3.4f, -176.6f}},
    {{{-214.6f, 0.0f, -1400.0f}, {114.4f, 0.0f, 1400.0f}}, {6.7f, -173.3f}},
    {{{-332.4f, 0.0f, -1400.0f}, {1700.0f, 0.0f, 921.2f}}, {41.2f, -138.8f}},
    {{{187.4f, 0.0f, -1400.0f}, {1012.0f, 0.0f, 1400.0f}}, {16.4f, -163.6f}},
    {{{1700.0f, 0.0f, -1100.6f}, {-1700.0f, 0.0f, -99.6f}}, {286.4f, 106.4f}},
    {{{1214.9f, 0.0f, -1400.0f}, {13.7f, 0.0f, 1400.0f}}, {336.8f, 156.8f}},
    {{{1700.0f, 0.0f, -450.3f}, {-1700.0f, 0.0f, -850.0f}}, {263.3f, 83.3f}},
    {{{1700.0f, 0.0f, -499.6f}, {-1700.0f, 0.0f, 99.4f}}, {280.0f, 100.0f}},
    {{{1700.0f, 0.0f, 501.5f}, {-1700.0f, 0.0f, -901.7f}}, {247.6f, 67.6f}},
    {{{1700.0f, 0.0f, 449.7f}, {-1700.0f, 0.0f, 50.0f}}, {263.3f, 83.3f}},
    {{{1700.0f, 0.0f, 353.6f}, {-1700.0f, 0.0f, 1138.3f}}, {283.0f, 103.0f}},
    {{{1700.0f, 0.0f, 1263.3f}, {-1700.0f, 0.0f, 44.1f}}, {250.3f, 70.3f}},
    {{{-826.4f, 0.0f, -1500.0f}, {-473.8f, 0.0f, 1500.0f}}, {6.7f, -173.3f}},
    {{{-743.1f, 0.0f, -1500.0f}, {1800.0f, 0.0f, 1293.0f}}, {42.3f, -137.7f}},
    {{{301.4f, 0.0f, -1500.0f}, {-401.7f, 0.0f, 1500.0f}}, {346.8f, 166.8f}},
    {{{410.1f, 0.0f, -1500.0f}, {589.7f, 0.0f, 1500.0f}}, {3.4f, -176.6f}},
    {{{1800.0f, 0.0f, -238.5f}, {-1800.0f, 0.0f, -661.8f}}, {263.3f, 83.3f}},
    {{{1800.0f, 0.0f, 161.5f}, {-1800.0f, 0.0f, -261.8f}}, {263.3f, 83.3f}},
    {{{1800.0f, 0.0f, 771.7f}, {-1800.0f, 0.0f, -72.2f}}, {256.8f, 76.8f}},
    {{{949.8f, 0.0f, 1500.0f}, {-1800.0f, 0.0f, -1480.7f}}, {222.7f, 42.7f}},
    {{{-338.0f, 0.0f, -1600.0f}, {37.8f, 0.0f, 1600.0f}}, {6.7f, -173.3f}},
    {{{651.0f, 0.0f, -1600.0f}, {-1485.0f, 0.0f, 1600.0f}}, {326.3f, 146.3f}},
    {{{350.2f, 0.0f, -1600.0f}, {349.5f, 0.0f, 1600.0f}}, {360.0f, 180.0f}},
    {{{1900.0f, 0.0f, -859.3f}, {-1900.0f, 0.0f, 259.1f}}, {286.4f, 106.4f}},
    {{{1900.0f, 0.0f, 383.8f}, {-1900.0f, 0.0f, -1184.0f}}, {247.6f, 67.6f}},
    {{{1900.0f, 0.0f, -34.9f}, {-1900.0f, 0.0f, 634.6f}}, {280.0f, 100.0f}},
    {{{1900.0f, 0.0f, 1290.3f}, {-1900.0f, 0.0f, -1390.6f}}, {234.8f, 54.8f}}
};
