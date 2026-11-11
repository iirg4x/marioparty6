#include "REL/framework/minigame_core.h"
#include "REL/framework/sound.h"
#include "REL/framework/phase_durations.h"
#include "REL/m622DLL/random.h"
#include "dolphin/types.h"
#include "REL/m622DLL/scene_owner.h"
#include "REL/m622DLL/model_setup_owner.h"
extern "C" {
int HuAudFXPlay(int seId);
#include "game/hu3d.h"
#include "game/charman.h"
#include "game/data.h"
}


extern "C" void fn_1_264(void *object);

extern "C" void *fn_1_6C(void *object, s16 destroy);

struct ActionTable {
    s32 values[50];
};

extern "C" {
extern const ActionTable lbl_1_rodata_30 = {{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3,
    3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
    0, 1, 2, 3, 4, 0, 1, 2, 3, 4
}};
}






extern "C" { extern s16 lbl_1_bss_B0; }

extern "C" { extern s16 lbl_1_bss_B2; }

/* Initializes the scene models and builds a shuffled action table for each phase. */
SceneOwnerB9C::SceneOwnerB9C()
    : model0(DATANUM(0x510000, 4)),
      model1(DATANUM(0x510000, 5)),
      model2(DATANUM(0x510000, 6)),
      model3(DATANUM(0x510000, 9)),
      model4(DATANUM(0x510000, 7)),
      model5(DATANUM(0x510000, 8)),
      model6(DATANUM(0x510000, 10)),
      model7(DATANUM(0x510000, 11)),
      model8(DATANUM(0x510000, 12)),
      model9(DATANUM(0x510000, 15)),
      model10(DATANUM(0x510000, 16)),
      model11(DATANUM(0x510000, 1))
{
    updateCount = 0;
    phaseStarted = 0;
    sceneState = 0;
    delayFrames = 0x2D;
    completedSegments = 0;
    segmentPosition = 0.0f;
    sceneIndex = 0;
    unknown368 = 0;
    soundActive = 0;

    model0.setAttribute(1);
    model1.setAttribute(1);
    model2.setAttribute(1);
    model3.setAttribute(1);
    model6.setAttribute(1);
    model4.setAttribute(1);
    model5.setAttribute(1);
    model7.setAttribute(1);
    model8.setAttribute(1);
    model9.setAttribute(1);
    model10.setAttribute(1);

    fn_80026A44(model11, 1);
    fn_80026760(model11,
                     pathPosition = 0.0f,
                     0.0f, 0.0f);
    fn_80026B8C(model11, 2);

    s16 motionModelId = model7;
    lbl_1_bss_B0 = fn_80030458(
        motionModelId, fn_80008DD0(DATANUM(0x510000, 13), 0x10000000, 2));
    lbl_1_bss_B2 = fn_80030458(
        motionModelId, fn_80008DD0(DATANUM(0x510000, 14), 0x10000000, 2));

    ActionTable initialActions = lbl_1_rodata_30;
    s32 initialAction = RandomUnsigned() & 3;

    for (s32 phase = 0; phase < 4; phase++) {
        for (s32 index = 0; index < 50; index++) {
            phases[phase].selectedAction[index] = initialActions.values[index];
        }

        for (s32 index = 0; index < 24; index++) {
            s32 otherIndex = (s32)(((u16)RandomUnsigned() * 0x18) >> 16);
            s32 value = phases[phase].selectedAction[index];
            s32 otherValue = phases[phase].selectedAction[otherIndex];
            phases[phase].selectedAction[index] = otherValue;
            phases[phase].selectedAction[otherIndex] = value;
        }
        for (s32 index = 24; index < 50; index++) {
            s32 otherIndex = (s32)(((u16)RandomUnsigned() * 0x18) >> 16);
            s32 value = phases[phase].selectedAction[index];
            s32 otherValue = phases[phase].selectedAction[otherIndex + 24];
            phases[phase].selectedAction[index] = otherValue;
            phases[phase].selectedAction[otherIndex + 24] = value;
        }

        for (s32 index = 0; index < 24; index++) {
            if (phases[phase].selectedAction[index] == initialAction) {
                s32 firstValue = phases[phase].selectedAction[0];
                phases[phase].selectedAction[0] = initialAction;
                phases[phase].selectedAction[index] = firstValue;
                break;
            }
        }
    }
}



