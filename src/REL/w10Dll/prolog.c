/* Runs module constructors at startup and destructors during shutdown. */
#include "REL/w10Dll/w10.h"
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

/* Runs module constructors and initializes W10 when its REL is loaded. */
int _prolog(void) {
    const VoidFunc *constructor = _ctors;
    while (*constructor != 0) {
        (*constructor)();
        constructor++;
    }
    ObjectSetup();
    return 0;
}

/* Runs module destructors when W10's REL is unloaded. */
void _epilog(void) {
    const VoidFunc *destructor = _dtors;
    while (*destructor != 0) {
        (*destructor)();
        destructor++;
    }
}
