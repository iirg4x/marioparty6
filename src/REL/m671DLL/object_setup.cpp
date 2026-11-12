#include "REL/framework/phase_duration_statics.h"
#include "REL/framework/model.h"
#include "dolphin/types.h"

#include "REL/m671DLL/scene_object.h"

#include "REL/framework/minigame_core.h"

#include "REL/framework/thread.h"

#include "REL/m671DLL/camera_motion.h"

#include "game/hu3d.h"

extern const s32 heapSize = 0x100000;

extern "C" void ObjectSetup()
{
    SceneObjectOwner *root = new SceneObjectOwner;
    root->setupActorManager();
}
