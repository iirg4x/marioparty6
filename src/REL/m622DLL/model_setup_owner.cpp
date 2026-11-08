#include "dolphin/types.h"
extern "C" {
#include "game/hu3d.h"
}
#include "REL/m622DLL/model_setup_owner.h"

static inline s16 model_id(const s16 *model)
{
    return *model;
}

ModelSetupOwner::ModelSetupOwner() : model(DATANUM(0x510000, 0)), hookModel(DATANUM(0x510000, 2))
{
    s16 currentModel;

    currentModel = model_id(&model.modelId);
    fn_80029850(currentModel, 1);
    model.fn_1_33CB0(2);
    model.fn_1_33950(((1 << 30) | (1 << 0)));
    fn_80026A44(currentModel, (1 << 0));
    currentModel = model_id(&hookModel.modelId);
    hookModel.fn_1_33CB0(2);
    fn_80026A44(currentModel, ((1 << 30) | (1 << 0)));
    fn_80026A44(currentModel, (1 << 0));
}

void ModelSetupOwner::update()
{
}

typedef ModelSetupOwner ModelAttributeOwner;

/* Clears the owner model attributes and restores the stored motion time. */
void ModelSetupOwner::fn_1_658C()
{
    s16 modelId = (s16)((ModelAttributeOwner *)this)->model.modelId;

    fn_80026A44(modelId, 0x40000002);
    s16 motionId = (s16)((ModelAttributeOwner *)this)->hookModel.modelId;
    fn_8002D414(motionId, 0.0f);
}

/* Enables the model attributes configured for this owner. */
void ModelSetupOwner::fn_1_65F8()
{
    s16 modelId;

    modelId = (s16)((ModelAttributeOwner *)this)->model.modelId;
    fn_800269E8(modelId, 0x40000002);
}

void ModelSetupOwner::clearFlag(int request)
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

ModelSetupOwner::~ModelSetupOwner()
{
}
