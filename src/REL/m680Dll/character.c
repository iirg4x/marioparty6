/* Strawberry Shortfuse: character setup, movement and timed animations. */
#include "REL/m680Dll/character.h"
#include "datadir_enum.h"
#include "REL/m680Dll/scheduler.h"
#include "game/board/player.h"
#include "game/charman.h"
#include "game/hu3d.h"

#define M680_VOICE_OFF_MOTION 36

void fn_1_6538(M680CharacterState *work, s32 player);
void fn_1_9600(void);
void fn_1_AFF8(OMOBJ *object);
void fn_1_B0A4(M680CharacterState *work);
void fn_1_B0A8(M680CharacterState *work);
void fn_1_B0AC(M680CharacterState *work);
s32 fn_1_BB38(void *argument);
s32 fn_1_BD1C(void *argument);

/* Sorts ranking records before the player objects are created. */
void fn_1_AB70(s32 count, M680PlayerRankEntry *entries)
{
    M680PlayerRankEntry *heap;
    M680PlayerRankEntry held;
    s32 lastParent;
    s32 end;
    s32 parent;
    s32 child;

    heap = entries - 1;
    lastParent = count / 2;
    while (lastParent >= 1) {
        parent = lastParent;
        held = heap[lastParent];
        child = parent * 2;
        while (child <= count) {
            if (child < count && heap[child].rank > heap[child + 1].rank) {
                child++;
            }
            if (held.rank > heap[child].rank) {
                heap[parent] = heap[child];
                parent = child;
                child = parent * 2;
            } else {
                break;
            }
        }
        heap[parent] = held;
        lastParent--;
    }

    end = count;
    while (end > 1) {
        held = heap[end];
        heap[end] = heap[1];
        end--;
        parent = 1;
        child = parent * 2;
        while (child <= end) {
            if (child < end && heap[child].rank > heap[child + 1].rank) {
                child++;
            }
            if (held.rank > heap[child].rank) {
                heap[parent] = heap[child];
                parent = child;
                child = parent * 2;
            } else {
                break;
            }
        }
        heap[parent] = held;
    }
}

/* Creates a player's model and motions, then installs the per-frame character update. */
void fn_1_AD38(OMOBJ *object)
{
    M680CharacterState *data;
    s32 motion;
    s16 motionId;

    data = object->data;
    data->object = object;
    data->playerIndex = (s16)object->work[0];
    data->character = GwPlayerConf[data->playerIndex].charNo;
    data->group = GwPlayerConf[data->playerIndex].grpNo;
    data->padNo = GwPlayerConf[data->playerIndex].padNo;
    data->playerType = GwPlayerConf[data->playerIndex].type;
    data->difficulty = GwPlayerConf[data->playerIndex].comDif;
    data->playerSlot = (s16)object->work[1];

    *object->mdlId = CharModelCreate(data->character, 4);
    data->modelId = *object->mdlId;
    for (motion = 0; motion < 10; motion++) {
        motionId = CharMotionCreate(data->character, lbl_1_data_6C0[motion]);
        data->motionIds[motion] = motionId;
        object->mtnId[motion] = motionId;
    }
    CharMotionSet(data->character, *object->mtnId);
    CharModelAttrReset(data->character, HU3D_MOTATTR_PAUSE);
    CharModelAttrSet(data->character, HU3D_MOTATTR_LOOP);
    CharModelAttrReset(data->character, 1);
    CharMotionDataClose(data->character);
    CharModelVoiceFlagSet(data->character, 1);
    CharEffectLayerSet(6);
    CharModelStepSet(data->character, 0);
    /* This character motion is loaded without its voice cue. */
    CharMotionVoiceOnSet(data->character, M680_VOICE_OFF_MOTION, 0);
    Hu3DModelLayerSet(data->modelId, 3);
    Hu3DModelShadowSet(data->modelId);

    data->position.x = 0.0f;
    data->position.y = 0.0f;
    data->position.z = 0.0f;
    data->rotation.x = 0.0f;
    data->rotation.y = 0.0f;
    data->rotation.z = 0.0f;
    data->targetAngle = 180.0f;
    data->clearedValue = 0.0f;
    data->eliminated = 0;
    Hu3DModelPosSetV(data->modelId, &data->position);
    omSetTra(data->object, data->position.x, data->position.y, data->position.z);
    Hu3DModelRotSetV(data->modelId, &data->rotation);
    omSetRot(data->object, data->rotation.x, data->rotation.y, data->rotation.z);
    fn_1_6538(data, data->playerSlot);
    object->objFunc = fn_1_AFF8;
}

