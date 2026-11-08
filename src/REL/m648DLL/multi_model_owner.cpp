#include "REL/m648DLL/callback_walk.h"
extern "C" {
#include "dolphin/types.h"
}

extern "C" {
#include "dolphin/mtx/GeoTypes.h"
}

extern "C" {
#include <math.h>
}

extern "C" {
#include "dolphin/gx.h"
}

extern "C" {
#include "game/data.h"
}

extern "C" {
#include "game/hu3d.h"
}

extern "C" {
#include "game/memory.h"
}

#include "REL/m648DLL/framework/linknode.h"

#include "REL/framework/linknode.h"

extern "C" {
#include "game/main.h"
}

extern "C" {
#include "game/object.h"
}

extern "C" {
#include "game/audio.h"
}

extern "C" {
#include "game/charman.h"
}

extern "C" {
#include "game/gamemes.h"
}

extern "C" {
#include "game/hsfex.h"
}

extern "C" {
#include "game/gamework.h"
}

extern "C" {
#include "game/mg/seqman.h"
}

extern "C" {
#include "game/mg/timer.h"
}

extern "C" {
#include "game/mg/score.h"
}

extern "C" {
#include "game/pad.h"
}

extern "C" {
#include "game/frand.h"
}

extern "C" {
#include "game/sprite.h"
}

extern "C" {
#include "game/wipe.h"
}

extern "C" {
#include "game/mg/actman.h"
}

extern "C" {
#include "game/board/object.h"
}

extern "C" {
#include "game/board/tutorial.h"
}

extern "C" {
#include "datadir_enum.h"
}

#include "REL/framework/model.h"
#include "REL/m648DLL/animation_object.h"
#include "REL/m648DLL/multi_model_owner.h"
extern "C" {
}

extern "C" {

extern void *__ct__4NodeFv(void *object);
extern void *insert4__4NodeFP4Node(void *node, void *parent);
extern void __msl_assertion_failed(const char *expression, const char *file,
                                   int line);

extern void *lbl_1_bss_9C;
extern char lbl_1_data_108[];
extern char lbl_1_data_112[];
extern const u8 lbl_1_data_5C[];
extern const u8 lbl_1_data_15C[];
void *__ct__4NodeFv(void *object);

void *insert4__4NodeFP4Node(void *node, void *parent);

}

AnimationUpdateOwner::AnimationUpdateOwner() {}

extern "C" {

}

/* Native C++ callback and singleton-lifetime recovery.
 * Class names are reconstructed. These are consumed interface views, not
 * claims about complete original class layouts; none is allocated here.
 * The callback descriptor at +4 is a real pointer-to-member, not a C callback
 * or a manually encoded compiler helper. Unknown word +0 is not padding.
 * Fragment selectors choose entire functions, not compiler behavior or TUs. */

struct CallbackListOwner {
    u32 nodeLinks[2];
    const u8 *callbackTable;
    u32 ownerState[2];
};

extern "C" const u8 lbl_1_data_15C[];
extern "C" const u8 lbl_1_data_5C[];
extern "C" void *lbl_1_bss_9C;
void fn_1_4B3C(void);
extern "C" void *__dt__4NodeFv(void *object, s16 mode);
void operator delete(void *object);

AnimationUpdateOwner::~AnimationUpdateOwner() { fn_1_4B3C(); }

void fn_1_4D00(void);

void AnimationUpdateOwner::update()
{
    fn_1_4D00();
}

struct SoundRequestFlags { u32 state[3]; u32 flags; };
void fn_1_4ABC(void);
void AnimationUpdateOwner::clearRequest(int bit)
{
    state &= ~(1UL << bit);
    switch (bit) {
    case 0:
    case 1:
    case 3:
    case 4:
        break;
    case 2:
        fn_1_4ABC();
        break;
    }
}

/* Sets a model's rendering layer, camera, visibility, and shadow state. */
void fn_1_4454(s16 modelId, u8 showModel)
{
    fn_80026B8C(modelId, 3);
    fn_80026B64(modelId, 1);
    if (showModel) {
        fn_80026A44(modelId, (1 << 0));
    } else {
        fn_800269E8(modelId, (1 << 0));
    }
    fn_800269E8(modelId, 0x40000002);
    fn_80026E44(modelId);
    fn_80026F9C(modelId);
}

void fn_1_44E0(Vec *position, s32 index)
{
    position->x = (f32)(index * 0x104 - 0x186);
    position->y = 0.0f;
    position->z = -400.0f;
}

extern "C" {
extern void *__ct__4NodeFv(void *);
}

