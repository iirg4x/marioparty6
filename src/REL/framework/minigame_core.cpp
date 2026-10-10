// Shared object-manager control, result messages and exit flow for C++ minigames.
static const char rcsid[] = "$Id: minigame_core.cpp,v 1.35.2.1 2004/06/23 07:40:41 hanamasu Exp $";

#include "dolphin/math.h"
#include "REL/framework/minigame_core.h"

extern "C" {
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/gamework.h"
#include "game/mg/actman.h"
#include "game/mgdata.h"
#include "game/wipe.h"
}

void emptyThreadHook();

// Durations in frames for the game's introduction, start and finish phases.
const long MinigameCore::introDelay = secondsToFrames(2L);
const long MinigameCore::startDelay = secondsToFrames(3.5);
const long MinigameCore::finishDelay = secondsToFrames(3.5);

// Constructed before the game's own setup; begin with no message delay or winners.
MinigameCore::MinigameCore() : messageFrames(0)
{
    // This shared thread hook has no effect.
    emptyThreadHook();
    for (int index = 0; index < 4; ++index)
        winners[index] = -1;
}

// At game deletion, set each registered winner's bonus to ten coins outside battle and practice.
MinigameCore::~MinigameCore()
{
    if (MgDataTbl[GwSystem.mgNo].type != MG_TYPE_BATTLE) {
        MinigameCore &game = static_cast<MinigameCore &>(getChecked());
        for (int index = 0; index < 4; ++index) {
            int playerNo = game.winners[index];
            if (playerNo >= 0 && !_CheckFlag(FLAG_MG_PRACTICE))
                GwPlayer[playerNo].mgCoinBonus = 10;
        }
    }
}

// Called during game setup to initialize the object system and register the game's objects.
// A model-free object then polls exit requests and runs the shared frame update.
void MinigameCore::setup(int objectLimit, int managerPriority)
{
    OMOBJMAN *manager = omInitObjMan(objectLimit, managerPriority);
    omGameSysInit(manager);
    setupObjects(manager);
    omAddObjEx(manager, 0, 0, 0, OM_GRP_NONE, updateCallback);
}

// Game setup uses this path when it needs the actor manager's player and actor arrays.
// Register the game's objects before installing the shared frame callback.
void MinigameCore::setupActorManager()
{
    OMOBJMAN *manager = MgActorObjectSetup();
    setupObjects(manager);
    omAddObjEx(manager, 0, 0, 0, OM_GRP_NONE, updateCallback);
}

// Called by the object manager each frame; start exit only when no wipe is active.
// While waiting for that opportunity, continue the game's regular frame update.
void MinigameCore::updateCallback(OMOBJ *object)
{
    if (omSysExitReq != 0 && !WipeCheck()) {
        wipeOut();
        // Sound effects stop immediately; sequence and streamed music fade out.
        HuAudFadeOut(1000);
        object->objFunc = exitCallback;
    } else {
        MinigameCore &game = static_cast<MinigameCore &>(getUnchecked());
        game.update();
    }
}

// Called by updateCallback each regular frame to run the game and count down its message delay.
void MinigameCore::update()
{
    updateGame();
    decrementMessageFrames();
}

// The frame callbacks use this empty default unless the game overrides it.
void MinigameCore::updateGame() {}

// Called by the object manager after an exit request; keep the game updating through the wipe.
// Once it ends, run game cleanup, delete the game, and request the preceding overlay.
void MinigameCore::exitCallback(OMOBJ *)
{
    MinigameCore *game = static_cast<MinigameCore *>(&getUnchecked());
    if (WipeCheck()) {
        game->updateGame();
        game->decrementMessageFrames();
        return;
    }
    game->finishGame();
    delete game;
    CharModelKill(CHARNO_NONE);
    omOvlReturnEx(1, 1);
}

// Game result code registers a player in the first free winner slot.
// Duplicate players are accepted; once all four slots are filled, additions are ignored.
void MinigameCore::addWinner(int playerNo)
{
    MinigameCore &game = static_cast<MinigameCore &>(getChecked());
    int *slots = game.winners;
    for (int index = 0; index < 4; ++index) {
        if (slots[index] < 0) {
            slots[index] = playerNo;
            break;
        }
    }
}

