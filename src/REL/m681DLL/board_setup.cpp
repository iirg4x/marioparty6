#include "REL/m681DLL/character_model.h"
#include "REL/m681DLL/board_setup.h"
#include "REL/m681DLL/frame_time.h"
#include "dolphin/types.h"

#include "dolphin/mtx.h"

#include "REL/m681DLL/framework/model.h"
#include "REL/m681DLL/vector_state.h"
#include "REL/m681DLL/event_manager.h"
#include "REL/m681DLL/owner_182c.h"
#include "REL/m681DLL/monitor.h"
#include "REL/m681DLL/model_pair.h"
#include "REL/framework/minigame_singleton.h"

extern "C" void fn_8005DDEC(s16 layerNo);


extern "C" {

}


extern "C" {


}


void operator delete(void *);


inline Owner47C::~Owner47C() { delete value; }

BoardController::BoardController() {}

BoardController::~BoardController() {}


void *operator new(u32 size);

void fn_1_EEC(BoardController *)
{
    new VectorStateComposite;
}


extern "C" { void configureShadowCamera__FsPC3VecP3Vecfff(s32, Vec *, Vec *, f32, f32, f32); }

extern "C" { const Vec lbl_1_rodata_E8 = { -16.0f, 176.0f, -80.0f }; }

void *operator new(u32 size);

void fn_1_F30(BoardController *owner)
{
    ModelPair *updatedChild = new ModelPair;
    if (owner->child.value != updatedChild) {
        delete owner->child.value;
        owner->child.value = updatedChild;
    }

    Vec current;
    ModelPair *currentChild = owner->child.value;
    fn_1_2FD4(currentChild, &current);
    Vec defaultValue = lbl_1_rodata_E8;
    configureShadowCamera__FsPC3VecP3Vecfff(1, &current, &defaultValue,
               800.0f, 600.0f, 5.0f);
}


extern "C" { void *__ct__21ModelOwnerComposite84Fi(void *object, s32 index); }

extern "C" {}
 Vec *fn_1_8184(ModelOwnerComposite84 *object, Vec *value);
extern "C" { }

extern "C" { void *__ct__12PlayerObjectFlPC14PositionVector(void *object, s32 index, const Vec *value); }

void fn_1_1014(BoardController *)
{
    new M681GameState;

    for (s32 index = 0; index < 4; ++index) {
        void *layer;
        if ((layer = ::operator new(0x54)) != 0) {
            layer = __ct__21ModelOwnerComposite84Fi(layer, index);
        }
        void *effectOwner = layer;
        void *effect = ::operator new(0xD8);
        if (effect != 0) {
            Vec value;
            effect = __ct__12PlayerObjectFlPC14PositionVector(
                effect, index, fn_1_8184(
                static_cast<ModelOwnerComposite84 *>(effectOwner), &value));
        }
    }

    fn_8005DDEC(4);
}


void fn_1_10D8(BoardController *)
{
    new Monitor;
}

extern "C" {


}

extern "C" {
}

static inline f32 accelerationPerFrame(f64 accelerationPerSecondSquared)
{
    return (f32)((accelerationPerSecondSquared * 100.0) / (60.0 * 60.0));
}

extern "C" {
s32 lbl_1_bss_8 = minutesToFrames(60L);
f32 lbl_1_bss_C = accelerationPerFrame(9.80665);
}
