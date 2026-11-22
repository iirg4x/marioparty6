// Declares the position table for the scene and its actors.

#ifndef REL_M673DLL_POSITION_TABLE_H
#define REL_M673DLL_POSITION_TABLE_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/framework/game_object2.h"
#include "REL/framework/singleton.h"
#include <vector>
class PlayerActor;

// Collects players and scene positions by object category.
struct ScenePositionTable : GameObject, MinigameSingleton<ScenePositionTable> {
    std::vector<PlayerActor *> players; // Players in slot order.
    Vec unk1C[4]; /* The game's code never reads this field. */
    u8 unk4C[4]; /* The game's code never reads this field. */
    std::vector<Vec> unk50; /* The game's code never reads this field. */
    Vec position; // Current position of the moving actor.
    f32 angle; // Current heading of the moving actor.
    s32 state; // Current state of the moving actor.
    // Stores a player's position and marks its tracking entry active.
    void setTarget(s32 index, PlayerActor *player);
    // Initializes the vectors that collect players and scene positions.
    ScenePositionTable();
    // Destroys the vectors of collected players and scene objects.
    virtual ~ScenePositionTable();
    // Refreshes the tracked player and moving-actor state.
    virtual void update();
    // Resets the tracked moving actor's position, angle, and state.
    void fn_1_7C08();
    // Copies category 3 scene positions into the position table.
    void fn_1_7B24();
    // Collects the player actors and initializes their tracked positions.
    void fn_1_7A14();
    // Returns the moving actor's tracked position.
    Vec getPosition() const
    {
        return position;
    }

    // Returns the moving actor's tracked heading.
    f32 getAngle() const
    {
        return angle;
    }

    // Returns the moving actor's tracked movement state.
    s32 getState() const
    {
        return state;
    }
};

// Stores player positions and activity flags.
void fn_1_7C8C(ScenePositionTable *table);

// Refreshes the moving actor's position, heading, and movement state.
void fn_1_7D6C(ScenePositionTable *table);

#endif