// Game code uses this to test the winner list; negative inputs can also match empty slots.
int MinigameCore::hasWinner(int playerNo)
{
    MinigameCore &game = static_cast<MinigameCore &>(getChecked());
    int *slots = game.winners;
    for (int index = 0; index < 4; ++index) {
        if (playerNo == slots[index])
            return 1;
    }
    return 0;
}

// Game phase code calls this to show its minigame-type start banner and save the message handle.
void MinigameCore::showStart()
{
    MinigameCore &game = static_cast<MinigameCore &>(getChecked());
    game.messageId = GameMesCreate(GAMEMES_MES_MG, GAMEMES_MG_TYPE_START);
}

// Game phase code calls this to show its minigame-type finish banner and save the message handle.
void MinigameCore::showFinish()
{
    MinigameCore &game = static_cast<MinigameCore &>(getChecked());
    game.messageId = GameMesCreate(GAMEMES_MES_MG, GAMEMES_MG_TYPE_FINISH);
}

// Used by the message-completion helpers; zero and negative frame counts are already expired.
inline bool messageDelayExpired(const MinigameCore &game)
{
    return game.messageFrames <= 0;
}

// Supply the delay test as a byte flag for messageDone.
inline unsigned char messageDelayExpiredFlag(const MinigameCore &game)
{
    return messageDelayExpired(game);
}

// Game phase code polls this until every live message and the game's frame delay have finished.
// This uses combined message status; a timer or another message also keeps this false.
bool MinigameCore::messageDone()
{
    MinigameCore &game = static_cast<MinigameCore &>(getChecked());
    bool isDone = false;
    if (GameMesStatGet(GAMEMES_ID_NONE) == 0 && messageDelayExpiredFlag(game))
        isDone = true;
    return isDone;
}

// Game result code calls this to start the result delay and sort winners by player number.
// Display their character names, or a draw when the first winner slot is empty.
void MinigameCore::showWinners()
{
    MinigameCore &game = static_cast<MinigameCore &>(getChecked());
    game.messageFrames = startDelay;
    int *entries = game.winners;
    int winnerCount = 0;
    for (; winnerCount < 4; ++winnerCount) {
        if (entries[winnerCount] < 0)
            break;
    }
    // Result message handles are not saved; messageDone checks the entire message system.
    if (winnerCount == 0) {
        GameMesCreate(GAMEMES_MES_MG, GAMEMES_MG_TYPE_DRAW);
        return;
    }
    for (int index = 1; index < winnerCount; ++index) {
        int cursor = index;
        while (cursor >= 1 && entries[cursor - 1] > entries[cursor]) {
            int previousPlayerNo = entries[cursor - 1];
            entries[cursor - 1] = entries[cursor];
            entries[cursor] = previousPlayerNo;
            --cursor;
        }
    }
    int characterNos[4];
    for (int index = 0; index < 4; ++index)
        characterNos[index] = entries[index] < 0 ? -1 : GwPlayerConf[entries[index]].charNo;
    GameMesCreate(GAMEMES_MES_MG_WINNER, GAMEMES_MG_TYPE_WIN, characterNos[0], characterNos[1],
                  characterNos[2], characterNos[3]);
}

// Game result code calls this with its record score or time; practice suppresses both banner and
// jingle.
void MinigameCore::showRecord(int recordValue)
{
    if (_CheckFlag(FLAG_MG_PRACTICE))
        return;
    MinigameCore &game = static_cast<MinigameCore &>(getChecked());
    game.messageId = GameMesCreate(GAMEMES_MES_MG_RECORD, recordValue);
    HuAudJinglePlay(MSM_STREAM_MGMUS_16);
}

// Game phase code requests a 60-frame incoming wipe using the previously selected effect.
void MinigameCore::wipeIn()
{
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREV, 60);
}

// updateCallback requests this 60-frame outgoing normal wipe before game cleanup.
void MinigameCore::wipeOut()
{
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
}

// Game code sets the exit request; updateCallback waits for a wipe-free frame to handle it.
void MinigameCore::requestExit()
{
    omSysExitReq = 1;
}

// Convert the phase durations from seconds to frames at 60 frames per second.
template <class Time>
Time secondsToFrames(Time seconds)
{
    return 60 * seconds;
}