extern "C" {
extern void *insert4__4NodeFP4Node(void *, void *);
}

extern "C" {
extern void __ct__10OwnedModelFUi(void *, s32);
}

struct MotionHookModels;
void fn_1_4EB8(MotionHookModels *, s32);

struct CallbackListRoot;
extern CallbackListRoot lbl_1_bss_C0;

/* Creates the owner and configures its seven linked motion models. */
MotionHookModels *fn_1_453C(MotionHookModels *owner,
                                     s32 sequence, s32 mode)
{
    s16 motionId;
    MotionHookModels * baseOwner;
    s16 modelOneId;
    s16 modelTwoId;
    s16 modelThreeId;
    s16 modelFourId;
    s16 currentModelOne;
    s16 modelFiveId;
    s16 modelSixId;
    s16 modelSevenId;
    s16 currentModelTwo;
    s16 currentModelThree;
    s16 currentModelFour;
    s16 currentModelFive;
    s16 currentModelSix;
    s16 currentModelSeven;
    baseOwner = owner;
    __ct__4NodeFv(baseOwner);
    __ct__10OwnedModelFUi(& owner->hookModel, DATANUM(0x6b0000, 7));
    __ct__10OwnedModelFUi(& owner->models[0], DATANUM(0x6b0000, 22));
    __ct__10OwnedModelFUi(& owner->models[1], DATANUM(0x6b0000, 23));
    __ct__10OwnedModelFUi(& owner->models[2], DATANUM(0x6b0000, 24));
    __ct__10OwnedModelFUi(& owner->models[3], DATANUM(0x6b0000, 25));
    __ct__10OwnedModelFUi(& owner->models[4], DATANUM(0x6b0000, 26));
    __ct__10OwnedModelFUi(& owner->models[5], DATANUM(0x6b0000, 27));
    owner->complete = 0;
    owner->mode = mode;
    owner->sequence = sequence;
    owner->callbackCount = 0;
    insert4__4NodeFP4Node(owner, &lbl_1_bss_C0);
    modelOneId = (s16) owner->hookModel.modelId;
    currentModelOne = modelOneId;
    fn_80026B8C(currentModelOne, 3);
    fn_80026B64(currentModelOne, 1U);
    fn_80026A44(currentModelOne, 1U);
    fn_800269E8(currentModelOne, 0x40000002U);
    fn_80026E44(currentModelOne);
    fn_80026F9C(currentModelOne);
    modelTwoId = (s16) owner->models[0].modelId;
    currentModelTwo = modelTwoId;
    fn_80026B8C(currentModelTwo, 3);
    fn_80026B64(currentModelTwo, 1U);
    fn_800269E8(currentModelTwo, 1U);
    fn_800269E8(currentModelTwo, 0x40000002U);
    fn_80026E44(currentModelTwo);
    fn_80026F9C(currentModelTwo);
    modelThreeId = (s16) owner->models[1].modelId;
    currentModelThree = modelThreeId;
    fn_80026B8C(currentModelThree, 3);
    fn_80026B64(currentModelThree, 1U);
    fn_800269E8(currentModelThree, 1U);
    fn_800269E8(currentModelThree, 0x40000002U);
    fn_80026E44(currentModelThree);
    fn_80026F9C(currentModelThree);
    modelFourId = (s16) owner->models[2].modelId;
    currentModelFour = modelFourId;
    fn_80026B8C(currentModelFour, 3);
    fn_80026B64(currentModelFour, 1U);
    fn_800269E8(currentModelFour, 1U);
    fn_800269E8(currentModelFour, 0x40000002U);
    fn_80026E44(currentModelFour);
    fn_80026F9C(currentModelFour);
    modelFiveId = (s16) owner->models[3].modelId;
    currentModelFive = modelFiveId;
    fn_80026B8C(currentModelFive, 3);
    fn_80026B64(currentModelFive, 1U);
    fn_800269E8(currentModelFive, 1U);
    fn_800269E8(currentModelFive, 0x40000002U);
    fn_80026E44(currentModelFive);
    fn_80026F9C(currentModelFive);
    modelSixId = (s16) owner->models[4].modelId;
    currentModelSix = modelSixId;
    fn_80026B8C(currentModelSix, 3);
    fn_80026B64(currentModelSix, 1U);
    fn_800269E8(currentModelSix, 1U);
    fn_800269E8(currentModelSix, 0x40000002U);
    fn_80026E44(currentModelSix);
    fn_80026F9C(currentModelSix);
    modelSevenId = (s16) owner->models[5].modelId;
    currentModelSeven = modelSevenId;
    fn_80026B8C(currentModelSeven, 3);
    fn_80026B64(currentModelSeven, 1U);
    fn_800269E8(currentModelSeven, 1U);
    fn_800269E8(currentModelSeven, 0x40000002U);
    fn_80026E44(currentModelSeven);
    fn_80026F9C(currentModelSeven);
    owner->position.x = (f32)(sequence * 0x104 - 0x186);
    owner->position.y = 0.0f;
    owner->position.z = - 400.0f;
    {
        s16 modelId = (s16) owner->hookModel.modelId;
        s16 modelId2 = modelId;
        motionId = fn_80030458(modelId2, fn_80008DD0(DATANUM(0x6b0000, 8), 0x10000000, 2));
        fn_8002C95C(modelId2, motionId);
    }
    fn_1_4EB8(owner, 7);
    owner->fn_1_52D8();
    return owner;
}