void operator delete(void *);


extern "C" { extern f32 lbl_1_bss_AC; }

extern "C" { extern s32 lbl_1_bss_B4; }















void fn_1_1CC4(void);

void fn_1_25E8(void);

void fn_1_2BE8(void);

void fn_1_3460(void);

void fn_1_469C(void);

void fn_1_3C60(void);

SceneOwnerB9C::~SceneOwnerB9C()
{
    ModelEntry::fn_1_1BD8();
    ModelPairOwner::fn_1_24D8();
    ModelCameraPlacement::fn_1_2AFC();
    ModelPulseOwner::fn_1_3374();
    MovingPair::fn_1_458C();
    FourModelOwner::fn_1_3B08();
}

/* Builds the next scene group and attaches its selected model set. */
void SceneOwnerB9C::fn_1_E00()
{
    if (updateCount < 4) {
        s32 phase = updateCount;
        Vec position = {-730.0f, 60.0f, 0.0f};
        position.z = (f32)((phase * 0x122) - 0x24E);

        ModelEntry *scene = new ModelEntry(model0,
                                  &position, phase);

        new ModelPairOwner(model1,
                                model2,
                                &position, (u32)phase);

        s32 selectedAction = phases[phase].selectedAction[sceneIndex];
        ModelCameraPlacement *placement = 0;
        ModelPulseOwner *pulse = 0;
        MovingPair *pair = 0;
        FourModelOwner *fourModels = 0;
        switch (selectedAction) {
        case 0:
            placement = new ModelCameraPlacement(model3,
                                     &position, phase);
            break;
        case 1:
            pulse = new ModelPulseOwner(model6,
                                     &position, phase);
            break;
        case 2:
            pair = new MovingPair(model4,
                                     model5,
                                     &position, phase);
            break;
        case 3:
            fourModels = new FourModelOwner(model7,
                                     model8, model9,
                                     model10,
                                     &position, phase);
            break;
        }
        scene->fn_1_2368(placement, pulse, pair, fourModels);
        updateCount++;
        if (updateCount == 4) {
            sceneIndex++;
        }
    }
}

/* Advances the scene phases and moves its model group along the stage path. */
void SceneOwnerB9C::fn_1_1058()
{

    if (phaseStarted == 0) {
        phaseStarted = 1;
        updateCount = 0;
    }
    if (sceneIndex < 0x18) {
        fn_1_E00();
    }
    if ((delayFrames = delayFrames - 1) > 0) {
        lbl_1_bss_AC = 0.0f;
        return;
    }

    lbl_1_bss_AC = 8.5f;
    delayFrames = 0;
    segmentPosition += lbl_1_bss_AC;
    if (segmentPosition >= 235.0f) {
        lbl_1_bss_AC = 235.0f - (segmentPosition - lbl_1_bss_AC);
        segmentPosition = 0.0f;
        switch (completedSegments) {
        case 0:
            delayFrames = 0xB4;
            break;
        default:
            delayFrames = 0x2D;
            break;
        }
        completedSegments++;
        phaseStarted = 0;
    }
    if ((pathPosition += lbl_1_bss_AC) > 200.0f) {
        pathPosition -= 200.0f;
    }
    fn_80026760(model11,
                    pathPosition, 0.0f, 0.0f);
}

void SceneOwnerB9C::update()
{
    lbl_1_bss_B4 = (lbl_1_bss_B4 + 1) & 3;
    fn_1_1058();

    if (lbl_1_bss_AC > 0.0f) {
        ModelSetupOwner::getChecked().fn_1_658C();
        if (soundActive == 0) {
            HuAudFXPlay(MSM_SE_MG622_SCENERY_SCROLL_START);
        }
        soundActive = 1;
    } else {
        ModelSetupOwner::getChecked().fn_1_65F8();
        soundActive = 0;
    }

    fn_1_1CC4();
    fn_1_25E8();
    fn_1_2BE8();
    fn_1_3460();
    fn_1_469C();
    fn_1_3C60();
}

