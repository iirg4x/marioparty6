#define _MATH_H
#include "game/mg/actman.h"
#include "game/mg/seqman.h"
#include "REL/m668DLL/data.h"

enum { M668_ACTOR_STACK_SIZE = 36864 };

typedef void (*M668VoidFunc)(void);
extern const M668VoidFunc _ctors[];
extern const M668VoidFunc _dtors[];
extern OMOBJMAN *lbl_1_bss_4;
void fn_1_A0(void);
void fn_1_F4(void);

int _prolog(void)
{
    const M668VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_A0();
    return 0;
}

void _epilog(void)
{
    const M668VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

void fn_1_A0(void)
{
    lbl_1_bss_4 = MgActorObjectSetup();
    HuPrcChildCreate(fn_1_F4, 100, M668_ACTOR_STACK_SIZE, 0, lbl_1_bss_4);
}

void fn_1_F4(void)
{
    MgSeqCreate(&lbl_1_data_0.param);
    for (;;) {
        MgActorExec();
        HuPrcVSleep();
    }
}
