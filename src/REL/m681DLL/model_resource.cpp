#include "math.h"
#include "REL/m681DLL/model_resource.h"
extern "C" {
#include "game/hu3d.h"
}
#include "game/hsfex.h"
#include "dolphin/mtx.h"
#include "datadir_enum.h"
#include "REL/m681DLL/game_constants.h"

// Creates the player resource model and selects its attachment node.
ModelResource::ModelResource(s32 player)
    : OwnedModel(DATANUM(0x8c0000, 8))
{
    nodeName = "pipepass";
    setAttribute(((1 << 30) | (1 << 1)));
}

// Releases the owned resource model.
ModelResource::~ModelResource() {}

// Clears the model attribute used during the fall.
void fn_1_5168(Model *object)
{
    object->resetAttribute(((1 << 30) | (1 << 1)));
}

// Copies the attachment node position and rotation to the output vectors.
void fn_1_5198(ModelResource *object, HuVecF *sourcePosition, HuVecF *rotation)
{
    f64 x;
    f64 y;
    f64 z;

    object->getObjectPosition(object->nodeName, sourcePosition);
    HSF_TRANSFORM *modelTransform = &object->getObject(object->nodeName)->mesh.base;
    z = modelTransform->rot.z;
    y = modelTransform->rot.y;
    x = modelTransform->rot.x;
    rotation->x = x;
    rotation->y = y;
    rotation->z = z;
}

// Builds the attachment node matrix with the two orientation adjustments.
extern "C" f32 (*fn_1_524C(ModelResource *object, Mtx result))[4]
{
    Mtx rotation;
    s16 objectId;

    objectId = (s16)object->modelId;
    fn_80023398(objectId, object->nodeName, result);
    PSMTXRotRad(rotation, 'z', 1.5707964f);
    PSMTXConcat(result, rotation, result);
    PSMTXRotRad(rotation, 'y', -1.5707964f);
    PSMTXConcat(result, rotation, result);
    return result;
}