void SceneOwnerB9C::flagEnabled(int request)
{
    flags &= ~(1U << request);
    switch (request) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        break;
    }
}

void fn_1_1978(s16 modelId, const Vec *position, u8 mode)
{
    f32 value = position->x;

    if (value > 0.0f && mode == 0) {
        fn_800269E8(modelId, (1 << 0));
        return;
    }

    if ((value > (-598.365f) && value < (-371.511f)) ||
        value > 160.0f) {
        fn_80026A44(modelId, (1 << 0));
        if (value > 0.0f) {
            fn_80026B8C(modelId, 2);
        } else {
            fn_80026B8C(modelId, 0);
        }
    } else {
        fn_800269E8(modelId, (1 << 0));
    }
}

extern "C" ListRoot<ModelEntry> lbl_1_bss_40;

ModelEntry::ModelEntry(const OwnedModel &modelData,
                           const Vec *position, s32 requestId)
    : active(0),
      requestId(requestId),
      frameData(*position),
      model(modelData)
{
    Callback initialData;
    s16 modelIdForCalls;

    insert4(&lbl_1_bss_40);
    modelIdForCalls = model;
    fn_80026B8C(modelIdForCalls, 2);
    fn_80026A44(modelIdForCalls, 1);
    fn_800269E8(modelIdForCalls, 0x4000);
    model.setPosition(&frameData);
    initialData = &ModelEntry::fn_1_1CF4;
    this->initialData = initialData;
}

void ModelEntry::fn_1_1BD8()
{
    while (!lbl_1_bss_40.empty()) {
        delete lbl_1_bss_40.back();
    }
}

void fn_1_2330(ModelEntry *);
void fn_1_1CC4()
{
    lbl_1_bss_40.visit(fn_1_2330);
}


extern "C" { extern f32 lbl_1_bss_AC;  extern s32 lbl_1_bss_B4; }
void ModelEntry::fn_1_1CF4()
{
    frameData.x += lbl_1_bss_AC;
    model.setPosition(&frameData);
    if (frameData.x > 550.0f && lbl_1_bss_B4 == requestId) {
        delete this;
    }
}

extern "C" {  }
ModelEntry *fn_1_1DB8(s32 ownerId)
{
    ModelEntry *current = lbl_1_bss_40.front();
    Node *sentinel = lbl_1_bss_40.end_node();
    while (current != sentinel) {
        if (current->frameData.x > 60.0f &&
            current->frameData.x < 230.0f &&
            current->requestId == ownerId && current->active == 0) {
            return current;
        }
        current = current->next();
    }
    return 0;
}

s32 fn_1_1E68(s32 requestId)
{
    ModelEntry *current = lbl_1_bss_40.front();
    Node *sentinel = lbl_1_bss_40.end_node();
    while (current != sentinel) {
        if (current->frameData.x > 160.0f &&
            current->frameData.x < 211.0f &&
            current->requestId == requestId) {
            if (current->model0 != 0) return 0;
            if (current->model1 != 0) return 1;
            if (current->model2 != 0) return 2;
            if (current->model3 != 0) return 3;
            return 4;
        }
        current = current->next();
    }
    return -1;
}

bool ModelEntry::fn_1_1F5C()
{
    return lbl_1_bss_AC > 0.0f;
}


s32 ModelEntry::fn_1_1F84(s16 characterModelId, s16 itemId)
{
    static const Vec initialModelData = {0.0f, -10.0f, 0.0f};
    char *hookId;
    if (frameData.x > 190.0f && frameData.x < 230.0f) {
        active = 1;
        this->characterModelId = characterModelId;
        itemHookId = itemId;
        frame = 0;
        hookId = fn_8005C0F0(itemHookId, 1, 0);
        fn_8002741C(this->characterModelId, hookId, model);
        Vec modelData = initialModelData;
        model.setPosition(&modelData);
        Callback initialData = &ModelEntry::fn_1_217C;
        this->initialData = initialData;
        soundPlayAt(1, MSM_SE_MG622_ITEM_ATTACH, &frameData);
        return 1;
    }
    return 0;
}

