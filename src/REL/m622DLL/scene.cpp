#include "REL/framework/minigame_core.h"
#include "REL/m622DLL/frame_time.h"
#include "REL/m622DLL/random.h"
#include "dolphin/types.h"
#include "REL/m622DLL/model.h"
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





extern "C" { void __ct__10OwnedModelFUi(void *modelSlot, s32 dataNumber); }

extern "C" { void fn_1_33950__5ModelFUl(void *modelSlot, s32 enabled); }


static inline s16 model_handle_id(const s16 *model)
{
    return *model;
}

/* Initializes the scene models and builds a shuffled action table for each phase. */
SceneOwnerB9C::SceneOwnerB9C()
{
    __ct__10OwnedModelFUi(&model0, DATANUM(0x510000, 4));
    __ct__10OwnedModelFUi(&model1, DATANUM(0x510000, 5));
    __ct__10OwnedModelFUi(&model2, DATANUM(0x510000, 6));
    __ct__10OwnedModelFUi(&model3, DATANUM(0x510000, 9));
    __ct__10OwnedModelFUi(&model4, DATANUM(0x510000, 7));
    __ct__10OwnedModelFUi(&model5, DATANUM(0x510000, 8));
    __ct__10OwnedModelFUi(&model6, DATANUM(0x510000, 10));
    __ct__10OwnedModelFUi(&model7, DATANUM(0x510000, 11));
    __ct__10OwnedModelFUi(&model8, DATANUM(0x510000, 12));
    __ct__10OwnedModelFUi(&model9, DATANUM(0x510000, 15));
    __ct__10OwnedModelFUi(&model10, DATANUM(0x510000, 16));
    __ct__10OwnedModelFUi(&model11, DATANUM(0x510000, 1));

    updateCount = 0;
    phaseStarted = 0;
    sceneState = 0;
    delayFrames = 0x2D;
    completedSegments = 0;
    segmentPosition = 0.0f;
    sceneIndex = 0;
    unknown368 = 0;
    soundActive = 0;

    fn_1_33950__5ModelFUl(&model0, 1);
    fn_1_33950__5ModelFUl(&model1, 1);
    fn_1_33950__5ModelFUl(&model2, 1);
    fn_1_33950__5ModelFUl(&model3, 1);
    fn_1_33950__5ModelFUl(&model6, 1);
    fn_1_33950__5ModelFUl(&model4, 1);
    fn_1_33950__5ModelFUl(&model5, 1);
    fn_1_33950__5ModelFUl(&model7, 1);
    fn_1_33950__5ModelFUl(&model8, 1);
    fn_1_33950__5ModelFUl(&model9, 1);
    fn_1_33950__5ModelFUl(&model10, 1);

    fn_80026A44(model_handle_id(&model11.modelId), 1);
    fn_80026760(model_handle_id(&model11.modelId),
                     pathPosition = 0.0f,
                     0.0f, 0.0f);
    fn_80026B8C(model_handle_id(&model11.modelId), 2);

    s16 motionModelId = model_handle_id(&model7.modelId);
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
        ModelVector position = {{-730.0f, 60.0f, 0.0f}};
        position.values[2] = (f32)((phase * 0x122) - 0x24E);

        ModelEntry *scene = new ModelEntry(&model0.modelId,
                                  &position, phase);

        new ModelPairOwner(&model1.modelId,
                                &model2.modelId,
                                &position, (u32)phase);

        s32 selectedAction = phases[phase].selectedAction[sceneIndex];
        ModelCameraPlacement *placement = 0;
        ModelPulseOwner *pulse = 0;
        MovingPair *pair = 0;
        FourModelOwner *fourModels = 0;
        switch (selectedAction) {
        case 0:
            placement = new ModelCameraPlacement(&model3.modelId,
                                     &position, phase);
            break;
        case 1:
            pulse = new ModelPulseOwner(&model6.modelId,
                                     &position, phase);
            break;
        case 2:
            pair = new MovingPair(&model4.modelId,
                                     &model5.modelId,
                                     &position, phase);
            break;
        case 3:
            fourModels = new FourModelOwner(&model7.modelId,
                                     &model8.modelId, &model9.modelId,
                                     &model10.modelId,
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
    fn_80026760(scene_model_id(&model11.modelId),
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

void SceneOwnerB9C::clearFlag(int request)
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

void fn_1_1978(s16 modelId, const f32 *position, u8 mode)
{
    f32 value = *position;

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

extern "C" {
typedef SceneList<ModelEntry> DrawRoot;
extern DrawRoot lbl_1_bss_40;

void __ct__4NodeFv(void *object);
void insert4__4NodeFP4Node(void *object, void *list);
void fn_1_33A3C__5ModelFP3Vec(void *destination, const void *source);
void fn_1_B02C(s32, s32, void *);


}

ModelEntry::ModelEntry(const void *modelData,
                           const ModelVector1A7C *frameData, s32 requestId)
    : active(0),
      requestId(requestId),
      frameData(*frameData),
      model((const OwnedModel *)modelData)
{
    Callback initialData;
    s16 modelIdForCalls;

    insert4__4NodeFP4Node(this, &lbl_1_bss_40);
    modelIdForCalls = scene_model_id(&this->model.modelId);
    fn_80026B8C(modelIdForCalls, 2);
    fn_80026A44(modelIdForCalls, 1);
    fn_800269E8(modelIdForCalls, 0x4000);
    fn_1_33A3C__5ModelFP3Vec(&this->model, &this->frameData);
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
    frameData.values[0] += lbl_1_bss_AC;
    fn_1_33A3C__5ModelFP3Vec(&model, &frameData);
    if (frameData.values[0] > 550.0f && lbl_1_bss_B4 == requestId) {
        delete this;
    }
}

extern "C" {  }
ModelEntry *fn_1_1DB8(s32 ownerId)
{
    ModelEntry *current = lbl_1_bss_40.begin();
    ModelEntry *sentinel = lbl_1_bss_40.end();
    while (current != sentinel) {
        if (current->frameData.values[0] > 60.0f &&
            current->frameData.values[0] < 230.0f &&
            current->requestId == ownerId && current->active == 0) {
            return current;
        }
        current = DrawRoot::next(current);
    }
    return 0;
}

s32 fn_1_1E68(s32 requestId)
{
    ModelEntry *current = lbl_1_bss_40.begin();
    ModelEntry *sentinel = lbl_1_bss_40.end();
    while (current != sentinel) {
        if (current->frameData.values[0] > 160.0f &&
            current->frameData.values[0] < 211.0f &&
            current->requestId == requestId) {
            if (current->model0 != 0) return 0;
            if (current->model1 != 0) return 1;
            if (current->model2 != 0) return 2;
            if (current->model3 != 0) return 3;
            return 4;
        }
        current = DrawRoot::next(current);
    }
    return -1;
}

bool ModelEntry::fn_1_1F5C()
{
    return lbl_1_bss_AC > 0.0f;
}


s32 ModelEntry::fn_1_1F84(s16 characterModelId, s16 itemId)
{
    static const ModelVector1A7C initialModelData = {{0.0f, -10.0f, 0.0f}};
    ModelVector1A7C modelData;
    Callback initialData;
    char *hookId;
    if (frameData.values[0] > 190.0f && frameData.values[0] < 230.0f) {
        active = 1;
        this->characterModelId = characterModelId;
        itemHookId = itemId;
        frame = 0;
        hookId = fn_8005C0F0(itemHookId, 1, 0);
        fn_8002741C(this->characterModelId, hookId, model);
        modelData = initialModelData;
        fn_1_33A3C__5ModelFP3Vec(&model, &modelData);
        initialData = &ModelEntry::fn_1_217C;
        this->initialData = initialData;
        fn_1_B02C(1, MSM_SE_MG622_ITEM_ATTACH, &frameData);
        return 1;
    }
    return 0;
}

s32 ModelEntry::fn_1_20C0()
{
    if (model0 != 0) { model0->fn_1_2E08(); return 0; }
    if (model1 != 0) { model1->fn_1_3678(); return 1; }
    if (model2 != 0) {
        fn_1_B02C(1, MSM_SE_MG622_ENTRY_MODEL_ACTION, &frameData);
        model2->fn_1_49CC(); return 2;
    }
    if (model3 != 0) {
        fn_1_B02C(1, MSM_SE_GUIDE_OBJECT_CATCH, &frameData);
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


extern "C" {

extern void __ct__4NodeFv(void *object);
extern void insert4__4NodeFP4Node(void *object, void *list);
extern void fn_1_33A3C__5ModelFP3Vec(void *modelId, const void *vector);
}

typedef SceneList<ModelPairOwner> PairRoot;
extern "C" PairRoot lbl_1_bss_54;

ModelPairOwner::ModelPairOwner(const s16 *firstSource,
                         const s16 *secondSource, const ModelVector *vector,
                         u32 requestId)
    : requestId(requestId),
      vector(*vector),
      firstModel((const OwnedModel *)firstSource),
      secondModel((const OwnedModel *)secondSource)
{
    s16 modelIdForCalls;

    insert4__4NodeFP4Node(this, &lbl_1_bss_54);

    modelIdForCalls = scene_model_id(&this->firstModel.modelId);
    fn_80026B8C(modelIdForCalls, 2);
    fn_80026A44(modelIdForCalls, 1);
    fn_800269E8(modelIdForCalls, 0x4000);
    fn_1_33A3C__5ModelFP3Vec(&this->firstModel, &this->vector);

    modelIdForCalls = scene_model_id(&this->secondModel.modelId);
    fn_80026B8C(modelIdForCalls, 1);
    fn_80026A44(modelIdForCalls, 1);
    fn_1_33A3C__5ModelFP3Vec(&this->secondModel, &this->vector);
}

ModelPairNode::~ModelPairNode() {}

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
    vector.values[0] += lbl_1_bss_AC;
    fn_1_33A3C__5ModelFP3Vec(&firstModel, &vector);
    s16 modelId = secondModel;
    if (vector.values[0] > (-648.0f) && vector.values[0] < (-321.0f)) {
        fn_80026A44(modelId, 1);
        fn_1_33A3C__5ModelFP3Vec(&secondModel, &vector);
    } else {
        fn_800269E8(modelId, 1);
    }
    if (vector.values[0] > 550.0f && lbl_1_bss_B4 == requestId) {
        delete this;
    }
}

void fn_1_2774(ModelPairOwner *self)
{
    self->fn_1_2618();
}



typedef SceneList<ModelCameraPlacement> PlacementRoot;
extern "C" PlacementRoot lbl_1_bss_68;

ModelCameraPlacement::ModelCameraPlacement(void *modelData,
    const ModelCameraVector *cameraValues, s32 mode)
    : active(0),
      mode(mode),
      cameraPosition(*cameraValues),
      modelScale(0.4f),
      modelOffset(0.0f),
      model((const OwnedModel *)modelData)
{
    s16 modelIdForCalls;
    insert4__4NodeFP4Node(this, &lbl_1_bss_68);
    modelIdForCalls = scene_model_id(&model.modelId);
    fn_800268B0(modelIdForCalls, modelScale, modelScale, modelScale);
    fn_1_1978(modelIdForCalls, &cameraPosition.values[0], 0);
    fn_800269E8(modelIdForCalls, 0x4000);
    Callback initialValues = &ModelCameraPlacement::fn_1_2C18;
    this->initialValues = initialValues;
}

ModelCameraNode::~ModelCameraNode() {}

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
    ModelCameraVector position;
    cameraPosition.values[0] += lbl_1_bss_AC;
    position.values[0] = cameraPosition.values[0];
    position.values[1] = cameraPosition.values[1];
    position.values[2] = cameraPosition.values[2];
    position.values[1] += 30.0f + modelOffset;
    fn_1_33A3C__5ModelFP3Vec(&model, &position);
    fn_1_1978(model, &cameraPosition.values[0], active);
    if (cameraPosition.values[0] > 550.0f && lbl_1_bss_B4 == mode) {
        delete this;
    }
}

void ModelCameraPlacement::fn_1_2E08()
{
    s16 modelIdForCalls = scene_model_id(&model.modelId);
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

typedef SceneList<ModelPulseOwner> PulseRoot;
extern "C" PulseRoot lbl_1_bss_7C;

ModelPulseOwner::ModelPulseOwner(const void *modelData,
    const ModelVector *frameData, s32 requestId)
    : initialized(0),
      requestId(requestId),
      frameData(*frameData),
      scale(0.4f),
      model((const OwnedModel *)modelData)
{
    CallbackState callback;
    s16 modelId;
    insert4__4NodeFP4Node(this, &lbl_1_bss_7C);
    modelId = scene_model_id(&model.modelId);
    fn_800268B0(modelId, scale, scale, scale);
    fn_1_1978(modelId, &this->frameData.values[0], 0);
    fn_800269E8(modelId, 0x4000);
    callback = &ModelPulseOwner::fn_1_3490;
    this->callback = callback;
}

ModelPulseNode::~ModelPulseNode() {}

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
    ModelVector position;
    frameData.values[0] += lbl_1_bss_AC;
    position.values[0] = frameData.values[0];
    position.values[1] = frameData.values[1];
    position.values[2] = frameData.values[2];
    position.values[1] += 4.3f;
    fn_1_33A3C__5ModelFP3Vec(&model, &position);
    fn_1_1978(model, &frameData.values[0], initialized);
    if (frameData.values[0] > 550.0f && lbl_1_bss_B4 == requestId) {
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


typedef SceneList<FourModelOwner> FourRoot;
extern "C" FourRoot lbl_1_bss_A4;

FourModelOwner::FourModelOwner(
    const void *firstModelData,
    const void *secondModelData,
    const void *thirdModelData,
    const void *fourthModelData,
    const struct ModelVector *frameData,
    s32 requestId)
    : initialized(0),
      requestId(requestId),
      frameData(*frameData),
      scale(0.4f),
      model0((const OwnedModel *)firstModelData),
      model1((const OwnedModel *)secondModelData),
      model2((const OwnedModel *)thirdModelData),
      model3((const OwnedModel *)fourthModelData)
{
    struct ModelVector initialData;
    FourModelOwner::Callback callback;
    s16 modelIdCopy;

    insert4__4NodeFP4Node(this, &lbl_1_bss_A4);

    modelIdCopy = scene_model_id(&this->model0.modelId);
    fn_800268B0(modelIdCopy, this->scale, this->scale,
                      this->scale);
    fn_800269E8(modelIdCopy, 0x4000);
    modelIdCopy = scene_model_id(&this->model1.modelId);
    fn_800268B0(modelIdCopy, this->scale, this->scale,
                      this->scale);
    fn_1_33950__5ModelFUl(&this->model0, 1);
    fn_1_33950__5ModelFUl(&this->model1, 1);
    fn_1_33950__5ModelFUl(&this->model2, 1);
    fn_1_33950__5ModelFUl(&this->model3, 1);
    callback = &FourModelOwner::fn_1_3C90;
    this->callback = callback;
}

FourModelNode::~FourModelNode() {}
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
    ModelVector position;
    frameData.values[0] += lbl_1_bss_AC;
    position.values[0] = frameData.values[0];
    position.values[1] = frameData.values[1];
    position.values[2] = frameData.values[2];
    position.values[1] += 4.3f;
    fn_1_33A3C__5ModelFP3Vec(&model0, &position);
    fn_1_33A3C__5ModelFP3Vec(&model1, &position);
    if (frameData.values[0] > 0.0f) {
        fn_1_1978(model0, &frameData.values[0], initialized);
        fn_1_33950__5ModelFUl(&model1, 1);
    } else {
        fn_1_1978(model1, &frameData.values[0], 0);
        fn_1_33950__5ModelFUl(&model0, 1);
    }
    if (frameData.values[0] > 550.0f && lbl_1_bss_B4 == requestId) {
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
            fn_80026760(transitionModelIdForCalls, frameData.values[0],
                (f32)(4.3 + (f64)frameData.values[1]), frameData.values[2]);
            fn_80026B8C(transitionModelIdForCalls, 5);
            fn_1_B02C(1, MSM_SE_MG622_TRANSITION_MODEL_APPEAR, &frameData);
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

typedef SceneList<MovingPair> MovingRoot;
extern "C" MovingRoot lbl_1_bss_90;

void fn_1_4D70(MovingPair *);

MovingPair::MovingPair(
    const void *frontModelData,
    const void *backModelData,
    const struct ModelVector *frameData,
    s32 requestId)
    : initialized(0),
      requestId(requestId),
      frameData(*frameData),
      frontModel((const OwnedModel *)frontModelData),
      backModel((const OwnedModel *)backModelData)
{
    struct ModelVector initialData;
    s16 modelIdForCalls;
    MovingPair::Callback callback;

    insert4__4NodeFP4Node(this, &lbl_1_bss_90);
    modelIdForCalls = scene_model_id(&this->frontModel.modelId);
    fn_800269E8(modelIdForCalls, ((1 << 30) | (1 << 1)));
    fn_800269E8(modelIdForCalls, (1 << 14));
    fn_8002D414(modelIdForCalls, 0.0f);
    modelIdForCalls = scene_model_id(&this->backModel.modelId);
    fn_800269E8(modelIdForCalls, ((1 << 30) | (1 << 1)));
    fn_800269E8(modelIdForCalls, (1 << 14));
    fn_800269E8(scene_model_id(&this->frontModel.modelId), (1 << 0));
    fn_800269E8(scene_model_id(&this->backModel.modelId), (1 << 0));
    callback = &MovingPair::fn_1_46CC;
    this->callback = callback;
}

MovingPairNode::~MovingPairNode() {}



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
    ModelVector position;
    frameData.values[0] += lbl_1_bss_AC;
    position.values[0] = frameData.values[0];
    position.values[1] = frameData.values[1];
    position.values[2] = frameData.values[2];
    fn_1_33A3C__5ModelFP3Vec(&frontModel, &position);
    fn_1_33A3C__5ModelFP3Vec(&backModel, &position);
    if (frameData.values[0] > 0.0f) {
        fn_1_1978(frontModel, &frameData.values[0], initialized);
        fn_800269E8(backModel, 1);
    } else {
        fn_1_1978(backModel, &frameData.values[0], 0);
        fn_800269E8(frontModel, 1);
    }
    if (frameData.values[0] > 550.0f && lbl_1_bss_B4 == requestId) {
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
    ModelVector position;
    frameData.values[0] += lbl_1_bss_AC;
    position.values[0] = frameData.values[0];
    position.values[1] = frameData.values[1];
    position.values[2] = frameData.values[2];
    fn_1_33A3C__5ModelFP3Vec(&frontModel, &position);
    fn_1_33A3C__5ModelFP3Vec(&backModel, &position);
    if (frameData.values[0] > 0.0f) {
        fn_1_1978(frontModel, &frameData.values[0], initialized);
        fn_800269E8(backModel, 1);
    } else {
        fn_1_1978(backModel, &frameData.values[0], 0);
        fn_800269E8(frontModel, 1);
    }
    if (frameData.values[0] > 550.0f && lbl_1_bss_B4 == requestId) {
        delete this;
    }
}



void fn_1_4D70(MovingPair *self)
{
    (self->*self->callback)();
}

template <class T> void visitSceneNodes(T *current, T *sentinel, void (*visitor)(T *))
{
    while (current != sentinel) {
        visitor(current);
        current = SceneList<T>::next(current);
    }
}

template <class T> void SceneList<T>::visit(void (*visitor)(T *))
{
    visitSceneNodes(begin(), end(), visitor);
}

extern "C" {

s32 lbl_1_bss_28 = secondsToFrames(2L);
s32 lbl_1_bss_2C = (s32)secondsToFrames(3.5);
s32 lbl_1_bss_30 = (s32)secondsToFrames(3.5);
SceneList<ModelEntry> lbl_1_bss_40;
SceneList<ModelPairOwner> lbl_1_bss_54;
SceneList<ModelCameraPlacement> lbl_1_bss_68;
SceneList<ModelPulseOwner> lbl_1_bss_7C;
SceneList<MovingPair> lbl_1_bss_90;
SceneList<FourModelOwner> lbl_1_bss_A4;
f32 lbl_1_bss_AC;
s16 lbl_1_bss_B0;
s16 lbl_1_bss_B2;
s32 lbl_1_bss_B4;
}
