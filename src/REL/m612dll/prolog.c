/* Module entry and exit hooks run the registered static initializers and finalizers. */
#include "REL/m612dll.h"
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

/* The loader calls this entry point before the Memory Lane scene is initialized. */
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

/* The loader calls this hook when the Memory Lane module is unloaded. */
void _epilog(void)
{
    const VoidFunc *destructor = _dtors;
    while (*destructor != 0) {
        (*destructor)();
        destructor++;
    }
}