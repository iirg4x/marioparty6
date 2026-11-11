#include "REL/framework/frame_time.h"

#include "math.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "dolphin/types.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "REL/framework/model.h"
#include "REL/m681DLL/model_resource.h"

extern "C" {
#include "REL/m681DLL.h"
#include "REL/framework/world_units.h"
s32 __ct__10OwnedModelFUi(void *, s32);
s32 setAttribute__5ModelFUl(void *, s32);
extern s32 lbl_1_bss_90;
extern f32 lbl_1_bss_94;
void fn_80023398(s32 objectId, u32 modelId, Mtx matrix);
void Replace__5ModelFs(void *object, s16 releaseMode);
void __dl__FPv(void *object);
}

struct ModelObject {
    s16 objectId;
    u16 unknown_02;
    char *nodeName;
};

struct ModelObjectMatrix {
    s16 objectId;
    u16 unknown_02;
    u32 modelId;
};

ModelResource::ModelResource(s32 player)
    : OwnedModel(DATANUM(0x8c0000, 8))
{
    nodeName = "pipepass";
    setAttribute(((1 << 30) | (1 << 1)));
}

ModelResource::~ModelResource() {}

void fn_1_5168(Model *object)
{
    object->resetAttribute(((1 << 30) | (1 << 1)));
}

void fn_1_5198(ModelObject *object, HuVecF *sourcePosition, HuVecF *rotation)
{
    f64 x;
    f64 y;
    f64 z;

    ((Model *)object)->getObjectPosition(object->nodeName, sourcePosition);
    HSF_TRANSFORM *modelTransform = &((Model *)object)->getObject(object->nodeName)->mesh.base;
    z = modelTransform->rot.z;
    y = modelTransform->rot.y;
    x = modelTransform->rot.x;
    rotation->x = x;
    rotation->y = y;
    rotation->z = z;
}

extern "C" f32 (*fn_1_524C(ModelObjectMatrix *object, Mtx result))[4]
{
    Mtx rotation;
    s32 objectId;

    objectId = (s16)object->objectId;
    fn_80023398(objectId, object->modelId, result);
    PSMTXRotRad(rotation, 'z', 1.5707964f);
    PSMTXConcat(result, rotation, result);
    PSMTXRotRad(rotation, 'y', -1.5707964f);
    PSMTXConcat(result, rotation, result);
    return result;
}

extern "C" {

}

extern "C" {
}

extern "C" {
s32 lbl_1_bss_90 = minutesToFrames(60L);
f32 lbl_1_bss_94 = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));
}