s32 ModelEntry::fn_1_20C0()
{
    if (model0 != 0) { model0->fn_1_2E08(); return 0; }
    if (model1 != 0) { model1->fn_1_3678(); return 1; }
    if (model2 != 0) {
        soundPlayAt(1, MSM_SE_MG622_ENTRY_MODEL_ACTION, &frameData);
        model2->fn_1_49CC(); return 2;
    }
    if (model3 != 0) {
        soundPlayAt(1, MSM_SE_GUIDE_OBJECT_CATCH, &frameData);
        model3->fn_1_3FE0(); return 3;
    }
    return 4;
}

void ModelEntry::fn_1_217C()
{
    char *hookObject;
    f32 transparency;
    frame += 1;
    transparency = 1.0f;
    if (frame > 20) {
        transparency = 1.0f - ((f32)(frame - 20) / 10.0f);
    }
    if (transparency < 0.0f) {
        Callback callbackState;
        hookObject = fn_8005C0F0(itemHookId, 1, 0);
        fn_800275B4(characterModelId, hookObject);
        transparency = 0.0f;
        callbackState = &ModelEntry::fn_1_22C0;
        initialData = callbackState;
    }
    fn_80026CBC(model, transparency);
}

void ModelEntry::fn_1_22C0()
{
    delete this;
}

void fn_1_2330(ModelEntry *self)
{
    (self->*self->initialData)();
}

void ModelEntry::fn_1_2368(ModelCameraPlacement *cameraPlacement,
                               ModelPulseOwner *pulse, MovingPair *movingPair,
                               FourModelOwner *fourModels)
{
    model0 = cameraPlacement;
    model1 = pulse;
    model2 = movingPair;
    model3 = fourModels;
}


extern "C" ListRoot<ModelPairOwner> lbl_1_bss_54;

ModelPairOwner::ModelPairOwner(const OwnedModel &firstSource,
                         const OwnedModel &secondSource, const Vec *position,
                         u32 requestId)
    : requestId(requestId),
      vector(*position),
      firstModel(firstSource),
      secondModel(secondSource)
{
    s16 modelIdForCalls;

    insert4(&lbl_1_bss_54);

    modelIdForCalls = firstModel;
    fn_80026B8C(modelIdForCalls, 2);
    fn_80026A44(modelIdForCalls, 1);
    fn_800269E8(modelIdForCalls, 0x4000);
    firstModel.setPosition(&vector);

    modelIdForCalls = secondModel;
    fn_80026B8C(modelIdForCalls, 1);
    fn_80026A44(modelIdForCalls, 1);
    secondModel.setPosition(&vector);
}

void ModelPairOwner::fn_1_24D8()
{
    while (!lbl_1_bss_54.empty()) {
        delete lbl_1_bss_54.back();
    }
}

void fn_1_2774(ModelPairOwner *);
void fn_1_25E8()
{
    lbl_1_bss_54.visit(fn_1_2774);
}

extern "C" {
extern f32 lbl_1_bss_AC;
extern s32 lbl_1_bss_B4;

}

void ModelPairOwner::fn_1_2618()
{
    vector.x += lbl_1_bss_AC;
    firstModel.setPosition(&vector);
    s16 modelId = secondModel;
    if (vector.x > (-648.0f) && vector.x < (-321.0f)) {
        fn_80026A44(modelId, 1);
        secondModel.setPosition(&vector);
    } else {
        fn_800269E8(modelId, 1);
    }
    if (vector.x > 550.0f && lbl_1_bss_B4 == requestId) {
        delete this;
    }
}

void fn_1_2774(ModelPairOwner *self)
{
    self->fn_1_2618();
}



extern "C" ListRoot<ModelCameraPlacement> lbl_1_bss_68;

ModelCameraPlacement::ModelCameraPlacement(const OwnedModel &modelData,
    const Vec *cameraValues, s32 mode)
    : active(0),
      mode(mode),
      cameraPosition(*cameraValues),
      modelScale(0.4f),
      modelOffset(0.0f),
      model(modelData)
{
    s16 modelIdForCalls;
    insert4(&lbl_1_bss_68);
    modelIdForCalls = model;
    fn_800268B0(modelIdForCalls, modelScale, modelScale, modelScale);
    fn_1_1978(modelIdForCalls, &cameraPosition, 0);
    fn_800269E8(modelIdForCalls, 0x4000);
    Callback initialValues = &ModelCameraPlacement::fn_1_2C18;
    this->initialValues = initialValues;
}

