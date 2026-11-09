#include "dolphin/types.h"
#include "Runtime.PPCEABI.H/NMWException.h"

/* Module-local CodeWarrior global-object chain. The record/API matches the
 * existing Runtime.PPCEABI.H implementation, not a game-object ABI emulation. */

extern "C" { DestructorChain *lbl_1_bss_70; }

void *__register_global_object(void *object, void *destructor, void *regmem) {
    ((DestructorChain *)regmem)->next = lbl_1_bss_70;
    ((DestructorChain *)regmem)->destructor = destructor;
    ((DestructorChain *)regmem)->object = object;
    lbl_1_bss_70 = (DestructorChain *)regmem;
    return object;
}

void __destroy_global_chain(void) {
    DestructorChain *iter;
    while ((iter = lbl_1_bss_70) != 0) {
        lbl_1_bss_70 = iter->next;
        DTORCALL(iter->destructor, iter->object);
    }
}

__declspec(section ".dtors")
static void (*const __destroy_global_chain_reference)(void) = __destroy_global_chain;
