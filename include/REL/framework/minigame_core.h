// Shared lifetime, frame callbacks and result-message interface for C++ minigames.
#ifndef REL_FRAMEWORK_MINIGAME_CORE_H
#define REL_FRAMEWORK_MINIGAME_CORE_H

extern "C" {
#include "game/object.h"
}
#include "REL/framework/minigame_singleton.h"

template <class Time> Time secondsToFrames(Time seconds);

// Base interface used by the game's object-manager callbacks and result messages.
class MinigameCore : public MinigameSingleton<MinigameCore> {
public:
    MinigameCore();
    virtual ~MinigameCore();
    // Both setup paths ask the game to populate the newly created object manager.
    virtual void setupObjects(OMOBJMAN *) = 0;
    // Called on regular frames and while the outgoing exit wipe is active.
    // The frame that starts exit skips this callback.
    virtual void updateGame();
    // Called once after the outgoing exit wipe, just before the game is deleted.
    virtual void finishGame() = 0;

    int winners[4]; // Registered player numbers; negative entries are free slots.
    int messageId; // Last start, finish or record message handle; -1 if creation fails, unset until
                   // then.
    int messageFrames; // Delay in frames; zero or negative means expired, showWinners sets it to
                       // 210.

    static const long introDelay; // Introduction duration: 120 frames.
    static const long startDelay; // Start duration and winner-display delay: 210 frames.
    static const long finishDelay; // Finish duration: 210 frames.

    void setup(int objectLimit, int managerPriority);
    void setupActorManager();
    void update();
    static void updateCallback(OMOBJ *object);
    static void exitCallback(OMOBJ *object);
    static void addWinner(int playerNo);
    static int hasWinner(int playerNo);
    static void showStart();
    static void showFinish();
    static bool messageDone();
    static void showWinners();
    static void showRecord(int recordValue);
    static void wipeIn();
    static void wipeOut();
    static void requestExit();

    // Regular and exit frame callbacks decrement positive delays; return one only on reaching zero.
    int decrementMessageFrames()
    {
        int expired = 0;
        if (messageFrames > 0) {
            if ((messageFrames -= 1) == 0)
                expired = 1;
        }
        return expired;
    }
};

#endif
