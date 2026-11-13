#include "REL/framework/frame_time_inline.h"
/* Model motion callbacks and their shared entry list. */
extern "C" {
#include "math.h"
#include "game/hu3d.h"
#include "game/data.h"
#include "datadir_enum.h"
#include "REL/m603DLL.h"
}
#include "REL/framework/linknode.h"
#include "REL/framework/model.h"

#include "REL/m603DLL/model_motion.h"

extern "C" {
ListRoot<MotionModel> lbl_1_bss_A70;
}

extern "C" {
const u32 motionCameraBits[4] = {1, 2, 4, 8};
}

/* Create a camera-specific auxiliary model and load its motion. */
MotionModel::MotionModel(s32 resourceStateValue, s32 cameraIndex)
    : model(DATANUM(0x3e0000, 13))
{
    visible = 1;
    positionReady = 0;
    secondaryState = 0;
    resourceState = resourceStateValue;
    this->cameraIndex = cameraIndex;
    insert4(&lbl_1_bss_A70);
    motionId = fn_80030458(model.id(), fn_80008DD0(DATANUM(0x3e0000, 14), 0x10000000, 2));
    s16 modelId = model.id();
    fn_80026B8C(modelId, 0);
    fn_80026B64(modelId, (u16)motionCameraBits[cameraIndex]);
    fn_800269E8(modelId, (1 << 14));
    fn_8002C95C(modelId, motionId);
    fn_1_D9CC();
}

/* Delete every auxiliary motion model. */
void fn_1_A12C()
{
    while (!lbl_1_bss_A70.empty()) delete lbl_1_bss_A70.back();
}

/* Advance the auxiliary motion model list. */
void fn_1_D698()
{
    lbl_1_bss_A70.visit(MotionModel::fn_1_D6C8);
}

/* Run one model callback and apply its transform. */
void MotionModel::fn_1_D6C8(MotionModel *self)
{
    (self->*self->callback)();
    self->fn_1_D708();
}

/* Apply the stored model position and rotation. */
void MotionModel::fn_1_D708()
{
    model.setRotation(&direction);
    model.setPosition(&position);
}

/* Find an available auxiliary model for the requested camera. */
int fn_1_D748(int key)
{
    MotionModel *node = lbl_1_bss_A70.front();
    Node *end = lbl_1_bss_A70.end_node();
    for (; node != end; node = node->next()) {
        if (node->cameraIndex == key && !node->positionReady && !node->secondaryState) {
            return node->resourceState;
        }
    }
    return -1;
}

/* Set the visible flag and restore the model attributes when hidden. */
void MotionModel::fn_1_D7D4()
{
    if (!this->visible) {
        this->visible = 1;
        HU3D_MODELID modelId = this->model.id();
        fn_80026A44(modelId, (1 << 0));
        fn_800269E8(modelId, ((1 << 30) | (1 << 0)));
        fn_80026A44(modelId, ((1 << 30) | (1 << 1)));
    }
}

/* Assign the visible flag before clearing it and setting both model attributes. */
void MotionModel::fn_1_D858()
{
    // The assignment makes this branch unconditional before visible is cleared.
    if (this->visible = 1) {
        this->visible = 0;
        HU3D_MODELID modelId = this->model.id();
        fn_800269E8(modelId, ((1 << 30) | (1 << 0)));
        fn_800269E8(modelId, ((1 << 30) | (1 << 1)));
    }
}

/* Show the model and copy its position and direction. */
void MotionModel::fn_1_D8D8(Vec *position, Vec *direction)
{
    fn_1_D7D4();
    this->positionReady = 1;
    this->position = *position;
    this->direction = *direction;
    MotionModel::Callback callback = &MotionModel::fn_1_DD9C;
    this->callback = callback;
}

/* Hide the model and clear the position-ready state. */
void MotionModel::fn_1_D9CC()
{
    s16 modelId;

    fn_1_D858();
    positionReady = 0;
    modelId = model.id();
    fn_800269E8(modelId, (1 << 0));
    MotionModel::Callback callback = &MotionModel::fn_1_DDA0;
    this->callback = callback;
}

/* Hide the model and mark its secondary state. */
void MotionModel::fn_1_DA9C()
{
    s16 modelId;

    fn_1_D858();
    this->secondaryState = 1;
    modelId = this->model.id();
    MotionModel::Callback callback = &MotionModel::fn_1_DDA4;
    this->callback = callback;
}

/* Reset the matching auxiliary model unless it has entered its secondary state. */
void fn_1_DB70(s32 resourceStateValue)
{
    Node *end;
    MotionModel *node;

    node = lbl_1_bss_A70.front();
    end = lbl_1_bss_A70.end_node();

    for (; node != end; node = node->next()) {
        if (node->resourceState == resourceStateValue && !node->secondaryState) {
            node->fn_1_D9CC();
        }
    }
}

/* Move all eligible models for a camera into their secondary state. */
void fn_1_DC8C(s32 cameraIndex)
{
    Node *end;
    MotionModel *node;

    node = lbl_1_bss_A70.front();
    end = lbl_1_bss_A70.end_node();

    for (; node != end; node = node->next()) {
        if (node->cameraIndex == cameraIndex && !node->secondaryState) {
            node->fn_1_DA9C();
        }
    }
}

/* Leave the positioned model unchanged. */
void MotionModel::fn_1_DD9C() {}
/* Leave the reset model unchanged. */
void MotionModel::fn_1_DDA0() {}
/* Leave the secondary model state unchanged. */
void MotionModel::fn_1_DDA4() {}

/* Position the matching eligible auxiliary model. */
void fn_1_DDA8(s32 resourceState, Vec *position, Vec *direction)
{
    Node *end;
    MotionModel *node;

    node = lbl_1_bss_A70.front();
    end = lbl_1_bss_A70.end_node();

    for (; static_cast<Node *>(node) != end; node = node->next()) {
        if (node->resourceState == resourceState && node->secondaryState == 0) {
            node->fn_1_D8D8(position, direction);
        }
    }
}