/* Updates the character heading and copies its transform to the model and object each frame. */
void fn_1_AFF8(OMOBJ *root)
{
    M680CharacterState *sequence;

    sequence = root->data;
    sequence->clearedValue = 0.0f;
    if (sequence->playerType != 0) {
        fn_1_B0A8(sequence);
    } else {
        fn_1_B0A4(sequence);
    }
    fn_1_B0AC(sequence);
    Hu3DModelPosSetV(sequence->modelId, &sequence->position);
    omSetTra(sequence->object, sequence->position.x, sequence->position.y, sequence->position.z);
    Hu3DModelRotSetV(sequence->modelId, &sequence->rotation);
    omSetRot(sequence->object, sequence->rotation.x, sequence->rotation.y, sequence->rotation.z);
}

/* The human-controlled character has no separate per-frame movement input. */
void fn_1_B0A4(M680CharacterState *work)
{
}

/* The computer-controlled character has no separate per-frame movement input. */
void fn_1_B0A8(M680CharacterState *work)
{
}

/* Turns the character toward its target heading by at most twelve degrees per frame. */
void fn_1_B0AC(M680CharacterState *work) {
    f32 delta;

    while (work->targetAngle > 180.0f) {
        work->targetAngle -= 360.0f;
    }
    while (work->targetAngle < (-180.0f)) {
        work->targetAngle += 360.0f;
    }
    while (work->rotation.y > 180.0f) {
        work->rotation.y -= 360.0f;
    }
    while (work->rotation.y < (-180.0f)) {
        work->rotation.y += 360.0f;
    }
    delta = work->targetAngle - work->rotation.y;
    while (delta > 180.0f) {
        delta -= 360.0f;
    }
    while (delta < (-180.0f)) {
        delta += 360.0f;
    }
    if (delta > 0.0f) {
        if (delta > 12.0f) {
            delta = 12.0f;
        }
    } else if ((delta < 0.0f) && (delta < (-12.0f))) {
        delta = (-12.0f);
    }
    work->rotation.y += delta;
}

