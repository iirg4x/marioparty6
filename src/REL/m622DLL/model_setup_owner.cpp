#include "dolphin/types.h"
extern "C" {
#include "game/hu3d.h"
}
#include "REL/m622DLL/model_setup_owner.h"

/* Creates the two stage models and configures their layers and attributes. */
ModelSetupOwner::ModelSetupOwner() : model(DATANUM(0x510000, 0)), hookModel(DATANUM(0x510000, 2))
{
    s16 currentModel;

    currentModel = model;
    fn_80029850(currentModel, 1);
    model.setLayer(2);
    model.setAttribute(((1 << 30) | (1 << 0)));
    fn_80026A44(currentModel, (1 << 0));
    currentModel = hookModel;
    hookModel.setLayer(2);
    fn_80026A44(currentModel, ((1 << 30) | (1 << 0)));
    fn_80026A44(currentModel, (1 << 0));
}

/* Returns without changing state. */
void ModelSetupOwner::update()
{
}

/* Clears the owner model attributes and sets the hook model motion time to zero.
 * Both model ids are read through `this` cast to its own class, as in fn_1_65F8. */
void ModelSetupOwner::fn_1_658C()
{
    s16 modelId = (s16)((ModelSetupOwner *)this)->model.modelId;

    fn_80026A44(modelId, 0x40000002);
    s16 hookModelId = (s16)((ModelSetupOwner *)this)->hookModel.modelId;
    fn_8002D414(hookModelId, 0.0f);
}

/* Enables the model attributes configured for this owner. */
void ModelSetupOwner::fn_1_65F8()
{
    s16 modelId;

    modelId = (s16)((ModelSetupOwner *)this)->model.modelId;
    fn_800269E8(modelId, 0x40000002);
}

/* Clears the requested flag; the listed requests have no further action. */
void ModelSetupOwner::flagEnabled(int request)
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

/* Releases the object through its member and base destructors. */
ModelSetupOwner::~ModelSetupOwner()
{
}
