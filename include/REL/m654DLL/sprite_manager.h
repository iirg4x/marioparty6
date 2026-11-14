#ifndef REL_M654_SPRITE_MANAGER_H
#define REL_M654_SPRITE_MANAGER_H
#include "REL/m654DLL/display_provider.h"
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
// Owns one player's score display: a new display replaces and deletes the one held.
struct ScoreDisplaySlot {
    DisplayProvider *value;
    ScoreDisplaySlot(DisplayProvider *initial = 0);
    ~ScoreDisplaySlot();
    // Replaces the owned display, deleting the previous one.
    ScoreDisplaySlot &operator=(DisplayProvider *object)
    {
        if (value != object) {
            delete value;
            value = object;
        }
        return *this;
    }
};
struct SpriteManager : RequestObject, MinigameSingleton<SpriteManager> {
    ScoreDisplaySlot owners[4];
    SpriteManager();
    virtual ~SpriteManager();
    virtual void flagEnabled(int event);
    virtual void update();
};
DisplayProvider *fn_1_6EA8(int, int, int);
#endif
