// Initialize the shared object-system state used when a minigame or mode starts.
#define _MATH_H
#include "game/object.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/pad.h"

#define OM_GAME_EXIT_CHECK_PRIO 32732

// Set by minigame mode flow when its pause menu should offer an exit action.
extern BOOL MgPauseExitF;
// Pause-menu callback that handles the minigame exit action.
extern void GameMesExitCheck(OMOBJ *obj);

// Called by mode and minigame setup after creating its object manager.
// Resets each player's minigame coin and score, and resets bonus coins except
// in practice mode. Sets the default viewport and pause behavior, ensures the
// message-window system is initialized with message data 0 (HuWinInit returns
// early if already initialized), and installs the optional pause-menu callback.
void omGameSysInit(OMOBJMAN *objectManager)
{
    int playerNo;
    OMOBJ *exitCheckObj;

    omSystemKeyCheckSetup(objectManager);
    Hu3DCameraScissorSet(1, 0, 0, 640, 480);
    omSysPauseEnable(0);

    for(playerNo=0; playerNo<GW_PLAYER_MAX; playerNo++) {
        GWMgCoinBonusSet(playerNo, 0);
        GWMgCoinSet(playerNo, 0);
        GWMgScoreSet(playerNo, 0);
    }

    if(MgPauseExitF) {
        exitCheckObj =
            omAddObjEx(objectManager, OM_GAME_EXIT_CHECK_PRIO, 0, 0, -1, GameMesExitCheck);
        // Keep the exit callback active while pause handling stops other objects.
        omSetStatBit(exitCheckObj, OM_STAT_NOPAUSE|OM_STAT_SPRPAUSE);
    }
    HuWinInit(0);
}

// Requests rumble on the configured pad only when no wipe transition is active,
// vibration is enabled and the player is human; the pad routine ignores pads
// with an error status.
void omVibrate(s16 playerNo, s16 rumbleMaxTime, s16 rumbleOffTime, s16 rumbleOnTime)
{
    if (!WipeCheckIn() && GWVibrateGet() != FALSE &&
        GwPlayerConf[playerNo].type == GW_PLAYER_TYPE_MAN) {
        HuPadRumbleSet(GwPlayerConf[playerNo].padNo, rumbleMaxTime, rumbleOffTime, rumbleOnTime);
    }
}

// Used by system-key handling to read a controller error status.
// The legacy -4 status is treated as PAD_ERR_NONE; all other statuses pass through.
s16 omPadErrChk(s16 padNo)
{
    s16 padStatus = HuPadStatGet(padNo);
    if(padStatus == -4) {
        return PAD_ERR_NONE;
    } else {
        return padStatus;
    }
}
