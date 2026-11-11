#include "REL/m672DLL/random.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"
#include "REL/framework/phase_duration_statics.h"
#include "dolphin/types.h"
#include <string.h>
#include "REL/m672DLL/collision_object.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_hm.h"
#include "REL/framework/model.h"
#include "REL/framework/event_callback_inline.h"

/* Capacity of the minigame heap used by initializeMemoryManager. */
extern const s32 heapSize = 0x100000;
#include "dolphin/os.h"
#include "dolphin/mtx.h"
#include "game/data.h"
#include "game/hu3d.h"
#include <stdio.h>
#include <stdlib.h>

void* operator new(unsigned long);
#include "REL/m672DLL/scene_thread_owner.h"

extern "C" void ObjectSetup() {
    SceneOwner672* root=new SceneOwner672;
    root->setupActorManager();
}
