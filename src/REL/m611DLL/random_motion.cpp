#include "REL/m611DLL/game_requests.h"
#include "datadir_enum.h"
#include "REL/m611DLL/angle.h"
#include "REL/m611DLL/list_rotation.h"
#include "REL/m611DLL/motion_position_table.h"
#include "REL/framework/minigame_core.h"
#include "REL/m611DLL/scene_camera.h"
#include "REL/framework/list_object.h"
#include "REL/framework/phase_duration_statics.h"
#include "REL/framework/seconds_to_frames_inline.h"
#include "REL/m611DLL/wandering_model.h"
#include "REL/m611DLL/motion_transform.h"
#include "REL/framework/random.h"
#include "REL/framework/model.h"
#include <cmath>
#include <new>

extern "C" {
#include "game/hu3d.h"
#include "dolphin/mtx.h"
#include "dolphin/types.h"
#include <math.h>
#include "humath.h"
}

// Head of the wandering model list.
ListRoot<WanderingModel> lbl_1_bss_18;

// Chooses a horizontal destination for a wandering model.
void fn_1_21C(Vec *position)
{
    randomizeMotionPosition(position);
}

// Creates the three model groups and their moving instances.
MotionPositionTable::MotionPositionTable(bool dayMode)
    : slot0Model(dayMode ? DATANUM(0x460000, 18) :
                                 DATANUM(0x460000, 19)),
      slot1Model(dayMode ? DATANUM(0x460000, 20) :
                                 DATANUM(0x460000, 21)),
      slot2Model(dayMode ? DATANUM(0x460000, 22) :
                                 DATANUM(0x460000, 23))
{
    s32 slot0Instance;
    s32 slot1Instance;
    s32 slot2Instance;
    s32 index;
    s16 slot0ModelId;
    s16 slot1ModelId;
    s16 slot2ModelId;
    s16 modelId;
    slot0ModelId = static_cast<s16>(slot0Model.modelId);
    modelId = slot0ModelId;
    fn_800269E8(modelId, 0x1);
    slot1ModelId = static_cast<s16>(slot1Model.modelId);
    modelId = slot1ModelId;
    fn_800269E8(modelId, 0x1);
    slot2ModelId = static_cast<s16>(slot2Model.modelId);
    modelId = slot2ModelId;
    fn_800269E8(modelId, 0x1);
    for (index = 0; index < 3; ++index) {
        fn_1_21C(&positions[index]);
        motionFrames[index] =
            static_cast<s32>(secondsToFrames(2.0f) + (RandomFloat() * secondsToFrames(3.0f)));
    }
    for (slot0Instance = 0; slot0Instance < 6; ++slot0Instance) {
        Vec position;
        position.x = static_cast<f32>(3000.0 * (RandomFloat() - 0.5));
        position.y = static_cast<f32>(-100.0 - static_cast<f64>(slot0Instance * 60));
        position.z = static_cast<f32>(3000.0 * (RandomFloat() - 0.5));
        new WanderingModel(slot0Model, &position, 0);
    }
    for (slot1Instance = 0; slot1Instance < 6; ++slot1Instance) {
        Vec position;
        position.x = static_cast<f32>(3000.0 * (RandomFloat() - 0.5));
        position.y = static_cast<f32>(-120.0 - static_cast<f64>(slot1Instance * 60));
        position.z = static_cast<f32>(3000.0 * (RandomFloat() - 0.5));
        new WanderingModel(slot1Model, &position, 1);
    }
    for (slot2Instance = 0; slot2Instance < 6; ++slot2Instance) {
        Vec position;
        position.x = static_cast<f32>(3000.0 * (RandomFloat() - 0.5));
        position.y = static_cast<f32>(-140.0 - static_cast<f64>(slot2Instance * 60));
        position.z = static_cast<f32>(3000.0 * (RandomFloat() - 0.5));
        new WanderingModel(slot2Model, &position, 2);
    }
}

// Deletes the moving instances owned by the scene.
MotionPositionTable::~MotionPositionTable()
{
    fn_1_DEC();
}

