/* Minigame REL startup, shutdown, and sequence setup. */
#include "dolphin/math.h"
#include "game/object.h"
#include "game/mg/seqman.h"
#include "game/mg/actman.h"
#include "REL/m617dll/shared.h"

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_A0(void);

/* The REL loader calls this entry point to run constructors and start the game. */
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

/* The REL loader calls this entry point on unload to run registered destructors. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Called at startup to create actor management and configure the sequence delays. */
void fn_1_A0(void)
{
    lbl_1_bss_38E4 = fn_80080418();
    fn_8003AB38(lbl_1_bss_38E4, 32730, 0U, 0U, -1, fn_8003D4FC);
    fn_80070788(&lbl_1_data_0);
    fn_80071CE8(2, 1000);
    fn_80071CE8(7, 1000);
}
