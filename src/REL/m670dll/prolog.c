/* Starts the microphone pillar game when its overlay is loaded and runs its teardown on unload. */
#include "REL/m670dll.h"
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
/* The overlay loader calls this to run constructors and create the game scene. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_1658();
    return 0;
}
/* The overlay loader calls this while unloading the game scene. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}