// Refreshes expired targets and advances the moving instances.
void MotionPositionTable::update()
{
    s32 index;

    for (index = 0; index < 3; ++index) {
        if (--motionFrames[index] < 0) {
            fn_1_21C(&positions[index]);
            motionFrames[index] =
                static_cast<s32>(secondsToFrames(2.0f) + RandomFloat() * secondsToFrames(3.0f));
        }
    }
    fn_1_ED8();
}

// Acknowledges a scene request.
void MotionPositionTable::flagEnabled(int request)
{
    acknowledgeSceneRequest(flags, request);
}

// Initializes the moving model, its start position, and its update callback.
WanderingModel::WanderingModel(const OwnedModel &modelSource,
                               Vec *initialPosition, s32 slotIndex)
    : positionIndex(slotIndex), model(modelSource)
{
    s16 modelId;
    f32 modelScale;
    void (WanderingModel::*updateCallback)();

    this->speed = 1.0f;
    this->turnRate = 0.0f;
    this->insert4(&lbl_1_bss_18);
    this->position = *initialPosition;

    modelId = this->model;
    modelScale = 0.3f + 0.3f * RandomFloat();
    fn_800268B0(modelId, modelScale, modelScale, modelScale);
    fn_80026A44(modelId, 0x1);
    fn_80026A44(modelId, 0x40000002);
    fn_800269E8(modelId, 0x40000001);
    fn_8002D414(modelId, 2.0f);
    fn_80026B8C(modelId, 0);
    fn_80026B64(modelId, 1);
    this->heading = RandomFloat() * 360.0f - 180.0f;
    this->model.setPosition(&this->position);
    fn_1_1018(this);
    updateCallback = &WanderingModel::fn_1_1154;
    this->callback = updateCallback;
}

// Deletes every moving instance.
void fn_1_DEC()
{
    deleteEffects(lbl_1_bss_18);
}

// Updates the moving instances and rotates their list order.
void fn_1_ED8()
{
    lbl_1_bss_18.visit(fn_1_F5C);
    rotateListFront(lbl_1_bss_18);
}

// Runs one instance callback and applies its model transform.
void fn_1_F5C(WanderingModel *object)
{
    (object->*object->callback)();
    object->applyModelTransform();
}

// Applies the current heading and position to the model.
void WanderingModel::applyModelTransform()
{
    applyMotionTransform(model, heading, &position);
}

// Initializes an instance target and its movement weights.
void fn_1_1018(WanderingModel *object)
{
    s32 index;
    MotionPositionTable *table;
    f32 turnRateScale;

    object->duration = static_cast<s32>(secondsToFrames(1.0f) +
                                        RandomFloat() * secondsToFrames(2.0f));
    index = object->positionIndex;

    table = &MinigameSingleton<MotionPositionTable>::getChecked();
    object->target.x = table->positions[index].x;
    object->target.y = ((f32 *)&table->positions[index])[1];
    object->target.z = ((f32 *)&table->positions[index])[2];
    turnRateScale = 1.0f;
    object->targetTurnRate = static_cast<f32>(turnRateScale * RandomFloat() + 0.2 * turnRateScale);
    object->targetSpeed = 0.5f + RandomFloat();
}

// Moves the model and eases its speed and turn rate toward their targets.
void WanderingModel::fn_1_1154()
{
    f32 angle;

    if (--this->duration < 0) {
        fn_1_1018(this);
    }

    this->speed += (1.0f / 20.0f) * (this->targetSpeed - this->speed);
    this->turnRate += (1.0f / 20.0f) * (this->targetTurnRate - this->turnRate);
    angle = static_cast<f32>(180.0 *
                  (std::atan2(this->target.x - this->position.x,
                         this->target.z - this->position.z) /
                   M_PI));
    this->heading += fn_1_B8(angle, this->heading, this->turnRate);
    fn_1_1368(this);
}

// Moves the instance forward at its current heading and speed.
void fn_1_1368(WanderingModel *object)
{
    Mtx rotation;
    Vec source;
    Vec transformed;

    static f64 speedFrames = secondsToFrames(1.0f);
    source.x = 0.0f;
    source.y = 0.0f;
    source.z = static_cast<f32>(static_cast<f64>(400.0f * object->speed) / speedFrames);
    rotateForwardStep(object, rotation, source, transformed);
    object->position.x += transformed.x;
    object->position.y += transformed.y;
    object->position.z += transformed.z;
}
