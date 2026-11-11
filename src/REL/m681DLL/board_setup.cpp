#include "REL/m681DLL/character_model.h"
#include "REL/m681DLL/board_setup.h"
#include "REL/framework/frame_time.h"
#include "dolphin/types.h"

#include "dolphin/mtx.h"

#include "REL/framework/model.h"
#include "REL/m681DLL/vector_state.h"
#include "REL/m681DLL/event_manager.h"
#include "REL/m681DLL/owner_182c.h"
#include "REL/m681DLL/monitor.h"
#include "REL/m681DLL/model_pair.h"
#include "REL/m681DLL/player_object.h"
#include "REL/framework/singleton.h"
#include "REL/framework/world_units.h"
#include "REL/framework/scene_shadow.h"

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
    configureShadowCamera(1, &current, &defaultValue, 800.0f, 600.0f, 5.0f);
}

extern "C" {}
 Vec *fn_1_8184(ModelOwnerComposite84 *object, Vec *value);
extern "C" { }

void fn_1_1014(BoardController *)
{
    new M681GameState;

    for (s32 index = 0; index < 4; ++index) {
        ModelOwnerComposite84 *character = new ModelOwnerComposite84(index);
        Vec position;
        new PlayerObject(index, fn_1_8184(character, &position));
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

extern "C" {
s32 lbl_1_bss_8 = minutesToFrames(60L);
f32 lbl_1_bss_C = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));
}
