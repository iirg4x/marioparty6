// Runs the overlay's startup and shutdown entry points.
#include "math.h"

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_A0(void);

// The REL loader calls this when ttwarsdll starts; constructors run before the overlay scene setup.
int _prolog(void)
{
    const VoidFunc *constructor = _ctors;
    while (*constructor != 0) {
        (*constructor)();
        constructor++;
    }
    fn_1_A0();
    return 0;
}

// The REL loader calls this during unload so registered static objects can release their state.
void _epilog(void)
{
    const VoidFunc *destructor = _dtors;
    while (*destructor != 0) {
        (*destructor)();
        destructor++;
    }
}
