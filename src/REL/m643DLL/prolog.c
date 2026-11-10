/* Runs C++ static constructors and the minigame startup hook on load. */
#include "REL/m643DLL/m643.h"
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[], _dtors[];
/* The runtime invokes this entry point to run constructors before setup. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) { (*ctor)(); ctor++; }
    ObjectSetup();
    return 0;
}
/* The runtime invokes this entry point to run registered destructors on unload. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) { (*dtor)(); dtor++; }
}
