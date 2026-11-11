#include "Runtime.PPCEABI.H/NMWException.h"

DestructorChain *__global_destructor_chain;

void *__register_global_object(void *object, void *destructor, void *regmem)
{
    static_cast<DestructorChain *>(regmem)->next = __global_destructor_chain;
    static_cast<DestructorChain *>(regmem)->destructor = destructor;
    static_cast<DestructorChain *>(regmem)->object = object;
    __global_destructor_chain = static_cast<DestructorChain *>(regmem);
    return object;
}

void __destroy_global_chain(void)
{
    DestructorChain *record;
    while ((record = __global_destructor_chain) != 0) {
        __global_destructor_chain = record->next;
        DTORCALL(record->destructor, record->object);
    }
}

// Runtime.PPCEABI.H/global_destructor_chain.c retains this termination entry.
__declspec(section ".dtors")
static void *const __destroy_global_chain_reference = __destroy_global_chain;