void ModelCameraPlacement::fn_1_2AFC()
{
    while (!lbl_1_bss_68.empty()) {
        delete lbl_1_bss_68.back();
    }
}

void fn_1_3120(ModelCameraPlacement *);
void fn_1_2BE8()
{
    lbl_1_bss_68.visit(fn_1_3120);
}

void ModelCameraPlacement::fn_1_2C18()
{
    Vec position;
    cameraPosition.x += lbl_1_bss_AC;
    position.x = cameraPosition.x;
    position.y = cameraPosition.y;
    position.z = cameraPosition.z;
    position.y += 30.0f + modelOffset;
    model.setPosition(&position);
    fn_1_1978(model, &cameraPosition, active);
    if (cameraPosition.x > 550.0f && lbl_1_bss_B4 == mode) {
        delete this;
    }
}

void ModelCameraPlacement::fn_1_2E08()
{
    s16 modelIdForCalls = model;
    fn_80026A44(modelIdForCalls, ((1 << 30) | (1 << 1)));
    fn_800269E8(modelIdForCalls, ((1 << 30) | (1 << 0)));
    active = 1;
    Callback initialValues = &ModelCameraPlacement::fn_1_2EAC;
    this->initialValues = initialValues;
}

void ModelCameraPlacement::fn_1_2EAC()
{
    if ((modelScale += 0.1f) > 1.0f) {
        modelScale = 1.0f;
    }
    if ((modelOffset += 2.5f) > 25.0f) {
        modelOffset = 25.0f;
    }
    fn_800268B0(model, modelScale, modelScale, modelScale);
    fn_1_2C18();
}

void fn_1_3120(ModelCameraPlacement *self)
{
    (self->*self->initialValues)();
}


extern "C" {  }

extern "C" ListRoot<ModelPulseOwner> lbl_1_bss_7C;

ModelPulseOwner::ModelPulseOwner(const OwnedModel &modelData,
    const Vec *position, s32 requestId)
    : initialized(0),
      requestId(requestId),
      frameData(*position),
      scale(0.4f),
      model(modelData)
{
    CallbackState callback;
    s16 modelId;
    insert4(&lbl_1_bss_7C);
    modelId = model;
    fn_800268B0(modelId, scale, scale, scale);
    fn_1_1978(modelId, &frameData, 0);
    fn_800269E8(modelId, 0x4000);
    callback = &ModelPulseOwner::fn_1_3490;
    this->callback = callback;
}

void ModelPulseOwner::fn_1_3374()
{
    while (!lbl_1_bss_7C.empty()) {
        delete lbl_1_bss_7C.back();
    }
}

void fn_1_38F4(ModelPulseOwner *);
void fn_1_3460()
{
    lbl_1_bss_7C.visit(fn_1_38F4);
}

void ModelPulseOwner::fn_1_3490()
{
    Vec position;
    frameData.x += lbl_1_bss_AC;
    position.x = frameData.x;
    position.y = frameData.y;
    position.z = frameData.z;
    position.y += 4.3f;
    model.setPosition(&position);
    fn_1_1978(model, &frameData, initialized);
    if (frameData.x > 550.0f && lbl_1_bss_B4 == requestId) {
        delete this;
    }
}

void ModelPulseOwner::fn_1_3678()
{
    initialized = 1;
    CallbackState callback = &ModelPulseOwner::fn_1_36C4;
    this->callback = callback;
}

void ModelPulseOwner::fn_1_36C4()
{
    if ((scale += 0.1f) > 1.0f) {
        scale = 1.0f;
    }
    fn_800268B0(model, scale, scale, scale);
    fn_1_3490();
}

void fn_1_38F4(ModelPulseOwner *self)
{
    (self->*self->callback)();
}


extern "C" ListRoot<FourModelOwner> lbl_1_bss_A4;

