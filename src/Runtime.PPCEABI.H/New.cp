#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/alloc.h"
#include "PowerPC_EABI_Support/Runtime/New.h"

std::new_handler std::__new_handler = 0;

void* operator new(unsigned long arg0) {
    void* result;

    if (arg0 == 0) {
        arg0 = sizeof(int);
    }

    for (;;) {
        if ((result = malloc(arg0)) != NULL) {
            break;
        }
        if (!std::__new_handler) {
            std::__throw_bad_alloc();
            break;
        }
        std::__new_handler();
    }

    return result;
}

void operator delete(void* arg0) throw() {
    if (arg0 != 0) {
        free(arg0);
    }
}

void* operator new[](unsigned long arg0) {
    return operator new(arg0);
}

void operator delete[](void* arg0) throw() {
    if (arg0 != 0) {
        free(arg0);
    }
}
