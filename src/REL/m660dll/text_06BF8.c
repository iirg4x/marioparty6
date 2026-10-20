/* Runs the stage's gameplay sequence, player actions, moving objects and collision updates. */
#include "REL/m660dll/camera_lighting.h"
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

typedef struct M660IndexRange_7940 {
    s16 first;
    s16 last;
} M660IndexRange_7940;

typedef struct M660Ranges_7940 {
    M660IndexRange_7940 values[3];
} M660Ranges_7940;

typedef struct M660ThreeWords_7940 {
    s32 values[3];
} M660ThreeWords_7940;

typedef struct M660ActionValues_71F0 {
    f32 values[3];
} M660ActionValues_71F0;

typedef struct M660Origins { Vec values[2]; } M660Origins;
extern const M660ActionValues_71F0 lbl_1_rodata_348;
extern const M660ActionValues_71F0 lbl_1_rodata_354;
extern const M660ActionValues_71F0 lbl_1_rodata_360;
extern const M660ActionValues_71F0 lbl_1_rodata_36C;
extern s16 lbl_1_bss_2680[];
void fn_1_3014(MGTIMER *timer);
s32 fn_1_3040(MGTIMER *timer);
void fn_1_3068(MGTIMER *timer);
void fn_1_7008(void);
void fn_1_2B34(void);
void fn_1_24C4(void);
s32 fn_1_7080(void);
void fn_1_71C(void);
void fn_1_11F50(void);
void fn_1_71F0(void);
void fn_1_818C(void);
void fn_1_9050(void);
void fn_1_915C(void);
void fn_1_4004(Vec *origin);
f32 fn_1_3C54(f32 range);
struct VoicePosition;
void fn_1_67D4(s16 slot, s16 mode, struct VoicePosition *position);
void fn_1_601C(M660Player *player, s16 mode);
void fn_1_55DC(s16 modelIndex, s16 effectIndex);
s16 fn_1_2E80(f32 *value, f32 step, f32 start, f32 end);
void fn_1_4F64(Vec *direction, s16 playerIndex);
void fn_1_42E0(M660Model *object, s16 effectIndex);
s32 fn_1_7940(s16 effectIndex);
void fn_1_15484(s16 effectIndex);
void fn_1_142F8(s16 effectIndex);

/* Updates the pan of an active stage sound when its effect handle is playing. */
void fn_1_6BF8(s16 soundIndex, s16 pan)
{
    if (lbl_1_bss_1F4.soundStatus[soundIndex] == 1 &&
        HuAudFXStatusGet(lbl_1_bss_1F4.soundEffectId[soundIndex]) == 2) {
        HuAudFXPanning(lbl_1_bss_1F4.soundEffectId[soundIndex], pan);
    }
}

/* Updates the pitch of an active stage sound when its effect handle is playing. */
void fn_1_6C88(s16 soundIndex, s16 pitch)
{
    if (lbl_1_bss_1F4.soundStatus[soundIndex] == 1 &&
        HuAudFXStatusGet(lbl_1_bss_1F4.soundEffectId[soundIndex]) == 2) {
        HuAudFXPitchSet(lbl_1_bss_1F4.soundEffectId[soundIndex], pitch);
    }
}

/* Updates the volume of an active stage sound when its effect handle is playing. */
void fn_1_6D18(s16 soundIndex, s16 volume)
{
    if (lbl_1_bss_1F4.soundStatus[soundIndex] == 1 &&
        HuAudFXStatusGet(lbl_1_bss_1F4.soundEffectId[soundIndex]) == 2) {
        HuAudFXVolSet(lbl_1_bss_1F4.soundEffectId[soundIndex], volume);
    }
}

/* When its effect is playing or paused, marks the slot unused, then stops the effect. */
void fn_1_6DA8(s16 soundSlot) {
    if ((lbl_1_bss_1F4.soundStatus[soundSlot] == 1) &&
        ((HuAudFXStatusGet(lbl_1_bss_1F4.soundEffectId[soundSlot]) == 2) ||
         (HuAudFXStatusGet(lbl_1_bss_1F4.soundEffectId[soundSlot]) == 3))) {
        lbl_1_bss_1F4.soundStatus[soundSlot] = -1;
        HuAudFXStop(lbl_1_bss_1F4.soundEffectId[soundSlot]);
    }
}

