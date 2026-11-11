/* Runs board startup constructors, initializes the board and runs shutdown destructors. */
#include "game/gamework.h"
#include "REL/w05Dll/work.h"

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

/* The REL manager calls this after linking Castaway Bay; run constructors before board setup. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    ObjectSetup();
    return 0;
}

/* The REL manager calls this while unloading Castaway Bay; run its registered destructors. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}
