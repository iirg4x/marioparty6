// Collects player, actor, and scene object positions.

#include "dolphin/mtx.h"
#include "dolphin/types.h"
#include "REL/m673DLL/player_actor.h"
#include "REL/m673DLL/moving_actor.h"
#include "REL/framework/game_object_man.h"
#include "REL/m673DLL/position_table.h"

// Initializes the vectors that collect players and scene positions.
ScenePositionTable::ScenePositionTable() : GameObject(5), MinigameSingleton<ScenePositionTable>()
{
    this->fn_1_7A14();
    this->fn_1_7B24();
    this->fn_1_7C08();
}

// Stores a player's position and marks its tracking entry active.
inline void ScenePositionTable::setTarget(s32 index, PlayerActor *player)
{
    unk1C[index] = player->component.position.value;
    unk4C[index] = 1;
}

// Destroys the vectors of collected players and scene objects.
ScenePositionTable::~ScenePositionTable()
{
}

// Collects the player actors and initializes their tracked positions.
void ScenePositionTable::fn_1_7A14()
{
    std::vector<GameObject *> playerObjects;
    s32 index;

    collectGameObjects(1, &playerObjects);
    for (index = 0; index < 4; index++) {
        PlayerActor *player = static_cast<PlayerActor *>(playerObjects[index]);
        this->setTarget(index, player);
        this->players.push_back(player);
    }
}

// A category-3 scene object provides a position after its GameObject base.
struct PositionSource : GameObject {
    Vec position;
};

// Copies category 3 scene positions into the position table.
void ScenePositionTable::fn_1_7B24()
{
    std::vector<GameObject *> objects;
    s32 count;
    s32 index;
    PositionSource *source;

    collectGameObjects(3, &objects);
    count = objects.size();
    for (index = 0; index < count; index++) {
        source = static_cast<PositionSource *>(objects[index]);
        Vec position = source->position;
        this->unk50.push_back(position);
    }
}

// Resets the tracked moving actor's position, angle, and state.
void ScenePositionTable::fn_1_7C08()
{
    this->position.x = 0.0f;
    this->position.y = 0.0f;
    this->position.z = 0.0f;
    this->angle = 0.0f;
    this->state = -1;
}

// Refreshes the tracked player and moving-actor state.
void ScenePositionTable::update()
{
    fn_1_7C8C(this);
    fn_1_7D6C(this);
}

// Stores player positions and activity flags.
void fn_1_7C8C(ScenePositionTable *table)
{
    std::vector<GameObject *> objects;
    PlayerActor *player;
    s32 index;

    collectGameObjects(1, &objects);
    for (index = 0; index < 4; ++index) {
        player = static_cast<PlayerActor *>(objects[index]);
        table->unk1C[index] = player->component.position.value;
        table->unk4C[index] = player->isActive();
    }
}

// Refreshes the moving actor's position, heading, and movement state.
void fn_1_7D6C(ScenePositionTable *table)
{
    std::vector<GameObject *> actors;
    MovingActor *actor;
    f32 heading;

    collectGameObjects(2, &actors);
    actor = static_cast<MovingActor *>(actors[0]);
    Vec position = actor->component.position.value;
    table->position.x = position.x;
    table->position.y = position.y;
    table->position.z = position.z;
    heading = actor->headingAngle;
    table->angle = heading;
    table->state = actor->getMovementState();
}
