#include "REL/framework/world_units.h"
#include <new>
#include "REL/m681DLL/character_model.h"
#include "REL/m681DLL/board_setup.h"
#include "REL/framework/frame_time.h"
#include "dolphin/types.h"

#include "dolphin/mtx.h"

#include "REL/framework/model.h"
#include "REL/m681DLL/vector_state.h"
#include "REL/framework/event_manager.h"
#include "REL/m681DLL/game_controller.h"
#include "REL/m681DLL/monitor.h"
#include "REL/m681DLL/model_pair.h"
#include "REL/m681DLL/player_object.h"
#include "REL/framework/singleton.h"

#include "REL/framework/scene_shadow.h"

extern "C" {
#include "game/charman.h"
#include "REL/m681DLL/game_constants.h"

}

// Deletes the owned model pair.
inline ModelPairOwner::~ModelPairOwner() { delete value; }

// Publishes the board controller and starts with no model pair.
BoardController::BoardController() {}

// Releases the board controller and its owned model pair.
BoardController::~BoardController() {}

// Creates the camera controller.
void fn_1_EEC(BoardController *)
{
    new GameCamera;
}

extern "C" { const Vec lbl_1_rodata_E8 = { -16.0f, 176.0f, -80.0f }; }

// Replaces the model pair and configures the shadow camera from its light.
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

// Creates the game state, four stands and their players, then enables layer 4.
void fn_1_1014(BoardController *)
{
    new M681GameState;

    for (s32 index = 0; index < 4; ++index) {
        PlayerStand *character = new PlayerStand(index);
        Vec position;
        new PlayerObject(index, fn_1_8184(character, &position));
    }

    fn_8005DDEC(4);
}

// Creates the monitor.
void fn_1_10D8(BoardController *)
{
    new Monitor;
}
