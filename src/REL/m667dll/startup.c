/* Talkie Walkie loader callbacks and object-manager process state. */
#include "game/process.h"

HUPROCESS *lbl_1_bss_0; /* Object-manager process used by the minigame's scene objects. */

typedef void (*M667VoidFunc)(void);
extern const M667VoidFunc _ctors[];
extern const M667VoidFunc _dtors[];
void fn_1_148(void);

/* The loader runs constructors, then initializes the minigame module. */
int _prolog(void)
{
    const M667VoidFunc *constructor = _ctors;
    while (*constructor != 0) {
        (*constructor)();
        constructor++;
    }
    fn_1_148();
    return 0;
}

/* The loader runs registered destructors when the minigame module shuts down. */
void _epilog(void)
{
    const M667VoidFunc *destructor = _dtors;
    while (*destructor != 0) {
        (*destructor)();
        destructor++;
    }
}