FourModelOwner::FourModelOwner(
    const OwnedModel &firstModelData,
    const OwnedModel &secondModelData,
    const OwnedModel &thirdModelData,
    const OwnedModel &fourthModelData,
    const Vec *frameData,
    s32 requestId)
    : initialized(0),
      requestId(requestId),
      frameData(*frameData),
      scale(0.4f),
      model0(firstModelData),
      model1(secondModelData),
      model2(thirdModelData),
      model3(fourthModelData)
{
    Vec initialData;
    FourModelOwner::Callback callback;
    s16 modelIdCopy;

    insert4(&lbl_1_bss_A4);

    modelIdCopy = model0;
    fn_800268B0(modelIdCopy, this->scale, this->scale,
                      this->scale);
    fn_800269E8(modelIdCopy, 0x4000);
    modelIdCopy = model1;
    fn_800268B0(modelIdCopy, this->scale, this->scale,
                      this->scale);
    this->model0.setAttribute(1);
    this->model1.setAttribute(1);
    this->model2.setAttribute(1);
    this->model3.setAttribute(1);
    callback = &FourModelOwner::fn_1_3C90;
    this->callback = callback;
}

void FourModelOwner::fn_1_3B08()
{
    while (!lbl_1_bss_A4.empty()) { delete lbl_1_bss_A4.back(); }
}
void fn_1_4398(FourModelOwner *);
void fn_1_3C60()
{
    lbl_1_bss_A4.visit(fn_1_4398);
}

void FourModelOwner::fn_1_3C90()
{
    Vec position;
    frameData.x += lbl_1_bss_AC;
    position.x = frameData.x;
    position.y = frameData.y;
    position.z = frameData.z;
    position.y += 4.3f;
    model0.setPosition(&position);
    model1.setPosition(&position);
    if (frameData.x > 0.0f) {
        fn_1_1978(model0, &frameData, initialized);
        model1.setAttribute(1);
    } else {
        fn_1_1978(model1, &frameData, 0);
        model0.setAttribute(1);
    }
    if (frameData.x > 550.0f && lbl_1_bss_B4 == requestId) {
        delete this;
    }
}

void FourModelOwner::fn_1_3FE0()
{
    s16 currentModel;
    Callback callback;
    state = 0;
    currentModel = model0;
    fn_8002C95C(currentModel, lbl_1_bss_B0);
    fn_80026A44(currentModel, ((1 << 30) | (1 << 1)));
    fn_800269E8(currentModel, ((1 << 30) | (1 << 0)));
    initialized = 1;
    callback = &FourModelOwner::fn_1_40A0;
    this->callback = callback;
}

extern "C" char lbl_1_data_BC[12];

void FourModelOwner::fn_1_40A0()
{
    s16 attributeModelIdForCalls;
    s16 transitionModelIdForCalls;
    switch (state) {
    case 0:
        if ((scale += 0.1f) > 1.0f) {
            scale = 1.0f;
            unknown34 = 0;
            fn_8002C95C(model0, lbl_1_bss_B2);
            attributeModelIdForCalls = model3;
            fn_80026A44(attributeModelIdForCalls, ((1 << 30) | (1 << 1)));
            fn_800269E8(attributeModelIdForCalls, ((1 << 30) | (1 << 0)));
            fn_80026A44(attributeModelIdForCalls, 1);
            fn_8002741C(model0, lbl_1_data_BC, attributeModelIdForCalls);
            state = 1;
        }
        fn_800268B0(model0, scale, scale, scale);
        fn_1_3C90();
        return;
    case 1:
        if (lbl_1_bss_AC > 0.0f) {
            fn_800269E8(model3, 1);
            fn_800269E8(model0, 1);
            transitionModelIdForCalls = model2;
            fn_80026A44(transitionModelIdForCalls, ((1 << 30) | (1 << 1)));
            fn_80026A44(transitionModelIdForCalls, ((1 << 30) | (1 << 0)));
            fn_80026A44(transitionModelIdForCalls, 1);
            fn_8002D414(transitionModelIdForCalls, 0.0f);
            fn_80026760(transitionModelIdForCalls, frameData.x,
                (f32)(4.3 + (f64)frameData.y), frameData.z);
            fn_80026B8C(transitionModelIdForCalls, 5);
            soundPlayAt(1, MSM_SE_MG622_TRANSITION_MODEL_APPEAR, &frameData);
            state = 2;
            return;
        }
        fn_1_3C90();
        return;
    case 2:
        if (fn_8002D75C(model2)) {
            delete this;
        }
    }
}

