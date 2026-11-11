typedef void (*UpdateHook)(void);

extern UpdateHook lbl_1_bss_24;
extern UpdateHook lbl_1_bss_20;

void fn_1_300(UpdateHook hook)
{
    lbl_1_bss_24 = hook;
}

void fn_1_310(UpdateHook hook)
{
    lbl_1_bss_20 = hook;
}
