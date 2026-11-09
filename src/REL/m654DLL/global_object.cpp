#include "Runtime.PPCEABI.H/NMWException.h"

DestructorChain *lbl_1_bss_E8;

void *__register_global_object(void *object, void *destructor, void *regmem)
{
    static_cast<DestructorChain *>(regmem)->next = lbl_1_bss_E8;
    static_cast<DestructorChain *>(regmem)->destructor = destructor;
    static_cast<DestructorChain *>(regmem)->object = object;
    lbl_1_bss_E8 = static_cast<DestructorChain *>(regmem);
    return object;
}

void __destroy_global_chain(void)
{
    DestructorChain *record;
    while ((record = lbl_1_bss_E8) != 0) {
        lbl_1_bss_E8 = record->next;
        DTORCALL(record->destructor, record->object);
    }
}

// Runtime.PPCEABI.H/global_destructor_chain.c retains this termination entry.
__declspec(section ".dtors")
static void *const __destroy_global_chain_reference = __destroy_global_chain;
