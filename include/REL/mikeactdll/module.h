#ifndef MIKEACTDLL_MODULE_H
#define MIKEACTDLL_MODULE_H
#include "game/object.h"
void fn_1_250(OMOBJ *object);
#include "REL/mikeactdll/event_callback.h"
#include "REL/mikeactdll/model_hook_setup.h"
#include "REL/mikeactdll/module_types.h"

extern u16 lbl_1_bss_34;

void fn_1_1C0(void);

static inline void MikeActActivityInit(void)
{
    fn_1_4518(0);
    fn_1_45A0(0);
    fn_1_3208();
    fn_1_101C0((s16)lbl_1_bss_34);
    fn_1_3FC4();
    fn_1_D950();
    fn_1_1998();
    fn_1_E308();
    fn_1_598();
    fn_1_F1F8();
    fn_1_20E8();
}

#endif
