#include "REL/m603DLL/frame_time.h"
/* Model motion callbacks and their shared entry list. */
extern "C" {
#include "math.h"
#include "game/hu3d.h"
#include "game/data.h"
#include "datadir_enum.h"
#include "REL/m603DLL.h"
}
#include "REL/framework/linknode.h"
#define M603_OWNED_MODEL_DECLARATION
#include "REL/m603DLL/model.h"
#undef M603_OWNED_MODEL_DECLARATION

#include "REL/m603DLL/model_motion.h"

typedef void (*ModelVisitor)(MotionModel *);
class MotionModelList : public Node {
public:
    ~MotionModelList() {}
    bool empty() const { return link0 == this && link0 == link4; }
    Node *first_node() { return link0; }
    Node *last_node() { return link4; }
    MotionModel *front() { return static_cast<MotionModel *>(first_node()); }
    MotionModel *back() { return static_cast<MotionModel *>(last_node()); }
    MotionModel *begin() { return static_cast<MotionModel *>(first_node()); }
    Node *end() { return this; }
    Node *end_node() { return this; }
    void fn_1_DEE8(ModelVisitor);
};
extern "C" {

long lbl_1_bss_A58 = secondsToFrames(2L);
long lbl_1_bss_A5C = (long)secondsToFrames(3.5);
long lbl_1_bss_A60 = (long)secondsToFrames(3.5);
MotionModelList lbl_1_bss_A70;
}
void fn_1_DF48(Node *, Node *, ModelVisitor);

extern "C" {
const u32 lbl_1_rodata_668[4] = {1, 2, 4, 8};
}

MotionModel::MotionModel(s32 resourceStateValue, s32 cameraIndex)
    : OwnedModel(DATANUM(0x3e0000, 13))
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

ModelCallbackNode::~ModelCallbackNode() {}

void fn_1_A12C()
{
    while (!lbl_1_bss_A70.empty()) delete lbl_1_bss_A70.back();
}

void fn_1_D698()
{
    lbl_1_bss_A70.fn_1_DEE8(MotionModel::fn_1_D6C8);
}

void MotionModel::fn_1_D6C8(MotionModel *self)
{
    (self->*self->callback)();
    self->fn_1_D708();
}

void MotionModel::fn_1_D708()
{
    fn_1_33ACC(&direction);
    fn_1_33A3C(&position);
}

int fn_1_D748(int key)
{
    MotionModel *node = lbl_1_bss_A70.front();
    Node *end = lbl_1_bss_A70.end_node();
    for (; node != end; node = node->next_node()) {
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

static inline MotionModel *firstModel() { return static_cast<MotionModel *>(lbl_1_bss_A70.first_node()); }

void fn_1_DB70(s32 resourceStateValue)
{
    Node *end;
    MotionModel *node;

    node = firstModel();
    end = lbl_1_bss_A70.end_node();

    for (; node != end; node = node->next_node()) {
        if (node->resourceState == resourceStateValue && !node->secondaryState) {
            node->fn_1_D9CC();
        }
    }
}

void fn_1_DC8C(s32 cameraIndex)
{
    Node *end;
    MotionModel *node;

    node = firstModel();
    end = lbl_1_bss_A70.end_node();

    for (; node != end; node = node->next_node()) {
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

    node = firstModel();
    end = lbl_1_bss_A70.end_node();

    for (; static_cast<Node *>(node) != end; node = node->next_node()) {
        if (node->resourceState == resourceState && node->secondaryState == 0) {
            node->fn_1_D8D8(position, direction);
        }
    }
}

void MotionModelList::fn_1_DEE8(ModelVisitor visitor)
{
    fn_1_DF48(begin(), end(), visitor);
}

void fn_1_DF48(Node *current, Node *sentinel, ModelVisitor callback)
{
    while (current != sentinel) {
        callback(static_cast<MotionModel *>(current));
        Node *next = current->link0;
        current = next;
    }
}
