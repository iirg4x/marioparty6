// Passes a scene event on to the objects of two categories.
#include <vector>
#include "REL/m634DLL/game_manager.h"
#include "REL/framework/game_object_man.h"

struct EventReceiver : GameObject {
    virtual void dispatchFirst(int value);
    virtual void dispatchSecond(int value);
};

void GameManager::fn_1_CA30(const EventPacket *event)
{
    std::vector<GameObject *> firstCollection;
    std::vector<GameObject *> secondCollection;
    int firstCount;
    int secondCount;
    EventReceiver *firstObject;
    EventReceiver *secondObject;
    int firstIndex;
    int secondIndex;

    collectGameObjects(0x50, &firstCollection);
    collectGameObjects(0x60, &secondCollection);
    firstCount = firstCollection.size();
    secondCount = secondCollection.size();
    for (firstIndex = 0; firstIndex < firstCount; firstIndex++) {
        firstObject = static_cast<EventReceiver *>(firstCollection[firstIndex]);
        firstObject->dispatchFirst(event->type);
    }
    for (secondIndex = 0; secondIndex < secondCount; secondIndex++) {
        secondObject = static_cast<EventReceiver *>(secondCollection[secondIndex]);
        secondObject->dispatchSecond(event->type);
    }
}
