/* Black Hole runs global constructors before module setup and destructors on exit. */
#include "REL/m651dll.h"

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

/* Runs module constructors, then creates the game object manager and scene. */
int _prolog(void)
{
    const VoidFunc *constructorEntry = _ctors;
    while (*constructorEntry != 0) {
        (*constructorEntry)();
        constructorEntry++;
    }
    fn_1_40C();
    return 0;
}

/* Runs registered module destructors when the REL is unloaded. */
void _epilog(void)
{
    const VoidFunc *destructorEntry = _dtors;
    while (*destructorEntry != 0) {
        (*destructorEntry)();
        destructorEntry++;
    }
}
