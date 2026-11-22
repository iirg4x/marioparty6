// Declares the thread that runs the minigame sequence.

#ifndef REL_M673DLL_MINIGAME_THREAD_H
#define REL_M673DLL_MINIGAME_THREAD_H

#include <memory>
#include "dolphin/types.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"

struct TimerOwner;
struct SceneChild;

// Runs the scene and tracks the game timer.
struct MinigameThread : MinigameCore, ThreadEntry {
    std::auto_ptr<TimerOwner> hudTimer; // On-screen timer used during play.
    std::auto_ptr<SceneChild> child; // Scene managers owned by this thread.
    // Destroys the game timer and child scene managers.
    virtual ~MinigameThread();
    // Creates the game timer and child scene managers.
    MinigameThread();
    // Creates the camera, scene objects, player actors, and position table.
    virtual void setupObjects(OMOBJMAN *);
    // Resumes the thread and advances scene events, objects, and collision checks.
    virtual void updateGame();
    // Leaves the scene unchanged at the end of the game.
    virtual void finishGame();
    // Runs the minigame's opening, play, result, and closing phases.
    virtual void run(void *initialArgument);
};

// Accumulates elapsed time and polls the HUD timer. Play ends when the timer expires
// or four players are inactive.
u8 fn_1_7080(MinigameThread *owner);

// Counts players who are out of play.
s32 fn_1_74A4(MinigameThread *);

// Returns whether all players and the moving actor have finished their current phase.
u8 fn_1_731C(MinigameThread *);

#endif
