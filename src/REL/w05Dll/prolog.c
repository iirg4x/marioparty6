/* Runs board startup constructors, initializes the board and runs shutdown destructors. */
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_A0(void);

/* The REL manager calls this after linking Castaway Bay; run constructors before board setup. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_A0();
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

#include "game/gamework.h"

void mbObjectSetup(s32 boardNo, void (*init)(void), void (*close)(void));
void fn_1_F4(void);
void fn_1_1D50(void);