struct MotionHookNode : Node { ~MotionHookNode(); };

MotionHookNode::~MotionHookNode() {}

void operator delete(void *node);

typedef OwnedModel ReleasedModel;

extern "C" { void *__dt__4NodeFv(void *node, s16 state); }

extern "C" { void fn_1_10308(void *node); }

CallbackListEntry *fn_1_4960(CallbackListEntry *entry, s16 destroyOwner)
{
    if (entry != 0) {
        entry->model6.~ReleasedModel();
        entry->model5.~ReleasedModel();
        entry->model4.~ReleasedModel();
        entry->model3.~ReleasedModel();
        entry->model2.~ReleasedModel();
        entry->model1.~ReleasedModel();
        entry->model0.~ReleasedModel();
        if (entry != 0) {
            __dt__4NodeFv(entry, 0);
        }
        if (destroyOwner > 0) {
            operator delete(entry);
        }
    }
    return entry;
}

/* Releases every callback entry, including its seven associated models. */

struct ListNode {
    ListNode *next;
    ListNode *previous;
};

struct ListEntry : ListNode {
    ListEntry *next_node() { return static_cast<ListEntry *>(next); }
};

struct ListRoot : ListNode {
    ListNode *first_node() { return next; }
    ListNode *end_node() { return this; }
    ListEntry *front() { return static_cast<ListEntry *>(first_node()); }
};

extern "C" { extern ListRoot lbl_1_bss_588; }

extern "C" { extern ListRoot lbl_1_bss_A50; }

extern "C" { void fn_1_A160(ListEntry *, int); }

extern "C" { void fn_1_C8C0(ListEntry *, int); }

extern "C" { void fn_1_C350(ListEntry *); }

struct MotionHookModels;
void fn_1_50C0(MotionHookModels *);

void fn_1_4ABC()
{
    CallbackListEntry *node = lbl_1_bss_C0.front();
    Node *end = lbl_1_bss_C0.end_node();
    for (; node != end; node = node->next_node()) {
        fn_1_50C0(reinterpret_cast<MotionHookModels *>(node));
    }
}

u8 MotionHookModels::fn_1_4B20(s32 sequenceType) {
    if (attributeState == sequenceType) {
        return 1;
    }
    return 0;
}

void fn_1_4B3C(void)
{
    while (!lbl_1_bss_C0.empty()) {
        CallbackListEntry *entry = lbl_1_bss_C0.back();
        fn_1_4960(entry, 1);
    }
}

typedef CallbackListRoot ListCallbackState;

typedef CountedCallback ListCallbackNode;

void fn_1_4D30(ListCallbackNode *);

void fn_1_4D00()
{
    fn_1_58B8(&lbl_1_bss_C0,
                           fn_1_4D30);
}

void fn_1_4D30(CountedCallback *object)
{
    (object->*object->callback)();
    object->fn_1_4D7C();
    object->callbackCount++;
}

void MotionHookModels::fn_1_4D7C() {
    Vec rotation;

    rotation.x = 0.0f;
    rotation.y = 0.0f;
    rotation.z = 0.0f;
    hookModel.fn_1_33ACC(&rotation);
    hookModel.fn_1_33A3C(&position);
}

unsigned long RandomUnsigned();

struct MotionSelection {
    u8 unknown_00[0x0A]; /* No code names this field. */
    s16 firstSelection;
    s16 secondSelection;
    s16 thirdSelection;
    s16 fourthSelection;
    s16 fifthSelection;
    s16 sixthSelection;
};

