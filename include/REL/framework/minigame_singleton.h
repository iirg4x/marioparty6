#ifndef REL_FRAMEWORK_MINIGAME_SINGLETON_H
#define REL_FRAMEWORK_MINIGAME_SINGLETON_H

extern "C" {
#include "REL/framework/assertion.h"
}

// Keeps one current game interface so shared callbacks and result helpers can reach it.
template <class Game> class MinigameSingleton {
public:
    static Game *self; // Current interface; zero before construction and after
                                    // deletion.

    // During game construction, require that no other interface is active, then select this one.
    MinigameSingleton()
    {
        !self ? (void)0 : __msl_assertion_failed("self == 0", "singleton.h", 50);
        self = static_cast<Game *>(this);
    }

    // Game deletion clears the interface used by shared callbacks.
    ~MinigameSingleton() { self = 0; }

    // Frame callbacks use this when game construction is already complete.
    static Game &getUnchecked() { return *self; }
    // Result helpers require an active game before reading its shared state.
    static Game &getChecked()
    {
        (self != 0) ? (void)0 : __msl_assertion_failed("self != 0", "singleton.h", 38);
        return *self;
    }
};

template <class Game> Game *MinigameSingleton<Game>::self;

#endif
