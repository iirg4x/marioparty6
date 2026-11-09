#ifndef REL_M654_SPRITE_MANAGER_H
#define REL_M654_SPRITE_MANAGER_H
#include "REL/framework/game_object.h"
#include "REL/m654DLL/manager_event_base.h"
#include "REL/framework/minigame_singleton.h"
// Owns one player's score display: a new display replaces and deletes the one held.
struct ScoreDisplaySlot {
    GameObject *value;
    ScoreDisplaySlot(GameObject *initial = 0);
    ~ScoreDisplaySlot();
    ScoreDisplaySlot &operator=(GameObject *object)
    {
        if (value != object) {
            delete value;
            value = object;
        }
        return *this;
    }
};
struct SpriteManager : ManagerEventBase, MinigameSingleton<SpriteManager> {
    ScoreDisplaySlot owners[4];
    SpriteManager();
    virtual ~SpriteManager();
    virtual void handleEvent(int event);
    virtual void advanceFrame();
};
GameObject *fn_1_6EA8(int, int, int);
#endif
