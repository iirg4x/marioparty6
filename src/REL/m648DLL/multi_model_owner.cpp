#include "REL/m648DLL/multi_model_owner.h"
#include "REL/framework/frame_time_inline.h"
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
AnimationUpdateOwner::AnimationUpdateOwner() {}

void fn_1_4B3C(void);

AnimationUpdateOwner::~AnimationUpdateOwner() { fn_1_4B3C(); }

void fn_1_4D00(void);

void AnimationUpdateOwner::update()
{
    fn_1_4D00();
}

void fn_1_4ABC(void);
void AnimationUpdateOwner::flagEnabled(int bit)
{
    flags &= ~(1UL << bit);
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

void fn_1_4EB8(MotionHookModels *, s32);

ListRoot<MotionHookModels> lbl_1_bss_C0;

/* Creates the owner and configures its seven linked motion models. */
MotionHookModels::MotionHookModels(s32 sequence, s32 mode)
    : hookModel(DATANUM(0x6b0000, 7)),
      itemModel0(DATANUM(0x6b0000, 22)),
      itemModel1(DATANUM(0x6b0000, 23)),
      itemModel2(DATANUM(0x6b0000, 24)),
      itemModel3(DATANUM(0x6b0000, 25)),
      itemModel4(DATANUM(0x6b0000, 26)),
      itemModel5(DATANUM(0x6b0000, 27)),
      complete(0),
      mode(mode),
      sequence(sequence),
      callbackCount(0)
{
    s16 currentModel;
    s16 motionId;

    insert4(&lbl_1_bss_C0);
    fn_1_4454(hookModel, 1);
    fn_1_4454(itemModel0, 0);
    fn_1_4454(itemModel1, 0);
    fn_1_4454(itemModel2, 0);
    fn_1_4454(itemModel3, 0);
    fn_1_4454(itemModel4, 0);
    fn_1_4454(itemModel5, 0);
    fn_1_44E0(&position, sequence);
    currentModel = hookModel;
    motionId = fn_80030458(currentModel, fn_80008DD0(DATANUM(0x6b0000, 8), 0x10000000, 2));
    fn_8002C95C(currentModel, motionId);
    fn_1_4EB8(this, 7);
    fn_1_52D8();
}

MotionHookModels::~MotionHookModels() {}

void fn_1_50C0(MotionHookModels *);

void fn_1_4ABC()
{
    MotionHookModels *node = lbl_1_bss_C0.front();
    Node *end = lbl_1_bss_C0.end_node();
    for (; node != end; node = node->next()) {
        fn_1_50C0(node);
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
        delete lbl_1_bss_C0.back();
    }
}

void fn_1_4D30(MotionHookModels *);

void fn_1_4D00()
{
    lbl_1_bss_C0.visit(fn_1_4D30);
}

void fn_1_4D30(MotionHookModels *object)
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
    hookModel.setRotation(&rotation);
    hookModel.setPosition(&position);
}

unsigned long RandomUnsigned();

s16 MotionHookModels::fn_1_4DEC(s32 selection) const
{
    s16 modelId = itemModel0;
    switch (selection) {
    case 0: modelId = itemModel0; break;
    case 1: modelId = itemModel1; break;
    case 2: modelId = itemModel2; break;
    case 3: modelId = itemModel3; break;
    case 4: modelId = itemModel4; break;
    case 5: modelId = itemModel5; break;
    }
    return modelId;
}

extern "C" { char lbl_1_data_11E[14] = "itemhook_R"; }

void fn_1_4EB8(MotionHookModels *owner, s32 selection)
{
    if (owner->attributeState >= 0) {
        s16 previousModelId = owner->fn_1_4DEC(owner->attributeState);
        fn_800269E8(previousModelId, (1 << 0));
        fn_800275B4(owner->hookModel, lbl_1_data_11E);
    }

    s16 modelId = owner->fn_1_4DEC(selection);
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

    Callback position = &MotionHookModels::fn_1_5388;
    callback = position;
}

void MotionHookModels::fn_1_5388()
{
}

inline void MotionHookModels::fn_1_538C()
{
    s16 currentModel;
    Callback callback;

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
    Callback next;

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
        fn_1_4EB8(this, ((u16)RandomUnsigned() * 6) >> 16);
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
        Callback next = &MotionHookModels::fn_1_58B4;
        callback = next;
    }
}

void MotionHookModels::fn_1_58B4()
{
}
