#include "REL/m605DLL/model_position_engine.h"
#include "game/hu3d_query.h"
extern "C" {
#include "game/audio.h"
}
#include "REL/framework/sound.h"
#include "REL/framework/model.h"
#include "REL/framework/random.h"
#include <memory>
#include <vector>
#include "datadir_enum.h"
#include "REL/m605DLL/module_globals.h"
#include "REL/m605DLL/animation_owner.h"
#include "REL/m605DLL/scene_controller.h"
#include "REL/framework/scene_shadow_object.h"
#include "REL/m605DLL/timed_model_object.h"
#include <stdio.h>

#include "REL/m605DLL/scene_stage.h"

#include "REL/m605DLL/particle_effect_object.h"

// Manages scenery models, bats, hook spheres and timed scene events.

// Lists the nodes hidden on the stage model.
char *lbl_1_data_198[] = {
    "D_ue1", "D_ue2", "iseki01b", "iseki02b", "iseki3b", "iseki14b", "ookabe", 0
};

// Loads a bat model and sets its initial attributes.
OwnedModel *fn_1_3AEC()
{
    OwnedModel *object = new OwnedModel(DATANUM(0x400000, 19));
    object->setAttribute(0x40000001);
    return object;
}

// Loads the course models, bat attachments, shadows and timed scenery.
SceneStage::SceneStage()
    : iseki(DATANUM(0x400000, 0)), taimatu(DATANUM(0x400000, 21)), i8akari(DATANUM(0x400000, 22)),
      hasi(DATANUM(0x400000, 8)), prapara(DATANUM(0x400000, 9)), ita(DATANUM(0x400000, 10)),
      attachmentModel(DATANUM(0x400000, 20))
{
    Vec lightPosition;
    Vec lightDirection;
    GXColor lightColor;
    Vec position;
    s32 index;
    char **name;
    s32 timedModelIndex;

    iseki.setShadowMap();
    iseki.setLightsDynamic(false);
    fn_800297D4(0, &lightPosition, &lightDirection, &lightColor);
    shadow.reset(new SceneShadow(1, &lightDirection, 5.0f, 100));
    for (name = lbl_1_data_198; *name != 0;) {
        iseki.resetObjectShadowMap(*name++);
    }
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = -1200.0f;
    prapara.setPosition(&position);
    hasi.setShadowMap();
    hasi.setAttribute(0x40000080);
    hasi.setAttribute(0x40000002);
    hasi.resetObjectShadowMap("Ati1");
    ita.setShadowMap();
    ita.setAttribute(0x40000002);
    taimatu.setAttribute(0x40000001);
    i8akari.setAttribute(0x40000001);
    i8akari.setAttribute(0x400000);
    i8akari.setLayer(5);
    for (index = 0; index < 9; index++) {
        attachments[index] = fn_1_3AEC();
        attachments[index]->setAttribute(0x40000002);
    }
    attachmentModel.setHook("bat_N", *attachments[0]);
    attachmentModel.setHook("bat_N2", *attachments[1]);
    attachmentModel.setHook("bat_N3", *attachments[2]);
    attachmentModel.setHook("bat_N4", *attachments[3]);
    attachmentModel.setHook("bat_N5", *attachments[4]);
    attachmentModel.setHook("bat_N6", *attachments[5]);
    attachmentModel.setHook("bat_N7", *attachments[6]);
    attachmentModel.setHook("bat_N8", *attachments[7]);
    attachmentModel.setHook("bat_N9", *attachments[8]);
    attachmentModel.setAttribute(0x40000002);
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 2770.0f;
    new TimedModelObject(DATANUM(0x400000, 6), position);
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 17670.0f;
    new TimedModelObject(DATANUM(0x400000, 7), position);
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 8780.0f;
    new TimedModelObject(DATANUM(0x400000, 5), position);
    {
        f64 horizontalOffset = RandomFloat(-300.0f, 300.0f);
        position.x = horizontalOffset;
        position.y = 0.0f;
        position.z = 4200.0f;
        new TimedModelObject(DATANUM(0x400000, 2), position);
    }
    {
        f64 horizontalOffset = RandomFloat(-300.0f, 300.0f);
        position.x = horizontalOffset;
        position.y = 0.0f;
        position.z = 11000.0f;
        new TimedModelObject(DATANUM(0x400000, 2), position);
    }
    {
        f64 horizontalOffset = RandomFloat(-300.0f, 300.0f);
        position.x = horizontalOffset;
        position.y = 0.0f;
        position.z = 12000.0f;
        new TimedModelObject(DATANUM(0x400000, 3), position);
    }
    {
        f64 horizontalOffset = RandomFloat(-300.0f, 300.0f);
        position.x = horizontalOffset;
        position.y = 0.0f;
        position.z = 15500.0f;
        new TimedModelObject(DATANUM(0x400000, 3), position);
    }
    {
        f64 horizontalOffset = RandomFloat(-300.0f, 300.0f);
        position.x = horizontalOffset;
        position.y = 0.0f;
        position.z = 20000.0f;
        new TimedModelObject(DATANUM(0x400000, 3), position);
    }
    {
        f64 horizontalOffset = RandomFloat(-300.0f, 300.0f);
        position.x = horizontalOffset;
        position.y = 0.0f;
        position.z = 6775.0f;
        new TimedModelObject(DATANUM(0x400000, 4), position);
    }
    {
        f64 horizontalOffset = RandomFloat(-300.0f, 300.0f);
        position.x = horizontalOffset;
        position.y = 0.0f;
        position.z = 10284.0f;
        new TimedModelObject(DATANUM(0x400000, 4), position);
    }
    {
        f64 horizontalOffset = RandomFloat(-300.0f, 300.0f);
        position.x = horizontalOffset;
        position.y = 0.0f;
        position.z = 10508.0f;
        new TimedModelObject(DATANUM(0x400000, 4), position);
    }
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 0.0f;
    for (timedModelIndex = 0; timedModelIndex < 8; timedModelIndex++) {
        new TimedModelObject(timedModelIndex + DATANUM(0x400000, 11), position);
    }
}

