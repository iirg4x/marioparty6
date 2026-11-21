#ifndef REL_M623DLL_SCORE_OWNER_H
#define REL_M623DLL_SCORE_OWNER_H
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
extern "C" {
#include "game/mg/score.h"
}

// Holds a group's score and the sprites used to display it.
struct MGScoreEntry {
    // Sprite handle for the score box.
    short box;
    // Sprite group used for the display.
    short spriteGroup;
    // Numeric score display.
    MGSCORE *score;
    // Current coin count.
    int value;
    // Screen X of the display.
    float x;
    // Screen Y of the display.
    float y;
};

// Owns the two group score displays.
class MGScoreOwner : public RequestObject, public MinigameSingleton<MGScoreOwner> {
  public:
    MGScoreOwner();
    virtual ~MGScoreOwner();
    virtual void flagEnabled(int request);
    virtual void update();
    void showEntry(MGScoreEntry *entry);
    // The game's code never reads this field.
    unsigned char unk10;
    MGScoreEntry entries[2];
};

// Adds to the displayed coin count for a score entry.
void fn_1_8A10(unsigned long entryIndex, int increment);

// Returns the coin count for a score entry.
int fn_1_8AAC(unsigned long entryIndex);

#endif
