// Provides a player's configuration, controller input and rumble settings.
#ifndef REL_FRAMEWORK_P_INFO_H
#define REL_FRAMEWORK_P_INFO_H

#include "dolphin/types.h"

class PlayerInfo {
public:
    PlayerInfo(int playerNo);
    int padNo();                    // controller number, -1 for a computer player
    int characterNo();
    int groupNo();                  // team
    bool isComputer();
    int difficulty();               // computer difficulty
    void rumble(s32 type, s32 duration);
    // The controller's state from the pad arrays (game/pad.h), 0 for a computer player.
    int buttons();
    int buttonsDown();
    int buttonsRepeat();
    float stickX();
    float stickY();
    float subStickX();
    float subStickY();
    float triggerL();
    float triggerR();
    int dpad();
    int dpadDown();
    int dpadRepeat();
    int stickDirections();          // the main stick as D-pad buttons (fn_80006B90)
    int subStickDirections();       // the C stick as D-pad buttons (fn_80006D00)
    float height();                 // the character's height (fn_8005C4C8)
    // Returns the player configuration index.
    int number() const { return playerNo; }

    s32 playerNo;
};

// True when rumble is on but no human player's controller can rumble.
bool noRumbleController(void);
void setRumbleEnabled(unsigned char enabled);

#endif