// Advances course effects and submits the final hook spheres for shadows.
void SceneStage::update()
{
    fn_1_4AE8();
    static s32 attachmentStep = 0;

    switch (attachmentStep) {
    case 0:
        if (lbl_1_bss_34 > 31200.0f) {
            s32 motionIndex;

            attachmentModel.resetAttribute(0x40000002);
            for (motionIndex = 0; motionIndex < 9; motionIndex++) {
                attachments[motionIndex]->resetAttribute(0x40000002);
            }
            attachmentStep++;
            soundPlay(0x642);
        }
        break;
    case 1:
        if (lbl_1_bss_34 > 37500.0f) {
            s32 motionIndex;

            attachmentModel.resetAttribute(0x40000002);
            attachmentModel.setMotionTime(0.0f);
            for (motionIndex = 0; motionIndex < 9; motionIndex++) {
                attachments[motionIndex]->resetAttribute(0x40000002);
                attachments[motionIndex]->setMotionTime(0.0f);
            }
            attachmentStep++;
            soundPlay(0x642);
        }
        break;
    case 2:
        if (lbl_1_bss_34 > 45400.0f) {
            s32 motionIndex;

            attachmentModel.resetAttribute(0x40000002);
            attachmentModel.setMotionTime(0.0f);
            for (motionIndex = 0; motionIndex < 9; motionIndex++) {
                attachments[motionIndex]->resetAttribute(0x40000002);
                attachments[motionIndex]->setMotionTime(0.0f);
            }
            attachmentStep++;
            soundPlay(0x642);
        }
        break;
    }

    if (lbl_1_bss_38 > 49286.0f) {
        s32 hookIndex;

        std::vector<Sphere *> sequence;
        for (hookIndex = 0; hookIndex < 4; hookIndex++) {
            sequence.push_back(&hooks[hookIndex]);
        }
        shadow->appendSpheres(&sequence, -1);
    }
}

// Tests whether a height lies below the course floor at the supplied depth.
bool fn_1_49D0(f32 height, f32 depth)
{
    return height < fn_1_4A0C(depth);
}

// Returns the floor height across the descending course section.
f32 fn_1_4A0C(f32 position)
{
    f32 progress;

    if (position < 35700.0f) {
        return 0.0f;
    }
    if (position <= 36900.0f) {
        progress = (position - 35700.0f) / 1200.0f;
        return -700.0f * progress;
    }
    return -700.0f;
}

// Reports whether the depth is within the descending course section.
s32 fn_1_4AA4(f32 depth)
{
    if (depth < 35700.0f) {
        return 0;
    }
    if (depth <= 36900.0f) {
        return 1;
    }
    return 0;
}

// Advances the collapsing scenery, camera shake and associated sound effects.
void SceneStage::fn_1_4AE8()
{
    static u8 bridgeCollapsing = 0;
    static s32 bridgeSound;
    SceneController *transitionTarget = &SceneController::getChecked();
    if (lbl_1_bss_34 > 22268.58f && lbl_1_bss_34 <= 27962.19f) {
        f32 amount = lbl_1_bss_34 - 22268.58f;
        amount /= 27962.19f - 22268.58f;
        hasi.resetAttribute(0x40000080);
        hasi.resetAttribute(0x40000002);
        hasi.setMotionTime(500.0f * amount);
        fn_1_23DC(transitionTarget, 6.0f, 0.0f);
        bridgeCollapsing = 1;
        static u8 bridgeSoundPending = 1;
        if (bridgeSoundPending != 0) {
            bridgeSoundPending = 0;
            bridgeSound = soundPlay(0x643);
        }
    }
    if (bridgeCollapsing != 0 && lbl_1_bss_34 > 27962.19f) {
        fn_1_23DC(transitionTarget, 0.0f, 0.0f);
        bridgeCollapsing = 0;
        HuAudFXFadeOut(bridgeSound, 200);
    }
    if (lbl_1_bss_34 > 47500.0f) {
        ita.resetAttribute(0x40000002);
        static u8 plankSoundPending = 1;
        if (plankSoundPending != 0) {
            plankSoundPending = 0;
            soundPlay(0x641);
        }
    }
}

// Reads the model identifier through its conversion operator.
static inline s32 modelIdOf(const Model &model)
{
    return model;
}

// Creates particle effects at model hooks for the initial scene step.
void SceneStage::flagEnabled(int step)
{
    s32 index;
    char hookName[16];
    Vec position;
    AnimationOwner *animations = &AnimationOwner::getChecked();
    Model *model;
    Sphere *hook;
    Vec *hookPosition;

    switch (step) {
    case 0:
        for (index = 0; index < 4; index++) {
            sprintf(hookName, "Hook%d", index + 1);
            Hu3DModelObjPosGet(taimatu, hookName, &position);
            model = fn_1_690(animations);
            fn_80026760(modelIdOf(*model), position.x, position.y, position.z);
            hook = &hooks[index];
            hookPosition = &hook->center;
            *hookPosition = position;
            hooks[index].setRadius(100.0f);
        }
        flags &= ~(1UL << step);
        break;
    case 1:
        flags &= ~(1UL << step);
        break;
    case 2:
        flags &= ~(1UL << step);
        break;
    case 3:
        flags &= ~(1UL << step);
        break;
    case 4:
        flags &= ~(1UL << step);
        break;
    }
}

// Destroys the course models, attachments and shadow owner.
SceneStage::~SceneStage() {}
