#include "REL/framework/frame_time.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "datadir_enum.h"
#include "game/data.h"
#include "REL/framework/model.h"
#include "REL/m681DLL/character_sequence.h"
#include "REL/m681DLL/model_pair.h"
#include "REL/m681DLL/motion_object.h"

extern "C" {
void __msl_assertion_failed(const char *expression, const char *file, int line);
void __ct__10OwnedModelFUi(void *object, u32 value);
void setAttribute__5ModelFUl(void *object, u32 value);
void setLayer__5ModelFs(void *object, s32 value);
void setLightsDynamic__5ModelFb(void *object, s32 value);
Vec *getObjectPosition__5ModelFPcP3Vec(void *object, const char *name, Vec *result);
void updateMotion__5ModelFv(void *object);
void *__nw__FUl(u32 size);
const char *lbl_1_data_25C[2] = { "kurinullr", "kurinulll" };
}

void *operator new(unsigned long);

ModelPair::ModelPair()
    : firstModel(DATANUM(0x8c0000, 0)), secondModel(DATANUM(0x8c0000, 1))
{
    Vec result;
    s32 index;

    setLayer__5ModelFs(this, 0);
    setLightsDynamic__5ModelFb(this, 0);
    setAttribute__5ModelFUl(this, 0x40000001);
    setLayer__5ModelFs(&secondModel, 2);
    updateMotion__5ModelFv(this);

    for (index = 0; index < 2; ++index) {
        new MotionObject(
            getObjectPosition__5ModelFPcP3Vec(this, lbl_1_data_25C[index], &result),
            index);
    }
}

void operator delete(void *);

extern "C" {
void Replace__5ModelFs(void *, short);
}

ModelPair::~ModelPair() {}

extern "C" {
#include "REL/m681DLL.h"
#include "REL/framework/world_units.h"
}

Vec *fn_1_2FD4(ModelPair *object, Vec *direction)
{
    Vec position;
    GXColor color;

    object->firstModel.setLight("inf_light1", &position, direction, &color);
    return direction;
}

extern s32 lbl_1_bss_78;
extern f32 lbl_1_bss_7C;

extern "C" {

}

extern "C" {
}

extern "C" {
s32 lbl_1_bss_78 = minutesToFrames(60L);
f32 lbl_1_bss_7C = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));
}
