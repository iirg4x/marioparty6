/* Runs the minigame module's constructors at load and destructors at unload. */
#include "REL/m635dll.h"

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

/* Run the module constructors and initialize the Garden Grab sequence at load. */
int _prolog(void)
{
    /* The loader-provided table ends with a null function pointer. */
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_A0();
    return 0;
}

/* Run the module destructors when the loader unloads Garden Grab. */
void _epilog(void)
{
    /* Release static resources in the order supplied by the module linker. */
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}
