#include "Runtime.PPCEABI.H/NMWException.h"

extern "C" {
DestructorChain *lbl_1_bss_610;
}

void *__register_global_object(void *object, void *destructor, void *regmem) {
    ((DestructorChain *)regmem)->next = lbl_1_bss_610;
    ((DestructorChain *)regmem)->destructor = destructor;
    ((DestructorChain *)regmem)->object = object;
    lbl_1_bss_610 = (DestructorChain *)regmem;
    return object;
}

void fn_1_10060(void) {
    DestructorChain *iter;
    while ((iter = lbl_1_bss_610) != 0) {
        lbl_1_bss_610 = iter->next;
        DTORCALL(iter->destructor, iter->object);
    }
}



__declspec(section ".dtors")
static void *const __destroy_global_chain_reference = fn_1_10060;
