#include "REL/framework/world_units.h"
#include <new>
#include "REL/framework/frame_time.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "datadir_enum.h"
#include "game/data.h"
#include "REL/framework/model.h"
#include <vector>
#include "REL/m681DLL/model_pair.h"
#include "REL/m681DLL/motion_object.h"
#include "REL/m681DLL/character_model.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "REL/m681DLL/game_constants.h"

extern "C" {
char *lbl_1_data_25C[2] = { "kurinullr", "kurinulll" };
}

// Creates the two stage models and their animated child objects.
ModelPair::ModelPair()
    : firstModel(DATANUM(0x8c0000, 0)), secondModel(DATANUM(0x8c0000, 1))
{
    Vec result;
    s32 index;

    firstModel.setLayer(0);
    firstModel.setLightsDynamic(false);
    firstModel.setAttribute(0x40000001);
    secondModel.setLayer(2);
    firstModel.updateMotion();

    for (index = 0; index < 2; ++index) {
        new MotionObject(&firstModel.getObjectPosition(lbl_1_data_25C[index], &result), index);
    }
}

// Releases both owned stage models.
ModelPair::~ModelPair() {}

// Returns the direction of the first model light.
Vec *fn_1_2FD4(ModelPair *object, Vec *direction)
{
    Vec position;
    GXColor color;

    object->firstModel.setLight("inf_light1", &position, direction, &color);
    return direction;
}