void fn_1_6E70(s16 soundSlot)
{
    fn_1_6DA8(soundSlot);
}

void fn_1_6F38(void) {
    HuAudAllStop();
}

/* Runs one gameplay tick, initializing player collision state on the first tick. */
s16 fn_1_6F58(void)
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
    M660Origins parameters = {{{-788.0f, 60.0f, -788.0f}, {788.0f, 60.0f, -788.0f}}};

    fn_1_71C();
    fn_1_11F50();
    fn_1_71F0();
    fn_1_818C();
    fn_1_9050();
    fn_1_4004(parameters.values);
    fn_1_4004(&parameters.values[1]);
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
    M660ActionValues_71F0 spawnDistance = {{20.0f, 150.0f, 300.0f}};
    M660ActionValues_71F0 verticalOffsets = {{100.0f, 270.0f, 480.0f}};
    M660ActionValues_71F0 movementSpeeds = {{60.0f, 120.0f, 200.0f}};
    M660ActionValues_71F0 heights = {{230.0f, 230.0f, 330.0f}};
    s16 playerIndex;
    M660Model *target;
    M660Slot *boundPlayer;
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
            player->setupState = 1;
            lbl_1_bss_1F4.selectedSlot = player->actionSlot;
            MgPlayerDespawn(lbl_1_bss_1E8.players[playerIndex]);
            player->state = 0;
            player->collisionActive = 0;
            lbl_1_bss_544[playerIndex].shiftState = 1;
            lbl_1_bss_544[playerIndex].shiftMotion = 6;
            lbl_1_bss_544[playerIndex].shiftStart = 0.0f;
            lbl_1_bss_544[playerIndex].shiftEnd = 5.0f;
            lbl_1_bss_544[playerIndex].shiftFlags = HU3D_MOTATTR_LOOP;
            lbl_1_bss_544[playerIndex].attributesToReset = 0;

            target = &lbl_1_bss_60C[lbl_1_bss_1F4.selectedSlot + 45];
            boundPlayer = &lbl_1_bss_80[lbl_1_bss_1F4.selectedSlot];
            offsetPosition.x = spawnPosition.x =
                target->position.x - player->position.x;
            offsetPosition.z = spawnPosition.z =
                target->position.z - player->position.z;
            distance = sqrt((f64)((spawnPosition.x * spawnPosition.x) +
                                  (spawnPosition.z * spawnPosition.z)));
            distance = distance == 0.0f ? 1.0f : distance;
            travel = spawnDistance.values[boundPlayer->effectIndex] /
                     distance;
            spawnPosition.x = target->position.x -
                              (spawnPosition.x * travel);
            spawnPosition.y = heights.values[boundPlayer->effectIndex];
            spawnPosition.z = target->position.z -
                              (spawnPosition.z * travel);
            lbl_1_bss_60C[playerIndex].attachment = spawnPosition;
            player->actionOrigin = player->position;

            travel = verticalOffsets.values[boundPlayer->effectIndex] /
                          distance;
            offsetPosition.x = target->position.x -
                               (offsetPosition.x * travel);
            offsetPosition.y = heights.values[boundPlayer->effectIndex];
            offsetPosition.z = target->position.z -
                               (offsetPosition.z * travel);
            direction = offsetPosition;
            directionPointer = &direction;
            fn_1_4F64(directionPointer, boundPlayer->effectIndex);
            fn_1_55DC((s16)(lbl_1_bss_1F4.selectedSlot + 45),
                      boundPlayer->effectIndex);
            omVibrate(player->selectedPlayer, 24, 20, 0);
            player->actionState++;
            break;
        }
        case 2:
            if (lbl_1_bss_1F4.phase >= 2) {
                player->actionState++;
            } else {
                s16 completed;
                f32 movementSpeed;
                target =
                    &lbl_1_bss_60C[lbl_1_bss_1F4.selectedSlot + 45];
                boundPlayer =
                    &lbl_1_bss_80[lbl_1_bss_1F4.selectedSlot];
                movementSpeed =
                    movementSpeeds.values[boundPlayer->effectIndex];
                /* The effect-specific value above is overwritten; the helper receives a fixed step
                 * count of 30. */
                movementSpeed = 30.0f;
                completed = fn_1_2E80(&player->position.x, movementSpeed,
                                      player->actionOrigin.x,
                                      lbl_1_bss_60C[playerIndex].attachment.x);
                completed += fn_1_2E80(&player->position.y, movementSpeed,
                                       player->actionOrigin.y,
                                       lbl_1_bss_60C[playerIndex].attachment.y);
                completed += fn_1_2E80(&player->position.z, movementSpeed,
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

/* Called by fn_1_818C when a new effect set starts; selects paths and initializes its objects. */
s32 fn_1_7940(s16 effectIndex)
{
    Vec delta;
    s16 usedSlots[5];
    s32 result = -1;
    M660Ranges_7940 objectRanges = {{{0, 4}, {5, 7}, {8, 8}}};
    M660Ranges_7940 slotRanges = {{{13, 0}, {8, 13}, {7, 21}}};
    M660ThreeWords_7940 layers = {{1, 1, 1}};
    s16 entryIndex;
    s16 playerIndex;
    f32 extraAngle;

    for (playerIndex = 0; playerIndex < 5; playerIndex++) {
        usedSlots[playerIndex] = -1;
    }

    for (playerIndex = 14; playerIndex <= 17; playerIndex++) {
        Hu3DModelLayerSet(lbl_1_bss_60C[playerIndex].modelId,
                          (s16)layers.values[effectIndex]);
    }

    {
    f32 level;
    playerIndex = objectRanges.values[effectIndex].first;
    level = 0.0f;
    entryIndex = 0;
    while (playerIndex <= objectRanges.values[effectIndex].last) {
        M660Slot *slot = &lbl_1_bss_80[playerIndex];

        if (slot->active == 0) {
            s16 choice;
            s16 side;
            s16 choiceTry;
            s16 usedIndex;
            f32 travelSpeed;
            M660Model *object;

            choiceTry = 0;
            do {
                choice = slotRanges.values[effectIndex].last +
                         frandmod(slotRanges.values[effectIndex].first);
                for (usedIndex = 0; usedIndex < 5; usedIndex++) {
                    if (usedSlots[usedIndex] != -1 &&
                        (choice == usedSlots[usedIndex] ||
                         choice == usedSlots[usedIndex] + 1 ||
                         choice == usedSlots[usedIndex] - 1)) {
                        usedIndex = 99;
                        break;
                    }
                }
                if (usedIndex != 99) {
                    usedSlots[entryIndex] = choice;
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
            slot->state = 0;
            slot->delay = entryIndex == 0 ? 0 : frandmod(30);
            slot->pathIndex = choice;
            slot->effectIndex = effectIndex;
            slot->travelSpeed = travelSpeed +
                             fn_1_3C54(lbl_1_bss_1F4.speedRandomRange);
            slot->velocity.x = (f32)((f64)slot->travelSpeed *
                                    sin((3.141592653589793 * extraAngle) / 180.0));
            slot->velocity.z = (f32)((f64)slot->travelSpeed *
                                    cos((3.141592653589793 * extraAngle) / 180.0));
            slot->phase = fn_1_3C54(50.0f);

            object = &lbl_1_bss_60C[playerIndex + 45];
            object->position = lbl_1_data_100[choice].points[side];
            object->position.y = level;
            object->rotation.y = extraAngle;
            Hu3DModelAttrReset(lbl_1_bss_60C[playerIndex + 45].modelId, 1U);

            lbl_1_bss_60C[slot->modelA].position = object->position;
            lbl_1_bss_60C[slot->modelA].rotation = object->rotation;
            Hu3DModelAttrReset(lbl_1_bss_60C[slot->modelA].modelId, 1U);

            lbl_1_bss_60C[slot->modelB].position = object->position;
            lbl_1_bss_60C[slot->modelB].rotation = object->rotation;
            Hu3DModelAttrReset(lbl_1_bss_60C[slot->modelB].modelId, 1U);
        }
        playerIndex++;
        level += 1.0f;
        entryIndex++;
    }
    }

    return result;
}

/* Returns whether an active moving object is still inside the stage bounds. */
s32 fn_1_8084(void)
{
    M660Slot *player;
    s32 found = 0;
    s16 index = 0;
    M660Model *object;

    while (index < 9) {
        player = &lbl_1_bss_80[index];
        object = &lbl_1_bss_60C[index + 45];
        if (player->active != 0 &&
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
        M660Slot *player;
        s16 scanIndex = 0;
        M660Model *scanObject;

        while (scanIndex < 9) {
            player = &lbl_1_bss_80[scanIndex];
            scanObject = &lbl_1_bss_60C[scanIndex + 45];
            if (player->active != 0 &&
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
                    Hu3DMotionTimeGet(object->modelId) <= 0.0f) {
                    Hu3DModelAttrReset(object->modelId, HU3D_MOTATTR_LOOP);
                    Hu3DMotionTimeSet(object->modelId, 1.0f);
                    Hu3DMotionSpeedSet(object->modelId, 2.0f);
                    Hu3DModelAttrReset(lbl_1_bss_60C[slot->modelA].modelId, HU3D_MOTATTR_LOOP);
                    Hu3DMotionTimeSet(lbl_1_bss_60C[slot->modelA].modelId, 1.0f);
                    Hu3DMotionSpeedSet(lbl_1_bss_60C[slot->modelA].modelId, 2.0f);
                    Hu3DModelAttrReset(lbl_1_bss_60C[slot->modelB].modelId, HU3D_MOTATTR_LOOP);
                    Hu3DMotionTimeSet(lbl_1_bss_60C[slot->modelB].modelId, 1.0f);
                    Hu3DMotionSpeedSet(lbl_1_bss_60C[slot->modelB].modelId, 2.0f);
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
                    Hu3DModelAttrSet(object->modelId, 1);
                    Hu3DModelAttrSet(lbl_1_bss_60C[slot->modelA].modelId, 1);
                    Hu3DModelAttrSet(lbl_1_bss_60C[slot->modelB].modelId, 1);
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
                    if (lbl_1_bss_1F4.slotHistory[lbl_1_bss_1F4.historyCount - 1] != nextEffect ||
                        lbl_1_bss_1F4.slotHistory[lbl_1_bss_1F4.historyCount - 2] != nextEffect)
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
            lbl_1_bss_1F4.slotHistory[lbl_1_bss_1F4.historyCount] = nextEffect;
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
        Hu3DModelLayerSet(lbl_1_bss_60C[order[index] + 45].modelId, layers[index]);
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
        Hu3DModelLayerSet(lbl_1_bss_60C[order[index] + 50].modelId, layers[index]);
        lbl_1_bss_60C[order[index] + 50].position.y += phaseFrame;
        index++;
        phaseFrame += 0.1f;
    }
}

/* Updates the stage point light from its stored position, direction and attenuation. */
void fn_1_9050(void)
{
    M660SceneLight *light = &lbl_1_bss_38;
    Hu3DLLightPosSetV(light->modelId, light->lightSlot, &light->position, &light->direction);
    Hu3DLLightPointSet(light->modelId, light->lightSlot, light->referenceDistance,
        light->referenceBrightness, GX_DA_STEEP);
}

/* The scene collision system calls this narrow hook to pair players that contact each other. */
s32 fn_1_90A8(COL_NARROW_PARAM *first,
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
                            if (lbl_1_bss_0[objectIndex - 31].directions[edgeIndex] != -1) {
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
                                    PSVECNormalize(&delta, &normalized);
                                    collisionAngle = 180.0 * (atan2(normalized.x, normalized.z) /
                                                              3.141592653589793);
                                    normalAngle = 180.0 * (atan2(object->normals[edgeIndex].x,
                                        object->normals[edgeIndex].z) / 3.141592653589793);
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
                                    PSVECNormalize(&movement, &normalized);
                                    movementAngle = 180.0 * (atan2(normalized.x, normalized.z) /
                                                             3.141592653589793);
                                    normalAngle = 180.0 * (atan2(object->normals[edgeIndex].x,
                                        object->normals[edgeIndex].z) / 3.141592653589793);
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
                                        /* The saved normal uses the contact ordinal, not the
                                         * colliding edge index. */
                                        player->contactNormals[contactCount] =
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
    {{{-485.399994f, 0.0f, -1400.0f}, {-814.799988f, 0.0f, 1400.0f}}, {353.299988f, 173.300003f}},
    {{{-383.899994f, 0.0f, -1400.0f}, {-216.300003f, 0.0f, 1400.0f}}, {3.4000001f, -176.600006f}},
    {{{-214.600006f, 0.0f, -1400.0f}, {114.400002f, 0.0f, 1400.0f}}, {6.69999981f, -173.300003f}},
    {{{-332.399994f, 0.0f, -1400.0f}, {1700.0f, 0.0f, 921.200012f}}, {41.2000008f, -138.800003f}},
    {{{187.399994f, 0.0f, -1400.0f}, {1012.0f, 0.0f, 1400.0f}}, {16.3999996f, -163.600006f}},
    {{{1700.0f, 0.0f, -1100.59998f}, {-1700.0f, 0.0f, -99.5999985f}}, {286.399994f, 106.400002f}},
    {{{1214.90002f, 0.0f, -1400.0f}, {13.6999998f, 0.0f, 1400.0f}}, {336.799988f, 156.800003f}},
    {{{1700.0f, 0.0f, -450.299988f}, {-1700.0f, 0.0f, -850.0f}}, {263.299988f, 83.3000031f}},
    {{{1700.0f, 0.0f, -499.600006f}, {-1700.0f, 0.0f, 99.4000015f}}, {280.0f, 100.0f}},
    {{{1700.0f, 0.0f, 501.5f}, {-1700.0f, 0.0f, -901.700012f}}, {247.600006f, 67.5999985f}},
    {{{1700.0f, 0.0f, 449.700012f}, {-1700.0f, 0.0f, 50.0f}}, {263.299988f, 83.3000031f}},
    {{{1700.0f, 0.0f, 353.600006f}, {-1700.0f, 0.0f, 1138.30005f}}, {283.0f, 103.0f}},
    {{{1700.0f, 0.0f, 1263.30005f}, {-1700.0f, 0.0f, 44.0999985f}}, {250.300003f, 70.3000031f}},
    {{{-826.400024f, 0.0f, -1500.0f}, {-473.799988f, 0.0f, 1500.0f}}, {6.69999981f, -173.300003f}},
    {{{-743.099976f, 0.0f, -1500.0f}, {1800.0f, 0.0f, 1293.0f}}, {42.2999992f, -137.699997f}},
    {{{301.399994f, 0.0f, -1500.0f}, {-401.700012f, 0.0f, 1500.0f}}, {346.799988f, 166.800003f}},
    {{{410.100006f, 0.0f, -1500.0f}, {589.700012f, 0.0f, 1500.0f}}, {3.4000001f, -176.600006f}},
    {{{1800.0f, 0.0f, -238.5f}, {-1800.0f, 0.0f, -661.799988f}}, {263.299988f, 83.3000031f}},
    {{{1800.0f, 0.0f, 161.5f}, {-1800.0f, 0.0f, -261.799988f}}, {263.299988f, 83.3000031f}},
    {{{1800.0f, 0.0f, 771.700012f}, {-1800.0f, 0.0f, -72.1999969f}}, {256.799988f, 76.8000031f}},
    {{{949.799988f, 0.0f, 1500.0f}, {-1800.0f, 0.0f, -1480.69995f}}, {222.699997f, 42.7000008f}},
    {{{-338.0f, 0.0f, -1600.0f}, {37.7999992f, 0.0f, 1600.0f}}, {6.69999981f, -173.300003f}},
    {{{651.0f, 0.0f, -1600.0f}, {-1485.0f, 0.0f, 1600.0f}}, {326.299988f, 146.300003f}},
    {{{350.200012f, 0.0f, -1600.0f}, {349.5f, 0.0f, 1600.0f}}, {360.0f, 180.0f}},
    {{{1900.0f, 0.0f, -859.299988f}, {-1900.0f, 0.0f, 259.100006f}}, {286.399994f, 106.400002f}},
    {{{1900.0f, 0.0f, 383.799988f}, {-1900.0f, 0.0f, -1184.0f}}, {247.600006f, 67.5999985f}},
    {{{1900.0f, 0.0f, -34.9000015f}, {-1900.0f, 0.0f, 634.599976f}}, {280.0f, 100.0f}},
    {{{1900.0f, 0.0f, 1290.30005f}, {-1900.0f, 0.0f, -1390.59998f}}, {234.800003f, 54.7999992f}}
};
