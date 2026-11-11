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
const u32 lbl_1_rodata_668[4] = {1, 2, 4, 8};
}

MotionModel::MotionModel(s32 resourceStateValue, s32 cameraIndex)
    : model(DATANUM(0x3e0000, 13))
{
    visible = 1;
    positionReady = 0;
    secondaryState = 0;
    resourceState = resourceStateValue;
    this->cameraIndex = cameraIndex;
    insert4(&lbl_1_bss_A70);
    motionId = fn_80030458(primary_model(), fn_80008DD0(DATANUM(0x3e0000, 14), 0x10000000, 2));
    {
        s16 modelId = model_id();
        fn_80026B8C(modelId, 0);
        fn_80026B64(modelId, (u16)lbl_1_rodata_668[cameraIndex]);
        fn_800269E8(modelId, (1 << 14));
        fn_8002C95C(modelId, motionId);
    }
    fn_1_D9CC();
}

void fn_1_A12C()
{
    while (!lbl_1_bss_A70.empty()) delete lbl_1_bss_A70.back();
}

void fn_1_D698()
{
    lbl_1_bss_A70.visit(MotionModel::fn_1_D6C8);
}

void MotionModel::fn_1_D6C8(MotionModel *self)
{
    (self->*self->callback)();
    self->fn_1_D708();
}

void MotionModel::fn_1_D708()
{
    model.setRotation(&direction);
    model.setPosition(&position);
}

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

void MotionModel::fn_1_D7D4()
{
    if (!this->visible) {
        this->visible = 1;
        HU3D_MODELID modelId = this->primary_model();
        fn_80026A44(modelId, (1 << 0));
        fn_800269E8(modelId, ((1 << 30) | (1 << 0)));
        fn_80026A44(modelId, ((1 << 30) | (1 << 1)));
    }
}

void MotionModel::fn_1_D858()
{
    if (this->visible = 1) {
        this->visible = 0;
        HU3D_MODELID modelId = this->primary_model();
        fn_800269E8(modelId, ((1 << 30) | (1 << 0)));
        fn_800269E8(modelId, ((1 << 30) | (1 << 1)));
    }
}

void MotionModel::fn_1_D8D8(Vec *position, Vec *direction)
{
    fn_1_D7D4();
    this->positionReady = 1;
    this->position = *position;
    this->direction = *direction;
    MotionModel::Callback callback = &MotionModel::fn_1_DD9C;
    this->callback = callback;
}

void MotionModel::fn_1_D9CC()
{
    s16 modelId;

    fn_1_D858();
    positionReady = 0;
    modelId = model_id();
    fn_800269E8(modelId, (1 << 0));
    {
        MotionModel::Callback callback = &MotionModel::fn_1_DDA0;
        this->callback = callback;
    }
}

void MotionModel::fn_1_DA9C()
{
    s16 modelId;

    fn_1_D858();
    this->secondaryState = 1;
    modelId = this->model_id();
    {
        MotionModel::Callback callback = &MotionModel::fn_1_DDA4;
        this->callback = callback;
    }
}

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

void MotionModel::fn_1_DD9C() {}
void MotionModel::fn_1_DDA0() {}
void MotionModel::fn_1_DDA4() {}

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
