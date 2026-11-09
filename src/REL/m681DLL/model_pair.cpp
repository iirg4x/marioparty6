#include "REL/m681DLL/frame_time.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "datadir_enum.h"
#include "game/data.h"
#include "REL/m681DLL/framework/model.h"
#include "REL/m681DLL/character_sequence.h"
#include "REL/m681DLL/model_pair.h"
#include "REL/m681DLL/motion_object.h"


extern "C" {
void __msl_assertion_failed(const char *expression, const char *file, int line);
void __ct__10OwnedModelFUi(void *object, u32 value);
void fn_1_33950__5ModelFUl(void *object, u32 value);
void fn_1_33CB0__5ModelFs(void *object, s32 value);
void fn_1_33DF8__5ModelFb(void *object, s32 value);
Vec *fn_1_344A0__5ModelFPcP3Vec(void *object, const char *name, Vec *result);
void fn_1_34734__5ModelFv(void *object);
void *__nw__FUl(u32 size);
const char *lbl_1_data_25C[2] = { "kurinullr", "kurinulll" };
}

void *operator new(unsigned long);

ModelPair::ModelPair()
    : firstModel(DATANUM(0x8c0000, 0)), secondModel(DATANUM(0x8c0000, 1))
{
    Vec result;
    s32 index;

    fn_1_33CB0__5ModelFs(this, 0);
    fn_1_33DF8__5ModelFb(this, 0);
    fn_1_33950__5ModelFUl(this, 0x40000001);
    fn_1_33CB0__5ModelFs(&secondModel, 2);
    fn_1_34734__5ModelFv(this);

    for (index = 0; index < 2; ++index) {
        new MotionObject(
            fn_1_344A0__5ModelFPcP3Vec(this, lbl_1_data_25C[index], &result),
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
}


Vec *fn_1_2FD4(ModelPair *object, Vec *direction)
{
    Vec position;
    GXColor color;

    object->firstModel.fn_1_33D68("inf_light1", &position, direction, &color);
    return direction;
}

extern s32 lbl_1_bss_78;
extern f32 lbl_1_bss_7C;


extern "C" {


}

extern "C" {
}

static inline f32 accelerationPerFrame(f64 accelerationPerSecondSquared)
{
    return (f32)((accelerationPerSecondSquared * 100.0) / (60.0 * 60.0));
}

extern "C" {
s32 lbl_1_bss_78 = minutesToFrames(60L);
f32 lbl_1_bss_7C = accelerationPerFrame(9.80665);
}
