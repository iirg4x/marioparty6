/* Module entry and exit, and the minigame sequence parameters that start the maze game. */
#include "REL/m679dll/module_types.h"

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

HUPROCESS *lbl_1_bss_0;
void fn_1_164(void);

int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_164();
    return 0;
}

void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

HUPROCESS *fn_1_A0(void)
{
    return lbl_1_bss_0;
}

void fn_1_B0(s16 mode, s16 frameNo)
{
    fn_80071A9C();
}

void fn_1_D0(s16 mode, s16 frameNo)
{
    fn_80071AC4(5U);
}

void fn_1_F4(s16 mode, s16 frameNo)
{
}

void fn_1_F8(s16 mode, s16 frameNo)
{
    if (fn_80071B58() == 0U) {
        fn_80071B08();
    }
    if (fn_1_D830() == 0) {
        fn_800856A8();
    }
}

void fn_1_134(s16 mode, s16 frameNo)
{
    fn_80071A9C();
}

void fn_1_154(s16 mode, s16 frameNo)
{
}

void fn_1_158(s16 mode, s16 frameNo)
{
}

void fn_1_15C(s16 mode, s16 frameNo)
{
}

void fn_1_160(s16 mode, s16 frameNo)
{
}

/* The game's code never reads this value. */
static s32 data_0 = -1;
MGSEQ_PARAM lbl_1_data_4 = {
    0, 0, fn_1_B0, fn_1_D0, fn_1_F4, fn_1_F8, fn_1_134,
    fn_1_154, fn_1_158, fn_1_15C, fn_1_160
};

void fn_1_164(void)
{
    lbl_1_bss_0 = fn_80080418();
    fn_80070788(&lbl_1_data_4);
    fn_1_D7A0();
}
