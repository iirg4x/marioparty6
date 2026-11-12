#include <vector>
#include <functional>
#include "REL/m671DLL/player_object.h"
#include "REL/m671DLL/animation_owner.h"
#include "REL/framework/linknode.h"
#include "REL/framework/game_object.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/event_hm.h"

#include "game/hu3d.h"
#include "game/data.h"

void collectGameObjects(int category, std::vector<GameObject *> *list);

#include "REL/m671DLL/position_table.h"

PositionTable::PositionTable() : GameObject(3), MinigameSingleton<PositionTable>() { fn_1_84BC(this); }
PositionTable::~PositionTable() {}

void PositionTable::fn_1_83DC()
{
    std::vector<GameObject *> objects;
    Player *object;
    int index;

    collectGameObjects(0, &objects);
    for (index = 0; index < 4; ++index) {
        object = (Player *)objects[index];

        this->positions[index] = object->collision.value;
        this->activeFlags[index] = 1;
    }
}

void fn_1_84BC(PositionTable *owner)
{
    std::vector<GameObject *> objects;
    int index;
    MotionPosition firstPosition;
    MotionPosition secondPosition;

    collectGameObjects(1, &objects);
    owner->count = objects.size();
    owner->activeCount = 0;
    for (index = 0; index < owner->count; ++index) {
        AnimationOwner *object = (AnimationOwner *)objects[index];

        firstPosition = object->position;
        owner->firstPositions[index].value = firstPosition.value;
        secondPosition = object->previousPosition;
        owner->secondPositions[index].value = secondPosition.value;
    }
}

void PositionTable::update()
{
    fn_1_8644(this);
    fn_1_8724(this);
}

void fn_1_8644(PositionTable *owner)
{
    std::vector<GameObject *> objects;
    Player *object;
    int index;

    collectGameObjects(0, &objects);
    for (index = 0; index < 4; ++index) {
        object = (Player *)objects[index];

        owner->positions[index] = object->collision.value;
        owner->activeFlags[index] = object->isActive();
    }
}

/* Copies the active objects' position records into the table for this mode. */
void fn_1_8724(PositionTable *owner)
{
    std::vector<GameObject *> objects;
    int index;
    MotionPosition firstPosition;
    MotionPosition secondPosition;
    float thirdPosition;

    collectGameObjects(1, &objects);
    owner->count = objects.size();
    owner->activeCount = 0;
    for (index = 0; index < owner->count; ++index) {
        AnimationOwner *object = (AnimationOwner *)objects[index];

        firstPosition = object->position;
        owner->firstPositions[index].value = firstPosition.value;
        secondPosition = object->previousPosition;
        owner->secondPositions[index].value = secondPosition.value;
        thirdPosition = object->descentSpeed;
        owner->thirdPositions[index] = thirdPosition;
        if (object->isActive() != 0) {
            ++owner->activeCount;
        }
    }
}
