// One object of a class at a time: the minigame's game object and the scene objects that
// callbacks and result helpers reach through it.
//
// The asserts sit on lines 38 (self != 0) and 50 (self == 0), so the checked accessor comes before
// the constructor. The class and function names are descriptive.
#ifndef REL_FRAMEWORK_SINGLETON_H
#define REL_FRAMEWORK_SINGLETON_H

// The build defines NDEBUG for every object; these asserts are compiled in.
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"

// Keeps one current object of Game so shared callbacks and result helpers can reach it.
//
// Usage: a class derives from MinigameSingleton<itself>. Constructing it makes it the current
// object and deleting it clears the pointer, so code outside the class (frame callbacks, result
// helpers, other scene objects) reaches it as MinigameSingleton<Game>::getChecked() without a
// pointer of its own.
//
// getChecked() asserts that the object exists: callers may run before it is constructed or after
// it is deleted. current() returns the pointer unchecked, zero when no object exists. Only one
// object of a class may exist at a time; constructing a second one while the first is alive
// asserts.
template <class Game> class MinigameSingleton {
public:
    // The current object: zero before construction and after deletion.
    static Game *self;

    // Game deletion clears the pointer used by shared callbacks.
    ~MinigameSingleton()
    {
        self = 0;
    }

    // Result helpers require an existing object before reading its shared state.
    static Game &getChecked()
    {
        assert(self != 0);
        return *self;
    }

    static Game *current()
    {
        return self;
    }

    // During construction, require that no other object is current, then select this one.
    MinigameSingleton()
    {
        assert(self == 0);
        self = static_cast<Game *>(this);
    }
};

template <class Game> Game *MinigameSingleton<Game>::self;

#endif