s16 fn_1_4DEC(const MotionSelection *selection, s32 index)
{
    s16 initialValue = selection->firstSelection;
    s16 result = initialValue;
    switch (index) {
    case 0: { s16 selected = selection->firstSelection; result = selected; break; }
    case 1: { s16 selected = selection->secondSelection; result = selected; break; }
    case 2: { s16 selected = selection->thirdSelection; result = selected; break; }
    case 3: { s16 selected = selection->fourthSelection; result = selected; break; }
    case 4: { s16 selected = selection->fifthSelection; result = selected; break; }
    case 5: { s16 selected = selection->sixthSelection; result = selected; break; }
    }
    return result;
}

extern "C" { char lbl_1_data_11E[14] = "itemhook_R"; }

void fn_1_4EB8(MotionHookModels *owner, s32 selection)
{
    if (owner->attributeState >= 0) {
        s16 previousModelId = owner->selectMotion(owner->attributeState);
        fn_800269E8(previousModelId, (1 << 0));
        fn_800275B4(owner->hookModel, lbl_1_data_11E);
    }

    s16 modelId = owner->selectMotion(selection);
    fn_80026A44(modelId, (1 << 0));
    fn_8002741C(owner->hookModel, lbl_1_data_11E, modelId);
    owner->attributeState = selection;
}

/* Changes the active model hook and randomizes the next hook target. */
void fn_1_50C0(MotionHookModels *owner)
{
    fn_1_4EB8(owner, ((u16)RandomUnsigned() * 6) >> 16);
    owner->fn_1_538C();
}

static inline void changeHook5548(MotionHookModels *owner, s32 selection)
{
    if (owner->attributeState >= 0) {
        s16 previousModelId = owner->selectMotion(owner->attributeState);
        fn_800269E8(previousModelId, (1 << 0));
        fn_800275B4(owner->hookModel, lbl_1_data_11E);
    }

    s16 modelId = owner->selectMotion(selection);
    fn_80026A44(modelId, (1 << 0));
    fn_8002741C(owner->hookModel, lbl_1_data_11E, modelId);
    owner->attributeState = selection;
}

/* Restarts a completed motion and installs its next callback. */

void MotionHookModels::fn_1_52D8()
{
    s16 currentModel;
    s16 modelId;
    modelId = (s16)hookModel.modelId;
    currentModel = modelId;
    fn_800269E8(currentModel, ((1 << 30) | (1 << 1)));
    fn_80026A44(currentModel, ((1 << 30) | (1 << 2)));
    fn_8002D414(currentModel, 0.0f);

    CallbackSetup5548 position = &MotionHookModels::fn_1_5388;
    callback = position;
}

void MotionHookModels::fn_1_5388()
{
}

typedef CallbackSetup5548 MotionCallbackRecord;

inline void MotionHookModels::fn_1_538C()
{
    s16 currentModel;
    MotionCallbackRecord callback;

    currentModel = this->hookModel;
    fn_80026A44(currentModel, ((1 << 30) | (1 << 1)));
    fn_80026A44(currentModel, ((1 << 30) | (1 << 2)));
    fn_8002D414(currentModel, 0.0f);

    callback = &MotionHookModels::fn_1_543C;
    this->callback = callback;
}

void MotionHookModels::fn_1_543C()
{
    s16 modelId = (s16)hookModel.modelId;
    if (fn_8002D75C(modelId))
        complete = 1;
}

void MotionHookModels::fn_1_548C()
{
    s16 currentModel;
    s16 modelId;
    MotionCallbackRecord next;

    complete = 0;
    modelId = (s16)hookModel.modelId;
    currentModel = modelId;
    fn_80026A44(currentModel, ((1 << 30) | (1 << 1)));
    fn_800269E8(currentModel, ((1 << 30) | (1 << 2)));
    fn_8002D414(currentModel, fn_8002D530(currentModel));
    shouldRestart = 0;

    next = &MotionHookModels::fn_1_5548;
    callback = next;
}

void MotionHookModels::fn_1_5548()
{
    if (fn_8002D75C(this->hookModel) != 0 && this->shouldRestart != 0)
    {
        changeHook5548(this, ((u16)RandomUnsigned() * 6) >> 16);
        this->fn_1_538C();
    }
}

void MotionHookModels::fn_1_5800()
{
    s16 currentModel;

    complete = 0;
    currentModel = hookModel;
    fn_80026A44(currentModel, ((1 << 30) | (1 << 1)));
    fn_800269E8(currentModel, ((1 << 30) | (1 << 2)));
    fn_8002D414(currentModel, fn_8002D530(currentModel));

    {
        MotionCallbackRecord next = &MotionHookModels::fn_1_58B4;
        callback = next;
    }
}

void MotionHookModels::fn_1_58B4()
{
}