/* Moves the character along a curved path and restores the destination motion when it arrives. */
s32 fn_1_B26C(void *argument)
{
    M680CharacterState *work;
    Vec previousPosition;
    f32 offsetAngle;
    f32 amplitude;
    f32 targetDeltaX;
    f32 targetDeltaZ;
    f32 targetAngle;
    f32 currentAngle;
    f32 targetAngleResult;
    f32 currentAngleResult;
    f32 currentDeltaX;
    f32 currentDeltaZ;
    f32 progress;
    s32 done;

    work = argument;
    done = 0;
    switch (work->state) {
    case 0:
        work->frame = 0;
        work->state++;
        /* Fall through to update the new motion. */
    case 1:
        work->frame++;
        progress = (f32)work->frame / (f32)work->travelFrames;
        previousPosition = work->position;
        work->position.x = work->destination.x * progress + work->origin.x * (1.0f - progress);
        work->position.y = work->destination.y * progress + work->origin.y * (1.0f - progress);
        work->position.z = work->destination.z * progress + work->origin.z * (1.0f - progress);
        amplitude = (f32)(50.0 * sin(3.141592653589793 * (f64)progress));
        targetDeltaZ = work->destination.z - work->origin.z;
        targetDeltaX = work->destination.x - work->origin.x;
        if (0.0f == targetDeltaZ) {
            if (targetDeltaX >= 0.0f) {
                targetAngle = 1.5707964f;
            } else {
                targetAngle = (-1.5707964f);
            }
        } else {
            targetAngleResult = (f32)atan2((f64)targetDeltaX, (f64)targetDeltaZ);
            targetAngle = targetAngleResult;
        }
        offsetAngle = targetAngle;
        work->position.x = (f32) ((f64) work->position.x +
                                  (f64) amplitude * sin((f64) offsetAngle - 1.5707963267948966));
        work->position.z = (f32) ((f64) work->position.z +
                                  (f64) amplitude * cos((f64) offsetAngle - 1.5707963267948966));
        currentDeltaZ = work->position.z - previousPosition.z;
        currentDeltaX = work->position.x - previousPosition.x;
        if (0.0f == currentDeltaZ) {
            if (currentDeltaX >= 0.0f) {
                currentAngle = 1.5707964f;
            } else {
                currentAngle = (-1.5707964f);
            }
        } else {
            currentAngleResult = (f32)atan2((f64)currentDeltaX, (f64)currentDeltaZ);
            currentAngle = currentAngleResult;
        }
        work->targetAngle = (f32)((f64)(180.0f * currentAngle) / 3.141592653589793);
        if (work->motionIds[2] != Hu3DMotionIDGet(work->modelId) &&
            work->motionIds[2] != Hu3DMotionShiftIDGet(work->modelId)) {
            CharMotionShiftSet(work->character, work->motionIds[2], 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
        }
        if (work->frame >= work->travelFrames) {
            work->state++;
        }
        break;
    case 2:
        work->targetAngle = work->endAngle;
        CharMotionShiftSet(work->character, work->motionIds[0], 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
        done = 1;
        break;
    }
    return done;
}

/* Runs the special destination animation and turns the character over twenty-nine frames. */
s32 fn_1_B6CC(void *argument)
{
    M680CharacterState *work;
    s32 done;
    f32 progress;

    work = argument;
    done = 0;
    switch (work->state) {
    case 0:
        CharMotionShiftSet(work->character, work->motionIds[8], 0.0f,
                           5.0f, 0);
        Hu3DMotionShiftSpeedSet(work->modelId, 2.0f);
        work->frame = 0;
        (work->state)++;
        /* Fall through to update the new motion. */
    case 1:
        (work->frame)++;
        progress = (f32)work->frame / 29.0f;
        work->targetAngle = 180.0f - 90.0f * progress;
        work->rotation.y = work->targetAngle;
        if (work->frame >= 29) {
            CharMotionShiftSet(work->character, work->motionIds[0], 0.0f,
                               5.0f, HU3D_MOTATTR_LOOP);
            CharMotionSpeedSet(work->character, 1.0f);
            done = 1;
        }
        break;
    }
    return done;
}

/* Chooses a destination and speed, then schedules the character's movement animation. */
void fn_1_B844(M680CharacterState *work, s32 targetIndex, s32 speedIndex)
{
    Vec targets[6] = {
        {0.0f, 0.0f, 1350.0f}, {-150.0f, 0.0f, 1350.0f},
        {-300.0f, 0.0f, 1350.0f}, {-450.0f, 0.0f, 1350.0f},
        {-20.5f, 0.0f, 1191.225f}, {-20.5f, 0.0f, 1191.225f}
    };
    f32 rates[6] = {90.0f, 90.0f, 90.0f, 90.0f, 180.0f, 90.0f};
    f32 speeds[2] = {5.0f, 10.0f};
    f32 deltaX;
    f32 deltaZ;

    work->origin = work->position;
    work->destination = targets[targetIndex];
    work->endAngle = rates[targetIndex];
    deltaX = work->origin.x - work->destination.x;
    deltaZ = work->origin.z - work->destination.z;
    work->travelFrames = (s32)(sqrtf(deltaX * deltaX + deltaZ * deltaZ) / speeds[speedIndex]);
    work->state = 0;
    if (targetIndex == 5) {
        fn_1_BE08(fn_1_B6CC, work);
    } else {
        fn_1_BE08(fn_1_B26C, work);
    }
}

/* Starts the character's result-pose callback for the pre-winner sequence. */
void fn_1_BAF8(M680CharacterState *state) {
    state->state = 0;
    fn_1_BE08(fn_1_BB38, state);
}

/* Turns the character back to zero degrees and reports completion after its idle motion starts. */
s32 fn_1_BB38(void *argument)
{
    M680CharacterState *work;
    f32 progress;

    work = argument;
    switch (work->state) {
    case 0:
        CharMotionShiftSet(work->character, work->motionIds[8], 0.0f,
                           5.0f, HU3D_MOTATTR_LOOP);
        work->frame = 0;
        (work->state)++;
        /* Fall through to update the new motion. */
    case 1:
        (work->frame)++;
        progress = (f32)work->frame / 60.0f;
        work->targetAngle = 90.0f * (1.0f - progress);
        work->rotation.y = work->targetAngle;
        if (work->frame >= 60) {
            CharMotionShiftSet(work->character, work->motionIds[0], 0.0f,
                               5.0f, HU3D_MOTATTR_LOOP);
            (work->state)++;
        }
        break;
    case 2:
        if (work->motionIds[0] == Hu3DMotionIDGet(work->modelId)) {
            fn_1_9600();
            return 1;
        }
        break;
    }
    return 0;
}

/* Starts the character's extra motion and schedules a callback to wait for its end. */
void fn_1_BCBC(M680CharacterState *motion) {
    CharMotionShiftSet(motion->character, motion->motionIds[9], 0.0f, 5.0f, 0);
    fn_1_BE08(fn_1_BD1C, motion);
}

/* Completes the scheduled action when the character's motion ends. */
s32 fn_1_BD1C(void *argument) {
    M680CharacterState *state = argument;

    if (Hu3DMotionEndCheck(state->modelId) != 0) {
        return 1;
    }
    return 0;
}

u32 lbl_1_data_6C0[10] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mariomot, 2), DATANUM(DATA_mario, 167),
    DATANUM(DATA_mariomot, 24), DATANUM(DATA_mariomot, 34),
    DATANUM(DATA_mariomot, 36), DATANUM(DATA_mariomot, 6),
    DATANUM(DATA_mariomot, 13), DATANUM(DATA_mariomot, 77)
};
