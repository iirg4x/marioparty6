/* Runs module constructors and destructors and exposes its object manager. */
#include "dolphin/math.h"
#include "game/object.h"

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
extern HUPROCESS *lbl_1_bss_0[2];
void fn_1_530(void);

/* Runs module constructors before starting Rocky Road's object manager. */
int _prolog(void)
{
    const VoidFunc *constructor = _ctors;
    while (*constructor != 0) {
        (*constructor)();
        constructor++;
    }
    fn_1_530();
    return 0;
}

/* Runs the module destructors during unloading. */
void _epilog(void)
{
    const VoidFunc *destructor = _dtors;
    while (*destructor != 0) {
        (*destructor)();
        destructor++;
    }
}

/* Returns the object manager shared by Rocky Road's scene and gameplay objects. */
OMOBJMAN *fn_1_A0(void)
{
    return lbl_1_bss_0[0];
}
