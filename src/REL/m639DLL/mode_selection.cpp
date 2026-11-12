#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/random.h"
// Registers the module's state owner and initializes its status.
#include "REL/m639DLL/mode_game.h"
#include "REL/m639DLL/mode_selection.h"
#include "REL/m639DLL/scene_object_owner.h"

/* The size of the minigame's heap. */
extern const s32 heapSize = 0x20000;

#include "REL/m639DLL/frame_durations.h"

M639StateOwner::M639StateOwner() : initialized(0), selection(-1) {}

M639StateOwner::~M639StateOwner() {}

/* Selects a mode once, after checking which of its component states are available. */
int M639StateOwner::fn_1_2CAC()
{
    bool firstAvailable;
    bool secondAvailable;
    unsigned char stateBits;
    if (initialized) {
        return selection;
    }

    firstAvailable = ModeGame::getChecked().fn_1_3B0(0);
    secondAvailable = ModeGame::getChecked().fn_1_3B0(1);

    stateBits = ((unsigned char)firstAvailable ? 1 : 0) | ((unsigned char)secondAvailable ? 2 : 0);
    switch (stateBits) {
    case 0:
        selection = -1;
        break;
    case 1:
        selection = 0;
        break;
    case 2:
        selection = 1;
        break;
    case 3:
        selection = ((int)RandomUnsigned() & 1) ? 0 : 1;
        break;
    }
    initialized = 1;
    return selection;
}

extern "C" void ObjectSetup()
{
    SceneObjectOwner *root = new SceneObjectOwner;
    root->setup(1, 8192);
}
