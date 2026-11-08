#ifndef REL_M622DLL_ASSERT_H
#define REL_M622DLL_ASSERT_H

extern "C" {
#include "REL/framework/assertion.h"
}

#ifndef NDEBUG
#define assert(condition) \
    ((condition) ? (void)0 : __msl_assertion_failed(#condition, __FILE__, __LINE__))
#else
#define assert(condition) ((void)0)
#endif

#endif