void fn_1_4398(FourModelOwner *self)
{
    (self->*self->callback)();
}

extern "C" {
char lbl_1_data_BC[12] = "itemhook_c";
}

extern "C" ListRoot<MovingPair> lbl_1_bss_90;

void fn_1_4D70(MovingPair *);

MovingPair::MovingPair(
    const OwnedModel &frontModelData,
    const OwnedModel &backModelData,
    const Vec *frameData,
    s32 requestId)
    : initialized(0),
      requestId(requestId),
      frameData(*frameData),
      frontModel(frontModelData),
      backModel(backModelData)
{
    Vec initialData;
    s16 modelIdForCalls;
    MovingPair::Callback callback;

    insert4(&lbl_1_bss_90);
    modelIdForCalls = frontModel;
    fn_800269E8(modelIdForCalls, ((1 << 30) | (1 << 1)));
    fn_800269E8(modelIdForCalls, (1 << 14));
    fn_8002D414(modelIdForCalls, 0.0f);
    modelIdForCalls = backModel;
    fn_800269E8(modelIdForCalls, ((1 << 30) | (1 << 1)));
    fn_800269E8(modelIdForCalls, (1 << 14));
    fn_800269E8(frontModel, (1 << 0));
    fn_800269E8(backModel, (1 << 0));
    callback = &MovingPair::fn_1_46CC;
    this->callback = callback;
}



void MovingPair::fn_1_458C()
{
    while (!lbl_1_bss_90.empty()) {
        delete lbl_1_bss_90.back();
    }
}

void fn_1_469C()
{
    lbl_1_bss_90.visit(fn_1_4D70);
}

void MovingPair::fn_1_46CC()
{
    Vec position;
    frameData.x += lbl_1_bss_AC;
    position.x = frameData.x;
    position.y = frameData.y;
    position.z = frameData.z;
    frontModel.setPosition(&position);
    backModel.setPosition(&position);
    if (frameData.x > 0.0f) {
        fn_1_1978(frontModel, &frameData, initialized);
        fn_800269E8(backModel, 1);
    } else {
        fn_1_1978(backModel, &frameData, 0);
        fn_800269E8(frontModel, 1);
    }
    if (frameData.x > 550.0f && lbl_1_bss_B4 == requestId) {
        delete this;
    }
}

void MovingPair::fn_1_49CC()
{
    s16 modelIdForCalls = frontModel;
    fn_80026A44(modelIdForCalls, ((1 << 30) | (1 << 1)));
    fn_80026A44(modelIdForCalls, ((1 << 30) | (1 << 0)));
    initialized = 1;
    Callback initialValues = &MovingPair::fn_1_4A70;
    callback = initialValues;
}

void MovingPair::fn_1_4A70()
{
    Vec position;
    frameData.x += lbl_1_bss_AC;
    position.x = frameData.x;
    position.y = frameData.y;
    position.z = frameData.z;
    frontModel.setPosition(&position);
    backModel.setPosition(&position);
    if (frameData.x > 0.0f) {
        fn_1_1978(frontModel, &frameData, initialized);
        fn_800269E8(backModel, 1);
    } else {
        fn_1_1978(backModel, &frameData, 0);
        fn_800269E8(frontModel, 1);
    }
    if (frameData.x > 550.0f && lbl_1_bss_B4 == requestId) {
        delete this;
    }
}



void fn_1_4D70(MovingPair *self)
{
    (self->*self->callback)();
}

extern "C" {
ListRoot<ModelEntry> lbl_1_bss_40;
ListRoot<ModelPairOwner> lbl_1_bss_54;
ListRoot<ModelCameraPlacement> lbl_1_bss_68;
ListRoot<ModelPulseOwner> lbl_1_bss_7C;
ListRoot<MovingPair> lbl_1_bss_90;
ListRoot<FourModelOwner> lbl_1_bss_A4;
f32 lbl_1_bss_AC;
s16 lbl_1_bss_B0;
s16 lbl_1_bss_B2;
s32 lbl_1_bss_B4;
}
