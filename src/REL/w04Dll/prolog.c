// Module initialization and shutdown.
#include "REL/w04Dll/board.h"
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

// The REL loader calls this on module load to run constructors and initialize the board.
int _prolog(void) {
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    ObjectSetup();
    return 0;
}

// The REL loader calls this on module unload to run the registered destructors.
void _epilog(void) {
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}
