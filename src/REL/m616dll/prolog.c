/* REL entry points that initialize and tear down M616's timed game sequence. */
#include "REL/m616dll.h"

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

/* The REL loader calls this once; construct globals before creating the game objects. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;

    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_1590();
    return 0;
}

/* The REL loader calls this before unloading the module to destroy constructed globals. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;

    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}
