/* Starts and shuts down the instruction-screen module. */
#include <math.h>
#include "REL/instDll.h"

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

/* Called by the module loader on screen load to run constructors and initialize it. */
int _prolog(void)
{
    const VoidFunc *constructor = _ctors;
    while (*constructor != 0) {
        (*constructor)();
        constructor++;
    }
    ObjectSetup();
    return 0;
}

/* Called by the module loader on unload to run registered destructors. */
void _epilog(void)
{
    const VoidFunc *destructor = _dtors;
    while (*destructor != 0) {
        (*destructor)();
        destructor++;
    }
}
