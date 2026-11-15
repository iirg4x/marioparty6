// Passes a scene event on to the objects of two categories.
#include <vector>
#include "REL/m634DLL/game_manager.h"
#include "REL/framework/game_object_man.h"
#include "REL/m634DLL/particle_object_base.h"
#include "REL/m634DLL/mode_render_object.h"

// Sends the scene event to the two object categories.
void GameManager::fn_1_CA30(const EventPacket *event)
{
    std::vector<GameObject *> firstCollection;
    std::vector<GameObject *> secondCollection;
    int firstCount;
    int secondCount;
    ParticleObjectBase *firstObject;
    ModeRenderObject *secondObject;
    int firstIndex;
    int secondIndex;

    collectGameObjects(0x50, &firstCollection);
    collectGameObjects(0x60, &secondCollection);
    firstCount = firstCollection.size();
    secondCount = secondCollection.size();
    for (firstIndex = 0; firstIndex < firstCount; firstIndex++) {
        firstObject = static_cast<ParticleObjectBase *>(firstCollection[firstIndex]);
        firstObject->mode(event->type);
    }
    for (secondIndex = 0; secondIndex < secondCount; secondIndex++) {
        secondObject = static_cast<ModeRenderObject *>(secondCollection[secondIndex]);
        secondObject->mode(event->type);
    }
}
